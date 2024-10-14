#ifndef CLARGS_VALUE_CONTAINER_HPP
#define CLARGS_VALUE_CONTAINER_HPP

#include <CLArgs/core.hpp>

#include <cstddef>
#include <optional>
#include <tuple>
#include <type_traits>
#include <variant>

namespace CLArgs
{
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

        using ValuesTuple = std::tuple<std::optional<typename Parsables::ValueType>...>;
        static_assert(AllUnique_v<Parsables...>, "Duplicate template parameter types is not allowed in ValueContainer");
        ValuesTuple values_;
    };
} // namespace CLArgs

template <CLArgs::Parsable... Parsables>
CLArgs::ValueContainer<Parsables...>::ValueContainer() : values_{std::make_tuple(std::optional<typename Parsables::ValueType>{}...)}
{
}

template <CLArgs::Parsable... Parsables>
template <CLArgs::Parsable T>
void
CLArgs::ValueContainer<Parsables...>::set_value(const typename T::ValueType &value)
{
    constexpr std::size_t index = index_of_type<T>();
    static_assert(std::is_same_v<std::tuple_element_t<index, ValuesTuple>, std::optional<typename T::ValueType>>);
    std::get<index>(values_) = std::optional<typename T::ValueType>{value};
}

template <CLArgs::Parsable... Parsables>
template <CLArgs::Parsable T>
const std::optional<typename T::ValueType> &
CLArgs::ValueContainer<Parsables...>::get_value() const
{
    constexpr std::size_t index = index_of_type<T>();
    static_assert(std::is_same_v<std::tuple_element_t<index, ValuesTuple>, std::optional<typename T::ValueType>>);
    return std::get<index>(values_);
}

template <CLArgs::Parsable... Parsables>
template <CLArgs::Parsable T>
consteval std::size_t
CLArgs::ValueContainer<Parsables...>::index_of_type()
{
    return TupleTypeIndex_v<T, std::tuple<Parsables...>>;
}

#endif
