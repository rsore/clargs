#include <catch2/catch_test_macros.hpp>

#include <CLArgs/parser.hpp>

#include <array>

template <std::size_t N>
std::pair<int, char **>
create_argc_argv(const std::array<const char *, N> &args)
{
    const int argc = static_cast<int>(N);
    char    **argv = const_cast<char **>(args.data());
    return std::make_pair(argc, argv);
}

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

TEST_CASE("Parse arguments", "[parse]")
{
    constexpr std::array args = {"program", "-v", "--file", "test.txt"};
    auto [argc, argv]         = create_argc_argv(args);

    CLArgs::Parser<VerboseFlag, FileOption> parser;
    REQUIRE_NOTHROW(parser.parse(argc, argv));
}
