#!/bin/bash
build_dir="../build"
set -e
if [[ $EUID -ne 0 ]]; then
   echo -e "\e[31mError. Super user privileges are required.\e[0m"
   echo -e "Type \e[34msudo $0\e[0m"
   exit 1
fi
cd "$(dirname "$0")"
cd "$build_dir"
make install > "$build_dir/install.log"
echo -e "\e[32mInstallation completed successfully.\e[0m"