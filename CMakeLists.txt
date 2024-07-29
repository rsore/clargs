cmake_minimum_required(VERSION 3.20)

project(CLArgs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(CLArgs INTERFACE
        include/CLArgs/parser.hpp
        include/CLArgs/concepts.hpp
        include/CLArgs/singleton.hpp
        include/CLArgs/misc.hpp
        include/CLArgs/platform.hpp
        include/CLArgs/from_string.hpp
)

target_include_directories(CLArgs INTERFACE
        include/
)

option(CLARGS_BUILD_EXAMPLES "Build CLArgs examples" ON)

if (${CLARGS_BUILD_EXAMPLES})
    add_subdirectory(examples/hello)
    add_subdirectory(examples/basic)
endif ()
