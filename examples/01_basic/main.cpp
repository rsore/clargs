#include <CLArgs/parser.hpp>

#include <filesystem>
#include <sstream>
#include <string_view>

struct VerboseOption
{
    static constexpr std::string_view identifier{"--verbose"};
    static constexpr std::string_view alias{"-v"};
    static constexpr std::string_view description{"Enable verbose output"};
    static constexpr bool             required{false};
};

struct FileOption
{
    static constexpr std::string_view identifier{"--file"};
    static constexpr std::string_view value_hint{"FILE"};
    static constexpr std::string_view description{"Specify file to load"};
    static constexpr bool             required{true};
    using ValueType = std::filesystem::path;
};

int
main(int argc, char **argv)
{
    CLArgs::Parser<VerboseOption, FileOption> parser;
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

    const bool has_verbose = parser.has_option<VerboseOption>();
    std::cout << "Has option " << VerboseOption::identifier << ": " << has_verbose << "\n";

    if (const auto file = parser.get_option_value<FileOption>(); file.has_value())
    {
        std::cout << "File: " << file.value() << std::endl;
    }

    return EXIT_SUCCESS;
}
