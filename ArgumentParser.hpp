#ifndef ARGUMENTPARSER_HPP
#define ARGUMENTPARSER_HPP

/**
 * TODO:
 * - Handle duplicate arguments, e.g. user passes '-v --verbose' or '-f --foo -f'
 * - Generate usage string. All flags and options should be listed, along with their aliases if applicable, description
 * and value type
 * - Look into possibility of avoiding heap allocations. We know the max amount of arguments at compile-time, so we
 * should be able to stack-allocate.
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

    template <CmdFlag Flag>
    [[nodiscard]] bool has_flag() const;

    template <CmdOption Option>
    [[nodiscard]] bool has_option() const;

    template <CmdOption Option>
    [[nodiscard]] std::optional<typename Option::ValueType> get_option();

    void debug_print() const;

  private:
    template <CmdArgument Arg, CmdArgument... Rest>
    void parse_arguments(int &, char **&);

    std::string_view                                      program_;
    std::unordered_set<std::type_index>                   flags_;
    std::unordered_map<std::type_index, std::string_view> options_;

    std::uint32_t debug_string_comparisons_{};
};

template <CmdArgument... Args>
ArgumentParser<Args...>::ArgumentParser(int argc, char **argv)
    : program_(*argv++)
{
    argc -= 1;
    parse_arguments<Args...>(argc, argv);
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
    return flags_.contains(std::type_index(typeid(Flag)));
}

template <CmdArgument... Args>
template <CmdOption Option>
inline bool
ArgumentParser<Args...>::has_option() const
{
    return options_.contains(std::type_index(typeid(Option)));
}

template <CmdArgument... Args>
template <CmdOption Option>
inline std::optional<typename Option::ValueType>
ArgumentParser<Args...>::get_option()
{
    std::string_view sv;
    try
    {
        sv = options_.at(std::type_index(typeid(Option)));
    }
    catch (std::out_of_range &)
    {
        return std::nullopt;
    }

    typename Option::ValueType result;
    std::stringstream          ss;
    ss << sv;
    ss >> result;

    return result;
}

template <CmdArgument... Args>
void
ArgumentParser<Args...>::debug_print() const
{
    std::cout << "----- DEBUG_PRINT -----" << std::endl;
    std::cout << "Total string comparisons: " << debug_string_comparisons_ << std::endl;

    std::cout << "Flags" << std::endl;
    for (const auto &flag : flags_)
    {
        std::cout << "  " << flag.name() << " (" << flag.hash_code() << ")" << std::endl;
    }
    std::cout << "Options" << std::endl;
    for (const auto &[key, value] : options_)
    {
        std::cout << "  " << key.name() << " (" << key.hash_code() << ")" << " = " << value << std::endl;
    }
}

template <CmdArgument... Args>
template <CmdArgument Arg, CmdArgument... Rest>
void
ArgumentParser<Args...>::parse_arguments(int &argc, char **&argv)
{
    static_assert(CmdFlag<Arg> || CmdOption<Arg>);

    for (int i{}; i < argc; ++i)
    {
        debug_string_comparisons_ += 1;

        bool match = argv[i] == Arg::identifier;
        if constexpr (CmdHasAlias<Arg>)
        {
            debug_string_comparisons_ += 1;
            match = match || (argv[i] == Arg::alias);
        }

        if (!match)
        {
            continue;
        }

        if constexpr (CmdOption<Arg>)
        {
            if (i < argc - 1)
            {
                options_[std::type_index(typeid(Arg))] = argv[i + 1];
            }
        }
        else if constexpr (CmdFlag<Arg>)
        {
            flags_.insert(std::type_index(typeid(Arg)));
        }

        break;
    }

    if constexpr (sizeof...(Rest) > 0)
    {
        parse_arguments<Rest...>(argc, argv);
    }
}

#endif