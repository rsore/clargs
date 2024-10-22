#include <CLArgs/common_options.hpp>
#include <CLArgs/parser_builder.hpp>
#include "test_utils.hpp"

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <string_view>

template <CLArgs::CmdOption Option>
using OptionTesterParser = decltype(CLArgs::ParserBuilder{}.add_option<Option>().build());

template <CLArgs::CmdOption Option>
constexpr void
test_option_not_passed()
{
    SECTION("Flag not passed to parser")
    {
        OptionTesterParser<Option> parser;
        constexpr std::array       args = {"program"};
        auto [argc, argv]               = CLArgs::Testing::create_argc_argv_from_array(args);
        REQUIRE_NOTHROW(parser.parse(argc, argv));
        CHECK_FALSE(parser.template get_option<Option>().has_value());
    }
}

template <CLArgs::CmdOption Option, CLArgs::StringLiteral Value, CLArgs::StringLiteral Identifier, CLArgs::StringLiteral... Identifiers>
constexpr void
test_option_passed()
{
    SECTION("Pass argument \"" + std::string(Identifier.value) + "\" to parser")
    {
        OptionTesterParser<Option> parser;
        constexpr std::array       args = {"program", Identifier.value, Value.value};
        auto [argc, argv]               = CLArgs::Testing::create_argc_argv_from_array(args);
        REQUIRE_NOTHROW(parser.parse(argc, argv));
        CHECK(parser.template get_option<Option>().has_value());
    }

    if constexpr (sizeof...(Identifiers) > 0)
    {
        test_option_passed<Option, Value, Identifiers...>();
    }
}

template <CLArgs::CmdOption Option, CLArgs::StringLiteral Value, CLArgs::StringLiteral... Identifiers>
constexpr void
test_option()
{
    test_option_not_passed<Option>();
    test_option_passed<Option, Value, Identifiers...>();
}

TEST_CASE("Parser with CommonOptions::Config", "[common_options]")
{
    test_option<CLArgs::CommonOptions::Config, "conf.ini", "--configuration", "--config">();
}
