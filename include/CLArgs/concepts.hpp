#ifndef CLARGS_CONCEPTS_HPP
#define CLARGS_CONCEPTS_HPP

#include <concepts>
#include <string_view>

namespace CLArgs
{
    template <typename T>
    concept CmdOption = requires {
        { T::identifier } -> std::convertible_to<std::string_view>;
        { T::description } -> std::convertible_to<std::string_view>;
        { T::required } -> std::convertible_to<bool>;
    };

    template <typename T>
    concept CmdHasValue = requires {
        { T::value_hint } -> std::convertible_to<std::string_view>;
        typename T::ValueType;
    };

    template <typename T>
    concept CmdHasAlias = requires {
        { T::alias } -> std::convertible_to<std::string_view>;
    };

    template <typename T, typename... Ts>
    concept IsPartOf = (std::is_same_v<T, Ts> || ...);
} // namespace CLArgs

#endif
