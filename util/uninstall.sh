#!/bin/bash
build_dir="../build"
spice3f5_libs_dir="/usr/local/lib/spice3f5"
set -e

if [[ $EUID -ne 0 ]]; then
   echo -e "\e[31mError. Super user privileges are required.\e[0m"
   echo -e "Type \e[34msudo $0\e[0m"
   exit 1
fi

cd "$(dirname "$0")"
cd "$build_dir"

for file in $(cat install_manifest.txt) ; do
  rm "$file"
done
rm $spice3f5_libs_dir -r

echo -e "\e[32mUninstall completed successfully.\e[0m"