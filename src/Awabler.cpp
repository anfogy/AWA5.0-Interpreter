#include "Awabler.hpp"

bool Awabler::verbose = false;
bool Awabler::legacy = false;
int Awabler::totalWarnings = 0;

std::string Awabler::convertAwatalk(int number, int length) {
    number = number & ((1 << length) - 1);
    std::string binStr;
    for (int i = length - 1; i >= 0; i--) {
        if (number & (1 << i)) binStr += '1';
        else binStr += '0';
    }

    replace(binStr, "01", "awawa ");
    replace(binStr, "11", "wawa ");
    replace(binStr, "0", "awa ");
    replace(binStr, "1", "wa ");
    replace(binStr, "wa wa", "wawa");
    strip(binStr);

    return binStr;
}

int Awabler::convertAwatism(const std::string& instruction) {
    static const std::vector<std::string> lookup = {
        "nop", "prn", "pr1", "red", "r3d", "blw", "sbm", "pop", "dpl", "srn", "mrg",
        "add", "sub", "mul", "div", "cnt", "lbl", "jmp", "eql", "lss", "gr8", // AWA5.0
        
        "mov", // AWA5.0++

        "trm" // AWA5.0
    };

    auto it = std::find(lookup.begin(), lookup.end(), instruction);
    if (it == lookup.end()) {
        logWarning("Instruction \"" + instruction + "\" undefined.");
        return 0;
    }

    int index = static_cast<int>(std::distance(lookup.begin(), it));
    if (index != 22 && index >= 21 && Awabler::legacy) {
        logWarning("Tried to transpile AWA5.0++instruction \"" + instruction + "\" under legacy mode.");
        return 0;
    }

    return (index == 22) ? 31 : index;
}

int Awabler::convertAwaSCII(std::string& byte) {
    if (Awabler::legacy) {
        static const std::vector<std::string> lookup = {
            "A", "W", "a", "w", "J", "E", "L", "Y", "H", "O",
            "S", "I", "U", "M", "j", "e", "l", "y", "h", "o",
            "s", "i", "u", "m", "P", "C", "N", "T", "p", "c",
            "n", "t", "B", "D", "F", "G", "R", "b", "d", "f",
            "g", "r", "0", "1", "2", "3", "4", "5", "6", "7",
            "8", "9", "space", ".", ",", "!", "'", "(", ")", "~",
            "_", "/", ";", "\\n"
        };

        auto it = std::find(lookup.begin(), lookup.end(), byte);
        if (it == lookup.end()) {
            logWarning("Token \"" + byte + "\" is not found in the AwaSCII table.");
            return -1;
        }

        return static_cast<int>(std::distance(lookup.begin(), it));
    }
    else {
        if (byte == "space") {
            return 32;
        }
        else if (byte == "\\t") {
            return 9;
        }
        else if (byte == "\\n") {
            return 10; 
        }
        else if (byte == "\\r") {
            return 13;
        }
        else if (byte.length() != 1) {
            logWarning("Token \"" + byte + "\" is not a single character, and is not recognized as a special token (space or \\n).");
            return -1;
        }

        unsigned char c = static_cast<unsigned char>(byte[0]);
        if (c > 127 || c < 0) {
            logWarning("Token \"" + byte + "\" is outside the valid ASCII range (0-127).");
            return -1;
        }

        return static_cast<int>(c);
    }
}

Awabler::LineResult Awabler::convertLine(const std::string& line) {
    static const std::vector<std::string> s8 = {"blw"};
    static const std::vector<std::string> u5 = {"sbm", "srn", "lbl", "jmp"};

    static const std::vector<std::string> multipleParams = { "mov" };

    std::string trimmed = line;
    strip(trimmed);
    if (trimmed.empty()) {
        return { "", -1, {} };
	}

    size_t pos = trimmed.find(' ');
    if (pos == std::string::npos) {
        if (std::find(s8.begin(), s8.end(), trimmed) != s8.end() ||
            std::find(u5.begin(), u5.end(), trimmed) != u5.end() ||
            std::find(multipleParams.begin(), multipleParams.end(), trimmed) != multipleParams.end()) {
            logWarning("Instruction \"" + trimmed + "\" requires a parameter.");
            return { "", -1, {} };
        }

        int instrCode = convertAwatism(trimmed);

        return { convertAwatalk(instrCode, 5), instrCode, {} };
    }

    std::string instruction = trimmed.substr(0, pos);
    std::string paramStr = trimmed.substr(pos);
    strip(paramStr);

    if (std::find(s8.begin(), s8.end(), instruction) == s8.end() &&
        std::find(u5.begin(), u5.end(), instruction) == u5.end() &&
        std::find(multipleParams.begin(), multipleParams.end(), instruction) == multipleParams.end()) {
        logWarning("Instruction \"" + instruction + "\" does not require the argument \"" + paramStr + "\".");
        return { "", -1, {} };
    }

    std::vector<ParamInfo> parameters{};
    size_t commaPos = paramStr.find(',');
    if (commaPos != std::string::npos) {
        auto it = std::find(multipleParams.begin(), multipleParams.end(), instruction);
        if (it == multipleParams.end()) {
             logWarning("Instruction \"" + instruction + "\" has multiple parameters, which is not required.");
             return { "", -1, {} };
        }

        int index = static_cast<int>(std::distance(multipleParams.begin(), it));
        if (index == 0) {   // mov
            std::string firstParamStr = paramStr.substr(0, commaPos);
            std::string secondParamStr = paramStr.substr(commaPos + 1);
            strip(firstParamStr);
            strip(secondParamStr);

            if (firstParamStr[0] == 'r') {
                try {
                    parameters.push_back({std::stoi(firstParamStr.substr(1)), 4});
                }
                catch (...) {
                    logWarning("First parameter \"" + firstParamStr + "\" of instruction \"" + instruction + "\" is invalid.");
                    return { "", -1, {} };
                }
            }
            else {
                logWarning("First parameter \"" + firstParamStr + "\" of instruction \"" + instruction + "\" must start with 'r'.");
                return { "", -1, {} };
            }

			if (secondParamStr[0] == 'r') { // mov rX, rY
                try {
					parameters.push_back({1, 1});
                    parameters.push_back({std::stoi(secondParamStr.substr(1)), 4});
                }
                catch (...) {
                    logWarning("Second parameter \"" + secondParamStr + "\" of instruction \"" + instruction + "\" is invalid.");
                    return { "", -1, {} };
                }
            }
			else {  // mov rX, Y
                try {
					parameters.push_back({0, 1});
                    parameters.push_back({std::stoi(secondParamStr), 8});
                }
                catch (...) {
                    logWarning("Second parameter \"" + secondParamStr + "\" of instruction \"" + instruction + "\" is invalid.");
                    return { "", -1, {} };
                }
            }
        }
    }
    else if (paramStr.substr(0, 2) == "S(" && paramStr.back() == ')') {
        std::string inner = paramStr.substr(2, paramStr.size() - 3);
        parameters.push_back({convertAwaSCII(inner), 8});
    }
    else {
        try {
            parameters.push_back(Awabler::ParamInfo{ std::stoi(paramStr), (std::find(u5.begin(), u5.end(), instruction) != u5.end()) ? 5 : 8 });
        }
        catch (...) {
            logWarning("Parameter \"" + paramStr + "\" of instruction \"" + instruction + "\" is invalid.");
            return { "", -1, {} };
        }
    }

    int instrCode = convertAwatism(instruction);
    std::string convInstr = convertAwatalk(instrCode, 5);
    std::string convParam = std::accumulate(
        parameters.begin() + 1, parameters.end(),
        convertAwatalk(parameters[0].value, parameters[0].length),
        [&parameters](const std::string& acc, const ParamInfo& param) {
            return acc + " " + convertAwatalk(param.value, param.length);
        }
    );

    return Awabler::LineResult{ convInstr + " " + convParam, instrCode, parameters };
}

std::string Awabler::convertCode(std::string& code) {
	if (!Awabler::legacy) totalWarnings++;
    replace(code, ";", "\n");

    std::istringstream iss(code);
    std::vector<std::string> originalLines;
    std::string line;
    while (std::getline(iss, line)) {
        strip(line);

        if (!line.empty()) originalLines.push_back(line);
    }

    std::vector<LineResult> results;
    for (const auto& ln : originalLines) {
        results.push_back(convertLine(ln));
    }

    std::vector<std::string> parts = {Awabler::legacy ? "awa" : "awawa"};
    for (const auto& res : results) {
        if (!res.converted.empty())
            parts.push_back(res.converted);
    }
    std::string convertedCode = join(parts, " ");

    if (verbose) {
        for (size_t i = 0; i < originalLines.size(); i++) {
            std::string param = results[i].parameters.empty()
                ? "None"
                : join(results[i].parameters, ", ", [](Awabler::ParamInfo param) { return std::to_string(param.value); });
            std::cout << std::left << std::setw(20) << originalLines[i]
                << std::setw(50) << results[i].converted
                << std::setw(5) << results[i].instructionCode
                << param << std::endl;
        }

        std::cout << std::string(100, '-') << std::endl;
    }

    return convertedCode;
}

void Awabler::logWarning(const std::string& message) {
    totalWarnings++;
    std::cerr << "[Awabler] [" << std::setfill('0') << std::setw(4) << totalWarnings << "] Warning: " << message << std::endl;
}