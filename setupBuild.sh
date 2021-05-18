# Compile libraries
cd misc/yaml-cpp-0.6.3/
mkdir build
cd build
cmake ..
make
cd ../../../

# Setup Build Directories
mkdir build
cd build
cmake ..

# Build Project
make
