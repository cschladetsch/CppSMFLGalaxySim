# Core Systems

This directory contains the fundamental systems that power the CppSFMLVisualizer engine.

## Components

### DisplaySystem
The main application controller that manages:
- Window creation and event processing
- Visual mode registration and switching
- Main game loop with fixed timestep
- Performance metrics collection

### Renderer
High-performance 2D rendering system featuring:
- Batched draw calls for particles and primitives
- Camera system integration
- Post-processing effects support
- Multiple blend modes

### Camera2D
2D camera system providing:
- Position, rotation, and zoom controls
- Screen-to-world coordinate transformations
- Viewport management

### ThreadPool
Modern C++23 thread pool implementation:
- Work-stealing queue for load balancing
- Future-based task submission
- Parallel algorithms support
- Automatic thread count detection

### VisualMode
Base interface for all visualization modes:
- Lifecycle management (Initialize, Update, Render)
- Input event handling
- Mode switching support

## Architecture

The core systems follow these design principles:
- **RAII**: Resource management through constructors/destructors
- **Interface Segregation**: Small, focused interfaces
- **Dependency Injection**: Systems receive dependencies through constructors
- **Error Handling**: Using std::expected for error propagation