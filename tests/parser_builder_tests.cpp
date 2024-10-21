#include <CLArgs/parser_builder.hpp>
#include "test_utils.hpp"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <string>

using VerboseFlag   = CLArgs::Flag<"--verbose,-v", "Enable verbose output">;
using QuietFlag     = CLArgs::Flag<"--quiet,-q", "Enable quiet output">;
using RecursiveFlag = CLArgs::Flag<"--recursive,-r", "Enable recursion">;
using ConfigOption  = CLArgs::Option<"--config,--configuration,-c", "<filepath>", "Specify config file", std::filesystem::path>;
using NameOption    = CLArgs::Option<"--username,--user", "<username>", "Specify username", std::string>;

TEST_CASE("Parser builder", "[parser_builder]")
{
    constexpr std::array args = {"program", "-v", "--config", "test.txt", "--recursive"};
    auto [argc, argv]         = CLArgs::Testing::create_argc_argv_from_array(args);

    auto parser = CLArgs::ParserBuilder{}
                      .add_flag<VerboseFlag>()
                      .add_flag<QuietFlag>()
                      .add_flag<RecursiveFlag>()
                      .add_option<ConfigOption>()
                      .add_option<NameOption>()
                      .build();

    REQUIRE_FALSE(parser.has_flag<VerboseFlag>());
    REQUIRE_FALSE(parser.has_flag<QuietFlag>());
    REQUIRE_FALSE(parser.has_flag<RecursiveFlag>());

    REQUIRE_FALSE(parser.get_option<ConfigOption>().has_value());
    REQUIRE_FALSE(parser.get_option<NameOption>().has_value());

    REQUIRE_NOTHROW(parser.parse(argc, argv));

    REQUIRE(parser.has_flag<VerboseFlag>());
    REQUIRE_FALSE(parser.has_flag<QuietFlag>());
    REQUIRE(parser.has_flag<RecursiveFlag>());

    REQUIRE(parser.get_option<ConfigOption>().has_value());
    REQUIRE(parser.get_option<ConfigOption>().value() == "test.txt");
    REQUIRE_FALSE(parser.get_option<NameOption>().has_value());
}
