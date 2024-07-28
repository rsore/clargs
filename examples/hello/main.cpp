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

struct FloatOption
{
    static constexpr std::string_view identifier{ "--float" };
    static constexpr std::string_view description{ "Specify floating point value" };
    static constexpr std::string_view value_hint{ "NUMBER" };
    static constexpr bool             required{ true };
    using ValueType = float;
};

using ArgumentParser = CLArgs::Parser<HelpFlag, HelloFlag, VerboseFlag, FileOption, DirectoryOption, FloatOption>;

int
main(const int argc, char **argv)
{
    ArgumentParser argument_parser;

    try
    {
        argument_parser.parse(argc, argv);
    }
    catch (std::exception &e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        std::cerr << argument_parser.help() << std::endl;
        return EXIT_FAILURE;
    }

    if (argument_parser.has_option<HelpFlag>())
    {
        std::cout << argument_parser.help() << std::endl;
        return EXIT_SUCCESS;
    }

    std::cout << "Program is '" << argument_parser.program() << "'" << std::endl;

    std::cout << "Has verbose:   " << std::boolalpha << argument_parser.has_option<VerboseFlag>() << std::endl;
    std::cout << "Has hello:     " << std::boolalpha << argument_parser.has_option<HelloFlag>() << std::endl;
    std::cout << "Has file:      " << std::boolalpha << argument_parser.has_option<FileOption>() << std::endl;
    std::cout << "Has directory: " << std::boolalpha << argument_parser.has_option<DirectoryOption>() << std::endl;
    std::cout << "Has float:     " << std::boolalpha << argument_parser.has_option<FloatOption>() << std::endl;

    if (const auto file_option = argument_parser.get_option_value<FileOption>(); file_option.has_value())
    {
        std::cout << "File: " << file_option.value() << std::endl;
    }
    else
    {
        std::cout << "File option is not defined" << std::endl;
    }
    if (const auto directory_option = argument_parser.get_option_value<DirectoryOption>(); directory_option.has_value())
    {
        std::cout << "Directory: " << directory_option.value() << std::endl;
    }
    else
    {
        std::cout << "Directory option is not defined" << std::endl;
    }
    if (const auto float_option = argument_parser.get_option_value<FloatOption>(); float_option.has_value())
    {
        std::cout << "Float: " << float_option.value() << std::endl;
    }
    else
    {
        std::cout << "Float option is not defined" << std::endl;
    }

    return EXIT_SUCCESS;
}