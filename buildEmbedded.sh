#!/bin/bash

# Compile libraries
#cd misc/yaml-cpp-0.6.3/
#mkdir build
#cd build
#cmake ..
#make
#cd ../../../

if { [ "$1" = "-clean" ] || [ "$2" = "-clean" ]; } && [ -d build ]; then
    echo "Performing clean build"
    rm -rdf build
fi
# Setup Build Directories
if [ ! -d build ]; then
    mkdir build
fi
cd build
# If -d flag is present, build in debug mode
if [ "$1" = "-d" ] || [ "$2" = "-d" ]; then
    echo "RUNNING UNDER DEBUG MODE"
    ARGS="-DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=Debug -DGFX_EMBEDDED=1"
else
    echo "RUNNING UNDER RELEASE MODE"
    ARGS="-Wno-dev -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=Release -DGFX_EMBEDDED=1"
fi
cmake ${ARGS} ..

# Build Project
TYPE=`uname`
if [ ${TYPE} = "Darwin" ]; then
    make -j$(sysctl -n hw.physicalcpu)
else
    #make -j$(nproc) - May be memory limited. Use a single core
    make
fi
if [ $? != 0 ]; then
    echo -e "\033[0;31m --- Build errors detected! ---"
else
    if [ "$1" = "-t" ] || [ "$2" = "-t" ]; then
        ctest --output-on-failure
    else
        # Start an X server to run the program
        cd ..
        startx $(pwd)/engineExample > studious.log 2> error.log
        echo "$?"
    fi
fi
