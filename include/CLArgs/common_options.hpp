#ifndef CLARGS_COMMON_OPTIONS_HPP
#define CLARGS_COMMON_OPTIONS_HPP

#include <CLArgs/core.hpp>

#include <filesystem>

namespace CLArgs::CommonOptions
{
    using Config = Option<"--configuration,--config", "<filepath>", "Specify the path to configuration file", std::filesystem::path>;
}

#endif
