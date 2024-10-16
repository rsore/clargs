#include <CLArgs/core.hpp>
#include <CLArgs/value_container.hpp>

#include <catch2/catch_test_macros.hpp>

#include <filesystem>

using VerboseFlag = CLArgs::Flag<"--verbose,-v", "Enable verbose output">;
using FileOption  = CLArgs::Option<"--file", "FILE", "Specify file to load", std::filesystem::path>;

TEST_CASE("Empty ValueContainer contains all std::nullopt", "[value_container]")
{
    CLArgs::ValueContainer<VerboseFlag, FileOption> container;

    REQUIRE_FALSE(container.get_value<VerboseFlag>().has_value());
    REQUIRE(container.get_value<VerboseFlag>() == std::nullopt);

    REQUIRE_FALSE(container.get_value<FileOption>().has_value());
    REQUIRE(container.get_value<FileOption>() == std::nullopt);
}

TEST_CASE("Values can be inserted into ValueContainer, and safely retrieved", "[value_container]")
{
    CLArgs::ValueContainer<VerboseFlag, FileOption> container;

    REQUIRE_FALSE(container.get_value<VerboseFlag>().has_value());
    REQUIRE_FALSE(container.get_value<FileOption>().has_value());

    container.set_value<VerboseFlag>(false);
    REQUIRE(container.get_value<VerboseFlag>().has_value());
    REQUIRE(container.get_value<VerboseFlag>().value() == false);
    REQUIRE_FALSE(container.get_value<FileOption>().has_value());

    container.set_value<VerboseFlag>(true);
    REQUIRE(container.get_value<VerboseFlag>().has_value());
    REQUIRE(container.get_value<VerboseFlag>().value() == true);
    REQUIRE_FALSE(container.get_value<FileOption>().has_value());

    container.set_value<FileOption>("conf.ini");
    REQUIRE(container.get_value<FileOption>().has_value());
    REQUIRE(container.get_value<FileOption>().value() == "conf.ini");

    container.set_value<FileOption>("configuration_file.txt");
    REQUIRE(container.get_value<FileOption>().has_value());
    REQUIRE(container.get_value<FileOption>().value() == "configuration_file.txt");
}

TEST_CASE("Values in ValueContainer can be reset to std::nullopt using reset()", "[value_container]")
{
    CLArgs::ValueContainer<VerboseFlag, FileOption> container;

    REQUIRE_FALSE(container.get_value<VerboseFlag>().has_value());
    REQUIRE_FALSE(container.get_value<FileOption>().has_value());

    container.set_value<VerboseFlag>(true);
    REQUIRE(container.get_value<VerboseFlag>().has_value());
    REQUIRE(container.get_value<VerboseFlag>().value() == true);

    container.set_value<FileOption>("conf.ini");
    REQUIRE(container.get_value<FileOption>().has_value());
    REQUIRE(container.get_value<FileOption>().value() == "conf.ini");

    container.reset();
    REQUIRE_FALSE(container.get_value<VerboseFlag>().has_value());
    REQUIRE_FALSE(container.get_value<FileOption>().has_value());

    container.set_value<VerboseFlag>(false);
    REQUIRE(container.get_value<VerboseFlag>().has_value());
    REQUIRE(container.get_value<VerboseFlag>().value() == false);

    container.set_value<FileOption>("new_config.ini");
    REQUIRE(container.get_value<FileOption>().has_value());
    REQUIRE(container.get_value<FileOption>().value() == "new_config.ini");
}
