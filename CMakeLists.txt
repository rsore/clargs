cmake_minimum_required(VERSION 3.20)

project(CLArgs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(CLArgs INTERFACE
        include/CLArgs/parser.hpp
        include/CLArgs/core.hpp
        include/CLArgs/parse_value.hpp
        include/CLArgs/value_container.hpp
        include/CLArgs/parser_builder.hpp
        include/CLArgs/common_flags.hpp
        include/CLArgs/common_options.hpp
)

target_include_directories(CLArgs INTERFACE
        include/
)

option(CLARGS_BUILD_EXAMPLES "Build CLArgs examples" ON)
option(CLARGS_BUILD_TESTS "Build CLArgs tests" ON)

if (${CLARGS_BUILD_EXAMPLES})
    add_subdirectory(examples)
endif ()

if (${CLARGS_BUILD_TESTS})
    add_subdirectory(tests/)
endif ()
