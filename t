#!/bin/bash

# Test script for CppSFMLVisualizer
# Usage: ./t [build_type] [test_args...]
# Example: ./t Debug --success

BUILD_TYPE=${1:-Release}
shift # Remove first argument to pass the rest to the test executable

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${YELLOW}Building and running tests...${NC}"

# First, build the project
if ./b "$BUILD_TYPE"; then
    echo -e "${GREEN}Build successful${NC}"
    echo "----------------------------------------"
    
    TEST_EXECUTABLE="build/${BUILD_TYPE}/Test/CppSFMLVisualizerTests"
    
    if [ -f "$TEST_EXECUTABLE" ]; then
        echo -e "${BLUE}Running tests...${NC}"
        echo ""
        
        # Run the tests with any additional arguments
        "$TEST_EXECUTABLE" "$@"
        
        TEST_RESULT=$?
        
        echo ""
        echo "----------------------------------------"
        
        if [ $TEST_RESULT -eq 0 ]; then
            echo -e "${GREEN}All tests passed!${NC}"
            
            # Optionally run with detailed output
            if [[ ! " $@ " =~ " -s " ]] && [[ ! " $@ " =~ " --success " ]]; then
                echo -e "${YELLOW}Tip: Use './t $BUILD_TYPE -s' for detailed output${NC}"
            fi
        else
            echo -e "${RED}Some tests failed!${NC}"
            echo -e "${YELLOW}Run with './t $BUILD_TYPE -s' for more details${NC}"
        fi
        
        exit $TEST_RESULT
    else
        echo -e "${RED}Test executable not found at $TEST_EXECUTABLE${NC}"
        echo -e "${YELLOW}Make sure tests are built by checking CMakeLists.txt${NC}"
        exit 1
    fi
else
    echo -e "${RED}Build failed, cannot run tests${NC}"
    exit 1
fi