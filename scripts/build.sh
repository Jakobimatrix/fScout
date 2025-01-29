#!/bin/bash

# compile for linux debug
# ./build_script.sh
# compile for linux release
# ./build_script.sh -r
# compile for testing
# ./build_script.sh -t
# compile for release and testing
# ./build_script.sh -rt
# compile for linux release compare
# ./build_script.sh -c

buildType="Debug"
buildFlags=""
build_folder="../build"

# Parse command-line arguments
while getopts "rtc" flag
do
    case "${flag}" in
        r)
          build_folder="../build_release"
          buildType="Release"
          ;;
        c)
          build_folder="../build_compare_release"
          buildType="Release"
          ;;
        t)
          buildFlags="-DTEST_SETTINGS=ON -DTEST_UTIL=ON"
          ;;
    esac
done

# Create build directory if it doesn't exist
mkdir -p "$build_folder"
cd "$build_folder"

# Construct the cmake command
cmake_command="cmake .. -DCMAKE_BUILD_TYPE=$buildType $buildFlags"

# Run cmake and make
echo "Running: $cmake_command"
$cmake_command

# Compile with parallel jobs
make -j8

# Run tests if build was successful
ctest --output-on-failure
