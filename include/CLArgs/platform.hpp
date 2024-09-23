#ifndef CLARGS_PLATFORM_HPP
#define CLARGS_PLATFORM_HPP

namespace CLArgs::_internal
{
    inline void debug_break();
}

#if defined(_WIN32)
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>
inline void
CLArgs::_internal::debug_break()
{
    DebugBreak();
}
#elif defined(__linux__)
    #include <csignal>
inline void
CLArgs::_internal::debug_break()
{
    raise(SIGTRAP);
}
#else
inline void
CLArgs::_internal::debug_break()
{
    // noopt
}
#endif

#endif
