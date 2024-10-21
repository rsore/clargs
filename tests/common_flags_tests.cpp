#include <CLArgs/common_flags.hpp>
#include <CLArgs/parser.hpp>
#include <CLArgs/parser_builder.hpp>
#include "test_utils.hpp"

#include <catch2/catch_test_macros.hpp>

#include <array>

TEST_CASE("Parser with CommonFlags::Verbose", "[flag_parsing]")
{
    CLArgs::Parser parser = CLArgs::ParserBuilder{}.add_flag<CLArgs::CommonFlags::Verbose>().build();

    SECTION("Not passed")
    {
        constexpr std::array args = {"program"};
        auto [argc, argv]         = CLArgs::Testing::create_argc_argv_from_array(args);
        REQUIRE_NOTHROW(parser.parse(argc, argv));
        CHECK_FALSE(parser.has_flag<CLArgs::CommonFlags::Verbose>());
    }

    SECTION("Pass -v")
    {
        constexpr std::array args = {"program", "-v"};
        auto [argc, argv]         = CLArgs::Testing::create_argc_argv_from_array(args);
        REQUIRE_NOTHROW(parser.parse(argc, argv));
        CHECK(parser.has_flag<CLArgs::CommonFlags::Verbose>());
    }

    SECTION("Pass --verbose")
    {
        constexpr std::array args = {"program", "--verbose"};
        auto [argc, argv]         = CLArgs::Testing::create_argc_argv_from_array(args);
        REQUIRE_NOTHROW(parser.parse(argc, argv));
        CHECK(parser.has_flag<CLArgs::CommonFlags::Verbose>());
    }
}
