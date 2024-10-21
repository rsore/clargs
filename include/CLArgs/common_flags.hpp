#ifndef CLARGS_COMMON_FLAGS_HPP
#define CLARGS_COMMON_FLAGS_HPP

#include <CLArgs/core.hpp>

namespace CLArgs::CommonFlags
{
    using All          = Flag<"--all,-a", "Include all">;
    using Debug        = Flag<"--debug", "Run in debug mode">;
    using Experimental = Flag<"--experimental", "Enable experimental features">;
    using Force        = Flag<"--force,-f", "Force the action">;
    using Help         = Flag<"--help,-h", "Show help menu">;
    using Overwrite    = Flag<"--overwrite", "Allow overwriting existing data">;
    using Parallel     = Flag<"--parallel", "Enable parallel execution">;
    using Profile      = Flag<"--profile", "Profile program performance">;
    using Quiet        = Flag<"--quiet,-q", "Enable quiet output">;
    using Recursive    = Flag<"--recursive,-r", "Enable recursive mode">;
    using Verbose      = Flag<"--verbose,-v", "Enable verbose output">;
    using Version      = Flag<"--version", "Show program version">;
} // namespace CLArgs::CommonFlags

#endif
