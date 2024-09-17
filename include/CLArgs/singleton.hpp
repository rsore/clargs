#ifndef CLARGS_SINGLETON_HPP
#define CLARGS_SINGLETON_HPP

#include <CLArgs/parser.hpp>

namespace CLArgs
{
    template <Parsable... Parsables>
    class ParserSingleton
    {
        using ArgumentParser = Parser<Parsables...>;

    public:
        static void parse(int, char **);

        static std::string help() noexcept;

        static std::filesystem::path program() noexcept;

        template <CmdFlag Flag>
        static bool has_flag() noexcept
            requires CLArgs::IsPartOf<Flag, Parsables...>;

        template <CmdOption Option>
        std::optional<typename Option::ValueType> static get_option() noexcept
            requires CLArgs::IsPartOf<Option, Parsables...>;

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

template <CLArgs::Parsable... Parsables>
void
CLArgs::ParserSingleton<Parsables...>::parse(int argc, char **argv)
{
    get_instance().parse(argc, argv);
}

template <CLArgs::Parsable... Parsables>
std::string
CLArgs::ParserSingleton<Parsables...>::help() noexcept
{
    return get_instance().help();
}

template <CLArgs::Parsable... Parsables>
std::filesystem::path
CLArgs::ParserSingleton<Parsables...>::program() noexcept
{
    return get_instance().program();
}

template <CLArgs::Parsable... Parsables>
template <CLArgs::CmdFlag Flag>
bool
CLArgs::ParserSingleton<Parsables...>::has_flag() noexcept
    requires CLArgs::IsPartOf<Flag, Parsables...>
{
    return get_instance().template has_flag<Flag>();
}

template <CLArgs::Parsable... Parsables>
template <CLArgs::CmdOption Option>
std::optional<typename Option::ValueType>
CLArgs::ParserSingleton<Parsables...>::get_option() noexcept
    requires CLArgs::IsPartOf<Option, Parsables...>
{
    return get_instance().template get_option<Option>();
}

#endif
