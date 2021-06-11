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
cmake -Wno-dev -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ..

# Build Project
TYPE=`uname`
if [ TYPE = "Darwin" ]; then
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
