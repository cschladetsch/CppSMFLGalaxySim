#pragma once

#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <unordered_map>

namespace Graphics {

class Shader {
public:
  Shader();
  ~Shader();

  bool LoadFromFile(const std::string& vertexShaderFile, 
                    const std::string& fragmentShaderFile);
  bool LoadFromMemory(const std::string& vertexShader, 
                      const std::string& fragmentShader);

  void Use();
  void SetUniform(const std::string& name, float value);
  void SetUniform(const std::string& name, const glm::vec2& value);
  void SetUniform(const std::string& name, const glm::vec3& value);
  void SetUniform(const std::string& name, const glm::vec4& value);
  void SetUniform(const std::string& name, const glm::mat4& value);
  void SetUniform(const std::string& name, int value);
  void SetUniform(const std::string& name, const sf::Texture& texture);

  sf::Shader* GetSFMLShader() { return shader_.get(); }
  const sf::Shader* GetSFMLShader() const { return shader_.get(); }

private:
  std::unique_ptr<sf::Shader> shader_;
  std::unordered_map<std::string, int> uniformLocations_;
};

class ShaderManager {
public:
  static ShaderManager& GetInstance();

  bool LoadShader(const std::string& name, 
                  const std::string& vertexFile, 
                  const std::string& fragmentFile);
  bool LoadShaderFromMemory(const std::string& name,
                            const std::string& vertexShader,
                            const std::string& fragmentShader);
  
  Shader* GetShader(const std::string& name);
  void ReleaseShader(const std::string& name);
  void ReleaseAll();

private:
  ShaderManager() = default;
  std::unordered_map<std::string, std::unique_ptr<Shader>> shaders_;
};

} // namespace Graphics