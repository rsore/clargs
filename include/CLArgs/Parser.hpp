#ifndef ARGUMENTPARSER_HPP
#define ARGUMENTPARSER_HPP

/**
 * TODO:
 * - Handle required arguments: If they are not passed by the user, issue an error and exit
 * - Handle option groups with validators (For example mutually exclusive options)
 * - Proper from_sv() implementation. We currently just use std::stringstream as a middleman for casting, but it is
 * expensive
 */

#include <any>
#include <cassert>
#include <cstddef>
#include <filesystem>
#include <iomanip>
#include <optional>
#include <sstream>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace CLArgs
{
    template <typename T>
    concept CmdOption = requires
    {
        {
            T::identifier
        } -> std::convertible_to<std::string_view>;
        {
            T::description
        } -> std::convertible_to<std::string_view>;
        {
            T::required
        } -> std::convertible_to<bool>;
    };

    template <typename T>
    concept CmdHasValue = requires
    {
        {
            T::value_hint
        } -> std::convertible_to<std::string_view>;
        typename T::ValueType;
    };

    template <typename T>
    concept CmdHasAlias = requires
    {
        {
            T::alias
        } -> std::convertible_to<std::string_view>;
    };

    template <CmdOption Arg>
    constexpr std::size_t identifier_length();

    template <CmdOption Arg, CmdOption... Rest>
    static constexpr std::size_t max_identifier_length();

    template <typename T>
    static T from_sv(std::string_view);

    template <CmdOption... Args>
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
        [[nodiscard]] bool has_option() const noexcept;

        template <CmdOption Option, typename = std::enable_if<CmdHasValue<Option>>>
        [[nodiscard]] std::optional<typename Option::ValueType> get_option_value() const noexcept;

      private:
        template <CmdOption Arg, CmdOption... Rest>
        void process_arg(std::vector<std::string_view>::iterator &);

        template <CmdOption Arg, CmdOption... Rest>
        static constexpr void append_args_to_usage(std::stringstream &);

        template <CmdOption Arg, CmdOption... Rest>
        static constexpr void append_option_descriptions_to_usage(std::stringstream &);

        std::string_view              program_{ "program_name" };
        std::vector<std::string_view> arguments_;

        std::unordered_map<std::type_index, std::any> options_;

        bool valid_{ false };

        static constexpr std::size_t max_identifier_length_{ max_identifier_length<Args...>() };
    };
} // namespace CLArgs

template <CLArgs::CmdOption... Args>
void
CLArgs::Parser<Args...>::Parser::parse(int argc, char **argv)
{
    program_ = *argv++;
    argc -= 1;

    arguments_.resize(argc);
    for (int i{}; i < argc; ++i)
    {
        arguments_[i] = argv[i];
    }

    while (!arguments_.empty())
    {
        auto front = arguments_.begin();
        process_arg<Args...>(front);
    }

    valid_ = true;
}

template <CLArgs::CmdOption... Args>
template <CLArgs::CmdOption Arg, CLArgs::CmdOption... Rest>
void
CLArgs::Parser<Args...>::process_arg(std::vector<std::string_view>::iterator &iter)
{
    std::string_view arg = *iter;

    bool found{ false };
    if (arg == Arg::identifier)
    {
        found = true;
    }
    else if constexpr (CmdHasAlias<Arg>)
    {
        if (arg == Arg::alias)
        {
            found = true;
        }
    }

    if (found)
    {
        const auto type_index = std::type_index(typeid(Arg));
        if (options_.contains(type_index))
        {
            std::stringstream ss;
            ss << "Duplicate argument \"" << Arg::identifier;
            if constexpr (CmdHasAlias<Arg>)
            {
                ss << " / " << Arg::alias;
            }
            ss << "\"";
            throw std::invalid_argument(ss.str());
        }

        if constexpr (CmdHasValue<Arg>)
        {
            auto value_iter = iter + 1;

            if (value_iter == arguments_.end())
            {
                std::stringstream ss;
                ss << "Expected value for option '" << arg << '\'';
                throw std::invalid_argument(ss.str());
            }
            options_[type_index] = std::make_any<Arg::ValueType>(from_sv<Arg::ValueType>(*value_iter));
            arguments_.erase(iter, value_iter + 1);
        }
        else
        {
            options_[type_index] = std::any{};
            arguments_.erase(iter);
        }
    }
    else
    {
        if constexpr (sizeof...(Rest) > 0)
        {
            process_arg<Rest...>(iter);
        }
        else
        {
            std::stringstream ss;
            ss << "Unknown option '" << arg << '\'';
            throw std::invalid_argument(ss.str());
        }
    }
}

template <CLArgs::CmdOption... Args>
std::string
CLArgs::Parser<Args...>::help() const noexcept
{
    std::stringstream ss;
    ss << "Usage: " << program_;
    append_args_to_usage<Args...>(ss);
    ss << '\n';

    ss << "Options:\n";
    append_option_descriptions_to_usage<Args...>(ss);

    return ss.str();
}

template <CLArgs::CmdOption... Args>
std::filesystem::path
CLArgs::Parser<Args...>::program() const noexcept
{
    assert(valid_);

    return program_;
}

template <CLArgs::CmdOption... Args>
template <CLArgs::CmdOption Option>
bool
CLArgs::Parser<Args...>::has_option() const noexcept
{
    assert(valid_);

    return options_.contains(std::type_index(typeid(Option)));
}

template <CLArgs::CmdOption... Args>
template <CLArgs::CmdOption Option, typename>
std::optional<typename Option::ValueType>
CLArgs::Parser<Args...>::get_option_value() const noexcept
{
    assert(valid_);

    const auto it = options_.find(std::type_index(typeid(Option)));
    if (it == options_.end())
    {
        return std::nullopt;
    }

    try
    {
        const auto result = std::any_cast<Option::ValueType>(it->second);
        return result;
    }
    catch (std::bad_any_cast &)
    {
        std::stringstream ss;
        ss << "Error performing any_cast in CLargs::Parser::get_option_value() with" << '\n';
        ss << "  Option: \"" << typeid(Option).name() << "\"\n";
        ss << "  ValueType: \"" << typeid(Option::ValueType).name() << "\"\n";
        ss << "Returning nullopt as fallback";
        std::cerr << ss.str() << std::endl;
        return std::nullopt;
    }
}

template <CLArgs::CmdOption... Args>
template <CLArgs::CmdOption Arg, CLArgs::CmdOption... Rest>
constexpr void
CLArgs::Parser<Args...>::append_args_to_usage(std::stringstream &ss)
{
    ss << ' ';

    constexpr bool required = Arg::required;
    if constexpr (!required)
    {
        ss << '[';
    }

    ss << Arg::identifier;

    if constexpr (CmdHasValue<Arg>)
    {
        ss << ' ' << Arg::value_hint;
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

template <CLArgs::CmdOption... Args>
template <CLArgs::CmdOption Arg, CLArgs::CmdOption... Rest>
constexpr void
CLArgs::Parser<Args...>::append_option_descriptions_to_usage(std::stringstream &ss)
{
    constexpr std::size_t calculated_padding = max_identifier_length_ - identifier_length<Arg>() + 4;

    ss << std::setw(2) << "" << Arg::identifier;
    if constexpr (CmdHasAlias<Arg>)
    {
        ss << ", " << Arg::alias;
    }

    ss << std::setw(calculated_padding) << "";

    constexpr std::string_view required_str = "REQUIRED";
    if constexpr (Arg::required)
    {
        ss << required_str;
    }
    else
    {
        ss << std::setw(required_str.length()) << "";
    }

    ss << "  " << Arg::description << '\n';

    if constexpr (sizeof...(Rest) > 0)
    {
        append_option_descriptions_to_usage<Rest...>(ss);
    }
}

template <CLArgs::CmdOption Arg>
constexpr std::size_t
CLArgs::identifier_length()
{
    std::size_t length = Arg::identifier.length();
    if constexpr (CmdHasAlias<Arg>)
    {
        constexpr std::size_t alias_length = Arg::alias.length();
        length += alias_length + 2; // + 2 to account for ", " between identifier and alias
    }

    return length;
}

template <CLArgs::CmdOption Arg, CLArgs::CmdOption... Rest>
constexpr std::size_t
CLArgs::max_identifier_length()
{
    std::size_t max_length = identifier_length<Arg>();

    if constexpr (sizeof...(Rest) > 0)
    {
        max_length = std::max(max_length, max_identifier_length<Rest...>());
    }

    return max_length;
}

template <typename T>
static T
CLArgs::from_sv(std::string_view sv)
{
    T                 result;
    std::stringstream ss;
    ss << sv;
    ss >> result;

    return result;
}

#endif
