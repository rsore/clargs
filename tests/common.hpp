#ifndef CLARGS_TESTS_COMMON_HPP
#define CLARGS_TESTS_COMMON_HPP

#include <array>
#include <cstdint>

template <std::size_t N>
std::pair<int, char **>
create_argc_argv(const std::array<const char *, N> &args)
{
    const int argc = static_cast<int>(N);
    char    **argv = const_cast<char **>(args.data()); // NOLINT(cppcoreguidelines-pro-type-const-cast)
    return std::make_pair(argc, argv);
}

#define GIVE_ME_ARGC_ARGV(...)                   \
    constexpr std::array args = { __VA_ARGS__ }; \
    auto [argc, argv]         = create_argc_argv(args);

#endif
