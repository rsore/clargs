#ifndef CLARGS_PARSER_HPP
#define CLARGS_PARSER_HPP

#include <CLArgs/assert.hpp>
#include <CLArgs/core.hpp>
#include <CLArgs/from_string.hpp>

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <tuple>
#include <variant>
#include <vector>

namespace CLArgs
{
    template <typename T, typename Variant>
    struct PrependTypeToVariant;

    template <typename T, typename... Ts>
    struct PrependTypeToVariant<T, std::variant<Ts...>>
    {
        using Type = std::variant<T, Ts...>;
    };

    template <typename... Ts>
    struct UniquelyFilteredVariantImpl;

    template <typename T, typename... Ts>
    struct UniquelyFilteredVariantImpl<T, Ts...>
    {
        using Type = std::conditional_t<IsPartOf<T, Ts...>,
                                        typename UniquelyFilteredVariantImpl<Ts...>::Type,
                                        typename PrependTypeToVariant<T, typename UniquelyFilteredVariantImpl<Ts...>::Type>::Type>;
    };

    template <>
    struct UniquelyFilteredVariantImpl<>
    {
        using Type = std::variant<>;
    };

    template <typename... Ts>
    using UniquelyFilteredVariant = typename UniquelyFilteredVariantImpl<Ts...>::Type;

    template <typename T, typename Tuple>
    struct TupleTypeIndex;

    template <typename T, typename... Types>
    struct TupleTypeIndex<T, std::tuple<T, Types...>>
    {
        static constexpr std::size_t value{0};
    };

    template <typename T, typename U, typename... Types>
    struct TupleTypeIndex<T, std::tuple<U, Types...>>
    {
        static constexpr std::size_t value = 1 + TupleTypeIndex<T, std::tuple<Types...>>::value;
    };

    template <typename T>
    concept Parsable = CmdFlag<T> || CmdOption<T>;

    template <Parsable Parsable>
    static consteval std::size_t identifier_length();

    template <Parsable This, Parsable... Rest>
    static consteval std::size_t max_identifier_length();

    template <Parsable... Parsables>
    class ValueContainer
    {
    public:
        ValueContainer()
        {
            ((values_[index_of_type<Parsables>()] = std::optional<typename Parsables::ValueType>{std::nullopt}), ...);
        }

        template <Parsable T>
        void set_value(const typename T::ValueType &);

        template <Parsable T>
        [[nodiscard]] const std::optional<typename T::ValueType> &get_value() const;

    private:
        template <Parsable T>
        static consteval std::size_t index_of_type();

        std::array<UniquelyFilteredVariant<std::optional<typename Parsables::ValueType>...>, sizeof...(Parsables)> values_;
    };

    template <Parsable... Parsables>
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
        [[nodiscard]] bool has_flag() const noexcept
            requires IsPartOf<Flag, Parsables...>;

        template <CmdOption Option>
        [[nodiscard]] const std::optional<typename Option::ValueType> &get_option() const noexcept
            requires IsPartOf<Option, Parsables...>;

    private:
        void check_invariant() const;

        template <Parsable This, Parsable... Rest>
        void process_arg(std::vector<std::string_view> &, std::vector<std::string_view>::iterator &);

        template <Parsable This, Parsable... Rest>
        static constexpr void append_option_descriptions_to_usage(std::stringstream &);

        std::string_view program_;

        ValueContainer<Parsables...> values_{};

        bool has_successfully_parsed_args_{false};

        static constexpr std::size_t max_identifier_length_{max_identifier_length<Parsables...>()};
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

    std::vector<std::string_view> arguments(argc);
    for (int i{}; i < argc; ++i)
    {
        arguments[i] = argv[i];
    }

    while (!arguments.empty())
    {
        auto front = arguments.begin();
        process_arg<Parsables...>(arguments, front);
    }

    has_successfully_parsed_args_ = true;
}

template <CLArgs::Parsable... Parsables>
template <CLArgs::Parsable This, CLArgs::Parsable... Rest>
void
CLArgs::Parser<Parsables...>::process_arg(std::vector<std::string_view> &all, std::vector<std::string_view>::iterator &current)
{
    std::string_view arg = *current;

    const bool found = std::ranges::find(This::identifiers, arg) != This::identifiers.end();
    if (found)
    {
        if (values_.template get_value<This>() != std::nullopt)
        {
            std::stringstream ss;
            ss << "Duplicate argument \"" << This::identifiers[0] << "\"";
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
                const auto value = from_string<typename This::ValueType>(*value_iter);
                values_.template set_value<This>(value);
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
            values_.template set_value<This>(true);
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
std::filesystem::path
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
template <CLArgs::Parsable T>
consteval std::size_t
CLArgs::ValueContainer<Parsables...>::index_of_type()
{
    return TupleTypeIndex<T, std::tuple<Parsables...>>::value;
}

template <CLArgs::Parsable... Parsables>
template <CLArgs::Parsable T>
void
CLArgs::ValueContainer<Parsables...>::set_value(const typename T::ValueType &value)
{
    constexpr std::size_t index = index_of_type<T>();
    values_[index]              = std::optional<typename T::ValueType>{value};
}

template <CLArgs::Parsable... Parsables>
template <CLArgs::Parsable T>
const std::optional<typename T::ValueType> &
CLArgs::ValueContainer<Parsables...>::get_value() const
{
    constexpr std::size_t index = index_of_type<T>();
    return std::get<std::optional<typename T::ValueType>>(values_[index]);
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
    constexpr std::size_t calculated_padding = max_identifier_length_ - identifier_length<This>() + 4;

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

template <CLArgs::Parsable Parsable>
consteval std::size_t
CLArgs::identifier_length()
{
    std::size_t length{};

    for (const auto identifier : Parsable::identifiers)
    {
        length += identifier.length();
    }
    length += (Parsable::identifiers.size() - 1) * 2; // Account for ", " between identifiers

    return length;
}

template <CLArgs::Parsable This, CLArgs::Parsable... Rest>
consteval std::size_t
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
