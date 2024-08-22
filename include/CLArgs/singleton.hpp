#ifndef CLARGS_SINGLETON_HPP
#define CLARGS_SINGLETON_HPP

#include <CLArgs/parser.hpp>

namespace CLArgs
{
    template <CmdOption... Options>
    class ParserSingleton
    {
        using ArgumentParser = Parser<Options...>;

    public:
        static void parse(int, char **);

        static std::string help() noexcept;

        static std::filesystem::path program() noexcept;

        template <CmdOption Option>
        static bool has_option() noexcept
            requires CLArgs::IsPartOf<Option, Options...>;

        template <CmdOption Option>
        std::optional<typename Option::ValueType> static get_option_value() noexcept
            requires CLArgs::IsPartOf<Option, Options...> && CLArgs::CmdHasValue<Option>;

    private:
        ParserSingleton() = default;

        static ArgumentParser &
        get_instance()
        {
            static ArgumentParser instance;
            return instance;
        }
    };
} // namespace CLArgs

template <CLArgs::CmdOption... Options>
void
CLArgs::ParserSingleton<Options...>::parse(int argc, char **argv)
{
    get_instance().parse(argc, argv);
}

template <CLArgs::CmdOption... Options>
std::string
CLArgs::ParserSingleton<Options...>::help() noexcept
{
    return get_instance().help();
}

template <CLArgs::CmdOption... Options>
std::filesystem::path
CLArgs::ParserSingleton<Options...>::program() noexcept
{
    return get_instance().program();
}

template <CLArgs::CmdOption... Options>
template <CLArgs::CmdOption Option>
bool
CLArgs::ParserSingleton<Options...>::has_option() noexcept
    requires CLArgs::IsPartOf<Option, Options...>
{
    return get_instance().template has_option<Option>();
}

template <CLArgs::CmdOption... Options>
template <CLArgs::CmdOption Option>
std::optional<typename Option::ValueType>
CLArgs::ParserSingleton<Options...>::get_option_value() noexcept
    requires CLArgs::IsPartOf<Option, Options...> && CLArgs::CmdHasValue<Option>
{
    return get_instance().template get_option_value<Option>();
}

#endif
