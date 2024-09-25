#include <CLArgs/argument_queue.hpp>
#include "test_utils.hpp"

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <string_view>

using namespace std::literals;

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
    SECTION("argv cannot be nullptr")
    {
        auto [argc, _] = CLArgs::Testing::create_argc_argv_from_array(std::array{"foo", "bar", "baz"});

        CHECK_THROWS(CLArgs::ArgumentQueue(argc, nullptr));
    }

    SECTION("Any element of argv cannot be nullptr")
    {
        auto [argc, argv] = CLArgs::Testing::create_argc_argv_from_array(std::array{"foo", static_cast<const char *>(nullptr), "baz"});
        CHECK_THROWS(CLArgs::ArgumentQueue(argc, argv));
    }
}

TEST_CASE("Can get front element of ArgumentQueue", "[ArgumentQueue]")
{
    SECTION("Returns first element of queue when it exists")
    {
        {
            auto [argc, argv] = CLArgs::Testing::create_argc_argv_from_array(std::array{"foo", "bar", "baz"});
            CLArgs::ArgumentQueue queue(argc, argv);

            CHECK(queue.front() == "foo");
        }
        {
            auto [argc, argv] = CLArgs::Testing::create_argc_argv_from_array(std::array{"bar", "baz", "foo"});
            CLArgs::ArgumentQueue queue(argc, argv);

            CHECK(queue.front() == "bar");
        }
    }

    SECTION("Checking front() several times will return the same value")
    {
        auto [argc, argv] = CLArgs::Testing::create_argc_argv_from_array(std::array{"foo", "bar", "baz"});
        CLArgs::ArgumentQueue queue(argc, argv);

        CHECK(queue.front() == "foo");
        CHECK(queue.front() == "foo");
        CHECK(queue.front() == "foo");
    }
}

TEST_CASE("Can dequeue front element, reducing the size of the queue", "[ArgumentQueue]")
{
    {
        auto [argc, argv] = CLArgs::Testing::create_argc_argv_from_array(std::array{"foo", "bar", "baz"});

        CLArgs::ArgumentQueue queue(argc, argv);
        REQUIRE(queue.size() == 3);

        REQUIRE(queue.dequeue() == "foo");
        REQUIRE(queue.size() == 2);
        REQUIRE(queue.front() == "bar");
    }
    {
        auto [argc, argv] = CLArgs::Testing::create_argc_argv_from_array(std::array{"qux", "baz", "bar", "foo"});

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
        auto [argc, argv] = CLArgs::Testing::create_argc_argv_from_array(std::array{"foo"});

        CLArgs::ArgumentQueue queue(argc, argv);
        CHECK(queue.empty() == false);
    }

    SECTION("Dequeuing empty queue throws exception")
    {
        auto [argc, argv] = CLArgs::Testing::create_argc_argv_from_array(std::array{"foo"});

        CLArgs::ArgumentQueue queue(argc, argv);
        REQUIRE(queue.size() == 1);
        REQUIRE_FALSE(queue.empty());
        REQUIRE_NOTHROW(std::ignore = queue.dequeue());
        REQUIRE(queue.empty());
        REQUIRE_THROWS(std::ignore = queue.dequeue());
    }
}


TEST_CASE("ArgumentQueue supports dequeuing multiple elements", "[ArgumentQueue]")
{
    auto [argc, argv] = CLArgs::Testing::create_argc_argv_from_array(std::array{"foo", "bar", "baz", "qux", "quux", "corge"});
    CLArgs::ArgumentQueue queue(argc, argv);

    SECTION("Can retrieve N values, given the queue is larger than or equal to N")
    {
        REQUIRE(queue.size() == 6);
        CHECK(queue.dequeue<4>() == std::array{"foo"sv, "bar"sv, "baz"sv, "qux"sv});
        CHECK(queue.size() == 2);
        CHECK(queue.dequeue<2>() == std::array{"quux"sv, "corge"sv});
        CHECK(queue.empty());
        CHECK_THROWS(std::ignore = queue.dequeue<2>());
    }

    SECTION("Dequeued values are readable regardless of concurrency")
    {
        const auto first3 = queue.dequeue<3>();
        const auto next2  = queue.dequeue<2>();

        REQUIRE(queue.size() == 1);
        CHECK(queue.front() == "corge");
        CHECK(next2 == std::array{"qux"sv, "quux"sv});
        CHECK(first3 == std::array{"foo"sv, "bar"sv, "baz"sv});
    }
}
