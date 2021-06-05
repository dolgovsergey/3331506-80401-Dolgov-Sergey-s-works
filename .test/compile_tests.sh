#!/bin/bash

echo "########################################"
echo "Using gcc to compile"

# tell PVS Studio that this is an academic project to check it for free
how-to-use-pvs-studio-free -c 1 ./src 

export GCC_COLORS='error=01;31:warning=01;35:note=01;36:caret=01;32:locus=01:quote=01'

# build with json export, it will be used in the pvs analysis later
# CMakeLists.txt is unconviently in .test directory
# also supressing useless progress messages
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=On -S ./.test >/dev/null
cmake --build . >/dev/null 

retVal=$?
if [ $retVal -ne 0 ]; then
    echo -e "\e[41mCompilation error!\e[0m"
else
    echo -e "\e[30;42mCompilation success\e[0m"
fi
exit $retVal




