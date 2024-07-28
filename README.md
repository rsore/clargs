# CLArgs

Command-line argument parser written in C++20.

Uses C++20 concepts and constraints to handle command-line argument parsing with static type-checking.

## Example usage

```cpp
struct VerboseFlag
{
    static constexpr std::string_view identifier{ "--verbose" };
    static constexpr std::string_view alias{ "-v" };
    static constexpr std::string_view description{ "Enable verbose printing" };
    static constexpr bool             required{ false };
};

struct FileOption
{
    static constexpr std::string_view identifier{ "--file" };
    static constexpr std::string_view alias{ "-f" };
    static constexpr std::string_view description{ "Specify input file" };
    static constexpr std::string_view value_hint{ "FILE" };
    static constexpr bool             required{ true };
    using ValueType = std::string;
};

int
main(int argc, char **argv)
{
    CLArgs::Parser<VerboseFlag, FileOption> argument_parser;
    try
    {
        argument_parser.parse(argc, argv);
    }
    catch (std::exception &e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        std::cerr << argument_parser.help() << std::endl;
        return EXIT_FAILURE;
    }
    
    if (argument_parser.has_option<VerboseFlag>())
    {
        std::cout << "Verbose printing enabled" << std::endl;
    }
    
    if (const auto file = argument_parser.get_option_value<FileOption>(); file.has_value())
    {
        std::cout << "File: " << file.value() << std::endl;
    }
}
```

For full examples, see the examples/ directory

## Error handling
Static type-checking should ensure proper usage and disallow building with invalid or unsupported `ValueType`s. 

All arguments are parsed, validated, cast and stored during the `parse` method and that is the only potential point of failure, meaning that if the application makes it past that function call with no
exceptions, then everything should be fine.