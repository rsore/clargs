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
                   std::int8_t,
                   std::int16_t,
                   std::int32_t,
                   std::int64_t,
                   unsigned short,
                   unsigned int,
                   unsigned long,
                   unsigned long long,
                   std::uint8_t,
                   std::uint16_t,
                   std::uint32_t,
                   std::uint64_t,
                   std::size_t)
{
    CHECK_THROWS_AS(CLArgs::parse_value<TestType>(""), CLArgs::ParseValueException<TestType>);

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
        CHECK_THROWS_AS(CLArgs::parse_value<TestType>("-100"), CLArgs::ParseValueException<TestType>);
    }
}

TEMPLATE_TEST_CASE("parse_value() performs integer bounds checking",
                   "[parse_value]",
                   short,
                   int,
                   std::int8_t,
                   std::int16_t,
                   std::int32_t,
                   unsigned short,
                   unsigned int,
                   std::uint8_t,
                   std::uint16_t,
                   std::uint32_t)
{
    REQUIRE(CLArgs::parse_value<TestType>(std::to_string(std::numeric_limits<TestType>::min())) == std::numeric_limits<TestType>::min());
    REQUIRE(CLArgs::parse_value<TestType>(std::to_string(std::numeric_limits<TestType>::max())) == std::numeric_limits<TestType>::max());

    if constexpr (std::is_signed_v<TestType>)
    {
        CHECK_THROWS_AS(CLArgs::parse_value<TestType>(std::to_string(static_cast<std::int64_t>(std::numeric_limits<TestType>::min()) - 1)),
                        CLArgs::ParseValueException<TestType>);
    }
    CHECK_THROWS_AS(CLArgs::parse_value<TestType>(std::to_string(static_cast<std::uint64_t>(std::numeric_limits<TestType>::max()) + 1)),
                    CLArgs::ParseValueException<TestType>);
}

TEMPLATE_TEST_CASE("parse_value() verifies formatting",
                   "[parse_value]",
                   short,
                   int,
                   long,
                   long long,
                   std::int8_t,
                   std::int16_t,
                   std::int32_t,
                   std::int64_t,
                   unsigned short,
                   unsigned int,
                   unsigned long,
                   unsigned long long,
                   std::uint8_t,
                   std::uint16_t,
                   std::uint32_t,
                   std::uint64_t,
                   std::size_t)
{
    CHECK_THROWS_AS(CLArgs::parse_value<TestType>(""), CLArgs::ParseValueException<TestType>);
    CHECK_THROWS_AS(CLArgs::parse_value<TestType>("   "), CLArgs::ParseValueException<TestType>);
    CHECK_THROWS_AS(CLArgs::parse_value<TestType>(" 123"), CLArgs::ParseValueException<TestType>);
    CHECK_THROWS_AS(CLArgs::parse_value<TestType>("123 "), CLArgs::ParseValueException<TestType>);
    CHECK_THROWS_AS(CLArgs::parse_value<TestType>("abc"), CLArgs::ParseValueException<TestType>);
    CHECK_THROWS_AS(CLArgs::parse_value<TestType>("12abc34"), CLArgs::ParseValueException<TestType>);
    CHECK_THROWS_AS(CLArgs::parse_value<TestType>("12.34"), CLArgs::ParseValueException<TestType>);
    CHECK_THROWS_AS(CLArgs::parse_value<TestType>("@!#$"), CLArgs::ParseValueException<TestType>);
}

TEMPLATE_TEST_CASE("parse_value() can parse hexadecimal values for unsigned types",
                   "[parse_value]",
                   short,
                   int,
                   long,
                   long long,
                   std::int8_t,
                   std::int16_t,
                   std::int32_t,
                   std::int64_t,
                   unsigned short,
                   unsigned int,
                   unsigned long,
                   unsigned long long,
                   std::uint8_t,
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
        CHECK_THROWS_AS(CLArgs::parse_value<TestType>("0xFF"), CLArgs::ParseValueException<TestType>);
        CHECK_THROWS_AS(CLArgs::parse_value<TestType>("0XFF"), CLArgs::ParseValueException<TestType>);
    }
    CHECK_THROWS_AS(CLArgs::parse_value<TestType>("0x"), CLArgs::ParseValueException<TestType>);
    CHECK_THROWS_AS(CLArgs::parse_value<TestType>("0X"), CLArgs::ParseValueException<TestType>);
}

TEMPLATE_TEST_CASE("parse_value() can parse binary values for unsigned types",
                   "[parse_value]",
                   short,
                   int,
                   long,
                   long long,
                   std::int8_t,
                   std::int16_t,
                   std::int32_t,
                   std::int64_t,
                   unsigned short,
                   unsigned int,
                   unsigned long,
                   unsigned long long,
                   std::uint8_t,
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
        CHECK_THROWS_AS(CLArgs::parse_value<TestType>("0b100"), CLArgs::ParseValueException<TestType>);
        CHECK_THROWS_AS(CLArgs::parse_value<TestType>("0B100"), CLArgs::ParseValueException<TestType>);
    }
    CHECK_THROWS_AS(CLArgs::parse_value<TestType>("0b"), CLArgs::ParseValueException<TestType>);
    CHECK_THROWS_AS(CLArgs::parse_value<TestType>("0B"), CLArgs::ParseValueException<TestType>);
}

TEST_CASE("parse_value() can parse boolean values", "[parse_value]")
{
    CHECK_THROWS_AS(CLArgs::parse_value<bool>(""), CLArgs::ParseValueException<bool>);

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

    CHECK_THROWS_AS(CLArgs::parse_value<bool>("enable"), CLArgs::ParseValueException<bool>);
}

TEST_CASE("parse_value() can parse characters", "[parse_value]")
{
    CHECK_THROWS_AS(CLArgs::parse_value<char>(""), CLArgs::ParseValueException<char>);

    CHECK(CLArgs::parse_value<char>("a") == 'a');
    CHECK(CLArgs::parse_value<char>("Z") == 'Z');
    CHECK(CLArgs::parse_value<char>("1") == '1');
    CHECK(CLArgs::parse_value<char>("@") == '@');
    CHECK(CLArgs::parse_value<char>(" ") == ' ');
    CHECK(CLArgs::parse_value<char>("\n") == '\n');
    CHECK(CLArgs::parse_value<char>("\t") == '\t');

    CHECK_THROWS_AS(CLArgs::parse_value<char>("ab"), CLArgs::ParseValueException<char>);
    CHECK_THROWS_AS(CLArgs::parse_value<char>("123"), CLArgs::ParseValueException<char>);
    CHECK_THROWS_AS(CLArgs::parse_value<char>("abc"), CLArgs::ParseValueException<char>);
    CHECK_THROWS_AS(CLArgs::parse_value<char>("  "), CLArgs::ParseValueException<char>);

    CHECK_THROWS_AS(CLArgs::parse_value<char>("\0"), CLArgs::ParseValueException<char>);
    CHECK_THROWS_AS(CLArgs::parse_value<char>("\0\0"), CLArgs::ParseValueException<char>);
}
