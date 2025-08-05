# AWA5.0 Interpreter
A C++ port of [Temp Tempai's AWA5.0 Interpreter](https://github.com/TempTempai/AWA5.0).

[Skip blabbering](#Usage)

## TODOs
*The to-do list is not stable, as I may always change my mind and decide to delete or add more things.*

- [ ] Support for all instructions(Awatisms) stated in [The AWA5.0 Specification](https://github.com/TempTempai/AWA5.0/blob/main/Documentation/AWA5.0%20Specification.pdf)
    - [ ] System
        - [ ] Read(`red`) and ReadNum(`r3d`) implementations
        - [x] Other system instructions
    - [x] Pile manipulation
    - [x] Arithmetic
    - [x] Program flow

- [ ] Input supports
    - [ ] Read from stdin
        - [ ] Awalang support
        - [ ] Awably support
    - [ ] Read from command line arguments
        - [ ] Read from file
            - [ ] Awalang support
            - [ ] Awably support

- [ ] Debug tools
    - [ ] Stack(Bubble Abyss) visualizer
    - [ ] Per-line execution
    - [ ] Speed profiler for sections

- [ ] Improvements upon the specification
    - [ ] More instructions, for example, reading the stack.
    - [ ] Pointers for bubbles in the Abyss
    - [ ] Full ASCII support
    - [ ] Static linking
    - [ ] ~~Error handling~~ Not needed as the language is perfect

- [ ] Code improvements
    - [ ] Add comments
    - [ ] Cleanups, reconstructions
    - [ ] Performance optimizations

### Some things I already have but need to be ported to C++
- [ ] Development tools
    - [ ] Awably(assembly-like language for AWA) to Awalang (awawa awa) transpiler

### Future plans<sub>(aka I don't think I'll work on the following things in the future)</sub>
- [ ] AWA-VM / AWA JIT
- [ ] AWA-OS
- [ ] Self-hosted AWA Interpreter

## Differences
This port has multiple differences from the original AWA5.0 interpreter.

### Out-of-bounds access handling
Let's compare the code for handling Equal(`eql`) between the original AWA5.0 interpreter and this port.

The original AWA5.0 interpreter:
```js
if (!isDouble(bubbleAbyss[bubbleAbyss.length - 1])
    && !isDouble(bubbleAbyss[bubbleAbyss.length - 2])
    && bubbleAbyss[bubbleAbyss.length - 1] == bubbleAbyss[bubbleAbyss.length - 2]) {
        //True, execute next line
    }
    else {
        // False, Skip the next line
    }
```
\
My implementation:
```cpp
if (bubbleAbyss.size() >= 2) {
    Bubble b1 = bubbleAbyss.back();
    Bubble b2 = bubbleAbyss[bubbleAbyss.size() - 2];
    if (!isDouble(b1) && !isDouble(b2) && getInt(b1) == getInt(b2)) {
        //True, execute next line
    }
    else {
        // False, Skip the next line
    }
}
```
\
With the original implementation, if the stack is empty or has only one element, it will pull an undefine out off the stack and compare it. It has 4 conditions.

| Stack  | Bubble A  | Bubble B  | Result |
|--------|-----------|-----------|--------|
| Empty  | Undefined | Undefined | True   |
| 1      | 1         | Undefined | False  |
| 1 2    | 2         | 1         | False  |
| 2 2    | 2         | 2         | True   |

With my implementation, it will check if the stack has at least 2 elements before comparing them. If less than 2 elements are present, it will simply ignore that particular instruction. \
This behavior applies to all instructions, illegal instructions will be ignored, and the program will continue executing, while throwing a warning.

### Error handling
The AWA5.0 Specification states that the language is perfect and does not need error handling. \
This statement can be easily accomplished for the original interpreter, as it's running on JavaScript. However, this port is written in C++, checks are needed to prevent undefined behaviors. \
To follow the specifications, while wanted to have something to be more clear for debugging, I decided to throw warnings instead of errors to alert the user. It's now called "Warning handling". awa

### Undefined behaviors
While the port mostly ignores undefined behaviors, treating it as a `nop` instruction. There're some redefined behaviors when came to such condition that is unstated in the specification. \
\
General undefined bahaviors:
- If the last instruction has no argument when it's supposed to, it'll be ignored.
- All instructions that require at least X bubbles will be ignored if the stack has less than X bubbles.
- 
\
Instruction-specific undefined behaviors:

| Instruction                | Condition                                               | Original behavior                           | Port behavior                                |
|----------------------------|---------------------------------------------------------|---------------------------------------------|----------------------------------------------|
| Surround(`srn`)            | Trying to surround more bubbles than what the stack has | Fill `undefined` in the blown double bubble | Surround the max present bubble in the stack |
| Count(`cnt`)               | Trying to count on an empty stack                       | Blow 0                                      | Blow 0                                       |
| Jump(`jmp`)                | Jumping to an invalid label                             | Ignored                                     | Ignored with a warning                       |
| Merge(`mrg`)<sup>[1]</sup> | Merging two simple bubbles                              | Merge two into a double bubble              | Merge two into a double bubble               |

[1]: The reason why Merge(`mrg`) is on the list is that the AWA5.0 Specification states the instruction should act like Add(`4dd`) if two simple bubbles are present. But the original and other 3rd party interpreters treat it as a merge into a double bubble instead, so I decided to maintain this as a feature, instead of fixing it.

## Usage
This version of the interpreter is not ready for any uses, however, you can still try to compile it. In this example, I'll use `gcc` as the compiler, you should know the drill anyways.
```bash
git clone https://github.com/anfogy/AWA5.0-Interpreter.git
cd AWA5.0-Interpreter
gcc -std=c++20 -o awa main.cpp
```

### Load AWA programs
You can't load programs directly into the executable, you need to modify the program in the source code. Which is located in `main.cpp:737`, string argument passed in `interpreter.run()`. \
This version on its own included a very simple 4-stack reversal program for testing, it should output `4321` when run, you can find the Awably code of it in `4-elements_stack_reversal_via_loop.awa`.