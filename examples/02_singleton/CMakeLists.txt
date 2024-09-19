add_executable(CLArgsExample_02Singleton
        main.cpp
)

target_compile_options(CLArgsExample_02Singleton PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)

target_link_libraries(CLArgsExample_02Singleton PUBLIC
        CLArgs
)