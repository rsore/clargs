#include <CLArgs/argument_queue.hpp>
#include "../test_utils.hpp"

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <array>
#include <string_view>
#include <vector>

TEST_CASE("ArgumentQueue dequeuing", "[ArgumentQueue]")
{
    const auto str_arr      = CLArgs::Testing::generate_random_strings<5, 20, 10'000>();
    const auto args         = CLArgs::Testing::cstr_array_from_string_array(str_arr);
    const auto [argc, argv] = CLArgs::Testing::create_argc_argv_from_array(args);

    BENCHMARK("Dequeue 10000 elements from ArgumentQueue to std::vector using plain while loop")
    {
        CLArgs::ArgumentQueue queue(argc, argv);
        REQUIRE(queue.size() == 10'000);

        std::vector<std::string_view> results;
        results.reserve(queue.size());
        while (!queue.empty())
        {
            results.push_back(queue.dequeue());
        }

        REQUIRE(results.size() == 10'000);
        REQUIRE(queue.empty());

        return results;
    };

    BENCHMARK("Dequeue 10000 elements from ArgumentQueue to std::vector using std::ranges::generate")
    {
        CLArgs::ArgumentQueue queue(argc, argv);
        REQUIRE(queue.size() == 10'000);

        std::vector<std::string_view> results(queue.size());
        std::ranges::generate(results, [&queue] { return queue.dequeue(); });

        REQUIRE(results.size() == 10'000);
        REQUIRE(queue.empty());

        return results;
    };

    BENCHMARK("Dequeue 10000 elements from ArgumentQueue to std::array using std::ranges::generate")
    {
        CLArgs::ArgumentQueue queue(argc, argv);
        REQUIRE(queue.size() == 10'000);

        std::array<std::string_view, 10'000> results{};
        std::ranges::generate(results, [&queue] { return queue.dequeue(); });

        REQUIRE(results.size() == 10'000);
        REQUIRE(queue.empty());

        return results;
    };
}
