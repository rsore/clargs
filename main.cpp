#include <iostream>
#include <string_view>
#include <tuple>

#include "ArgumentParser.hpp"

struct HelloFlag
{
    static constexpr std::string_view identifier{ "--hello" };
    static constexpr std::string_view description{ "Print hello world" };
    static constexpr bool             required{ false };
};

struct VerboseFlag
{
    static constexpr std::string_view identifier{ "--verbose" };
    static constexpr std::string_view alias{"-v"};
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
    ArgumentParser<HelloFlag, VerboseFlag, FileOption, DirectoryOption> argument_parser(argc, argv);

    std::cout << "Program is '" << argument_parser.program() << "'" << std::endl;

    std::cout << "Has verbose:   " << std::boolalpha << argument_parser.has_flag<VerboseFlag>() << std::endl;
    std::cout << "Has hello:     " << std::boolalpha << argument_parser.has_flag<HelloFlag>() << std::endl;
    std::cout << "Has file:      " << std::boolalpha << argument_parser.has_option<FileOption>() << std::endl;
    std::cout << "Has directory: " << std::boolalpha << argument_parser.has_option<DirectoryOption>() << std::endl;

    const auto fileOption = argument_parser.get_option<FileOption>();
    if (fileOption.has_value())
    {
        std::cout << "File: " << fileOption.value() << std::endl;
    }
    else
    {
        std::cout << "File option is not defined" << std::endl;
    }
    const auto directoryOption = argument_parser.get_option<DirectoryOption>();
    if (directoryOption.has_value())
    {
        std::cout << "Directory: " << directoryOption.value() << std::endl;
    }
    else
    {
        std::cout << "Directory option is not defined" << std::endl;
    }

    std::cout << argument_parser.usage() << std::endl;

    return 0;
}