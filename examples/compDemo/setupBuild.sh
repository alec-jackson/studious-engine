#!/bin/bash

cleanBuild=false
runBuild=false
embeddedBuild=false
debugBuild=false
runTests=false
singleJob=false
buildAll=false
target=yourProjectName
while [ $# -ne 0 ]; do
    arg="$1"
    case "$arg" in
        -clean)
            cleanBuild=true
            ;;
        -a)
            buildAll=true
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
            buildAll=true
            runBuild=true
            ;;
        -s)
            singleJob=true
            ;;
        -target)
            # This just modifies the run target, does not affect compilation
            shift
            target="$1"
            ;;
        *)
            echo "Unsupported arg $arg"
            ;;
    esac
    shift
done

echo "Target: $target"

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
if [ "$buildAll" == true ]; then
    echo "Building with Examples"
    ARGS="$ARGS -DEXAMPLES=1"
fi

cmake ${ARGS} ..

# Build Project
TYPE=`uname`
JOBS=1
if [ "$singleJob" != true ]; then
    if [ ${TYPE} = "Darwin" ]; then
        JOBS=$(sysctl -n hw.physicalcpu)
    else
        JOBS=$(nproc)
    fi
fi
echo "Building with ${JOBS} threads."
cmake --build . -j ${JOBS}
if [ $? != 0 ]; then
    echo -e "\033[0;31m --- Build errors detected! ---"
    tput init
else
    if [ "$runTests" == true ]; then
        ctest --output-on-failure
    else
        if [ "$runBuild" == true ]; then
            # Run program
            cd ..
            ./$target
        fi
    fi
fi
