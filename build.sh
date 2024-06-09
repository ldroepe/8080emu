#!/usr/bin/env bash

build_dir=./build
src_dir=./src

test -d "$build_dir" || mkdir "$build_dir"
cmake -S "$src_dir" -B "$build_dir"
cmake --build "$build_dir"
