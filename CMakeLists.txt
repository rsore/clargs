cmake_minimum_required(VERSION 3.20)

project(CLArgs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(CLARGS_AMALGAMATED "Use CLArgs as a single-header library" ON)
option(CLARGS_BUILD_EXAMPLES "Build CLArgs examples" OFF)
option(CLARGS_BUILD_TESTS "Build CLArgs tests" OFF)

set(CLARGS_GENERATED_INCLUDE_DIR "${CMAKE_BINARY_DIR}/clargs/include/")
set(CLARGS_AMALGAMATED_OUTPUT_DIR "${CLARGS_GENERATED_INCLUDE_DIR}/CLArgs")
set(CLARGS_AMALGAMATED_HEADER "${CLARGS_AMALGAMATED_OUTPUT_DIR}/clargs.hpp")
file(MAKE_DIRECTORY ${CLARGS_AMALGAMATED_OUTPUT_DIR})

find_package(Python3 REQUIRED COMPONENTS Interpreter)
message("CLArgs: Amalgamating headers...")
execute_process(
        COMMAND ${CMAKE_COMMAND} -E env ${Python3_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/amalgamate.py
        --header-dir ${CMAKE_CURRENT_LIST_DIR}/include/CLArgs
        --license ${CMAKE_CURRENT_LIST_DIR}/LICENSE
        --output-file ${CLARGS_AMALGAMATED_HEADER}
        --force
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        RESULT_VARIABLE CLARGS_AMALGAMATION_RESULT
        OUTPUT_VARIABLE CLARGS_AMALGAMATION_OUTPUT
        ERROR_VARIABLE CLARGS_AMALGAMATION_ERROR
)
if (NOT CLARGS_AMALGAMATION_RESULT EQUAL "0")
    message(FATAL_ERROR "CLArgs: Amalgamation failed: ${CLARGS_AMALGAMATION_ERROR}")
endif ()
message("CLArgs: Amalgamated header '${CLARGS_AMALGAMATED_HEADER}' created")

add_library(CLArgs_source INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/include/CLArgs/parser.hpp
        ${CMAKE_CURRENT_LIST_DIR}/include/CLArgs/core.hpp
        ${CMAKE_CURRENT_LIST_DIR}/include/CLArgs/parse_value.hpp
        ${CMAKE_CURRENT_LIST_DIR}/include/CLArgs/value_container.hpp
        ${CMAKE_CURRENT_LIST_DIR}/include/CLArgs/parser_builder.hpp
        ${CMAKE_CURRENT_LIST_DIR}/include/CLArgs/common_flags.hpp
        ${CMAKE_CURRENT_LIST_DIR}/include/CLArgs/common_options.hpp
)

target_include_directories(CLArgs_source INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/include/
)

add_library(CLArgs_amalgamated INTERFACE)
target_include_directories(CLArgs_amalgamated INTERFACE
        ${CLARGS_GENERATED_INCLUDE_DIR}
)

add_library(CLArgs INTERFACE)
if (CLARGS_AMALGAMATED)
    message("CLArgs: CLArgs target using amalgamated header")
    target_link_libraries(CLArgs INTERFACE CLArgs_amalgamated)
else ()
    message("CLArgs: CLArgs target using source headers")
    target_link_libraries(CLArgs INTERFACE CLArgs_source)
endif ()

if (${CLARGS_BUILD_EXAMPLES})
    add_subdirectory(examples)
endif ()

if (${CLARGS_BUILD_TESTS})
    add_subdirectory(tests/)
endif ()
