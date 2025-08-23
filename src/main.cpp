#include "argparse.hpp"
#include "AwaInterpreter.hpp"
#include "Awabler.hpp"
#include <unordered_set>

const std::vector<std::string> keywords = {
    "nop", "prn", "pr1", "red", "r3d", "blw", "sbm", "pop",
    "dpl", "srn", "mrg", "4dd", "sub", "mul", "div", "cnt",
    "lbl", "jmp", "eql", "lss", "gr8", "trm"
};

static const std::vector<std::string> argFilter = { "blw", "sbm", "srn", "lbl", "jmp" };

static std::optional<bool> determineAwaType(const std::string& input) {
    for (const auto& element : keywords) {
        if (element.find(input) != std::string::npos) {
            return false;
        }
    }

    return true;
}

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

        awa = lastValidLine;
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

static void writeStacktrace(const std::vector<std::pair<int, std::pair<std::string, std::vector<Bubble>>>>& stacktrace) {
    if (stacktrace.empty()) {
        return;
	}
    std::cout << std::endl;

    std::ofstream ofs;
    ofs.open("stacktrace.txt", std::ofstream::out | std::ofstream::trunc);
    if (ofs.is_open()) {
        for (std::pair<int, std::pair<std::string, std::vector<Bubble>>> outerPairs : stacktrace) {
			int executionTime = outerPairs.first;
            std::string instruction = outerPairs.second.first;
            std::vector<Bubble> stack = outerPairs.second.second;

            std::string line;
            for (int p = 0; p < stack.size(); p++) {
                Bubble bubble = stack[p];

                if (isDouble(bubble)) {
                    BubbleVector list = getList(bubble);
                    for (int i = 0; i < list.size(); i++) {
                        if (i == 0) {
                            line.append(" (");
                        }
                        line.append(((i == 0) ? "" : " ") + std::to_string(getInt(list[i])));
                        if (i == list.size() - 1) {
                            line.append(")");
                        }
                    }
                }
                else {
                    line.append(" " + std::to_string(getInt(bubble)));
                }

                if (p == 0) {
                    int num = 19;
                    num -= static_cast<int>(instruction.size()) - 3;

                    std::string spaces(num, ' ');
                    line.insert(0, spaces);
                }
            }
            line.insert(0, instruction);

            std::ostringstream oss;
            oss << "[" << std::setfill('0') << std::setw(4) << executionTime << "] ";
            line.insert(0, oss.str());

            ofs << line;
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
        if (!legacyMode) std::cerr << "[Awabler] [0001] Warning: You're currently compiling Awalang code under AWA5.0++, the code generated may lead to compatibility issues with other interpreters. Use the \"--legacy\" option for legacy Awabling. Find more details in the README." << std::endl;

		awa = Awabler::convertCode(awa);
        
		if (debugMode) std::cout << awa << std::endl << std::string(100, '-') << std::endl;
    }

    AwaInterpreter interpreter;
    std::vector<std::pair<int, std::pair<std::string, std::vector<Bubble>>>> stacktrace = interpreter.run(awa, input, debugMode);

    std::cout << std::endl;

    if (debugMode) writeStacktrace(stacktrace);

    return 0;
}