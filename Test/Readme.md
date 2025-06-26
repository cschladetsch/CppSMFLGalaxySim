# Test Directory

This directory contains all unit tests and integration tests for the CppSFMLVisualizer project.

## Structure

- `main.cpp` - Catch2 test runner entry point
- `Core/` - Tests for core systems
  - `ThreadPoolTest.cpp` - Thread pool functionality tests

## Running Tests

### Build and run all tests:
```bash
cd build/Release
ctest
```

### Run tests with detailed output:
```bash
./build/Release/Test/CppSFMLVisualizerTests -s
```

### Run specific test cases:
```bash
./build/Release/Test/CppSFMLVisualizerTests -s -c "ThreadPool*"
```

## Writing New Tests

Example test structure:
```cpp
#include <catch2/catch_all.hpp>
#include "YourHeader.hpp"

TEST_CASE("Feature description", "[Category]") {
    SECTION("Specific behavior") {
        // Arrange
        // Act
        // Assert
        REQUIRE(result == expected);
    }
}
```

## Test Guidelines

- Write tests for all public interfaces
- Use descriptive test names
- Follow AAA pattern (Arrange, Act, Assert)
- Keep tests isolated and independent
- Mock external dependencies when needed
- Aim for 80%+ code coverage

## Categories

Use these tags for test organization:
- `[Core]` - Core system tests
- `[Graphics]` - Rendering tests
- `[Physics]` - Physics simulation tests
- `[Performance]` - Performance benchmarks
- `[Integration]` - Integration tests