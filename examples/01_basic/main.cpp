#include <CLArgs/flag.hpp>
#include <CLArgs/option.hpp>
#include <CLArgs/parser.hpp>

#include <filesystem>

using VerboseFlag  = CLArgs::Flag<"--verbose,-v", "Enable verbose output">;
using ConfigOption = CLArgs::Option<"--config,--configuration,-c", "<filepath>", "Specify config file", std::filesystem::path>;

int
main(int argc, char **argv)
{
    int s = 4;
    CLArgs::Parser<VerboseFlag, ConfigOption> parser;
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
    std::cout << "Has verbose option: " << std::boolalpha << has_verbose << "\n";

    if (const auto config = parser.get_option<ConfigOption>(); config.has_value())
    {
        std::cout << "Config file: " << config.value() << std::endl;
    }

    return EXIT_SUCCESS;
}
