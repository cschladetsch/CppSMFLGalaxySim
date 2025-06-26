#pragma once

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <optional>
#include <chrono>
#include <SFML/Graphics.hpp>
#include <spdlog/spdlog.h>

namespace Core {

class VisualMode;
class Renderer;
class InputManager;
class PerformanceProfiler;

struct DisplayConfig {
    unsigned int width = 1920;
    unsigned int height = 1080;
    std::string title = "CppSFMLVisualizer";
    bool fullscreen = false;
    bool vsync = true;
    unsigned int framerate_limit = 60;
    unsigned int antialiasing_level = 8;
};

enum class DisplayError {
    WindowCreationFailed,
    ModeNotFound,
    InitializationFailed
};

class DisplaySystem {
public:
    DisplaySystem();
    ~DisplaySystem();
    
    bool Initialize(const DisplayConfig& config);
    void Run();
    void Shutdown();
    
    void RegisterVisualMode(std::unique_ptr<VisualMode> mode);
    bool SwitchMode(const std::string& modeName);
    
    [[nodiscard]] sf::RenderWindow& GetWindow() noexcept { return window_; }
    [[nodiscard]] const sf::RenderWindow& GetWindow() const noexcept { return window_; }
    [[nodiscard]] Renderer& GetRenderer() noexcept { return *renderer_; }
    [[nodiscard]] InputManager& GetInputManager() noexcept { return *inputManager_; }
    
private:
    void ProcessEvents();
    void Update(float deltaTime);
    void Render();
    void UpdatePerformanceMetrics();
    
private:
    sf::RenderWindow window_;
    std::unique_ptr<Renderer> renderer_;
    std::unique_ptr<InputManager> inputManager_;
    std::unique_ptr<PerformanceProfiler> profiler_;
    
    std::vector<std::unique_ptr<VisualMode>> visualModes_;
    std::unordered_map<std::string, std::size_t> modeIndices_;
    std::size_t currentModeIndex_ = 0;
    
    bool isRunning_ = false;
    DisplayConfig config_;
    
    std::chrono::steady_clock::time_point lastFrameTime_;
    float deltaTime_ = 0.0f;
    
    static constexpr float MAX_DELTA_TIME = 1.0f / 30.0f; // Cap at 30 FPS minimum
};

} // namespace Core