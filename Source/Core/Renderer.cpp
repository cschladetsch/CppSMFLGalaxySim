#include "Core/Renderer.hpp"
#include "Core/Camera2D.hpp"
#include "Graphics/ParticleSystem.hpp"
#include <spdlog/spdlog.h>

namespace Core {

Renderer::Renderer(sf::RenderWindow& window)
    : window_(window)
    , camera_(std::make_unique<Camera2D>())
    , vertexArray_(sf::PrimitiveType::Quads)
    , vertices_()
    , renderTexture_()
    , renderSprite_()
    , currentBlendMode_(sf::BlendAlpha) {
    
    vertices_.reserve(MAX_BATCH_SIZE * 4);
    
    // Create render texture for post-processing
    auto size = window_.getSize();
    if (!renderTexture_.create(size.x, size.y)) {
        spdlog::error("Failed to create render texture");
    }
    renderSprite_.setTexture(renderTexture_.getTexture());
}

Renderer::~Renderer() = default;

void Renderer::BeginFrame() {
    window_.clear(sf::Color::Black);
    vertices_.clear();
}

void Renderer::EndFrame() {
    FlushBatch();
    window_.display();
}

void Renderer::DrawParticles(std::span<const Graphics::Particle> particles) {
    for (const auto& particle : particles) {
        if (!particle.active) continue;
        
        sf::Vector2f pos(particle.position.x, particle.position.y);
        float halfSize = particle.size * 0.5f;
        
        // Create quad for particle
        sf::Vertex v1, v2, v3, v4;
        
        v1.position = pos + sf::Vector2f(-halfSize, -halfSize);
        v2.position = pos + sf::Vector2f(halfSize, -halfSize);
        v3.position = pos + sf::Vector2f(halfSize, halfSize);
        v4.position = pos + sf::Vector2f(-halfSize, halfSize);
        
        v1.color = v2.color = v3.color = v4.color = particle.color;
        
        vertices_.push_back(v1);
        vertices_.push_back(v2);
        vertices_.push_back(v3);
        vertices_.push_back(v4);
        
        if (vertices_.size() >= MAX_BATCH_SIZE * 4) {
            FlushBatch();
        }
    }
}

void Renderer::DrawText(const std::string& text, const glm::vec2& position, const TextStyle& style) {
    if (!style.font) return;
    
    sf::Text sfText(text, *style.font, style.characterSize);
    sfText.setPosition(position.x, position.y);
    sfText.setFillColor(style.color);
    sfText.setOutlineColor(style.outlineColor);
    sfText.setOutlineThickness(style.outlineThickness);
    
    window_.draw(sfText);
}

void Renderer::DrawLine(const glm::vec2& start, const glm::vec2& end, const sf::Color& color, float thickness) {
    sf::VertexArray line(sf::Lines, 2);
    line[0].position = sf::Vector2f(start.x, start.y);
    line[0].color = color;
    line[1].position = sf::Vector2f(end.x, end.y);
    line[1].color = color;
    
    window_.draw(line);
}

void Renderer::DrawCircle(const glm::vec2& center, float radius, const sf::Color& color, bool filled) {
    sf::CircleShape circle(radius);
    circle.setPosition(center.x - radius, center.y - radius);
    
    if (filled) {
        circle.setFillColor(color);
    } else {
        circle.setFillColor(sf::Color::Transparent);
        circle.setOutlineColor(color);
        circle.setOutlineThickness(1.0f);
    }
    
    window_.draw(circle);
}

void Renderer::DrawRectangle(const glm::vec2& position, const glm::vec2& size, const sf::Color& color, bool filled) {
    sf::RectangleShape rect(sf::Vector2f(size.x, size.y));
    rect.setPosition(position.x, position.y);
    
    if (filled) {
        rect.setFillColor(color);
    } else {
        rect.setFillColor(sf::Color::Transparent);
        rect.setOutlineColor(color);
        rect.setOutlineThickness(1.0f);
    }
    
    window_.draw(rect);
}

void Renderer::SetCamera(const Camera2D& camera) {
    window_.setView(camera.GetView());
}

void Renderer::ResetCamera() {
    window_.setView(window_.getDefaultView());
}

void Renderer::SetBlendMode(sf::BlendMode mode) {
    currentBlendMode_ = mode;
}

void Renderer::ResetBlendMode() {
    currentBlendMode_ = sf::BlendAlpha;
}

void Renderer::FlushBatch() {
    if (vertices_.empty()) return;
    
    vertexArray_.clear();
    for (const auto& vertex : vertices_) {
        vertexArray_.append(vertex);
    }
    
    sf::RenderStates states;
    states.blendMode = currentBlendMode_;
    
    window_.draw(vertexArray_, states);
    vertices_.clear();
}

void Renderer::DrawMesh(const Graphics::Mesh& mesh, const Transform& transform) {
    // TODO: Implement mesh rendering
}

void Renderer::ApplyPostProcessing(const PostProcessChain& effects) {
    // TODO: Implement post-processing
}

} // namespace Core