#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <SFML/Graphics.hpp>
#include <random>
#include <concepts>

namespace Graphics {

struct Particle {
    glm::vec2 position{0.0f, 0.0f};
    glm::vec2 velocity{0.0f, 0.0f};
    glm::vec2 acceleration{0.0f, 0.0f};
    sf::Color color{255, 255, 255, 255};
    float size = 1.0f;
    float lifetime = 1.0f;
    float age = 0.0f;
    float mass = 1.0f;
    bool active = true;
};

template<typename T>
concept ParticleEmitter = requires(T emitter, Particle& particle) {
    { emitter.Emit(particle) } -> std::same_as<void>;
    { emitter.GetEmissionRate() } -> std::convertible_to<float>;
};

template<typename T>
concept ParticleUpdater = requires(T updater, Particle& particle, float deltaTime) {
    { updater.Update(particle, deltaTime) } -> std::same_as<void>;
};

class ParticleSystem {
public:
    explicit ParticleSystem(std::size_t maxParticles = 10000);
    ~ParticleSystem();
    
    void Update(float deltaTime);
    void Render(sf::RenderTarget& target);
    
    template<ParticleEmitter E>
    void SetEmitter(std::unique_ptr<E> emitter) {
        emitter_ = std::move(emitter);
    }
    
    template<ParticleUpdater U>
    void AddUpdater(std::unique_ptr<U> updater) {
        updaters_.push_back(std::move(updater));
    }
    
    void EmitParticle(const Particle& particleTemplate);
    void EmitBurst(std::size_t count, const Particle& particleTemplate);
    
    void Clear();
    void SetBlendMode(sf::BlendMode mode) { blendMode_ = mode; }
    
    [[nodiscard]] std::size_t GetActiveParticleCount() const;
    [[nodiscard]] std::size_t GetMaxParticles() const { return maxParticles_; }
    
    void SetGravity(const glm::vec2& gravity) { gravity_ = gravity; }
    void SetDamping(float damping) { damping_ = damping; }
    
private:
    void UpdateParticle(Particle& particle, float deltaTime);
    Particle* GetInactiveParticle();
    
private:
    std::vector<Particle> particles_;
    std::size_t maxParticles_;
    
    std::unique_ptr<void, void(*)(void*)> emitter_{nullptr, [](void*){}};
    std::vector<std::unique_ptr<void, void(*)(void*)>> updaters_;
    
    sf::VertexArray vertices_;
    sf::BlendMode blendMode_ = sf::BlendAdd;
    
    glm::vec2 gravity_{0.0f, 0.0f};
    float damping_ = 0.99f;
    float emissionAccumulator_ = 0.0f;
    
    std::mt19937 rng_{std::random_device{}()};
};

class RandomEmitter {
public:
    RandomEmitter(const glm::vec2& position, float radius, float emissionRate);
    
    void Emit(Particle& particle);
    float GetEmissionRate() const { return emissionRate_; }
    
    void SetPosition(const glm::vec2& position) { position_ = position; }
    void SetRadius(float radius) { radius_ = radius; }
    void SetVelocityRange(const glm::vec2& min, const glm::vec2& max) {
        velocityMin_ = min;
        velocityMax_ = max;
    }
    
private:
    glm::vec2 position_;
    float radius_;
    float emissionRate_;
    glm::vec2 velocityMin_{-100.0f, -100.0f};
    glm::vec2 velocityMax_{100.0f, 100.0f};
    
    std::mt19937 rng_{std::random_device{}()};
    std::uniform_real_distribution<float> angleDist_{0.0f, 2.0f * 3.14159f};
    std::uniform_real_distribution<float> radiusDist_{0.0f, 1.0f};
    std::uniform_real_distribution<float> velocityDist_{0.0f, 1.0f};
};

class ColorUpdater {
public:
    ColorUpdater(const sf::Color& startColor, const sf::Color& endColor);
    
    void Update(Particle& particle, float deltaTime);
    
private:
    sf::Color startColor_;
    sf::Color endColor_;
};

} // namespace Graphics