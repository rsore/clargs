#include <CLArgs/common_flags.hpp>
#include <CLArgs/parser.hpp>
#include <CLArgs/parser_builder.hpp>
#include "test_utils.hpp"

#include <catch2/catch_test_macros.hpp>

#include <array>

template <CLArgs::CmdFlag Flag>
using FlagTesterParser = decltype(CLArgs::ParserBuilder{}.add_flag<Flag>().build());

template <CLArgs::CmdFlag Flag>
constexpr void
test_flag_not_passed()
{
    SECTION("Not passed")
    {
        FlagTesterParser<Flag> parser;
        constexpr std::array   args = {"program"};
        auto [argc, argv]           = CLArgs::Testing::create_argc_argv_from_array(args);
        REQUIRE_NOTHROW(parser.parse(argc, argv));
        CHECK_FALSE(parser.template has_flag<Flag>());
    }
}

template <CLArgs::CmdFlag Flag, CLArgs::StringLiteral Identifier, CLArgs::StringLiteral... Identifiers>
constexpr void
test_flag_passed()
{
    SECTION("Pass " + std::string(Identifier.value))
    {
        FlagTesterParser<Flag> parser;
        constexpr std::array   args = {"program", Identifier.value};
        auto [argc, argv]           = CLArgs::Testing::create_argc_argv_from_array(args);
        REQUIRE_NOTHROW(parser.parse(argc, argv));
        CHECK(parser.template has_flag<Flag>());
    }

    if constexpr (sizeof...(Identifiers) > 0)
    {
        test_flag_passed<Flag, Identifiers...>();
    }
}

template <CLArgs::CmdFlag Flag, CLArgs::StringLiteral... Identifiers>
constexpr void
test_flag()
{
    test_flag_not_passed<Flag>();
    test_flag_passed<Flag, Identifiers...>();
}

TEST_CASE("Parser with CommonFlags::Help", "[common_flags]")
{
    test_flag<CLArgs::CommonFlags::Help, "--help", "-h">();
}

TEST_CASE("Parser with CommonFlags::Verbose", "[common_flags]")
{
    test_flag<CLArgs::CommonFlags::Verbose, "--verbose", "-v">();
}

TEST_CASE("Parser with CommonFlags::Quiet", "[common_flags]")
{
    test_flag<CLArgs::CommonFlags::Quiet, "--quiet", "-q">();
}
