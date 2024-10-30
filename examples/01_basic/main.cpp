#include <CLArgs/clargs.hpp>

#include <filesystem>
#include <iostream>

using HelpFlag     = CLArgs::Flag<"--help,-h", "Show help menu">;
using VerboseFlag  = CLArgs::Flag<"--verbose,-v", "Enable verbose output">;
using ConfigOption = CLArgs::Option<"--config,--configuration,-c", "<filepath>", "Specify config file", std::filesystem::path>;

int
main(int argc, char **argv)
{
    auto parser = CLArgs::ParserBuilder{}
                      .add_program_description<"Basic example program to showcase CLArgs library.">()
                      .add_flag<HelpFlag>()
                      .add_flag<VerboseFlag>()
                      .add_option<ConfigOption>()
                      .build();
    try
    {
        parser.parse(argc, argv);
    }
    catch (std::exception &e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        std::cerr << parser.usage() << std::endl;
        return EXIT_FAILURE;
    }

    if (parser.has_flag<HelpFlag>())
    {
        std::cout << parser.help() << std::endl;
        return EXIT_SUCCESS;
    }

    std::cout << "Program: " << parser.program() << std::endl;

    const bool has_verbose = parser.has_flag<VerboseFlag>();
    std::cout << "Has verbose option: " << std::boolalpha << has_verbose << std::endl;

    if (const auto config = parser.get_option<ConfigOption>(); config.has_value())
    {
        std::cout << "Config file: " << config.value() << std::endl;
    }
    else
    {
        std::cout << "No config file provided." << std::endl;
    }

    return EXIT_SUCCESS;
}
