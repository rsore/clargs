#ifndef CLARGS_CORE_HPP
#define CLARGS_CORE_HPP

#include <algorithm>
#include <array>
#include <concepts>
#include <string>
#include <string_view>

namespace CLArgs
{
    template <typename T>
    concept CmdFlag = requires {
        { T::identifier } -> std::convertible_to<std::string_view>;
        { T::description } -> std::convertible_to<std::string_view>;
        { T::required } -> std::convertible_to<bool>;
    };

    template <typename T>
    concept CmdOption = CmdFlag<T> && requires {
        { T::value_hint } -> std::convertible_to<std::string_view>;
        typename T::ValueType;
    };

    template <typename T>
    concept Parseable = CmdFlag<T> || CmdOption<T>;

    template <typename T>
    concept CmdHasAlias = requires {
        { T::alias } -> std::convertible_to<std::string_view>;
    };

    template <typename T, typename... Ts>
    concept IsPartOf = (std::is_same_v<T, Ts> || ...);

    template <std::size_t N>
    struct StringLiteral
    {
        constexpr StringLiteral(const char (&str)[N]); // NOLINT (suppress non-explicit single-argument constructor warning)

        char value[N]{};
    };

    static constexpr auto default_delimiter{','};

    template <StringLiteral str, char delimiter = default_delimiter>
    consteval std::size_t count_delimiters();

    template <StringLiteral str, char delimiter = default_delimiter>
    consteval std::size_t count_delimited_elements();

    template <StringLiteral str, char delimiter = default_delimiter>
    consteval auto array_from_delimited_string();
} // namespace CLArgs

template <std::size_t N>
constexpr CLArgs::StringLiteral<N>::StringLiteral(const char (&str)[N])
{
    std::copy_n(str, N, value);
}

template <CLArgs::StringLiteral str, char delimiter>
consteval std::size_t
CLArgs::count_delimiters()
{
    constexpr std::string_view strv{str.value};

    std::size_t count = 0;
    for (const char ch : strv)
    {
        if (ch == delimiter)
        {
            count += 1;
        }
    }
    return count;
}

template <CLArgs::StringLiteral str, char delimiter>
consteval std::size_t
CLArgs::count_delimited_elements()
{
    return count_delimiters<str, delimiter>() + 1;
}

template <CLArgs::StringLiteral str, char delimiter>
consteval auto
CLArgs::array_from_delimited_string()
{
    constexpr std::string_view strv{str.value};

    static_assert(!strv.empty(), "string cannot be empty");
    static_assert(strv.front() != delimiter, "string cannot begin with delimiter");
    static_assert(strv.back() != delimiter, "string cannot end with delimiter");
    static_assert(strv.find(std::string(2, delimiter)) == std::string_view::npos, "consecutive delimiters are not allowed");

    std::array<std::string_view, count_delimited_elements<str, delimiter>()> result{};

    std::size_t start = 0;
    std::size_t index = 0;

    for (std::size_t i = 0; i <= strv.size(); ++i)
    {
        if (i == strv.size() || strv[i] == delimiter)
        {
            result[index++] = strv.substr(start, i - start);
            start           = i + 1;
        }
    }

    return result;
}

#endif
