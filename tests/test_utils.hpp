#ifndef CLARGS_TEST_UTILS_HPP
#define CLARGS_TEST_UTILS_HPP

#include <algorithm>
#include <array>
#include <cstddef>
#include <random>

namespace CLArgs::Testing
{
    template <std::size_t N>
    std::pair<int, char **>
    create_argc_argv_from_array(const std::array<const char *, N> &args)
    {
        const int argc = static_cast<int>(N);
        char    **argv = const_cast<char **>(args.data());
        return std::make_pair(argc, argv);
    }

    template <std::size_t MinLength, std::size_t MaxLength>
    std::string
    generate_random_string()
    {
        static constexpr std::array char_set{'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
                                             'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 'E', 'F',
                                             'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
                                             'W', 'X', 'Y', 'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
        static std::random_device   dev;
        static std::mt19937         rng(dev());
        static std::uniform_int_distribution<std::mt19937::result_type> char_set_dist(0, char_set.size() - 1);
        static std::uniform_int_distribution<std::mt19937::result_type> length_dist(MinLength, MaxLength);

        std::string random_string(length_dist(rng), ' ');
        std::ranges::generate(random_string, [] { return char_set[char_set_dist(rng)]; });
        return random_string;
    }

    template <std::size_t MinLength, std::size_t MaxLength, std::size_t N>
    std::array<std::string, N>
    generate_random_strings()
    {
        std::array<std::string, N> arr{};
        std::ranges::generate(arr, [] { return CLArgs::Testing::generate_random_string<MinLength, MaxLength>(); });

        return arr;
    }

    template <std::size_t N>
    std::array<const char *, N>
    cstr_array_from_string_array(const std::array<std::string, N> &str_arr)
    {
        std::array<const char *, N> sv_arr{};
        std::transform(str_arr.cbegin(), str_arr.cend(), sv_arr.begin(), [](const std::string &str) { return str.c_str(); });
        return sv_arr;
    }

} // namespace CLArgs::Testing

#endif
