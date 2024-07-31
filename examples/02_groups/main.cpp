#include <CLArgs/concepts.hpp>
#include <CLArgs/parser.hpp>

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

struct MutualExclusionValidator
{
    static constexpr std::string_view description{"Mutual exclusivity: At most one option may be passed"};

    template <typename TupleLike>
    static bool
    validate(std::unordered_map<std::type_index, std::any> &options)
    {
        std::uint32_t count{};
        count_present_options<TupleLike>(options, count);
        return count <= 1;
    }

    template <typename TupleLike, std::uint32_t Index = 0>
    static void
    count_present_options(std::unordered_map<std::type_index, std::any> &options, std::uint32_t &count)
    {
        using Option = std::tuple_element_t<Index, TupleLike>;
        static_assert(CLArgs::CmdOption<Option>);

        const auto key = std::type_index(typeid(Option));
        if (options.contains(key))
        {
            count += 1;
        }

        if constexpr (Index < std::tuple_size_v<TupleLike> - 1)
        {
            count_present_options<TupleLike, Index + 1>(options, count);
        }
    }
};

struct VerboseQuietGroup
{
    static constexpr std::string_view name{"Output modifiers"};
    using Options   = CLArgs::Options<VerboseOption, QuietOption>;
    using Validator = MutualExclusionValidator;
};
static_assert(CLArgs::CmdGroup<VerboseQuietGroup>);
static_assert(CLArgs::CmdGroupValidator<MutualExclusionValidator, typename VerboseQuietGroup::Options>);

int
main(int argc, char **argv)
{
    std::unordered_map<std::type_index, std::any> options;
    options[std::type_index(typeid(QuietOption))]   = std::any{};
    options[std::type_index(typeid(VerboseOption))] = std::any{};

    std::cout << std::boolalpha << VerboseQuietGroup::Validator::validate<typename VerboseQuietGroup::Options>(options) << std::endl;

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
