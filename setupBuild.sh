#!/bin/bash

cleanBuild=false
runBuild=false
embeddedBuild=false
debugBuild=false
runTests=false
singleJob=false
while [ $# -ne 0 ]; do
    arg="$1"
    case "$arg" in
        -clean)
            cleanBuild=true
            ;;
        -d)
            debugBuild=true
            ;;
        -t)
            runTests=true
            ;;
        -e)
            embeddedBuild=true
            ;;
        -r)
            runBuild=true
            ;;
        -s)
            singleJob=true
            ;;
        *)
            echo "Unsupported arg $arg"
            ;;
    esac
    shift
done

if { [ "$cleanBuild" == true ]; } && [ -d build ]; then
    echo "Performing clean build"
    rm -rdf build
fi
# Setup Build Directories
if [ ! -d build ]; then
    mkdir build
fi
cd build
# If -d flag is present, build in debug mode
if [ "$debugBuild" == true ]; then
    echo "RUNNING UNDER DEBUG MODE"
    ARGS="-DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=Debug"
else
    echo "RUNNING UNDER RELEASE MODE"
    ARGS="-Wno-dev -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=Release"
fi
if [ "$embeddedBuild" == true ]; then
    echo "Building EMBEDDED TARGET"
    ARGS="$ARGS -DGFX_EMBEDDED=1"
fi
cmake ${ARGS} ..

# Build Project
TYPE=`uname`
if [ "$singleJob" != true ]; then
    if [ ${TYPE} = "Darwin" ]; then
        make -j$(sysctl -n hw.physicalcpu)
    else
        make -j$(nproc)
    fi
else
    echo "Single threaded build mode enabled"
    make
fi
if [ $? != 0 ]; then
    echo -e "\033[0;31m --- Build errors detected! ---"
else
    if [ "$runTests" == true ]; then
        ctest --output-on-failure
    else
        if [ "$runBuild" == true ]; then
            # Run program
            cd ..
            ./engineExample
        fi
    fi
fi
