# Visual Modes

This directory contains all the visual mode implementations that showcase different graphics programming techniques.

## Implemented Modes

### ParticleGalaxyMode
A stunning N-body gravitational simulation featuring:
- 50,000+ particles with real-time physics
- Multiple galaxy presets (spiral, binary stars, globular cluster)
- Interactive celestial body placement
- Time dilation control
- Object trails visualization
- Multi-threaded physics computation

## Planned Modes

### Procedural Wave Visualizer
- Real-time audio spectrum analysis
- Multiple wave types (sine, square, sawtooth)
- 3D-looking wave surfaces
- Lissajous curves visualization

### Cellular Automata Showcase
- Conway's Game of Life
- Langton's Ant
- Rule 30 and custom rules
- Large grid support with sparse representation

### Fractal Explorer
- Mandelbrot and Julia sets
- Deep zoom with arbitrary precision
- Real-time parameter adjustment
- Progressive refinement rendering

### Network Graph Visualizer
- Force-directed layouts
- Real-time data flow animation
- Community detection
- Interactive node manipulation

## Creating New Modes

To add a new visual mode:

1. Create a new class inheriting from `Core::VisualMode`
2. Implement all virtual methods
3. Add unique visual features and interactions
4. Register the mode in `main.cpp`
5. Update CMakeLists.txt

Example structure:
```cpp
class YourMode : public Core::VisualMode {
public:
    explicit YourMode(Core::DisplaySystem& displaySystem);
    void Initialize() override;
    void Update(float deltaTime) override;
    void Render(sf::RenderTarget& target) override;
    void HandleInput(const Core::InputEvent& event) override;
    std::string GetName() const override { return "Your Mode"; }
};
```