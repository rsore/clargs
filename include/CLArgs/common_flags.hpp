#ifndef CLARGS_COMMON_FLAGS_HPP
#define CLARGS_COMMON_FLAGS_HPP

#include <CLArgs/core.hpp>

namespace CLArgs::CommonFlags
{
    using Help    = Flag<"--help,-h", "Show help menu">;
    using Verbose = Flag<"--verbose,-v", "Enable verbose output">;
    using Quiet   = Flag<"--quiet,-q", "Enable quiet output">;
} // namespace CLArgs::CommonFlags

#endif
