#!/bin/bash
build_folder="../build"
while getopts r: flag
do
    case "${flag}" in
        r) 
	build_folder="../build_release"
    esac
done

if [ -d "$build_folder" ];then

  rm -rf $build_folder/*
fi

/bin/bash build.sh
