#ifndef CLARGS_FLAG_HPP
#define CLARGS_FLAG_HPP

#include <CLArgs/string_literal.hpp>

namespace CLArgs
{
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
} // namespace CLArgs

#endif
