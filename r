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
    
    # Run the executable with default resolution if not specified
    if [ -f "Bin/CppSFMLVisualizer" ]; then
        # Check if --width or --height are already specified
        if [[ ! "$@" =~ --width ]] && [[ ! "$@" =~ --height ]]; then
            # Add default resolution
            ./Bin/CppSFMLVisualizer --width 800 --height 600 "$@"
        else
            # Use provided arguments as-is
            ./Bin/CppSFMLVisualizer "$@"
        fi
    else
        echo -e "${RED}Executable not found at Bin/CppSFMLVisualizer${NC}"
        exit 1
    fi
else
    echo -e "${RED}Build failed, cannot run${NC}"
    exit 1
fi