---
layout: default
title: Basic
parent: Code Examples
grand_parent: CLArgs
---

# Basic
In this example, we define two flags and one option: `HelpFlag`, `VerboseFlag` and `ConfigOption`.

- `HelpFlag`: Displays the help menu and exits. It can be enabled with either "--help" or "-h". It has a description that appears in the help menu.
- `VerboseFlag`: Enables verbose output. It can be enabled either with "--verbose" or "-v". It has a description that appears in the help menu.
- `ConfigOption`: Used to specify the path to a configuration file to load. It can be enabled either with "--configuration", "--config" or "-c". 
  It has a value hint and description that appear in the help menu, and its value type is `std::filesystem::path`, so the parser will 
  automatically create a path object from the provided file path.

To create the actual parser, we utilize the `ParserBuilder` class. This allows us to incrementally build up a parser, supporting the flags
and options we want. We also add a program description which is added as part of the created help menu.

## How It Works
- Definitions: Flags and options are defined by aliasing `CLArgs::Flag` and `CLArgs::Option`. These types contain information about their identifiers, descriptions, value types and more.
- Parsing: The parse method of the CLArgs::Parser class handles all parsing, validation, and object creation. 
  If any issues occur, such as invalid input for the specified value type or unknown arguments, exceptions are thrown with helpful error messages.
- Error Handling: If an error occurs during parsing, the program catches the exception, prints an error message, and displays the help menu.
- Accessing Option Values: After successful parsing, you can check if flags are present using `has_flag<>()` 
  and check for and retrieve option values using `get_option<>()`. The value is returned as a `std::optional`, so you should check if it contains a value before using it.

## Code example

```cpp
#include <CLArgs/clargs.hpp>
#include <filesystem>
#include <iostream>

using HelpFlag     = CLArgs::Flag<"--help,-h", "Show help menu">;
using VerboseFlag  = CLArgs::Flag<"--verbose,-v", "Enable verbose output">;
using ConfigOption = CLArgs::Option<"--config,--configuration,-c", "<filepath>", "Specify config file", std::filesystem::path>;

int
main(int argc, char **argv)
{
    CLArgs::Parser parser = CLArgs::ParserBuilder{}
                                .add_program_description<"Basic example program to showcase CLArgs library.">()
                                .add_flag<HelpFlag>()
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
        std::cerr << parser.usage() << std::endl;
        return EXIT_FAILURE;
    }

    if (parser.has_flag<HelpFlag>())
    {
        std::cout << parser.help() << std::endl;
        return EXIT_SUCCESS;
    }

    std::cout << "Program: " << parser.program() << std::endl;

    const bool has_verbose = parser.has_flag<VerboseFlag>();
    std::cout << "Has verbose flag: " << std::boolalpha << has_verbose << "\n";

    if (const auto config = parser.get_option<ConfigOption>(); config.has_value())
    {
        std::cout << "Config file: " << config.value() << std::endl;
    }
    else
    {
        std::cout << "No config file provided." << std::endl;
    }

    return EXIT_SUCCESS;
}
```

## Running the application

After building the application, you can run it using the following arguments:
```
./basic -v --config conf.ini
```
Alternatively:
```
./basic --verbose -c conf.ini
```

If you pass the help flag (`--help/-h`), the full help menu is displayed:
```
$ ./basic -h
Basic example program to showcase CLArgs library.

Usage: ./basic [OPTIONS...]

Options:
  --help, -h                       Show help menu
  --verbose, -v                    Enable verbose output
  --config, --configuration, -c    Specify config file
```

If you omit the required file path for the configuration option, an error message will be displayed along with the help menu:
```
$ ./basic --configuration
Error: Expected value for option "--configuration"
Usage: ./basic [OPTIONS...]
```

If you pass an unknown flag or option, an error message will be displayed along with the help menu:
```
$ ./basic --recursive
Error: Unknown option "--recursive"
Usage: ./basic [OPTIONS...]
```