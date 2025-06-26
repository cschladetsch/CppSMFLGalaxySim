#pragma once

#include <memory>
#include <vector>
#include <span>
#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>

namespace Graphics {
    struct Particle;
    struct Mesh {
        // Placeholder for mesh data
    };
    class PostProcessEffect;
}

namespace Core {

class Camera2D;

struct Transform {
    glm::vec2 position{0.0f, 0.0f};
    float rotation = 0.0f;
    glm::vec2 scale{1.0f, 1.0f};
};

struct TextStyle {
    sf::Font* font = nullptr;
    unsigned int characterSize = 16;
    sf::Color color = sf::Color::White;
    float outlineThickness = 0.0f;
    sf::Color outlineColor = sf::Color::Black;
};

struct PostProcessChain {
    std::vector<std::unique_ptr<Graphics::PostProcessEffect>> effects;
};

class Renderer {
public:
    explicit Renderer(sf::RenderWindow& window);
    ~Renderer();
    
    void BeginFrame();
    void EndFrame();
    
    void DrawParticles(std::span<const Graphics::Particle> particles);
    void DrawMesh(const Graphics::Mesh& mesh, const Transform& transform);
    void DrawText(const std::string& text, const glm::vec2& position, const TextStyle& style);
    void DrawLine(const glm::vec2& start, const glm::vec2& end, const sf::Color& color, float thickness = 1.0f);
    void DrawCircle(const glm::vec2& center, float radius, const sf::Color& color, bool filled = true);
    void DrawRectangle(const glm::vec2& position, const glm::vec2& size, const sf::Color& color, bool filled = true);
    
    void SetCamera(const Camera2D& camera);
    void ResetCamera();
    void ApplyPostProcessing(const PostProcessChain& effects);
    
    void SetBlendMode(sf::BlendMode mode);
    void ResetBlendMode();
    
    [[nodiscard]] sf::RenderWindow& GetWindow() noexcept { return window_; }
    [[nodiscard]] const sf::RenderWindow& GetWindow() const noexcept { return window_; }
    
private:
    void UpdateVertexArray();
    void FlushBatch();
    
private:
    sf::RenderWindow& window_;
    std::unique_ptr<Camera2D> camera_;
    
    sf::VertexArray vertexArray_;
    std::vector<sf::Vertex> vertices_;
    
    sf::RenderTexture renderTexture_;
    sf::Sprite renderSprite_;
    
    sf::BlendMode currentBlendMode_ = sf::BlendAlpha;
    
    static constexpr std::size_t MAX_BATCH_SIZE = 10000;
};

} // namespace Core