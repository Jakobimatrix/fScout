#!/bin/bash
name="fScout_GUI"

build_folder="../build"
while getopts "rc" flag
do
    case "${flag}" in
        r) build_folder="../build_release";;
        c) build_folder="../build_compare_release";;
    esac
done

exe="$build_folder/src/executable/$name"
if [ -e $exe ]; then
    exec $exe
else
    echo "Cannot find executable $exe."
fi
