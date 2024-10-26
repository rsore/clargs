import os
import re
from collections import defaultdict, deque
import argparse
import pathlib

CLARGS_INCLUDE_PATTERN = re.compile(r'#include <CLArgs/(.+\.hpp)>')
CLARGS_GUARD_PATTERN = re.compile(r'#(ifndef|define|endif)')


class Amalgamator:
    @staticmethod
    def find_headers(header_dir):
        headers = {}
        for root, _, files in os.walk(header_dir):
            for file in files:
                if file.endswith(".hpp"):
                    path = os.path.join(root, file)
                    headers[file] = path
        return headers

    @staticmethod
    def parse_dependencies(headers):
        dependencies = defaultdict(set)
        for file, path in headers.items():
            with open(path, "r") as f:
                for line in f:
                    match = CLARGS_INCLUDE_PATTERN.search(line)
                    if match:
                        include_file = match.group(1)
                        if include_file in headers:
                            dependencies[file].add(include_file)
        return dependencies

    @staticmethod
    def resolve_order(headers, dependencies):
        in_degree = {file: 0 for file in headers}
        for deps in dependencies.values():
            for dep in deps:
                in_degree[dep] += 1

        queue = deque([file for file in headers if in_degree[file] == 0])
        ordered_files = []

        while queue:
            file = queue.popleft()
            ordered_files.append(file)
            for dep in dependencies[file]:
                in_degree[dep] -= 1
                if in_degree[dep] == 0:
                    queue.append(dep)
        ordered_files.reverse()

        if len(ordered_files) != len(headers):
            raise RuntimeError("Circular dependency detected among headers.")
        return ordered_files

    @staticmethod
    def strip_includes_and_include_guards(content):
        stripped_lines = []
        for line in content:
            if not CLARGS_GUARD_PATTERN.search(line) and not CLARGS_INCLUDE_PATTERN.search(line):
                stripped_lines.append(line)
        return stripped_lines

    @staticmethod
    def create_single_header(headers, order, output_file):
        with open(output_file, "w") as outfile:
            outfile.write("#ifndef CLARGS_CLARGS_HPP\n#define CLARGS_CLARGS_HPP\n\n")
            for file in order:
                with open(headers[file], "r") as f:
                    content = f.readlines()
                    stripped_content = Amalgamator.strip_includes_and_include_guards(content)
                    outfile.writelines(stripped_content)
                    outfile.write("\n")
            outfile.write("#endif\n")

    @staticmethod
    def amalgamate(header_dir, output_file):
        print("Config:")
        print(f"  Header directory: {header_dir}")
        print(f"  Output file:      {output_file}")
        print("")

        print("Detecting headers...")
        headers = Amalgamator.find_headers(header_dir)
        print("Headers detected:")
        for header in headers:
            print(f" - {header}")
        print("")

        print("Parsing header dependencies...")
        header_dependencies = Amalgamator.parse_dependencies(headers)
        print("Header dependencies detected:")
        for header, dependencies in header_dependencies.items():
            print(f" - {header}")
            for dependency in dependencies:
                print(f"    - {dependency}")
        print("")

        print("Resolving header order...")
        order = Amalgamator.resolve_order(headers, header_dependencies)
        print("Header order:")
        for header in order:
            print(f" - {header}")
        print("")

        print(f"Creating amalgamated header file {output_file}...")
        Amalgamator.create_single_header(headers, order, output_file)
        print(f"Successfully created {output_file}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Amalgamate all headers to a single library header')
    parser.add_argument("--header-dir",
                        default="./include/CLArgs",
                        help="Specify path to directory containing library headers",
                        type=pathlib.Path)
    parser.add_argument("-o", "--output",
                        default="./clargs.hpp",
                        help="Specify filepath used to generate final header",
                        type=pathlib.Path)
    args = parser.parse_args()

    Amalgamator.amalgamate(args.header_dir, args.output)
