cmake_minimum_required(VERSION 3.20)

project(CLArgs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(CLARGS_SINGLE_HEADER "Use CLArgs as a single-header library" ON)
message(STATUS "CLArgs: Option CLARGS_SINGLE_HEADER: ${CLARGS_SINGLE_HEADER}")

option(CLARGS_BUILD_EXAMPLES "Build CLArgs examples" OFF)
message(STATUS "CLArgs: Option CLARGS_BUILD_EXAMPLES: ${CLARGS_BUILD_EXAMPLES}")

option(CLARGS_BUILD_TESTS "Build CLArgs tests" OFF)
message(STATUS "CLArgs: Option CLARGS_BUILD_TESTS: ${CLARGS_BUILD_TESTS}")

set(CLARGS_GENERATED_INCLUDE_DIR "${CMAKE_BINARY_DIR}/clargs/include/")
set(CLARGS_AMALGAMATED_OUTPUT_DIR "${CLARGS_GENERATED_INCLUDE_DIR}/CLArgs")
set(CLARGS_AMALGAMATED_HEADER "${CLARGS_AMALGAMATED_OUTPUT_DIR}/clargs.hpp")
file(MAKE_DIRECTORY ${CLARGS_AMALGAMATED_OUTPUT_DIR})

find_package(Python3 REQUIRED COMPONENTS Interpreter)


message(STATUS "CLArgs: Amalgamating headers...")
execute_process(
        COMMAND ${CMAKE_COMMAND} -E env ${Python3_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/scripts/amalgamate.py
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
message(STATUS "CLArgs: Amalgamated header '${CLARGS_AMALGAMATED_HEADER}' created")

add_custom_command(
        OUTPUT ${CLARGS_AMALGAMATED_HEADER}
        COMMAND ${CMAKE_COMMAND} -E env ${Python3_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/scripts/amalgamate.py
        --header-dir ${CMAKE_CURRENT_LIST_DIR}/include/CLArgs
        --license ${CMAKE_CURRENT_LIST_DIR}/LICENSE
        --output-file ${CLARGS_AMALGAMATED_HEADER}
        --force
        DEPENDS
        ${CMAKE_CURRENT_LIST_DIR}/scripts/amalgamate.py
        ${CMAKE_CURRENT_LIST_DIR}/include/CLArgs/parser.hpp
        ${CMAKE_CURRENT_LIST_DIR}/include/CLArgs/core.hpp
        ${CMAKE_CURRENT_LIST_DIR}/include/CLArgs/parse_value.hpp
        ${CMAKE_CURRENT_LIST_DIR}/include/CLArgs/value_container.hpp
        ${CMAKE_CURRENT_LIST_DIR}/include/CLArgs/parser_builder.hpp
        ${CMAKE_CURRENT_LIST_DIR}/include/CLArgs/common_flags.hpp
        ${CMAKE_CURRENT_LIST_DIR}/include/CLArgs/common_options.hpp
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        COMMENT "CLArgs: Amalgamating headers..."
)
add_custom_target(CLArgs_generate_amalgamated_header
        DEPENDS ${CLARGS_AMALGAMATED_HEADER}
)

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

add_library(CLArgs_amalgamated INTERFACE
        ${CLARGS_AMALGAMATED_HEADER}
)
target_include_directories(CLArgs_amalgamated INTERFACE
        ${CLARGS_GENERATED_INCLUDE_DIR}
)
add_dependencies(CLArgs_amalgamated CLArgs_generate_amalgamated_header)

add_library(CLArgs INTERFACE)
if (CLARGS_SINGLE_HEADER)
    message(STATUS "CLArgs: CLArgs target using amalgamated header")
    target_link_libraries(CLArgs INTERFACE CLArgs_amalgamated)
else ()
    message(STATUS "CLArgs: CLArgs target using source headers")
    target_link_libraries(CLArgs INTERFACE CLArgs_source)
endif ()

if (${CLARGS_BUILD_EXAMPLES})
    add_subdirectory(examples)
endif ()

if (${CLARGS_BUILD_TESTS})
    add_subdirectory(tests/)
endif ()
