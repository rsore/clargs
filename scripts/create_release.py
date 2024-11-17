from amalgamate import amalgamate
from logger import logger

import argparse
from pathlib import Path
import shutil
import re
import zipfile
import os


def validate_version(version: str):
    version_pattern = r'^[0-9]+\.[0-9]+\.[0-9]+$'

    logger.verbose_log("Validating version...")
    if not re.match(version_pattern, version):
        raise ValueError(f"Invalid version format: '{version}'. Must match pattern '[0-9]+.[0-9]+.[0-9]+'")
    logger.verbose_log(f"Version '{version}' is valid")

def copy_file(source: Path, dest: Path):
    logger.verbose_log(f"Copying file '{source}' -> '{dest}'")
    shutil.copy(source, dest)

def mkdir(directory: Path):
    logger.verbose_log(f"Creating directory {directory}")
    directory.mkdir(parents=True, exist_ok=True)

def get_cmake_list_content(version: str):
    return f"""
cmake_minimum_required(VERSION 3.20)

project(CLArgs VERSION {version})

add_library(CLArgs INTERFACE
    ${{CMAKE_CURRENT_LIST_DIR}}/include/CLArgs/clargs.hpp
)

target_include_directories(CLArgs INTERFACE 
    ${{CMAKE_CURRENT_LIST_DIR}}/include
)
""".lstrip('\n')

def write_file(dest: Path, content: str):
    logger.verbose_log(f"Creating file '{dest}'")
    with open(dest, 'w', newline="\n") as f:
        f.write(content)

def create_release_dir(output_dir: Path, version: str, source_headers_dir: Path, license_path: Path, readme_path: Path):
    release_dir = output_dir / f"CLArgs-{version}"
    mkdir(release_dir)

    header_dest = release_dir / "include" / "CLArgs"
    mkdir(header_dest)

    copy_file(license_path, release_dir / "LICENSE")
    copy_file(readme_path, release_dir / "README.md")

    write_file(release_dir / "CMakeLists.txt", get_cmake_list_content(version))

    dest_clargs_file = release_dir / "include" / "CLArgs" / "clargs.hpp"
    logger.verbose_log(f"Creating header file '{dest_clargs_file}'")
    amalgamate(source_headers_dir, license_path, False, dest_clargs_file, False, version)

    return release_dir


def create_zip_archive(output_dir: Path, version: str, release_dir: Path):
    zip_file = output_dir / f"CLArgs-{version}.zip"
    logger.verbose_log(f"Creating zip archive: {zip_file}")

    with zipfile.ZipFile(zip_file, 'w', zipfile.ZIP_DEFLATED) as zipf:
        for root, dirs, files in os.walk(release_dir):
            for file in files:
                zipf.write(str(os.path.join(root, file)),
                           os.path.relpath(str(os.path.join(root, file)), release_dir))

    return zip_file


def main():
    parser = argparse.ArgumentParser(description="Release script for CLArgs")
    parser.add_argument("--version",
                        required=True,
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

    logger.verbose = args.verbose
    logger.verbose_log("Verbose output is enabled")

    validate_version(args.version)

    if not args.output_dir.exists():
        logger.verbose_log(f"Creating output directory {args.output_dir}")
        args.output_dir.mkdir(parents=True, exist_ok=True)

    release_dir = create_release_dir(args.output_dir, args.version, args.header_dir, args.license, args.readme)
    zip_file = create_zip_archive(args.output_dir, args.version, release_dir)
    logger.log(f"Successfully created release zip file '{zip_file}'")


if __name__ == "__main__":
    main()
