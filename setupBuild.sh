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
cmake ..

# Build Project
make

# Run program
cd ..
./engineExample
