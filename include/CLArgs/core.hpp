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
        { T::identifiers } -> std::convertible_to<std::array<std::string_view, std::tuple_size_v<decltype(T::identifiers)>>>;
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

    template <typename T, typename... Ts>
    concept IsPartOf = (std::is_same_v<T, Ts> || ...);

    template <std::size_t N>
    struct StringLiteral
    {
        constexpr StringLiteral(const char (&str)[N]); // NOLINT(google-explicit-constructor)

        char value[N]{};
    };

    template <StringLiteral str, char delimiter = ','>
    [[nodiscard]] consteval auto array_from_delimited_string();
} // namespace CLArgs

template <std::size_t N>
constexpr CLArgs::StringLiteral<N>::StringLiteral(const char (&str)[N])
{
    std::copy_n(str, N, value);
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

    std::array<std::string_view, std::ranges::count(strv, delimiter) + 1> result{};

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
