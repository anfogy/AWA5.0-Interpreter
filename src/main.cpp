#include "argparse.hpp"
#include "AwaInterpreter.hpp"
#include "Awabler.hpp"
#include <unordered_set>
#include <array>

const std::vector<std::string> keywords = {
    "nop", "prn", "pr1", "red", "r3d", "blw", "sbm", "pop",
    "dpl", "srn", "mrg", "4dd", "sub", "mul", "div", "cnt",
    "lbl", "jmp", "eql", "lss", "gr8", "trm", "mov"
};

static const std::vector<std::string> argFilter = { "blw", "sbm", "srn", "lbl", "jmp", "mov", "pop"};

/**
* @brief Determines whether the input code is Awalang or Awably.
*
* @param input The code to be determined.
* 
* @return true for Awalang, false for Awably.
* 
* @remark Function is only called when isAwalang is not passed in as an argument.
*/
static bool determineAwaType(const std::string& input) {
    for (const auto& element : keywords) {
        if (element.find(input) != std::string::npos) {
            return false;
        }
    }

    return true;
}

/**
* @brief Filters inputted code.
* @details If the code is Awalang, it will keep only the last valid line of code.
*   If the code is Awably, it will filter out all invalid instructions and arguments.
* 
* @param awa The code to be filtered.
* @param isAwalang Whether the code is Awalang or not.
*/
static void filterInput(std::string& awa, std::optional<bool> isAwalang) {
    if (isAwalang.value()) {
        std::istringstream iss(awa);
        std::string line, lastValidLine;
        while (std::getline(iss, line)) {
            std::string trimmed = line;
            trimmed.erase(trimmed.find_last_not_of(" \t\r\n") + 1);
            trimmed.erase(0, trimmed.find_first_not_of(" \t\r\n"));
            if (!trimmed.empty()) {
                lastValidLine = trimmed;
            }
        }

        awa.clear();
        for (char c : lastValidLine) {
            if (c == 'a' || c == 'w' || c == ' ') {
                awa += std::tolower(c);
            }
        }
    }
    else {
        static const std::unordered_set<std::string> keywordSet(keywords.begin(), keywords.end());
        static const std::unordered_set<std::string> argFilterSet(argFilter.begin(), argFilter.end());
		replace(awa, ";", "\n");
        std::istringstream iss(awa);
        std::string line, filtered;
        while (std::getline(iss, line)) {
            std::istringstream linestream(line);
            std::string instruction, argument;
            linestream >> instruction >> argument;
            if (!instruction.empty() && keywordSet.count(instruction)) {
                if (!instruction.empty() && argFilterSet.count(instruction)) {
                    instruction += " " + argument;
                }
                filtered += instruction + "; ";
            }
        }
        awa = filtered;
    }
}

/**
* @brief Writes the stacktrace to a file named "stacktrace.txt".
* @details Writes stacktrace containing instruction, parameters, and stack status to a file named "stacktrace.txt".
*
* @param stacktrace The stacktrace to be written to the file.
* @param isLegacy Whether the stacktrace is generated from legacy Awa or not.
* 
* @remark This function is only called when debug mode is enabled.
*/
static void writeStacktrace(const std::vector<StacktraceEntry>& stacktrace, bool isLegacy) {
    if (stacktrace.empty()) {
        return;
    }
    std::cout << std::endl;

    std::ofstream ofs;
    ofs.open("stacktrace.txt", std::ofstream::out | std::ofstream::trunc);
    if (ofs.is_open()) {
        size_t maxStackLength = 0;

		// Calculate maximum stack length for formatting
        for (const StacktraceEntry& entry : stacktrace) {
            std::string stackLine;
            for (const Bubble& bubble : entry.stack) {
                if (isDouble(bubble)) {
                    BubbleVector list = getList(bubble);
                    for (size_t i = 0; i < list.size(); i++) {
                        if (i == 0) {
                            stackLine.append(" (");
                        }
                        stackLine.append(((i == 0) ? "" : " ") + std::to_string(getInt(list[i])));
                        if (i == list.size() - 1) {
                            stackLine.append(")");
                        }
                    }
                } else {
                    stackLine.append(" " + std::to_string(getInt(bubble)));
                }
            }

            maxStackLength = std::max(std::max(maxStackLength, stackLine.size() + entry.instruction.size() + 28), static_cast<size_t>(38));
        }

        ofs << "Step   Instruction          Stack";
        if (!isLegacy) ofs << std::string(std::max(static_cast<int>(maxStackLength) - 33, 5), ' ') << "Registers";
        ofs << std::endl;

        for (const StacktraceEntry& entry : stacktrace) {
            const std::vector<Bubble>& stack = entry.stack;
            const std::array<int, 16>& registers = entry.registers;
            int executionTime = entry.executionTime;
            const std::string& instruction = entry.instruction;

            std::string line;
            for (size_t p = 0; p < stack.size(); p++) {
                Bubble bubble = stack[p];

                if (isDouble(bubble)) {
                    BubbleVector list = getList(bubble);
                    for (size_t i = 0; i < list.size(); i++) {
                        if (i == 0) {
                            line.append(" (");
                        }
                        line.append(((i == 0) ? "" : " ") + std::to_string(getInt(list[i])));
                        if (i == list.size() - 1) {
                            line.append(")");
                        }
                    }
                } else {
                    line.append(" " + std::to_string(getInt(bubble)));
                }

                if (p == 0) {
                    int num = 17;
                    num -= static_cast<int>(instruction.size()) - 3;

                    std::string spaces(num, ' ');
                    line.insert(0, spaces);
                }
            }
            line.insert(0, instruction);

            std::ostringstream oss;
            oss << "[" << std::setfill('0') << std::setw(4) << executionTime << "] ";
            line.insert(0, oss.str());

            if (!isLegacy) {
                size_t padding = (maxStackLength > line.size()) ? (maxStackLength - line.size()) : 0;
                line.append(std::string(padding, ' '));
            }
            ofs << line;

            if (!isLegacy) {
                for (size_t regIndex = 0; regIndex < registers.size(); ++regIndex) {
                    ofs << "r" << regIndex << "=" << registers[regIndex];
                    if (regIndex < registers.size() - 1) {
                        ofs << ", ";
                    }
                }
            }
            ofs << std::endl;
        }
    }

    std::cout << "Stacktrace written to stacktrace.txt" << std::endl;

    ofs.close();
}

int main(int argc, char* argv[]) {
    auto args = parse_arguments(argc, argv);
    if (!args.valid) return 1;

    std::string awa = args.awa;
    std::string input = args.input;
    bool interactiveMode = args.interactiveMode;
    bool debugMode = args.debugMode;
	bool legacyMode = args.legacyMode;
    std::optional<bool> isAwalang = args.isAwalang;
    std::optional<std::string> filePath = args.filePath;
    std::string executableName = args.executableName;

    Awabler::verbose = debugMode;
    Awabler::legacy = legacyMode;

    if (filePath) {
        std::ifstream file(*filePath, std::ios::in | std::ios::binary);
        if (!file) {
            std::cerr << "Error: Unable to read " << *filePath << std::endl;

            return 1;
        }

        std::string fileContents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        awa = fileContents;
    }

    if (isAwalang.has_value()) {
		filterInput(awa, isAwalang);
    }
    else {
        isAwalang = determineAwaType(awa);
        filterInput(awa, isAwalang);
    }

    if (debugMode) std::cout << awa << std::endl << std::string(100, '-') << std::endl;

    if (!isAwalang.value()) {
        if (!legacyMode) std::cerr << "[Awabler] [0001] Warning: You're currently transpiling Awalang code under AWA5.0++, the code generated may lead to compatibility issues with other interpreters. Use the \"--legacy\" option for legacy Awabling. Find more details in the README." << std::endl;

		awa = Awabler::convertCode(awa);
        
		if (debugMode) std::cout << awa << std::endl << std::string(100, '-') << std::endl;
    }

    AwaInterpreter interpreter;
    std::pair<std::vector<StacktraceEntry>, bool> info = interpreter.run(awa, input, debugMode);

    std::cout << std::endl;

    if (debugMode) writeStacktrace(info.first, info.second);

    return 0;
}