# AWA5.0 Interpreter
A C++ port of [Temp Tempai's AWA5.0 Interpreter](https://github.com/TempTempai/AWA5.0). \
The port is based on an improved specification, **AWA5.0++**. For more information, please refer to the [AWA5.0++](#AWA50) section.

[Skip to usage](#Usage)

## TODOs
*The to-do list is not stable, as I may always change my mind and decide to delete or add more things.*

<details>
<summary>Basics</summary>

- [x] Support for all instructions(Awatisms) stated in [The AWA5.0 Specification](https://github.com/TempTempai/AWA5.0/blob/main/Documentation/AWA5.0%20Specification.pdf)
    - [x] System
    - [x] Pile manipulation
    - [x] Arithmetic
    - [x] Program flow

- [ ] Input supports
    - [ ] Read from stdin
        - [ ] Awalang support
        - [ ] Awably support
    - [x] Read from command line arguments
        - [x] Directly passing
        - [x] Read from file

- [x] Development tools
    - [x] Awably(assembly-like language for AWA) to Awalang (awawa awa) transpiler

- [ ] Debug tools
    - [x] Stack(Bubble Abyss) trace
    - [ ] Per-line execution
    - [ ] Speed profiler for sections

- [ ] Improvements upon the specification
    - [x] Registers
    - [ ] Pointers for bubbles in the Abyss
    - [x] Full ASCII support
    - [ ] Static linking
    - [x] ~~Error handling~~ Not needed as the language is perfect
</details>

<details>
<summary>Future plans<sub>(aka I don't think I'll work on the following things in the future)</sub></summary>

- [ ] AWA-VM / AWA JIT
- [ ] AWA-OS
- [ ] Self-hosted AWA Interpreter
</details>

## Differences
This port has multiple differences from the original AWA5.0 interpreter.

### AWA5.0
<details>
<summary>Out-of-bounds access handling</summary>

The original interpreter treats missing stack elements as `undefined`. This port checks the stack size and ignores the instruction (with a warning) if insufficient elements exist.  
Example ¨C `eql` instruction:

| Stack  | Bubble A    | Bubble B    | Result |
|--------|-------------|-------------|--------|
| Empty  | `undefined` | `undefined` | True   |
| 1      | 1           | `undefined` | False  |
| 1 2    | 2           | 1           | False  |
| 2 2    | 2           | 2           | True   |

This behavior applies to all instructions, ignoring illegal instructions with a warning, and continue executing.
</details>

<details>
<summary>Error handling</summary>

The specification states the language is perfect and needs no error handling. To avoid undefined behaviour in C++, this port emits warnings instead of errors ¨C ¡°warning handling¡±. awa
</details>

<details>
<summary>Undefined behaviors</summary>

Most undefined cases are treated as `nop`. Specific deviations are: \
\
Instruction-specific undefined behaviors:

| Instruction                | Condition                                               | Original behavior                           | Port behavior                                |
|----------------------------|---------------------------------------------------------|---------------------------------------------|----------------------------------------------|
| Surround(`srn`)            | Surround more than avaliable                            | Fill `undefined` in the blown double bubble | Surround the max present bubble in the stack |
| Count(`cnt`)               | Empty stack                                             | Blow 0                                      | Blow 0                                       |
| Jump(`jmp`)                | Invalid label                                           | Ignored                                     | Ignored + warning                            |
| Merge(`mrg`)*              | Merging two simple bubbles                              | Merge into a double bubble                  | Merge into a double bubble                   |
| Read(`red`)/Read Num(`r3d`)| Empty input                                             | An empty double bubble will be pushed       | Ignored + warning                            |

\*: The reason why Merge(`mrg`) is on the list is that the AWA5.0 Specification states the instruction should act like Add(`4dd`) if two simple bubbles are present. But the original and other 3rd party interpreters treat it as a merge into a double bubble instead, so I decided to maintain this as a feature, instead of fixing it.
</details>

### AWA5.0++
All features below are not stated in the original AWA5.0 Specification, but are added in this port to improve the language, they might not be supported by other interpreters. \
You can use the `--legacy` flag to disable these features. \
Awalang under the new AWA5.0++ specification will have a starting header `awawa`, corresponding to `awa` in the original specification. \
\
Features below are listed as `Change Name(Affected Scope)`.

<details>
<summary>ASCII Support(All, Abyss related)</summary>

The original AWA5.0 Specification uses AwaSCII, an optimized version of ASCII. This port now fully supports the standard ASCII table. \
The Blow(`blw`) instruction can blow any ASCII character, and Read(`red`) can read any ASCII characters from the input. \
Output is limited to printable ASCII characters (9, 10, 13, and 32 to 126). Unprintable characters will show as `?`.

</details>

<details>
<summary>
Registers(<code>mov &lt;Register&gt;, &lt;Value | Register&gt;</code>, <code>pop &lt;Register&gt;</code>, Awatisms that require parameter)
</summary>

In the new AWA4.0++, there are 16 registers, located in the Bubble Pond. \
Bubble Pond contains sentient bubbles capable of responsive locomotion upon calling. \
Bubbles in the pond can be manipulated by their name, `r0` to `r15`. \
All sentient bubbles are initialized as `0` at the start of the program, if a sentient bubble already has value stored, later `mov`/`pop` instructions will overwrite the previous value. \
\
Usage of the sentient bubbles is shown as follows:
- `blw r0`(Blow `r0` to the stack)
- `sbm r0`(Submerge down `r0` positions)
- `srn r0`(Surround `r0` bubbles)
- `jmp r0`(Jump to label `r0`)
- `mov r0, 1`(Write `1` to `r0`) / `mov r0, r1`(Write `r1` to `r0`)
- `pop r0`(Pop the top bubble into `r0`)

---

New Awatism:
- Pile Manipulation ->
    - Move(`mov <Register> <Value | Register>`, `0x15 | 10101`, `wa awawa awawa <1b: IsSecondParamRegister> <4b: Register> <8b: Value | 4b: Register>`): Move a value into a sentient bubble in the pond. \


Modified Awatisms:
- Pile Manipulation ->
    - Pop(`pop <Register>`, `0x07`, `awa awawawawa <4b: Register>`): Now requires an argument(`r0`\~`r15`), the name of the sentient bubble to store the popped value, 0 would be stored if a double bubble is popped.
    - Blow(`blw <Value | Register>`, `0x05`, `awa awawa awawa <1b: IsParamRegister> <8b: Value | 4b: Register>`)
    - Submerge(`sbm <Value | Register>`, `0x06`, `awa awawawa awa <1b: IsParamRegister> <5b: Value | 4b: Register>`)
    - Surround(`srn <Value | Register>`, `0x09`, `awawa awa awawa <1b: IsParamRegister> <5b: Value | 4b: Register>`)


- Program Flow ->
    - Jump(`jmp <Value | Register>`, `0x11`, `wa awa awa awawa <1b: IsParamRegister> <5b: Value | 4b: Register>`)

</details>

## Usage
You can build the interpreter by running the corresponding build scripts in the repository.
<details>
<summary>Windows</summary>

```bash
git clone https://github.com/anfogy/AWA5.0-Interpreter.git
cd AWA5.0-Interpreter
./build.bat
```
This would require the [Visual C++ Build Tool](https://aka.ms/vs/17/release/vs_BuildTools.exe) to be installed. \
`git` is not essential, you can download the repository as an archive file and extract it. \
\
Once built, you can run the interpreter with the following command:
```bash
cd build
./awa.exe
```
A help message should pop up, after that you're good to go!
</details>

<details>
<summary>Linux</summary>

```bash
git clone https://github.com/anfogy/AWA5.0-Interpreter.git
cd AWA5.0-Interpreter
make
```
This would require `g++`, `make`, `binutils-gold` to be installed, if you don't have them, you can install `build-essential` and `binutils-gold` using your package manager. \
`git` is not essential, you can download the repository as an archive file and extract it. \
\
Once built, you can run the interpreter with the following command:
```bash
./awa
```
A help message should pop up, after that you're good to go!
</details>