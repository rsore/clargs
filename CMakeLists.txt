cmake_minimum_required(VERSION 3.20)

project(CLArgs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(CLARGS_GENERATED_INCLUDE_DIR "${CMAKE_BINARY_DIR}/clargs/include/")
set(AMALGAMATED_OUTPUT_DIR "${CLARGS_GENERATED_INCLUDE_DIR}/CLArgs")
set(AMALGAMATED_HEADER "${AMALGAMATED_OUTPUT_DIR}/clargs.hpp")
file(MAKE_DIRECTORY ${AMALGAMATED_OUTPUT_DIR})

execute_process(
        COMMAND ${CMAKE_COMMAND} -E env ${Python3_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/amalgamate.py
        --header-dir ${CMAKE_CURRENT_LIST_DIR}/include/CLArgs
        --license ${CMAKE_CURRENT_LIST_DIR}/LICENSE
        --output-file ${AMALGAMATED_HEADER}
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        RESULT_VARIABLE AMALGAMATION_RESULT
        OUTPUT_VARIABLE AMALGAMATION_OUTPUT
        ERROR_VARIABLE AMALGAMATION_ERROR
)
if (NOT AMALGAMATION_RESULT EQUAL "0")
    message(FATAL_ERROR "Amalgamation failed: ${AMALGAMATION_ERROR}")
endif()

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

add_library(CLArgs_single_header INTERFACE)
target_include_directories(CLArgs_single_header INTERFACE ${CLARGS_GENERATED_INCLUDE_DIR})

option(CLARGS_BUILD_EXAMPLES "Build CLArgs examples" ON)
option(CLARGS_BUILD_TESTS "Build CLArgs tests" ON)

if (${CLARGS_BUILD_EXAMPLES})
    add_subdirectory(examples)
endif ()

if (${CLARGS_BUILD_TESTS})
    add_subdirectory(tests/)
endif ()
