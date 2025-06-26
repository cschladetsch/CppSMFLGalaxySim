#include "Modes/ParticleGalaxyMode.hpp"
#include "Core/DisplaySystem.hpp"
#include "Core/Renderer.hpp"
#include "Utils/Math.hpp"
#include <algorithm>
#include <execution>
#include <numbers>
#include <random>
#include <spdlog/spdlog.h>

namespace Modes {

struct ParticleGalaxyMode::QuadTreeNode {
  glm::vec2 center;
  float halfSize;
  float totalMass = 0.0f;
  glm::vec2 centerOfMass{0.0f, 0.0f};
  std::vector<std::size_t> particleIndices;
  std::unique_ptr<QuadTreeNode> children[4];

  static constexpr std::size_t MAX_PARTICLES_PER_NODE = 10;
  static constexpr float MIN_NODE_SIZE = 10.0f;
};

ParticleGalaxyMode::ParticleGalaxyMode(Core::DisplaySystem &displaySystem)
    : VisualMode(displaySystem),
      particleSystem_(std::make_unique<Graphics::ParticleSystem>(30000)),
      threadPool_(std::make_unique<Core::ThreadPool>()), massiveObjects_(),
      rng_(std::random_device{}()) {}

ParticleGalaxyMode::~ParticleGalaxyMode() = default;

void ParticleGalaxyMode::Initialize() {
  spdlog::info("Initializing Particle Galaxy Mode");
  
  // We'll handle physics manually for N-body simulation
  particleSystem_->SetGravity(glm::vec2(0.0f, 0.0f));
  particleSystem_->SetDamping(1.0f);

  // Set particle system blend mode for glowing effect
  particleSystem_->SetBlendMode(sf::BlendAdd);

  // Create initial galaxy
  CreateGalaxyPreset(0);
}

void ParticleGalaxyMode::CreateGalaxyPreset(int preset) {
  currentPreset_ = preset;

  // Clear existing particles
  massiveObjects_.clear();
  particleSystem_->Clear();

  auto windowSize = GetDisplaySystem().GetWindow().getSize();
  glm::vec2 center(windowSize.x * 0.5f, windowSize.y * 0.5f);

  std::uniform_real_distribution<float> angleDist(0.0f, Utils::TWO_PI);
  std::uniform_real_distribution<float> radiusDist(0.0f, 1.0f);
  std::uniform_real_distribution<float> speedDist(0.5f, 1.5f);
  std::normal_distribution<float> heightDist(0.0f, 20.0f);

  switch (preset) {
  case 0: { // Milky Way-like Spiral Galaxy
    // Central supermassive black hole (Sagittarius A*)
    CelestialBody blackHole;
    blackHole.position = center;
    blackHole.velocity = glm::vec2(0.0f, 0.0f);
    blackHole.mass = 30000.0f;
    blackHole.radius = 5.0f;
    blackHole.color = sf::Color(255, 255, 200);
    massiveObjects_.push_back(blackHole);

    // Galaxy parameters for Milky Way
    const int numArms = 4; // Milky Way has 4 main spiral arms
    const float armAngleOffset = Utils::TWO_PI / numArms;
    const float maxRadius = 600.0f;
    const float coreRadius = 80.0f;
    const float armWidth = 40.0f;
    const float diskThickness = 15.0f;

    // Star type distribution
    std::uniform_real_distribution<float> starTypeDist(0.0f, 1.0f);
    std::normal_distribution<float> diskHeightDist(0.0f, diskThickness);
    std::exponential_distribution<float> coreDensityDist(3.0f);

    // Create galactic bulge/core (dense center with yellowish glow)
    for (int i = 0; i < 8000; ++i) {
      float r = coreRadius * (1.0f - coreDensityDist(rng_) / 3.0f);
      r = std::max(3.0f, r);
      float angle = angleDist(rng_);

      // Bulge is more spherical than disk
      float bulgeHeight =
          std::normal_distribution<float>(0.0f, coreRadius * 0.3f)(rng_);

      Graphics::Particle particle;
      particle.position = center + glm::vec2(r * std::cos(angle),
                                             r * std::sin(angle) + bulgeHeight);

      // Orbital velocity
      float orbitalSpeed =
          std::sqrt(gravitationalConstant_ * blackHole.mass / r) *
          speedDist(rng_);
      glm::vec2 toCenter = glm::normalize(center - particle.position);
      particle.velocity = glm::vec2(-toCenter.y, toCenter.x) * orbitalSpeed;

      // Core stars are mostly older (yellow/red/orange)
      float starType = starTypeDist(rng_);
      if (starType < 0.6f) {
        // Red dwarf (most common)
        particle.color = sf::Color(255, 160, 100, 255);
        particle.size = 0.3f + radiusDist(rng_) * 0.3f;
      } else if (starType < 0.85f) {
        // K-type orange star
        particle.color = sf::Color(255, 200, 150, 255);
        particle.size = 0.5f + radiusDist(rng_) * 0.5f;
      } else if (starType < 0.95f) {
        // G-type yellow star like our Sun
        particle.color = sf::Color(255, 240, 200, 255);
        particle.size = 0.8f + radiusDist(rng_) * 0.4f;
      } else {
        // Red giant
        particle.color = sf::Color(255, 120, 80, 255);
        particle.size = 1.5f + radiusDist(rng_) * 0.8f;
      }

      // Brightness increases towards center
      float brightnessFactor = 1.0f + (1.0f - r / coreRadius) * 0.5f;
      particle.color.r =
          std::min(255, static_cast<int>(particle.color.r * brightnessFactor));
      particle.color.g =
          std::min(255, static_cast<int>(particle.color.g * brightnessFactor));
      particle.color.b =
          std::min(255, static_cast<int>(particle.color.b * brightnessFactor));

      particle.mass = 1.0f;
      particle.lifetime = 1000000.0f;
      particle.active = true;

      particleSystem_->EmitParticle(particle);
    }

    // Create spiral arms with varied star populations
    for (int i = 0; i < 20000; ++i) {
      float radius = coreRadius + (maxRadius - coreRadius) *
                                      std::pow(radiusDist(rng_), 0.6f);

      // Determine which arm
      int armIndex = i % numArms;
      float armBaseAngle = armIndex * armAngleOffset;

      // Logarithmic spiral with bar structure near center
      float windingFactor = 0.2f; // Tighter winding for Milky Way
      float barRadius = coreRadius * 1.5f;
      float spiralAngle;

      if (radius < barRadius) {
        // Bar structure
        spiralAngle = armBaseAngle;
      } else {
        // Spiral arms
        spiralAngle =
            armBaseAngle + std::log(radius / barRadius) * windingFactor;
      }

      // Star type varies by location
      float distanceRatio = (radius - coreRadius) / (maxRadius - coreRadius);

      // Add spread to create arm thickness and inter-arm stars
      float inArm = radiusDist(rng_);
      float armSpread;

      // Arm width decreases with distance for tapering effect
      float currentArmWidth = armWidth * (1.0f - distanceRatio * 0.7f);

      if (inArm < 0.6f) {
        // Star in spiral arm (higher density)
        float spreadFactor = currentArmWidth / std::max(radius, 50.0f);
        armSpread = std::normal_distribution<float>(0.0f, spreadFactor)(rng_);
      } else {
        // Inter-arm star (lower density)
        armSpread = std::uniform_real_distribution<float>(
            -armAngleOffset / 2, armAngleOffset / 2)(rng_);
      }

      spiralAngle += armSpread;

      // Disk height decreases with radius
      float heightScale =
          1.0f - (radius - coreRadius) / (maxRadius - coreRadius) * 0.7f;
      float height = diskHeightDist(rng_) * heightScale;

      Graphics::Particle particle;
      particle.position =
          center + glm::vec2(radius * std::cos(spiralAngle),
                             radius * std::sin(spiralAngle) + height);

      // Orbital velocity with some variation
      float orbitalSpeed =
          std::sqrt(gravitationalConstant_ * blackHole.mass / radius) *
          speedDist(rng_);
      glm::vec2 toCenter = glm::normalize(center - particle.position);
      particle.velocity = glm::vec2(-toCenter.y, toCenter.x) * orbitalSpeed;

      // Star type varies by location
      float starType = starTypeDist(rng_);

      if (inArm < 0.6f) {
        // Spiral arms have younger, bluer stars and star forming regions
        if (starType < 0.1f) {
          // O-type blue supergiant (very rare)
          particle.color = sf::Color(155, 176, 255, 255);
          particle.size = 2.0f + radiusDist(rng_) * 1.0f;
        } else if (starType < 0.3f) {
          // B-type blue giant
          particle.color = sf::Color(170, 191, 255, 255);
          particle.size = 1.2f + radiusDist(rng_) * 0.6f;
        } else if (starType < 0.5f) {
          // A-type blue-white star
          particle.color = sf::Color(202, 215, 255, 255);
          particle.size = 0.8f + radiusDist(rng_) * 0.4f;
        } else if (starType < 0.7f) {
          // F-type white star
          particle.color = sf::Color(248, 247, 255, 255);
          particle.size = 0.7f + radiusDist(rng_) * 0.3f;
        } else if (starType < 0.85f) {
          // G-type yellow star
          particle.color = sf::Color(255, 244, 234, 255);
          particle.size = 0.6f + radiusDist(rng_) * 0.3f;
        } else {
          // K-type orange star
          particle.color = sf::Color(255, 210, 161, 255);
          particle.size = 0.5f + radiusDist(rng_) * 0.25f;
        }

        // Add some nebulosity in star forming regions
        if (starType < 0.2f && radiusDist(rng_) < 0.3f) {
          particle.color.a = 180; // Slightly transparent for nebula effect
        }
      } else {
        // Inter-arm regions have older, redder stars
        if (starType < 0.7f) {
          // M-type red dwarf
          particle.color = sf::Color(255, 204, 111, 220);
          particle.size = 0.2f + radiusDist(rng_) * 0.2f;
        } else if (starType < 0.9f) {
          // K-type orange dwarf
          particle.color = sf::Color(255, 210, 161, 220);
          particle.size = 0.4f + radiusDist(rng_) * 0.3f;
        } else if (starType < 0.98f) {
          // G-type yellow star
          particle.color = sf::Color(255, 244, 234, 220);
          particle.size = 0.6f + radiusDist(rng_) * 0.3f;
        } else {
          // Red giant
          particle.color = sf::Color(255, 167, 82, 200);
          particle.size = 1.0f + radiusDist(rng_) * 0.8f;
        }
      }

      // Fade out stars towards edge with smooth tapering
      float edgeFade = 1.0f;
      if (distanceRatio > 0.6f) {
        // Exponential falloff for outer regions
        edgeFade = std::exp(-5.0f * (distanceRatio - 0.6f));
      }

      // Also reduce density in outer regions
      if (distanceRatio > 0.7f &&
          radiusDist(rng_) > (1.0f - distanceRatio) * 2.0f) {
        continue; // Skip this particle to reduce density
      }

      particle.color.a = static_cast<sf::Uint8>(particle.color.a * edgeFade);

      particle.mass = 1.0f;
      particle.lifetime = 1000000.0f;
      particle.active = true;

      particleSystem_->EmitParticle(particle);
    }

    // Add some globular clusters around the galaxy
    std::uniform_int_distribution<int> clusterCountDist(3, 6);
    int numClusters = clusterCountDist(rng_);

    for (int c = 0; c < numClusters; ++c) {
      float clusterRadius =
          maxRadius * std::uniform_real_distribution<float>(0.3f, 1.2f)(rng_);
      float clusterAngle = angleDist(rng_);
      float clusterHeight = std::normal_distribution<float>(0.0f, 100.0f)(rng_);

      glm::vec2 clusterCenter =
          center +
          glm::vec2(clusterRadius * std::cos(clusterAngle),
                    clusterRadius * std::sin(clusterAngle) + clusterHeight);

      // Each cluster has 100-300 stars
      int starsInCluster = std::uniform_int_distribution<int>(100, 300)(rng_);
      float clusterSize = 20.0f;

      for (int s = 0; s < starsInCluster; ++s) {
        glm::vec2 offset(
            std::normal_distribution<float>(0.0f, clusterSize)(rng_),
            std::normal_distribution<float>(0.0f, clusterSize)(rng_));

        Graphics::Particle particle;
        particle.position = clusterCenter + offset;

        // Cluster orbital velocity
        float orbitalSpeed =
            std::sqrt(gravitationalConstant_ * blackHole.mass / clusterRadius) *
            0.8f;
        glm::vec2 toCenter = glm::normalize(center - clusterCenter);
        particle.velocity = glm::vec2(-toCenter.y, toCenter.x) * orbitalSpeed;

        // Globular clusters have old stars
        particle.color = sf::Color(255, 220, 180, 255);
        particle.size = 0.3f + radiusDist(rng_) * 0.4f;
        particle.mass = 1.0f;
        particle.lifetime = 1000000.0f;
        particle.active = true;

        particleSystem_->EmitParticle(particle);
      }
    }

    spdlog::info("Created Milky Way galaxy with {} stars",
                 particleSystem_->GetActiveParticleCount());
    break;
  }

  case 1: { // Binary Star System
    // Two massive stars orbiting each other
    float separation = 200.0f;
    float totalMass = 5000.0f;

    CelestialBody star1;
    star1.position = center + glm::vec2(-separation * 0.5f, 0.0f);
    star1.velocity = glm::vec2(0.0f, -30.0f);
    star1.mass = totalMass * 0.6f;
    star1.radius = 15.0f;
    star1.color = sf::Color(255, 200, 100);
    massiveObjects_.push_back(star1);

    CelestialBody star2;
    star2.position = center + glm::vec2(separation * 0.5f, 0.0f);
    star2.velocity = glm::vec2(0.0f, 30.0f);
    star2.mass = totalMass * 0.4f;
    star2.radius = 12.0f;
    star2.color = sf::Color(100, 150, 255);
    massiveObjects_.push_back(star2);

    // Create accretion disks
    for (int i = 0; i < 30000; ++i) {
      float angle = angleDist(rng_);
      float radius = 50.0f + 300.0f * std::pow(radiusDist(rng_), 2.0f);

      // Randomly assign to one of the stars
      int starIndex = (i % 3 == 0) ? 0 : 1;
      glm::vec2 starPos = massiveObjects_[starIndex].position;

      Graphics::Particle particle;
      particle.position = starPos + glm::vec2(radius * std::cos(angle),
                                              radius * std::sin(angle) *
                                                  0.3f // Flatten the disk
                                    );

      // Orbital velocity
      float orbitalSpeed = std::sqrt(gravitationalConstant_ *
                                     massiveObjects_[starIndex].mass / radius);
      glm::vec2 toStar = glm::normalize(starPos - particle.position);
      particle.velocity = massiveObjects_[starIndex].velocity +
                          glm::vec2(-toStar.y, toStar.x) * orbitalSpeed;

      particle.color = (starIndex == 0) ? sf::Color(255, 220, 180, 150)
                                        : sf::Color(180, 200, 255, 150);

      particle.size = particleSize_;
      particle.mass = 1.0f;
      particle.lifetime = 1000000.0f;
      particle.active = true;

      particleSystem_->EmitParticle(particle);
    }
    break;
  }

  case 2: { // Globular Cluster
    // Dense sphere of stars
    float clusterRadius = 300.0f;

    for (int i = 0; i < 40000; ++i) {
      // Uniform distribution in sphere
      float theta = angleDist(rng_);
      float phi = std::acos(1.0f - 2.0f * radiusDist(rng_));
      float r =
          clusterRadius *
          std::pow(radiusDist(rng_), 0.333f); // Cube root for uniform volume

      Graphics::Particle particle;
      particle.position =
          center + glm::vec2(r * std::sin(phi) * std::cos(theta),
                             r * std::sin(phi) * std::sin(theta));

      // Random velocity with slight overall rotation
      particle.velocity =
          glm::vec2(std::normal_distribution<float>(-10.0f, 20.0f)(rng_),
                    std::normal_distribution<float>(-10.0f, 20.0f)(rng_));

      // Color variation for different star types
      float starType = radiusDist(rng_);
      if (starType < 0.7f) {
        particle.color = sf::Color(255, 255, 200, 200); // Main sequence
      } else if (starType < 0.9f) {
        particle.color = sf::Color(255, 150, 100, 200); // Red giants
      } else {
        particle.color = sf::Color(150, 180, 255, 255); // Blue giants
      }

      particle.size = particleSize_ * (starType < 0.9f ? 1.0f : 1.5f);
      particle.mass = 1.0f;
      particle.lifetime = 1000000.0f;
      particle.active = true;

      particleSystem_->EmitParticle(particle);
    }
    break;
  }

  default:
    CreateGalaxyPreset(0); // Default to spiral
    break;
  }

  spdlog::info(
      "Created galaxy preset {} with {} particles and {} massive objects",
      preset, particleSystem_->GetActiveParticleCount(), massiveObjects_.size());
}

void ParticleGalaxyMode::Update(float deltaTime) {
  if (paused_)
    return;

  // Handle demo mode
  if (demoMode_) {
    demoTimer_ += deltaTime;
    if (demoTimer_ >= DEMO_DURATION) {
      demoTimer_ = 0.0f;
      currentPreset_ = (currentPreset_ + 1) % NUM_PRESETS;
      CreateGalaxyPreset(currentPreset_);
      spdlog::info("Demo mode: Switched to preset {}", currentPreset_ + 1);
    }
  }

  float scaledDeltaTime = deltaTime * timeDilation_;

  // Update physics in parallel
  UpdatePhysics(scaledDeltaTime);

  // Update particle system for rendering
  particleSystem_->Update(scaledDeltaTime);
}

void ParticleGalaxyMode::UpdatePhysics(float deltaTime) {
  // Update massive objects (they affect each other)
  for (std::size_t i = 0; i < massiveObjects_.size(); ++i) {
    glm::vec2 totalForce(0.0f, 0.0f);

    for (std::size_t j = 0; j < massiveObjects_.size(); ++j) {
      if (i == j)
        continue;

      totalForce += CalculateGravitationalForce(
          massiveObjects_[i].position, massiveObjects_[j].position,
          massiveObjects_[i].mass, massiveObjects_[j].mass);
    }

    glm::vec2 acceleration = totalForce / massiveObjects_[i].mass;
    massiveObjects_[i].velocity += acceleration * deltaTime;
    massiveObjects_[i].position += massiveObjects_[i].velocity * deltaTime;

    // Update trail
    if (showTrails_) {
      massiveObjects_[i].trail.push_back(massiveObjects_[i].position);
      if (massiveObjects_[i].trail.size() > CelestialBody::MAX_TRAIL_LENGTH) {
        massiveObjects_[i].trail.erase(massiveObjects_[i].trail.begin());
      }
    }
  }

  // Update particles in parallel batches
  auto& particles = particleSystem_->GetParticles();
  const std::size_t batchSize = particles.size() / threadPool_->GetNumThreads();
  std::vector<std::future<void>> futures;

  for (std::size_t i = 0; i < threadPool_->GetNumThreads(); ++i) {
    std::size_t start = i * batchSize;
    std::size_t end = (i == threadPool_->GetNumThreads() - 1)
                          ? particles.size()
                          : (i + 1) * batchSize;

    futures.push_back(threadPool_->Submit([this, start, end, deltaTime]() {
      UpdateParticlePhysics(start, end, deltaTime);
    }));
  }

  // Wait for all threads to complete
  for (auto &future : futures) {
    future.wait();
  }
}

void ParticleGalaxyMode::UpdateParticlePhysics(std::size_t start,
                                               std::size_t end,
                                               float deltaTime) {
  auto& particles = particleSystem_->GetParticles();
  
  for (std::size_t i = start; i < end && i < particles.size(); ++i) {
    if (!particles[i].active)
      continue;

    glm::vec2 totalForce(0.0f, 0.0f);

    // Calculate force from massive objects only (optimization)
    for (const auto &massiveObject : massiveObjects_) {
      totalForce += CalculateGravitationalForce(
          particles[i].position, massiveObject.position,
          particles[i].mass, massiveObject.mass);
    }

    glm::vec2 acceleration = totalForce / particles[i].mass;
    particles[i].velocity += acceleration * deltaTime;
    particles[i].position += particles[i].velocity * deltaTime;

    // Check if particle is too far from center (remove if so)
    auto windowSize = GetDisplaySystem().GetWindow().getSize();
    glm::vec2 center(windowSize.x * 0.5f, windowSize.y * 0.5f);
    float distanceFromCenter =
        glm::length(particles[i].position - center);

    if (distanceFromCenter > windowSize.x * 1.5f) {
      particles[i].active = false;
    }
  }
}

glm::vec2 ParticleGalaxyMode::CalculateGravitationalForce(const glm::vec2 &pos1,
                                                          const glm::vec2 &pos2,
                                                          float mass1,
                                                          float mass2) const {
  glm::vec2 direction = pos2 - pos1;
  float distanceSq = glm::dot(direction, direction);

  // Prevent division by zero and extreme forces
  constexpr float MIN_DISTANCE_SQ = 10.0f;
  distanceSq = std::max(distanceSq, MIN_DISTANCE_SQ);

  float forceMagnitude = gravitationalConstant_ * mass1 * mass2 / distanceSq;
  glm::vec2 forceDirection = glm::normalize(direction);

  return forceDirection * forceMagnitude;
}

void ParticleGalaxyMode::Render(sf::RenderTarget &target) {
  auto &renderer = GetDisplaySystem().GetRenderer();

  // Draw grid if enabled
  if (showGrid_) {
    auto windowSize = target.getSize();
    sf::Color gridColor(50, 50, 50, 100);
    float gridSpacing = 50.0f;

    for (float x = 0; x < windowSize.x; x += gridSpacing) {
      renderer.DrawLine(glm::vec2(x, 0), glm::vec2(x, windowSize.y), gridColor,
                        0.5f);
    }
    for (float y = 0; y < windowSize.y; y += gridSpacing) {
      renderer.DrawLine(glm::vec2(0, y), glm::vec2(windowSize.x, y), gridColor,
                        0.5f);
    }
  }

  // Draw trails for massive objects
  if (showTrails_) {
    for (const auto &body : massiveObjects_) {
      for (std::size_t i = 1; i < body.trail.size(); ++i) {
        float alpha = static_cast<float>(i) / body.trail.size();
        sf::Color trailColor = body.color;
        trailColor.a = static_cast<sf::Uint8>(100 * alpha);

        renderer.DrawLine(body.trail[i - 1], body.trail[i], trailColor, 2.0f);
      }
    }
  }

  // Draw particles
  particleSystem_->Render(target);

  // Draw massive objects
  for (const auto &body : massiveObjects_) {
    renderer.DrawCircle(body.position, body.radius, body.color, true);

    // Draw glow effect
    for (int i = 1; i <= 3; ++i) {
      sf::Color glowColor = body.color;
      glowColor.a = static_cast<sf::Uint8>(50 / i);
      renderer.DrawCircle(body.position, body.radius + i * 5.0f, glowColor,
                          false);
    }
  }

  // Draw UI info
  sf::Font font;
  if (font.loadFromFile("Assets/Fonts/arial.ttf")) {
    sf::Text infoText;
    infoText.setFont(font);
    infoText.setCharacterSize(14);
    infoText.setFillColor(sf::Color::White);

    std::string info = "Particle Galaxy Mode\n";
    info += "Particles: " +
            std::to_string(particleSystem_->GetActiveParticleCount()) + "\n";
    info += "Time Dilation: " + std::to_string(timeDilation_) + "x\n";
    info += "Preset: " + std::to_string(currentPreset_ + 1) + "/" +
            std::to_string(NUM_PRESETS) + "\n";
    info += "Controls: 1-5: Presets, Mouse: Add mass, Scroll: Time dilation\n";
    info += "Space: Pause, T: Trails, G: Grid";

    infoText.setString(info);
    infoText.setPosition(10, 10);
    target.draw(infoText);
  }
}

void ParticleGalaxyMode::HandleInput(const Core::InputEvent &event) {
  switch (event.type) {
  case Core::InputEvent::Type::KeyPressed:
    if (event.key.code >= sf::Keyboard::Num1 &&
        event.key.code <= sf::Keyboard::Num5) {
      int preset = event.key.code - sf::Keyboard::Num1;
      if (preset < NUM_PRESETS) {
        CreateGalaxyPreset(preset);
      }
    } else if (event.key.code == sf::Keyboard::Space) {
      paused_ = !paused_;
    } else if (event.key.code == sf::Keyboard::T) {
      showTrails_ = !showTrails_;
      if (!showTrails_) {
        for (auto &body : massiveObjects_) {
          body.trail.clear();
        }
      }
    } else if (event.key.code == sf::Keyboard::G) {
      showGrid_ = !showGrid_;
    } else if (event.key.code == sf::Keyboard::R) {
      CreateGalaxyPreset(currentPreset_);
    }
    break;

  case Core::InputEvent::Type::MouseButtonPressed:
    if (event.mouseButton.button == sf::Mouse::Left) {
      AddMassiveObject(event.mouseButton.position);
    }
    break;

  case Core::InputEvent::Type::MouseWheelScrolled:
    timeDilation_ *= (event.mouseWheel.delta > 0) ? 1.1f : 0.9f;
    timeDilation_ = glm::clamp(timeDilation_, 0.1f, 10.0f);
    break;

  default:
    break;
  }
}

void ParticleGalaxyMode::AddMassiveObject(const glm::vec2 &position) {
  CelestialBody newBody;
  newBody.position = position;
  newBody.velocity = glm::vec2(0.0f, 0.0f);
  newBody.mass = 1000.0f;
  newBody.radius = 8.0f;
  newBody.color = sf::Color(std::uniform_int_distribution<int>(150, 255)(rng_),
                            std::uniform_int_distribution<int>(150, 255)(rng_),
                            std::uniform_int_distribution<int>(150, 255)(rng_));

  massiveObjects_.push_back(newBody);
  spdlog::info("Added massive object at ({}, {})", position.x, position.y);
}

void ParticleGalaxyMode::OnActivate() {
  spdlog::info("Particle Galaxy Mode activated");
}

void ParticleGalaxyMode::OnDeactivate() {
  spdlog::info("Particle Galaxy Mode deactivated");
}

} // namespace Modes