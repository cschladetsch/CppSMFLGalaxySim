#pragma once

#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>

namespace Core {

class Camera2D {
public:
  Camera2D();
  explicit Camera2D(const sf::FloatRect &viewport);
  ~Camera2D();

  void SetPosition(const glm::vec2 &position);
  void Move(const glm::vec2 &offset);

  void SetRotation(float degrees);
  void Rotate(float degrees);

  void SetZoom(float zoom);
  void Zoom(float factor);

  void SetViewport(const sf::FloatRect &viewport);
  void SetSize(const glm::vec2 &size);

  [[nodiscard]] glm::vec2 GetPosition() const { return position_; }
  [[nodiscard]] float GetRotation() const { return rotation_; }
  [[nodiscard]] float GetZoom() const { return zoom_; }
  [[nodiscard]] glm::vec2 GetSize() const { return size_; }

  [[nodiscard]] const sf::View &GetView() const { return view_; }

  [[nodiscard]] glm::vec2 ScreenToWorld(const glm::vec2 &screenPos) const;
  [[nodiscard]] glm::vec2 WorldToScreen(const glm::vec2 &worldPos) const;

  void Update();

private:
  sf::View view_;
  glm::vec2 position_{0.0f, 0.0f};
  glm::vec2 size_{1920.0f, 1080.0f};
  float rotation_ = 0.0f;
  float zoom_ = 1.0f;

  bool needsUpdate_ = true;
};

} // namespace Core