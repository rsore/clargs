#ifndef CLARGS_COMMON_OPTIONS_HPP
#define CLARGS_COMMON_OPTIONS_HPP

#include <CLArgs/core.hpp>

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <string>

namespace CLArgs::CommonOptions
{
    using Config     = Option<"--configuration,--config", "<filepath>", "Specify the config file path", std::filesystem::path>;
    using Output     = Option<"--output,-o", "<filepath>", "Specify the output file path", std::filesystem::path>;
    using Input      = Option<"--input,-i", "<filepath>", "Specify the input file path", std::filesystem::path>;
    using Timeout    = Option<"--timeout", "<seconds>", "Specify the timeout duration in seconds", std::chrono::seconds>;
    using Ip         = Option<"--ip,--address", "<ip address>", "Specify the IP address", std::string>;
    using Port       = Option<"--port", "<number>", "Specify the port number", std::uint16_t>;
    using Threads    = Option<"--threads", "<number>", "Specify the number of threads", std::uint16_t>;
    using Username   = Option<"--username,--user", "<username>", "Specify the username", std::string>;
    using Password   = Option<"--password,--pass", "<password>", "Specify the password", std::string>;
    using MaxRetries = Option<"--max-retries", "<number>", "Specify the maximum number of retries", std::uint32_t>;
} // namespace CLArgs::CommonOptions

#endif // CLARGS_COMMON_OPTIONS_HPP
