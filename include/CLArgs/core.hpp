#ifndef CLARGS_CORE_HPP
#define CLARGS_CORE_HPP

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <ranges>
#include <string_view>
#include <tuple>

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

    template <CmdFlag... Flags>
    using CmdFlagList = std::tuple<Flags...>;

    template <CmdOption... Options>
    using CmdOptionList = std::tuple<Options...>;

    template <typename...>
    struct AllFlags : std::true_type
    {
    };

    template <typename First, typename... Rest>
    struct AllFlags<First, Rest...> : std::bool_constant<CmdFlag<First> && !CmdOption<First> && AllFlags<Rest...>::value>
    {
    };

    template <typename... Ts>
    inline constexpr bool all_flags_v = AllFlags<Ts...>::value;

    template <typename...>
    struct AllOptions : std::true_type
    {
    };

    template <typename First, typename... Rest>
    struct AllOptions<First, Rest...> : std::bool_constant<CmdOption<First> && AllOptions<Rest...>::value>
    {
    };

    template <typename... Ts>
    inline constexpr bool all_options_v = AllOptions<Ts...>::value;

    template <typename Tuple>
    struct TupleContainsAllFlags;

    template <typename... Ts>
    struct TupleContainsAllFlags<std::tuple<Ts...>> : AllFlags<Ts...>
    {
    };

    template <typename Tuple>
    inline constexpr bool tuple_contains_all_flags_v = TupleContainsAllFlags<Tuple>::value;

    template <typename Tuple>
    struct TupleContainsAllOptions;

    template <typename... Ts>
    struct TupleContainsAllOptions<std::tuple<Ts...>> : AllOptions<Ts...>
    {
    };

    template <typename Tuple>
    inline constexpr bool tuple_contains_all_options_v = TupleContainsAllOptions<Tuple>::value;

    template <typename T>
    concept Parsable = CmdFlag<T> || CmdOption<T>;

    template <typename T, typename... Ts>
    struct IsPartOf : std::disjunction<std::is_same<T, Ts>...>
    {
    };

    template <typename T, typename... Ts>
    inline constexpr bool is_part_of_v = IsPartOf<T, Ts...>::value;

    template <typename T, typename Tuple>
    struct IsPartOfTuple;

    template <typename T, typename... Ts>
    struct IsPartOfTuple<T, std::tuple<Ts...>> : std::disjunction<std::is_same<T, Ts>...>
    {
    };

    template <typename T, typename Tuple>
    inline constexpr bool is_part_of_tuple_v = IsPartOfTuple<T, Tuple>::value;

    template <typename...>
    struct AllUnique : std::true_type
    {
    };

    template <typename First, typename... Rest>
    struct AllUnique<First, Rest...> : std::conditional_t<is_part_of_v<First, Rest...>, std::false_type, AllUnique<Rest...>>
    {
    };

    template <typename... Ts>
    inline constexpr bool all_unique_v = AllUnique<Ts...>::value;

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
    inline constexpr std::size_t tuple_type_index_v = TupleTypeIndex<T, Tuple>::value;

    template <typename Tuple1, typename Tuple2>
    struct ConcatTuples;

    template <typename... Ts1, typename... Ts2>
    struct ConcatTuples<std::tuple<Ts1...>, std::tuple<Ts2...>>
    {
        using Type = std::tuple<Ts1..., Ts2...>;
    };

    template <typename Tuple1, typename Tuple2>
    using concat_tuples_t = typename ConcatTuples<Tuple1, Tuple2>::Type;

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

#endif // CLARGS_CORE_HPP
