#include <catch2/catch_test_macros.hpp>

#include <CLArgs/parser.hpp>

#include <array>

struct VerboseOption
{
    static constexpr std::string_view identifier{ "--verbose" };
    static constexpr std::string_view alias{ "-v" };
    static constexpr std::string_view description{ "Enable verbose output" };
    static constexpr bool             required{ false };
};

struct FileOption
{
    static constexpr std::string_view identifier{ "--file" };
    static constexpr std::string_view value_hint{ "FILE" };
    static constexpr std::string_view description{ "Specify file to load" };
    static constexpr bool             required{ true };
    using ValueType = std::filesystem::path;
};

TEST_CASE("Parse arguments", "[parse]")
{
    std::array args = {"program", "-v", "--file", "test.txt"};

    int argc = static_cast<int>(args.size());
    char** argv = const_cast<char **>(args.data());

    CLArgs::Parser<VerboseOption, FileOption> parser;
    REQUIRE_NOTHROW(parser.parse(argc, argv));
}