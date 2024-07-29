#ifndef CLARGS_PLATFORM_HPP
#define CLARGS_PLATFORM_HPP

#if defined(_WIN32)
#    ifndef NOMINMAX
#        define NOMINMAX
#    endif
#    include <windows.h>
#    define CLARGS_PLATFORM_DEBUG_BREAK() DebugBreak();
#elif defined(__linux__)
#    include <csignal>
#    define CLARGS_PLATFORM_DEBUG_BREAK() raise(SIGTRAP);
#else
#    define CLARGS_PLATFORM_DEBUG_BREAK()
#endif

#endif