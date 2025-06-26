# Assets Directory

This directory contains all external resources used by the CppSFMLVisualizer application.

## Structure

### Fonts/
TrueType and OpenType fonts for text rendering:
- Place `.ttf` and `.otf` files here
- Default font: `arial.ttf` (you need to add this)

### Textures/
Image files for sprites and textures:
- Supported formats: PNG, JPG, BMP
- Use PNG for transparency support
- Keep texture sizes as powers of 2 for best performance

### Audio/
Sound effects and music files:
- Supported formats: WAV, OGG, FLAC
- Use OGG for compressed music
- Use WAV for short sound effects

### Shaders/
GLSL shader files for advanced effects:
- Vertex shaders (`.vert`)
- Fragment shaders (`.frag`)
- Geometry shaders (`.geom`)

## Asset Loading

Assets are automatically copied to the build directory during compilation. In code, load them using relative paths:

```cpp
font.loadFromFile("Assets/Fonts/arial.ttf");
texture.loadFromFile("Assets/Textures/particle.png");
```

## Asset Guidelines

- Keep file sizes reasonable (textures < 4096x4096)
- Use appropriate compression for file types
- Organize assets in subdirectories by type
- Include license information for third-party assets