#include "AwaInterpreter.hpp"

static std::map<int, std::string> AwatismsMap = {
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
	{21, "mov"},
    {31, "trm"}
};

static std::string reverse(int value) {
    auto it = AwatismsMap.find(value);
    if (it != AwatismsMap.end()) {
        return it->second;
    }
    return "undefined";
}

std::pair<std::vector<StacktraceEntry>, bool> AwaInterpreter::run(const std::string& code, const std::string& input, const bool isDebug) {
    bubbleAbyss.clear();
    stacktrace.clear();

    data = ReadAwatalk(code);
    
    if (isDebug) {
        for (int i = 0; i < data.size();) {
			std::cout << data[i] << " ";
            i++;
        };
        std::cout << "\n" << std::string(100, '-') << "\n";
        std::cout << std::endl;
    }

    buildLabelTable();

    std::cout << "Output:" << std::endl;
    executeInstructions(input);

    return std::make_pair(stacktrace, AwaInterpreter::legacy);
}

std::vector<int> AwaInterpreter::ReadAwatalk(const std::string& awa) {
    std::vector<int> instructions;

    size_t awaIndex = 0;
    for (; awaIndex < awa.size() - 6; awaIndex++) {
        if (awa.substr(awaIndex, 6) == "awawa ") {
            AwaInterpreter::legacy = false;
            awaIndex += 5;
            break;
        }

        if (awa.substr(awaIndex, 4) == "awa ") {
            AwaInterpreter::legacy = true;
            awaIndex += 3;
            break;
        }
    }
    if (awaIndex >= awa.size() - (AwaInterpreter::legacy ? 3 : 5)) {
        return instructions;
    }

    int bitCounter = 0;
    int targetBit = 5;
    int newValue = 0;
    std::vector<int> bitsToRead;
	bool param = false;
    bool signed_ = false;       // Rename due to collision with the "signed" keyword
    bool newInstruction = true;

	bool previousValueDependent = false;    // Would only be true if non-legacy
    int previousInstruction = -1;

    while (awaIndex < awa.size() - 1) {
        if (awa.substr(awaIndex, 2) == "wa") {
            if (targetBit == 8 && bitCounter == 0 && signed_) {
                newValue = -1;
            }
            else {
                newValue = (newValue << 1) + 1;
            }
            awaIndex += 2;
            bitCounter++;
        }
        else if (awaIndex < awa.size() - 3 && awa.substr(awaIndex, 4) == " awa") {
            bitCounter++;
            newValue <<= 1;
            awaIndex += 4;
        }
        else {
            awaIndex++;
        }

        if (bitCounter >= targetBit) {
            instructions.push_back(newValue);

            bitCounter = 0;

            // For conditionals
            if (previousValueDependent) {
                previousValueDependent = false;

                switch (previousInstruction) {
                case blw:
                    if (newValue) {
                        signed_ = false;
                        bitsToRead.push_back(4);
                    }
                    else {
                        signed_ = true;
                        bitsToRead.push_back(8);
                    }
                    break;
                case mov:
                    if (newValue) {
                        signed_ = false;
                        bitsToRead.insert(bitsToRead.end(), { 4, 4 });
                    }
                    else {

                        signed_ = true;
                        bitsToRead.insert(bitsToRead.end(), { 4, 8 });
                    }
                    break;
                case sbm:
				case srn:
				case jmp:
                    signed_ = false;
                    if (newValue) {
                        bitsToRead.push_back(4);
                    }
                    else {
                        bitsToRead.push_back(5);
                    }
					break;
                default:
                    break;
                }
            }

            if (newInstruction) {
				previousInstruction = newValue;

                if (!AwaInterpreter::legacy) {
                    switch (newValue) {
                        case blw:
                            signed_ = false;
                            bitsToRead.push_back(1);
					    	previousValueDependent = true;
                            break;
                        case sbm:
                        case srn:
                        case jmp:
                            signed_ = false;
                            bitsToRead.push_back(1);
							previousValueDependent = true;
                            break;
                        case pop:
                            signed_ = false;
                            bitsToRead.push_back(4);
                            break;
                        case mov:
                            signed_ = false;
							bitsToRead.push_back(1);
							previousValueDependent = true;
                            break;
                        default:
                            break;
                    }
                } else {
                    switch (newValue) {
                        case blw:
                            signed_ = true;
                            bitsToRead.push_back(8);
                            break;
                        case sbm:
                        case srn:
                        case jmp:
                        case lbl:
                            signed_ = false;
                            bitsToRead.push_back(5);
                            break;
                        default:
                            break;
                    }
                }
            }

            if (bitsToRead.size() == 0) {
                targetBit = 5;
                signed_ = false;
				newInstruction = true;
            }
            else {
                targetBit = bitsToRead.front();
				bitsToRead.erase(bitsToRead.begin());
				newInstruction = false;
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
        case blw:
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

void AwaInterpreter::executeInstructions(const std::string& input) {
    size_t i = 0;
    bool terminate = false;
    unsigned int executionStep = 0;

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
                }
                else {
                    logWarning("Warning: Print attempted to print an empty stack", executionStep);
                }
                break;
            case pr1:
                if (!bubbleAbyss.empty()) {
                    Bubble bubble = bubbleAbyss.back();
                    bubbleAbyss.pop_back();
                    printBubble(bubble, true);
                }
                else {
                    logWarning("Warning: Print Num attempted to print an empty stack", executionStep);
                }
                break;
            case red: {
                if (input.empty()) {
                    logWarning("Warning: Read has no input to read", executionStep);
                    break;
                }

                if (AwaInterpreter::legacy) {
                    BubbleVector bubbles;
                    for (auto it = input.rbegin(); it != input.rend(); ++it) {
                        char c = *it;
                        size_t idx = AwaSCII.find(c);
                        if (idx != std::string::npos) {
                            bubbles.push_back(Bubble(static_cast<int>(idx)));
                        }
                    }
                    bubbleAbyss.push_back(Bubble(bubbles));
                }
                else
                {
                    BubbleVector bubbles;
                    for (auto it = input.rbegin(); it != input.rend(); ++it) {
                        unsigned char uc = static_cast<unsigned char>(*it);
                        if (uc < 128 || uc > 0) {
                            bubbles.push_back(Bubble(static_cast<int>(uc)));
                        }
                    }
                    bubbleAbyss.push_back(Bubble(bubbles));
                }
                break;
            }
            case r3d: {
                if (input.empty()) {
                    logWarning("Warning: Read Num has no input to read", executionStep);
                    break;
		    	}

                std::istringstream iss(input);
                std::string token;
                int number = 0;
                bool found = false;
                while (iss >> token) {
                    size_t pos = 0;
                    while (pos < token.size() && (token[pos] == '+' || token[pos] == '-')) ++pos;
                    if (pos < token.size() && std::isdigit(token[pos])) {
                        try {
                            number = std::stoi(token);
                            found = true;
                            break;
                        } catch (...) {}
                    }
                }
                bubbleAbyss.push_back(Bubble(found ? number : 0));
                break;
            }
            case blw:
                if (AwaInterpreter::legacy) {
                    if (i + 1 < data.size()) {
                        i++;
                        bubbleAbyss.push_back(Bubble(data[i]));
                    }
                    else {
                        logWarning("Warning: Blow has no valid argument", executionStep);
                    }
                }
                else {
                    if (i + 2 < data.size()) {
                        if (data[++i]) {
                            int registerIndex = data[++i];
                            if (registerIndex >= 0 && static_cast<size_t>(registerIndex) < bubblePond.size())
                                bubbleAbyss.push_back(Bubble(bubblePond[registerIndex]));
                        }
                        else {
                            bubbleAbyss.push_back(Bubble(data[++i]));
                        }
                    }
                    else {
                        logWarning("Warning: Blow has no or insufficient valid argument", executionStep);
                    }
                }
                break;
            case sbm:
                if (i + (AwaInterpreter::legacy ? 1 : 2) < data.size()) {
                    int pos = 0;
                    if (!AwaInterpreter::legacy) {
						if (data[++i]) {
                            int registerIndex = data[++i];
                            if (registerIndex >= 0 && static_cast<size_t>(registerIndex) < bubblePond.size())
                                pos = bubblePond[registerIndex];
                        }
                        else {
                            pos = data[++i];
                        }
                    }
                    else {
						pos = data[++i];
                    }

                    if (!bubbleAbyss.empty()) {
                        Bubble bubble = bubbleAbyss.back();
                        bubbleAbyss.pop_back();
                        if (pos == 0) {
                            bubbleAbyss.insert(bubbleAbyss.begin(), bubble);
                        }
                        else if (pos > 0 && static_cast<size_t>(pos) <= bubbleAbyss.size()) {
                            bubbleAbyss.insert(bubbleAbyss.end() - pos, bubble);
                        }
                    }
                    else {
                        logWarning("Warning: Submerge attempted to submarge on an empty stack", executionStep);
                    }
                }
                else {
                    logWarning("Warning: Submerge has no or insufficient valid argument", executionStep);
                }
                break;
            case pop:
                if (!bubbleAbyss.empty()) {
                    Bubble bubble = bubbleAbyss.back();
                    bool isDouble = ::isDouble(bubble);
                    if (!AwaInterpreter::legacy) 
                    {
                        if (i + 1 < data.size()) {
                            bubblePond[data[++i]] = isDouble ? 0 : getInt(bubble);
                        }
                        else {
                            logWarning("Warning: Pop has no valid argument", executionStep);
						}
                    }

                    bubbleAbyss.pop_back();
                    if (isDouble) {
                        BubbleVector list = getList(bubble);
                        for (auto& b : list) {
                            bubbleAbyss.push_back(b);
                        }
                    }
                }
                else {
                    logWarning("Warning: Pop attempted to pop on an empty stack", executionStep);
                }
                break;
            case dpl:
                if (!bubbleAbyss.empty()) {
                    Bubble original = bubbleAbyss.back();
                    if (isDouble(original)) {
                        BubbleVector copiedList = getList(original);
                        bubbleAbyss.push_back(Bubble(copiedList));
                    }
                    else {
                        bubbleAbyss.push_back(Bubble(getInt(original)));
                    }
                }
                else {
                    logWarning("Warning: Duplicate attempted to duplicate on an empty stack", executionStep);
                }
                break;
            case srn:
                if (i + (AwaInterpreter::legacy ? 1 : 2) < data.size()) {
                    int count = 0;
					if (!AwaInterpreter::legacy) {
                        if (data[++i]) {
                            int registerIndex = data[++i];
                            if (registerIndex >= 0 && static_cast<size_t>(registerIndex) < bubblePond.size())
                                count = bubblePond[registerIndex];
                        }
                        else {
                            count = data[++i];
                        }
                    }
                    else {
						count = data[++i];
                    }

                    if (count > 0) {
                        count = std::min(count, static_cast<int>(bubbleAbyss.size()));

                        for (int idx = 0; idx < count; ++idx) {
                            if (isDouble(bubbleAbyss[bubbleAbyss.size() - 1 - idx])) {
                                totalWarnings++;
                                 std::cerr << "[AwaInterpreter] " << "[" << std::setfill('0') << std::setw(4) << totalWarnings << "] Warning: Surround on step " << executionStep << " attempted to surround a double bubble." << std::endl;

                                break;
                            }
                        }

                        BubbleVector newBubble;
                        while (count-- > 0) {
                            newBubble.insert(newBubble.begin(), bubbleAbyss.back());
                            bubbleAbyss.pop_back();
                        }
                        bubbleAbyss.push_back(Bubble(newBubble));
                    }
                }
                else {
                    logWarning("Warning: Surround has no or insufficient valid argument", executionStep);
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
                        newBubble.push_back(bubble2);
                        newBubble.push_back(bubble1);
                        bubbleAbyss.push_back(Bubble(newBubble));
                    }
                    else if (b1Double && !b2Double) {
                        BubbleVector list = getList(bubble1);
                        list.push_back(bubble2);
                        bubbleAbyss.push_back(Bubble(list));
                    }
                    else if (!b1Double && b2Double) {
                        BubbleVector list = getList(bubble2);
                        list.insert(list.begin(), bubble1);
                        bubbleAbyss.push_back(Bubble(list));
                    }
                    else {
                        BubbleVector list1 = getList(bubble1);
                        BubbleVector list2 = getList(bubble2);
                        list1.insert(list1.begin(), list2.begin(), list2.end());
                        bubbleAbyss.push_back(Bubble(list1));
                    }
                }
                else {
                    logWarning("Warning: Merge attempted to merge on a stack with " + std::to_string(bubbleAbyss.size()) + " bubbles", executionStep);
                }
                break;
            case add:
                if (bubbleAbyss.size() >= 2) {
                    Bubble bubble1 = bubbleAbyss.back();
                    bubbleAbyss.pop_back();
                    Bubble bubble2 = bubbleAbyss.back();
                    bubbleAbyss.pop_back();
                    bubbleAbyss.push_back(addBubbles(bubble1, bubble2));
                }
                else {
                    logWarning("Warning: Add attempted to add on a stack with " + std::to_string(bubbleAbyss.size()) + " bubbles", executionStep);
                }
                break;
            case sub:
                if (bubbleAbyss.size() >= 2) {
                    Bubble bubble1 = bubbleAbyss.back();
                    bubbleAbyss.pop_back();
                    Bubble bubble2 = bubbleAbyss.back();
                    bubbleAbyss.pop_back();
                    bubbleAbyss.push_back(subBubbles(bubble1, bubble2));
                }
                else {
                    logWarning("Warning: Subtract attempted to subtract on a stack with " + std::to_string(bubbleAbyss.size()) + " bubbles", executionStep);
                }
                break;
            case mul:
                if (bubbleAbyss.size() >= 2) {
                    Bubble bubble1 = bubbleAbyss.back();
                    bubbleAbyss.pop_back();
                    Bubble bubble2 = bubbleAbyss.back();
                    bubbleAbyss.pop_back();
                    bubbleAbyss.push_back(mulBubbles(bubble1, bubble2));
                }
                else {
                    logWarning("Warning: Multiply attempted to multiply on a stack with " + std::to_string(bubbleAbyss.size()) + " bubbles", executionStep);
                }
                break;
            case div_:
                if (bubbleAbyss.size() >= 2) {
                    Bubble bubble1 = bubbleAbyss.back();
                    bubbleAbyss.pop_back();
                    Bubble bubble2 = bubbleAbyss.back();
                    bubbleAbyss.pop_back();
                    bubbleAbyss.push_back(divBubbles(bubble1, bubble2));
                }
                else {
                    logWarning("Warning: Division attempted to divide on a stack with " + std::to_string(bubbleAbyss.size()) + " bubbles", executionStep);
                }
                break;
            case cnt:
                if (!bubbleAbyss.empty()) {
                    Bubble bubble = bubbleAbyss.back();
                    if (isDouble(bubble)) {
                        BubbleVector list = getList(bubble);
                        bubbleAbyss.push_back(Bubble(static_cast<int>(list.size())));
                    }
                    else {
                        bubbleAbyss.push_back(Bubble(0));
                    }
                }
                else {
                    bubbleAbyss.push_back(Bubble(0));
                }
                break;
            case lbl:
                if (i + 1 < data.size()) {
                    i++;
                }
                else {
                    logWarning("Warning: Label has no valid argument", executionStep);
                }
                break;
            case jmp:
                if (i + (AwaInterpreter::legacy ? 1 : 2) < data.size()) {
                    int label = 0;
                    if (!AwaInterpreter::legacy) {
                        if (data[++i]) {
                            int registerIndex = data[++i];
                            if (registerIndex >= 0 && static_cast<size_t>(registerIndex) < bubblePond.size())
                                label = bubblePond[registerIndex];
                        }
                        else {
                            label = data[++i];
                        }
                    }
                    else {
                        label = data[++i];
					}

                    if (lblTable.find(label) != lblTable.end()) {
                        i = lblTable[label];
                    }
                    else {
                        logWarning("Warning: Jump attempted to jump to a non-existing label " + std::to_string(label), executionStep);
                    }
                }
                else {
                    logWarning("Warning: Jump has no valid argument", executionStep);
                }
                break;
            case eql:
                if (bubbleAbyss.size() < 2) {
                    logWarning("Warning: Equal attempted to compare on a stack with " + std::to_string(bubbleAbyss.size()) + " bubbles", executionStep);
                }
                else {
                    Bubble b1 = bubbleAbyss.back();
                    Bubble b2 = bubbleAbyss[bubbleAbyss.size() - 2];
                    if (!isDouble(b1) && !isDouble(b2) && getInt(b1) == getInt(b2)) {
                    }
                    else {
                        skipNextInstruction(i);
                    }
                }
                break;
            case lss:
                if (bubbleAbyss.size() < 2) {
                    logWarning("Warning: Less Than attempted to compare on a stack with " + std::to_string(bubbleAbyss.size()) + " bubbles", executionStep);
                }
                else {
                    Bubble b1 = bubbleAbyss.back();
                    Bubble b2 = bubbleAbyss[bubbleAbyss.size() - 2];
                    if (!isDouble(b1) && !isDouble(b2) && getInt(b1) < getInt(b2)) {
                    }
                    else {
                        skipNextInstruction(i);
                    }
                }
                break;
            case gr8:
                if (bubbleAbyss.size() < 2) {
                    logWarning("Warning: Greater Than attempted to compare on a stack with " + std::to_string(bubbleAbyss.size()) + " bubbles", executionStep);
                }
                else {
                    Bubble b1 = bubbleAbyss.back();
                    Bubble b2 = bubbleAbyss[bubbleAbyss.size() - 2];
                    if (!isDouble(b1) && !isDouble(b2) && getInt(b1) > getInt(b2)) {
                    }
                    else {
                        skipNextInstruction(i);
                    }
                }
                break;
            case mov:
                if (AwaInterpreter::legacy) {
                    logWarning("Warning: Move is not supported in legacy mode", executionStep);
                } else {
                    if (i + 3 < data.size()) {
                        bool isSecondParamReg = data[++i];
		    			int target = data[++i];
		    			int input = data[++i];

                        if (isSecondParamReg) {
                            bubblePond[target] = bubblePond[input];
                        }
                        else {
                            bubblePond[target] = input;
                        }
                    }
                    else {
                        logWarning("Warning: Move has no or insufficient valid arguments", executionStep);
                    }
                }
                break;
            case trm:
                terminate = true;
                break;
        }

        std::string argument;
        switch (op) {
        case jmp:
        case sbm:
        case srn:
        case blw:
            if (!AwaInterpreter::legacy) {
                argument = (data[i - 1] ? "r" : "") + std::to_string(data[i]);
            }
            else {
                argument = std::to_string(data[i]);
            }
            break;
        case lbl:
            argument = std::to_string(data[i]);
            break;
        case pop:
            if (!AwaInterpreter::legacy) {
                argument = "r" + std::to_string(data[i]);
			}
            break;
        case mov:
            argument = "r" + std::to_string(data[i - 1]) + ", " + (data[i - 2] ? "r" : "") + std::to_string(data[i]);
			break;
        default:
            break;
        }

        executionStep++;
        i++;

        stacktrace.push_back({executionStep, reverse(op) + " " + argument, bubbleAbyss, bubblePond});
    }
}

void AwaInterpreter::skipNextInstruction(size_t& i) {
    if (i + 1 < data.size()) {
        int nextOp = data[i + 1];
        switch (nextOp) {
        case pop:
        case lbl:
        case jmp:
            i += 2;
            break;

        case blw:
        case sbm:
        case srn:
            i += 3;
            break;

        case mov:
            i += 4;
			break;
        default:
            i++;
            break;
        }
    }
    else {
        i++;
    }
}

Bubble AwaInterpreter::addBubbles(const Bubble& a, const Bubble& b) {
    if (!isDouble(a) && !isDouble(b)) {
        return Bubble(getInt(a) + getInt(b));
    }
    else if (isDouble(a) && !isDouble(b)) {
        BubbleVector list = getList(a);
        for (auto& elem : list) {
            elem = addBubbles(elem, b);
        }
        return Bubble(list);
    }
    else if (!isDouble(a) && isDouble(b)) {
        BubbleVector list = getList(b);
        for (auto& elem : list) {
            elem = addBubbles(a, elem);
        }
        return Bubble(list);
    }
    else {
        BubbleVector listA = getList(a);
        BubbleVector listB = getList(b);
        BubbleVector newList;
        size_t minSize = std::min(listA.size(), listB.size());
        for (size_t i = 0; i < minSize; i++) {
            BubbleVector temp = getList(addBubbles(listA[i], listB[i]));
            newList.insert(newList.end(), temp.begin(), temp.end());
        }
        return Bubble(newList);
    }
}

Bubble AwaInterpreter::subBubbles(const Bubble& a, const Bubble& b) {
    if (!isDouble(a) && !isDouble(b)) {
        return Bubble(getInt(a) - getInt(b));
    }
    else if (isDouble(a) && !isDouble(b)) {
        BubbleVector list = getList(a);
        for (auto& elem : list) {
            elem = subBubbles(elem, b);
        }
        return Bubble(list);
    }
    else if (!isDouble(a) && isDouble(b)) {
        BubbleVector list = getList(b);
        for (auto& elem : list) {
            elem = subBubbles(a, elem);
        }
        return Bubble(list);
    }
    else {
        BubbleVector listA = getList(a);
        BubbleVector listB = getList(b);
        BubbleVector newList;
        size_t minSize = std::min(listA.size(), listB.size());
        for (size_t i = 0; i < minSize; i++) {
            BubbleVector temp = getList(subBubbles(listA[i], listB[i]));
            newList.insert(newList.end(), temp.begin(), temp.end());
        }
        return Bubble(newList);
    }
}

Bubble AwaInterpreter::mulBubbles(const Bubble& a, const Bubble& b) {
    if (!isDouble(a) && !isDouble(b)) {
        return Bubble(getInt(a) * getInt(b));
    }
    else if (isDouble(a) && !isDouble(b)) {
        BubbleVector list = getList(a);
        for (auto& elem : list) {
            elem = mulBubbles(elem, b);
        }
        return Bubble(list);
    }
    else if (!isDouble(a) && isDouble(b)) {
        BubbleVector list = getList(b);
        for (auto& elem : list) {
            elem = mulBubbles(a, elem);
        }
        return Bubble(list);
    }
    else {
        BubbleVector listA = getList(a);
        BubbleVector listB = getList(b);
        BubbleVector newList;
        size_t minSize = std::min(listA.size(), listB.size());
        for (size_t i = 0; i < minSize; i++) {
            BubbleVector temp = getList(mulBubbles(listA[i], listB[i]));
            newList.insert(newList.end(), temp.begin(), temp.end());
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
        }
        else {
            quotient = static_cast<int>(std::floor(tempD));
        }
        int remainder = aVal - quotient * bVal;
        return Bubble(BubbleVector{ Bubble(remainder), Bubble(quotient) });
    }
    else if (isDouble(a) && !isDouble(b)) {
        BubbleVector list = getList(a);
        for (auto& elem : list) {
            elem = divBubbles(elem, b);
        }
        return Bubble(list);
    }
    else if (!isDouble(a) && isDouble(b)) {
        BubbleVector list = getList(b);
        for (auto& elem : list) {
            elem = divBubbles(a, elem);
        }
        return Bubble(list);
    }
    else {
        BubbleVector listA = getList(a);
        BubbleVector listB = getList(b);
        BubbleVector newList;
        size_t minSize = std::min(listA.size(), listB.size());
        for (size_t i = 0; i < minSize; i++) {
            BubbleVector temp = getList(divBubbles(listA[i], listB[i]));
            newList.insert(newList.end(), temp.begin(), temp.end());
        }
        return Bubble(newList);
    }
}

void AwaInterpreter::printBubble(const Bubble& bubble, bool numbersOut) {
    if (!isDouble(bubble)) {
        if (numbersOut) {
            std::cout << getInt(bubble) << " ";
        }
        else {
            int idx = getInt(bubble);
            if (AwaInterpreter::legacy) {
                if (idx >= 0 && static_cast<size_t>(idx) < AwaSCII.size()) {
                    std::cout << AwaSCII[idx];
                }
            }
            else {
                switch (idx) {
                case 0:
                    return;
                case 9:
					std::cout << "\t";
                    return;
                case 10:
					std::cout << "\n";
                    return;
                case 13:
                    std::cout << "\r";
                    return;
                default:
                    if (idx >= 32 && idx <= 126) {
                        std::cout << static_cast<char>(idx);
                    }
                    else {
                        std::cout << "?";
                    }
                }
			}
        }
    }
    else {
        BubbleVector list = getList(bubble);
        for (auto it = list.rbegin(); it != list.rend(); ++it) {
            printBubble(*it, numbersOut);
        }
    }
}

// Add a helper function to log warnings
void AwaInterpreter::logWarning(const std::string& message, unsigned int executionStep) {
    totalWarnings++;
    std::cerr << "[AwaInterpreter] " << "[" << std::setfill('0') << std::setw(4) << totalWarnings << "] " << message << " on step " << executionStep << "." << std::endl;
}
