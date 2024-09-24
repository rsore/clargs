#include <CLArgs/parser.hpp>
#include "test_utils.hpp"

#include <catch2/catch_test_macros.hpp>

using VerboseFlag = CLArgs::Flag<"--verbose,-v", "Enable verbose output">;
using FileOption  = CLArgs::Option<"--file", "FILE", "Specify file to load", std::filesystem::path>;

TEST_CASE("Parse arguments", "[parse]")
{
    constexpr std::array args = {"program", "-v", "--file", "test.txt"};
    auto [argc, argv]         = CLArgs::Testing::create_argc_argv_from_array(args);

    CLArgs::Parser<VerboseFlag, FileOption> parser;
    REQUIRE_NOTHROW(parser.parse(argc, argv));

    REQUIRE(parser.has_flag<VerboseFlag>());

    const auto file = parser.get_option<FileOption>();
    REQUIRE(file.has_value());
    REQUIRE(file.value() == "test.txt");
}
