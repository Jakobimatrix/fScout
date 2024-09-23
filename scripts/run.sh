#!/bin/bash
name="finder_start"

build_folder="../build"
while getopts r: flag
do
    case "${flag}" in
        r) build_folder="../build_release";;
    esac
done

exe="$build_folder/src/executable/$name"
if [ -e $exe ]; then
    exec $exe
else
    echo "Cannot find executable $exe."
fi
