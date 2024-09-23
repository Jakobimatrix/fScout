#!/bin/bash

# compile for linux debug
# ./build_script.sh
# compile for linux release
# ./build_script.sh -r


build_folder="../build"
cmake_command="cmake -DCMAKE_BUILD_TYPE=Debug .."
toolchain_file=""
qt5_dir=""
while getopts r flag
do
    case "${flag}" in
        r)
          build_folder="../build_release"
          cmake_command="cmake -DCMAKE_BUILD_TYPE=Release ..";;
    esac
done

mkdir -p "$build_folder"
cd "$build_folder"

cmake $cmake_command
make -j8
