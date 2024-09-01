[![tests](https://github.com/rubensorensen/clargs/actions/workflows/tests.yml/badge.svg)](https://github.com/rubensorensen/clargs/actions/workflows/tests.yml)
[![docs](https://github.com/rubensorensen/clargs/actions/workflows/docs.yml/badge.svg)](https://github.com/rubensorensen/clargs/actions/workflows/docs.yml)
[![CodeQL](https://github.com/rsore/clargs/actions/workflows/github-code-scanning/codeql/badge.svg)](https://github.com/rsore/clargs/actions/workflows/github-code-scanning/codeql)
[![formatting](https://github.com/rsore/clargs/actions/workflows/clang-format.yml/badge.svg)](https://github.com/rsore/clargs/actions/workflows/clang-format.yml)
[![static-code-analysis](https://github.com/rsore/clargs/actions/workflows/clang-tidy-analysis.yml/badge.svg)](https://github.com/rsore/clargs/actions/workflows/clang-tidy-analysis.yml)

# CLArgs

Command-line argument parser written in C++20.

Uses C++20 concepts and constraints to handle command-line argument parsing with static type-checking.

## Example usage

```cpp
using VerboseFlag   = CLArgs::Flag<"--verbose,-v", "Enable verbose output">;
using ConfigOption  = CLArgs::Option<"--config,--configuration,-c", "<filepath>", "Specify config file", std::filesystem::path>;

int
main(int argc, char **argv)
{
    CLArgs::Parser<VerboseFlag, ConfigOption> parser;
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
Static type-checking should ensure proper usage and disallow building with invalid or unsupported `ValueType`s. 

All arguments are parsed, validated, cast and stored during the `parse` method and that is the only potential point of failure, meaning that if the application makes it past that function call with no
exceptions, then everything should be fine.
