#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <cmath>
#include <variant>
#include <string>
#include <iomanip>
#include <optional>
#include <sstream>
#include <array>

struct Bubble;
using BubbleVector = std::vector<Bubble>;
struct Bubble {
    std::variant<int, BubbleVector> value;
    Bubble(int i) : value(i) {}
    Bubble(const BubbleVector& v) : value(v) {}
};

/**
* @brief Checks if a Bubble object is a DoubleBubble (i.e. contains a BubbleVector) or a SimpleBubble (i.e. contains an int).
* 
* @param bubble The Bubble object to be checked.
* 
* @return true if the Bubble is a DoubleBubble, false if it is a SimpleBubble.
*/
static bool isDouble(const Bubble& bubble) {
    return std::holds_alternative<BubbleVector>(bubble.value);
}

/**
* @brief Retrieves the integer value from a Bubble object if it is a SimpleBubble.
* 
* @param bubble The Bubble object from which to retrieve the integer value.
* 
* @return The integer value contained in the Bubble if it is a SimpleBubble.
* @throws std::bad_variant_access if the Bubble does not contain an integer value (i.e. is a DoubleBubble).
*/
static int getInt(const Bubble& bubble) {
    if (std::holds_alternative<int>(bubble.value)) {
        return std::get<int>(bubble.value);
    }
    throw std::bad_variant_access();
}

/**
* @brief Retrieves the BubbleVector from a Bubble object if it is a DoubleBubble.
* 
* @param bubble The Bubble object from which to retrieve the BubbleVector.
* 
* @return The BubbleVector contained in the Bubble if it is a DoubleBubble.
* @throws std::bad_variant_access if the Bubble does not contain a BubbleVector (i.e. is a SimpleBubble).
*/
static BubbleVector getList(const Bubble& bubble) {
    if (std::holds_alternative<BubbleVector>(bubble.value)) {
        return std::get<BubbleVector>(bubble.value);
    }
    throw std::bad_variant_access();
}

enum Awatisms {
    nop = 0,
    prn = 1,
    pr1 = 2,
    red = 3,
    r3d = 4,
    blw = 5,
    sbm = 6,
    pop = 7,
    dpl = 8,
    srn = 9,
    mrg = 10,
    add = 11,
    sub = 12,
    mul = 13,
    div_ = 14,
    cnt = 15,
    lbl = 16,
    jmp = 17,
    eql = 18,
    lss = 19,
    gr8 = 20,
    mov = 21,
    trm = 31
};

struct StacktraceEntry {
    unsigned int executionTime;
    std::string instruction;
    std::vector<Bubble> stack;
    std::array<int, 16> registers;
};

class AwaInterpreter {
public:
    /**
	* @brief Executes Awalang code and produces a stacktrace of the execution.
    * 
	* @param code The Awalang code to be executed, represented as a string.
	* @param input The input string to be used for instructions that require input (e.g. "red").
	* @param isDebug Boolean flag indicating whether to generate debug information during execution.
    * 
	* @return A pair with a vector of stacktrace entries, and a boolean for whether the code is legacy or not.
    */
    std::pair<std::vector<StacktraceEntry>, bool> run(const std::string& code, const std::string& input, const bool isDebug);
private:
    bool legacy = false;
    
    /**
	* @brief Converts Awalang code into a vector of integers representing instructions and their parameters.
    * 
	* @param awa The Awalang code to be converted.
    * 
	* @return A vector of integers, representing the sequence of instructions and parameters in bytecodes.
    */
    std::vector<int> ReadAwatalk(const std::string& awaBlock);

    /**
	* @brief Executes the instructions stored in the data vector, where the switch-case structure is located to handle each instruction's behavior.
    * 
	* @param input The input string to be used for instructions that require input (e.g. "red").
    */
    void executeInstructions(const std::string& input);
    void skipNextInstruction(size_t& i);
    void buildLabelTable();

    Bubble addBubbles(const Bubble& a, const Bubble& b);
    Bubble subBubbles(const Bubble& a, const Bubble& b);
    Bubble mulBubbles(const Bubble& a, const Bubble& b);
    Bubble divBubbles(const Bubble& a, const Bubble& b);
    void printBubble(const Bubble& bubble, bool numbersOut);

    /**
	* @brief Logs a warning message.
    * 
	* @param message The warning message to be logged.
	* @param executionStep The current execution step or instruction pointer.
    */
    void logWarning(const std::string& message, unsigned int executionStep);

    std::array<int, 16> bubblePond{};
    std::vector<Bubble> bubbleAbyss;
    std::map<int, size_t> lblTable;
    std::vector<int> data;
    unsigned int totalWarnings = 0;
    std::vector<StacktraceEntry> stacktrace;
    const std::string AwaSCII = "AWawJELYHOSIUMjelyhosiumPCNTpcntBDFGRbdfgr0123456789 .,!'()~_/;\n";
};
