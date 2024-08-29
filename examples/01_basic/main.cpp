#include <CLArgs/flag.hpp>
#include <CLArgs/option.hpp>
#include <CLArgs/parser.hpp>

#include <filesystem>

using VerboseFlag   = CLArgs::Flag<"--verbose,-v", "Enable verbose output", false>;
using FileOption    = CLArgs::Option<"--file", "FILE", "Specify file to load", true, std::filesystem::path>;
using RecursiveFlag = CLArgs::Flag<"--recursive,-r", "Enable recursive travel", false>;
using ConfigOption  = CLArgs::Option<"--config,--configuration,-c", "<filepath>", "Specify config file", true, std::filesystem::path>;

int
main(int argc, char **argv)
{
    CLArgs::Parser<VerboseFlag, FileOption, RecursiveFlag, ConfigOption> parser;
    try
    {
        parser.parse(argc, argv);
    }
    catch (std::exception &e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        std::cerr << parser.help() << std::endl;
        return EXIT_FAILURE;
    }

    const bool has_verbose = parser.has_flag<VerboseFlag>();
    std::cout << "Has option " << VerboseFlag::identifiers[0] << ": " << std::boolalpha << has_verbose << "\n";

    if (const auto file = parser.get_option<FileOption>(); file.has_value())
    {
        std::cout << "File: " << file.value() << std::endl;
    }

    if (const auto config = parser.get_option<ConfigOption>(); config.has_value())
    {
        std::cout << "Config file: " << config.value() << std::endl;
    }

    return EXIT_SUCCESS;
}
