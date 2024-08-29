#ifndef CLARGS_SINGLETON_HPP
#define CLARGS_SINGLETON_HPP

#include <CLArgs/parser.hpp>

namespace CLArgs
{
    template <Parseable... Parseables>
    class ParserSingleton
    {
        using ArgumentParser = Parser<Parseables...>;

    public:
        static void parse(int, char **);

        static std::string help() noexcept;

        static std::filesystem::path program() noexcept;

        template <CmdFlag Flag>
        static bool has_flag() noexcept
            requires CLArgs::IsPartOf<Flag, Parseables...>;

        template <CmdOption Option>
        std::optional<typename Option::ValueType> static get_option() noexcept
            requires CLArgs::IsPartOf<Option, Parseables...>;

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

template <CLArgs::Parseable... Parseables>
void
CLArgs::ParserSingleton<Parseables...>::parse(int argc, char **argv)
{
    get_instance().parse(argc, argv);
}

template <CLArgs::Parseable... Parseables>
std::string
CLArgs::ParserSingleton<Parseables...>::help() noexcept
{
    return get_instance().help();
}

template <CLArgs::Parseable... Parseables>
std::filesystem::path
CLArgs::ParserSingleton<Parseables...>::program() noexcept
{
    return get_instance().program();
}

template <CLArgs::Parseable... Parseables>
template <CLArgs::CmdFlag Flag>
bool
CLArgs::ParserSingleton<Parseables...>::has_flag() noexcept
    requires CLArgs::IsPartOf<Flag, Parseables...>
{
    return get_instance().template has_flag<Flag>();
}

template <CLArgs::Parseable... Parseables>
template <CLArgs::CmdOption Option>
std::optional<typename Option::ValueType>
CLArgs::ParserSingleton<Parseables...>::get_option() noexcept
    requires CLArgs::IsPartOf<Option, Parseables...>
{
    return get_instance().template get_option<Option>();
}

#endif
