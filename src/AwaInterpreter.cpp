#include "AwaInterpreter.hpp"
#include <algorithm>
#include <iostream>
#include <cmath>
#include <sstream>

std::map<int, std::string> AwatismsMap = {
    {0, "nop"},
    {1, "prn"},
    {2, "pr1"},
    {3, "red"},
    {4, "r3d"},
    {5, "blw"},
    {6, "sbm"},
    {7, "pop"},
    {8, "dpl"},
    {9, "srn"},
    {10, "mrg"},
    {11, "4dd"},
    {12, "sub"},
    {13, "mul"},
    {14, "div"},
    {15, "cnt"},
    {16, "lbl"},
    {17, "jmp"},
    {18, "eql"},
    {19, "lss"},
    {20, "gr8"},
    {31, "trm"}
};

static std::string reverse(int value) {
    auto it = AwatismsMap.find(value);
    if (it != AwatismsMap.end()) {
        return it->second;
    }
    return "undefined";
}

std::vector<std::pair<std::string, std::vector<Bubble>>> AwaInterpreter::run(const std::string& code) {
    bubbleAbyss.clear();
    stacktrace.clear();
    data = ReadAwatalk(code);
    buildLabelTable();
    executeInstructions();
    return stacktrace;
}

std::vector<int> AwaInterpreter::ReadAwatalk(const std::string& awaBlock) {
    std::vector<int> instructions;
    std::string cleanedAwas;
    for (char c : awaBlock) {
        if (c == 'a' || c == 'w' || c == ' ') {
            cleanedAwas += std::tolower(c);
        }
    }
    size_t awaIndex = 0;
    for (; awaIndex < cleanedAwas.size() - 3; awaIndex++) {
        if (cleanedAwas.substr(awaIndex, 3) == "awa") {
            awaIndex += 3;
            break;
        }
    }
    if (awaIndex >= cleanedAwas.size() - 3) {
        return instructions;
    }
    int bitCounter = 0;
    int targetBit = 5;
    int newValue = 0;
    bool param = false;
    bool signed_ = false;
    while (awaIndex < cleanedAwas.size() - 1) {
        if (cleanedAwas.substr(awaIndex, 2) == "wa") {
            if (bitCounter == 0 && signed_) {
                newValue = -1;
            } else {
                newValue = (newValue << 1) + 1;
            }
            awaIndex += 2;
            bitCounter++;
        } else if (awaIndex < cleanedAwas.size() - 3 && cleanedAwas.substr(awaIndex, 4) == " awa") {
            bitCounter++;
            newValue <<= 1;
            awaIndex += 4;
        } else {
            awaIndex++;
        }
        if (bitCounter >= targetBit) {
            instructions.push_back(newValue);
            bitCounter = 0;
            if (param) {
                targetBit = 5;
                param = false;
                signed_ = false;
            } else {
                switch (newValue) {
                case blo:
                    targetBit = 8;
                    signed_ = true;
                    param = true;
                    break;
                case sbm:
                case srn:
                case lbl:
                case jmp:
                    targetBit = 5;
                    signed_ = false;
                    param = true;
                    break;
                default:
                    break;
                }
            }
            newValue = 0;
        }
    }
    return instructions;
}

void AwaInterpreter::buildLabelTable() {
    lblTable.clear();
    for (size_t i = 0; i < data.size(); i++) {
        switch (data[i]) {
        case lbl:
            lblTable[data[i + 1]] = i + 1;
            i++;
            break;
        case blo:
        case sbm:
        case srn:
        case jmp:
            i++;
            break;
        default:
            break;
        }
    }
}

void AwaInterpreter::executeInstructions() {
    size_t i = 0;
    bool terminate = false;
    unsigned int executionTime = 0;
    while (i < data.size() && !terminate) {
        int op = data[i];
        switch (op) {
        case nop:
            break;
        case prn:
            if (!bubbleAbyss.empty()) {
                Bubble bubble = bubbleAbyss.back();
                bubbleAbyss.pop_back();
                printBubble(bubble, false);
            } else {
                totalWarnings++;
                std::cerr << "[" << totalWarnings << "] " << "Warning: Print on time " << executionTime << " attempted to print an empty stack." << std::endl;
            }
            break;
        case pr1:
            if (!bubbleAbyss.empty()) {
                Bubble bubble = bubbleAbyss.back();
                bubbleAbyss.pop_back();
                printBubble(bubble, true);
            } else {
                totalWarnings++;
                std::cerr << "[" << totalWarnings << "] " << "Warning: Print Num on time " << executionTime << " attempted to print an empty stack." << std::endl;
            }
            break;
        case red: {
            bubbleAbyss.push_back(Bubble(0));
            break;
        }
        case r3d: {
            bubbleAbyss.push_back(Bubble(0));
            break;
        }
        case blo:
            if (i + 1 < data.size()) {
                i++;
                bubbleAbyss.push_back(Bubble(data[i]));
            } else {
                totalWarnings++;
                std::cerr << "[" << totalWarnings << "] " << "Warning: Blow on time " << executionTime << " has no valid argument." << std::endl;
            }
            break;
        case sbm:
            if (i + 1 < data.size()) {
                i++;
                if (!bubbleAbyss.empty()) {
                    Bubble bubble = bubbleAbyss.back();
                    bubbleAbyss.pop_back();
                    int pos = data[i];
                    if (pos == 0) {
                        bubbleAbyss.insert(bubbleAbyss.begin(), bubble);
                    } else if (pos > 0 && static_cast<size_t>(pos) <= bubbleAbyss.size()) {
                        bubbleAbyss.insert(bubbleAbyss.end() - pos, bubble);
                    }
                }
            } else {
                totalWarnings++;
                std::cerr << "[" << totalWarnings << "] " << "Warning: Submerge on time " << executionTime << " has no valid argument." << std::endl;
            }
            break;
        case pop:
            if (!bubbleAbyss.empty()) {
                Bubble bubble = bubbleAbyss.back();
                bubbleAbyss.pop_back();
                if (isDouble(bubble)) {
                    BubbleVector list = getList(bubble);
                    for (auto& b : list) {
                        bubbleAbyss.push_back(b);
                    }
                }
            } else {
                totalWarnings++;
                std::cerr << "[" << totalWarnings << "] " << "Warning: Pop on time " << executionTime << " attempted to pop on an empty stack." << std::endl;
            }
            break;
        case dpl:
            if (!bubbleAbyss.empty()) {
                Bubble original = bubbleAbyss.back();
                if (isDouble(original)) {
                    BubbleVector copiedList = getList(original);
                    bubbleAbyss.push_back(Bubble(copiedList));
                } else {
                    bubbleAbyss.push_back(Bubble(getInt(original)));
                }
            } else {
                totalWarnings++;
                std::cerr << "[" << totalWarnings << "] " << "Warning: Duplicate on time " << executionTime << " attempted to duplicate on an empty stack." << std::endl;
            }
            break;
        case srn:
            if (i + 1 < data.size()) {
                i++;
                int count = data[i];
                if (count > 0) {
                    if (static_cast<size_t>(count) <= bubbleAbyss.size()) {
                        BubbleVector newBubble;
                        while (count-- > 0) {
                            newBubble.insert(newBubble.begin(), bubbleAbyss.back());
                            bubbleAbyss.pop_back();
                        }
                        bubbleAbyss.push_back(Bubble(newBubble));
                    } else {
                        count = static_cast<int>(bubbleAbyss.size());
                        BubbleVector newBubble;
                        while (count-- > 0) {
                            newBubble.insert(newBubble.begin(), bubbleAbyss.back());
                            bubbleAbyss.pop_back();
                        }
                        bubbleAbyss.push_back(Bubble(newBubble));
                    }
                }
            } else {
                totalWarnings++;
                std::cerr << "[" << totalWarnings << "] " << "Warning: Surround on time " << executionTime << " has no valid argument." << std::endl;
            }
            break;
        case mrg:
            if (bubbleAbyss.size() >= 2) {
                Bubble bubble1 = bubbleAbyss.back();
                bubbleAbyss.pop_back();
                Bubble bubble2 = bubbleAbyss.back();
                bubbleAbyss.pop_back();
                bool b1Double = isDouble(bubble1);
                bool b2Double = isDouble(bubble2);
                if (!b1Double && !b2Double) {
                    BubbleVector newBubble;
                    newBubble.push_back(bubble1);
                    newBubble.push_back(bubble2);
                    bubbleAbyss.push_back(Bubble(newBubble));
                } else if (b1Double && !b2Double) {
                    BubbleVector list = getList(bubble1);
                    list.push_back(bubble2);
                    bubbleAbyss.push_back(Bubble(list));
                } else if (!b1Double && b2Double) {
                    BubbleVector list = getList(bubble2);
                    list.insert(list.begin(), bubble1);
                    bubbleAbyss.push_back(Bubble(list));
                } else {
                    BubbleVector list1 = getList(bubble1);
                    BubbleVector list2 = getList(bubble2);
                    list1.insert(list1.end(), list2.begin(), list2.end());
                    bubbleAbyss.push_back(Bubble(list1));
                }
            } else {
                totalWarnings++;
                std::cerr << "[" << totalWarnings << "] " << "Warning: Merge on time " << executionTime << " attempted to merge on a stack with " + std::to_string(bubbleAbyss.size()) + " bubbles." << std::endl;
            }
            break;
        case add:
            if (bubbleAbyss.size() >= 2) {
                Bubble bubble1 = bubbleAbyss.back();
                bubbleAbyss.pop_back();
                Bubble bubble2 = bubbleAbyss.back();
                bubbleAbyss.pop_back();
                bubbleAbyss.push_back(addBubbles(bubble1, bubble2));
            } else {
                totalWarnings++;
                std::cerr << "[" << totalWarnings << "] " << "Warning: Add on time " << executionTime << " attempted to add on a stack with " + std::to_string(bubbleAbyss.size()) + " bubbles." << std::endl;
            }
            break;
        case sub:
            if (bubbleAbyss.size() >= 2) {
                Bubble bubble1 = bubbleAbyss.back();
                bubbleAbyss.pop_back();
                Bubble bubble2 = bubbleAbyss.back();
                bubbleAbyss.pop_back();
                bubbleAbyss.push_back(subBubbles(bubble1, bubble2));
            } else {
                totalWarnings++;
                std::cerr << "[" << totalWarnings << "] " << "Warning: Subtract on time " << executionTime << " attempted to subtract on a stack with " + std::to_string(bubbleAbyss.size()) + " bubbles." << std::endl;
            }
            break;
        case mul:
            if (bubbleAbyss.size() >= 2) {
                Bubble bubble1 = bubbleAbyss.back();
                bubbleAbyss.pop_back();
                Bubble bubble2 = bubbleAbyss.back();
                bubbleAbyss.pop_back();
                bubbleAbyss.push_back(mulBubbles(bubble1, bubble2));
            } else {
                totalWarnings++;
                std::cerr << "[" << totalWarnings << "] " << "Warning: Multiply on time " << executionTime << " attempted to multiply on a stack with " + std::to_string(bubbleAbyss.size()) + " bubbles." << std::endl;
            }
            break;
        case div_:
            if (bubbleAbyss.size() >= 2) {
                Bubble bubble1 = bubbleAbyss.back();
                bubbleAbyss.pop_back();
                Bubble bubble2 = bubbleAbyss.back();
                bubbleAbyss.pop_back();
                bubbleAbyss.push_back(divBubbles(bubble1, bubble2));
            } else {
                totalWarnings++;
                std::cerr << "[" << totalWarnings << "] " << "Warning: Division on time " << executionTime << " attempted to divide on a stack with " + std::to_string(bubbleAbyss.size()) + " bubbles." << std::endl;
            }
            break;
        case cnt:
            if (!bubbleAbyss.empty()) {
                Bubble bubble = bubbleAbyss.back();
                if (isDouble(bubble)) {
                    BubbleVector list = getList(bubble);
                    bubbleAbyss.push_back(Bubble(static_cast<int>(list.size())));
                } else {
                    bubbleAbyss.push_back(Bubble(0));
                }
            } else {
                bubbleAbyss.push_back(Bubble(0));
            }
            break;
        case lbl:
            if (i + 1 < data.size()) {
                i++;
            } else {
                totalWarnings++;
                std::cerr << "[" << totalWarnings << "] " << "Warning: Label on time " << executionTime << " has no valid argument." << std::endl;
            }
            break;
        case jmp:
            if (i + 1 < data.size()) {
                i++;
                int label = data[i];
                if (lblTable.find(label) != lblTable.end()) {
                    i = lblTable[label];
                } else {
                    totalWarnings++;
                    std::cerr << "[" << totalWarnings << "] " << "Warning: Jump on time " << executionTime << " attempted to jump to a non-existing label " << label << "." << std::endl;
                }
            } else {
                totalWarnings++;
                std::cerr << "[" << totalWarnings << "] " << "Warning: Jump on time " << executionTime << " has no valid argument." << std::endl;
            }
            break;
        case eql:
            if (bubbleAbyss.size() >= 2) {
                Bubble b1 = bubbleAbyss.back();
                Bubble b2 = bubbleAbyss[bubbleAbyss.size() - 2];
                if (!isDouble(b1) && !isDouble(b2) && getInt(b1) == getInt(b2)) {
                } else {
                    skipNextInstruction(i);
                }
            } else {
                totalWarnings++;
                std::cerr << "[" << totalWarnings << "] " << "Warning: Equal on time " << executionTime << " attempted to compare on a stack with " + std::to_string(bubbleAbyss.size()) + " bubbles." << std::endl;
            }
            break;
        case lss:
            if (bubbleAbyss.size() >= 2) {
                Bubble b1 = bubbleAbyss.back();
                Bubble b2 = bubbleAbyss[bubbleAbyss.size() - 2];
                if (!isDouble(b1) && !isDouble(b2) && getInt(b1) < getInt(b2)) {
                } else {
                    skipNextInstruction(i);
                }
            } else {
                totalWarnings++;
                std::cerr << "[" << totalWarnings << "] " << "Warning: Less Than on time " << executionTime << " attempted to compare on a stack with " + std::to_string(bubbleAbyss.size()) + " bubbles." << std::endl;
            }
            break;
        case gr8:
            if (bubbleAbyss.size() >= 2) {
                Bubble b1 = bubbleAbyss.back();
                Bubble b2 = bubbleAbyss[bubbleAbyss.size() - 2];
                if (!isDouble(b1) && !isDouble(b2) && getInt(b1) > getInt(b2)) {
                } else {
                    skipNextInstruction(i);
                }
            } else {
                totalWarnings++;
                std::cerr << "[" << totalWarnings << "] " << "Warning: Greater Than on time " << executionTime << " attempted to compare on a stack with " + std::to_string(bubbleAbyss.size()) + " bubbles." << std::endl;
            }
            break;
        case trm:
            terminate = true;
            break;
        }
        std::string argument;
        switch (op) {
        case blo:
        case sbm:
        case srn:
        case lbl:
        case jmp:
            argument = std::to_string(data[i]);
            break;
        default:
            break;
        }
        executionTime++;
        i++;

        stacktrace.push_back(std::make_pair(reverse(op) + " " + argument, bubbleAbyss));
    }
}

void AwaInterpreter::skipNextInstruction(size_t& i) {
    if (i + 1 < data.size()) {
        int nextOp = data[i + 1];
        switch (nextOp) {
        case blo:
        case sbm:
        case srn:
        case lbl:
        case jmp:
            i += 2;
            break;
        default:
            i++;
            break;
        }
    } else {
        i++;
    }
}

Bubble AwaInterpreter::addBubbles(const Bubble& a, const Bubble& b) {
    if (!isDouble(a) && !isDouble(b)) {
        return Bubble(getInt(a) + getInt(b));
    } else if (isDouble(a) && !isDouble(b)) {
        BubbleVector list = getList(a);
        for (auto& elem : list) {
            elem = addBubbles(elem, b);
        }
        return Bubble(list);
    } else if (!isDouble(a) && isDouble(b)) {
        BubbleVector list = getList(b);
        for (auto& elem : list) {
            elem = addBubbles(a, elem);
        }
        return Bubble(list);
    } else {
        BubbleVector listA = getList(a);
        BubbleVector listB = getList(b);
        BubbleVector newList;
        size_t minSize = std::min(listA.size(), listB.size());
        for (size_t i = 0; i < minSize; i++) {
            newList.push_back(addBubbles(listA[i], listB[i]));
        }
        return Bubble(newList);
    }
}

Bubble AwaInterpreter::subBubbles(const Bubble& a, const Bubble& b) {
    if (!isDouble(a) && !isDouble(b)) {
        return Bubble(getInt(a) - getInt(b));
    } else if (isDouble(a) && !isDouble(b)) {
        BubbleVector list = getList(a);
        for (auto& elem : list) {
            elem = subBubbles(elem, b);
        }
        return Bubble(list);
    } else if (!isDouble(a) && isDouble(b)) {
        BubbleVector list = getList(b);
        for (auto& elem : list) {
            elem = subBubbles(a, elem);
        }
        return Bubble(list);
    } else {
        BubbleVector listA = getList(a);
        BubbleVector listB = getList(b);
        BubbleVector newList;
        size_t minSize = std::min(listA.size(), listB.size());
        for (size_t i = 0; i < minSize; i++) {
            newList.push_back(subBubbles(listA[i], listB[i]));
        }
        return Bubble(newList);
    }
}

Bubble AwaInterpreter::mulBubbles(const Bubble& a, const Bubble& b) {
    if (!isDouble(a) && !isDouble(b)) {
        return Bubble(getInt(a) * getInt(b));
    } else if (isDouble(a) && !isDouble(b)) {
        BubbleVector list = getList(a);
        for (auto& elem : list) {
            elem = mulBubbles(elem, b);
        }
        return Bubble(list);
    } else if (!isDouble(a) && isDouble(b)) {
        BubbleVector list = getList(b);
        for (auto& elem : list) {
            elem = mulBubbles(a, elem);
        }
        return Bubble(list);
    } else {
        BubbleVector listA = getList(a);
        BubbleVector listB = getList(b);
        BubbleVector newList;
        size_t minSize = std::min(listA.size(), listB.size());
        for (size_t i = 0; i < minSize; i++) {
            newList.push_back(mulBubbles(listA[i], listB[i]));
        }
        return Bubble(newList);
    }
}

Bubble AwaInterpreter::divBubbles(const Bubble& a, const Bubble& b) {
    if (!isDouble(a) && !isDouble(b)) {
        int aVal = getInt(a);
        int bVal = getInt(b);
        if (bVal == 0) {
            return Bubble(BubbleVector{ Bubble(0), Bubble(0) });
        }
        double tempD = static_cast<double>(aVal) / bVal;
        int quotient;
        if (tempD < 0) {
            quotient = static_cast<int>(std::ceil(tempD));
        } else {
            quotient = static_cast<int>(std::floor(tempD));
        }
        int remainder = aVal - quotient * bVal;
        return Bubble(BubbleVector{ Bubble(remainder), Bubble(quotient) });
    } else if (isDouble(a) && !isDouble(b)) {
        BubbleVector list = getList(a);
        for (auto& elem : list) {
            elem = divBubbles(elem, b);
        }
        return Bubble(list);
    } else if (!isDouble(a) && isDouble(b)) {
        BubbleVector list = getList(b);
        for (auto& elem : list) {
            elem = divBubbles(a, elem);
        }
        return Bubble(list);
    } else {
        BubbleVector listA = getList(a);
        BubbleVector listB = getList(b);
        BubbleVector newList;
        size_t minSize = std::min(listA.size(), listB.size());
        for (size_t i = 0; i < minSize; i++) {
            newList.push_back(divBubbles(listA[i], listB[i]));
        }
        return Bubble(newList);
    }
}

void AwaInterpreter::printBubble(const Bubble& bubble, bool numbersOut) {
    if (!isDouble(bubble)) {
        if (numbersOut) {
            std::cout << getInt(bubble) << " ";
        } else {
            int idx = getInt(bubble);
            if (idx >= 0 && static_cast<size_t>(idx) < AwaSCII.size()) {
                std::cout << AwaSCII[idx];
            }
        }
    } else {
        BubbleVector list = getList(bubble);
        for (const Bubble& b : list) {
            printBubble(b, numbersOut);
        }
    }
}
