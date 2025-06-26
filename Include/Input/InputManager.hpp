#pragma once

#include <SFML/Window.hpp>
#include <functional>
#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>

namespace Core {

struct KeyEvent {
  sf::Keyboard::Key code;
  bool alt;
  bool control;
  bool shift;
  bool system;
};

struct MouseButtonEvent {
  sf::Mouse::Button button;
  glm::vec2 position;
};

struct MouseMoveEvent {
  glm::vec2 position;
  glm::vec2 delta;
};

struct MouseWheelEvent {
  float delta;
  glm::vec2 position;
};

struct SizeEvent {
  unsigned int width;
  unsigned int height;
};

struct InputEvent {
  enum class Type {
    KeyPressed,
    KeyReleased,
    MouseButtonPressed,
    MouseButtonReleased,
    MouseMoved,
    MouseWheelScrolled,
    WindowResized
  };

  Type type;

  union {
    KeyEvent key;
    MouseButtonEvent mouseButton;
    MouseMoveEvent mouseMove;
    MouseWheelEvent mouseWheel;
    SizeEvent size;
  };
};

class InputManager {
public:
  using EventHandler = std::function<void(const InputEvent &)>;
  using KeyBinding =
      std::pair<sf::Keyboard::Key, unsigned int>; // Key + modifiers

  InputManager();
  ~InputManager();

  void ProcessEvent(const sf::Event &event);
  void Update();

  [[nodiscard]] bool IsKeyPressed(sf::Keyboard::Key key) const;
  [[nodiscard]] bool IsMouseButtonPressed(sf::Mouse::Button button) const;
  [[nodiscard]] glm::vec2 GetMousePosition() const { return mousePosition_; }
  [[nodiscard]] glm::vec2 GetMouseDelta() const { return mouseDelta_; }

  void RegisterEventHandler(InputEvent::Type type, EventHandler handler);
  void RegisterKeyBinding(const std::string &action, sf::Keyboard::Key key,
                          unsigned int modifiers = 0);
  [[nodiscard]] bool IsActionPressed(const std::string &action) const;

  void SetMouseSensitivity(float sensitivity) {
    mouseSensitivity_ = sensitivity;
  }
  [[nodiscard]] float GetMouseSensitivity() const { return mouseSensitivity_; }

private:
  std::unordered_map<InputEvent::Type, std::vector<EventHandler>>
      eventHandlers_;
  std::unordered_map<std::string, KeyBinding> keyBindings_;

  bool keyStates_[sf::Keyboard::KeyCount] = {};
  bool mouseButtonStates_[sf::Mouse::ButtonCount] = {};

  glm::vec2 mousePosition_{0.0f, 0.0f};
  glm::vec2 lastMousePosition_{0.0f, 0.0f};
  glm::vec2 mouseDelta_{0.0f, 0.0f};

  float mouseSensitivity_ = 1.0f;

  unsigned int currentModifiers_ = 0;

  static constexpr unsigned int MOD_ALT = 1 << 0;
  static constexpr unsigned int MOD_CONTROL = 1 << 1;
  static constexpr unsigned int MOD_SHIFT = 1 << 2;
  static constexpr unsigned int MOD_SYSTEM = 1 << 3;
};

} // namespace Core