#ifndef CLARGS_VALUE_CONTAINER_HPP
#define CLARGS_VALUE_CONTAINER_HPP

#include <CLArgs/core.hpp>

#include <cstdint>
#include <optional>
#include <tuple>
#include <variant>

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


    template <Parsable... Parsables>
    class ValueContainer
    {
    public:
        ValueContainer();

        template <Parsable T>
        void set_value(const typename T::ValueType &);

        template <Parsable T>
        [[nodiscard]] const std::optional<typename T::ValueType> &get_value() const;

    private:
        template <Parsable T>
        static consteval std::size_t index_of_type();

        std::array<UniquelyFilteredVariant<std::optional<typename Parsables::ValueType>...>, sizeof...(Parsables)> values_;
    };
} // namespace CLArgs

template <CLArgs::Parsable... Parsables>
CLArgs::ValueContainer<Parsables...>::ValueContainer()
{
    ((values_[index_of_type<Parsables>()] = std::optional<typename Parsables::ValueType>{std::nullopt}), ...);
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
template <CLArgs::Parsable T>
consteval std::size_t
CLArgs::ValueContainer<Parsables...>::index_of_type()
{
    return TupleTypeIndex<T, std::tuple<Parsables...>>::value;
}

#endif
