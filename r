#!/bin/bash

# Run script for CppSFMLVisualizer
# Usage: ./r [build_type] [program_args...]
# Example: ./r Debug --fullscreen

BUILD_TYPE=${1:-Release}
shift # Remove first argument to pass the rest to the program

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# First, build the project
echo -e "${YELLOW}Building before running...${NC}"
if ./b "$BUILD_TYPE"; then
    echo -e "${GREEN}Build successful, running CppSFMLVisualizer...${NC}"
    echo "----------------------------------------"
    
    # Run the executable
    if [ -f "Bin/CppSFMLVisualizer" ]; then
        ./Bin/CppSFMLVisualizer "$@"
    else
        echo -e "${RED}Executable not found at Bin/CppSFMLVisualizer${NC}"
        exit 1
    fi
else
    echo -e "${RED}Build failed, cannot run${NC}"
    exit 1
fi