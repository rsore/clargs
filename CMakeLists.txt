####################################################################################################
# CLArgs CMake Configuration
#
# This CMake configuration file is for the CLArgs library, which can be used
# either as a multi-header or a single-header library.
#
# Usage:
#
# - `CLArgs` is the primary interface target that users should link against.
#   It provides access to the CLArgs library API, either as:
#   - A multi-header version (`CLArgs_internal_source`) that includes individual header files.
#   - A single-header version (`CLArgs_internal_amalgamated`) that amalgamates all headers into one.
#
# - The `CLArgs_internal_source` and `CLArgs_internal_amalgamated` targets are **internal**
#   and should **not** be linked directly by users. These targets are used to generate the
#   corresponding build variants of the library. Instead, always use the `CLArgs` target,
#   which automatically selects the correct version based on the `CLARGS_SINGLE_HEADER`
#   configuration option.
#
# Configuration Options:
#
# - `CLARGS_SINGLE_HEADER` (default: ON): If enabled, the library will use a single-header
#   amalgamated version. If disabled, it will use the multi-header source version.
#
# - `CLARGS_BUILD_EXAMPLES` (default: OFF): Set to ON to build example applications using
#   the CLArgs library.
#
# - `CLARGS_BUILD_TESTS` (default: OFF): Set to ON to build unit tests for CLArgs.
#
# Example:
#
# - To use the CLArgs library in your project, simply link to the `CLArgs` target:
#
#     add_executable(my_exe main.cpp)
#     target_link_libraries(my_exe PUBLIC CLArgs)
#
####################################################################################################
cmake_minimum_required(VERSION 3.20)

project(CLArgs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(CLARGS_SINGLE_HEADER "Use CLArgs as a single-header library" ON)
message(STATUS "CLArgs: Option CLARGS_SINGLE_HEADER: ${CLARGS_SINGLE_HEADER}")

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_LIST_DIR)
    set(CLARGS_BUILD_EXAMPLES_DEFAULT ON)
    set(CLARGS_BUILD_TESTS_DEFAULT ON)
else ()
    set(CLARGS_BUILD_EXAMPLES_DEFAULT OFF)
    set(CLARGS_BUILD_TESTS_DEFAULT OFF)
endif ()

option(CLARGS_BUILD_EXAMPLES "Build CLArgs examples" CLARGS_BUILD_EXAMPLES_DEFAULT)
message(STATUS "CLArgs: Option CLARGS_BUILD_EXAMPLES: ${CLARGS_BUILD_EXAMPLES}")

option(CLARGS_BUILD_TESTS "Build CLArgs tests" CLARGS_BUILD_TESTS_DEFAULT)
message(STATUS "CLArgs: Option CLARGS_BUILD_TESTS: ${CLARGS_BUILD_TESTS}")

set(CLARGS_HEADERS
        ${CMAKE_CURRENT_LIST_DIR}/include/CLArgs/common_flags.hpp
        ${CMAKE_CURRENT_LIST_DIR}/include/CLArgs/common_options.hpp
        ${CMAKE_CURRENT_LIST_DIR}/include/CLArgs/core.hpp
        ${CMAKE_CURRENT_LIST_DIR}/include/CLArgs/parser.hpp
        ${CMAKE_CURRENT_LIST_DIR}/include/CLArgs/parser_builder.hpp
        ${CMAKE_CURRENT_LIST_DIR}/include/CLArgs/parse_value.hpp
        ${CMAKE_CURRENT_LIST_DIR}/include/CLArgs/value_container.hpp
)

set(CLARGS_AMALGAMATE_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/scripts/amalgamate.py")
set(CLARGS_GENERATED_INCLUDE_DIR "${CMAKE_BINARY_DIR}/clargs/include/")
set(CLARGS_AMALGAMATED_OUTPUT_DIR "${CLARGS_GENERATED_INCLUDE_DIR}/CLArgs")
set(CLARGS_AMALGAMATED_HEADER "${CLARGS_AMALGAMATED_OUTPUT_DIR}/clargs.hpp")
file(MAKE_DIRECTORY ${CLARGS_AMALGAMATED_OUTPUT_DIR})

set(CLARGS_AMALGAMATE_SCRIPT_WITH_ARGS
        ${CLARGS_AMALGAMATE_SCRIPT}
        --header-dir ${CMAKE_CURRENT_LIST_DIR}/include/CLArgs
        --license ${CMAKE_CURRENT_LIST_DIR}/LICENSE
        --output-file ${CLARGS_AMALGAMATED_HEADER}
        --force
)

find_package(Python3 REQUIRED COMPONENTS Interpreter)

message(STATUS "CLArgs: Amalgamating headers...")
execute_process(
        COMMAND ${CMAKE_COMMAND} -E env ${Python3_EXECUTABLE} ${CLARGS_AMALGAMATE_SCRIPT_WITH_ARGS}
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        RESULT_VARIABLE CLARGS_AMALGAMATION_RESULT
        OUTPUT_VARIABLE CLARGS_AMALGAMATION_OUTPUT
        ERROR_VARIABLE CLARGS_AMALGAMATION_ERROR
)
if (NOT CLARGS_AMALGAMATION_RESULT EQUAL "0")
    message(FATAL_ERROR "CLArgs: Amalgamation failed: ${CLARGS_AMALGAMATION_ERROR}")
endif ()
message(STATUS "CLArgs: Amalgamated header '${CLARGS_AMALGAMATED_HEADER}' created")

add_custom_command(
        OUTPUT ${CLARGS_AMALGAMATED_HEADER}
        COMMAND ${CMAKE_COMMAND} -E env ${Python3_EXECUTABLE} ${CLARGS_AMALGAMATE_SCRIPT_WITH_ARGS}
        DEPENDS ${CLARGS_HEADERS} ${CLARGS_AMALGAMATE_SCRIPT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        COMMENT "CLArgs: Re-generating '${CLARGS_AMALGAMATED_HEADER}'"
)
add_custom_target(CLArgs_internal_generate_amalgamated_header DEPENDS ${CLARGS_AMALGAMATED_HEADER})

add_library(CLArgs_internal_source INTERFACE ${CLARGS_HEADERS})
target_include_directories(CLArgs_internal_source INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include/)

add_library(CLArgs_internal_amalgamated INTERFACE ${CLARGS_AMALGAMATED_HEADER})
target_include_directories(CLArgs_internal_amalgamated INTERFACE ${CLARGS_GENERATED_INCLUDE_DIR})
add_dependencies(CLArgs_internal_amalgamated CLArgs_internal_generate_amalgamated_header)

if (CLARGS_SINGLE_HEADER)
    message(STATUS "CLArgs: CLArgs target using amalgamated header")
    add_library(CLArgs ALIAS CLArgs_internal_amalgamated)
else ()
    message(STATUS "CLArgs: CLArgs target using source headers")
    add_library(CLArgs ALIAS CLArgs_internal_source)
endif ()

if (CLARGS_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif ()

if (CLARGS_BUILD_TESTS)
    add_subdirectory(tests/)
endif ()