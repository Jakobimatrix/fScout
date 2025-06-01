#!/bin/bash

set -e

show_help() {
    echo "Usage: ./build.sh [options]"
    echo "Options:"
    echo "  -c              Clean build"
    echo "  -d              Debug build"
    echo "  -r              Release build"
    echo "  -o              RelWithDebInfo build"
    echo "  -i              Install after build"
    echo "  --compiler COMP Use specific compiler (e.g. gcc, clang)"
    echo "  -h              Show this help message"
    echo "  -t              Enable tests"
    echo "  -f              Enable fuzzing"
    echo "  -v              Verbode: dump CMake variables"
    echo "  -l              List available compilers"
    exit 0
}

# Defaults
CLEAN=false
INSTALL=false
BUILD_TYPE=""
COMPILER=""
ENABLE_TESTS=OFF
ENABLE_FUZZING=OFF
LIST_COMPILERS=false
VERBOSE=false
ARGS=()
COMPILER="g++"

CLANG_CPP_PATH="/usr/bin/clang++-19"
CLANG_C_PATH="/usr/bin/clang-19"
GCC_CPP_PATH="/usr/bin/g++-13"
GCC_C_PATH="/usr/bin/gcc-13"


list_available_compiler() {
    for base in gcc g++ clang clang++; do
        echo ""
        echo "=== $base ==="
        # Find unversioned
        if command -v "$base" &>/dev/null; then
            echo "  $(command -v $base)"
        fi
        # Find versioned (handle pluses for g++ and clang++)
        if [[ "$base" == "g++" || "$base" == "clang++" ]]; then
            compgen -c | grep -E "^${base//+/\\+}-[0-9]+$" | sort -V | while read -r ver; do
                if command -v "$ver" &>/dev/null; then
                    echo "  $(command -v $ver)"
                fi
            done
        else
            compgen -c | grep -E "^${base}-[0-9]+$" | sort -V | while read -r ver; do
                if command -v "$ver" &>/dev/null; then
                    echo "  $(command -v $ver)"
                fi
            done
        fi
    done
}

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -c) CLEAN=true ;;
        -d) BUILD_TYPE="Debug" ;;
        -r) BUILD_TYPE="Release" ;;
        -o) BUILD_TYPE="RelWithDebInfo" ;;
        -i) INSTALL=true ;;
        --compiler)
            shift
            COMPILER="$1"
            ;;
        -t) ENABLE_TESTS=ON ;;
        -f) ENABLE_FUZZING=ON ;;
        -v) VERBOSE=true ;;
        -l) LIST_COMPILERS=true ;;
        -h) show_help ;;
        *)
            echo "Unknown option: $1"
            show_help
            ;;
    esac
    shift
done

if [[ "$LIST_COMPILERS" == true ]]; then
    list_available_compiler
    if [[ -z "$BUILD_TYPE" ]]; then
        exit 0
    fi
fi

# Validate build type
if [[ -z "$BUILD_TYPE" ]]; then
    echo "Error: You must specify a build type (-d, -r, or -o)"
    show_help
    exit 1
fi

# Normalize to correct C++ compiler
if [[ "$COMPILER" == "gcc" ]]; then
    COMPILER="g++"
elif [[ "$COMPILER" == "clang" ]]; then
    COMPILER="clang++"
fi

# set compiler paths
if [[ "$COMPILER" == "g++" ]]; then
    COMPILER_PATH="$GCC_CPP_PATH"
    CC_PATH="$GCC_C_PATH"
    COMPILER_NAME="gcc"
elif [[ "$COMPILER" == "clang++" ]]; then
    COMPILER_PATH="$CLANG_CPP_PATH"
    CC_PATH="$CLANG_C_PATH"
    COMPILER_NAME="clang"
else
    echo "Error: $COMPILER not valid input."
    exit 1
fi

if [ ! -f "$COMPILER_PATH" ]; then
    echo "$COMPILER_PATH not found! Check the paths variables at the begin of this script!"
    list_available_compiler
    exit 1
fi
if [ ! -f "$CC_PATH" ]; then
    echo "$CC_PATH not found! Check the paths variables at the begin of this script!"
    list_available_compiler
    exit 1
fi


# Validate fuzzer option
if [[ "$ENABLE_FUZZING" == "ON" && "$COMPILER" != "clang++" ]]; then
    echo "Error: Fuzzing (-f) is only supported with the clang compiler."
    exit 1
fi



BUILD_DIR="build-${COMPILER_NAME,,}-${BUILD_TYPE,,}"

# Clean build directory if requested
if [[ "$CLEAN" == true ]]; then
    echo "Cleaning build directory: $BUILD_DIR"
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Run CMake and build
echo "Using cpp compiler at: $COMPILER_PATH"
echo "Using c compiler at: $CC_PATH"
echo "To change compiler versions, set the variables in this script!!"
echo "Configuring with CMake..."
echo "Running: cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_CXX_COMPILER=$COMPILER_PATH -DCMAKE_C_COMPILER=$CC_PATH -DBUILD_TESTING=$ENABLE_TESTS -DENABLE_FUZZING=$ENABLE_FUZZING .."
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_CXX_COMPILER=$COMPILER_PATH -DCMAKE_C_COMPILER=$CC_PATH -DBUILD_TESTING=$ENABLE_TESTS -DENABLE_FUZZING=$ENABLE_FUZZING ..
if [[ "$VERBOSE" == true ]]; then
    echo "Dumping CMake variables:"
    cmake -LAH ..
fi
echo "Building project..."
cmake --build . -- -j$(nproc)


# Run tests if enabled
if [[ "$ENABLE_TESTS" == "ON" ]]; then
    echo "Running ctest --output-on-failure"
    ctest --output-on-failure

    # Check if any tests were found
    NUM_TESTS=$(ctest -N | grep -c "Test #[0-9]\+:" || true)
    if [[ "$NUM_TESTS" -eq 0 ]]; then
        echo "ERROR: No tests were found or executed!"
        exit 2
    fi
fi

# Install if requested
if [[ "$INSTALL" == true ]]; then
    echo "cmake --install ."
    cmake --install .
fi

