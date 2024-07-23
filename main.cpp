#include <iostream>
#include <string_view>

#include "ArgumentParser.hpp"

struct HelloFlag
{
    static constexpr std::string_view flag{ "--hello" };
    static constexpr std::string_view description{ "Print hello world" };
    static constexpr bool             required{ false };
};

struct VerboseFlag
{
    static constexpr std::string_view flag{ "--verbose" };
    static constexpr std::string_view description{ "Enable verbose printing" };
    static constexpr bool             required{ false };
};

struct FileOption
{
    static constexpr std::string_view option{ "--file" };
    static constexpr std::string_view description{ "Enable verbose printing" };
    static constexpr std::string_view value_hint{ "FILE" };
    static constexpr bool             required{ false };
    using ValueType = std::string;
};

struct DirectoryOption
{
    static constexpr std::string_view option{ "--directory" };
    static constexpr std::string_view description{ "Specify input directory" };
    static constexpr std::string_view value_hint{ "PATH" };
    static constexpr bool             required{ false };
    using ValueType = std::string;
};

int
main(const int argc, char **argv)
{
    ArgumentParser<HelloFlag, VerboseFlag, FileOption> argument_parser(argc, argv);

    std::cout << "Program is '" << argument_parser.program() << "'" << std::endl;
    argument_parser.debug_print();

    std::cout << "Has verbose:   " << std::boolalpha << argument_parser.has_flag<VerboseFlag>() << std::endl;
    std::cout << "Has hello:     " << std::boolalpha << argument_parser.has_flag<HelloFlag>() << std::endl;
    std::cout << "Has file:      " << std::boolalpha << argument_parser.has_option<FileOption>() << std::endl;
    std::cout << "Has directory: " << std::boolalpha << argument_parser.has_option<DirectoryOption>() << std::endl;

    std::cout << "File: " << argument_parser.get_option<FileOption>() << std::endl;

    return 0;
}