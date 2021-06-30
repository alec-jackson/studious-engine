#!/bin/bash

# Compile libraries
#cd misc/yaml-cpp-0.6.3/
#mkdir build
#cd build
#cmake ..
#make
#cd ../../../

# Setup Build Directories
if [ ! -d build ]; then
    mkdir build
fi
cd build
# If -d flag is present, build in debug mode
if [ "$1" = "-d" ]; then
    echo "RUNNING UNDER DEBUG MODE"
    ARGS="-DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=Debug"
else
    echo "RUNNING UNDER RELEASE MODE"
    ARGS="-Wno-dev -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=Release"
fi
cmake ${ARGS} ..

# Build Project
TYPE=`uname`
if [ ${TYPE} = "Darwin" ]; then
    make -j$(sysctl -n hw.physicalcpu)
else
    make -j$(nproc)
fi
if [ $? != 0 ]; then
    echo -e "\033[0;31m --- Build errors detected! ---"
else
    # Run program
    cd ..
    ./engineExample
fi
