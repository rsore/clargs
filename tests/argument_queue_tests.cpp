#include <CLArgs/argument_queue.hpp>
#include "test_utils.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("ArgumentQueue can report the current size", "[ArgumentQueue]")
{
    {
        constexpr std::array args{"foo"};
        auto [argc, argv] = CLArgs::Testing::create_argc_argv_from_array(args);

        CLArgs::ArgumentQueue queue(argc, argv);
        CHECK(queue.size() == 1);
    }
    {
        constexpr std::array args{"foo", "bar"};
        auto [argc, argv] = CLArgs::Testing::create_argc_argv_from_array(args);

        CLArgs::ArgumentQueue queue(argc, argv);
        CHECK(queue.size() == 2);
    }
    {
        constexpr std::array args{"foo", "bar", "baz"};
        auto [argc, argv] = CLArgs::Testing::create_argc_argv_from_array(args);

        CLArgs::ArgumentQueue queue(argc, argv);
        CHECK(queue.size() == 3);
    }
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

TEST_CASE("Can get front element of ArgumentQueue", "[ArgumentQueue]")
{
    SECTION("Returns first element of queue when it exists")
    {
        {
            constexpr std::array<const char *, 3> args{"foo", "bar", "baz"};
            auto [argc, argv] = CLArgs::Testing::create_argc_argv_from_array(args);
            CLArgs::ArgumentQueue queue(argc, argv);

            CHECK(queue.front() == "foo");
        }
        {
            constexpr std::array<const char *, 3> args{"bar", "baz", "foo"};
            auto [argc, argv] = CLArgs::Testing::create_argc_argv_from_array(args);
            CLArgs::ArgumentQueue queue(argc, argv);

            CHECK(queue.front() == "bar");
        }
    }
}

TEST_CASE("Can dequeue front element, reducing the size of the queue", "[ArgumentQueue]")
{
    {
        constexpr std::array args{"foo", "bar", "baz"};
        auto [argc, argv] = CLArgs::Testing::create_argc_argv_from_array(args);

        CLArgs::ArgumentQueue queue(argc, argv);
        REQUIRE(queue.size() == 3);

        REQUIRE(queue.dequeue() == "foo");
        REQUIRE(queue.size() == 2);
        REQUIRE(queue.front() == "bar");
    }
    {
        constexpr std::array args{"qux", "baz", "bar", "foo"};
        auto [argc, argv] = CLArgs::Testing::create_argc_argv_from_array(args);

        CLArgs::ArgumentQueue queue(argc, argv);
        REQUIRE(queue.size() == 4);

        REQUIRE(queue.dequeue() == "qux");
        REQUIRE(queue.size() == 3);
        REQUIRE(queue.front() == "baz");
    }
}

TEST_CASE("ArgumentQueue tracks emptiness", "[ArgumentQueue]")
{
    SECTION("Newly created queue is not empty")
    {
        constexpr std::array args{"foo"};
        auto [argc, argv] = CLArgs::Testing::create_argc_argv_from_array(args);

        CLArgs::ArgumentQueue queue(argc, argv);
        CHECK(queue.empty() == false);
    }

    SECTION("Dequeuing empty queue throws exception")
    {
        constexpr std::array args{"foo"};
        auto [argc, argv] = CLArgs::Testing::create_argc_argv_from_array(args);

        CLArgs::ArgumentQueue queue(argc, argv);
        REQUIRE(queue.size() == 1);
        REQUIRE(!queue.empty());
        REQUIRE_NOTHROW(std::ignore = queue.dequeue());
        REQUIRE(queue.empty());
        REQUIRE_THROWS(std::ignore = queue.dequeue());
    }
}

