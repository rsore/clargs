#include <CLArgs/parse_value.hpp>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <string>
#include <string_view>
#include <type_traits>

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
        CHECK_THROWS_AS(CLArgs::parse_value<TestType>(std::to_string(std::numeric_limits<std::int64_t>::min())),
                        CLArgs::ParseValueException<TestType>);
    }
    CHECK_THROWS_AS(CLArgs::parse_value<TestType>(std::to_string(static_cast<std::uint64_t>(std::numeric_limits<TestType>::max()) + 1)),
                    CLArgs::ParseValueException<TestType>);
    CHECK_THROWS_AS(CLArgs::parse_value<TestType>(std::to_string(std::numeric_limits<std::uint64_t>::max())),
                    CLArgs::ParseValueException<TestType>);
}

TEMPLATE_TEST_CASE("parse_value() verifies formatting for integer types",
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

TEMPLATE_TEST_CASE("parse_value() can parse all floating-point types", "[parse_value]", float, double, long double)
{
    CHECK_THROWS_AS(CLArgs::parse_value<TestType>(""), CLArgs::ParseValueException<TestType>);

    CHECK(std::is_same_v<decltype(CLArgs::parse_value<TestType>("0.0")), TestType>);

    CHECK_THAT(CLArgs::parse_value<TestType>("0.0"), Catch::Matchers::WithinRel(0.0, 0.001));
    CHECK_THAT(CLArgs::parse_value<TestType>("0.5"), Catch::Matchers::WithinRel(0.5, 0.001));
    CHECK_THAT(CLArgs::parse_value<TestType>("123"), Catch::Matchers::WithinRel(123.0, 0.001));
    CHECK_THAT(CLArgs::parse_value<TestType>("0000"), Catch::Matchers::WithinRel(0.0, 0.001));
    CHECK_THAT(CLArgs::parse_value<TestType>("0000.000"), Catch::Matchers::WithinRel(0.0, 0.001));
    CHECK_THAT(CLArgs::parse_value<TestType>("0010.010"), Catch::Matchers::WithinRel(10.01, 0.001));
    CHECK_THAT(CLArgs::parse_value<TestType>("-3.1415"), Catch::Matchers::WithinRel(-3.1415, 0.001));
    CHECK_THAT(CLArgs::parse_value<TestType>(".123"), Catch::Matchers::WithinRel(0.123, 0.001));
}

TEST_CASE("parse_value() performs floating-point bounds checking", "[parse_value], [!nonportable]")
{
    REQUIRE_THAT(CLArgs::parse_value<float>(std::to_string(std::numeric_limits<float>::lowest())),
                 Catch::Matchers::WithinRel(std::numeric_limits<float>::lowest()));

    REQUIRE_THAT(CLArgs::parse_value<float>(std::to_string(std::numeric_limits<float>::max())),
                 Catch::Matchers::WithinRel(std::numeric_limits<float>::max()));

    if constexpr (sizeof(long double) > sizeof(float)) // NOLINT
    {
        CHECK_THROWS_AS(CLArgs::parse_value<float>(std::to_string(std::numeric_limits<long double>::lowest())),
                        CLArgs::ParseValueException<float>);
        CHECK_THROWS_AS(CLArgs::parse_value<float>(std::to_string(std::numeric_limits<long double>::max())),
                        CLArgs::ParseValueException<float>);
    }
}

TEMPLATE_TEST_CASE("parse_value() verifies formatting for floating-point types", "[parse_value]", float, double, long double)
{
    CHECK_THROWS_AS(CLArgs::parse_value<TestType>(""), CLArgs::ParseValueException<TestType>);
    CHECK_THROWS_AS(CLArgs::parse_value<TestType>("   "), CLArgs::ParseValueException<TestType>);
    CHECK_THROWS_AS(CLArgs::parse_value<TestType>(" 123.0"), CLArgs::ParseValueException<TestType>);
    CHECK_THROWS_AS(CLArgs::parse_value<TestType>("123.3 "), CLArgs::ParseValueException<TestType>);
    CHECK_THROWS_AS(CLArgs::parse_value<TestType>("abc"), CLArgs::ParseValueException<TestType>);
    CHECK_THROWS_AS(CLArgs::parse_value<TestType>("12abc34"), CLArgs::ParseValueException<TestType>);
    CHECK_THROWS_AS(CLArgs::parse_value<TestType>("12..34"), CLArgs::ParseValueException<TestType>);
    CHECK_THROWS_AS(CLArgs::parse_value<TestType>("@!#$"), CLArgs::ParseValueException<TestType>);
}

TEMPLATE_TEST_CASE("parse_value() cannot parse hexadecimal values for floating-point numbers", "[parse_value]", float, double, long double)
{
    CHECK_THROWS_AS(CLArgs::parse_value<TestType>("0x"), CLArgs::ParseValueException<TestType>);
    CHECK_THROWS_AS(CLArgs::parse_value<TestType>("0X"), CLArgs::ParseValueException<TestType>);
    CHECK_THROWS_AS(CLArgs::parse_value<TestType>("0xFF"), CLArgs::ParseValueException<TestType>);
    CHECK_THROWS_AS(CLArgs::parse_value<TestType>("0XFF"), CLArgs::ParseValueException<TestType>);
    CHECK_THROWS_AS(CLArgs::parse_value<TestType>("0x1A.3"), CLArgs::ParseValueException<TestType>);
    CHECK_THROWS_AS(CLArgs::parse_value<TestType>("0X4E.C"), CLArgs::ParseValueException<TestType>);
}

TEMPLATE_TEST_CASE("parse_value() cannot parse binary values for floating-point numbers", "[parse_value]", float, double, long double)
{
    CHECK_THROWS_AS(CLArgs::parse_value<TestType>("0b"), CLArgs::ParseValueException<TestType>);
    CHECK_THROWS_AS(CLArgs::parse_value<TestType>("0B"), CLArgs::ParseValueException<TestType>);
    CHECK_THROWS_AS(CLArgs::parse_value<TestType>("0b010"), CLArgs::ParseValueException<TestType>);
    CHECK_THROWS_AS(CLArgs::parse_value<TestType>("0B1101"), CLArgs::ParseValueException<TestType>);
    CHECK_THROWS_AS(CLArgs::parse_value<TestType>("0b01.1001"), CLArgs::ParseValueException<TestType>);
    CHECK_THROWS_AS(CLArgs::parse_value<TestType>("0b1001.0110"), CLArgs::ParseValueException<TestType>);
}

TEST_CASE("parse_value() can parse characters", "[parse_value]")
{
    CHECK_THROWS_AS(CLArgs::parse_value<char>(""), CLArgs::ParseValueException<char>);

    CHECK(std::is_same_v<decltype(CLArgs::parse_value<char>("a")), char>);

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

TEST_CASE("parse_value() can parse boolean values", "[parse_value]")
{
    CHECK_THROWS_AS(CLArgs::parse_value<bool>(""), CLArgs::ParseValueException<bool>);

    CHECK(std::is_same_v<decltype(CLArgs::parse_value<bool>("true")), bool>);

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

TEST_CASE("parse_value() can parse strings", "[parse_value]")
{
    CHECK_THROWS_AS(CLArgs::parse_value<std::string>(""), CLArgs::ParseValueException<std::string>);

    CHECK(std::is_same_v<decltype(CLArgs::parse_value<std::string>("foo")), std::string>);

    CHECK_THAT(CLArgs::parse_value<std::string>("Hello world"), Catch::Matchers::Equals("Hello world"));
    CHECK_THAT(CLArgs::parse_value<std::string>("Foo"), Catch::Matchers::Equals("Foo"));
    CHECK_THAT(CLArgs::parse_value<std::string>("    Bar"), Catch::Matchers::Equals("    Bar"));
    CHECK_THAT(CLArgs::parse_value<std::string>("Baz  "), Catch::Matchers::Equals("Baz  "));
}

TEST_CASE("parse_value() can parse strings views", "[parse_value]")
{
    CHECK_THROWS_AS(CLArgs::parse_value<std::string_view>(""), CLArgs::ParseValueException<std::string_view>);

    CHECK(std::is_same_v<decltype(CLArgs::parse_value<std::string_view>("bar")), std::string_view>);

    CHECK(CLArgs::parse_value<std::string_view>("Hello world") == "Hello world");
    CHECK(CLArgs::parse_value<std::string_view>("Foo") == "Foo");
    CHECK(CLArgs::parse_value<std::string_view>("    Bar") == "    Bar");
    CHECK(CLArgs::parse_value<std::string_view>("Baz  ") == "Baz  ");
}

TEST_CASE("parse_value() can parse filesystem paths", "[parse_value]")
{
    CHECK_THROWS_AS(CLArgs::parse_value<std::filesystem::path>(""), CLArgs::ParseValueException<std::filesystem::path>);

    CHECK(std::is_same_v<decltype(CLArgs::parse_value<std::filesystem::path>("hello.txt")), std::filesystem::path>);

    CHECK(CLArgs::parse_value<std::filesystem::path>("/home/user/testfile.txt") == std::filesystem::path("/home/user/testfile.txt"));
    CHECK(CLArgs::parse_value<std::filesystem::path>("/var/log/system.log") == std::filesystem::path("/var/log/system.log"));
    CHECK(CLArgs::parse_value<std::filesystem::path>("./nginx/nginx.conf") == std::filesystem::path("./nginx/nginx.conf"));
    CHECK(CLArgs::parse_value<std::filesystem::path>("../bin/python3") == std::filesystem::path("../bin/python3"));

    CHECK(CLArgs::parse_value<std::filesystem::path>("C:\\Program Files\\MyApp\\config.json") ==
          std::filesystem::path("C:/Program Files/MyApp/config.json"));
    CHECK(CLArgs::parse_value<std::filesystem::path>("D:\\Projects\\Code\\main.py") == std::filesystem::path("D:/Projects/Code/main.py"));
    CHECK(CLArgs::parse_value<std::filesystem::path>(".\\Users\\JohnDoe\\Documents\\resume.docx") ==
          std::filesystem::path("./Users/JohnDoe/Documents/resume.docx"));
    CHECK(CLArgs::parse_value<std::filesystem::path>("..\\Media\\Videos\\vacation_clip.mp4") ==
          std::filesystem::path("../Media/Videos/vacation_clip.mp4"));
    CHECK(CLArgs::parse_value<std::filesystem::path>("//ServerName\\SharedFolder\\backup_2024.zip") ==
          std::filesystem::path("//ServerName/SharedFolder/backup_2024.zip"));
}

TEMPLATE_TEST_CASE("parse_value() can parse std::chrono::duration types",
                   "[parse_value]",
                   std::chrono::nanoseconds,
                   std::chrono::microseconds,
                   std::chrono::milliseconds,
                   std::chrono::seconds,
                   std::chrono::minutes,
                   std::chrono::hours,
                   std::chrono::days,
                   std::chrono::weeks,
                   std::chrono::months,
                   std::chrono::years,
                   std::chrono::duration<float>,
                   std::chrono::duration<double>,
                   std::chrono::duration<long double>,
                   std::chrono::duration<std::uint8_t>,
                   std::chrono::duration<std::uint16_t>,
                   std::chrono::duration<std::uint32_t>,
                   std::chrono::duration<std::uint64_t>,
                   std::chrono::duration<std::int8_t>,
                   std::chrono::duration<std::int16_t>,
                   std::chrono::duration<std::int32_t>,
                   std::chrono::duration<std::int64_t>)
{
    CHECK_THROWS_AS(CLArgs::parse_value<TestType>(""), CLArgs::ParseValueException<TestType>);
    CHECK_THROWS_AS(CLArgs::parse_value<TestType>(" "), CLArgs::ParseValueException<TestType>);

    CHECK(std::is_same_v<decltype(CLArgs::parse_value<TestType>("1")), TestType>);

    CHECK(CLArgs::parse_value<TestType>("1") == TestType{1});
    CHECK(CLArgs::parse_value<TestType>("23") == TestType{23});

    CHECK_THROWS_AS(CLArgs::parse_value<TestType>("abc"), CLArgs::ParseValueException<TestType>);
    CHECK_THROWS_AS(CLArgs::parse_value<TestType>(" 20"), CLArgs::ParseValueException<TestType>);
    CHECK_THROWS_AS(CLArgs::parse_value<TestType>("10 "), CLArgs::ParseValueException<TestType>);
    CHECK_THROWS_AS(CLArgs::parse_value<TestType>(" 50 "), CLArgs::ParseValueException<TestType>);

    if constexpr (std::floating_point<typename TestType::rep>)
    {
        CHECK(CLArgs::parse_value<TestType>("1.5") == TestType{1.5});
    }
    else
    {
        CHECK_THROWS_AS(CLArgs::parse_value<TestType>("1.5"), CLArgs::ParseValueException<TestType>);
    }

    if constexpr (std::is_signed_v<typename TestType::rep>)
    {
        CHECK(CLArgs::parse_value<TestType>("-1") == TestType{-1});
        CHECK(CLArgs::parse_value<TestType>("-23") == TestType{-23});
    }
    else
    {
        CHECK_THROWS_AS(CLArgs::parse_value<TestType>("-1") == TestType{-1}, CLArgs::ParseValueException<TestType>);
        CHECK_THROWS_AS(CLArgs::parse_value<TestType>("-23") == TestType{-23}, CLArgs::ParseValueException<TestType>);
    }

    if constexpr (std::is_integral_v<typename TestType::rep>)
    {
        CHECK_THROWS_AS(CLArgs::parse_value<TestType>("9999999999999999999999999999999999999999"), CLArgs::ParseValueException<TestType>);
    }
}
