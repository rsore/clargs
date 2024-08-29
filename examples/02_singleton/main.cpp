#include <CLArgs/flag.hpp>
#include <CLArgs/option.hpp>
#include <CLArgs/singleton.hpp>

#include <filesystem>

using VerboseFlag = CLArgs::Flag<"--verbose,-v", "Enable verbose output">;
using ConfigOption =
        CLArgs::Option<"--configuration,--config,-c", "FILEPATH", "Specify path to configuration file", std::filesystem::path>;

using Parser = CLArgs::ParserSingleton<VerboseFlag, ConfigOption>;

int
main(int argc, char **argv)
{
    try
    {
        Parser::parse(argc, argv);
    }
    catch (std::exception &e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        std::cerr << Parser::help() << std::endl;
        return EXIT_FAILURE;
    }

    const bool has_verbose = Parser::has_flag<VerboseFlag>();
    std::cout << "Has verbose option: " << std::boolalpha << has_verbose << "\n";

    if (const auto file = Parser::get_option<ConfigOption>(); file.has_value())
    {
        std::cout << "Config file: " << file.value() << std::endl;
    }

    return EXIT_SUCCESS;
}
