#ifndef CLARGS_PARSER_HPP
#define CLARGS_PARSER_HPP

#include <CLArgs/core.hpp>
#include <CLArgs/parse_value.hpp>
#include <CLArgs/value_container.hpp>

#include <cstddef>
#include <optional>
#include <ranges>
#include <sstream>
#include <string_view>

namespace CLArgs
{
    template <typename Flags, typename Options, StringLiteral ProgramDescription>
    class Parser;

    template <CmdFlag... Flags, CmdOption... Options, StringLiteral ProgramDescription>
    class Parser<CmdFlagList<Flags...>, CmdOptionList<Options...>, ProgramDescription>
    {
    public:
        Parser() noexcept = default;
        ~Parser()         = default;

        Parser(const Parser &)  = delete;
        Parser(const Parser &&) = delete;

        Parser &operator=(const Parser &)  = delete;
        Parser &operator=(const Parser &&) = delete;

        void parse(int argc, char **argv);

        [[nodiscard]] std::string usage() const noexcept;
        [[nodiscard]] std::string help() const noexcept;

        [[nodiscard]] std::string_view program() const noexcept;

        template <CmdFlag Flag>
        [[nodiscard]] bool has_flag() const noexcept
            requires is_part_of_v<Flag, Flags...>;

        template <CmdOption Option>
        [[nodiscard]] const std::optional<typename Option::ValueType> &get_option() const noexcept
            requires is_part_of_v<Option, Options...>;

    private:
        template <Parsable This, Parsable... Rest>
        void parse_arg(auto &remaining_args);

        template <Parsable This, Parsable... Rest>
        static constexpr void append_option_descriptions_to_usage(std::stringstream &);

        std::string_view                     program_;
        ValueContainer<Flags..., Options...> values_{};

        static constexpr std::size_t max_identifier_length_{max_identifier_list_length<Flags..., Options...>()};
    };
} // namespace CLArgs

template <CLArgs::CmdFlag... Flags, CLArgs::CmdOption... Options, CLArgs::StringLiteral ProgramDescription>
void
CLArgs::Parser<CLArgs::CmdFlagList<Flags...>, CLArgs::CmdOptionList<Options...>, ProgramDescription>::parse(int argc, char **argv)
{
    if (argv == nullptr || *argv == nullptr)
    {
        throw std::invalid_argument("Passing nullptr to Parser::parse() is not allowed");
    }

    program_ = *argv++;
    argc -= 1;

    for (auto remaining_args = std::views::counted(argv, argc); !remaining_args.empty();)
    {
        parse_arg<Flags..., Options...>(remaining_args);
    }
}

template <CLArgs::CmdFlag... Flags, CLArgs::CmdOption... Options, CLArgs::StringLiteral ProgramDescription>
template <CLArgs::Parsable This, CLArgs::Parsable... Rest>
void
CLArgs::Parser<CLArgs::CmdFlagList<Flags...>, CLArgs::CmdOptionList<Options...>, ProgramDescription>::parse_arg(auto &remaining_args)
{
    const auto arg = remaining_args.front();

    if (std::ranges::find(This::identifiers, arg) != This::identifiers.end())
    {
        remaining_args = std::views::drop(remaining_args, 1);

        if (values_.template get_value<This>() != std::nullopt)
        {
            std::stringstream ss;
            ss << "Duplicate argument \"" << This::identifiers[0] << "\"";
            throw std::invalid_argument(ss.str());
        }

        // This assertion is here for future-proofing. If the definition of
        // Parsable is updated, this logic must also be updated
        static_assert(CmdFlag<This> || CmdOption<This>);

        if constexpr (CmdFlag<This>)
        {
            values_.template set_value<This>(true);
        }
        else if constexpr (CmdOption<This>)
        {
            if (remaining_args.empty())
            {
                std::stringstream ss;
                ss << "Expected value for option \"" << arg << "\"";
                throw std::invalid_argument(ss.str());
            }

            const auto value_arg = remaining_args.front();
            remaining_args       = std::views::drop(remaining_args, 1);

            try
            {
                const auto value = parse_value<typename This::ValueType>(value_arg);
                values_.template set_value<This>(value);
            }
            catch (std::exception &e)
            {
                std::stringstream ss;
                ss << "Failed to parse value for option \"" << arg << "\": " << e.what();
                throw std::invalid_argument(ss.str());
            }
        }

        return;
    }

    if constexpr (sizeof...(Rest) > 0)
    {
        parse_arg<Rest...>(remaining_args);
    }
    else
    {
        std::stringstream ss;
        ss << "Unknown option \"" << arg << "\"";
        throw std::invalid_argument(ss.str());
    }
}

template <CLArgs::CmdFlag... Flags, CLArgs::CmdOption... Options, CLArgs::StringLiteral ProgramDescription>
std::string
CLArgs::Parser<CLArgs::CmdFlagList<Flags...>, CLArgs::CmdOptionList<Options...>, ProgramDescription>::usage() const noexcept
{
    std::stringstream ss;
    ss << "Usage: " << program_;
    if constexpr (sizeof...(Flags) > 0 || sizeof...(Options) > 0)
    {
        ss << " [OPTIONS...]";
    }
    return ss.str();
}

template <CLArgs::CmdFlag... Flags, CLArgs::CmdOption... Options, CLArgs::StringLiteral ProgramDescription>
std::string
CLArgs::Parser<CLArgs::CmdFlagList<Flags...>, CLArgs::CmdOptionList<Options...>, ProgramDescription>::help() const noexcept
{
    std::stringstream ss;
    if constexpr (!std::string_view(ProgramDescription.value).empty())
    {
        ss << ProgramDescription.value << "\n\n";
    }
    ss << usage() << "\n\n";
    ss << "Options:\n";
    append_option_descriptions_to_usage<Flags..., Options...>(ss);

    return ss.str();
}

template <CLArgs::CmdFlag... Flags, CLArgs::CmdOption... Options, CLArgs::StringLiteral ProgramDescription>
std::string_view
CLArgs::Parser<CLArgs::CmdFlagList<Flags...>, CLArgs::CmdOptionList<Options...>, ProgramDescription>::program() const noexcept
{
    return program_;
}

template <CLArgs::CmdFlag... Flags, CLArgs::CmdOption... Options, CLArgs::StringLiteral ProgramDescription>
template <CLArgs::CmdFlag Flag>
bool
CLArgs::Parser<CLArgs::CmdFlagList<Flags...>, CLArgs::CmdOptionList<Options...>, ProgramDescription>::has_flag() const noexcept
    requires is_part_of_v<Flag, Flags...>
{
    const auto opt    = values_.template get_value<Flag>();
    const bool result = opt.has_value() && (opt.value() == true);
    return result;
}

template <CLArgs::CmdFlag... Flags, CLArgs::CmdOption... Options, CLArgs::StringLiteral ProgramDescription>
template <CLArgs::CmdOption Option>
const std::optional<typename Option::ValueType> &
CLArgs::Parser<CLArgs::CmdFlagList<Flags...>, CLArgs::CmdOptionList<Options...>, ProgramDescription>::get_option() const noexcept
    requires is_part_of_v<Option, Options...>
{
    return values_.template get_value<Option>();
}

template <CLArgs::CmdFlag... Flags, CLArgs::CmdOption... Options, CLArgs::StringLiteral ProgramDescription>
template <CLArgs::Parsable This, CLArgs::Parsable... Rest>
constexpr void
CLArgs::Parser<CLArgs::CmdFlagList<Flags...>, CLArgs::CmdOptionList<Options...>, ProgramDescription>::append_option_descriptions_to_usage(
    std::stringstream &ss)
{
    constexpr std::size_t calculated_padding = max_identifier_length_ - identifier_list_length<This>() + 4;

    ss << "  ";

    for (auto iter = This::identifiers.begin(); iter != This::identifiers.end(); ++iter)
    {
        ss << *iter;
        if (std::next(iter) != This::identifiers.end())
        {
            ss << ", ";
        }
    }

    ss << std::setw(calculated_padding) << "  " << This::description << '\n';

    if constexpr (sizeof...(Rest) > 0)
    {
        append_option_descriptions_to_usage<Rest...>(ss);
    }
}

#endif
