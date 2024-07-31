#include <CLArgs/concepts.hpp>
#include <CLArgs/parser.hpp>
#include <CLArgs/validators.hpp>

#include <filesystem>
#include <sstream>
#include <string_view>

struct VerboseOption
{
    static constexpr std::string_view identifier{ "--verbose" };
    static constexpr std::string_view alias{ "-v" };
    static constexpr std::string_view description{ "Enable verbose output" };
    static constexpr bool             required{ false };
};

struct QuietOption
{
    static constexpr std::string_view identifier{ "--quiet" };
    static constexpr std::string_view alias{ "-q" };
    static constexpr std::string_view description{ "Enable quiet output" };
    static constexpr bool             required{ false };
};

struct OutputModifierGroup
{
    static constexpr std::string_view name{"Output modifiers"};
    using Options   = CLArgs::Options<VerboseOption, QuietOption>;
    using Validator = CLArgs::MutualExclusionValidator;
};

int
main(int argc, char **argv)
{
    std::unordered_map<std::type_index, std::any> options;
    options[std::type_index(typeid(QuietOption))]   = std::any{};
    options[std::type_index(typeid(VerboseOption))] = std::any{};

    std::cout << std::boolalpha << OutputModifierGroup::Validator::validate<typename OutputModifierGroup::Options>(options) << std::endl;

    CLArgs::Parser<VerboseOption, QuietOption> parser;
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

    return EXIT_SUCCESS;
}
