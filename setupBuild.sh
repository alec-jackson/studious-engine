# Compile libraries
cd misc/yaml-cpp-0.6.3/
mkdir build
cd build
cmake ..
make
cd ..
mv libyaml-cpp.a ../../lib/yaml/libyaml-cpp.a
mkdir ../../lib/yaml/include
cp -r include/yaml-cpp/* ../../lib/yaml/include/
cd ../../

# Setup Build Directories
mkdir build
cd build
cmake ..

# Build Project
make
