#ifndef ARGUMENTPARSER_HPP
#define ARGUMENTPARSER_HPP

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
        T::description
    } -> std::convertible_to<std::string_view>;
    {
        T::required
    } -> std::convertible_to<bool>;
};

template <typename T>
concept CmdFlag = CmdArgumentBase<T> && requires
{
    {
        T::flag
    } -> std::convertible_to<std::string_view>;
};

template <typename T>
concept CmdOption = CmdArgumentBase<T> && requires
{
    {
        T::option
    } -> std::convertible_to<std::string_view>;
    {
        T::value_hint
    } -> std::convertible_to<std::string_view>;
    typename T::ValueType;
};

template <typename T>
concept CmdArgument = CmdOption<T> || CmdFlag<T>;

template <CmdArgument... Args>
class ArgumentParser
{
public:
    ArgumentParser(int argc, char **argv);

    ArgumentParser(const ArgumentParser &) = delete;

    ArgumentParser(const ArgumentParser &&) = delete;

    ArgumentParser &operator=(const ArgumentParser &) = delete;

    ArgumentParser &operator=(const ArgumentParser &&) = delete;

    [[nodiscard]] std::filesystem::path program() const;

    template <CmdFlag Flag>
    [[nodiscard]] constexpr bool has_flag() const;

    template <CmdOption Option>
    [[nodiscard]] constexpr bool has_option() const;

    template <CmdOption Option>
    [[nodiscard]] constexpr typename Option::ValueType get_option();

    void debug_print() const;

private:
    template <CmdArgument Arg, CmdArgument... Rest>
    void parse_arguments(int &, char **&);

    std::string_view                                      program_;
    std::unordered_set<std::type_index>                   flags_;
    std::unordered_map<std::type_index, std::string_view> options_;

    std::uint32_t debug_string_comparisons_{};
};

template <CmdArgument ... Args>
ArgumentParser<Args...>::ArgumentParser(int argc, char **argv)
    : program_(*argv++)
{
    argc -= 1;
    parse_arguments<Args...>(argc, argv);
}

template <CmdArgument ... Args>
std::filesystem::path
ArgumentParser<Args...>::program() const
{
    return program_;
}

template <CmdArgument ... Args>
template <CmdFlag Flag>
constexpr bool
ArgumentParser<Args...>::has_flag() const
{
    return flags_.contains(std::type_index(typeid(Flag)));
}

template <CmdArgument ... Args>
template <CmdOption Option>
constexpr bool
ArgumentParser<Args...>::has_option() const
{
    return options_.contains(std::type_index(typeid(Option)));
}

template <CmdArgument ... Args>
template <CmdOption Option>
constexpr typename Option::ValueType
ArgumentParser<Args...>::get_option()
{
    assert(has_option<Option>());

    const auto                 str_value = options_[std::type_index(typeid(Option))];
    typename Option::ValueType result;
    std::stringstream          ss;
    ss << str_value;
    ss >> result;

    return result;
}

template <CmdArgument ... Args>
void
ArgumentParser<Args...>::debug_print() const
{
    std::cout << "----- DEBUG_PRINT -----" << std::endl;
    std::cout << "Total string comparisons: " << debug_string_comparisons_ << std::endl;

    std::cout << "Flags" << std::endl;
    for (const auto &flag : flags_)
    {
        std::cout << "  " << flag.name() << std::endl;
    }
    std::cout << "Options" << std::endl;
    for (const auto &[key, value] : options_)
    {
        std::cout << "  " << key.name() << " = " << value << std::endl;
    }
}

template <CmdArgument ... Args>
template <CmdArgument Arg, CmdArgument ... Rest>
void
ArgumentParser<Args...>::parse_arguments(int &argc, char **&argv)
{
    static_assert(CmdFlag<Arg> || CmdOption<Arg>);

    const auto look_for_arg = [&](const std::string_view str)
    {
        for (int i{}; i < argc; ++i)
        {
            debug_string_comparisons_ += 1;
            if (!strcmp(argv[i], str.data()) == 0)
            {
                continue;
            }
            if constexpr (CmdFlag<Arg>)
            {
                flags_.insert(std::type_index(typeid(Arg)));
            }
            else if constexpr (CmdOption<Arg>)
            {
                if (i < argc - 1)
                {
                    const char *value                      = argv[i + 1];
                    options_[std::type_index(typeid(Arg))] = value;
                }
            }
            return;
        }
    };

    if constexpr (CmdFlag<Arg>)
    {
        look_for_arg(Arg::flag);
    }
    else if constexpr (CmdOption<Arg>)
    {
        look_for_arg(Arg::option);
    }

    if constexpr (sizeof...(Rest) > 0)
    {
        parse_arguments<Rest...>(argc, argv);
    }
}

#endif