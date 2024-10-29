import os
import re
from collections import defaultdict, deque
import argparse
import pathlib

CLARGS_INCLUDE_PATTERN = re.compile(r'#include <CLArgs/(.+\.hpp)>')
SYSTEM_INCLUDE_PATTERN = re.compile(r'#include <(.+?)>')
CLARGS_GUARD_PATTERN = re.compile(r'#(ifndef|define|endif)')

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


class Amalgamator:
    def __init__(self, header_dir, license_path, output_file):
        self.__header_dir = header_dir
        self.__license_path = license_path
        self.__output_file: pathlib.Path = output_file
        self.__headers = {}
        self.__clargs_dependencies = defaultdict(set)
        self.__system_dependencies = set()

    def __find_headers(self):
        for root, _, files in os.walk(self.__header_dir):
            for file in files:
                if file.endswith(".hpp"):
                    path = os.path.join(root, file)
                    self.__headers[file] = path

    def __parse_dependencies(self):
        for file, path in self.__headers.items():
            with open(path, "r") as f:
                for line in f:
                    match = CLARGS_INCLUDE_PATTERN.search(line)
                    if match:
                        include_file = match.group(1)
                        if include_file in self.__headers:
                            self.__clargs_dependencies[file].add(include_file)
                        continue
                    match = SYSTEM_INCLUDE_PATTERN.search(line)
                    if match:
                        self.__system_dependencies.add(match.group(1))

    def __resolve_order(self):
        indegree = {file: 0 for file in self.__headers}
        for deps in self.__clargs_dependencies.values():
            for dep in deps:
                indegree[dep] += 1

        queue = deque([file for file in self.__headers if indegree[file] == 0])
        ordered_files = []

        while queue:
            file = queue.popleft()
            ordered_files.append(file)
            for dep in self.__clargs_dependencies[file]:
                indegree[dep] -= 1
                if indegree[dep] == 0:
                    queue.append(dep)

        if len(ordered_files) != len(self.__headers):
            raise RuntimeError("Circular dependency detected among headers.")

        ordered_files.reverse()

        self.__headers = {file: self.__headers[file] for file in ordered_files}

    @staticmethod
    def __strip_header_includes(content):
        result = []
        for line in content:
            if not re.search(CLARGS_INCLUDE_PATTERN, line) and not re.search(SYSTEM_INCLUDE_PATTERN, line):
                result.append(line)
        return result

    @staticmethod
    def __strip_include_guards(content):
        result = []
        include_guard_pattern = re.compile(r'^\s*#(ifndef|define)\s+CLARGS_(\w+)_HPP\s*$')
        close_guard_pattern = re.compile(r'^\s*#endif\s*//\s*CLARGS_(\w+)_HPP\s*$')
        open_guard = None

        for line in content:
            if include_guard_pattern.match(line):
                open_guard = True
                continue
            if close_guard_pattern.match(line) and open_guard:
                open_guard = None
                continue
            result.append(line)

        return result

    @staticmethod
    def __format_content(content):
        formatted_content = []
        encountered_blank = False
        for line in content:
            if line == "" or line.isspace():
                if encountered_blank:
                    continue
                encountered_blank = True
                formatted_content.append("\n")
                continue
            encountered_blank = False
            formatted_content.append(line)
        return formatted_content

    def __create_single_header(self):
        header_guard = "CLARGS_" + self.__output_file.name.upper().replace(" ", "_").replace(".", "_")
        result = [
            f"#ifndef {header_guard}\n#define {header_guard}\n\n",
            "/**\n"
        ]
        for line in CLARGS_ASCII_ART.splitlines():
            result.append(f" *  {line}\n")

        with open(self.__license_path, "r") as license_file:
            for line in license_file:
                result.append(f" *  {line}")
        result.append(" */\n\n")

        for header in sorted(self.__system_dependencies):
            result.append(f'#include <{header}>\n')
        result.append("\n")

        for file, path in self.__headers.items():
            with open(path, "r") as f:
                content = f.readlines()
                content = self.__strip_include_guards(content)
                content = self.__strip_header_includes(content)
                result.extend(content)
                result.append("\n")

        result.append(f"#endif // {header_guard}\n")
        return result

    def __save_header(self, formatted_content):
        with open(self.__output_file, "w", newline='\n') as f:
            f.writelines(formatted_content)
        print(f"Successfully created {self.__output_file}")

    def amalgamate(self):
        self.__find_headers()
        self.__parse_dependencies()
        self.__resolve_order()
        result = self.__create_single_header()
        formatted_content = self.__format_content(result)
        self.__save_header(formatted_content)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Amalgamate all headers to a single library header')
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

    amalgamator = Amalgamator(args.header_dir, args.license, args.output_dir / "clargs.hpp")
    amalgamator.amalgamate()
