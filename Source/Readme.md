# Source Directory

This directory contains all the implementation files (.cpp) for the CppSFMLVisualizer project.

## Structure

### Core/
Core engine systems that power the application:
- `DisplaySystem.cpp` - Main application loop and window management
- `Renderer.cpp` - 2D rendering pipeline with batching support
- `Camera2D.cpp` - 2D camera system for view transformations
- `ThreadPool.cpp` - Multi-threading support for parallel computations
- `VisualMode.cpp` - Base class for visual modes

### Graphics/
Graphics and rendering components:
- `ParticleSystem.cpp` - High-performance particle rendering system
- `PostProcessing.cpp` - Post-processing effects pipeline

### Physics/
Physics simulation components:
- `PhysicsEngine.cpp` - Physics calculations and simulations

### Audio/
Audio processing and analysis:
- `AudioAnalyzer.cpp` - Real-time audio spectrum analysis

### Input/
Input handling and event processing:
- `InputManager.cpp` - Keyboard, mouse, and gamepad input handling

### Utils/
Utility functions and helpers:
- `Math.cpp` - Mathematical utilities and helper functions
- `PerformanceProfiler.cpp` - Performance monitoring and profiling

### Modes/
Visual mode implementations:
- `ParticleGalaxyMode.cpp` - N-body gravitational simulation with 50k+ particles

## Adding New Files

When adding new source files:
1. Place them in the appropriate subdirectory
2. Add the corresponding header file to `Include/`
3. Update `CMakeLists.txt` to include the new source file
4. Follow the existing code style and naming conventions