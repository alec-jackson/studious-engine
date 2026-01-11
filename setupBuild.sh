#!/bin/sh

cleanBuild=false
runBuild=false
embeddedBuild=false
debugBuild=false
runTests=false
singleJob=false
buildAll=false
installLib=false
filter_tests=false
physThreads=1
target=studious-3dExampleScene
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
        -tf)
            shift
            test_filter="$1"
            filter_tests=true
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
        -physThreads)
            shift
            physThreads="$1"
            ;;
        -i)
            # Installs the library on your system automatically
            installLib=true
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
if [ "$runTests" == true ]; then
    echo "Compiling tests"
    ARGS="$ARGS -DRUNTEST=1"
fi
# Pass phys threads through
ARGS="$ARGS -DPHYS_THREADS=$physThreads"

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
    if [ "$installLib" == true ]; then
        echo "Installing studious library files"
        sudo cmake --install .
    fi
    if [ "$runTests" == true ]; then
        if [ "$filter_tests" == true ]; then
            ctest --output-on-failure -j 4 -R $test_filter
        else
            ctest --output-on-failure -j 4
        fi
    else
        if [ "$runBuild" == true ]; then
            # Run program
            cd ..
            ./$target
        fi
    fi
fi
