#!/usr/bin/env python3

import argparse
import os
import shutil
import subprocess
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
BUILD_DIR = os.path.join(SCRIPT_DIR, "build")


def run(cmd, cwd=None):
    print(f"  {' '.join(cmd)}")
    result = subprocess.run(cmd, cwd=cwd)
    if result.returncode != 0:
        sys.exit(result.returncode)


def clean():
    if os.path.exists(BUILD_DIR):
        shutil.rmtree(BUILD_DIR)
        print("Cleaned build directory.")
    else:
        print("Nothing to clean.")


def build(build_type="Release"):
    os.makedirs(BUILD_DIR, exist_ok=True)
    run(["cmake", "..", f"-DCMAKE_BUILD_TYPE={build_type}"], cwd=BUILD_DIR)
    nproc = str(os.cpu_count() or 1)
    run(["make", f"-j{nproc}"], cwd=BUILD_DIR)


def test():
    exe = os.path.join(BUILD_DIR, "simd_img_tests")
    if not os.path.exists(exe):
        print("Test binary not found. Run with -b first.")
        sys.exit(1)
    run([exe])


def main():
    parser = argparse.ArgumentParser(description="Build script for simd-img")
    parser.add_argument("-b", "--build", action="store_true", help="configure and compile")
    parser.add_argument("-c", "--clean", action="store_true", help="remove build directory")
    parser.add_argument("-t", "--test", action="store_true", help="run unit tests")
    parser.add_argument("-r", "--rebuild", action="store_true", help="clean + build")
    parser.add_argument("--debug", action="store_true", help="build in Debug mode")
    args = parser.parse_args()

    if not any([args.build, args.clean, args.test, args.rebuild]):
        parser.print_help()
        sys.exit(1)

    build_type = "Debug" if args.debug else "Release"

    if args.clean or args.rebuild:
        clean()

    if args.build or args.rebuild:
        build(build_type)

    if args.test:
        test()


if __name__ == "__main__":
    main()
