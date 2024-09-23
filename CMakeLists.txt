cmake_minimum_required(VERSION 3.20)

project(CLArgs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(CLArgs INTERFACE
        include/CLArgs/parser.hpp
        include/CLArgs/core.hpp
        include/CLArgs/singleton.hpp
        include/CLArgs/assert.hpp
        include/CLArgs/platform.hpp
        include/CLArgs/parse_value.hpp
        include/CLArgs/value_container.hpp
)

target_include_directories(CLArgs INTERFACE
        include/
)

option(CLARGS_BUILD_EXAMPLES "Build CLArgs examples" ON)

if (${CLARGS_BUILD_EXAMPLES})
    add_subdirectory(examples)
endif ()

option(CLARGS_BUILD_TESTS "Build CLArgs tests" ON)

if (${CLARGS_BUILD_TESTS})
    add_subdirectory(tests/)
endif ()
