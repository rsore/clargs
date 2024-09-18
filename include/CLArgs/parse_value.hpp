#ifndef CLARGS_PARSE_VALUE_HPP
#define CLARGS_PARSE_VALUE_HPP

#include <exception>
#include <filesystem>
#include <sstream>
#include <string_view>

namespace CLArgs
{
    template <typename T>
    T parse_value(std::string_view);
}

template <>
inline int
CLArgs::parse_value<int>(const std::string_view sv)
{
    if (sv.empty())
    {
        throw std::invalid_argument("sv cannot be empty");
    }

    return std::stoi(sv.data());
}

template <>
inline unsigned int
CLArgs::parse_value<unsigned int>(const std::string_view sv)
{
    if (sv.empty())
    {
        throw std::invalid_argument("sv cannot be empty");
    }

    // TODO: Unsafe, might overflow
    return static_cast<unsigned int>(std::stoul(sv.data()));
}

template <>
inline long
CLArgs::parse_value<long>(const std::string_view sv)
{
    if (sv.empty())
    {
        throw std::invalid_argument("sv cannot be empty");
    }

    return std::stol(sv.data());
}

template <>
inline unsigned long
CLArgs::parse_value<unsigned long>(const std::string_view sv)
{
    if (sv.empty())
    {
        throw std::invalid_argument("sv cannot be empty");
    }

    return std::stoul(sv.data());
}

template <>
inline long long
CLArgs::parse_value<long long>(const std::string_view sv)
{
    if (sv.empty())
    {
        throw std::invalid_argument("sv cannot be empty");
    }

    return std::stoll(sv.data());
}

template <>
inline unsigned long long
CLArgs::parse_value<unsigned long long>(const std::string_view sv)
{
    if (sv.empty())
    {
        throw std::invalid_argument("sv cannot be empty");
    }

    return std::stoull(sv.data());
}

template <>
inline float
CLArgs::parse_value<float>(const std::string_view sv)
{
    if (sv.empty())
    {
        throw std::invalid_argument("sv cannot be empty");
    }

    return std::stof(sv.data());
}

template <>
inline double
CLArgs::parse_value<double>(const std::string_view sv)
{
    if (sv.empty())
    {
        throw std::invalid_argument("sv cannot be empty");
    }

    return std::stod(sv.data());
}

template <>
inline long double
CLArgs::parse_value<long double>(const std::string_view sv)
{
    if (sv.empty())
    {
        throw std::invalid_argument("sv cannot be empty");
    }

    return std::stold(sv.data());
}

template <>
inline char
CLArgs::parse_value<char>(const std::string_view sv)
{
    if (sv.empty())
    {
        throw std::invalid_argument("sv cannot be empty");
    }

    if (sv.length() != 1)
    {
        std::stringstream ss;
        ss << "Expected exactly one character, got \"" << sv << "\" (" << sv.length() << " characters";
        throw std::invalid_argument(ss.str());
    }
    return sv[0];
}

template <>
inline std::string
CLArgs::parse_value<std::string>(const std::string_view sv)
{
    if (sv.empty())
    {
        throw std::invalid_argument("sv cannot be empty");
    }

    return std::string(sv);
}

template <>
inline bool
CLArgs::parse_value<bool>(const std::string_view sv)
{
    if (sv.empty())
    {
        throw std::invalid_argument("sv cannot be empty");
    }

    if (sv == "true" || sv == "True" || sv == "TRUE" || sv == "1")
    {
        return true;
    }

    if (sv == "false" || sv == "False" || sv == "FALSE" || sv == "0")
    {
        return false;
    }

    throw std::invalid_argument(R"(Valid bool values are "true", "True", "TRUE", "1", "false", "False", "FALSE" and "0")");
}

template <>
inline std::filesystem::path
CLArgs::parse_value<std::filesystem::path>(const std::string_view sv)
{
    if (sv.empty())
    {
        throw std::invalid_argument("sv cannot be empty");
    }

    return sv;
}

#endif
