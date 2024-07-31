---
layout: default
title: Basic example
parent: Code Examples
grand_parent: CLArgs
---

# Basic example

In this example, we define two options: `VerboseOption` and `FileOption`. 
These options demonstrate how to handle different types of command-line arguments using CLArgs.

- `VerboseOption`: An optional flag that enables verbose output. It has an alias -v and a description that appears in the help menu.
- `FileOption`: A required option that specifies the file to load. It has a value hint (FILE) and its value type 
  is `std::filesystem::path`, so the parser will automatically create a path object from the provided file path.

## How It Works

- Option Definition: Options are defined as structs with static constexpr members to specify their identifiers, descriptions, and other properties.
- Parsing: The parse method of the CLArgs::Parser class handles all parsing, validation, and casting. 
  If any issues occur (such as missing required options), exceptions are thrown with helpful error messages.
- Error Handling: If an error occurs during parsing, the program catches the exception, prints an error message, and displays the help message.
- Accessing Option Values: After successful parsing, you can check if options are present using `has_option<>()` 
  and retrieve their values using `get_option_value<>()`. The value is returned as a `std::optional`, so you should check if it contains a value before using it.

## Code example

```cpp
#include <CLArgs/parser.hpp>

#include <filesystem>
#include <sstream>
#include <string_view>

struct VerboseOption
{
    static constexpr std::string_view identifier{ "--verbose" };
    static constexpr std::string_view alias{ "-v" };
    static constexpr std::string_view description{ "Enable verbose output" };
    static constexpr bool             required{ false };
};

struct FileOption
{
    static constexpr std::string_view identifier{ "--file" };
    static constexpr std::string_view value_hint{ "FILE" };
    static constexpr std::string_view description{ "Specify file to load" };
    static constexpr bool             required{ true };
    using ValueType = std::filesystem::path;
};

int
main(int argc, char **argv)
{
    CLArgs::Parser<VerboseOption, FileOption> parser;
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

    const bool has_verbose = parser.has_option<VerboseOption>();
    std::cout << "Has option " << VerboseOption::identifier << ": " << has_verbose << "\n";

    if (const auto file = parser.get_option_value<FileOption>(); file.has_value())
    {
        std::cout << "File: " << file.value() << std::endl;
    }

    return EXIT_SUCCESS;
}
```

## Running the application

Once the application has been built you can run the application with the defined arguments:
```
./basic --verbose --file filename.txt
```
Alternatively:
```
./basic -v --file filename.txt
```
If you forget to supply a filename an error message will be displayed along with the help menu:
```
$ ./basic --verbose --file
Error: Expected value for option "--file"
Usage: ./basic [--verbose] --file FILE
Options:
  --verbose, -v              Enable verbose output
  --file           REQUIRED  Specify file to load
```
