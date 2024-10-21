#ifndef CLARGS_COMMON_FLAGS_HPP
#define CLARGS_COMMON_FLAGS_HPP

#include <CLArgs/core.hpp>

namespace CLArgs::CommonFlags
{
    using Help         = Flag<"--help,-h", "Show help menu">;
    using Verbose      = Flag<"--verbose,-v", "Enable verbose output">;
    using Quiet        = Flag<"--quiet,-q", "Enable quiet output">;
    using Version      = Flag<"--version", "Show program version">;
    using Recursive    = Flag<"--recursive,-r", "Enable recursive mode">;
    using All          = Flag<"--all,-a", "Include all">;
    using Force        = Flag<"--force,-f", "Force the action">;
    using Parallel     = Flag<"--parallel", "Enable parallel execution">;
    using Experimental = Flag<"--experimental", "Enable experimental features">;
    using Profile      = Flag<"--profile", "Profile program performance">;
    using Debug        = Flag<"--debug", "Run in debug mode">;
    using Overwrite    = Flag<"--overwrite", "Allow overwriting existing data">;
} // namespace CLArgs::CommonFlags

#endif
