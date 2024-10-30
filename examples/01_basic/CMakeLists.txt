add_executable(CLArgsExample_01Basic
        main.cpp
)

target_compile_options(CLArgsExample_01Basic PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)

target_link_libraries(CLArgsExample_01Basic PUBLIC
        CLArgs_single_header
)