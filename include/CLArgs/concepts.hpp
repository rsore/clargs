#ifndef CLARGS_CONCEPTS_HPP
#define CLARGS_CONCEPTS_HPP

#include <any>
#include <concepts>
#include <string_view>
#include <typeindex>
#include <unordered_map>
#include <utility>

namespace CLArgs
{
    template <typename T, typename... Ts>
    concept IsPartOf = (std::is_same_v<T, Ts> || ...);

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

    template <CLArgs::CmdOption... Opts>
    using Options = std::tuple<Opts...>;

    template <typename T>
    concept CmdGroup = requires
    {
        typename T::Options;
        typename T::Validator;

        {
            T::name
        } -> std::convertible_to<std::string_view>;
    };

    template <typename T, typename TupleLike>
    concept CmdGroupValidator = requires(T t, std::unordered_map<std::type_index, std::any> options)
    {
        {
            T::description
        } -> std::convertible_to<std::string_view>;

        {
            T::template validate<TupleLike>(options)
        } -> std::convertible_to<bool>;
    };

} // namespace CLArgs

#endif
