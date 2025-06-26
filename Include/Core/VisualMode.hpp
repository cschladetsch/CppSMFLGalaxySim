#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <string>

namespace Core {

class DisplaySystem;
struct InputEvent;

class VisualMode {
public:
  explicit VisualMode(DisplaySystem &displaySystem)
      : displaySystem_(displaySystem) {}

  virtual ~VisualMode() = default;

  virtual void Initialize() = 0;
  virtual void Update(float deltaTime) = 0;
  virtual void Render(sf::RenderTarget &target) = 0;
  virtual void HandleInput(const InputEvent &event) = 0;

  [[nodiscard]] virtual std::string GetName() const = 0;
  [[nodiscard]] virtual std::string GetDescription() const { return ""; }

  virtual void OnActivate() {}
  virtual void OnDeactivate() {}

  virtual void OnResize(unsigned int width, unsigned int height) {}

protected:
  [[nodiscard]] DisplaySystem &GetDisplaySystem() noexcept {
    return displaySystem_;
  }
  [[nodiscard]] const DisplaySystem &GetDisplaySystem() const noexcept {
    return displaySystem_;
  }

private:
  DisplaySystem &displaySystem_;
};

template <typename T>
concept VisualModeType = std::is_base_of_v<VisualMode, T>;

} // namespace Core