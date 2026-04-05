#pragma once
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <cstdlib>
#include <optional>

/**
* @brief Replaces all occurrences of a substring in a string with another substring.
* 
* @param str The string to be modified.
* @param from The substring to be replaced.
* @param to The substring to replace with.
* 
* @remark Helper function.
*/
static void replace(std::string& str, const std::string& from, const std::string& to) {
    if (from.empty()) return;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

/**
* @brief Trims leading and trailing whitespace from a string.
* 
* @param s The string to be modified.
* 
* @remark Helper function.
*/
static void strip(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
        }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
        }).base(), s.end());
}

/**
* @brief Joins a vector of strings into a single string with a specified delimiter.
* 
* @param parts The vector of strings to be joined.
* @param delimiter The string to be inserted between each part.
* 
* @return A single string that is the result of joining the input strings with the delimiter.
* 
* @remark Helper function.
*/
static std::string join(const std::vector<std::string>& parts, const std::string& delimiter) {
    std::ostringstream oss;
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i != 0) {
            oss << delimiter;
        }
        oss << parts[i];
    }
    return oss.str();
}

class Awabler {
public:
    static bool verbose;
    static bool legacy;
    static std::string convertCode(std::string& code);

private:
    struct LineResult {
        std::string converted;
        int instructionCode;
        std::optional<int> parameter;
    };
    
    static std::string convertAwatalk(int number, int length = 8);
    static int convertAwatism(const std::string& instruction);
    static int convertAwaSCII(std::string& byte);
    static LineResult convertLine(const std::string& line);
};
