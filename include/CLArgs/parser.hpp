#ifndef CLARGS_PARSER_HPP
#define CLARGS_PARSER_HPP

#include <CLArgs/assert.hpp>
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
    template <Parsable... Parsables>
    class Parser
    {
    public:
         Parser() noexcept = default;
        ~Parser()          = default;

        Parser(const Parser &)  = delete;
        Parser(const Parser &&) = delete;

        Parser &operator=(const Parser &)  = delete;
        Parser &operator=(const Parser &&) = delete;

        void parse(int argc, char **argv);

        [[nodiscard]] std::string help() const noexcept;

        [[nodiscard]] std::string_view program() const noexcept;

        template <CmdFlag Flag>
        [[nodiscard]] bool has_flag() const noexcept
            requires IsPartOf<Flag, Parsables...>;

        template <CmdOption Option>
        [[nodiscard]] const std::optional<typename Option::ValueType> &get_option() const noexcept
            requires IsPartOf<Option, Parsables...>;

    private:
        void check_invariant() const;

        template <Parsable This, Parsable... Rest>
        void parse_arg(auto &remaining_args);

        template <Parsable This, Parsable... Rest>
        static constexpr void append_option_descriptions_to_usage(std::stringstream &);

        std::string_view             program_;
        ValueContainer<Parsables...> values_{};

        bool has_successfully_parsed_args_{false};

        static constexpr std::size_t max_identifier_length_{max_identifier_list_length<Parsables...>()};
    };
} // namespace CLArgs

template <CLArgs::Parsable... Parsables>
void
CLArgs::Parser<Parsables...>::parse(int argc, char **argv)
{
    if (argv == nullptr || *argv == nullptr)
    {
        throw std::invalid_argument("Passing nullptr to Parser::parse() is not allowed");
    }

    program_ = *argv++;
    argc -= 1;

    for (auto remaining_args = std::views::counted(argv, argc); !remaining_args.empty();)
    {
        parse_arg<Parsables...>(remaining_args);
    }

    has_successfully_parsed_args_ = true;
}

template <CLArgs::Parsable... Parsables>
template <CLArgs::Parsable This, CLArgs::Parsable... Rest>
void
CLArgs::Parser<Parsables...>::parse_arg(auto &remaining_args)
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

template <CLArgs::Parsable... Parsables>
std::string
CLArgs::Parser<Parsables...>::help() const noexcept
{
    std::stringstream ss;
    ss << "Usage: " << program_ << " [OPTIONS...]\n\n";
    ss << "Options:\n";
    append_option_descriptions_to_usage<Parsables...>(ss);

    return ss.str();
}

template <CLArgs::Parsable... Parsables>
std::string_view
CLArgs::Parser<Parsables...>::program() const noexcept
{
    check_invariant();
    return program_;
}

template <CLArgs::Parsable... Parsables>
template <CLArgs::CmdFlag Flag>
bool
CLArgs::Parser<Parsables...>::has_flag() const noexcept
    requires IsPartOf<Flag, Parsables...>
{
    check_invariant();
    const auto opt    = values_.template get_value<Flag>();
    const bool result = opt.has_value() && (opt.value() == true);
    return result;
}

template <CLArgs::Parsable... Parsables>
template <CLArgs::CmdOption Option>
const std::optional<typename Option::ValueType> &
CLArgs::Parser<Parsables...>::get_option() const noexcept
    requires IsPartOf<Option, Parsables...>
{
    check_invariant();
    return values_.template get_value<Option>();
}

template <CLArgs::Parsable... Parsables>
void
CLArgs::Parser<Parsables...>::check_invariant() const
{
    CLArgs::_internal::debug_assert(has_successfully_parsed_args_,
                                    "Have you called the parse() method yet? It must be called before any other method "
                                    "to ensure proper behaviour. If you don't, I will crash your application until you "
                                    "fix it.");
}

template <CLArgs::Parsable... Parsables>
template <CLArgs::Parsable This, CLArgs::Parsable... Rest>
constexpr void
CLArgs::Parser<Parsables...>::append_option_descriptions_to_usage(std::stringstream &ss)
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
