#!/bin/bash

# Build script for CppSFMLVisualizer
# Usage: ./b [build_type] [additional_cmake_args...]
# Example: ./b Debug -DCMAKE_VERBOSE_MAKEFILE=ON

BUILD_TYPE=${1:-Release}
shift # Remove first argument to pass the rest to cmake

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${YELLOW}Building CppSFMLVisualizer in ${BUILD_TYPE} mode...${NC}"

# Create build directory
BUILD_DIR="build/${BUILD_TYPE}"
mkdir -p "$BUILD_DIR"

# Check if ninja is available
if command -v ninja &> /dev/null; then
    GENERATOR="-G Ninja"
    BUILD_TOOL="ninja"
    echo -e "${BLUE}Using Ninja build system${NC}"
else
    GENERATOR=""
    BUILD_TOOL="make"
    echo -e "${BLUE}Ninja not found, using Make${NC}"
fi

# Check if clang++ is available
if command -v clang++ &> /dev/null; then
    CXX_COMPILER="-DCMAKE_CXX_COMPILER=clang++"
    echo -e "${BLUE}Using clang++ compiler${NC}"
else
    CXX_COMPILER=""
    echo -e "${BLUE}clang++ not found, using default compiler${NC}"
fi

# Configure with CMake
echo -e "${GREEN}Configuring CMake...${NC}"
if cmake -S . -B "$BUILD_DIR" \
    $GENERATOR \
    $CXX_COMPILER \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    "$@"; then
    echo -e "${GREEN}Configuration successful!${NC}"
else
    echo -e "${RED}Configuration failed!${NC}"
    exit 1
fi

# Build
echo -e "${GREEN}Building project...${NC}"
if cmake --build "$BUILD_DIR" -j$(nproc); then
    echo -e "${GREEN}Build successful!${NC}"
    echo -e "${GREEN}Executable location: Bin/CppSFMLVisualizer${NC}"
else
    echo -e "${RED}Build failed!${NC}"
    exit 1
fi