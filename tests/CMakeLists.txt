cmake_minimum_required(VERSION 3.20)

Include(FetchContent)

FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.6.0
)
FetchContent_MakeAvailable(Catch2)

add_executable(CLArgsTests
        parser_tests.cpp
        parse_value_tests.cpp
        test_utils.hpp
        value_container_tests.cpp
        parser_builder_tests.cpp
        common_flags_tests.cpp
        common_options_tests.cpp
)

target_compile_options(CLArgsTests PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)


target_link_libraries(CLArgsTests PRIVATE
        CLArgs
        Catch2::Catch2WithMain
)