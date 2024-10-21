#ifndef CLARGS_COMMON_FLAGS_HPP
#define CLARGS_COMMON_FLAGS_HPP

#include <CLArgs/core.hpp>

namespace CLArgs::CommonFlags
{
    using Verbose = Flag<"--verbose,-v", "Enable verbose output">;
}

#endif
