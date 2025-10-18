/*
 *  pattern.h
 *  lambda
 *
 *  Modern pattern system for Lambda cellular automata visualization
 *
 *	This file is part of lambda.
 *
 *	lambda is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.

 *	lambda is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.

 *	You should have received a copy of the GNU General Public License
 *	along with lambda.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef PATTERN_H
#define PATTERN_H

#include "cinder/app/App.h"
#include "cinder/gl/gl.h"
#include "cinder/Color.h"
#include "cinder/gl/Texture.h"
#include "world.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class GraphicsRenderer;

// Render modes for different geometry types
enum class RenderMode {
    CUBES,
    SPHERES,
    CYLINDERS,
    LINES,
    PLANES,
    POINTS,
    SPHERICAL_QUAD,  // Quad mapped to spherical coordinates
    POLYGON,         // Smooth polygon with per-vertex colors
    CUSTOM
};

// Blend modes for alpha compositing
enum class BlendMode {
    NORMAL,
    ADDITIVE,
    MULTIPLY,
    SCREEN
};

// Comprehensive rendering configuration
struct RenderConfig {
    // Geometry
    RenderMode mode = RenderMode::CUBES;
    vec3 position = vec3(0.0f);
    vec3 scale = vec3(1.0f);
    vec3 offset = vec3(0.0f);
    quat rotation = quat();
    float uniformScale = 1.0f;

    // Line-specific (for RenderMode::LINES)
    vec3 lineStart = vec3(0.0f);
    vec3 lineEnd = vec3(0.0f);
    float lineWidth = 1.0f;
    bool lineSmooth = true;

    // Spherical quad-specific (for RenderMode::SPHERICAL_QUAD)
    float sphericalTheta = 0.0f;  // Azimuthal angle
    float sphericalPhi = 0.0f;    // Polar angle
    float sphericalRho = 1.0f;    // Radius
    ivec2 sphericalGridPos = ivec2(0);  // Grid position for neighbor calc
    ColorA sphericalCornerColors[4];    // Per-vertex colors for the quad
    float sphericalCellState = 1.0f;    // Cell state for size variation

    // Polygon-specific (for RenderMode::POLYGON)
    vec3 polygonVertices[4];   // 4 vertices in world space
    ColorA polygonColors[4];   // Per-vertex colors

    // Color & Material
    ColorA color = ColorA(1.0f, 1.0f, 1.0f, 1.0f);
    ColorA emissiveColor = ColorA(0.0f, 0.0f, 0.0f, 0.0f);
    float metallic = 0.0f;
    float roughness = 0.5f;
    BlendMode blendMode = BlendMode::SCREEN;

    // Texture Mapping
    gl::TextureRef texture = nullptr;
    gl::TextureRef videoTexture = nullptr;
    vec2 texOffset = vec2(0.0f);
    vec2 texScale = vec2(1.0f);
    float texRotation = 0.0f;

    // Spatial Relationships
    float distanceFromCenter = 0.0f;
    vec3 centerPoint = vec3(0.0f);
    float distanceFromCamera = 0.0f;

    // Animation
    float animationPhase = 0.0f;
    float animationSpeed = 1.0f;
    vec3 velocity = vec3(0.0f);
    float pulseFrequency = 1.0f;
    float pulseAmount = 0.0f;

    // Effects
    float glow = 0.0f;
    float wireframe = 0.0f;
    float dissolve = 0.0f;
    float noise = 0.0f;
    vec3 noiseOffset = vec3(0.0f);

    // Audio Reactive
    float audioAmplitude = 0.0f;
    float audioBand = 0.0f;
    bool audioReactive = false;

    // Performance
    float cullDistance = 1000.0f;
    bool frustumCull = false;
    int lodLevel = 0;

    // Custom extensibility
    std::map<string, float> customFloats;
    std::map<string, vec3> customVec3s;

    RenderConfig() = default;
};

// Base class for all patterns
class Pattern {
public:
    Pattern(int id) : mId(id), mActive(false), mAlpha(1.0f),
                      mColorMap(0), mAlphaMap(0), mColor(1.0f, 1.0f, 1.0f), mAudioIntensity(0.0f) {}

    virtual ~Pattern() = default;

    // Core pattern interface
    virtual string getName() const = 0;
    virtual string getDescription() const { return ""; }

    // Check if this cell should be rendered by this pattern
    virtual bool isActive(int x, int y, int z, const Cell* cell, const World* world) const = 0;

    // Get render configuration for this cell
    virtual RenderConfig getRenderConfig(int x, int y, int z, const Cell* cell, const World* world,
                                        GraphicsRenderer* renderer) const = 0;

    // Optional: Update per-frame animations
    virtual void update(float time, float dt) {}

    // Audio reactivity control
    void setAudioReactivity(float intensity) { mAudioIntensity = intensity; }
    float getAudioReactivity() const { return mAudioIntensity; }

    // Pattern state
    void setActive(bool active) { mActive = active; }
    bool getActive() const { return mActive; }

    void setAlpha(float alpha) { mAlpha = alpha; }
    float getAlpha() const { return mAlpha; }

    void setColor(const Color& color) { mColor = color; }
    Color getColor() const { return mColor; }

    void setColorMap(int colormap) { mColorMap = colormap; }
    int getColorMap() const { return mColorMap; }

    void setAlphaMap(int alphamap) { mAlphaMap = alphamap; }
    int getAlphaMap() const { return mAlphaMap; }

    int getId() const { return mId; }

protected:
    int mId;
    bool mActive;
    float mAlpha;
    int mColorMap;
    int mAlphaMap;
    Color mColor;
    float mTime = 0.0f;
    float mAudioIntensity;  // 0.0 = no audio reactivity, 1.0 = full reactivity

    // Helper: Compute cell state value
    float getCellState(const Cell* cell, const World* world) const {
        // Note: World methods need to be const-correct
        // For now, cast away const (not ideal but matches existing codebase)
        World* w = const_cast<World*>(world);
        if (w->ruleType() == CONT) {
            return cell->phase;
        } else {
            return (cell->phase != 0) ? (1.0f / cell->phase) : 0.0f;
        }
    }

    // Helper: Apply color/alpha mapping
    ColorA applyMapping(float state) const {
        float red = mColor.r * abs(mColorMap - state);
        float green = mColor.g * abs(mColorMap - state);
        float blue = mColor.b * abs(mColorMap - state);
        float alpha = mAlpha * abs(mAlphaMap - state);
        return ColorA(red, green, blue, alpha);
    }

    // Helper: Apply color mapping but use full alpha (no state modulation)
    ColorA applyMappingWithFullAlpha(float state) const {
        float red = mColor.r * abs(mColorMap - state);
        float green = mColor.g * abs(mColorMap - state);
        float blue = mColor.b * abs(mColorMap - state);
        return ColorA(red, green, blue, mAlpha);  // Full alpha, no modulation
    }

    // Helper: Distance from world center
    float distanceFromCenter(int x, int y, int z, const World* world) const {
        World* w = const_cast<World*>(world);
        float cx = w->sizeX() * 0.5f;
        float cy = w->sizeY() * 0.5f;
        float cz = w->sizeZ() * 0.5f;
        float dx = x - cx;
        float dy = y - cy;
        float dz = z - cz;
        return sqrt(dx*dx + dy*dy + dz*dz);
    }

    // Helper: Normalized distance from world center (0-1)
    float normalizedDistance(int x, int y, int z, const World* world) const {
        World* w = const_cast<World*>(world);
        float cx = w->sizeX() * 0.5f;
        float cy = w->sizeY() * 0.5f;
        float cz = w->sizeZ() * 0.5f;
        float maxDist = sqrt(cx*cx + cy*cy + cz*cz);
        return distanceFromCenter(x, y, z, world) / maxDist;
    }

    // Helper: Get audio amplitude with intensity scaling
    float getAudioAmplitude(const GraphicsRenderer* renderer) const;

    // Helper: Get audio band (0=low, 1=mid, 2=high) with intensity scaling
    float getAudioBand(const GraphicsRenderer* renderer, int band) const;
};

// Pattern factory for creating patterns by ID
class PatternFactory {
public:
    static std::unique_ptr<Pattern> create(int id);
};

#endif
