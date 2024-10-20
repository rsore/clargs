#include <CLArgs/parser.hpp>
#include "test_utils.hpp"

#include <catch2/catch_test_macros.hpp>

using VerboseFlag   = CLArgs::Flag<"--verbose,-v", "Enable verbose output">;
using FlagList = CLArgs::CmdFlagList<VerboseFlag>;

using ConfigOption = CLArgs::Option<"--config,--configuration,-c", "<filepath>", "Specify config file", std::filesystem::path>;
using OptionList = CLArgs::CmdOptionList<ConfigOption>;

TEST_CASE("Parse arguments", "[parse]")
{
    constexpr std::array args = {"program", "-v", "--config", "test.txt"};
    auto [argc, argv]         = CLArgs::Testing::create_argc_argv_from_array(args);

    CLArgs::Parser<FlagList, OptionList> parser;
    REQUIRE_NOTHROW(parser.parse(argc, argv));

    REQUIRE(parser.has_flag<VerboseFlag>());

    const auto file = parser.get_option<ConfigOption>();
    REQUIRE(file.has_value());
    REQUIRE(file.value() == "test.txt");
}
