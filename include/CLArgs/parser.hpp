#ifndef CLARGS_PARSER_HPP
#define CLARGS_PARSER_HPP

#include <CLArgs/concepts.hpp>
#include <CLArgs/from_string.hpp>
#include <CLArgs/misc.hpp>

#include <algorithm>
#include <any>
#include <cstddef>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace CLArgs
{
    template <Parseable Parseable>
    constexpr std::size_t identifier_length();

    template <Parseable This, Parseable... Rest>
    static constexpr std::size_t max_identifier_length();

    template <std::size_t N>
    struct StringLiteral
    {
        constexpr
        StringLiteral(const char (&str)[N]); // NOLINT (Marked NOLINT to suppress warning about non-explicit single-argument constructor

        char value[N]{};
    };

    template <const StringLiteral Identifier, const StringLiteral Alias, const StringLiteral Description, const bool Required>
    struct Flag
    {
        static constexpr std::string_view identifier{Identifier.value};
        static constexpr std::string_view alias{Alias.value};
        static constexpr std::string_view description{Description.value};
        static constexpr bool             required{Required};
    };

    template <const StringLiteral Identifier,
              const StringLiteral Alias,
              const StringLiteral ValueHint,
              const StringLiteral Description,
              const bool          Required,
              typename ValType>
    struct Option
    {
        static constexpr std::string_view identifier{Identifier.value};
        static constexpr std::string_view alias{Alias.value};
        static constexpr std::string_view value_hint{ValueHint.value};
        static constexpr std::string_view description{Description.value};
        static constexpr bool             required{Required};
        using ValueType = ValType;
    };

    template <Parseable... Parseables>
    class Parser
    {
    public:
        Parser() noexcept = default;

        Parser(const Parser &)  = delete;
        Parser(const Parser &&) = delete;

        Parser &operator=(const Parser &)  = delete;
        Parser &operator=(const Parser &&) = delete;

        void parse(int argc, char **argv);

        [[nodiscard]] std::string help() const noexcept;

        [[nodiscard]] std::filesystem::path program() const noexcept;

        template <CmdFlag Flag>
        [[nodiscard]] bool has_option() const noexcept
            requires IsPartOf<Flag, Parseables...>;

        template <CmdOption Option>
        [[nodiscard]] std::optional<typename Option::ValueType> get_option_value() const noexcept
            requires IsPartOf<Option, Parseables...>;

    private:
        void check_invariant() const;

        template <Parseable This, Parseable... Rest>
        void process_arg(std::vector<std::string_view> &, std::vector<std::string_view>::iterator &);

        template <Parseable This, Parseable... Rest>
        void validate_required_options() const;

        template <Parseable This, Parseable... Rest>
        static constexpr void append_args_to_usage(std::stringstream &);

        template <Parseable This, Parseable... Rest>
        static constexpr void append_option_descriptions_to_usage(std::stringstream &);

        std::string_view                              program_;
        std::unordered_map<std::type_index, std::any> options_;

        bool has_successfully_parsed_args_{false};

        static constexpr std::size_t max_identifier_length_{max_identifier_length<Parseables...>()};
    };
} // namespace CLArgs

template <CLArgs::Parseable... Parseables>
void
CLArgs::Parser<Parseables...>::parse(int argc, char **argv)
{
    if (argv == nullptr || *argv == nullptr)
    {
        throw std::invalid_argument("Passing nullptr to Parser::parse() is not allowed");
    }

    program_ = *argv++;
    argc -= 1;

    std::vector<std::string_view> arguments(argc);
    for (int i{}; i < argc; ++i)
    {
        arguments[i] = argv[i];
    }

    while (!arguments.empty())
    {
        auto front = arguments.begin();
        process_arg<Parseables...>(arguments, front);
    }

    validate_required_options<Parseables...>();

    has_successfully_parsed_args_ = true;
}

template <CLArgs::Parseable... Parseables>
template <CLArgs::Parseable This, CLArgs::Parseable... Rest>
void
CLArgs::Parser<Parseables...>::process_arg(std::vector<std::string_view> &all, std::vector<std::string_view>::iterator &current)
{
    std::string_view arg = *current;

    bool found{false};
    if (arg == This::identifier)
    {
        found = true;
    }
    else if constexpr (CmdHasAlias<This>)
    {
        if (arg == This::alias)
        {
            found = true;
        }
    }

    if (found)
    {
        const auto type_index = std::type_index(typeid(This));
        if (options_.contains(type_index))
        {
            std::stringstream ss;
            ss << "Duplicate argument \"" << This::identifier;
            if constexpr (CmdHasAlias<This>)
            {
                ss << " / " << This::alias;
            }
            ss << "\"";
            throw std::invalid_argument(ss.str());
        }

        if constexpr (CmdOption<This>)
        {
            auto value_iter = current + 1;

            if (value_iter == all.end())
            {
                std::stringstream ss;
                ss << "Expected value for option \"" << arg << "\"";
                throw std::invalid_argument(ss.str());
            }
            try
            {
                const auto value     = from_string<typename This::ValueType>(*value_iter);
                options_[type_index] = std::make_any<typename This::ValueType>(value);
            }
            catch (std::exception &e)
            {
                std::stringstream ss;
                ss << "Failed to parse \"" << *value_iter << "\" as type " << typeid(typename This::ValueType).name() << " for option \""
                   << arg << "\": " << e.what();
                throw std::invalid_argument(ss.str());
            }
            all.erase(current, value_iter + 1);
        }
        else
        {
            options_[type_index] = std::any{};
            all.erase(current);
        }
    }
    else
    {
        if constexpr (sizeof...(Rest) > 0)
        {
            process_arg<Rest...>(all, current);
        }
        else
        {
            std::stringstream ss;
            ss << "Unknown option \"" << arg << "\"";
            throw std::invalid_argument(ss.str());
        }
    }
}

template <CLArgs::Parseable... Parseables>
template <CLArgs::Parseable This, CLArgs::Parseable... Rest>
void
CLArgs::Parser<Parseables...>::validate_required_options() const
{
    if constexpr (This::required)
    {
        if (!options_.contains(std::type_index(typeid(This))))
        {
            std::stringstream ss;
            ss << "Expected required option \"" << This::identifier << "\"";
            throw std::invalid_argument(ss.str());
        }
    }

    if constexpr (sizeof...(Rest) > 0)
    {
        validate_required_options<Rest...>();
    }
}

template <CLArgs::Parseable... Parseables>
std::string
CLArgs::Parser<Parseables...>::help() const noexcept
{
    std::stringstream ss;
    ss << "Usage: " << program_;
    append_args_to_usage<Parseables...>(ss);
    ss << '\n';

    ss << "Options:\n";
    append_option_descriptions_to_usage<Parseables...>(ss);

    return ss.str();
}

template <CLArgs::Parseable... Parseables>
std::filesystem::path
CLArgs::Parser<Parseables...>::program() const noexcept
{
    check_invariant();
    return program_;
}

template <CLArgs::Parseable... Parseables>
template <CLArgs::CmdFlag Flag>
bool
CLArgs::Parser<Parseables...>::has_option() const noexcept
    requires IsPartOf<Flag, Parseables...>
{
    check_invariant();
    return options_.contains(std::type_index(typeid(Flag)));
}

template <CLArgs::Parseable... Parseables>
template <CLArgs::CmdOption Option>
std::optional<typename Option::ValueType>
CLArgs::Parser<Parseables...>::get_option_value() const noexcept
    requires IsPartOf<Option, Parseables...>
{
    check_invariant();

    const auto it = options_.find(std::type_index(typeid(Option)));
    if (it == options_.end())
    {
        return std::nullopt;
    }

    try
    {
        const auto result = std::any_cast<typename Option::ValueType>(it->second);
        return result;
    }
    catch (std::bad_any_cast &)
    {
        // This exception should never be thrown as long as the implementation is correct.
        // The type validation is already done before inserting the element.
        // If this error occurs, there is an error with the library implementation.
        std::cerr << "Error performing any_cast in CLargs::Parser::get_option_value() with" << '\n';
        std::cerr << "  Option: \"" << typeid(Option).name() << "\"\n";
        std::cerr << "  ValueType: \"" << typeid(typename Option::ValueType).name() << "\"\n";
        std::cerr << "Returning nullopt as fallback" << std::endl;
        return std::nullopt;
    }
}

template <CLArgs::Parseable... Parseables>
void
CLArgs::Parser<Parseables...>::check_invariant() const
{
    CLARGS_ASSERT(has_successfully_parsed_args_,
                  "Have you called the parse() method yet? It must be called before any other method "
                  "to ensure proper behaviour. If you don't, I will crash your application until you "
                  "fix it.");
}

template <CLArgs::Parseable... Parseables>
template <CLArgs::Parseable This, CLArgs::Parseable... Rest>
constexpr void
CLArgs::Parser<Parseables...>::append_args_to_usage(std::stringstream &ss)
{
    ss << ' ';

    constexpr bool required = This::required;
    if constexpr (!required)
    {
        ss << '[';
    }

    ss << This::identifier;

    if constexpr (CmdOption<This>)
    {
        ss << ' ' << This::value_hint;
    }

    if constexpr (!required)
    {
        ss << ']';
    }

    if constexpr (sizeof...(Rest) > 0)
    {
        append_args_to_usage<Rest...>(ss);
    }
}

template <CLArgs::Parseable... Parseables>
template <CLArgs::Parseable This, CLArgs::Parseable... Rest>
constexpr void
CLArgs::Parser<Parseables...>::append_option_descriptions_to_usage(std::stringstream &ss)
{
    constexpr std::size_t calculated_padding = max_identifier_length_ - identifier_length<This>() + 4;

    ss << std::setw(2) << "" << This::identifier;
    if constexpr (CmdHasAlias<This>)
    {
        ss << ", " << This::alias;
    }

    ss << std::setw(calculated_padding) << "";

    constexpr std::string_view required_str = "REQUIRED";
    if constexpr (This::required)
    {
        ss << required_str;
    }
    else
    {
        ss << std::setw(required_str.length()) << "";
    }

    ss << "  " << This::description << '\n';

    if constexpr (sizeof...(Rest) > 0)
    {
        append_option_descriptions_to_usage<Rest...>(ss);
    }
}

template <std::size_t N>
constexpr CLArgs::StringLiteral<N>::StringLiteral(const char (&str)[N])
{
    std::copy_n(str, N, value);
}

template <CLArgs::Parseable Parseable>
constexpr std::size_t
CLArgs::identifier_length()
{
    std::size_t length = Parseable::identifier.length();
    if constexpr (CmdHasAlias<Parseable>)
    {
        constexpr std::size_t alias_length = Parseable::alias.length();
        length += alias_length + 2; // + 2 to account for ", " between identifier and alias
    }

    return length;
}

template <CLArgs::Parseable This, CLArgs::Parseable... Rest>
constexpr std::size_t
CLArgs::max_identifier_length()
{
    std::size_t max_length = identifier_length<This>();

    if constexpr (sizeof...(Rest) > 0)
    {
        max_length = std::max(max_length, max_identifier_length<Rest...>());
    }

    return max_length;
}

#endif
