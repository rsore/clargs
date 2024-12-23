[![tests](https://github.com/rubensorensen/clargs/actions/workflows/tests.yml/badge.svg)](https://github.com/rubensorensen/clargs/actions/workflows/tests.yml)
[![docs](https://github.com/rubensorensen/clargs/actions/workflows/docs.yml/badge.svg)](https://github.com/rubensorensen/clargs/actions/workflows/docs.yml)
[![formatting](https://github.com/rsore/clargs/actions/workflows/clang-format.yml/badge.svg)](https://github.com/rsore/clargs/actions/workflows/clang-format.yml)
[![static-code-analysis](https://github.com/rsore/clargs/actions/workflows/clang-tidy-analysis.yml/badge.svg)](https://github.com/rsore/clargs/actions/workflows/clang-tidy-analysis.yml)
![stability](https://img.shields.io/badge/stability-experimental-orange)

> **Note**: This project is under active development, and its APIs are subject to change without notice.

# CLArgs

This library is a modern C++ (C++20) command-line argument parser designed with a focus on type safety, compile-time validation, and runtime performance. Unlike many other libraries, it avoids macros, dynamic allocation, type-erasure, and expensive runtime operations.

At its core, this parser ensures that all argument types are validated at compile-time, minimizing the chances of runtime errors. Once parsing is complete, retrieving values is as simple as a constant-time array lookup, meaning that all command-line options are available as strongly-typed values with zero overhead.

## Example usage

```cpp
using VerboseFlag   = CLArgs::Flag<"--verbose,-v", "Enable verbose output">;
using ConfigOption  = CLArgs::Option<"--config,--configuration,-c", "<filepath>", "Specify config file", std::filesystem::path>;

int
main(int argc, char **argv)
{
    auto parser = CLArgs::ParserBuilder{}
                      .add_program_description<"Example program.">()
                      .add_flag<VerboseFlag>()
                      .add_option<ConfigOption>()
                      .build();
    try
    {
        parser.parse(argc, argv);
    }
    catch (std::exception &e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        std::cerr << parser.help() << std::endl;
        return EXIT_FAILURE;
    }

    const bool has_verbose = parser.has_flag<VerboseFlag>();
    std::cout << "Has verbose option: " << std::boolalpha << has_verbose << "\n";

    if (const auto config = parser.get_option<ConfigOption>(); config.has_value())
    {
        std::cout << "Config file: " << config.value() << std::endl;
    }
}
```

For full examples, see the examples/ directory

## Error handling
Static type-checking ensures proper usage and prevents building with invalid or unsupported value types.

All arguments are parsed, validated and stored during the `parse` method. This is the only potential point of failure, meaning that if the application makes it past that function call with no exceptions, values can safely be retrieved at any time with no risk.
