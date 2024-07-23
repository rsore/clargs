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

    // [[nodiscard]] std::string usage() const;

    [[nodiscard]] std::filesystem::path program() const;

    template <CmdFlag Flag>
    [[nodiscard]] constexpr bool has_flag() const;

    template <CmdOption Option>
    [[nodiscard]] constexpr bool has_option() const;

    template <CmdOption Option>
    [[nodiscard]] constexpr typename Option::ValueType get_option();

    // template <CmdFlag Flag>
    // [[nodiscard]] bool has_flag() const;

    // template <CmdOption Option>
    // [[nodiscard]] bool has_option() const;

    // template <CmdOption Option>
    // [[nodiscard]] std::optional<typename Option::ValueType> option_value() const;

    void debug_print() const;

private:
    template <CmdArgument Arg, CmdArgument... Rest>
    void parse_arguments(int &, char **&);

    // constexpr void validate_required_arguments() const;

    // template <CmdArgument Arg, CmdArgument... Rest>
    // constexpr void validate_required_arguments() const;

    // constexpr void process_arguments();

    // template <CmdArgument Arg>
    // void process_argument(std::string_view, bool &);

    // [[nodiscard]] constexpr std::size_t length_of_longest_argument() const;

    // template <CmdArgument Arg, CmdArgument... Rest>
    // [[nodiscard]] constexpr std::size_t length_of_argument() const;

    // template <CmdArgument Arg, CmdArgument... Rest>
    // void append_arg_usage_to_stringstream(std::stringstream &) const;

    // [[nodiscard]] constexpr std::uint32_t calculate_max_number_of_arguments() const;

    // template <CmdArgument Arg, CmdArgument... Rest>
    // constexpr void calculate_max_number_of_arguments(std::uint32_t &) const;

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

// template <CmdArgument ... Args>
// template <CmdOption Option>
// constexpr std::optional<const typename Option::TypeValue &>
// ArgumentParser<Args...>::get_option()
// {
//    std::unordered_map<std::type_index, std::string_view>::iterator it;
//     try
//     {
//         it = options_.at(std::type_index(typeid(Option)));
//     }
//     catch ([[maybe_unused]] const std::out_of_range &e)
//     {
//         return std::nullopt;
//     }
//     return *it;
// }

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


// template <Util::CmdArgument... Args>
// void
// Util::ArgumentParser<Args...>::parse(const int argc, char **argv)
// {
//     if (argc <= 0)
//     {
//         throw std::runtime_error("argc is invalid, must be at least 1");
//     }
//
//     const std::uint32_t max_args = calculate_max_number_of_arguments();
//     if (static_cast<std::uint32_t>(argc) > max_args + 1)
//     {
//         std::stringstream ss;
//         ss << "Expected at most " << max_args << " arguments, got " << argc - 1;
//         throw std::runtime_error(ss.str());
//     }
//
//     if (argv == nullptr)
//     {
//         throw std::runtime_error("argv cannot be nullptr");
//     }
//
//     program_ = *argv++;
//     args_.resize(argc - 1);
//     for (auto &arg : args_)
//     {
//         arg = *argv++;
//     }
//
//     validate_required_arguments();
//     process_arguments();
//
//     if (expecting_option_ != nullptr)
//     {
//         std::cerr << "Expected argument for last provided option" << std::endl;
//     }
// }
//
// template <Util::CmdArgument... Args>
// std::string
// Util::ArgumentParser<Args...>::usage() const
// {
//     std::stringstream ss;
//     ss << "usage: " << program_ << " [OPTIONS]" << std::endl;
//     ss << "Options:" << std::endl;
//     append_arg_usage_to_stringstream<Args...>(ss);
//
//     return ss.str();
// }
//
// template <Util::CmdArgument... Args>
// std::filesystem::path
// Util::ArgumentParser<Args...>::program() const
// {
//     return std::filesystem::path(program_);
// }
//
// template <Util::CmdArgument... Args>
// template <Util::CmdFlag Flag>
// bool
// Util::ArgumentParser<Args...>::has_flag() const
// {
//     return flags_.contains(std::type_index(typeid(Flag)));
// }
//
// template <Util::CmdArgument... Args>
// template <Util::CmdOption Option>
// bool
// Util::ArgumentParser<Args...>::has_option() const
// {
//     return options_.contains(std::type_index(typeid(Option)));
// }
//
// template <Util::CmdArgument... Args>
// template <Util::CmdOption Option>
// std::optional<typename Option::ValueType>
// Util::ArgumentParser<Args...>::option_value() const
// {
//     if (!has_option<Option>())
//     {
//         return std::nullopt;
//     }
//
//     const auto value = options_.at(std::type_index(typeid(Option)));
//     typename Option::ValueType result;
//     const bool success = parse_value<Option::ValueType>(value, result);
//     if (!success)
//     {
//         std::cerr << "Unable to parse option '" << Option::option << "' with value '" << value << "'" << std::endl;
//         return std::nullopt;
//     }
//
//     return result;
// }
//
// template <Util::CmdArgument... Args>
// constexpr void
// Util::ArgumentParser<Args...>::validate_required_arguments() const
// {
//     validate_required_arguments<Args...>();
// }
//
// template <Util::CmdArgument... Args>
// template <Util::CmdArgument Arg, Util::CmdArgument... Rest>
// constexpr void
// Util::ArgumentParser<Args...>::validate_required_arguments() const
// {
//     if constexpr (Arg::required)
//     {
//         if constexpr (CmdFlag<Arg>)
//         {
//             if (std::find(args_.begin(), args_.end(), Arg::flag) == args_.end())
//             {
//                 std::cerr << "Flag '" << Arg::flag << "' required, but was not given" << std::endl;
//             }
//         }
//         else if constexpr (CmdOption<Arg>)
//         {
//             if (std::find(args_.begin(), args_.end(), Arg::option) == args_.end())
//             {
//                 std::cerr << "Option '" << Arg::option << "' required, but was not given" << std::endl;
//             }
//         }
//     }
//
//     if constexpr (sizeof...(Rest) > 0)
//     {
//         validate_required_arguments<Rest...>();
//     }
// }
//
// template <Util::CmdArgument... Args>
// constexpr void
// Util::ArgumentParser<Args...>::process_arguments()
// {
//     for (const auto &arg : args_)
//     {
//         bool known_argument = false;
//         ((process_argument<Args>(arg, known_argument)), ...);
//
//         if (!known_argument)
//         {
//             std::cerr << "Unknown argument '" << arg << "'" << std::endl;
//         }
//     }
// }
//
// template <Util::CmdArgument... Args>
// template <Util::CmdArgument Arg>
// void
// Util::ArgumentParser<Args...>::process_argument(std::string_view argument, bool &known_argument)
// {
//     std::cout << "Processing" << std::endl;
//     if (expecting_option_ && expecting_option_ == &typeid(Arg))
//     {
//         options_[std::type_index(typeid(Arg))] = argument;
//         expecting_option_                      = nullptr;
//         known_argument                         = true;
//     }
//     else if constexpr (CmdFlag<Arg>)
//     {
//         if (argument == Arg::flag)
//         {
//             flags_.insert(std::type_index(typeid(Arg)));
//             known_argument = true;
//         }
//     }
//     else if constexpr (CmdOption<Arg>)
//     {
//         if (argument == Arg::option)
//         {
//             expecting_option_ = &typeid(Arg);
//             known_argument    = true;
//         }
//     }
// }
//
// template <Util::CmdArgument... Args>
// constexpr std::size_t
// Util::ArgumentParser<Args...>::length_of_longest_argument() const
// {
//     return length_of_argument<Args...>();
// }
//
// template <Util::CmdArgument... Args>
// template <Util::CmdArgument Arg, Util::CmdArgument... Rest>
// constexpr std::size_t
// Util::ArgumentParser<Args...>::length_of_argument() const
// {
//     std::size_t length{};
//     if constexpr (CmdFlag<Arg>)
//     {
//         length = Arg::flag.length();
//     }
//     else if constexpr (CmdOption<Arg>)
//     {
//         length = Arg::option.length();
//     }
//
//     if constexpr (sizeof...(Rest) > 0)
//     {
//         length = std::max(length, length_of_argument<Rest...>());
//     }
//
//     return length;
// }
//
// template <Util::CmdArgument... Args>
// template <Util::CmdArgument Arg, Util::CmdArgument... Rest>
// void
// Util::ArgumentParser<Args...>::append_arg_usage_to_stringstream(std::stringstream &ss) const
// {
//     constexpr std::uint32_t padding = 4;
//     const std::size_t       spacing = length_of_longest_argument();
//     std::size_t             length{};
//
//     ss << "  ";
//     if constexpr (CmdFlag<Arg>)
//     {
//         ss << Arg::flag;
//         length = Arg::flag.length();
//     }
//     else if constexpr (CmdOption<Arg>)
//     {
//         ss << Arg::option;
//         length = Arg::option.length();
//     }
//     else
//     {
//         ss << "UNKNOWN TYPE";
//     }
//
//     ss << std::setw(spacing - length + padding) << "" << Arg::description << std::endl;
//
//     if constexpr (sizeof...(Rest) > 0)
//     {
//         append_arg_usage_to_stringstream<Rest...>(ss);
//     }
// }
//
// template <Util::CmdArgument... Args>
// constexpr std::uint32_t
// Util::ArgumentParser<Args...>::calculate_max_number_of_arguments() const
// {
//     std::uint32_t result{};
//     calculate_max_number_of_arguments<Args...>(result);
//     return result;
// }
//
// template <Util::CmdArgument... Args>
// template <Util::CmdArgument Arg, Util::CmdArgument... Rest>
// constexpr void
// Util::ArgumentParser<Args...>::calculate_max_number_of_arguments(std::uint32_t &result) const
// {
//     if constexpr (CmdFlag<Arg>)
//     {
//         result += 1;
//     }
//     else if constexpr (CmdOption<Arg>)
//     {
//         result += 2;
//     }
//
//     if constexpr (sizeof...(Rest) > 0)
//     {
//         calculate_max_number_of_arguments<Rest...>(result);
//     }
// }
//
// template <Util::CmdArgument... Args>
// void
// Util::ArgumentParser<Args...>::debug_print() const
// {
//     std::cout << "Debug print" << std::endl;
//
//     std::cout << "Program: " << program_ << std::endl;
//     std::cout << "Flags:" << std::endl;
//     for (const auto &flag : flags_)
//     {
//         std::cout << "  " << flag.name() << std::endl;
//     }
//
//     std::cout << "Options:" << std::endl;
//     for (const auto &[key, value] : options_)
//     {
//         std::cout << "  " << key.name() << ": " << value << std::endl;
//     }
// }

#endif