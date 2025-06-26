enable_testing()

set(TEST_SOURCES
    Core/ThreadPoolTest.cpp
)

# Add source files needed for tests
set(TEST_LIB_SOURCES
    ${CMAKE_SOURCE_DIR}/Source/Core/ThreadPool.cpp
    ${CMAKE_SOURCE_DIR}/Source/Utils/PerformanceProfiler.cpp
)

add_executable(CppSFMLVisualizerTests ${TEST_SOURCES} ${TEST_LIB_SOURCES})

target_link_libraries(CppSFMLVisualizerTests 
    PRIVATE
        Catch2::Catch2WithMain
        sfml-graphics
        sfml-window
        sfml-system
        sfml-audio
        glm::glm
        nlohmann_json::nlohmann_json
        spdlog::spdlog
        Threads::Threads
)

target_include_directories(CppSFMLVisualizerTests 
    PRIVATE 
        ${CMAKE_SOURCE_DIR}/Include
)

include(Catch)
catch_discover_tests(CppSFMLVisualizerTests)