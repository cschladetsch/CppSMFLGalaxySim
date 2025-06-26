#include "Graphics/GPUParticleSystem.hpp"
#include <spdlog/spdlog.h>
#include <algorithm>
#include <random>

namespace Graphics {

// Particle vertex shader with GPU physics
static const char* particleVertexShader = R"(
#version 130

uniform float deltaTime;
uniform vec2 gravity;
uniform float damping;
uniform vec3 attractors[32]; // x, y, mass
uniform int numAttractors;
uniform mat4 projectionMatrix;

attribute vec2 position;
attribute vec2 velocity;
attribute vec2 acceleration;
attribute float lifetime;
attribute float age;
attribute float size;
attribute float mass;
attribute vec4 color;

varying vec4 fragColor;
varying float fragLifeRatio;

void main() {
    // Update age
    float newAge = age + deltaTime;
    fragLifeRatio = newAge / lifetime;
    
    // Calculate total acceleration including gravity and attractors
    vec2 totalAccel = acceleration + gravity;
    
    // N-body gravity calculations
    for (int i = 0; i < numAttractors; ++i) {
        vec2 diff = attractors[i].xy - position;
        float distSq = dot(diff, diff) + 0.001; // Prevent division by zero
        float force = attractors[i].z / distSq;
        totalAccel += normalize(diff) * force;
    }
    
    // Update velocity and position
    vec2 newVelocity = (velocity + totalAccel * deltaTime) * damping;
    vec2 newPosition = position + newVelocity * deltaTime;
    
    // Fade out based on age
    fragColor = color;
    fragColor.a *= (1.0 - fragLifeRatio);
    
    // Transform position
    gl_Position = projectionMatrix * vec4(newPosition, 0.0, 1.0);
    gl_PointSize = size * (1.0 - fragLifeRatio * 0.5);
}
)";

// Particle fragment shader
static const char* particleFragmentShader = R"(
#version 130

varying vec4 fragColor;
varying float fragLifeRatio;

void main() {
    // Calculate distance from center of point
    vec2 coord = gl_PointCoord - vec2(0.5);
    float dist = length(coord);
    
    // Soft particle edges
    float alpha = fragColor.a * smoothstep(0.5, 0.0, dist);
    
    // Add glow effect
    vec3 color = fragColor.rgb;
    float glow = exp(-dist * 3.0);
    color += fragColor.rgb * glow * 0.5;
    
    gl_FragColor = vec4(color, alpha);
}
)";

// Instanced rendering vertex shader
static const char* instancedVertexShader = R"(
#version 130

uniform mat4 projectionMatrix;
uniform sampler2D particleData; // Texture containing particle data
uniform float textureSize;

attribute vec2 vertexPosition; // Quad vertex position (-0.5 to 0.5)
attribute float instanceID;

varying vec4 fragColor;
varying vec2 texCoord;

void main() {
    // Fetch particle data from texture
    float texelIndex = instanceID / textureSize;
    vec4 data1 = texture2D(particleData, vec2(texelIndex, 0.0));
    vec4 data2 = texture2D(particleData, vec2(texelIndex, 0.25));
    vec4 data3 = texture2D(particleData, vec2(texelIndex, 0.5));
    
    vec2 position = data1.xy;
    float size = data2.x;
    vec4 color = data3;
    
    // Transform vertex
    vec2 worldPos = position + vertexPosition * size;
    gl_Position = projectionMatrix * vec4(worldPos, 0.0, 1.0);
    
    fragColor = color;
    texCoord = vertexPosition + vec2(0.5);
}
)";

// Instanced rendering fragment shader  
static const char* instancedFragmentShader = R"(
#version 130

varying vec4 fragColor;
varying vec2 texCoord;

void main() {
    // Calculate distance from center
    vec2 coord = texCoord - vec2(0.5);
    float dist = length(coord);
    
    // Soft edges with glow
    float alpha = fragColor.a * smoothstep(0.5, 0.0, dist);
    vec3 color = fragColor.rgb;
    
    // Enhanced glow effect
    float glow = exp(-dist * 4.0);
    color += fragColor.rgb * glow * 0.8;
    
    // Add bloom-like bright core
    float core = exp(-dist * 8.0);
    color += vec3(1.0) * core * 0.3;
    
    gl_FragColor = vec4(color, alpha);
}
)";

GPUParticleSystem::GPUParticleSystem(std::size_t maxParticles)
    : maxParticles_(maxParticles), activeParticles_(0),
      particles_(maxParticles), particleBuffer_(sf::PrimitiveType::Points),
      gravity_(0.0f, 0.0f), damping_(0.99f), blendMode_(sf::BlendAdd) {
    
    InitializeShaders();
    InitializeBuffers();
}

GPUParticleSystem::~GPUParticleSystem() = default;

void GPUParticleSystem::InitializeShaders() {
    auto& shaderMgr = ShaderManager::GetInstance();
    
    // Load particle update/render shader
    if (!shaderMgr.LoadShaderFromMemory("gpu_particle", 
                                        particleVertexShader, 
                                        particleFragmentShader)) {
        spdlog::error("Failed to load GPU particle shaders");
        
        // Fallback to basic shaders
        const char* basicVertex = R"(
            void main() {
                gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
                gl_FrontColor = gl_Color;
            }
        )";
        
        const char* basicFragment = R"(
            void main() {
                gl_FragColor = gl_Color;
            }
        )";
        
        shaderMgr.LoadShaderFromMemory("gpu_particle", basicVertex, basicFragment);
    }
    
    particleRenderShader_.reset(new Shader());
    particleRenderShader_->LoadFromMemory(instancedVertexShader, instancedFragmentShader);
    
    // Initialize quad template for instanced rendering
    quadTemplate_.setPrimitiveType(sf::PrimitiveType::TriangleStrip);
    quadTemplate_.resize(4);
    quadTemplate_[0].position = sf::Vector2f(-0.5f, -0.5f);
    quadTemplate_[1].position = sf::Vector2f(0.5f, -0.5f);
    quadTemplate_[2].position = sf::Vector2f(-0.5f, 0.5f);
    quadTemplate_[3].position = sf::Vector2f(0.5f, 0.5f);
}

void GPUParticleSystem::InitializeBuffers() {
    // Create vertex buffer for particles
    if (!particleBuffer_.create(maxParticles_)) {
        spdlog::error("Failed to create particle vertex buffer");
    }
    particleBuffer_.setPrimitiveType(sf::PrimitiveType::Points);
}

void GPUParticleSystem::Update(float deltaTime) {
    // GPU particles update themselves in the vertex shader
    // Here we just manage particle lifecycle on CPU
    
    for (std::size_t i = 0; i < activeParticles_; ++i) {
        particles_[i].age += deltaTime;
        
        // Deactivate dead particles
        if (particles_[i].age >= particles_[i].lifetime) {
            // Swap with last active particle
            if (i < activeParticles_ - 1) {
                std::swap(particles_[i], particles_[activeParticles_ - 1]);
                --i; // Re-check this index
            }
            --activeParticles_;
        }
    }
    
    UpdateParticleBuffer();
}

void GPUParticleSystem::UpdateParticleBuffer() {
    std::vector<sf::Vertex> vertices(activeParticles_);
    
    for (std::size_t i = 0; i < activeParticles_; ++i) {
        const auto& p = particles_[i];
        vertices[i].position = sf::Vector2f(p.position.x, p.position.y);
        
        // Encode additional data in color channels if needed
        sf::Uint8 r = static_cast<sf::Uint8>(p.color.r * 255);
        sf::Uint8 g = static_cast<sf::Uint8>(p.color.g * 255);
        sf::Uint8 b = static_cast<sf::Uint8>(p.color.b * 255);
        sf::Uint8 a = static_cast<sf::Uint8>(p.color.a * (1.0f - p.age / p.lifetime) * 255);
        
        vertices[i].color = sf::Color(r, g, b, a);
    }
    
    if (!vertices.empty()) {
        particleBuffer_.update(vertices.data(), vertices.size(), 0);
    }
}

void GPUParticleSystem::Render(sf::RenderTarget& target) {
    if (activeParticles_ == 0) return;
    
    sf::RenderStates states;
    states.blendMode = blendMode_;
    
    // Use GPU particle shader
    auto shader = ShaderManager::GetInstance().GetShader("gpu_particle");
    if (shader) {
        states.shader = shader->GetSFMLShader();
        
        // Set uniforms
        shader->SetUniform("gravity", gravity_);
        shader->SetUniform("damping", damping_);
        shader->SetUniform("deltaTime", 0.016f); // Assuming 60 FPS
        
        // Set attractor points
        if (!attractorPoints_.empty()) {
            shader->SetUniform("numAttractors", static_cast<int>(attractorPoints_.size()));
            // Note: SFML doesn't support array uniforms directly, 
            // so we'd need to set them individually in a real implementation
        }
    }
    
    target.draw(particleBuffer_, states);
}

void GPUParticleSystem::EmitParticle(const GPUParticleData& particle) {
    if (activeParticles_ < maxParticles_) {
        particles_[activeParticles_] = particle;
        particles_[activeParticles_].age = 0.0f;
        ++activeParticles_;
    }
}

void GPUParticleSystem::EmitBurst(std::size_t count, const GPUParticleData& particleTemplate) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * M_PI);
    std::uniform_real_distribution<float> speedDist(0.5f, 2.0f);
    
    for (std::size_t i = 0; i < count && activeParticles_ < maxParticles_; ++i) {
        GPUParticleData p = particleTemplate;
        
        // Add some randomization
        float angle = angleDist(gen);
        float speed = speedDist(gen);
        p.velocity = glm::vec2(cos(angle) * speed, sin(angle) * speed) * 100.0f;
        
        EmitParticle(p);
    }
}

void GPUParticleSystem::Clear() {
    activeParticles_ = 0;
}

void GPUParticleSystem::SetAttractorPoints(const std::vector<glm::vec3>& attractors) {
    attractorPoints_ = attractors;
}

} // namespace Graphics