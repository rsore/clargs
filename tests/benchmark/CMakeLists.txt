cmake_minimum_required(VERSION 3.20)

add_executable(CLArgsBenchmarks
        argument_queue_benchmarks.cpp
)

target_compile_options(CLArgsBenchmarks PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)


target_link_libraries(CLArgsBenchmarks PRIVATE
        CLArgs
        Catch2::Catch2WithMain
)