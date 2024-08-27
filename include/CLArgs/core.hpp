#ifndef CLARGS_CORE_HPP
#define CLARGS_CORE_HPP

#include <algorithm>
#include <array>
#include <concepts>
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
    [[nodiscard]] consteval std::size_t count_delimiters();

    template <StringLiteral str, char delimiter = default_delimiter>
    [[nodiscard]] consteval std::size_t count_delimited_elements();

    template <StringLiteral str, char delimiter = default_delimiter>
    [[nodiscard]] consteval auto array_from_delimited_string();
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
    return std::ranges::count(strv, delimiter);
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

    static_assert(delimiter != '\0', "null character cannot be used as a delimiter.");
    static_assert(!strv.empty(), "string cannot be empty");
    static_assert(strv.front() != delimiter, "string cannot begin with delimiter");
    static_assert(strv.back() != delimiter, "string cannot end with delimiter");

    constexpr char consecutive_delimiters[] = {delimiter, delimiter, '\0'};
    static_assert(strv.find(consecutive_delimiters) == std::string_view::npos, "consecutive delimiters are not allowed");

    std::array<std::string_view, count_delimited_elements<str, delimiter>()> result{};

    auto       start       = strv.begin();
    const auto end         = strv.end();
    auto       result_iter = result.begin();

    for (auto iter = start; iter != end; ++iter)
    {
        if (const auto next = std::next(iter); *iter == delimiter || next == end)
        {
            const auto first = start;
            const auto last  = (*iter == delimiter) ? iter : next;
            *result_iter     = std::string_view{first, last};
            start            = next;
            result_iter += 1;
        }
    }

    return result;
}

#endif
