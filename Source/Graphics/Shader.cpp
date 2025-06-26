#include "Graphics/Shader.hpp"
#include <spdlog/spdlog.h>

namespace Graphics {

Shader::Shader() : shader_(std::make_unique<sf::Shader>()) {}

Shader::~Shader() = default;

bool Shader::LoadFromFile(const std::string& vertexShaderFile, 
                          const std::string& fragmentShaderFile) {
  if (!shader_->loadFromFile(vertexShaderFile, fragmentShaderFile)) {
    spdlog::error("Failed to load shaders: {} and {}", 
                  vertexShaderFile, fragmentShaderFile);
    return false;
  }
  return true;
}

bool Shader::LoadFromMemory(const std::string& vertexShader, 
                            const std::string& fragmentShader) {
  if (!shader_->loadFromMemory(vertexShader, fragmentShader)) {
    spdlog::error("Failed to load shaders from memory");
    return false;
  }
  return true;
}

void Shader::Use() {
  sf::Shader::bind(shader_.get());
}

void Shader::SetUniform(const std::string& name, float value) {
  shader_->setUniform(name, value);
}

void Shader::SetUniform(const std::string& name, const glm::vec2& value) {
  shader_->setUniform(name, sf::Vector2f(value.x, value.y));
}

void Shader::SetUniform(const std::string& name, const glm::vec3& value) {
  shader_->setUniform(name, sf::Vector3f(value.x, value.y, value.z));
}

void Shader::SetUniform(const std::string& name, const glm::vec4& value) {
  shader_->setUniform(name, sf::Glsl::Vec4(value.x, value.y, value.z, value.w));
}

void Shader::SetUniform(const std::string& name, const glm::mat4& value) {
  shader_->setUniform(name, sf::Glsl::Mat4(&value[0][0]));
}

void Shader::SetUniform(const std::string& name, int value) {
  shader_->setUniform(name, value);
}

void Shader::SetUniform(const std::string& name, const sf::Texture& texture) {
  shader_->setUniform(name, texture);
}

ShaderManager& ShaderManager::GetInstance() {
  static ShaderManager instance;
  return instance;
}

bool ShaderManager::LoadShader(const std::string& name, 
                               const std::string& vertexFile, 
                               const std::string& fragmentFile) {
  auto shader = std::make_unique<Shader>();
  if (!shader->LoadFromFile(vertexFile, fragmentFile)) {
    return false;
  }
  
  shaders_[name] = std::move(shader);
  spdlog::info("Loaded shader: {}", name);
  return true;
}

bool ShaderManager::LoadShaderFromMemory(const std::string& name,
                                         const std::string& vertexShader,
                                         const std::string& fragmentShader) {
  auto shader = std::make_unique<Shader>();
  if (!shader->LoadFromMemory(vertexShader, fragmentShader)) {
    return false;
  }
  
  shaders_[name] = std::move(shader);
  spdlog::info("Loaded shader from memory: {}", name);
  return true;
}

Shader* ShaderManager::GetShader(const std::string& name) {
  auto it = shaders_.find(name);
  return it != shaders_.end() ? it->second.get() : nullptr;
}

void ShaderManager::ReleaseShader(const std::string& name) {
  shaders_.erase(name);
}

void ShaderManager::ReleaseAll() {
  shaders_.clear();
}

} // namespace Graphics