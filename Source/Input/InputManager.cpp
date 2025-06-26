#include "Input/InputManager.hpp"

namespace Core {

InputManager::InputManager() {
  std::fill(std::begin(keyStates_), std::end(keyStates_), false);
  std::fill(std::begin(mouseButtonStates_), std::end(mouseButtonStates_),
            false);
}

InputManager::~InputManager() = default;

void InputManager::ProcessEvent(const sf::Event &event) {
  InputEvent inputEvent;

  switch (event.type) {
  case sf::Event::KeyPressed:
    keyStates_[event.key.code] = true;
    currentModifiers_ = 0;
    if (event.key.alt)
      currentModifiers_ |= MOD_ALT;
    if (event.key.control)
      currentModifiers_ |= MOD_CONTROL;
    if (event.key.shift)
      currentModifiers_ |= MOD_SHIFT;
    if (event.key.system)
      currentModifiers_ |= MOD_SYSTEM;

    inputEvent.type = InputEvent::Type::KeyPressed;
    inputEvent.key.code = event.key.code;
    inputEvent.key.alt = event.key.alt;
    inputEvent.key.control = event.key.control;
    inputEvent.key.shift = event.key.shift;
    inputEvent.key.system = event.key.system;

    for (const auto &handler : eventHandlers_[InputEvent::Type::KeyPressed]) {
      handler(inputEvent);
    }
    break;

  case sf::Event::KeyReleased:
    keyStates_[event.key.code] = false;

    inputEvent.type = InputEvent::Type::KeyReleased;
    inputEvent.key.code = event.key.code;
    inputEvent.key.alt = event.key.alt;
    inputEvent.key.control = event.key.control;
    inputEvent.key.shift = event.key.shift;
    inputEvent.key.system = event.key.system;

    for (const auto &handler : eventHandlers_[InputEvent::Type::KeyReleased]) {
      handler(inputEvent);
    }
    break;

  case sf::Event::MouseButtonPressed:
    mouseButtonStates_[event.mouseButton.button] = true;

    inputEvent.type = InputEvent::Type::MouseButtonPressed;
    inputEvent.mouseButton.button = event.mouseButton.button;
    inputEvent.mouseButton.position =
        glm::vec2(event.mouseButton.x, event.mouseButton.y);

    for (const auto &handler :
         eventHandlers_[InputEvent::Type::MouseButtonPressed]) {
      handler(inputEvent);
    }
    break;

  case sf::Event::MouseButtonReleased:
    mouseButtonStates_[event.mouseButton.button] = false;

    inputEvent.type = InputEvent::Type::MouseButtonReleased;
    inputEvent.mouseButton.button = event.mouseButton.button;
    inputEvent.mouseButton.position =
        glm::vec2(event.mouseButton.x, event.mouseButton.y);

    for (const auto &handler :
         eventHandlers_[InputEvent::Type::MouseButtonReleased]) {
      handler(inputEvent);
    }
    break;

  case sf::Event::MouseMoved:
    mousePosition_ = glm::vec2(event.mouseMove.x, event.mouseMove.y);

    inputEvent.type = InputEvent::Type::MouseMoved;
    inputEvent.mouseMove.position = mousePosition_;
    inputEvent.mouseMove.delta = mouseDelta_;

    for (const auto &handler : eventHandlers_[InputEvent::Type::MouseMoved]) {
      handler(inputEvent);
    }
    break;

  case sf::Event::MouseWheelScrolled:
    inputEvent.type = InputEvent::Type::MouseWheelScrolled;
    inputEvent.mouseWheel.delta = event.mouseWheelScroll.delta;
    inputEvent.mouseWheel.position =
        glm::vec2(event.mouseWheelScroll.x, event.mouseWheelScroll.y);

    for (const auto &handler :
         eventHandlers_[InputEvent::Type::MouseWheelScrolled]) {
      handler(inputEvent);
    }
    break;

  case sf::Event::Resized:
    inputEvent.type = InputEvent::Type::WindowResized;
    inputEvent.size.width = event.size.width;
    inputEvent.size.height = event.size.height;

    for (const auto &handler :
         eventHandlers_[InputEvent::Type::WindowResized]) {
      handler(inputEvent);
    }
    break;

  default:
    break;
  }
}

void InputManager::Update() {
  mouseDelta_ = (mousePosition_ - lastMousePosition_) * mouseSensitivity_;
  lastMousePosition_ = mousePosition_;
}

bool InputManager::IsKeyPressed(sf::Keyboard::Key key) const {
  return keyStates_[key];
}

bool InputManager::IsMouseButtonPressed(sf::Mouse::Button button) const {
  return mouseButtonStates_[button];
}

void InputManager::RegisterEventHandler(InputEvent::Type type,
                                        EventHandler handler) {
  eventHandlers_[type].push_back(handler);
}

void InputManager::RegisterKeyBinding(const std::string &action,
                                      sf::Keyboard::Key key,
                                      unsigned int modifiers) {
  keyBindings_[action] = KeyBinding(key, modifiers);
}

bool InputManager::IsActionPressed(const std::string &action) const {
  auto it = keyBindings_.find(action);
  if (it == keyBindings_.end()) {
    return false;
  }

  const auto &[key, modifiers] = it->second;
  return IsKeyPressed(key) && (currentModifiers_ == modifiers);
}

} // namespace Core