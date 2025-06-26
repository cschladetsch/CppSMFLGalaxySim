#include "Core/DisplaySystem.hpp"
#include "Modes/ParticleGalaxyMode.hpp"
#include <spdlog/spdlog.h>
#include <iostream>
#include <exception>

int main(int argc, char* argv[]) {
    try {
        spdlog::set_level(spdlog::level::debug);
        spdlog::info("CppSFMLVisualizer starting...");
        
        Core::DisplayConfig config{
            .width = 1920,
            .height = 1080,
            .title = "CppSFMLVisualizer - High Performance Visual Display System",
            .fullscreen = false,
            .vsync = true,
            .framerate_limit = 0, // Unlimited when vsync is on
            .antialiasing_level = 8
        };
        
        // Parse command line arguments
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "--fullscreen" || arg == "-f") {
                config.fullscreen = true;
            } else if (arg == "--width" && i + 1 < argc) {
                config.width = std::stoul(argv[++i]);
            } else if (arg == "--height" && i + 1 < argc) {
                config.height = std::stoul(argv[++i]);
            } else if (arg == "--no-vsync") {
                config.vsync = false;
                config.framerate_limit = 60;
            }
        }
        
        Core::DisplaySystem displaySystem;
        
        if (!displaySystem.Initialize(config)) {
            spdlog::error("Failed to initialize display system");
            return 1;
        }
        
        // Register visual modes
        displaySystem.RegisterVisualMode(std::make_unique<Modes::ParticleGalaxyMode>(displaySystem));
        
        displaySystem.Run();
        displaySystem.Shutdown();
        
        spdlog::info("CppSFMLVisualizer shutdown complete");
        return 0;
        
    } catch (const std::exception& e) {
        spdlog::critical("Unhandled exception: {}", e.what());
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        spdlog::critical("Unknown exception occurred");
        std::cerr << "Fatal error: Unknown exception" << std::endl;
        return 1;
    }
}