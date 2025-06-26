#pragma once

#include "Graphics/Particle.hpp"
#include "Graphics/Shader.hpp"
#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>
#include <vector>
#include <memory>

namespace Graphics {

struct GPUParticleData {
  glm::vec2 position;
  glm::vec2 velocity;
  glm::vec2 acceleration;
  float lifetime;
  float age;
  float size;
  float mass;
  glm::vec4 color;
};

class GPUParticleSystem {
public:
  explicit GPUParticleSystem(std::size_t maxParticles);
  ~GPUParticleSystem();

  void Update(float deltaTime);
  void Render(sf::RenderTarget& target);
  
  void EmitParticle(const GPUParticleData& particle);
  void EmitBurst(std::size_t count, const GPUParticleData& particleTemplate);
  void Clear();

  void SetGravity(const glm::vec2& gravity) { gravity_ = gravity; }
  void SetDamping(float damping) { damping_ = damping; }
  void SetBlendMode(sf::BlendMode mode) { blendMode_ = mode; }
  void SetAttractorPoints(const std::vector<glm::vec3>& attractors);

  std::size_t GetActiveParticleCount() const { return activeParticles_; }
  std::size_t GetMaxParticles() const { return maxParticles_; }

private:
  void InitializeShaders();
  void InitializeBuffers();
  void UpdateParticleBuffer();

  std::size_t maxParticles_;
  std::size_t activeParticles_;
  std::vector<GPUParticleData> particles_;
  
  // GPU buffers
  sf::VertexBuffer particleBuffer_;
  sf::VertexBuffer instanceBuffer_;
  
  // Shader for GPU particle physics
  std::unique_ptr<Shader> particleUpdateShader_;
  std::unique_ptr<Shader> particleRenderShader_;
  
  // Physics parameters
  glm::vec2 gravity_;
  float damping_;
  std::vector<glm::vec3> attractorPoints_; // x, y, mass
  
  // Rendering
  sf::BlendMode blendMode_;
  
  // For instanced rendering
  sf::VertexArray quadTemplate_;
};

} // namespace Graphics