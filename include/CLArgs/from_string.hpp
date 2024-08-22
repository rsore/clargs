#ifndef CLARGS_FROM_STRING_HPP
#define CLARGS_FROM_STRING_HPP

#include <exception>
#include <filesystem>
#include <sstream>
#include <string_view>

namespace CLArgs
{
    template <typename T>
    T from_string(std::string_view);
}

template <>
inline int
CLArgs::from_string<int>(const std::string_view sv)
{
    if (sv.empty())
    {
        throw std::invalid_argument("sv cannot be empty");
    }

    return std::stoi(sv.data());
}

template <>
inline unsigned int
CLArgs::from_string<unsigned int>(const std::string_view sv)
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
CLArgs::from_string<long>(const std::string_view sv)
{
    if (sv.empty())
    {
        throw std::invalid_argument("sv cannot be empty");
    }

    return std::stol(sv.data());
}

template <>
inline unsigned long
CLArgs::from_string<unsigned long>(const std::string_view sv)
{
    if (sv.empty())
    {
        throw std::invalid_argument("sv cannot be empty");
    }

    return std::stoul(sv.data());
}

template <>
inline long long
CLArgs::from_string<long long>(const std::string_view sv)
{
    if (sv.empty())
    {
        throw std::invalid_argument("sv cannot be empty");
    }

    return std::stoll(sv.data());
}

template <>
inline unsigned long long
CLArgs::from_string<unsigned long long>(const std::string_view sv)
{
    if (sv.empty())
    {
        throw std::invalid_argument("sv cannot be empty");
    }

    return std::stoull(sv.data());
}

template <>
inline float
CLArgs::from_string<float>(const std::string_view sv)
{
    if (sv.empty())
    {
        throw std::invalid_argument("sv cannot be empty");
    }

    return std::stof(sv.data());
}

template <>
inline double
CLArgs::from_string<double>(const std::string_view sv)
{
    if (sv.empty())
    {
        throw std::invalid_argument("sv cannot be empty");
    }

    return std::stod(sv.data());
}

template <>
inline long double
CLArgs::from_string<long double>(const std::string_view sv)
{
    if (sv.empty())
    {
        throw std::invalid_argument("sv cannot be empty");
    }

    return std::stold(sv.data());
}

template <>
inline char
CLArgs::from_string<char>(const std::string_view sv)
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
CLArgs::from_string<std::string>(const std::string_view sv)
{
    if (sv.empty())
    {
        throw std::invalid_argument("sv cannot be empty");
    }

    return std::string(sv);
}

template <>
inline bool
CLArgs::from_string<bool>(const std::string_view sv)
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
CLArgs::from_string<std::filesystem::path>(const std::string_view sv)
{
    if (sv.empty())
    {
        throw std::invalid_argument("sv cannot be empty");
    }

    return sv;
}

#endif
