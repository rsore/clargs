#include <CLArgs/argument_queue.hpp>
#include "test_utils.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("ArgumentQueue tracks emptiness", "[ArgumentQueue]")
{
    constexpr std::array args{"foo"};
    auto [argc, argv] = CLArgs::Testing::create_argc_argv_from_array(args);

    CLArgs::ArgumentQueue queue(argc, argv);
    CHECK(queue.empty() == false);

    // TODO: Dequeue element, check not empty
}

TEST_CASE("ArgumentQueue throws when invalid arguments are passed", "[ArgumentQueue]")
{
    SECTION("argc cannot be less than 0")
    {
        constexpr std::array args{"foo", "bar", "baz"};
        auto [_, argv] = CLArgs::Testing::create_argc_argv_from_array(args);

        CHECK_THROWS(CLArgs::ArgumentQueue(-1, argv));
        CHECK_THROWS(CLArgs::ArgumentQueue(-5, argv));
    }

    SECTION("argv cannot be nullptr")
    {
        constexpr std::array args{"foo", "bar", "baz"};
        auto [argc, _] = CLArgs::Testing::create_argc_argv_from_array(args);

        CHECK_THROWS(CLArgs::ArgumentQueue(argc, nullptr));
    }

    SECTION("Any element of argv cannot be nullptr")
    {
        constexpr std::array<const char *, 3> args{"foo", nullptr, "baz"};
        auto [argc, argv] = CLArgs::Testing::create_argc_argv_from_array(args);
        CHECK_THROWS(CLArgs::ArgumentQueue(argc, argv));
    }
}
