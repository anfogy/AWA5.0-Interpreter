#include "argparse.hpp"
#include "AwaInterpreter.hpp"

const std::vector<std::string> keywords = {
    "nop", "prn", "pr1", "red", "r3d", "blw", "sbm", "pop",
    "dpl", "srn", "mrg", "4dd", "sub", "mul", "div", "cnt",
    "lbl", "jmp", "eql", "lss", "gr8", "trm"
};

template<typename T = std::string>
T determineInputType(const T& input) {
    // TODO: Implement actual logic
    return input;
}

static void writeStacktrace(const std::vector<std::pair<std::string, std::vector<Bubble>>>& stacktrace) {
    std::ofstream ofs;
    ofs.open("stacktrace.txt", std::ofstream::out | std::ofstream::trunc);
    if (ofs.is_open()) {
        for (std::pair<std::string, std::vector<Bubble>> pair : stacktrace) {
            std::string instruction = pair.first;
            std::vector<Bubble> stack = pair.second;

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

            ofs << line;
            ofs << std::endl;
        }
    }
    else {
        abort();
    }

    ofs.close();
}

int main(int argc, char* argv[]) {

    auto args = parse_arguments(argc, argv);
    if (!args.valid) return 1;

    std::string awa = args.awa;
    std::string input = args.input;
    bool interactiveMode = args.interactiveMode;
    bool debugMode = args.debugMode;
    std::optional<bool> isAwalang = args.isAwalang;
    std::optional<std::string> filePath = args.filePath;
    std::string executableName = args.executableName;

    if (filePath) {
        std::ifstream file(*filePath, std::ios::in | std::ios::binary);
        if (!file) {
            std::cerr << "Error: Cannot open file: " << *filePath << std::endl;
            return 1;
        }
        std::string fileContents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        awa = fileContents;
    }

    if (isAwalang.has_value()) {
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
        } else {
            std::istringstream iss(awa);
            std::string line, filtered;
            
            while (std::getline(iss, line)) {
                for (const auto& kw : keywords) {
                    if (line.find(kw) != std::string::npos) {
                        filtered += line + "\n";
                        break;
                    }
                }
            }
            awa = filtered;
        }
    } else {
        awa = determineInputType(awa);
    }

    AwaInterpreter interpreter;
    std::vector<std::pair<std::string, std::vector<Bubble>>> stacktrace = interpreter.run(awa);
    std::cout << std::endl;



    return 0;
}