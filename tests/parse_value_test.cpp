#include <catch2/catch_template_test_macros.hpp>

#include <CLArgs/parse_value.hpp>

#include <cstdint>
#include <ranges>

TEMPLATE_TEST_CASE("parse_value() can parse all integer types",
                   "[parse_value]",
                   short,
                   int,
                   long,
                   long long,
                   std::int16_t,
                   std::int32_t,
                   std::int64_t,
                   unsigned short,
                   unsigned int,
                   unsigned long,
                   unsigned long long,
                   std::uint16_t,
                   std::uint32_t,
                   std::uint64_t,
                   std::size_t)
{
    REQUIRE(CLArgs::parse_value<TestType>("0") == 0);
    REQUIRE(CLArgs::parse_value<TestType>("123") == 123);
    REQUIRE(CLArgs::parse_value<TestType>("0000") == 0);
    REQUIRE(CLArgs::parse_value<TestType>("00010") == 10);

    REQUIRE(std::is_same_v<decltype(CLArgs::parse_value<TestType>("0")), TestType>);

    if constexpr (std::is_signed_v<TestType>)
    {
        REQUIRE(CLArgs::parse_value<TestType>("-100") == -100);
    }
    else
    {
        REQUIRE_THROWS(CLArgs::parse_value<TestType>("-100"));
    }
}

TEMPLATE_TEST_CASE("parse_value() performs integer bounds checking",
                   "[parse_value]",
                   short,
                   int,
                   std::int16_t,
                   std::int32_t,
                   unsigned short,
                   unsigned int,
                   std::uint16_t,
                   std::uint32_t)
{
    REQUIRE(CLArgs::parse_value<TestType>(std::to_string(std::numeric_limits<TestType>::min())) == std::numeric_limits<TestType>::min());
    REQUIRE(CLArgs::parse_value<TestType>(std::to_string(std::numeric_limits<TestType>::max())) == std::numeric_limits<TestType>::max());

    if constexpr (std::is_signed_v<TestType>)
    {
        REQUIRE_THROWS(CLArgs::parse_value<TestType>(std::to_string(static_cast<std::int64_t>(std::numeric_limits<TestType>::min()) - 1)));
    }
    REQUIRE_THROWS(CLArgs::parse_value<TestType>(std::to_string(static_cast<std::uint64_t>(std::numeric_limits<TestType>::max()) + 1)));
}

TEMPLATE_TEST_CASE("parse_value() verifies formatting",
                   "[parse_value]",
                   short,
                   int,
                   long,
                   long long,
                   std::int16_t,
                   std::int32_t,
                   std::int64_t,
                   unsigned short,
                   unsigned int,
                   unsigned long,
                   unsigned long long,
                   std::uint16_t,
                   std::uint32_t,
                   std::uint64_t,
                   std::size_t)
{
    REQUIRE_THROWS(CLArgs::parse_value<TestType>(""));
    REQUIRE_THROWS(CLArgs::parse_value<TestType>("   "));
    REQUIRE_THROWS(CLArgs::parse_value<TestType>(" 123"));
    REQUIRE_THROWS(CLArgs::parse_value<TestType>("123 "));
    REQUIRE_THROWS(CLArgs::parse_value<TestType>("abc"));
    REQUIRE_THROWS(CLArgs::parse_value<TestType>("12abc34"));
    REQUIRE_THROWS(CLArgs::parse_value<TestType>("12.34"));
    REQUIRE_THROWS(CLArgs::parse_value<TestType>("@!#$"));
}
