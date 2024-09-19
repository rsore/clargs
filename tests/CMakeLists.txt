cmake_minimum_required(VERSION 3.20)

Include(FetchContent)

FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.6.0
)
FetchContent_MakeAvailable(Catch2)

add_executable(tests
        parser_tests.cpp
        parse_value_tests.cpp
)

target_link_libraries(tests PRIVATE
        CLArgs
        Catch2::Catch2WithMain
)