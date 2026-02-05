# kv

Real-time audio-reactive 3D cellular automata visualization engine built with [Cinder](https://libcinder.org/) and OpenGL.

kv generates complex geometric patterns driven by three-dimensional cellular automata rules, with real-time audio analysis, post-processing shader effects, and OSC network control.

## Features

**Cellular Automata Engine**
- 3D grid-based simulation with configurable dimensions
- Multiple rule types: continuous fading, Game of Life variants, genetic rules
- Self-Organizing Maps (SOM) for pattern learning and clustering

**Rendering**
- 24 cellular visualization patterns and 4 boid/flocking patterns
- GPU-instanced rendering (cubes, spheres, cylinders, lines, planes, points, polygons)
- Environment mapping with cubemaps
- Spherical coordinate projections

**Post-Processing Effects**
- Gaussian blur
- Radial distortion
- Motion blur
- Glitch / chromatic aberration
- Bloom / glow
- Mosaic pixelation (square, hexagon, triangle)
- Frame accumulation trails

**Audio Reactivity**
- Real-time microphone input
- FFT frequency band analysis (low / mid / high)
- MFCC spectral features (13 coefficients)
- Waveform visualization with 3D ribbon trails
- Audio-modulated pattern parameters

**Control**
- OSC protocol for remote parameter control (in: 7000, out: 57121)
- Multi-display / projector output
- Command-line configuration

## Requirements

- macOS
- [Cinder](https://libcinder.org/) framework
- Xcode

## Building

Open `xcode/kv.xcodeproj` in Xcode and build. The project targets OpenGL 3.2+ with GLSL 150.

## Usage

```
kv [-screenx <width>] [-screeny <height>] [-fps <rate>] [-wmode <0|1>] [-full <0|1>]
```

| Flag | Description | Default |
|------|-------------|---------|
| `-screenx` | Window width | 1024 |
| `-screeny` | Window height | 768 |
| `-fps` | Target frame rate | 30 |
| `-wmode` | Display mode (0 = main, 1 = secondary/projector) | 1 |
| `-full` | Fullscreen | 1 |

### Keyboard

| Key | Action |
|-----|--------|
| `F` | Toggle fullscreen |

## Architecture

```
src/
  kvApp.cpp          Main application entry point
  world.h/cpp        3D cellular automata engine
  rule.h/cpp         CA rule definitions (faders, life, continuous)
  ogl.h/cpp          OpenGL rendering engine
  pattern.h/cpp      24 visualization patterns
  boidpattern.h/cpp  4 boid/flocking patterns
  boids.h/cpp        Flocking simulation
  osc.h/cpp          OSC network messaging
  codepanel.h/cpp    Live code editing panel
  util.h/cpp         Math utilities

xcode/Assets/        GLSL shaders, cubemaps, textures
```
