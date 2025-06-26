#pragma once

#include "Core/ThreadPool.hpp"
#include "Core/VisualMode.hpp"
#include "Graphics/ParticleSystem.hpp"
#include "Input/InputManager.hpp"
#include <memory>
#include <random>
#include <vector>

namespace Modes {

struct CelestialBody {
  glm::vec2 position;
  glm::vec2 velocity;
  float mass;
  float radius;
  sf::Color color;
  std::vector<glm::vec2> trail;
  static constexpr std::size_t MAX_TRAIL_LENGTH = 50;
};

class ParticleGalaxyMode : public Core::VisualMode {
public:
  explicit ParticleGalaxyMode(Core::DisplaySystem &displaySystem);
  ~ParticleGalaxyMode() override;

  void Initialize() override;
  void Update(float deltaTime) override;
  void Render(sf::RenderTarget &target) override;
  void HandleInput(const Core::InputEvent &event) override;

  [[nodiscard]] std::string GetName() const override {
    return "Particle Galaxy";
  }
  [[nodiscard]] std::string GetDescription() const override {
    return "N-body gravitational simulation with 50,000+ particles";
  }

  void OnActivate() override;
  void OnDeactivate() override;
  
  void EnableDemoMode() { demoMode_ = true; }

private:
  void CreateGalaxyPreset(int preset);
  void AddMassiveObject(const glm::vec2 &position);
  void UpdatePhysics(float deltaTime);
  void UpdateParticlePhysics(std::size_t start, std::size_t end,
                             float deltaTime);

  glm::vec2 CalculateGravitationalForce(const glm::vec2 &pos1,
                                        const glm::vec2 &pos2, float mass1,
                                        float mass2) const;

private:
  std::unique_ptr<Graphics::ParticleSystem> particleSystem_;
  std::unique_ptr<Core::ThreadPool> threadPool_;

  std::vector<CelestialBody> massiveObjects_;

  float timeDilation_ = 1.0f;
  float gravitationalConstant_ = 100.0f;
  bool paused_ = false;

  int currentPreset_ = 0;
  static constexpr int NUM_PRESETS = 5;

  std::mt19937 rng_;

  // Spatial partitioning for optimization
  struct QuadTreeNode;
  std::unique_ptr<QuadTreeNode> quadTree_;

  // Visual settings
  bool showTrails_ = true;
  bool showGrid_ = false;
  float particleSize_ = 1.0f;
  
  // Demo mode
  bool demoMode_ = false;
  float demoTimer_ = 0.0f;
  static constexpr float DEMO_DURATION = 8.0f;
};

} // namespace Modes