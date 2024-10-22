#include <CLArgs/common_options.hpp>
#include <CLArgs/parser_builder.hpp>
#include "test_utils.hpp"

#include <catch2/catch_test_macros.hpp>

#include <array>

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

TEST_CASE("Parser with CommonOptions::Output", "[common_options]")
{
    test_option<CLArgs::CommonOptions::Output, "out.txt", "--output", "-o">();
}

TEST_CASE("Parser with CommonOptions::Input", "[common_options]")
{
    test_option<CLArgs::CommonOptions::Input, "in.txt", "--input", "-i">();
}

TEST_CASE("Parser with CommonOptions::Timeout", "[common_options]")
{
    test_option<CLArgs::CommonOptions::Timeout, "10", "--timeout">();
}

TEST_CASE("Parser with CommonOptions::Ip", "[common_options]")
{
    test_option<CLArgs::CommonOptions::Ip, "127.0.0.1", "--ip", "--address">();
}

TEST_CASE("Parser with CommonOptions::Port", "[common_options]")
{
    test_option<CLArgs::CommonOptions::Port, "6969", "--port">();
}

TEST_CASE("Parser with CommonOptions::Threads", "[common_options]")
{
    test_option<CLArgs::CommonOptions::Threads, "12", "--threads">();
}

TEST_CASE("Parser with CommonOptions::Username", "[common_options]")
{
    test_option<CLArgs::CommonOptions::Username, "donald_duck", "--username", "--user">();
}

TEST_CASE("Parser with CommonOptions::Password", "[common_options]")
{
    test_option<CLArgs::CommonOptions::Password, "opensaysm3", "--password", "--pass">();
}

TEST_CASE("Parser with CommonOptions::MaxRetries", "[common_options]")
{
    test_option<CLArgs::CommonOptions::MaxRetries, "2000", "--max-retries">();
}
