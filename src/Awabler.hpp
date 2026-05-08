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
#include <functional>
#include <numeric>

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
* @brief Joins a vector of elements into a single string with a specified delimiter.
*        If a conversion function is provided, it is used to convert elements to strings.
* 
* @tparam T The type of the elements in the vector.
* @tparam F The type of the function used to convert elements to strings (optional).
* 
* @param parts The vector of elements to be joined.
* @param delimiter The string to be inserted between each part.
* @param func (Optional) The function that takes an element of type T and returns its string representation.
* 
* @return A single string that is the result of joining the input elements (converted to strings if func is provided) with the delimiter.
* 
* @remark Helper function.
*/
template <typename T, typename F = std::function<std::string(const T&)>>
static std::string join(const std::vector<T>& parts, const std::string& delimiter, F func = [](const T& part) { return part; }) {
    std::ostringstream oss;
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i != 0) {
            oss << delimiter;
        }
        oss << func(parts[i]);
    }
    return oss.str();
}

class Awabler {
public:
    static bool verbose;
    static bool legacy;
    static int totalWarnings;
    static std::string convertCode(std::string& code);

private:
    struct ParamInfo {
        int value;
        int length;
    };

    struct LineResult {
        std::string converted;
        int instructionCode;
        std::vector<ParamInfo> parameters;
    };
    
    static std::string convertAwatalk(int number, int length = 8);
    static int convertAwatism(const std::string& instruction);
    static int convertAwaSCII(std::string& byte);
    static LineResult convertLine(const std::string& line);

    /**
    * @brief Logs a warning message.
    *
    * @param message The warning message to be logged.
    *
    * @remark Helper function.
    */
    static void logWarning(const std::string& message);
};
