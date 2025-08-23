#include "Awabler.hpp"

bool Awabler::verbose = false;
bool Awabler::legacy = false;
static int totalWarnings = 0;

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
        "4dd", "sub", "mul", "div", "cnt", "lbl", "jmp", "eql", "lss", "gr8", "trm"
    };

    auto it = std::find(lookup.begin(), lookup.end(), instruction);
    if (it == lookup.end()) {
        totalWarnings++;
        std::cerr << "[Awabler] " << "[" << totalWarnings << "] Warning: Instruction \"" << instruction << "\" undefined." << std::endl;
        return -1;
    }

    int index = static_cast<int>(std::distance(lookup.begin(), it));

    return (index == 21) ? 31 : index;
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
            totalWarnings++;
            std::cerr << "[Awabler] " << "[" << totalWarnings << "] Warning: Token \"" << byte << "\" is not found in the AwaSCII table." << std::endl;
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
            totalWarnings++;
            std::cerr << "[Awabler] [" << totalWarnings << "] Warning: Token \"" << byte << "\" is not a single character and is not recognized as a special token (space or \\n)." << std::endl;
            return -1;
        }

        unsigned char c = static_cast<unsigned char>(byte[0]);
        if (c > 127 || c < 0) {
            totalWarnings++;
            std::cerr << "[Awabler] [" << totalWarnings << "] Warning: Character \"" << byte << "\" is outside the valid ASCII range (0-127)." << std::endl;
            return -1;
        }

        return static_cast<int>(c);
    }
}

Awabler::LineResult Awabler::convertLine(const std::string& line) {
    static const std::vector<std::string> s8 = {"blw"};
    static const std::vector<std::string> u5 = {"sbm", "srn", "lbl", "jmp"};

    std::string trimmed = line;
    strip(trimmed);
    if (trimmed.empty()) {
        return {"", -1, std::nullopt};
	}

    size_t pos = trimmed.find(' ');
    if (pos == std::string::npos) {
        if (std::find(s8.begin(), s8.end(), trimmed) != s8.end() ||
            std::find(u5.begin(), u5.end(), trimmed) != u5.end()) {
            totalWarnings++;
            std::cerr << "[Awabler] " << "[" <<  totalWarnings << "] Warning: Instruction \"" << trimmed << "\" requires a parameter." << std::endl;
            return {"", -1, std::nullopt};
        }

        int instrCode = convertAwatism(trimmed);

        return {convertAwatalk(instrCode, 5), instrCode, std::nullopt};
    }

    std::string instruction = trimmed.substr(0, pos);
    std::string paramStr = trimmed.substr(pos + 1);
    strip(paramStr);

    if (std::find(s8.begin(), s8.end(), instruction) == s8.end() &&
        std::find(u5.begin(), u5.end(), instruction) == u5.end()) {
        totalWarnings++;
        std::cerr << "[Awabler] " << "[" <<  totalWarnings << "] Warning: Instruction \"" << instruction << "\" does not require the argument \"" << paramStr << "\"." << std::endl;
        return {"", -1, std::nullopt};
    }

    int parameter;
    if (paramStr.size() >= 3 && paramStr.substr(0, 2) == "S(" && paramStr.back() == ')') {
        std::string inner = paramStr.substr(2, paramStr.size() - 3);
        parameter = convertAwaSCII(inner);
    }
    else {
        try {
            parameter = std::stoi(paramStr);
        }
        catch (...) {
            totalWarnings++;
            std::cerr << "[Awabler] " << "[" <<  totalWarnings << "] Warning: Invalid parameter: \"" << paramStr << "\"." << std::endl;
            return {"", -1, std::nullopt};
        }
    }

    int instrCode = convertAwatism(instruction);
    int paramLength = (std::find(u5.begin(), u5.end(), instruction) != u5.end()) ? 5 : 8;
    std::string convInstr = convertAwatalk(instrCode, 5);
    std::string convParam = convertAwatalk(parameter, paramLength);

    return {convInstr + " " + convParam, instrCode, parameter};
}

std::string Awabler::convertCode(std::string& code) {
	if (Awabler::legacy) totalWarnings++;
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
            std::string param = results[i].parameter.has_value()
                ? std::to_string(results[i].parameter.value())
                : "None";
            std::cout << std::left << std::setw(20) << originalLines[i]
                << std::setw(50) << results[i].converted
                << std::setw(5) << results[i].instructionCode
                << " " << param << std::endl;
        }

        std::cout << std::string(100, '-') << std::endl;
    }

    return convertedCode;
}