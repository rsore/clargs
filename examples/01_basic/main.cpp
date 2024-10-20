#include <CLArgs/parser.hpp>

#include <filesystem>

using VerboseFlag   = CLArgs::Flag<"--verbose,-v", "Enable verbose output">;
using QuietFlag     = CLArgs::Flag<"--quiet,-q", "Enable quiet output">;
using RecursiveFlag = CLArgs::Flag<"--recursive,-r", "Enable recursion">;

using FlagList = CLArgs::CmdFlagList<VerboseFlag, QuietFlag, RecursiveFlag>;

using ConfigOption = CLArgs::Option<"--config,--configuration,-c", "<filepath>", "Specify config file", std::filesystem::path>;
using NameOption   = CLArgs::Option<"--name,-n", "username", "Specify username", std::string>;

using OptionList = CLArgs::CmdOptionList<ConfigOption, NameOption>;

int
main(int argc, char **argv)
{
    CLArgs::Parser<FlagList, OptionList> parser;
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

    std::cout << "Program: " << parser.program() << std::endl;

    const bool has_verbose = parser.has_flag<VerboseFlag>();
    std::cout << "Has verbose option: " << std::boolalpha << has_verbose << "\n";

    const bool has_recursive = parser.has_flag<RecursiveFlag>();
    std::cout << "Has recursive option: " << std::boolalpha << has_recursive << "\n";

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
