#ifndef ARGUMENTPARSER_HPP
#define ARGUMENTPARSER_HPP

/**
 * TODO:
 * - Handle duplicate arguments, e.g. user passes '-v --verbose' or '-f --foo -f'
 * - Handle required arguments: If they are not passed by the user, issue an error and exit
 */

#include <cstddef>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <unordered_map>
#include <optional>
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
    concept CmdHasValue = CmdOption<T> && requires
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

    template <CmdOption... Args>
    class Parser
    {
    public:
        Parser(int argc, char **argv);

        Parser(const Parser &) = delete;

        Parser(const Parser &&) = delete;

        Parser &operator=(const Parser &) = delete;

        Parser &operator=(const Parser &&) = delete;

        [[nodiscard]] std::string usage() const;

        [[nodiscard]] std::filesystem::path program() const;

        template <CmdOption Flag>
        [[nodiscard]] bool has_flag() const;

        template <CmdHasValue Option>
        [[nodiscard]] bool has_option() const;

        template <CmdHasValue Option>
        [[nodiscard]] std::optional<typename Option::ValueType> get_option();

    private:
        template <CmdOption Arg, CmdOption... Rest>
        static constexpr void append_args_to_usage(std::stringstream &);

        template <CmdOption Arg, CmdOption... Rest>
        static constexpr void append_option_descriptions_to_usage(std::stringstream &);

        std::string_view                                  program_;
        std::vector<std::string_view>                     arguments_;
        std::unordered_map<std::string_view, std::size_t> argument_index_map_;

        static constexpr std::size_t max_identifier_length_{ max_identifier_length<Args...>() };
    };
}

template <CLArgs::CmdOption... Args>
CLArgs::Parser<Args...>::Parser(int argc, char **argv)
    : program_(*argv++)
{
    argc -= 1;

    arguments_.resize(argc);
    for (std::size_t i{}; i < static_cast<std::size_t>(argc); ++i)
    {
        const std::string_view sv = argv[i];
        arguments_[i]             = sv;
        argument_index_map_[sv]   = i;
    }
}

template <CLArgs::CmdOption... Args>
std::string
CLArgs::Parser<Args...>::usage() const
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
CLArgs::Parser<Args...>::program() const
{
    return program_;
}

template <CLArgs::CmdOption... Args>
template <CLArgs::CmdOption Flag>
bool
CLArgs::Parser<Args...>::has_flag() const
{
    bool result = argument_index_map_.contains(Flag::identifier);
    if constexpr (CmdHasAlias<Flag>)
    {
        result = result || argument_index_map_.contains(Flag::alias);
    }
    return result;
}

template <CLArgs::CmdOption... Args>
template <CLArgs::CmdHasValue Option>
bool
CLArgs::Parser<Args...>::has_option() const
{
    bool result = argument_index_map_.contains(Option::identifier);
    if constexpr (CmdHasAlias<Option>)
    {
        result = result || argument_index_map_.contains(Option::alias);
    }
    return result;
}

template <CLArgs::CmdOption... Args>
template <CLArgs::CmdHasValue Option>
std::optional<typename Option::ValueType>
CLArgs::Parser<Args...>::get_option()
{
    auto it = argument_index_map_.find(Option::identifier);
    if (it == argument_index_map_.end())
    {
        if constexpr (CmdHasAlias<Option>)
        {
            it = argument_index_map_.find(Option::alias);
        }
        if (it == argument_index_map_.end())
        {
            return std::nullopt;
        }
    }

    const std::size_t index = it->second;
    if (index + 1 >= arguments_.size())
    {
        return std::nullopt;
    }

    const auto &               sv = arguments_[index + 1];
    typename Option::ValueType result;
    std::stringstream          ss;

    ss << sv;
    ss >> result;
    return result;
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

#endif
