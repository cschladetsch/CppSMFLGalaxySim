#include "Graphics/ParticleSystem.hpp"
#include <algorithm>

namespace Graphics {

ParticleSystem::ParticleSystem(std::size_t maxParticles)
    : particles_(maxParticles)
    , maxParticles_(maxParticles)
    , vertices_(sf::PrimitiveType::Triangles)
    , blendMode_(sf::BlendAdd)
    , gravity_(0.0f, 0.0f)
    , damping_(0.99f)
    , emissionAccumulator_(0.0f) {
    
    // Reserve space for triangles (6 segments per particle, 3 vertices per triangle)
    vertices_.resize(maxParticles * 6 * 3);
}

ParticleSystem::~ParticleSystem() = default;

void ParticleSystem::Update(float deltaTime) {
    // Update emission
    if (emitter_) {
        // TODO: Handle emitter updates
    }
    
    // Update particles
    for (auto& particle : particles_) {
        if (!particle.active) continue;
        
        UpdateParticle(particle, deltaTime);
        
        // Apply updaters
        for ([[maybe_unused]] const auto& updater : updaters_) {
            // TODO: Apply updaters
        }
    }
}

void ParticleSystem::Render(sf::RenderTarget& target) {
    // Clear and prepare vertex array
    vertices_.clear();
    vertices_.setPrimitiveType(sf::PrimitiveType::Quads);
    
    // Count active particles for optimization
    std::size_t activeCount = 0;
    for (const auto& particle : particles_) {
        if (particle.active) activeCount++;
    }
    
    // Pre-allocate vertices
    vertices_.resize(activeCount * 4);
    std::size_t vertexIndex = 0;
    
    // Render particles as quads (much faster than circles)
    for (const auto& particle : particles_) {
        if (!particle.active) continue;
        
        float halfSize = particle.size * 0.5f;
        sf::Vector2f pos(particle.position.x, particle.position.y);
        
        // Top-left
        vertices_[vertexIndex].position = pos + sf::Vector2f(-halfSize, -halfSize);
        vertices_[vertexIndex].color = particle.color;
        vertexIndex++;
        
        // Top-right
        vertices_[vertexIndex].position = pos + sf::Vector2f(halfSize, -halfSize);
        vertices_[vertexIndex].color = particle.color;
        vertexIndex++;
        
        // Bottom-right
        vertices_[vertexIndex].position = pos + sf::Vector2f(halfSize, halfSize);
        vertices_[vertexIndex].color = particle.color;
        vertexIndex++;
        
        // Bottom-left
        vertices_[vertexIndex].position = pos + sf::Vector2f(-halfSize, halfSize);
        vertices_[vertexIndex].color = particle.color;
        vertexIndex++;
    }
    
    // Draw all particles in one draw call
    sf::RenderStates states;
    states.blendMode = blendMode_;
    target.draw(vertices_, states);
}

void ParticleSystem::EmitParticle(const Particle& particleTemplate) {
    Particle* particle = GetInactiveParticle();
    if (particle) {
        *particle = particleTemplate;
        particle->active = true;
        particle->age = 0.0f;
    }
}

void ParticleSystem::EmitBurst(std::size_t count, const Particle& particleTemplate) {
    for (std::size_t i = 0; i < count; ++i) {
        EmitParticle(particleTemplate);
    }
}

void ParticleSystem::Clear() {
    for (auto& particle : particles_) {
        particle.active = false;
    }
}

std::size_t ParticleSystem::GetActiveParticleCount() const {
    return std::count_if(particles_.begin(), particles_.end(),
        [](const Particle& p) { return p.active; });
}

void ParticleSystem::UpdateParticle(Particle& particle, float deltaTime) {
    particle.age += deltaTime;
    
    if (particle.age >= particle.lifetime) {
        particle.active = false;
        return;
    }
    
    // Apply physics
    particle.velocity += (particle.acceleration + gravity_) * deltaTime;
    particle.velocity *= damping_;
    particle.position += particle.velocity * deltaTime;
    
    // Update alpha based on age
    float lifeRatio = particle.age / particle.lifetime;
    particle.color.a = static_cast<sf::Uint8>(255 * (1.0f - lifeRatio));
}

Particle* ParticleSystem::GetInactiveParticle() {
    auto it = std::find_if(particles_.begin(), particles_.end(),
        [](const Particle& p) { return !p.active; });
    
    return (it != particles_.end()) ? &(*it) : nullptr;
}

// RandomEmitter implementation
RandomEmitter::RandomEmitter(const glm::vec2& position, float radius, float emissionRate)
    : position_(position)
    , radius_(radius)
    , emissionRate_(emissionRate) {
}

void RandomEmitter::Emit(Particle& particle) {
    float angle = angleDist_(rng_);
    float r = radius_ * std::sqrt(radiusDist_(rng_));
    
    particle.position = position_ + glm::vec2(r * std::cos(angle), r * std::sin(angle));
    
    particle.velocity.x = glm::mix(velocityMin_.x, velocityMax_.x, velocityDist_(rng_));
    particle.velocity.y = glm::mix(velocityMin_.y, velocityMax_.y, velocityDist_(rng_));
}

// ColorUpdater implementation
ColorUpdater::ColorUpdater(const sf::Color& startColor, const sf::Color& endColor)
    : startColor_(startColor)
    , endColor_(endColor) {
}

void ColorUpdater::Update(Particle& particle, float deltaTime) {
    float lifeRatio = particle.age / particle.lifetime;
    
    particle.color.r = static_cast<sf::Uint8>(glm::mix(startColor_.r, endColor_.r, lifeRatio));
    particle.color.g = static_cast<sf::Uint8>(glm::mix(startColor_.g, endColor_.g, lifeRatio));
    particle.color.b = static_cast<sf::Uint8>(glm::mix(startColor_.b, endColor_.b, lifeRatio));
}

} // namespace Graphics