#ifndef CLARGS_PARSER_BUILDER_HPP
#define CLARGS_PARSER_BUILDER_HPP

#include <CLArgs/core.hpp>
#include <CLArgs/parser.hpp>

namespace CLArgs
{
    template <typename Flags = CmdFlagList<>, typename Options = CmdOptionList<>, StringLiteral ProgramDescription = "">
    class ParserBuilder;

    template <CmdFlag... Flags, CmdOption... Options, StringLiteral ProgramDescription>
    class ParserBuilder<CmdFlagList<Flags...>, CmdOptionList<Options...>, ProgramDescription>
    {
    public:
        template <CmdFlag NewFlag>
        [[nodiscard]] consteval auto add_flag();

        template <CmdOption NewOption>
        [[nodiscard]] consteval auto add_option();

        template <StringLiteral NewProgramDescription>
        [[nodiscard]] consteval auto add_program_description();

        [[nodiscard]] constexpr auto build();
    };
} // namespace CLArgs

template <CLArgs::CmdFlag... Flags, CLArgs::CmdOption... Options, CLArgs::StringLiteral ProgramDescription>
template <CLArgs::CmdFlag NewFlag>
consteval auto
CLArgs::ParserBuilder<CLArgs::CmdFlagList<Flags...>, CLArgs::CmdOptionList<Options...>, ProgramDescription>::add_flag()
{
    static_assert(!is_part_of_v<NewFlag, Flags...>, "Flag has already been added to builder");
    return ParserBuilder<CmdFlagList<Flags..., NewFlag>, CmdOptionList<Options...>, ProgramDescription>{};
}

template <CLArgs::CmdFlag... Flags, CLArgs::CmdOption... Options, CLArgs::StringLiteral ProgramDescription>
template <CLArgs::CmdOption NewOption>
consteval auto
CLArgs::ParserBuilder<CLArgs::CmdFlagList<Flags...>, CLArgs::CmdOptionList<Options...>, ProgramDescription>::add_option()
{
    static_assert(!is_part_of_v<NewOption, Options...>, "Option has already been added to builder");
    return ParserBuilder<CmdFlagList<Flags...>, CmdOptionList<Options..., NewOption>, ProgramDescription>{};
}

template <CLArgs::CmdFlag... Flags, CLArgs::CmdOption... Options, CLArgs::StringLiteral ProgramDescription>
template <CLArgs::StringLiteral NewProgramDescription>
consteval auto
CLArgs::ParserBuilder<CLArgs::CmdFlagList<Flags...>, CLArgs::CmdOptionList<Options...>, ProgramDescription>::add_program_description()
{
    return ParserBuilder<CmdFlagList<Flags...>, CmdOptionList<Options...>, NewProgramDescription>{};
}

template <CLArgs::CmdFlag... Flags, CLArgs::CmdOption... Options, CLArgs::StringLiteral ProgramDescription>
constexpr auto
CLArgs::ParserBuilder<CLArgs::CmdFlagList<Flags...>, CLArgs::CmdOptionList<Options...>, ProgramDescription>::build()
{
    return Parser<CmdFlagList<Flags...>, CmdOptionList<Options...>, ProgramDescription>{};
}

#endif // CLARGS_PARSER_BUILDER_HPP
