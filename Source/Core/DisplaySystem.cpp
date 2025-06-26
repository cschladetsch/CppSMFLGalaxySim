#include "Core/DisplaySystem.hpp"
#include "Core/Renderer.hpp"
#include "Core/VisualMode.hpp"
#include "Input/InputManager.hpp"
#include "Utils/PerformanceProfiler.hpp"
#include <algorithm>

namespace Core {

DisplaySystem::DisplaySystem()
    : window_(), renderer_(nullptr),
      inputManager_(std::make_unique<InputManager>()),
      profiler_(std::make_unique<PerformanceProfiler>()), visualModes_(),
      modeIndices_(), currentModeIndex_(0), isRunning_(false), config_(),
      lastFrameTime_(std::chrono::steady_clock::now()), deltaTime_(0.0f) {}

DisplaySystem::~DisplaySystem() { Shutdown(); }

bool DisplaySystem::Initialize(const DisplayConfig &config) {
  config_ = config;

  // Configure window settings
  sf::ContextSettings settings;
  settings.antialiasingLevel = config.antialiasing_level;
  settings.majorVersion = 3;
  settings.minorVersion = 3;

  sf::VideoMode videoMode(config.width, config.height);
  sf::Uint32 style =
      config.fullscreen ? sf::Style::Fullscreen : sf::Style::Default;

  window_.create(videoMode, config.title, style, settings);

  if (!window_.isOpen()) {
    spdlog::error("Failed to create SFML window");
    return false;
  }

  window_.setVerticalSyncEnabled(config.vsync);
  if (config.framerate_limit > 0 && !config.vsync) {
    window_.setFramerateLimit(config.framerate_limit);
  }

  // Initialize renderer
  renderer_ = std::make_unique<Renderer>(window_);

  // Setup input event handlers
  inputManager_->RegisterEventHandler(
      InputEvent::Type::KeyPressed, [this](const InputEvent &event) {
        if (event.key.code == sf::Keyboard::Escape) {
          isRunning_ = false;
        } else if (event.key.code >= sf::Keyboard::Num1 &&
                   event.key.code <= sf::Keyboard::Num9) {
          std::size_t modeIndex = event.key.code - sf::Keyboard::Num1;
          if (modeIndex < visualModes_.size()) {
            currentModeIndex_ = modeIndex;
            if (visualModes_[currentModeIndex_]) {
              visualModes_[currentModeIndex_]->OnActivate();
            }
          }
        }
      });

  spdlog::info("Display system initialized successfully");
  spdlog::info("Window: {}x{}, Fullscreen: {}, VSync: {}", config.width,
               config.height, config.fullscreen, config.vsync);

  return true;
}

void DisplaySystem::Run() {
  if (!window_.isOpen()) {
    spdlog::error("Cannot run - window not initialized");
    return;
  }

  isRunning_ = true;
  lastFrameTime_ = std::chrono::steady_clock::now();

  // Activate first mode if available
  if (!visualModes_.empty() && visualModes_[currentModeIndex_]) {
    visualModes_[currentModeIndex_]->OnActivate();
  }

  while (isRunning_ && window_.isOpen()) {
    profiler_->BeginFrame();

    ProcessEvents();
    Update(deltaTime_);
    Render();

    profiler_->EndFrame();
    UpdatePerformanceMetrics();
  }
}

void DisplaySystem::Shutdown() {
  if (isRunning_) {
    isRunning_ = false;

    // Deactivate current mode
    if (currentModeIndex_ < visualModes_.size() &&
        visualModes_[currentModeIndex_]) {
      visualModes_[currentModeIndex_]->OnDeactivate();
    }

    // Clean up modes
    visualModes_.clear();
    modeIndices_.clear();

    // Close window
    if (window_.isOpen()) {
      window_.close();
    }

    spdlog::info("Display system shutdown");
  }
}

void DisplaySystem::RegisterVisualMode(std::unique_ptr<VisualMode> mode) {
  if (!mode) {
    spdlog::warn("Attempted to register null visual mode");
    return;
  }

  std::string modeName = mode->GetName();
  std::size_t index = visualModes_.size();

  modeIndices_[modeName] = index;
  visualModes_.push_back(std::move(mode));

  spdlog::info("Registered visual mode: {} (index: {})", modeName, index);

  // Initialize the mode
  visualModes_.back()->Initialize();
}

bool DisplaySystem::SwitchMode(const std::string &modeName) {
  auto it = modeIndices_.find(modeName);
  if (it == modeIndices_.end()) {
    spdlog::error("Mode not found: {}", modeName);
    return false;
  }

  // Deactivate current mode
  if (currentModeIndex_ < visualModes_.size() &&
      visualModes_[currentModeIndex_]) {
    visualModes_[currentModeIndex_]->OnDeactivate();
  }

  // Switch to new mode
  currentModeIndex_ = it->second;

  // Activate new mode
  if (visualModes_[currentModeIndex_]) {
    visualModes_[currentModeIndex_]->OnActivate();
    spdlog::info("Switched to mode: {}", modeName);
  }

  return true;
}

void DisplaySystem::ProcessEvents() {
  sf::Event event;
  while (window_.pollEvent(event)) {
    if (event.type == sf::Event::Closed) {
      isRunning_ = false;
    }

    inputManager_->ProcessEvent(event);

    // Pass input to current visual mode
    if (currentModeIndex_ < visualModes_.size() &&
        visualModes_[currentModeIndex_]) {
      InputEvent inputEvent;

      switch (event.type) {
      case sf::Event::KeyPressed:
        inputEvent.type = InputEvent::Type::KeyPressed;
        inputEvent.key.code = event.key.code;
        inputEvent.key.alt = event.key.alt;
        inputEvent.key.control = event.key.control;
        inputEvent.key.shift = event.key.shift;
        inputEvent.key.system = event.key.system;
        visualModes_[currentModeIndex_]->HandleInput(inputEvent);
        break;

      case sf::Event::MouseButtonPressed:
        inputEvent.type = InputEvent::Type::MouseButtonPressed;
        inputEvent.mouseButton.button = event.mouseButton.button;
        inputEvent.mouseButton.position =
            glm::vec2(event.mouseButton.x, event.mouseButton.y);
        visualModes_[currentModeIndex_]->HandleInput(inputEvent);
        break;

      case sf::Event::MouseMoved:
        inputEvent.type = InputEvent::Type::MouseMoved;
        inputEvent.mouseMove.position =
            glm::vec2(event.mouseMove.x, event.mouseMove.y);
        visualModes_[currentModeIndex_]->HandleInput(inputEvent);
        break;

      case sf::Event::MouseWheelScrolled:
        inputEvent.type = InputEvent::Type::MouseWheelScrolled;
        inputEvent.mouseWheel.delta = event.mouseWheelScroll.delta;
        inputEvent.mouseWheel.position =
            glm::vec2(event.mouseWheelScroll.x, event.mouseWheelScroll.y);
        visualModes_[currentModeIndex_]->HandleInput(inputEvent);
        break;

      case sf::Event::Resized:
        inputEvent.type = InputEvent::Type::WindowResized;
        inputEvent.size.width = event.size.width;
        inputEvent.size.height = event.size.height;
        visualModes_[currentModeIndex_]->OnResize(event.size.width,
                                                  event.size.height);
        break;

      default:
        break;
      }
    }
  }
}

void DisplaySystem::Update(float deltaTime) {
  profiler_->BeginSection("Update");

  inputManager_->Update();

  // Update current visual mode
  if (currentModeIndex_ < visualModes_.size() &&
      visualModes_[currentModeIndex_]) {
    visualModes_[currentModeIndex_]->Update(deltaTime);
  }

  profiler_->EndSection("Update");
}

void DisplaySystem::Render() {
  profiler_->BeginSection("Render");

  renderer_->BeginFrame();

  // Render current visual mode
  if (currentModeIndex_ < visualModes_.size() &&
      visualModes_[currentModeIndex_]) {
    visualModes_[currentModeIndex_]->Render(window_);
  }

  renderer_->EndFrame();

  profiler_->EndSection("Render");
}

void DisplaySystem::UpdatePerformanceMetrics() {
  auto currentTime = std::chrono::steady_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::duration<float>>(
                     currentTime - lastFrameTime_)
                     .count();

  deltaTime_ = std::min(elapsed, MAX_DELTA_TIME);
  lastFrameTime_ = currentTime;

  profiler_->LogFrameTime(deltaTime_);
}

} // namespace Core