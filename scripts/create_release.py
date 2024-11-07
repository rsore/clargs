from amalgamate import amalgamate

import argparse
from pathlib import Path
import shutil
import re
import zipfile
import os

RELEASE_VERBOSE = False


def release_verbose_log(msg):
    if RELEASE_VERBOSE:
        print(msg)


def validate_version(version: str):
    version_pattern = r'^[0-9]+\.[0-9]+\.[0-9]+$'

    if not re.match(version_pattern, version):
        raise ValueError(f"Invalid version format: '{version}'. Must match pattern '[0-9]+.[0-9]+.[0-9]+'")
    else:
        release_verbose_log(f"Version '{version}' is valid")

def create_release_dir(output_dir, version, source_headers_dir, license_path, readme_path):
    release_dir = Path(output_dir) / f"CLArgs-{version}"
    release_dir.mkdir(parents=True, exist_ok=True)

    header_dest = release_dir / "include" / "CLArgs"
    header_dest.mkdir(parents=True, exist_ok=True)

    shutil.copy(license_path, release_dir / "LICENSE")

    shutil.copy(readme_path, release_dir / "README.md")

    cmake_lists = f"""cmake_minimum_required(VERSION 3.20)

project(CLArgs VERSION {version})

add_library(CLArgs INTERFACE
    ${{CMAKE_CURRENT_LIST_DIR}}/include/CLArgs/clargs.hpp
)
target_include_directories(CLArgs INTERFACE 
    ${{CMAKE_CURRENT_LIST_DIR}}/include
)
"""

    with open(release_dir / 'CMakeLists.txt', 'w', newline="\n") as f:
        f.write(cmake_lists)

    amalgamate(source_headers_dir, license_path, False, release_dir / "include" / "CLArgs" / "clargs.hpp")

    return release_dir

def create_zip_archive(output_dir, version, release_dir):
    zip_filename = f"{output_dir}/CLArgs-{version}.zip"
    release_verbose_log(f"Creating zip archive: {zip_filename}")

    with zipfile.ZipFile(zip_filename, 'w', zipfile.ZIP_DEFLATED) as zipf:
        for root, dirs, files in os.walk(release_dir):
            for file in files:
                zipf.write(str(os.path.join(root, file)),
                           os.path.relpath(str(os.path.join(root, file)), release_dir))

    print(f"Release package created: {zip_filename}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Release script for CLArgs")
    parser.add_argument("--version", required=True,
                        help="The version of the release")
    parser.add_argument("--header-dir",
                        default="./include/CLArgs",
                        help="Path to the CLArgs source headers",
                        type=Path)
    parser.add_argument("--license",
                        default="./LICENSE",
                        help="Path to the license file",
                        type=Path)
    parser.add_argument("--readme",
                        default="./README.md",
                        help="Path to the README file",
                        type=Path)
    parser.add_argument("--output-dir",
                        default=".",
                        help="Directory to output the release package",
                        type=Path)
    parser.add_argument("-f", "--force", action="store_true")
    parser.add_argument("-v", "--verbose", action="store_true")
    args = parser.parse_args()

    if args.verbose:
        RELEASE_VERBOSE = True
        release_verbose_log("Verbose output is enabled")

    validate_version(args.version)

    if not args.output_dir.exists():
        release_verbose_log(f"Creating output directory {args.output_dir}")
        args.output_dir.mkdir(parents=True, exist_ok=True)

    release_dir = create_release_dir(args.output_dir, args.version, args.header_dir, args.license, args.readme)
    create_zip_archive(args.output_dir, args.version, release_dir)
