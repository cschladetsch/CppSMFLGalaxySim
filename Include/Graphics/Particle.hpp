#pragma once

#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>

namespace Graphics {

struct Particle {
  glm::vec2 position{0.0f, 0.0f};
  glm::vec2 velocity{0.0f, 0.0f};
  glm::vec2 acceleration{0.0f, 0.0f};
  float mass = 1.0f;
  float size = 1.0f;
  float lifetime = 1.0f;
  float age = 0.0f;
  sf::Color color = sf::Color::White;
  bool active = false;
};

} // namespace Graphics