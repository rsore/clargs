#ifndef CLARGS_VALIDATORS_HPP
#define CLARGS_VALIDATORS_HPP

#include <CLArgs/concepts.hpp>
#include <unordered_map>
#include <cstdint>
#include <string_view>
#include <typeindex>
#include <any>

namespace CLArgs
{
    struct MutualExclusionValidator
    {
        static constexpr std::string_view description{"Mutual exclusivity: At most one option may be passed"};

        template <typename TupleLike>
        static bool
        validate(OptionMap &options)
        {
            std::uint32_t count{};
            count_present_options<TupleLike>(options, count);
            return count <= 1;
        }

        template <typename TupleLike, std::uint32_t Index = 0>
        static void
        count_present_options(OptionMap &options, std::uint32_t &count)
        {
            using Option = std::tuple_element_t<Index, TupleLike>;
            static_assert(CmdOption<Option>);

            const auto key = std::type_index(typeid(Option));
            if (options.contains(key))
            {
                count += 1;
            }

            if constexpr (Index < std::tuple_size_v<TupleLike> - 1)
            {
                count_present_options<TupleLike, Index + 1>(options, count);
            }
        }
    };
}

#endif
