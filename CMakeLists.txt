cmake_minimum_required(VERSION 3.25)
project(CppSFMLVisualizer 
    VERSION 1.0.0
    DESCRIPTION "High-performance 2D visual display system using C++23 and SFML"
    LANGUAGES CXX)

# C++23 Standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build type configuration
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall -Wextra -Wpedantic
        -Wno-unused-parameter
        -Wno-missing-field-initializers
    )
    
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g3 -O0 -DDEBUG)
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O3 -DNDEBUG -march=native)
    elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        add_compile_options(-O2 -g -DNDEBUG)
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(/W4)
    
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(/Od /Zi /DDEBUG)
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(/O2 /DNDEBUG)
    endif()
endif()

# Find packages
find_package(SFML 2.6 REQUIRED COMPONENTS graphics window system audio network)
find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)

# FetchContent for external dependencies
include(FetchContent)

# GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 0.9.9.8
)

# nlohmann/json
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)

# spdlog
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.13.0
)

# Catch2 for testing
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.1
)

FetchContent_MakeAvailable(glm json spdlog Catch2)

# Create main executable
set(SOURCES
    Source/main.cpp
    Source/Core/DisplaySystem.cpp
    Source/Core/VisualMode.cpp
    Source/Core/Renderer.cpp
    Source/Core/Camera2D.cpp
    Source/Core/ThreadPool.cpp
    Source/Graphics/ParticleSystem.cpp
    Source/Graphics/PostProcessing.cpp
    Source/Graphics/Shader.cpp
    Source/Graphics/GPUParticleSystem.cpp
    Source/Physics/PhysicsEngine.cpp
    Source/Audio/AudioAnalyzer.cpp
    Source/Input/InputManager.cpp
    Source/Utils/Math.cpp
    Source/Utils/PerformanceProfiler.cpp
    Source/Modes/ParticleGalaxyMode.cpp
)

set(HEADERS
    Include/Core/DisplaySystem.hpp
    Include/Core/VisualMode.hpp
    Include/Core/Renderer.hpp
    Include/Core/Camera2D.hpp
    Include/Core/ThreadPool.hpp
    Include/Graphics/ParticleSystem.hpp
    Include/Graphics/PostProcessing.hpp
    Include/Graphics/Shader.hpp
    Include/Graphics/GPUParticleSystem.hpp
    Include/Physics/PhysicsEngine.hpp
    Include/Audio/AudioAnalyzer.hpp
    Include/Input/InputManager.hpp
    Include/Utils/Math.hpp
    Include/Utils/PerformanceProfiler.hpp
    Include/Modes/ParticleGalaxyMode.hpp
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(${PROJECT_NAME} 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/Include
        ${CMAKE_CURRENT_SOURCE_DIR}/External
)

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    PRIVATE
        sfml-graphics
        sfml-window
        sfml-system
        sfml-audio
        sfml-network
        OpenGL::GL
        glm::glm
        nlohmann_json::nlohmann_json
        spdlog::spdlog
        Threads::Threads
)

# Copy assets to build directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/Assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/Assets
)

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin
)

# Testing
enable_testing()
add_subdirectory(Test)

# Installation
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

install(DIRECTORY Assets
    DESTINATION share/${PROJECT_NAME}
)