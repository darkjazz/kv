# kv

Real-time audio-reactive 3D cellular automata visualization engine built with [Cinder](https://libcinder.org/) and OpenGL.

kv generates complex geometric patterns driven by three-dimensional cellular automata rules, with real-time audio analysis, post-processing shader effects, and OSC network control.

## Features

**Cellular Automata Engine**
- 3D grid-based simulation with configurable dimensions
- Multiple rule types: continuous fading, Game of Life variants, genetic rules
- Self-Organizing Maps (SOM) for pattern learning and clustering
- Morphing world shapes: the cubic grid can be drawn warped into a sphere, torus, twist, blob, shell and more, morphing and tumbling over time while the simulation stays cubic

**Rendering**
- 24 cellular visualization patterns and 7 boid/flocking patterns
- GPU-instanced rendering (cubes, spheres, cylinders, lines, planes, points, polygons)
- Environment mapping with cubemaps
- Spherical coordinate projections

**Water**
- GPU water simulation with drops, auto-drops and audio-driven cymatics
- Standalone pool scene with refractive surface and caustics on the pool walls

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

The prebuilt `libcinder.a` is x86_64 only, so command-line builds must target that architecture:

```
cd xcode
xcodebuild -project kv.xcodeproj -configuration Release ARCHS=x86_64 ONLY_ACTIVE_ARCH=NO build
```

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

## World Shapes

The cellular automaton always runs on a cubic grid. Only its visual representation is reshaped: while cells are drawn, every instance position (cubes, spheres, lines, planes, triangles…) is mapped from the cube into the current shape, so all patterns follow it. Boids, water and overlays are not warped.

| # | Shape | Description |
|---|-------|-------------|
| 0 | cube | unwarped grid (default) |
| 1 | sphere | cube mapped into a ball |
| 2 | torus | x and z wrap around ring and tube, matching the CA's wrap-around edges |
| 3 | octahedron | diamond |
| 4 | cylinder | x–z square mapped to a disc |
| 5 | twist | column twisted around y, twist amount slowly changes |
| 6 | blob | rippling noise sphere |
| 7 | shell | nested cube layers become nested spherical shells around a hollow core |
| 8 | wave | grid sways in moving sine waves |

| OSC address | Arguments | Description |
|-------------|-----------|-------------|
| `/lambda/world/shape` | `i` shape A, `i` shape B, `f` morph 0–1 | set shapes and blend A → B |
| `/lambda/world/shape/evolve` | `f` morph step/frame, `f` rotation rad/frame, `i` random 0/1 | auto-morph through shapes and tumble (0, 0 = frozen) |
| `/lambda/world/shape/scale` | `f` scale | scale the shape within the grid |

From SuperCollider (`CinderApp`):

```supercollider
c.setShape(\sphere, \torus, 0.5);  // halfway between sphere and torus
c.morphShape(\shell, 20);          // SC-timed morph over 20 seconds
c.evolveShape(0.005, 0.003, true); // app-side evolution, random order
c.pulseShapeScale(0.5, 1.0, 8);    // scale swells and shrinks, 8 s period
c.resetShape;                      // back to the plain cube
```

## Water

Controlled under `/lambda/liquid/*`: `show`, `pool`, `camera`, `drop`, `autoDrop`, `cymatics`, `corners`, `jitter`, `gain`, `radius`, `damping`, `speed`, `caustic`, `surface`, `color`, `wallcolor`, `light`.

## Architecture

```
src/
  kvApp.cpp          Main application entry point
  world.h/cpp        3D cellular automata engine + world shape morphing
  rule.h/cpp         CA rule definitions (faders, life, continuous)
  ogl.h/cpp          OpenGL rendering engine
  pattern.h/cpp      24 visualization patterns
  boidpattern.h/cpp  7 boid/flocking patterns
  boids.h/cpp        Flocking simulation
  watersim.h/cpp     GPU water height-field simulation
  waterscene.h/cpp   Water surface, pool and caustics rendering
  osc.h/cpp          OSC network messaging
  codepanel.h/cpp    Live code editing panel
  util.h/cpp         Math utilities

xcode/Assets/        GLSL shaders, cubemaps, textures
```
