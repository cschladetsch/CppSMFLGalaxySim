# Include Directory

This directory contains all the header files (.hpp) for the CppSFMLVisualizer project.

## Structure

### Core/
Core engine interfaces and classes:
- `DisplaySystem.hpp` - Main application controller
- `Renderer.hpp` - 2D rendering interface with batching
- `Camera2D.hpp` - 2D camera for view transformations
- `ThreadPool.hpp` - Thread pool for parallel execution
- `VisualMode.hpp` - Base interface for all visual modes

### Graphics/
Graphics components and effects:
- `ParticleSystem.hpp` - Particle system with emitters and updaters
- `PostProcessing.hpp` - Post-processing effects interface

### Physics/
Physics simulation interfaces:
- `PhysicsEngine.hpp` - Physics engine interface

### Audio/
Audio processing interfaces:
- `AudioAnalyzer.hpp` - Audio analysis and FFT interface

### Input/
Input handling interfaces:
- `InputManager.hpp` - Unified input event system

### Utils/
Utility functions and helpers:
- `Math.hpp` - Mathematical constants and functions
- `PerformanceProfiler.hpp` - Performance profiling tools

### Modes/
Visual mode implementations:
- `ParticleGalaxyMode.hpp` - Gravitational N-body simulation

## Header Guidelines

- Use `#pragma once` for include guards
- Include only necessary headers
- Use forward declarations when possible
- Follow the project's namespace convention
- Document public interfaces with comments