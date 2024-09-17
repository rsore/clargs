#ifndef CLARGS_OPTION_HPP
#define CLARGS_OPTION_HPP

#include <CLArgs/string_literal.hpp>

#include <concepts>

namespace CLArgs
{
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
} // namespace CLArgs

#endif
