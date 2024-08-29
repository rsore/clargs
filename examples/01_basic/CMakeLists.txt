add_executable(Basic
        main.cpp
)

target_compile_options(Basic PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)

target_link_libraries(Basic PUBLIC
        CLArgs
)