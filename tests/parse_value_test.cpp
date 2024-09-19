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
    CHECK_THROWS(CLArgs::parse_value<TestType>(""));

    CHECK(std::is_same_v<decltype(CLArgs::parse_value<TestType>("0")), TestType>);

    CHECK(CLArgs::parse_value<TestType>("0") == 0);
    CHECK(CLArgs::parse_value<TestType>("123") == 123);
    CHECK(CLArgs::parse_value<TestType>("0000") == 0);
    CHECK(CLArgs::parse_value<TestType>("00010") == 10);

    if constexpr (std::is_signed_v<TestType>)
    {
        CHECK(CLArgs::parse_value<TestType>("-100") == -100);
    }
    else
    {
        CHECK_THROWS(CLArgs::parse_value<TestType>("-100"));
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
        CHECK_THROWS(CLArgs::parse_value<TestType>(std::to_string(static_cast<std::int64_t>(std::numeric_limits<TestType>::min()) - 1)));
    }
    CHECK_THROWS(CLArgs::parse_value<TestType>(std::to_string(static_cast<std::uint64_t>(std::numeric_limits<TestType>::max()) + 1)));
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
    CHECK_THROWS(CLArgs::parse_value<TestType>(""));
    CHECK_THROWS(CLArgs::parse_value<TestType>("   "));
    CHECK_THROWS(CLArgs::parse_value<TestType>(" 123"));
    CHECK_THROWS(CLArgs::parse_value<TestType>("123 "));
    CHECK_THROWS(CLArgs::parse_value<TestType>("abc"));
    CHECK_THROWS(CLArgs::parse_value<TestType>("12abc34"));
    CHECK_THROWS(CLArgs::parse_value<TestType>("12.34"));
    CHECK_THROWS(CLArgs::parse_value<TestType>("@!#$"));
}

TEMPLATE_TEST_CASE("parse_value() can parse hexadecimal values for unsigned types",
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

    if constexpr (std::is_unsigned_v<TestType>)
    {
        CHECK(CLArgs::parse_value<TestType>("0xFF") == 255);
        CHECK(CLArgs::parse_value<TestType>("0XFF") == 255);
    }
    else
    {
        CHECK_THROWS(CLArgs::parse_value<TestType>("0xFF"));
        CHECK_THROWS(CLArgs::parse_value<TestType>("0XFF"));
    }
    CHECK_THROWS(CLArgs::parse_value<TestType>("0x"));
    CHECK_THROWS(CLArgs::parse_value<TestType>("0X"));
}

TEMPLATE_TEST_CASE("parse_value() can parse binary values for unsigned types",
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
    if constexpr (std::is_unsigned_v<TestType>)
    {
        CHECK(CLArgs::parse_value<TestType>("0b100") == 4);
        CHECK(CLArgs::parse_value<TestType>("0B100") == 4);
    }
    else
    {
        CHECK_THROWS(CLArgs::parse_value<TestType>("0b100"));
        CHECK_THROWS(CLArgs::parse_value<TestType>("0B100"));
    }
    CHECK_THROWS(CLArgs::parse_value<TestType>("0b"));
    CHECK_THROWS(CLArgs::parse_value<TestType>("0B"));
}

TEST_CASE("parse_value() can parse boolean values", "[parse_value]")
{
    REQUIRE_THROWS(CLArgs::parse_value<bool>(""));

    CHECK(CLArgs::parse_value<bool>("true") == true);
    CHECK(CLArgs::parse_value<bool>("True") == true);
    CHECK(CLArgs::parse_value<bool>("TRUE") == true);
    CHECK(CLArgs::parse_value<bool>("TrUe") == true);
    CHECK(CLArgs::parse_value<bool>("yes") == true);
    CHECK(CLArgs::parse_value<bool>("YES") == true);
    CHECK(CLArgs::parse_value<bool>("Yes") == true);
    CHECK(CLArgs::parse_value<bool>("YeS") == true);
    CHECK(CLArgs::parse_value<bool>("y") == true);
    CHECK(CLArgs::parse_value<bool>("Y") == true);
    CHECK(CLArgs::parse_value<bool>("1") == true);

    CHECK(CLArgs::parse_value<bool>("false") == false);
    CHECK(CLArgs::parse_value<bool>("False") == false);
    CHECK(CLArgs::parse_value<bool>("FALSE") == false);
    CHECK(CLArgs::parse_value<bool>("FaLsE") == false);
    CHECK(CLArgs::parse_value<bool>("no") == false);
    CHECK(CLArgs::parse_value<bool>("NO") == false);
    CHECK(CLArgs::parse_value<bool>("No") == false);
    CHECK(CLArgs::parse_value<bool>("nO") == false);
    CHECK(CLArgs::parse_value<bool>("n") == false);
    CHECK(CLArgs::parse_value<bool>("N") == false);
    CHECK(CLArgs::parse_value<bool>("0") == false);

    REQUIRE_THROWS(CLArgs::parse_value<bool>("enable"));
}
