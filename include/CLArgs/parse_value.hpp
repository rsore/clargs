#ifndef CLARGS_PARSE_VALUE_HPP
#define CLARGS_PARSE_VALUE_HPP

#include <algorithm>
#include <array>
#include <charconv>
#include <concepts>
#include <exception>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string_view>

namespace CLArgs
{
    template <typename T>
    T parse_value(std::string_view);

    template <std::integral T>
    T parse_value(std::string_view)
        requires(!std::is_same_v<T, bool> && !std::is_same_v<T, char>);
} // namespace CLArgs


template <std::integral T>
T
CLArgs::parse_value(const std::string_view sv)
    requires(!std::is_same_v<T, bool> && !std::is_same_v<T, char>)
{
    if (sv.empty())
    {
        throw std::invalid_argument("sv cannot be empty");
    }

    const auto [first, last, base] = [sv]
    {
        if constexpr (std::is_unsigned_v<T>)
        {
            if (constexpr std::array hex_prefixes{"0x", "0X"};
                std::any_of(hex_prefixes.begin(), hex_prefixes.end(), [sv](const auto &prefix) { return sv.starts_with(prefix); }))
            {
                return std::make_tuple(sv.data() + 2, sv.data() + sv.length(), 16);
            }

            if (constexpr std::array binary_prefixes{"0b", "0B"};
                std::any_of(binary_prefixes.begin(), binary_prefixes.end(), [sv](const auto &prefix) { return sv.starts_with(prefix); }))
            {
                return std::make_tuple(sv.data() + 2, sv.data() + sv.length(), 2);
            }
        }
        return std::make_tuple(sv.data(), sv.data() + sv.length(), 10);
    }();

    const auto throw_with_error = [sv](const std::string_view error_msg)
    {
        const std::string error =
                "Was not able to parse \"" + std::string(sv) + "\" as type \"" + typeid(T).name() + "\": " + error_msg.data();
        throw std::invalid_argument(error);
    };

    const T value = [first, last, base, &throw_with_error]
    {
        T val{};
        const auto [ptr, ec] = std::from_chars(first, last, val, base);

        if (ec == std::errc::invalid_argument)
        {
            throw_with_error("Invalid format");
        }

        if (ec == std::errc::result_out_of_range)
        {
            throw_with_error("Number out of range");
        }

        if (ptr != last)
        {
            const std::string error_msg = "Invalid character at '" + std::string(1, *ptr) + '\'';
            throw_with_error(error_msg);
        }

        return val;
    }();

    return value;
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
