#ifndef CLARGS_PARSE_VALUE_HPP
#define CLARGS_PARSE_VALUE_HPP

#include <CLArgs/core.hpp>

#include <algorithm>
#include <array>
#include <charconv>
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
    what_ = std::string("Was not able to parse \"") + user_string.data() + "\" as type \"" + typeid(T).name() + "\": " + error_msg.data();
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
