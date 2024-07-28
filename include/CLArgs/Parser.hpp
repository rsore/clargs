#ifndef ARGUMENTPARSER_HPP
#define ARGUMENTPARSER_HPP

/**
 * TODO:
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

    template <CmdOption Option>
    constexpr std::size_t identifier_length();

    template <CmdOption Option, CmdOption... Rest>
    static constexpr std::size_t max_identifier_length();

    template <typename T>
    static T from_sv(std::string_view);

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
        [[nodiscard]] bool has_option() const noexcept;

        template <CmdOption Option, typename = std::enable_if<CmdHasValue<Option>>>
        [[nodiscard]] std::optional<typename Option::ValueType> get_option_value() const noexcept;

      private:
        template <CmdOption Option, CmdOption... Rest>
        void process_arg(std::vector<std::string_view>::iterator &);

        template <CmdOption Option, CmdOption... Rest>
        void validate_required_options() const;

        template <CmdOption Option, CmdOption... Rest>
        static constexpr void append_args_to_usage(std::stringstream &);

        template <CmdOption Option, CmdOption... Rest>
        static constexpr void append_option_descriptions_to_usage(std::stringstream &);

        std::string_view                              program_{ "program_name" };
        std::unordered_map<std::type_index, std::any> options_;

        std::vector<std::string_view> arguments_; // Used during parsing

        bool valid_{ false };

        static constexpr std::size_t max_identifier_length_{ max_identifier_length<Options...>() };
    };
} // namespace CLArgs

template <CLArgs::CmdOption... Options>
void
CLArgs::Parser<Options...>::Parser::parse(int argc, char **argv)
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
        process_arg<Options...>(front);
    }

    validate_required_options<Options...>();

    valid_ = true;
}

template <CLArgs::CmdOption... Options>
template <CLArgs::CmdOption Option, CLArgs::CmdOption... Rest>
void
CLArgs::Parser<Options...>::process_arg(std::vector<std::string_view>::iterator &iter)
{
    std::string_view arg = *iter;

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
            auto value_iter = iter + 1;

            if (value_iter == arguments_.end())
            {
                std::stringstream ss;
                ss << "Expected value for option '" << arg << '\'';
                throw std::invalid_argument(ss.str());
            }
            options_[type_index] = std::make_any<typename Option::ValueType>(from_sv<typename Option::ValueType>(*value_iter));
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
            ss << "Expected required option \"" << Option::identifier << '"';
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
    assert(valid_);

    return program_;
}

template <CLArgs::CmdOption... Options>
template <CLArgs::CmdOption Option>
bool
CLArgs::Parser<Options...>::has_option() const noexcept
{
    assert(valid_);

    return options_.contains(std::type_index(typeid(Option)));
}

template <CLArgs::CmdOption... Options>
template <CLArgs::CmdOption Option, typename>
std::optional<typename Option::ValueType>
CLArgs::Parser<Options...>::get_option_value() const noexcept
{
    assert(valid_);

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
        std::stringstream ss;
        ss << "Error performing any_cast in CLargs::Parser::get_option_value() with" << '\n';
        ss << "  Option: \"" << typeid(Option).name() << "\"\n";
        ss << "  ValueType: \"" << typeid(typename Option::ValueType).name() << "\"\n";
        ss << "Returning nullopt as fallback";
        std::cerr << ss.str() << std::endl;
        return std::nullopt;
    }
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
