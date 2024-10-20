#ifndef CLARGS_PARSER_BUILDER_HPP
#define CLARGS_PARSER_BUILDER_HPP

#include <CLArgs/core.hpp>
#include <CLArgs/parser.hpp>

namespace CLArgs
{
    template <typename Flags = CmdFlagList<>, typename Options = CmdOptionList<>>
    class ParserBuilder;


    template <CmdFlag... Flags, CmdOption... Options>
    class ParserBuilder<CmdFlagList<Flags...>, CmdOptionList<Options...>>
    {
    public:
        template <CmdFlag NewFlag>
        [[nodiscard]] consteval auto add_flag();

        template <CmdOption NewOption>
        [[nodiscard]] consteval auto add_option();

        [[nodiscard]] constexpr auto build();
    };
} // namespace CLArgs

template <CLArgs::CmdFlag... Flags, CLArgs::CmdOption... Options>
template <CLArgs::CmdFlag NewFlag>
consteval auto
CLArgs::ParserBuilder<CLArgs::CmdFlagList<Flags...>, CLArgs::CmdOptionList<Options...>>::add_flag()
{
    return ParserBuilder<CmdFlagList<Flags..., NewFlag>, CmdOptionList<Options...>>{};
}

template <CLArgs::CmdFlag... Flags, CLArgs::CmdOption... Options>
template <CLArgs::CmdOption NewOption>
consteval auto
CLArgs::ParserBuilder<CLArgs::CmdFlagList<Flags...>, CLArgs::CmdOptionList<Options...>>::add_option()
{
    return ParserBuilder<CmdFlagList<Flags...>, CmdOptionList<Options..., NewOption>>{};
}

template <CLArgs::CmdFlag... Flags, CLArgs::CmdOption... Options>
constexpr auto
CLArgs::ParserBuilder<CLArgs::CmdFlagList<Flags...>, CLArgs::CmdOptionList<Options...>>::build()
{
    return Parser<CmdFlagList<Flags...>, CmdOptionList<Options...>>{};
}

#endif
