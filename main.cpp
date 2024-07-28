#include <iostream>
#include <string_view>

#include "CLArgs/Parser.hpp"

struct HelpFlag
{
    static constexpr std::string_view identifier{ "--help" };
    static constexpr std::string_view alias{ "-h" };
    static constexpr std::string_view description{ "Display help menu" };
    static constexpr bool             required{ false };
};

struct HelloFlag
{
    static constexpr std::string_view identifier{ "--hello" };
    static constexpr std::string_view description{ "Print hello world" };
    static constexpr bool             required{ false };
};

struct VerboseFlag
{
    static constexpr std::string_view identifier{ "--verbose" };
    static constexpr std::string_view alias{ "-v" };
    static constexpr std::string_view description{ "Enable verbose printing" };
    static constexpr bool             required{ false };
};

struct FileOption
{
    static constexpr std::string_view identifier{ "--file" };
    static constexpr std::string_view alias{ "-f" };
    static constexpr std::string_view description{ "Specify input file" };
    static constexpr std::string_view value_hint{ "FILE" };
    static constexpr bool             required{ true };
    using ValueType = std::string;
};

struct DirectoryOption
{
    static constexpr std::string_view identifier{ "--directory" };
    static constexpr std::string_view description{ "Specify input directory" };
    static constexpr std::string_view value_hint{ "PATH" };
    static constexpr bool             required{ false };
    using ValueType = std::string;
};

int
main(const int argc, char **argv)
{
    using ArgumentParser = CLArgs::Parser<HelpFlag, HelloFlag, VerboseFlag, FileOption, DirectoryOption>;
    ArgumentParser argument_parser;

    argument_parser.parse(argc, argv);

    if (argument_parser.has_flag<HelpFlag>())
    {
        std::cout << argument_parser.help() << std::endl;
        return EXIT_SUCCESS;
    }

    std::cout << "Program is '" << argument_parser.program() << "'" << std::endl;

    std::cout << "Has verbose:   " << std::boolalpha << argument_parser.has_flag<VerboseFlag>() << std::endl;
    std::cout << "Has hello:     " << std::boolalpha << argument_parser.has_flag<HelloFlag>() << std::endl;
    std::cout << "Has file:      " << std::boolalpha << argument_parser.has_option<FileOption>() << std::endl;
    std::cout << "Has directory: " << std::boolalpha << argument_parser.has_option<DirectoryOption>() << std::endl;

    if (const auto fileOption = argument_parser.get_option<FileOption>(); fileOption.has_value())
    {
        std::cout << "File: " << fileOption.value() << std::endl;
    }
    else
    {
        std::cout << "File option is not defined" << std::endl;
    }
    if (const auto directoryOption = argument_parser.get_option<DirectoryOption>(); directoryOption.has_value())
    {
        std::cout << "Directory: " << directoryOption.value() << std::endl;
    }
    else
    {
        std::cout << "Directory option is not defined" << std::endl;
    }

    return EXIT_SUCCESS;
}