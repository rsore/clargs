#ifndef CLARGS_CORE_HPP
#define CLARGS_CORE_HPP

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <ranges>
#include <string_view>

namespace CLArgs
{
    template <std::size_t N>
    struct StringLiteral
    {
        constexpr StringLiteral(const char (&str)[N]); // NOLINT(google-explicit-constructor)

        char value[N]{};
    };

    template <StringLiteral str, char delimiter = ','>
    [[nodiscard]] consteval auto array_from_delimited_string();

    template <typename T>
    concept CmdFlag = requires {
        { T::identifiers } -> std::convertible_to<std::array<std::string_view, std::tuple_size_v<decltype(T::identifiers)>>>;
        { T::description } -> std::convertible_to<std::string_view>;

        typename T::ValueType;
        requires std::is_same_v<typename T::ValueType, bool>;
    };

    template <const StringLiteral Identifiers, const StringLiteral Description>
    struct Flag
    {
        static constexpr auto             identifiers{array_from_delimited_string<Identifiers>()};
        static constexpr std::string_view description{Description.value};

        using ValueType = bool;

        static_assert(identifiers.size() >= 1, "Must have at least one identifier");
    };

    template <typename T>
    concept CmdOption = requires {
        { T::identifiers } -> std::convertible_to<std::array<std::string_view, std::tuple_size_v<decltype(T::identifiers)>>>;
        { T::description } -> std::convertible_to<std::string_view>;
        { T::value_hint } -> std::convertible_to<std::string_view>;
        typename T::ValueType;
    };

    template <const StringLiteral Identifiers, const StringLiteral ValueHint, const StringLiteral Description, typename ValType>
    struct Option
    {
        static constexpr auto             identifiers{array_from_delimited_string<Identifiers>()};
        static constexpr std::string_view value_hint{ValueHint.value};
        static constexpr std::string_view description{Description.value};
        using ValueType = ValType;

        static_assert(identifiers.size() >= 1, "Must have at least one identifier");
    };

    template <typename T>
    concept Parsable = CmdFlag<T> || CmdOption<T>;

    template <typename T, typename... Ts>
    concept IsPartOf = (std::is_same_v<T, Ts> || ...);

    template <typename...>
    struct AllUnique : std::true_type
    {
    };

    template <typename First, typename... Rest>
    struct AllUnique<First, Rest...> : std::conditional_t<IsPartOf<First, Rest...>, std::false_type, AllUnique<Rest...>>
    {
    };

    template <typename... Ts>
    inline constexpr bool AllUnique_v = AllUnique<Ts...>::value;

    template <typename T, typename Tuple>
    struct TupleTypeIndex;

    template <typename T, typename... Types>
    struct TupleTypeIndex<T, std::tuple<T, Types...>>
    {
        static constexpr std::size_t value{0};
    };

    template <typename T, typename U, typename... Types>
    struct TupleTypeIndex<T, std::tuple<U, Types...>>
    {
        static constexpr std::size_t value = 1 + TupleTypeIndex<T, std::tuple<Types...>>::value;
    };

    template <typename T, typename Tuple>
    inline constexpr std::size_t TupleTypeIndex_v = TupleTypeIndex<T, Tuple>::value;

    template <Parsable Parsable>
    static consteval std::size_t identifier_list_length();

    template <Parsable This, Parsable... Rest>
    static consteval std::size_t max_identifier_list_length();
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

    constexpr std::size_t                       segment_count = std::ranges::count(strv, delimiter) + 1;
    std::array<std::string_view, segment_count> result{};

    std::ranges::transform(std::views::split(strv, delimiter),
                           result.begin(),
                           [](const auto &segment) -> std::string_view
                           {
                               const auto length = static_cast<std::size_t>(std::ranges::distance(segment));
                               return {segment.begin(), length};
                           });

    return result;
}

template <CLArgs::Parsable Parsable>
consteval std::size_t
CLArgs::identifier_list_length()
{
    std::size_t length{};

    for (const auto identifier : Parsable::identifiers)
    {
        length += identifier.length();
    }
    length += (Parsable::identifiers.size() - 1) * 2; // Account for ", " between identifiers

    return length;
}

template <CLArgs::Parsable This, CLArgs::Parsable... Rest>
consteval std::size_t
CLArgs::max_identifier_list_length()
{
    std::size_t max_length = identifier_list_length<This>();

    if constexpr (sizeof...(Rest) > 0)
    {
        max_length = std::max(max_length, max_identifier_list_length<Rest...>());
    }

    return max_length;
}

#endif
