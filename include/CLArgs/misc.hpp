#ifndef CLARGS_MISC_HPP
#define CLARGS_MISC_HPP

#include <CLArgs/platform.hpp>

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string_view>

#ifndef NDEBUG
    #define CLARGS_ASSERT(expr, msg) ::CLArgs::_internal::assert_impl(expr, msg, __FILE__, __LINE__)
    #define CLARGS_DEBUG_BREAK() CLARGS_PLATFORM_DEBUG_BREAK()
#else
    #define CLARGS_ASSERT(expr, msg)
    #define CLARGS_DEBUG_BREAK()
#endif

namespace CLArgs::_internal
{
    void assert_impl(bool, std::string_view, std::string_view, int);
}

inline void
CLArgs::_internal::assert_impl(const bool expr, const std::string_view msg, const std::string_view file, const int line)
{
    if (!expr)
    {
        std::cerr << file << ':' << line << ": Assertion failed: " << msg;
        CLARGS_DEBUG_BREAK();
        std::abort();
    }
}

#endif
