import os
import re
from collections import defaultdict, deque
import argparse
import pathlib

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


"""


def find_headers(directory):
    headers = {}
    for root, _, files in os.walk(directory):
        for file in files:
            if file.endswith(".hpp"):
                path = os.path.join(root, file)
                headers[file] = path
    return headers


def parse_dependencies(headers):
    clargs_dependencies = defaultdict(set)
    system_dependencies = set()

    for file, path in headers.items():
        with open(path, "r") as f:
            for line in f:
                match = CLARGS_INCLUDE_PATTERN.search(line)
                if match:
                    include_file = match.group(1)
                    if include_file in headers:
                        clargs_dependencies[file].add(include_file)
                    continue
                match = SYSTEM_INCLUDE_PATTERN.search(line)
                if match:
                    system_dependencies.add(match.group(1))

    return clargs_dependencies, system_dependencies


def resolve_order(headers, clargs_dependencies):
    indegree = {file: 0 for file in headers}
    for deps in clargs_dependencies.values():
        for dep in deps:
            indegree[dep] += 1

    queue = deque([file for file in headers if indegree[file] == 0])
    ordered_files = []

    while queue:
        file = queue.popleft()
        ordered_files.append(file)
        for dep in clargs_dependencies[file]:
            indegree[dep] -= 1
            if indegree[dep] == 0:
                queue.append(dep)

    if len(ordered_files) != len(headers):
        raise RuntimeError("Circular dependency detected among headers.")

    ordered_files.reverse()
    return {file: headers[file] for file in ordered_files}


def strip_header_includes(content):
    content_str = "".join(content)
    cleaned_content = re.sub(r"#include <(?:CLArgs/\w+\.hpp|[\w/]+)>", "", content_str)
    return cleaned_content.splitlines(keepends=True)


def strip_include_guards(content):
    include_guard_pattern = re.compile(r"^\s*#(ifndef|define)\s+(CLARGS_[A-Z0-9_]+_HPP)\s*$")
    close_guard_pattern = re.compile(r"^\s*#endif\s*//\s*(CLARGS_[A-Z0-9_]+_HPP)\s*$")

    result = []
    for line in content:
        if include_guard_pattern.match(line) or close_guard_pattern.match(line):
            continue
        result.append(line)

    return result


def remove_consecutive_newlines(content):
    content_str = "".join(content)
    formatted_content = re.sub(r"\n{2,}", "\n\n", content_str)
    return formatted_content.splitlines(keepends=True)


def create_single_header(headers, system_dependencies, license_path, output_file):
    header_guard = "CLARGS_" + output_file.name.upper().replace(" ", "_").replace(".", "_")
    result = [
        f"#ifndef {header_guard}\n#define {header_guard}\n\n",
        "/**\n"
    ]
    for line in CLARGS_ASCII_ART.splitlines():
        result.append(f" *  {line}\n")

    with open(license_path, "r") as license_file:
        for line in license_file:
            result.append(f" *  {line}")
    result.append(" */\n\n")

    for header in sorted(system_dependencies):
        result.append(f"#include <{header}>\n")
    result.append("\n")

    for file, path in headers.items():
        with open(path, "r") as f:
            content = f.readlines()
            content = strip_include_guards(content)
            content = strip_header_includes(content)
            result.extend(content)
            result.append("\n")

    result.append(f"#endif // {header_guard}\n")

    result = remove_consecutive_newlines(result)

    with open(output_file, "w", newline="\n") as f:
        f.writelines(result)


def amalgamate(header_dir, license_path, output_file):
    headers = find_headers(header_dir)
    clargs_dependencies, system_dependencies = parse_dependencies(headers)
    ordered_headers = resolve_order(headers, clargs_dependencies)
    create_single_header(ordered_headers, system_dependencies, license_path, output_file)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Amalgamate all headers to a single library header")
    parser.add_argument("--header-dir",
                        default="./include/CLArgs",
                        help="Specify path to directory containing library headers",
                        type=pathlib.Path)
    parser.add_argument("--license",
                        default="./LICENSE",
                        help="Specify path to license file",
                        type=pathlib.Path)
    parser.add_argument("--output-dir",
                        default="./",
                        help="Specify path to target directory for amalgamated header",
                        type=pathlib.Path)
    args = parser.parse_args()

    os.makedirs(args.output_dir, exist_ok=True)

    amalgamate(args.header_dir, args.license, args.output_dir / "clargs.hpp")
