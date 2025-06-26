#pragma once

#include <SFML/Graphics.hpp>
#include <memory>

namespace Graphics {

class PostProcessEffect {
public:
    virtual ~PostProcessEffect() = default;
    virtual void Apply(sf::RenderTexture& source, sf::RenderTexture& target) = 0;
};

} // namespace Graphics