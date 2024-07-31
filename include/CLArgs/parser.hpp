#ifndef CLARGS_PARSER_HPP
#define CLARGS_PARSER_HPP

/**
 * TODO:
 * - Handle option groups with validators (For example mutually exclusive options)
 * - Proper from_string() implementation. We currently just use std::stringstream as a middleman for casting, but it is
 * expensive
 */

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
    template <CmdOption Option>
    constexpr std::size_t identifier_length();

    template <CmdOption Option, CmdOption... Rest>
    static constexpr std::size_t max_identifier_length();

    template <CmdOption... Options>
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

        template <CmdOption Option>
        [[nodiscard]] bool has_option() const noexcept requires IsPartOf<Option, Options...>;

        template <CmdOption Option>
        [[nodiscard]] std::optional<typename Option::ValueType>
        get_option_value() const noexcept requires IsPartOf<Option, Options...> &&CmdHasValue<Option>;

      private:
        void check_invariant() const;

        template <CmdOption Option, CmdOption... Rest>
        void process_arg(std::vector<std::string_view> &, std::vector<std::string_view>::iterator &);

        template <CmdOption Option, CmdOption... Rest>
        void validate_required_options() const;

        template <CmdOption Option, CmdOption... Rest>
        static constexpr void append_args_to_usage(std::stringstream &);

        template <CmdOption Option, CmdOption... Rest>
        static constexpr void append_option_descriptions_to_usage(std::stringstream &);

        std::string_view program_;
        OptionMap        options_;

        bool has_successfully_parsed_args_{ false };

        static constexpr std::size_t max_identifier_length_{ max_identifier_length<Options...>() };
    };
} // namespace CLArgs

template <CLArgs::CmdOption... Options>
void
CLArgs::Parser<Options...>::parse(int argc, char **argv)
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
        process_arg<Options...>(arguments, front);
    }

    validate_required_options<Options...>();

    has_successfully_parsed_args_ = true;
}

template <CLArgs::CmdOption... Options>
template <CLArgs::CmdOption Option, CLArgs::CmdOption... Rest>
void
CLArgs::Parser<Options...>::process_arg(std::vector<std::string_view>           &all,
                                        std::vector<std::string_view>::iterator &current)
{
    std::string_view arg = *current;

    bool found{ false };
    if (arg == Option::identifier)
    {
        found = true;
    }
    else if constexpr (CmdHasAlias<Option>)
    {
        if (arg == Option::alias)
        {
            found = true;
        }
    }

    if (found)
    {
        const auto type_index = std::type_index(typeid(Option));
        if (options_.contains(type_index))
        {
            std::stringstream ss;
            ss << "Duplicate argument \"" << Option::identifier;
            if constexpr (CmdHasAlias<Option>)
            {
                ss << " / " << Option::alias;
            }
            ss << "\"";
            throw std::invalid_argument(ss.str());
        }

        if constexpr (CmdHasValue<Option>)
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
                const auto value     = from_string<typename Option::ValueType>(*value_iter);
                options_[type_index] = std::make_any<typename Option::ValueType>(value);
            }
            catch (std::exception &e)
            {
                std::stringstream ss;
                ss << "Failed to parse \"" << *value_iter << "\" as type " << typeid(typename Option::ValueType).name()
                   << " for option \"" << arg << "\": " << e.what();
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

template <CLArgs::CmdOption... Options>
template <CLArgs::CmdOption Option, CLArgs::CmdOption... Rest>
void
CLArgs::Parser<Options...>::validate_required_options() const
{
    if constexpr (Option::required)
    {
        if (!options_.contains(std::type_index(typeid(Option))))
        {
            std::stringstream ss;
            ss << "Expected required option \"" << Option::identifier << "\"";
            throw std::invalid_argument(ss.str());
        }
    }

    if constexpr (sizeof...(Rest) > 0)
    {
        validate_required_options<Rest...>();
    }
}

template <CLArgs::CmdOption... Options>
std::string
CLArgs::Parser<Options...>::help() const noexcept
{
    std::stringstream ss;
    ss << "Usage: " << program_;
    append_args_to_usage<Options...>(ss);
    ss << '\n';

    ss << "Options:\n";
    append_option_descriptions_to_usage<Options...>(ss);

    return ss.str();
}

template <CLArgs::CmdOption... Options>
std::filesystem::path
CLArgs::Parser<Options...>::program() const noexcept
{
    check_invariant();
    return program_;
}

template <CLArgs::CmdOption... Options>
template <CLArgs::CmdOption Option>
bool
CLArgs::Parser<Options...>::has_option() const noexcept requires IsPartOf<Option, Options...>
{
    check_invariant();
    return options_.contains(std::type_index(typeid(Option)));
}

template <CLArgs::CmdOption... Options>
template <CLArgs::CmdOption Option>
std::optional<typename Option::ValueType>
CLArgs::Parser<Options...>::get_option_value()
    const noexcept requires IsPartOf<Option, Options...> &&CmdHasValue<Option>
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

template <CLArgs::CmdOption... Options>
void
CLArgs::Parser<Options...>::check_invariant() const
{
    CLARGS_ASSERT(has_successfully_parsed_args_,
                  "Have you called the parse() method yet? It must be called before any other method "
                  "to ensure proper behaviour. If you don't, I will crash your application until you "
                  "fix it.");
}

template <CLArgs::CmdOption... Options>
template <CLArgs::CmdOption Option, CLArgs::CmdOption... Rest>
constexpr void
CLArgs::Parser<Options...>::append_args_to_usage(std::stringstream &ss)
{
    ss << ' ';

    constexpr bool required = Option::required;
    if constexpr (!required)
    {
        ss << '[';
    }

    ss << Option::identifier;

    if constexpr (CmdHasValue<Option>)
    {
        ss << ' ' << Option::value_hint;
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

template <CLArgs::CmdOption... Options>
template <CLArgs::CmdOption Option, CLArgs::CmdOption... Rest>
constexpr void
CLArgs::Parser<Options...>::append_option_descriptions_to_usage(std::stringstream &ss)
{
    constexpr std::size_t calculated_padding = max_identifier_length_ - identifier_length<Option>() + 4;

    ss << std::setw(2) << "" << Option::identifier;
    if constexpr (CmdHasAlias<Option>)
    {
        ss << ", " << Option::alias;
    }

    ss << std::setw(calculated_padding) << "";

    constexpr std::string_view required_str = "REQUIRED";
    if constexpr (Option::required)
    {
        ss << required_str;
    }
    else
    {
        ss << std::setw(required_str.length()) << "";
    }

    ss << "  " << Option::description << '\n';

    if constexpr (sizeof...(Rest) > 0)
    {
        append_option_descriptions_to_usage<Rest...>(ss);
    }
}

template <CLArgs::CmdOption Option>
constexpr std::size_t
CLArgs::identifier_length()
{
    std::size_t length = Option::identifier.length();
    if constexpr (CmdHasAlias<Option>)
    {
        constexpr std::size_t alias_length = Option::alias.length();
        length += alias_length + 2; // + 2 to account for ", " between identifier and alias
    }

    return length;
}

template <CLArgs::CmdOption Option, CLArgs::CmdOption... Rest>
constexpr std::size_t
CLArgs::max_identifier_length()
{
    std::size_t max_length = identifier_length<Option>();

    if constexpr (sizeof...(Rest) > 0)
    {
        max_length = std::max(max_length, max_identifier_length<Rest...>());
    }

    return max_length;
}

#endif
