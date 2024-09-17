#ifndef CLARGS_CORE_HPP
#define CLARGS_CORE_HPP

#include <CLArgs/flag.hpp>
#include <CLArgs/option.hpp>

#include <algorithm>
#include <array>
#include <concepts>
#include <cstdint>
#include <iostream>
#include <string_view>

namespace CLArgs
{
    template <typename T, typename... Ts>
    concept IsPartOf = (std::is_same_v<T, Ts> || ...);

    template <typename T>
    concept Parsable = CmdFlag<T> || CmdOption<T>;

    template <Parsable Parsable>
    static consteval std::size_t identifier_list_length();

    template <Parsable This, Parsable... Rest>
    static consteval std::size_t max_identifier_list_length();
} // namespace CLArgs

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
