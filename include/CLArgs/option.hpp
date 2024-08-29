#ifndef CLARGS_OPTION_HPP
#define CLARGS_OPTION_HPP

#include <CLArgs/core.hpp>

namespace CLArgs
{
    template <const StringLiteral Identifiers,
              const StringLiteral ValueHint,
              const StringLiteral Description,
              const bool          Required,
              typename ValType>
    struct Option
    {
        static constexpr auto             identifiers{array_from_delimited_string<Identifiers>()};
        static constexpr std::string_view value_hint{ValueHint.value};
        static constexpr std::string_view description{Description.value};
        static constexpr bool             required{Required};
        using ValueType = ValType;

        static_assert(identifiers.size() >= 1, "Must have at least one identifier");
    };
} // namespace CLArgs

#endif
