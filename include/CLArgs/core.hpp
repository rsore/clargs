#ifndef CLARGS_CORE_HPP
#define CLARGS_CORE_HPP

#include <CLArgs/flag.hpp>
#include <CLArgs/option.hpp>

#include <algorithm>
#include <array>
#include <concepts>
#include <iostream>
#include <string_view>

namespace CLArgs
{
    template <typename T, typename... Ts>
    concept IsPartOf = (std::is_same_v<T, Ts> || ...);

    template <typename T>
    concept Parsable = CmdFlag<T> || CmdOption<T>;
} // namespace CLArgs

#endif
