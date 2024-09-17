---
layout: default
title: Basic
parent: Code Examples
grand_parent: CLArgs
---

# Basic

In this example, we define one flag and one option: `VerboseFlag` and `ConfigOption`.

- `VerboseFlag`: Enables verbose output. It can be enabled either with "--verbose" or "-v". It has a description that appears in the help menu.
- `ConfigOption`: Used to specify the path to a configuration file to load. It can be enabled either with "--configuration", "--config" or "-c". 
  It has a value hint and description that appear in the help menu, and its value type is `std::filesystem::path`, so the parser will 
  automatically create a path object from the provided file path.

## How It Works

- Definitions: Flags and options are defined as types by aliasing `CLArgs::Flag` and `CLArgs::Option`, and contain information about their identifiers, descriptions, value types etc.
- Parsing: The parse method of the CLArgs::Parser class handles all parsing, validation, and object creation. 
  If any issues occur, such as invalid input for the specified value type, exceptions are thrown with helpful error messages.
- Error Handling: If an error occurs during parsing, the program catches the exception, prints an error message, and displays the help message.
- Accessing Option Values: After successful parsing, you can check if flags are present using `has_flag<>()` 
  and check for and retrieve option values using `get_option<>()`. The value is returned as a `std::optional`, so you should check if it contains a value before using it.

## Code example

```cpp
#include <CLArgs/parser.hpp>

#include <filesystem>

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

    return EXIT_SUCCESS;
}
```

## Running the application

Once the application has been built you can run the application with the defined arguments:
```
./basic -v --config conf.ini
```
Alternatively:
```
./basic --verbose -c conf.ini
```
If you omit the file path for the configuration option, an error message will be displayed along with the help menu:
```
$ ./basic --configuration
Error: Expected value for option "--configuration"
Usage: ./basic [OPTIONS...]

Options:
  --verbose, -v                 Enable verbose output
  --configuration,--config-c    Specify config file
```
