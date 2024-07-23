#ifndef ARGUMENTPARSER_HPP
#define ARGUMENTPARSER_HPP

/**
 * TODO:
 * - Handle duplicate arguments, e.g. user passes '-v --verbose' or '-f --foo -f'
 */

#include <cassert>
#include <cstdint>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>

template <typename T>
concept CmdArgumentBase = requires
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
concept CmdFlag = CmdArgumentBase<T>;

template <typename T>
concept CmdOption = CmdArgumentBase<T> && requires
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

template <typename T>
concept CmdArgument = CmdOption<T> || CmdFlag<T>;

template <CmdArgument... Args>
class ArgumentParser
{
  public:
    ArgumentParser(int argc, char **argv);

    ArgumentParser(const ArgumentParser &)  = delete;
    ArgumentParser(const ArgumentParser &&) = delete;

    ArgumentParser &operator=(const ArgumentParser &)  = delete;
    ArgumentParser &operator=(const ArgumentParser &&) = delete;

    [[nodiscard]] std::filesystem::path program() const;

    [[nodiscard]] std::string usage() const;

    template <CmdFlag Flag>
    [[nodiscard]] bool has_flag() const;

    template <CmdOption Option>
    [[nodiscard]] bool has_option() const;

    template <CmdOption Option>
    [[nodiscard]] std::optional<typename Option::ValueType> get_option();

  private:
    template <CmdArgument Arg>
    static consteval std::size_t identifier_length();

    template <CmdArgument Arg, CmdArgument... Rest>
    static constexpr std::size_t max_identifier_length();

    template <CmdArgument Arg, CmdArgument... Rest>
    static constexpr void append_args_to_usage(std::stringstream &);

    std::string_view                                  program_;
    std::vector<std::string_view>                     arguments_;
    std::unordered_map<std::string_view, std::size_t> argument_index_map_;

    static constexpr std::size_t max_identifier_length_{ max_identifier_length<Args...>() };
};

template <CmdArgument... Args>
std::string
ArgumentParser<Args...>::usage() const
{
    std::stringstream ss;
    ss << "Usage: " << program_ << " [OPTIONS]\n";
    ss << "Options:\n";

    append_args_to_usage<Args...>(ss);

    return ss.str();
}

template <CmdArgument... Args>
ArgumentParser<Args...>::ArgumentParser(int argc, char **argv)
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

template <CmdArgument... Args>
std::filesystem::path
ArgumentParser<Args...>::program() const
{
    return program_;
}

template <CmdArgument... Args>
template <CmdFlag Flag>
inline bool
ArgumentParser<Args...>::has_flag() const
{
    bool result = argument_index_map_.find(Flag::identifier) != argument_index_map_.end();
    if constexpr (CmdHasAlias<Flag>)
    {
        result = result || argument_index_map_.find(Flag::alias) != argument_index_map_.end();
    }
    return result;
}

template <CmdArgument... Args>
template <CmdOption Option>
inline bool
ArgumentParser<Args...>::has_option() const
{
    bool result = argument_index_map_.find(Option::identifier) != argument_index_map_.end();
    if constexpr (CmdHasAlias<Option>)
    {
        result = result || argument_index_map_.find(Option::alias) != argument_index_map_.end();
    }
    return result;
}

template <CmdArgument... Args>
template <CmdOption Option>
inline std::optional<typename Option::ValueType>
ArgumentParser<Args...>::get_option()
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
    // TODO: Is this correct? It looks like it would break when the last argument is a flag.
    if (index + 1 >= arguments_.size())
    {
        return std::nullopt;
    }

    const auto                &sv = arguments_[index + 1];
    typename Option::ValueType result;
    std::stringstream          ss;

    ss << sv;
    ss >> result;
    return result;
}

template <CmdArgument... Args>
template <CmdArgument Arg>
consteval std::size_t
          ArgumentParser<Args...>::identifier_length()
{
    std::size_t length = Arg::identifier.length();
    if constexpr (CmdHasAlias<Arg>)
    {
        constexpr std::size_t alias_length = Arg::alias.length();
        length += alias_length + 2; // + 2 to account for ", " between identifier and alias
    }

    return length;
}

template <CmdArgument... Args>
template <CmdArgument Arg, CmdArgument... Rest>
constexpr std::size_t
ArgumentParser<Args...>::max_identifier_length()
{
    std::size_t max_length = identifier_length<Arg>();

    if constexpr (sizeof...(Rest) > 0)
    {
        max_length = std::max(max_length, max_identifier_length<Rest...>());
    }

    return max_length;
}

template <CmdArgument... Args>
template <CmdArgument Arg, CmdArgument... Rest>
constexpr void
ArgumentParser<Args...>::append_args_to_usage(std::stringstream &ss)
{
    constexpr std::size_t calculated_padding = max_identifier_length_ - identifier_length<Arg>() + 4;

    ss << std::setw(2) << "" << Arg::identifier;
    if constexpr (CmdHasAlias<Arg>)
    {
        ss << ", " << Arg::alias;
    }

    ss << std::setw(calculated_padding) << "" << Arg::description << '\n';

    if constexpr (sizeof...(Rest) > 0)
    {
        append_args_to_usage<Rest...>(ss);
    }
}

#endif