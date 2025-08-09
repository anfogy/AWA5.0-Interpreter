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

struct Bubble;
using BubbleVector = std::vector<Bubble>;
struct Bubble {
    std::variant<int, BubbleVector> value;
    Bubble(int i) : value(i) {}
    Bubble(const BubbleVector& v) : value(v) {}
};

static bool isDouble(const Bubble& bubble) {
    return std::holds_alternative<BubbleVector>(bubble.value);
}

static int getInt(const Bubble& bubble) {
    if (std::holds_alternative<int>(bubble.value)) {
        return std::get<int>(bubble.value);
    }
    throw std::bad_variant_access();
}

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
    blo = 5,
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
    trm = 31
};

class AwaInterpreter {
public:
    std::vector<std::pair<int, std::pair<std::string, std::vector<Bubble>>>> run(const std::string& code, const std::string& input);
private:
    std::vector<int> ReadAwatalk(const std::string& awaBlock);
    void buildLabelTable();
    void executeInstructions(const std::string& input);
    void skipNextInstruction(size_t& i);
    Bubble addBubbles(const Bubble& a, const Bubble& b);
    Bubble subBubbles(const Bubble& a, const Bubble& b);
    Bubble mulBubbles(const Bubble& a, const Bubble& b);
    Bubble divBubbles(const Bubble& a, const Bubble& b);
    void printBubble(const Bubble& bubble, bool numbersOut);
    std::vector<Bubble> bubbleAbyss;
    std::map<int, size_t> lblTable;
    std::vector<int> data;
    unsigned int totalWarnings = 0;
    std::vector<std::pair<int, std::pair<std::string, std::vector<Bubble>>>> stacktrace;
    const std::string AwaSCII = "AWawJELYHOSIUMjelyhosiumPCNTpcntBDFGRbdfgr0123456789 .,!'()~_/;\n";
};
