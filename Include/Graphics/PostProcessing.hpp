#pragma once

#include <SFML/Graphics.hpp>
#include "Graphics/Shader.hpp"
#include <memory>
#include <vector>

namespace Graphics {

class PostProcessEffect {
public:
  virtual ~PostProcessEffect() = default;
  virtual void Apply(sf::RenderTexture &source, sf::RenderTexture &target) = 0;
  virtual void SetEnabled(bool enabled) { enabled_ = enabled; }
  virtual bool IsEnabled() const { return enabled_; }

protected:
  bool enabled_ = true;
};

class BloomEffect : public PostProcessEffect {
public:
  BloomEffect();
  ~BloomEffect() override;
  
  void Apply(sf::RenderTexture &source, sf::RenderTexture &target) override;
  
  void SetThreshold(float threshold) { threshold_ = threshold; }
  void SetIntensity(float intensity) { intensity_ = intensity; }
  void SetBlurPasses(int passes) { blurPasses_ = passes; }
  
protected:
  void InitializeShaders();
  void ExtractBrightPixels(sf::RenderTexture &source, sf::RenderTexture &bright);
  void GaussianBlur(sf::RenderTexture &texture, bool horizontal);
  
private:
  std::unique_ptr<Shader> thresholdShader_;
  std::unique_ptr<Shader> blurShader_;
  std::unique_ptr<Shader> combineShader_;
  
  std::unique_ptr<sf::RenderTexture> brightTexture_;
  std::unique_ptr<sf::RenderTexture> blurTexture1_;
  std::unique_ptr<sf::RenderTexture> blurTexture2_;
  
  float threshold_ = 0.8f;
  float intensity_ = 1.0f;
  int blurPasses_ = 3;
};

class HDRToneMappingEffect : public PostProcessEffect {
public:
  HDRToneMappingEffect();
  ~HDRToneMappingEffect() override;
  
  void Apply(sf::RenderTexture &source, sf::RenderTexture &target) override;
  
  void SetExposure(float exposure) { exposure_ = exposure; }
  void SetGamma(float gamma) { gamma_ = gamma; }
  
private:
  void InitializeShaders();
  
  std::unique_ptr<Shader> toneMappingShader_;
  float exposure_ = 1.0f;
  float gamma_ = 2.2f;
};

class PostProcessingPipeline {
public:
  PostProcessingPipeline();
  ~PostProcessingPipeline();
  
  void AddEffect(std::unique_ptr<PostProcessEffect> effect);
  void RemoveEffect(size_t index);
  void Clear();
  
  void Apply(sf::RenderTexture &source, sf::RenderTexture &target);
  
  PostProcessEffect* GetEffect(size_t index);
  size_t GetEffectCount() const { return effects_.size(); }
  
private:
  std::vector<std::unique_ptr<PostProcessEffect>> effects_;
  std::unique_ptr<sf::RenderTexture> intermediateTexture_;
};

} // namespace Graphics