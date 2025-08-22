#pragma once
#include <string>
#include <optional>
#include <iostream>
#include <fstream>
#include <vector>

struct ParsedArguments {
    std::string awa;
    std::string input;
    bool interactiveMode = false;
    bool debugMode = false;
    std::optional<bool> isAwalang = std::nullopt;
    std::optional<std::string> filePath = std::nullopt;
    std::string executableName;
    bool valid = true;
	bool legacyMode = false;
};

inline void print_usage(const std::string& executableName) {
    std::cerr << "Usage: " << executableName << " [Options] --interactive" << std::endl;
    std::cerr << "       " << executableName << " [Options] <Awalang | Awably code>" << std::endl;
    std::cerr << "       " << executableName << " [Options] --file <Path>" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Options: " << std::endl;
    std::cerr << "       " << " --interactive            Enter interactive mode(not implemented)" << std::endl;
    std::cerr << "       " << " -I,  --input             Use the next argument as input for all reads" << std::endl;
    std::cerr << "       " << " -Al, --awalang           Enforce interpreter to treat inputs as Awalang" << std::endl;
    std::cerr << "       " << " -Ab, --awably            Enforce interpreter to treat inputs as Awably" << std::endl;
    std::cerr << "       " << " -L,  --legacy            Enforce Awabler to generate legacy Awalang" << std::endl;
    std::cerr << "       " << " -D,  --debug             Generate extra information on the program" << std::endl;
    std::cerr << "       " << " -H,  --help              Display this message" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Examples: " << std::endl;
    std::cerr << "       " << executableName << " --awalang --file ./examples/4-elements_stack_reversal_via_loop.awa" << std::endl;
    std::cerr << "    The above command will execute the .awa file as Awalang." << std::endl;
    std::cerr << std::endl;
    std::cerr << "       " << executableName << " --awably --file ./examples/hello_world.awa -D" << std::endl;
    std::cerr << "    The above command will execute the .awa file as Awably and output debug informations." << std::endl;
    std::cerr << std::endl;
    std::cerr << "       " << executableName << " --awably \"red; prn;\" --input \"Hello, world.\" -L" << std::endl;
    std::cerr << "    The above command will execute \"red; prn;\" as Awably in legacy mode, with the input \"Hello, world.\"." << std::endl;
}

inline ParsedArguments parse_arguments(int argc, char* argv[]) {
    ParsedArguments args;
    std::string path = argv[0];
    size_t pos = path.find_last_of("/\\");
    args.executableName = (pos == std::string::npos) ? path : path.substr(pos + 1);
    args.executableName = (args.executableName.find(' ') != std::string::npos) ? "\"" + args.executableName + "\"" : args.executableName;

    if (argc == 1) {
        print_usage(args.executableName);
        args.valid = false;
        return args;
    }

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            print_usage(args.executableName);
            args.valid = false;

            return args;
		}
        else if (arg == "--interactive") {
            args.interactiveMode = true;
        }
        else if (arg == "-L" || arg == "--legacy") {
            args.legacyMode = true;
		}
        else if (arg == "-I" || arg == "--input") {
            if (i + 1 < argc) {
                args.input = argv[++i];
            }
            else {
                std::cerr << "[ArgumentParser] Error: --input requires an argument." << std::endl;
                print_usage(args.executableName);
                args.valid = false;

                return args;
            }
        }
        else if (arg == "-Al" || arg == "--awalang") {
            args.isAwalang = true;
        }
        else if (arg == "-Ab" || arg == "--awably") {
            args.isAwalang = false;
        }
        else if (arg == "-D" || arg == "--debug") {
            args.debugMode = true;
        }
        else if (arg == "--file") {
            if (i + 1 < argc) {
                args.filePath = argv[++i];
            }
            else {
                std::cerr << "[ArgumentParser] Error: --file requires a path argument." << std::endl;
                print_usage(args.executableName);
                args.valid = false;

                return args;
            }
        }
        else if (arg.starts_with("-")) {
            std::cerr << "[ArgumentParser] Error: Unknown option: " << arg << std::endl;
            print_usage(args.executableName);
            args.valid = false;

            return args;
        }
        else {
            if (args.awa.empty()) {
                args.awa = arg;
            }
        }
    }
    return args;
}
