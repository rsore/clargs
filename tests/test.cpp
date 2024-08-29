#include <catch2/catch_test_macros.hpp>

#include <CLArgs/flag.hpp>
#include <CLArgs/option.hpp>
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

using VerboseFlag = CLArgs::Flag<"--verbose,-v", "Enable verbose output", false>;
using FileOption = CLArgs::Option<"--file", "FILE", "Specify file to load", true, std::filesystem::path>;

TEST_CASE("Parse arguments", "[parse]")
{
    constexpr std::array args = {"program", "-v", "--file", "test.txt"};
    auto [argc, argv]         = create_argc_argv(args);

    CLArgs::Parser<VerboseFlag, FileOption> parser;
    REQUIRE_NOTHROW(parser.parse(argc, argv));

    REQUIRE(parser.has_flag<VerboseFlag>());

    const auto file = parser.get_option<FileOption>();
    REQUIRE(file.has_value());
    REQUIRE(file.value() == "test.txt");
}
