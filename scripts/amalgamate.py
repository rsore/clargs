from logger import logger

import os
import re
from collections import defaultdict, deque
import argparse
from pathlib import Path
import sys
from datetime import datetime, timezone

CLARGS_INCLUDE_PATTERN = re.compile(r"#include <CLArgs/(\w+\.hpp)>")
SYSTEM_INCLUDE_PATTERN = re.compile(r"#include <(.+?)>")

CLARGS_ASCII_ART = r"""
          _____ _      ___
         /  __ \ |    / _ \
         | /  \/ |   / /_\ \_ __ __ _ ___
         | |   | |   |  _  | '__/ _` / __|
         | \__/\ |___| | | | | | (_| \__ \
          \____|_____|_| |_/_|  \__, |___/
                                 __/ |
                                |___/

                              Command-line argument parser
""".lstrip('\n')

CLARGS_GENERATED_FILE_WARNING = r"""
============================================================================
WARNING: This file is auto-generated from multiple source headers.
Any changes made directly to this file may be overwritten.
For persistent modifications, please edit the original source headers and
re-run the amalgamation script to regenerate this file.
============================================================================
""".lstrip('\n')


def find_headers(directory):
    logger.verbose_log(f"Discovering headers in directory '{directory}':")
    headers = {}
    for root, _, files in os.walk(directory):
        for file in files:
            if file.endswith(".hpp"):
                path = os.path.join(root, file)
                headers[file] = path
                logger.verbose_log(f" - {file}")
    return headers


def parse_dependencies(headers):
    clargs_dependencies = defaultdict(set)
    system_dependencies = set()

    for file, path in headers.items():
        logger.verbose_log(f"Discovering dependencies for header '{file}':")
        with open(path, "r") as f:
            for line in f:
                match = CLARGS_INCLUDE_PATTERN.search(line)
                if match:
                    include_file = match.group(1)
                    if include_file in headers:
                        logger.verbose_log(f" - {include_file} (CLArgs dependency)")
                        clargs_dependencies[file].add(include_file)
                    continue
                match = SYSTEM_INCLUDE_PATTERN.search(line)
                if match:
                    include_file = match.group(1)
                    logger.verbose_log(f" - {include_file} (System dependency)")
                    system_dependencies.add(include_file)

    return clargs_dependencies, system_dependencies


def resolve_order(headers, clargs_dependencies):
    logger.verbose_log("Resolving topological ordering for headers:")
    indegree = {file: 0 for file in headers}
    for deps in clargs_dependencies.values():
        for dep in deps:
            indegree[dep] += 1

    queue = deque([file for file in headers if indegree[file] == 0])
    ordered_files = []

    while queue:
        file = queue.popleft()
        logger.verbose_log(f" - {file}")
        ordered_files.append(file)
        for dep in clargs_dependencies[file]:
            indegree[dep] -= 1
            if indegree[dep] == 0:
                queue.append(dep)

    logger.verbose_log("Checking for circular dependencies...")
    if len(ordered_files) != len(headers):
        raise RuntimeError("Circular dependency detected among headers.")
    logger.verbose_log("No circular dependencies detected, continuing")

    logger.verbose_log("Reversing topological ordering")
    ordered_files.reverse()
    return {file: headers[file] for file in ordered_files}


def strip_header_includes(content):
    content_str = "".join(content)
    cleaned_content = re.sub(r"#include <(?:CLArgs/\w+\.hpp|[\w/]+)>", "", content_str)
    return cleaned_content.splitlines(keepends=True)


def strip_header_guards(content):
    header_guard_pattern = re.compile(r"^\s*#(ifndef|define)\s+(CLARGS_[A-Z0-9_]+_HPP)\s*$")
    close_guard_pattern = re.compile(r"^\s*#endif\s*//\s*(CLARGS_[A-Z0-9_]+_HPP)\s*$")

    result = []
    for line in content:
        if header_guard_pattern.match(line) or close_guard_pattern.match(line):
            continue
        result.append(line)

    return result


def remove_consecutive_newlines(content):
    content_str = "".join(content)
    formatted_content = re.sub(r"\n{2,}", "\n\n", content_str)
    return formatted_content.splitlines(keepends=True)


def create_single_header(headers, system_dependencies, license_path, add_warning, output_file, with_timestamp,
                         version=None):
    logger.verbose_log("Creating amalgamated header:")
    header_guard = "CLARGS_" + output_file.name.upper().replace(" ", "_").replace(".", "_")
    result = []

    if add_warning:
        logger.verbose_log(" - Adding generated file warning")
        for line in CLARGS_GENERATED_FILE_WARNING.splitlines():
            result.append(f"// {line}\n")

    logger.verbose_log(f" - Opening header guard '{header_guard}'")
    result.extend([f"#ifndef {header_guard}\n",
                   f"#define {header_guard}\n\n",
                   "/**\n"])

    logger.verbose_log(" - Adding ASCII art")
    for line in CLARGS_ASCII_ART.splitlines():
        result.append(f" *  {line}\n")

    result.append(" *\n")

    if version is not None:
        logger.verbose_log(" - Adding version")
        result.append(f" *  Version: {version}\n")
    if with_timestamp:
        logger.verbose_log(" - Adding timestamp")
        result.append(f" *  Generated: {datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")}\n") # ISO 8601

    result.append(" *\n *  ---\n *\n")

    logger.verbose_log(" - Adding license")
    with open(license_path, "r") as license_file:
        for line in license_file:
            result.append(f" *  {line}")
    result.append(" */\n\n")

    logger.verbose_log(" - Adding system includes")
    for header in sorted(system_dependencies):
        result.append(f"#include <{header}>\n")
    result.append("\n")

    for file, path in headers.items():
        logger.verbose_log(f" - Processing content of header '{file}':")
        with open(path, "r") as f:
            logger.verbose_log("   - Reading content")
            content = f.readlines()
            logger.verbose_log("   - Removing header guards")
            content = strip_header_guards(content)
            logger.verbose_log("   - Removing includes")
            content = strip_header_includes(content)
            logger.verbose_log("   - Adding to result")
            result.extend(content)
            result.append("\n")

    logger.verbose_log(f" - Closing header guard '{header_guard}'")
    result.append(f"#endif // {header_guard}\n")

    logger.verbose_log(" - Cleaning up newlines")
    result = remove_consecutive_newlines(result)

    logger.verbose_log(f" - Writing final result to file '{output_file}'")
    with open(output_file, "w", newline="\n") as f:
        f.writelines(result)
    print(f"Successfully created file '{output_file}'")


def amalgamate(header_dir, license_path, add_warning, output_file, with_timestamp=True, version=None):
    headers = find_headers(header_dir)
    clargs_dependencies, system_dependencies = parse_dependencies(headers)
    ordered_headers = resolve_order(headers, clargs_dependencies)
    create_single_header(ordered_headers, system_dependencies, license_path, add_warning, output_file, with_timestamp,
                         version)


def main():
    parser = argparse.ArgumentParser(description="Amalgamate all headers to a single library header")
    parser.add_argument("--header-dir",
                        default="./include/CLArgs",
                        help="Specify path to directory containing library headers",
                        type=Path)
    parser.add_argument("--license",
                        default="./LICENSE",
                        help="Specify path to license file",
                        type=Path)
    parser.add_argument("--output-file",
                        default="./clargs.hpp",
                        help="Specify path for file to output amalgamated header to",
                        type=Path)
    parser.add_argument("--version",
                        default=None,
                        help="Specify CLArgs library version, which will be embedded in comment header of output file")
    parser.add_argument("-f", "--force", action="store_true")
    parser.add_argument("-v", "--verbose", action="store_true")
    parser.add_argument("--no-generated-file-warning",
                        action="store_true")
    parser.add_argument("--omit-timestamp",
                        action="store_true")
    args = parser.parse_args()

    logger.verbose = args.verbose
    logger.verbose_log("Verbose output is enabled")

    if args.output_file.exists() and not args.force:
        response = input(f"File '{args.output_file}' already exists. Overwrite it? (y/N) ")
        if response.lower() != "y":
            logger.verbose_log(f"Response was '{response}', exiting...")
            sys.exit(0)

    output_file_dir = args.output_file.parent
    if not output_file_dir.exists():
        logger.verbose_log(f"Directory '{output_file_dir}' does not exist, creating it...")
        os.makedirs(output_file_dir, exist_ok=False)
        logger.verbose_log(f"Directory '{output_file_dir}' created")

    logger.verbose_log(f"Looking for license file {args.license}...")
    if not args.license.exists():
        print(f"Error: License file '{args.license}' does not exist", file=sys.stderr)
        sys.exit(1)
    logger.verbose_log("License file found")

    logger.verbose_log(f"Looking for source header directory '{args.header_dir}'...")
    if not args.header_dir.exists():
        print(f"Error: Directory '{args.header_dir}' does not exist", file=sys.stderr)
        sys.exit(1)
    logger.verbose_log("Source header directory found")

    try:
        logger.verbose_log("Starting header amalgamation...")
        amalgamate(args.header_dir, args.license, not args.no_generated_file_warning, args.output_file,
                   not args.omit_timestamp, args.version)
        sys.exit(0)
    except RuntimeError as e:
        print(f"Error: {e}", sys.stderr)
        print("Aborting...", sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
