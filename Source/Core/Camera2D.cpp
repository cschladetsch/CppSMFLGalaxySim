#include "Core/Camera2D.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace Core {

Camera2D::Camera2D() 
    : view_()
    , position_(0.0f, 0.0f)
    , size_(1920.0f, 1080.0f)
    , rotation_(0.0f)
    , zoom_(1.0f)
    , needsUpdate_(true) {
    Update();
}

Camera2D::Camera2D(const sf::FloatRect& viewport)
    : view_()
    , position_(viewport.left + viewport.width * 0.5f, viewport.top + viewport.height * 0.5f)
    , size_(viewport.width, viewport.height)
    , rotation_(0.0f)
    , zoom_(1.0f)
    , needsUpdate_(true) {
    view_.setViewport(sf::FloatRect(0, 0, 1, 1));
    Update();
}

Camera2D::~Camera2D() = default;

void Camera2D::SetPosition(const glm::vec2& position) {
    position_ = position;
    needsUpdate_ = true;
}

void Camera2D::Move(const glm::vec2& offset) {
    position_ += offset;
    needsUpdate_ = true;
}

void Camera2D::SetRotation(float degrees) {
    rotation_ = degrees;
    needsUpdate_ = true;
}

void Camera2D::Rotate(float degrees) {
    rotation_ += degrees;
    needsUpdate_ = true;
}

void Camera2D::SetZoom(float zoom) {
    zoom_ = glm::max(0.1f, zoom);
    needsUpdate_ = true;
}

void Camera2D::Zoom(float factor) {
    zoom_ *= factor;
    zoom_ = glm::max(0.1f, zoom_);
    needsUpdate_ = true;
}

void Camera2D::SetViewport(const sf::FloatRect& viewport) {
    view_.setViewport(viewport);
}

void Camera2D::SetSize(const glm::vec2& size) {
    size_ = size;
    needsUpdate_ = true;
}

glm::vec2 Camera2D::ScreenToWorld(const glm::vec2& screenPos) const {
    // TODO: Implement proper screen to world transformation
    return screenPos;
}

glm::vec2 Camera2D::WorldToScreen(const glm::vec2& worldPos) const {
    // TODO: Implement proper world to screen transformation
    return worldPos;
}

void Camera2D::Update() {
    if (!needsUpdate_) return;
    
    view_.setCenter(position_.x, position_.y);
    view_.setSize(size_.x / zoom_, size_.y / zoom_);
    view_.setRotation(rotation_);
    
    needsUpdate_ = false;
}

} // namespace Core