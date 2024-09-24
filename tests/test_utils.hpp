#ifndef CLARGS_TEST_UTILS_HPP
#define CLARGS_TEST_UTILS_HPP

#include <array>

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
}

#endif
