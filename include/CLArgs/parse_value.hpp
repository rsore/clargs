#ifndef CLARGS_PARSE_VALUE_HPP
#define CLARGS_PARSE_VALUE_HPP

#include <CLArgs/core.hpp>

#include <algorithm>
#include <array>
#include <charconv>
#include <chrono>
#include <cmath>
#include <concepts>
#include <exception>
#include <filesystem>
#include <sstream>
#include <string_view>

namespace CLArgs
{
    template <typename T>
    T parse_value(std::string_view);

    template <std::integral T>
    T parse_value(std::string_view)
        requires(!std::is_same_v<T, bool> && !std::is_same_v<T, char>);

    template <std::floating_point T>
    T parse_value(std::string_view);

    template <typename T>
    concept StdChronoDuration = requires {
        typename T::rep;
        typename T::period;
    } && std::is_same_v<T, std::chrono::duration<typename T::rep, typename T::period>>;

    template <StdChronoDuration T>
    T parse_value(std::string_view);

    template <typename T>
    constexpr std::string_view pretty_string_of_type();

    template <std::integral T>
    constexpr std::string_view pretty_string_of_type()
        requires(!std::is_same_v<T, bool> && !std::is_same_v<T, char>);

    template <std::floating_point T>
    constexpr std::string_view pretty_string_of_type();

    template <StdChronoDuration T>
    constexpr std::string_view pretty_string_of_type();

    template <typename T>
    class ParseValueException final : public std::invalid_argument
    {
    public:
        explicit ParseValueException(std::string_view user_string, std::string_view error_msg);

        [[nodiscard]] const char *what() const noexcept override;

    private:
        std::string what_;
    };
} // namespace CLArgs

template <typename T>
CLArgs::ParseValueException<T>::ParseValueException(const std::string_view user_string, const std::string_view error_msg)
    : std::invalid_argument("")
{
    what_ = std::string("Unable to parse \"") + user_string.data() + "\" as type \"" + pretty_string_of_type<T>().data() +
            "\": " + error_msg.data();
}

template <typename T>
const char *
CLArgs::ParseValueException<T>::what() const noexcept
{
    return what_.c_str();
}

template <std::integral T>
T
CLArgs::parse_value(const std::string_view sv)
    requires(!std::is_same_v<T, bool> && !std::is_same_v<T, char>)
{
    if (sv.empty())
    {
        throw ParseValueException<T>(sv, "String cannot be empty");
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

    const T value = [first, last, base, sv]
    {
        T val{};
        const auto [ptr, ec] = std::from_chars(first, last, val, base);

        if (ec == std::errc::invalid_argument)
        {
            throw ParseValueException<T>(sv, "Invalid format");
        }

        if (ec == std::errc::result_out_of_range)
        {
            throw ParseValueException<T>(sv, "Number out of range");
        }

        if (ptr != last)
        {
            const std::string error_msg = std::string("Invalid character at '") + *ptr + '\'';
            throw ParseValueException<T>(sv, error_msg);
        }

        return val;
    }();

    return value;
}


template <std::floating_point T>
T
CLArgs::parse_value(const std::string_view sv)
{
    if (sv.empty())
    {
        throw ParseValueException<T>(sv, "String cannot be empty");
    }

    if (constexpr std::array hex_prefixes{"0x", "0X"};
        std::any_of(hex_prefixes.begin(), hex_prefixes.end(), [sv](const auto &prefix) { return sv.starts_with(prefix); }))
    {
        throw ParseValueException<T>(sv, "Hexadecimal formatting is not supported");
    }

    if (constexpr std::array binary_prefixes{"0b", "0B"};
        std::any_of(binary_prefixes.begin(), binary_prefixes.end(), [sv](const auto &prefix) { return sv.starts_with(prefix); }))
    {
        throw ParseValueException<T>(sv, "Binary formatting is not supported");
    }


    const T value = [sv]
    {
        const auto *first = sv.data();
        const auto  last  = sv.data() + sv.length();

        T val{};
        const auto [ptr, ec] = std::from_chars(first, last, val);

        if (ec == std::errc::invalid_argument)
        {
            throw ParseValueException<T>(sv, "Invalid format");
        }

        if (ec == std::errc::result_out_of_range)
        {
            throw ParseValueException<T>(sv, "Number out of range");
        }

        if (ptr != last)
        {
            const std::string error_msg = std::string("Invalid character at '") + *ptr + '\'';
            throw ParseValueException<T>(sv, error_msg);
        }

        return val;
    }();

    return value;
}

template <>
inline char
CLArgs::parse_value<char>(const std::string_view sv)
{
    if (sv.empty())
    {
        throw ParseValueException<char>(sv, "String cannot be empty");
    }

    if (sv.length() != 1)
    {
        throw ParseValueException<char>(sv, "Expected exactly one character");
    }

    return sv[0];
}

template <>
inline bool
CLArgs::parse_value<bool>(const std::string_view sv)
{
    if (sv.empty())
    {
        throw ParseValueException<bool>(sv, "String cannot be empty");
    }

    const auto any_matches_case_insensitive = []<StringLiteral... potential_matches>(const std::string_view to_check)
    {
        using Container = std::array<std::string_view, sizeof...(potential_matches)>;
        constexpr Container potential_matches_container{potential_matches.value...};
        return std::any_of(potential_matches_container.begin(),
                           potential_matches_container.end(),
                           [to_check](const auto &potential_match)
                           {
                               return std::equal(to_check.begin(),
                                                 to_check.end(),
                                                 potential_match.begin(),
                                                 potential_match.end(),
                                                 [](const char a, const char b) { return std::tolower(a) == std::tolower(b); });
                           });
    };

    if (any_matches_case_insensitive.operator()<"true", "yes", "y", "1">(sv))
    {
        return true;
    }

    if (any_matches_case_insensitive.operator()<"false", "no", "n", "0">(sv))
    {
        return false;
    }

    throw ParseValueException<bool>(sv, "Invalid format");
}

template <>
inline std::string
CLArgs::parse_value<std::string>(const std::string_view sv)
{
    if (sv.empty())
    {
        throw ParseValueException<std::string>(sv, "String cannot be empty");
    }

    return std::string(sv);
}

template <>
inline std::string_view
CLArgs::parse_value<std::string_view>(const std::string_view sv)
{
    if (sv.empty())
    {
        throw ParseValueException<std::string_view>(sv, "String cannot be empty");
    }

    return sv.data();
}

template <>
inline std::filesystem::path
CLArgs::parse_value<std::filesystem::path>(const std::string_view sv)
{
    if (sv.empty())
    {
        throw ParseValueException<std::filesystem::path>(sv, "String cannot be empty");
    }

    return {sv};
}

template <CLArgs::StdChronoDuration T>
T
CLArgs::parse_value(const std::string_view sv)
{
    if (sv.empty())
    {
        throw ParseValueException<T>(sv, "String cannot be empty");
    }

    using UnderlyingValueType = typename T::rep;

    try
    {
        const UnderlyingValueType value = parse_value<UnderlyingValueType>(sv);
        return T{std::move(value)};
    }
    catch (const ParseValueException<UnderlyingValueType> &e)
    {
        throw ParseValueException<T>(sv, std::move(e.what()));
    }
}

template <typename T>
constexpr std::string_view
CLArgs::pretty_string_of_type()
{
    return typeid(T).name();
}

template <std::integral T>
constexpr std::string_view
CLArgs::pretty_string_of_type()
    requires(!std::is_same_v<T, bool> && !std::is_same_v<T, char>)
{
    constexpr bool is_unsigned = std::is_unsigned_v<T>;
    switch (sizeof(T))
    {
        case 1:
            return is_unsigned ? "8-bit unsigned integer" : "8-bit signed integer";
        case 2:
            return is_unsigned ? "16-bit unsigned integer" : "16-bit signed integer";
        case 4:
            return is_unsigned ? "32-bit unsigned integer" : "32-bit signed integer";
        case 8:
            return is_unsigned ? "64-bit unsigned integer" : "64-bit signed integer";
        default:
            return is_unsigned ? "unsigned integer" : "signed integer";
    }
}

template <std::floating_point T>
constexpr std::string_view
CLArgs::pretty_string_of_type()
{
    switch (sizeof(T))
    {
        case 4:
            return "32-bit floating-point number";
        case 8:
            return "64-bit floating-point number";
        case 10:
            return "80-bit floating-point number";
        case 12:
            return "96-bit floating-point number";
        case 16:
            return "128-bit floating-point number";
        default:
            return "floating-point number";
    }
}

template <>
constexpr std::string_view
CLArgs::pretty_string_of_type<bool>()
{
    return "bool";
}

template <>
constexpr std::string_view
CLArgs::pretty_string_of_type<char>()
{
    return "char";
}

template <>
constexpr std::string_view
CLArgs::pretty_string_of_type<std::string>()
{
    return "string";
}

template <>
constexpr std::string_view
CLArgs::pretty_string_of_type<std::filesystem::path>()
{
    return "filesystem path";
}

template <CLArgs::StdChronoDuration T>
constexpr std::string_view
CLArgs::pretty_string_of_type()
{
    if constexpr (std::is_same_v<typename T::period, std::chrono::seconds::period>)
    {
        return "seconds";
    }
    else if constexpr (std::is_same_v<typename T::period, std::chrono::milliseconds::period>)
    {
        return "milliseconds";
    }
    else if constexpr (std::is_same_v<typename T::period, std::chrono::microseconds::period>)
    {
        return "microseconds";
    }
    else if constexpr (std::is_same_v<typename T::period, std::chrono::nanoseconds::period>)
    {
        return "nanoseconds";
    }
    else if constexpr (std::is_same_v<typename T::period, std::chrono::minutes::period>)
    {
        return "minutes";
    }
    else if constexpr (std::is_same_v<typename T::period, std::chrono::hours::period>)
    {
        return "hours";
    }
    else if constexpr (std::is_same_v<typename T::period, std::chrono::days::period>)
    {
        return "days";
    }
    else if constexpr (std::is_same_v<typename T::period, std::chrono::weeks::period>)
    {
        return "weeks";
    }
    else if constexpr (std::is_same_v<typename T::period, std::chrono::months::period>)
    {
        return "months";
    }
    else if constexpr (std::is_same_v<typename T::period, std::chrono::years::period>)
    {
        return "years";
    }

    return "duration";
}

#endif
