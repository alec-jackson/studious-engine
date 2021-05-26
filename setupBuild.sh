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
cmake -Wno-dev ..

# Build Project
make
if [ $? != 0 ]; then
    echo -e "\033[0;31m --- Build errors detected! ---"
else
    # Run program
    cd ..
    ./engineExample
fi

