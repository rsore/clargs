#ifndef CLARGS_ASSERT_HPP
#define CLARGS_ASSERT_HPP

#include <CLArgs/platform.hpp>

#include <iostream>
#include <source_location>
#include <string_view>

namespace CLArgs::_internal
{
    inline void assert(bool, std::string_view, std::source_location = std::source_location::current());
}

#ifdef NDEBUG
inline void
CLArgs::_internal::assert(const bool, const std::string_view, const std::source_location)
{
    // noopt
}
#else
inline void
CLArgs::_internal::assert(const bool expr, const std::string_view msg, const std::source_location loc)
{
    if (!expr)
    {
        std::cerr << loc.file_name() << ':' << loc.line() << " [ CLArgs assertion failed ] " << msg;
        debug_break();
        std::abort();
    }
}
#endif

#endif
