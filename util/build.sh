#!/bin/bash
build_dir="../build"
set -e
cd "$(dirname "$0")"
[ -d "$build_dir" ] || mkdir "$build_dir"
cd "$build_dir"
cmake .. -Wno > "$build_dir/cmake.log"
make > "$build_dir/build.log"
echo -e "\e[32mBuild completed successfully.\e[0m"