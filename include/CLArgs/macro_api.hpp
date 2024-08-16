#ifndef CLARGS_MACRO_API_HPP
#define CLARGS_MACRO_API_HPP

#include <string_view>

#define CLARGS_IDENTIFIER(str)                                                                                                             \
    static constexpr std::string_view identifier                                                                                           \
    {                                                                                                                                      \
        str                                                                                                                                \
    }

#define CLARGS_ALIAS(str)                                                                                                                  \
    static constexpr std::string_view alias                                                                                                \
    {                                                                                                                                      \
        str                                                                                                                                \
    }

#define CLARGS_DESCRIPTION(str)                                                                                                            \
    static constexpr std::string_view description                                                                                          \
    {                                                                                                                                      \
        str                                                                                                                                \
    }

#define CLARGS_REQUIRED(reqd)                                                                                                              \
    static constexpr bool required                                                                                                         \
    {                                                                                                                                      \
        reqd                                                                                                                               \
    }

#define CLARGS_VALUE_HINT(str)                                                                                                             \
    static constexpr std::string_view value_hint                                                                                           \
    {                                                                                                                                      \
        str                                                                                                                                \
    }

#define CLARGS_VALUE_TYPE(type) using ValueType = type

#endif
