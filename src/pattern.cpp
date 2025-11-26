/*
 *  pattern.cpp
 *  lambda
 *
 *  Pattern implementations for Lambda cellular automata visualization
 *
 *	This file is part of lambda.
 */

#include "pattern.h"
#include "ogl.h"

// ============================================================================
// Pattern Base Class - Audio Helper Implementations
// ============================================================================

float Pattern::getAudioAmplitude(const GraphicsRenderer* renderer) const {
    if (!renderer || mAudioIntensity <= 0.0f) return 0.0f;
    return renderer->mAudioAmplitude * mAudioIntensity;
}

float Pattern::getAudioBand(const GraphicsRenderer* renderer, int band) const {
    if (!renderer || mAudioIntensity <= 0.0f) return 0.0f;

    switch (band) {
        case 0: return renderer->mAudioLowBand * mAudioIntensity;   // Low frequencies
        case 1: return renderer->mAudioMidBand * mAudioIntensity;   // Mid frequencies
        case 2: return renderer->mAudioHighBand * mAudioIntensity;  // High frequencies
        default: return 0.0f;
    }
}

// ============================================================================
// Pattern00: Nested boundary rectangles
// ============================================================================
class Pattern00 : public Pattern {
private:
    float mBrightness = 2.5f;  // Overall brightness multiplier

public:
    Pattern00() : Pattern(0) {}

    string getName() const override { return "Nested Boundary Rectangles"; }
    string getDescription() const override { return "Draws nested rectangles on boundaries with inverse alpha scaling"; }

    bool isActive(int x, int y, int z, const Cell* cell, const World* world) const override {
        if (!mActive) return false;
        if (cell->phase <= 0.0f) return false;

        World* w = const_cast<World*>(world);

        // Only render on boundaries (same as Pattern10)
        return (x == 0 || y == 0 || z == 0 ||
                (x == w->sizeX() - 1 && y < w->sizeY() - 1 && z < w->sizeZ() - 1) ||
                (y == w->sizeY() - 1 && z < w->sizeZ() - 1 && x < w->sizeX() - 1) ||
                (z == w->sizeZ() - 1 && x < w->sizeX() - 1 && y < w->sizeY() - 1));
    }

    RenderConfig getRenderConfig(int x, int y, int z, const Cell* cell, const World* world,
                                 GraphicsRenderer* renderer) const override {
        RenderConfig config;
        config.mode = RenderMode::PLANES;

        World* w = const_cast<World*>(world);
        float cstate = getCellState(cell, world);
        float unmap = 1.0f - cstate;

        // Determine which boundaries we're on
        bool onXNeg = (x == 0);
        bool onYNeg = (y == 0);
        bool onZNeg = (z == 0);
        bool onXPos = (x == w->sizeX() - 1 && y < w->sizeY() - 1 && z < w->sizeZ() - 1);
        bool onYPos = (y == w->sizeY() - 1 && z < w->sizeZ() - 1 && x < w->sizeX() - 1);
        bool onZPos = (z == w->sizeZ() - 1 && x < w->sizeX() - 1 && y < w->sizeY() - 1);

        // Store rendering info for custom nested rectangles
        // Will draw 4 rects at sizes: 0.25, 0.5, 0.75, 1.0
        // with alpha = 1/size (so larger rects have smaller alpha)
        config.customFloats["unmap"] = unmap;
        config.customFloats["drawNested"] = 1.0f;  // Flag to draw nested rects

        config.customFloats["onXNeg"] = onXNeg ? 1.0f : 0.0f;
        config.customFloats["onYNeg"] = onYNeg ? 1.0f : 0.0f;
        config.customFloats["onZNeg"] = onZNeg ? 1.0f : 0.0f;
        config.customFloats["onXPos"] = onXPos ? 1.0f : 0.0f;
        config.customFloats["onYPos"] = onYPos ? 1.0f : 0.0f;
        config.customFloats["onZPos"] = onZPos ? 1.0f : 0.0f;

        // Base color with brightness affecting alpha only (to keep colors pure)
        ColorA baseColor = applyMapping(unmap);

        // Apply overall brightness and audio reactivity to ALPHA only
        float audioMod = 1.0f + (getAudioAmplitude(renderer) * 0.2f);  // 0-20% boost
        float totalBrightness = mBrightness * audioMod;

        baseColor.a = std::min(baseColor.a * totalBrightness, 1.0f);
        config.color = baseColor;

        return config;
    }

    void update(float time, float dt) override {
        mTime = time;
    }
};

// ============================================================================
// Pattern01: Filled cube planes on boundaries
// ============================================================================
class Pattern01 : public Pattern {
public:
    Pattern01() : Pattern(1) {}

    string getName() const override { return "Boundary Planes"; }
    string getDescription() const override { return "Draws filled planes on world boundaries"; }

    bool isActive(int x, int y, int z, const Cell* cell, const World* world) const override {
        if (!mActive) return false;

        World* w = const_cast<World*>(world);
        float cstate = getCellState(cell, world);
        if (w->ruleType() != CONT && cstate == 0.0f) return false;

        // Only draw on boundaries
        return (x == 0 || y == 0 || z == 0 ||
                (x == w->sizeX() - 1 && y < w->sizeY() - 1 && z < w->sizeZ() - 1) ||
                (y == w->sizeY() - 1 && z < w->sizeZ() - 1 && x < w->sizeX() - 1) ||
                (z == w->sizeZ() - 1 && x < w->sizeX() - 1 && y < w->sizeY() - 1));
    }

    RenderConfig getRenderConfig(int x, int y, int z, const Cell* cell, const World* world,
                                 GraphicsRenderer* renderer) const override {
        RenderConfig config;
        config.mode = RenderMode::CUBES;

        World* w = const_cast<World*>(world);
        float cstate = getCellState(cell, world);

        // Determine which plane and set appropriate scale
        // (This mimics the original pattern01 logic)
        vec3 scale(1.0f);
        if (x == 0 || x == w->sizeX() - 1) {
            scale.x = 0.1f; // Thin in X
        }
        if (y == 0 || y == w->sizeY() - 1) {
            scale.y = 0.1f; // Thin in Y
        }
        if (z == 0 || z == w->sizeZ() - 1) {
            scale.z = 0.1f; // Thin in Z
        }

        config.scale = scale;

        // Subtle audio reactivity: modulate scale with overall amplitude
        float audioMod = 1.0f + (getAudioAmplitude(renderer) * 0.2f);  // 0-20% scale boost
        config.uniformScale = mapf(cstate, 0.5f, 2.0f) * audioMod;
        config.color = applyMapping(cstate);

        return config;
    }
};

// ============================================================================
// Pattern02: Spheres throughout the world
// ============================================================================
class Pattern02 : public Pattern {
public:
    Pattern02() : Pattern(2) {}

    string getName() const override { return "World Spheres"; }
    string getDescription() const override { return "Draws spheres at all active cells"; }

    bool isActive(int x, int y, int z, const Cell* cell, const World* world) const override {
        if (!mActive) return false;

        World* w = const_cast<World*>(world);
        float cstate = getCellState(cell, world);
        return (w->ruleType() == CONT || cstate != 0.0f);
    }

    RenderConfig getRenderConfig(int x, int y, int z, const Cell* cell, const World* world,
                                 GraphicsRenderer* renderer) const override {
        RenderConfig config;
        config.mode = RenderMode::SPHERES;

        World* w = const_cast<World*>(world);
        float cstate = getCellState(cell, world);

        // Position-based radial scaling from center
        float cx = w->sizeX() * 0.5f;
        float cy = w->sizeY() * 0.5f;
        float cz = w->sizeZ() * 0.5f;
        float dx = (x - cx) / cx;
        float dy = (y - cy) / cy;
        float dz = (z - cz) / cz;
        float distFromCenter = sqrt(dx*dx + dy*dy + dz*dz);

        // Subtle audio reactivity: mid frequencies affect sphere size
        float audioMod = 1.0f + (getAudioBand(renderer, 1) * 0.3f);  // 0-30% boost from mids
        config.uniformScale = mapf(cstate * (distFromCenter * 0.5f), 0.1f, 2.0f) * audioMod;
        config.color = applyMapping(cstate);

        return config;
    }
};

// ============================================================================
// Pattern03: Spherical neighbor connections with dynamic motion (pattern10)
// ============================================================================
class Pattern03 : public Pattern {
private:
    float mMaxPhase = 28.0f;

public:
    Pattern03() : Pattern(3) {}

    string getName() const override { return "Spherical Neighbor Lines"; }
    string getDescription() const override { return "Points and lines to neighbor in spherical coords with rotating theta/phi"; }

    bool isActive(int x, int y, int z, const Cell* cell, const World* world) const override {
        if (!mActive) return false;

        // Original: state > 0.0 && z % 3 == 0
        return (cell->phase > 0.0f && z % 3 == 0);
    }

    RenderConfig getRenderConfig(int x, int y, int z, const Cell* cell, const World* world,
                                 GraphicsRenderer* renderer) const override {
        RenderConfig config;
        config.mode = RenderMode::CUSTOM;  // Custom rendering for points + lines

        World* w = const_cast<World*>(world);
        float state = cell->phase;

        // Calculate unmap value
        float maxState = w->rule()->numStates() - 1;
        float unmap = 1.0f - (state / maxState);

        // Store data for custom rendering
        config.customFloats["state"] = state;
        config.customFloats["unmap"] = unmap;
        config.customFloats["x"] = static_cast<float>(x);
        config.customFloats["y"] = static_cast<float>(y);
        config.customFloats["z"] = static_cast<float>(z);

        // Single color based on unmap value (pattern uses abs(colormap - unmap))
        // Using simple color mapping to match pattern colormap behavior
        ColorA baseColor = applyMapping(unmap);

        // Subtle audio reactivity: mid frequencies modulate brightness
        float audioMod = 1.0f + (getAudioBand(renderer, 1) * 0.3f);  // 0-30% boost from mids
        baseColor.r = std::min(baseColor.r * audioMod, 1.0f);
        baseColor.g = std::min(baseColor.g * audioMod, 1.0f);
        baseColor.b = std::min(baseColor.b * audioMod, 1.0f);

        config.color = baseColor;

        return config;
    }

    void update(float time, float dt) override {
        mTime = time;
    }
};

// ============================================================================
// Pattern04: Texture-mapped cubes
// ============================================================================
class Pattern04 : public Pattern {
private:
    mutable gl::TextureRef mTexture;  // Mutable to allow lazy loading in const method

public:
    Pattern04() : Pattern(4) {}

    string getName() const override { return "Textured Cubes"; }
    string getDescription() const override { return "Draws texture-mapped cubes on world boundaries"; }

    bool isActive(int x, int y, int z, const Cell* cell, const World* world) const override {
        if (!mActive) return false;

        World* w = const_cast<World*>(world);
        float cstate = getCellState(cell, world);
        if (w->ruleType() != CONT && cstate == 0.0f) return false;

        // Only render on boundaries to reduce texture overhead
        return (x == 0 || y == 0 || z == 0 ||
                (x == w->sizeX() - 1 && y < w->sizeY() - 1 && z < w->sizeZ() - 1) ||
                (y == w->sizeY() - 1 && z < w->sizeZ() - 1 && x < w->sizeX() - 1) ||
                (z == w->sizeZ() - 1 && x < w->sizeX() - 1 && y < w->sizeY() - 1));
    }

    RenderConfig getRenderConfig(int x, int y, int z, const Cell* cell, const World* world,
                                 GraphicsRenderer* renderer) const override {
        RenderConfig config;
        config.mode = RenderMode::CUBES;

        World* w = const_cast<World*>(world);
        float cstate = getCellState(cell, world);

        // Lazy load texture
        if (!mTexture) {
            try {
                auto imgSource = loadImage(app::loadAsset("fu_00.png"));
                console() << "Pattern04: Loaded image source " << imgSource->getWidth() << "x" << imgSource->getHeight() << std::endl;
                console() << "  Has alpha: " << imgSource->hasAlpha() << std::endl;
                console() << "  Color model: " << (int)imgSource->getColorModel() << std::endl;
                console() << "  Data type: " << (int)imgSource->getDataType() << std::endl;

                // Convert to Surface to check actual pixel data
                Surface8u surf(imgSource);
                console() << "  Surface size: " << surf.getWidth() << "x" << surf.getHeight() << std::endl;

                // Sample a few pixels to see if image has data
                if (surf.getWidth() > 0 && surf.getHeight() > 0) {
                    auto pixel = surf.getPixel(ivec2(surf.getWidth()/2, surf.getHeight()/2));
                    console() << "  Center pixel: R=" << (int)pixel.r << " G=" << (int)pixel.g
                              << " B=" << (int)pixel.b << std::endl;
                }

                gl::Texture::Format fmt;
                fmt.setWrap(GL_REPEAT, GL_REPEAT);
                fmt.setMinFilter(GL_LINEAR);
                fmt.setMagFilter(GL_LINEAR);

                mTexture = gl::Texture::create(surf, fmt);
                console() << "Pattern04: Created texture from fu_00.png successfully!" << std::endl;
            }
            catch (const std::exception& e) {
                console() << "Pattern04: Failed to load texture: " << e.what() << std::endl;
                console() << "Pattern04: Rendering without texture" << std::endl;
            }
        }

        // Set texture (may be null if failed to load)
        config.texture = mTexture;

        // Animated texture offset for scrolling effect
        config.texOffset = vec2(mTime * 0.1f, mTime * 0.05f);
        config.texScale = vec2(1.0f);
        config.texRotation = mTime * 0.2f;

        // Uniform cube size with pulsing based on state and position
        float pulse = sin(mTime + x * 0.2f + y * 0.3f + z * 0.4f) * 0.5f + 0.5f;

        // Subtle audio reactivity: amplitude modulates scale
        float audioMod = 1.0f + (getAudioAmplitude(renderer) * 0.25f);  // 0-25% boost
        config.uniformScale = mapf(cstate * (0.7f + pulse * 0.3f), 0.5f, 2.0f) * audioMod;

        // Bright color to be visible even without texture
        config.color = applyMapping(cstate);

        return config;
    }

    void update(float time, float dt) override {
        mTime = time;
    }
};

// ============================================================================
// Pattern05: Environment-mapped reflective spheres
// ============================================================================
class Pattern05 : public Pattern {
public:
    Pattern05() : Pattern(5) {}

    string getName() const override { return "Reflective Spheres"; }
    string getDescription() const override { return "Environment-mapped spheres on boundaries with cubemap reflections"; }

    bool isActive(int x, int y, int z, const Cell* cell, const World* world) const override {
        if (!mActive) return false;

        World* w = const_cast<World*>(world);
        float cstate = getCellState(cell, world);
        if (w->ruleType() != CONT && cstate == 0.0f) return false;

        // Only render on boundaries to reduce cubemap overhead
        return (x == 0 || y == 0 || z == 0 ||
                (x == w->sizeX() - 1 && y < w->sizeY() - 1 && z < w->sizeZ() - 1) ||
                (y == w->sizeY() - 1 && z < w->sizeZ() - 1 && x < w->sizeX() - 1) ||
                (z == w->sizeZ() - 1 && x < w->sizeX() - 1 && y < w->sizeY() - 1));
    }

    RenderConfig getRenderConfig(int x, int y, int z, const Cell* cell, const World* world,
                                 GraphicsRenderer* renderer) const override {
        RenderConfig config;
        config.mode = RenderMode::SPHERES;

        World* w = const_cast<World*>(world);
        float cstate = getCellState(cell, world);

        // Position-based radial scaling from center
        float cx = w->sizeX() * 0.5f;
        float cy = w->sizeY() * 0.5f;
        float cz = w->sizeZ() * 0.5f;
        float dx = (x - cx) / cx;
        float dy = (y - cy) / cy;
        float dz = (z - cz) / cz;
        float distFromCenter = sqrt(dx*dx + dy*dy + dz*dz);

        // Pulsing based on time and position
        float pulse = sin(mTime * 2.0f + x * 0.3f + y * 0.2f + z * 0.4f) * 0.5f + 0.5f;

        // Subtle audio reactivity: low frequencies affect reflective sphere size
        float audioMod = 1.0f + (getAudioBand(renderer, 0) * 0.35f);  // 0-35% boost from bass
        config.uniformScale = mapf(cstate * (0.5f + pulse * 0.5f) * distFromCenter, 0.2f, 1.5f) * audioMod;

        // Color tinting for the reflection
        config.color = applyMapping(cstate);

        return config;
    }

    void update(float time, float dt) override {
        mTime = time;
    }
};

// ============================================================================
// Pattern06: Spherical coordinate mapped pattern (migrated from pattern29)
// ============================================================================
class Pattern06 : public Pattern {
public:
    Pattern06() : Pattern(6) {}

    string getName() const override { return "Spherical Surface"; }
    string getDescription() const override { return "Maps cells to spherical coordinates"; }

    bool isActive(int x, int y, int z, const Cell* cell, const World* world) const override {
        if (!mActive) return false;
        float cstate = getCellState(cell, world);
        // Only render if state > 0 and z is divisible by 4 (from original pattern29)
        return (cstate > 0.0f && z % 4 == 0);
    }

    RenderConfig getRenderConfig(int x, int y, int z, const Cell* cell, const World* world,
                                 GraphicsRenderer* renderer) const override {
        RenderConfig config;
        config.mode = RenderMode::SPHERES;

        World* w = const_cast<World*>(world);
        float cstate = getCellState(cell, world);

        // Map to spherical coordinates
        float theta = (2.0f * M_PI / w->sizeX()) * x;
        float phi = (2.0f * M_PI / w->sizeY()) * y;
        float rho = z * 0.5f;  // Radius based on z coordinate

        // Convert spherical to Cartesian offset
        config.offset.x = rho * cos(theta) * cos(phi);
        config.offset.y = rho * sin(theta) * cos(phi);
        config.offset.z = rho * sin(phi);

        // Size based on unmap value
        float unmap = 1.0f - cstate;

        // Subtle audio reactivity: mid frequencies modulate sphere size
        float audioMod = 1.0f + (getAudioBand(renderer, 1) * 0.3f);  // 0-30% boost from mids
        config.uniformScale = unmap * 0.3f * audioMod;

        // Color with variation based on position
        float colorVariation = sin(unmap * M_PI);
        config.color = applyMapping(colorVariation);

        return config;
    }

    void update(float time, float dt) override {
        mTime = time;
    }
};

// ============================================================================
// Pattern07: Spherical surface with quads (true migration of pattern29)
// ============================================================================
class Pattern07 : public Pattern {
private:
    float mRadiusScale = 2.5f;  // Scale factor for radius (larger = bigger spheres)
    float mPulseSpeed = 0.3f;   // Speed of radial pulsing
    float mRotationSpeed = 0.2f; // Speed of theta/phi rotation
    float mColorCycleSpeed = 0.5f;

public:
    Pattern07() : Pattern(7) {}

    string getName() const override { return "Spherical Quads"; }
    string getDescription() const override { return "Renders animated quads on spherical surface layers"; }

    bool isActive(int x, int y, int z, const Cell* cell, const World* world) const override {
        if (!mActive) return false;
        float cstate = getCellState(cell, world);
        // Only render if state > 0 and z is divisible by 4 (creates discrete layers)
        return (cstate > 0.0f && z % 4 == 0);
    }

    RenderConfig getRenderConfig(int x, int y, int z, const Cell* cell, const World* world,
                                 GraphicsRenderer* renderer) const override {
        RenderConfig config;
        config.mode = RenderMode::SPHERICAL_QUAD;

        World* w = const_cast<World*>(world);
        float cstate = getCellState(cell, world);
        float unmap = 1.0f - cstate;

        // Compute base spherical coordinates
        float theta = (2.0f * M_PI / w->sizeX()) * x;
        float phi = (2.0f * M_PI / w->sizeY()) * y;

        // Add time-based rotation to theta and phi for dynamic motion
        theta += mTime * mRotationSpeed;
        phi += sin(mTime * mRotationSpeed * 0.7f) * 0.3f;  // Wobble in phi

        config.sphericalTheta = theta;
        config.sphericalPhi = phi;

        // Scale radius to match Pattern00's world extent (same as Pattern09)
        float worldScale = (w->sizeX() + w->sizeY() + w->sizeZ()) / 3.0f;
        float fragSizeScale = worldScale * 0.5f;  // Half world scale to match Pattern09

        // Base radius from z coordinate
        float normalizedZ = z / (float)w->sizeZ();
        float baseRadius = z * fragSizeScale * 0.5f;

        // Add pulsing animation - each layer pulses at different rate
        float pulse = sin(mTime * mPulseSpeed + normalizedZ * M_PI * 2.0f) * 0.15f + 1.0f;

        // Subtle audio reactivity: overall amplitude modulates radius
        float audioMod = 1.0f + (getAudioAmplitude(renderer) * 0.2f);  // 0-20% boost
        config.sphericalRho = baseRadius * pulse * audioMod;

        config.sphericalGridPos = ivec2(x, y);

        // Pass cell state for quad size variation
        config.sphericalCellState = cstate;

        // Dynamic color cycling based on time, position, and state
        float colorPhase = mTime * mColorCycleSpeed + normalizedZ * M_PI;

        // Create rainbow-like colors that cycle through spectrum
        float hue1 = fmod(colorPhase + unmap, 1.0f);
        float hue2 = fmod(colorPhase + unmap + 0.25f, 1.0f);
        float hue3 = fmod(colorPhase + unmap + 0.5f, 1.0f);
        float hue4 = fmod(colorPhase + unmap + 0.75f, 1.0f);

        // Convert hues to RGB (simple HSV to RGB)
        auto hueToColor = [this](float h, float brightness) -> ColorA {
            float r = abs(h * 6.0f - 3.0f) - 1.0f;
            float g = 2.0f - abs(h * 6.0f - 2.0f);
            float b = 2.0f - abs(h * 6.0f - 4.0f);
            r = glm::clamp(r, 0.0f, 1.0f);
            g = glm::clamp(g, 0.0f, 1.0f);
            b = glm::clamp(b, 0.0f, 1.0f);
            return ColorA(r * brightness * mColor.r,
                         g * brightness * mColor.g,
                         b * brightness * mColor.b,
                         mAlpha * brightness);
        };

        // Each corner gets a different color phase for gradient effect
        config.sphericalCornerColors[0] = hueToColor(hue1, 1.0f);
        config.sphericalCornerColors[1] = hueToColor(hue2, 0.9f);
        config.sphericalCornerColors[2] = hueToColor(hue3, 0.8f);
        config.sphericalCornerColors[3] = hueToColor(hue4, 0.9f);

        return config;
    }

    void update(float time, float dt) override {
        mTime = time;
    }
};

// ============================================================================
// Pattern08: Center plane patterns with nested rectangles (migrated from pattern05)
// ============================================================================
class Pattern08 : public Pattern {
public:
    Pattern08() : Pattern(8) {}

    string getName() const override { return "Center Planes"; }
    string getDescription() const override { return "Draws nested rectangles on world center planes"; }

    bool isActive(int x, int y, int z, const Cell* cell, const World* world) const override {
        if (!mActive) return false;
        float cstate = getCellState(cell, world);
        return (cstate > 0.0f);
    }

    RenderConfig getRenderConfig(int x, int y, int z, const Cell* cell, const World* world,
                                 GraphicsRenderer* renderer) const override {
        RenderConfig config;
        config.mode = RenderMode::PLANES;

        World* w = const_cast<World*>(world);
        float cstate = getCellState(cell, world);
        float unmap = 1.0f - cstate;

        // Check if on center planes (with even/odd state consideration)
        int midX = w->sizeX() / 2;
        int midY = w->sizeY() / 2;
        int midZ = w->sizeZ() / 2;

        bool isEvenState = (static_cast<int>(cell->phase) % 2 == 0);

        bool onXPlane = (x == midX && isEvenState) || (x == midX - 1 && !isEvenState);
        bool onYPlane = (y == midY && isEvenState) || (y == midY - 1 && !isEvenState);
        bool onZPlane = (z == midZ && isEvenState) || (z == midZ - 1 && !isEvenState);

        // Only render if on one of the center planes
        if (!onXPlane && !onYPlane && !onZPlane) {
            config.mode = RenderMode::CUSTOM;  // Skip rendering
            return config;
        }

        // Store which plane we're on and rendering info in custom fields
        config.customFloats["unmap"] = unmap;
        config.customFloats["onXPlane"] = onXPlane ? 1.0f : 0.0f;
        config.customFloats["onYPlane"] = onYPlane ? 1.0f : 0.0f;
        config.customFloats["onZPlane"] = onZPlane ? 1.0f : 0.0f;
        config.customFloats["cellPhase"] = cell->phase;

        // Base color - no RGB multiplication to preserve color accuracy
        ColorA baseColor = applyMapping(unmap);

        // Boost alpha for brightness without shifting color
        baseColor.a = std::min(1.0f, baseColor.a * 1.8f);

        // Subtle audio reactivity: mid frequencies modulate alpha/brightness
        float audioMod = 1.0f + (getAudioBand(renderer, 1) * 0.3f);  // 0-30% boost from mids
        baseColor.a *= std::min(1.0f, audioMod);
        config.color = baseColor;

        return config;
    }

    void update(float time, float dt) override {
        mTime = time;
    }
};

// ============================================================================
// Pattern09: Smooth animated polygons connecting neighbors in spherical coords (pattern26)
// ============================================================================
class Pattern09 : public Pattern {
public:
    Pattern09() : Pattern(9) {}

    string getName() const override { return "Spherical Neighbor Polygons"; }
    string getDescription() const override { return "Animated smooth polygons connecting 4 neighbors in spherical coordinates"; }

    bool isActive(int x, int y, int z, const Cell* cell, const World* world) const override {
        if (!mActive) return false;
        // Only render on every 4th Z layer and if cell has state
        return (cell->phase > 0.0f && z > 0 && z % 4 == 0);
    }

    RenderConfig getRenderConfig(int x, int y, int z, const Cell* cell, const World* world,
                                 GraphicsRenderer* renderer) const override {
        RenderConfig config;
        config.mode = RenderMode::POLYGON;

        World* w = const_cast<World*>(world);
        Cell* mutableCell = const_cast<Cell*>(cell);

        // Get 4 neighbor cells (indices 3, 5, 17, 19 from pattern26)
        Cell* neighbors[4];
        neighbors[0] = w->rule()->getNeighbor(mutableCell, 3);
        neighbors[1] = w->rule()->getNeighbor(mutableCell, 5);
        neighbors[2] = w->rule()->getNeighbor(mutableCell, 17);
        neighbors[3] = w->rule()->getNeighbor(mutableCell, 19);

        // Animation phase based on counter (wraps around maxphase=28)
        float maxphase = 28.0f;
        float animPhase = (2.0f * M_PI / maxphase) * (fmod(renderer->counter, maxphase) / maxphase);

        // Scale factor to match Pattern00's world extent
        // Use half world scale for appropriate reach
        float worldScale = (w->sizeX() + w->sizeY() + w->sizeZ()) / 3.0f;
        float fragSizeScale = worldScale * 0.5f;  // Half world scale

        // Subtle audio reactivity: high frequencies modulate polygon radius
        float audioMod = 1.0f + (getAudioBand(renderer, 2) * 0.25f);  // 0-25% boost from highs

        // Calculate vertices and colors for each neighbor in spherical coordinates
        for (int i = 0; i < 4; i++) {
            Cell* neighbor = neighbors[i];
            float neighborState = neighbor->phase;

            // Spherical coordinates with animation
            float theta = ((2.0f * M_PI) / w->sizeX() * neighbor->x) + animPhase;
            float phi = ((2.0f * M_PI) / w->sizeY() * neighbor->y) + animPhase;

            // Scale rho to fill world extent like pattern00
            // Original: rho = z * (fragSizeX * 0.5) + (fragSizeX * state * 0.5)
            float rho = (neighbor->z * (fragSizeScale * 0.5f) + (fragSizeScale * neighborState * 0.5f)) * audioMod;

            // Convert spherical to Cartesian
            config.polygonVertices[i].x = rho * cos(theta) * cos(phi);
            config.polygonVertices[i].y = rho * sin(theta) * cos(phi);
            config.polygonVertices[i].z = rho * sin(phi);

            // Per-vertex color based on neighbor state, with full alpha control
            float neighborUnmap = 1.0f - neighborState;
            config.polygonColors[i] = applyMappingWithFullAlpha(neighborUnmap);
        }

        return config;
    }

    void update(float time, float dt) override {
        mTime = time;
    }
};

// ============================================================================
// Pattern10: Boundary rectangles with state-based fill/stroke (pattern09)
// ============================================================================
class Pattern10 : public Pattern {
public:
    Pattern10() : Pattern(10) {}

    string getName() const override { return "Boundary Rectangles"; }
    string getDescription() const override { return "Draws filled/wireframe rectangles on boundaries based on cell state"; }

    bool isActive(int x, int y, int z, const Cell* cell, const World* world) const override {
        if (!mActive) return false;
        if (cell->phase <= 0.0f) return false;

        World* w = const_cast<World*>(world);

        // Only render on boundaries
        return (x == 0 || y == 0 || z == 0 ||
                (x == w->sizeX() - 1 && y < w->sizeY() - 1 && z < w->sizeZ() - 1) ||
                (y == w->sizeY() - 1 && z < w->sizeZ() - 1 && x < w->sizeX() - 1) ||
                (z == w->sizeZ() - 1 && x < w->sizeX() - 1 && y < w->sizeY() - 1));
    }

    RenderConfig getRenderConfig(int x, int y, int z, const Cell* cell, const World* world,
                                 GraphicsRenderer* renderer) const override {
        RenderConfig config;
        config.mode = RenderMode::PLANES;

        World* w = const_cast<World*>(world);
        float cstate = getCellState(cell, world);
        float unmap = 1.0f - cstate;

        // Determine which boundaries we're on
        bool onXNeg = (x == 0);
        bool onYNeg = (y == 0);
        bool onZNeg = (z == 0);
        bool onXPos = (x == w->sizeX() - 1 && y < w->sizeY() - 1 && z < w->sizeZ() - 1);
        bool onYPos = (y == w->sizeY() - 1 && z < w->sizeZ() - 1 && x < w->sizeX() - 1);
        bool onZPos = (z == w->sizeZ() - 1 && x < w->sizeX() - 1 && y < w->sizeY() - 1);

        // Determine fill vs wireframe based on cell state history
        float mapState = cell->states[w->index()];
        bool shouldFill = (mapState > 0.0f && mapState < 2.0f);

        // Store rendering info in custom fields
        config.customFloats["unmap"] = unmap;
        config.customFloats["mapState"] = mapState;
        config.customFloats["shouldFill"] = shouldFill ? 1.0f : 0.0f;

        config.customFloats["onXNeg"] = onXNeg ? 1.0f : 0.0f;
        config.customFloats["onYNeg"] = onYNeg ? 1.0f : 0.0f;
        config.customFloats["onZNeg"] = onZNeg ? 1.0f : 0.0f;
        config.customFloats["onXPos"] = onXPos ? 1.0f : 0.0f;
        config.customFloats["onYPos"] = onYPos ? 1.0f : 0.0f;
        config.customFloats["onZPos"] = onZPos ? 1.0f : 0.0f;

        // Base color with subtle audio reactivity
        ColorA baseColor = applyMapping(unmap);

        // Subtle audio reactivity: amplitude modulates brightness
        float audioMod = 1.0f + (getAudioAmplitude(renderer) * 0.2f);  // 0-20% boost
        baseColor.r = std::min(baseColor.r * audioMod, 1.0f);
        baseColor.g = std::min(baseColor.g * audioMod, 1.0f);
        baseColor.b = std::min(baseColor.b * audioMod, 1.0f);
        config.color = baseColor;

        return config;
    }

    void update(float time, float dt) override {
        mTime = time;
    }
};

// ============================================================================
// Pattern11: Horizontal bars on specific Z slices (pattern11)
// ============================================================================
class Pattern11 : public Pattern {
public:
    Pattern11() : Pattern(11) {}

    string getName() const override { return "Horizontal Bars"; }
    string getDescription() const override { return "Draws horizontal bars on specific Z slices"; }

    bool isActive(int x, int y, int z, const Cell* cell, const World* world) const override {
        if (!mActive) return false;
        if (cell->phase <= 0.0f) return false;

        World* w = const_cast<World*>(world);
        int midZ = w->sizeZ() / 2;

        // Only render on specific Z slices: 0, midZ-3, midZ, midZ+3, sizeZ-1
        return (z == 0 || z == midZ - 3 || z == midZ || z == midZ + 3 || z == w->sizeZ() - 1);
    }

    RenderConfig getRenderConfig(int x, int y, int z, const Cell* cell, const World* world,
                                 GraphicsRenderer* renderer) const override {
        RenderConfig config;
        config.mode = RenderMode::CUBES;

        World* w = const_cast<World*>(world);
        float cstate = getCellState(cell, world);

        // Calculate mapState from cell state history
        float maxState = w->rule()->numStates() - 1;
        float mapState = (maxState - cell->states[w->index()]) * (1.0f / maxState);

        // Color with subtle audio reactivity
        ColorA baseColor = applyMapping(mapState);

        // Subtle audio reactivity: mid frequencies modulate brightness
        float audioMod = 1.0f + (getAudioBand(renderer, 1) * 0.25f);  // 0-25% boost from mids
        baseColor.r = std::min(baseColor.r * audioMod, 1.0f);
        baseColor.g = std::min(baseColor.g * audioMod, 1.0f);
        baseColor.b = std::min(baseColor.b * audioMod, 1.0f);
        config.color = baseColor;

        // Horizontal bar dimensions
        // Original: xW = fragSizeX * 4.0 + (mapState * fragSizeX)
        // yH = zD = fragSizeX * 0.25
        float unmap = 1.0f - cstate;
        config.scale = vec3(
            4.0f + mapState,  // X - long horizontal bar
            0.25f,            // Y - thin
            0.25f             // Z - thin
        );

        // Y position offset (original: yB = y * (fragSizeX * 0.5) + (fragSizeX * 0.25))
        // This compresses Y and offsets it
        config.customFloats["yCompress"] = 0.5f;
        config.customFloats["yOffset"] = 0.25f;

        return config;
    }

    void update(float time, float dt) override {
        mTime = time;
    }
};

// ============================================================================
// Pattern12: Sphere + filled cube + wireframe cube composite (pattern12)
// ============================================================================
class Pattern12 : public Pattern {
public:
    Pattern12() : Pattern(12) {}

    string getName() const override { return "Composite Sphere+Cube"; }
    string getDescription() const override { return "Draws sphere, filled cube, and wireframe cube with state-based sizing"; }

    bool isActive(int x, int y, int z, const Cell* cell, const World* world) const override {
        if (!mActive) return false;

        World* w = const_cast<World*>(world);
        float cellState = cell->states[w->index()];

        // Only render if state is between 0 and 4 (exclusive of 0, inclusive of 4)
        return (cellState > 0.0f && cellState <= 4.0f);
    }

    RenderConfig getRenderConfig(int x, int y, int z, const Cell* cell, const World* world,
                                 GraphicsRenderer* renderer) const override {
        RenderConfig config;
        config.mode = RenderMode::CUSTOM;  // Will use custom rendering in drawFragment

        World* w = const_cast<World*>(world);
        float cellState = cell->states[w->index()];

        // Calculate mapState with fixed maxState of 4
        float maxState = 4.0f;
        float mapState = (maxState - cellState) * (1.0f / maxState);

        // Store data for custom rendering
        config.customFloats["mapState"] = mapState;

        // Base color
        ColorA baseColor = applyMapping(mapState);

        // Subtle audio reactivity: low frequencies modulate brightness
        float audioMod = 1.0f + (getAudioBand(renderer, 0) * 0.25f);  // 0-25% boost from bass
        baseColor.r = std::min(baseColor.r * audioMod, 1.0f);
        baseColor.g = std::min(baseColor.g * audioMod, 1.0f);
        baseColor.b = std::min(baseColor.b * audioMod, 1.0f);

        config.color = baseColor;

        return config;
    }

    void update(float time, float dt) override {
        mTime = time;
    }
};

// ============================================================================
// Pattern13: Network visualization with cube-mapped spheres and cylinder connections (pattern13)
// ============================================================================
class Pattern13 : public Pattern {
public:
    Pattern13() : Pattern(13) {}

    string getName() const override { return "Network Spheres"; }
    string getDescription() const override { return "Cube-mapped spheres with cylinder connections on 4-grid pattern"; }

    bool isActive(int x, int y, int z, const Cell* cell, const World* world) const override {
        if (!mActive) return false;

        World* w = const_cast<World*>(world);

        // Filter: inside boundaries, state > 0, and on 4-grid
        if (x <= 0 || y <= 0 || z <= 0 ||
            x >= w->sizeX() - 1 || y >= w->sizeY() - 1 || z >= w->sizeZ() - 1) {
            return false;
        }

        if (cell->phase <= 0.0f) return false;

        // Only render on every 4th coordinate (creates sparser network)
        return (x % 4 == 0 || y % 4 == 0 || z % 4 == 0);
    }

    RenderConfig getRenderConfig(int x, int y, int z, const Cell* cell, const World* world,
                                 GraphicsRenderer* renderer) const override {
        RenderConfig config;
        config.mode = RenderMode::CUSTOM;  // Custom rendering for sphere + cylinders

        World* w = const_cast<World*>(world);
        float cellState = cell->states[w->index()];

        // Calculate mapState
        float maxState = w->rule()->numStates() - 1;
        float mapState = (maxState - cellState) * (1.0f / maxState);

        // Circular motion offset (original pattern has sin/cos motion)
        float angle = (1.0f - mapState) * 2.0f * M_PI;
        vec3 circularOffset(
            sin(angle),  // X offset
            cos(angle),  // Y offset
            sin(angle)   // Z offset
        );

        config.customFloats["mapState"] = mapState;
        config.customFloats["offsetX"] = circularOffset.x;
        config.customFloats["offsetY"] = circularOffset.y;
        config.customFloats["offsetZ"] = circularOffset.z;
        config.customFloats["cellState"] = cellState;

        // Base color
        ColorA baseColor = applyMapping(mapState);

        // Subtle audio reactivity: high frequencies modulate brightness
        float audioMod = 1.0f + (getAudioBand(renderer, 2) * 0.25f);  // 0-25% boost from highs
        baseColor.r = std::min(baseColor.r * audioMod, 1.0f);
        baseColor.g = std::min(baseColor.g * audioMod, 1.0f);
        baseColor.b = std::min(baseColor.b * audioMod, 1.0f);

        config.color = baseColor;

        return config;
    }

    void update(float time, float dt) override {
        mTime = time;
    }
};

// ============================================================================
// Pattern14: Textured cubes with circular motion (pattern19)
// ============================================================================
class Pattern14 : public Pattern {
private:
    mutable gl::TextureRef mTexture;  // Mutable to allow lazy loading in const method

public:
    Pattern14() : Pattern(14) {}

    string getName() const override { return "Circular Motion Cubes"; }
    string getDescription() const override { return "Textured cubes on boundaries with circular motion using 04.png"; }

    bool isActive(int x, int y, int z, const Cell* cell, const World* world) const override {
        if (!mActive) return false;
        if (cell->phase <= 0.0f) return false;

        World* w = const_cast<World*>(world);

        // Only render on boundaries to reduce texture overhead
        return (x == 0 || y == 0 || z == 0 ||
                (x == w->sizeX() - 1 && y < w->sizeY() - 1 && z < w->sizeZ() - 1) ||
                (y == w->sizeY() - 1 && z < w->sizeZ() - 1 && x < w->sizeX() - 1) ||
                (z == w->sizeZ() - 1 && x < w->sizeX() - 1 && y < w->sizeY() - 1));
    }

    RenderConfig getRenderConfig(int x, int y, int z, const Cell* cell, const World* world,
                                 GraphicsRenderer* renderer) const override {
        RenderConfig config;
        config.mode = RenderMode::CUBES;

        World* w = const_cast<World*>(world);
        float cellState = cell->states[w->index()];

        // Lazy load texture
        if (!mTexture) {
            try {
                auto imgSource = loadImage(app::loadAsset("04.png"));
                gl::Texture::Format fmt;
                fmt.setWrap(GL_REPEAT, GL_REPEAT);
                fmt.setMinFilter(GL_LINEAR);
                fmt.setMagFilter(GL_LINEAR);
                mTexture = gl::Texture::create(imgSource, fmt);
                console() << "Pattern14: Loaded texture from 04.png successfully!" << std::endl;
            }
            catch (const std::exception& e) {
                console() << "Pattern14: Failed to load texture: " << e.what() << std::endl;
            }
        }

        config.texture = mTexture;

        // Calculate mapState
        float maxState = w->rule()->numStates() - 1;
        float mapState = (maxState - cellState) * (1.0f / maxState);

        // Circular motion offset (same as Pattern13)
        float angle = (1.0f - mapState) * 2.0f * M_PI;
        config.offset = vec3(
            sin(angle),  // X offset
            cos(angle),  // Y offset
            sin(angle)   // Z offset
        );

        // Cube size - state == 1.0 uses fixed size, otherwise uses mapState
        vec3 cubeScale;
        if (cellState == 1.0f) {
            cubeScale = vec3(1.5f, 1.5f, 1.5f);
        } else {
            cubeScale = vec3(mapState * 1.5f, mapState * 1.5f, mapState * 1.5f);
        }
        config.scale = cubeScale;

        // Color
        ColorA baseColor = applyMapping(mapState);

        // Subtle audio reactivity: mid frequencies modulate brightness
        float audioMod = 1.0f + (getAudioBand(renderer, 1) * 0.3f);  // 0-30% boost from mids
        baseColor.r = std::min(baseColor.r * audioMod, 1.0f);
        baseColor.g = std::min(baseColor.g * audioMod, 1.0f);
        baseColor.b = std::min(baseColor.b * audioMod, 1.0f);

        config.color = baseColor;

        return config;
    }

    void update(float time, float dt) override {
        mTime = time;
    }
};

// ============================================================================
// Pattern15: Random points with BMU glow (pattern22)
// ============================================================================
class Pattern15 : public Pattern {
public:
    Pattern15() : Pattern(15) {}

    string getName() const override { return "Random Points"; }
    string getDescription() const override { return "Random points within cells with BMU glow effect"; }

    bool isActive(int x, int y, int z, const Cell* cell, const World* world) const override {
        if (!mActive) return false;

        World* w = const_cast<World*>(world);
        float cstate = getCellState(cell, world);
        return (w->ruleType() == CONT || cstate != 0.0f);
    }

    RenderConfig getRenderConfig(int x, int y, int z, const Cell* cell, const World* world,
                                 GraphicsRenderer* renderer) const override {
        RenderConfig config;
        config.mode = RenderMode::CUSTOM;  // Custom rendering for multiple random points

        World* w = const_cast<World*>(world);
        float cstate = getCellState(cell, world);

        // Store cell info for custom rendering
        config.customFloats["cstate"] = cstate;
        config.customFloats["x"] = static_cast<float>(x);
        config.customFloats["y"] = static_cast<float>(y);
        config.customFloats["z"] = static_cast<float>(z);

        // Check if this cell is the BMU for glow effect
        bool isBMU = (w->currentBMU() &&
                      w->currentBMU()->x == x &&
                      w->currentBMU()->y == y &&
                      w->currentBMU()->z == z);
        config.customFloats["isBMU"] = isBMU ? 1.0f : 0.0f;

        // Base color
        config.color = applyMapping(cstate);

        return config;
    }

    void update(float time, float dt) override {
        mTime = time;
    }
};

// ============================================================================
// Pattern16: Conditional plane rectangles based on neighbor states (pattern23)
// ============================================================================
class Pattern16 : public Pattern {
public:
    Pattern16() : Pattern(16) {}

    string getName() const override { return "Neighbor Planes"; }
    string getDescription() const override { return "Draws plane rectangles based on neighbor cell states"; }

    bool isActive(int x, int y, int z, const Cell* cell, const World* world) const override {
        if (!mActive) return false;

        World* w = const_cast<World*>(world);
        float cstate = getCellState(cell, world);

        // Must be inside boundaries and have state > 0
        return (cstate > 0.0f &&
                x > 0 && y > 0 && z > 0 &&
                x < w->sizeX() - 1 && y < w->sizeY() - 1 && z < w->sizeZ() - 1);
    }

    RenderConfig getRenderConfig(int x, int y, int z, const Cell* cell, const World* world,
                                 GraphicsRenderer* renderer) const override {
        RenderConfig config;
        config.mode = RenderMode::CUSTOM;  // Custom rendering to check neighbors

        World* w = const_cast<World*>(world);
        float cellState = cell->states[w->index()];

        // Calculate unmap value
        float maxState = w->rule()->numStates() - 1;
        float unmap = 1.0f - (cell->phase / maxState);

        // Store data for custom rendering
        config.customFloats["unmap"] = unmap;
        config.customFloats["cellState"] = cellState;

        // Base color
        config.color = applyMapping(unmap);

        return config;
    }

    void update(float time, float dt) override {
        mTime = time;
    }
};

// ============================================================================
// Pattern17: Combined animated lines (pattern24 + pattern25)
// ============================================================================
class Pattern17 : public Pattern {
public:
    Pattern17() : Pattern(17) {}

    string getName() const override { return "Combined Animated Lines"; }
    string getDescription() const override { return "Animated lines with sine wave motion and cross patterns"; }

    bool isActive(int x, int y, int z, const Cell* cell, const World* world) const override {
        if (!mActive) return false;

        World* w = const_cast<World*>(world);
        float cstate = getCellState(cell, world);

        // Only render on every 5th Z layer and if cell has state
        return ((w->ruleType() == CONT || cstate != 0.0f) && z % 5 == 0);
    }

    RenderConfig getRenderConfig(int x, int y, int z, const Cell* cell, const World* world,
                                 GraphicsRenderer* renderer) const override {
        RenderConfig config;
        config.mode = RenderMode::CUSTOM;  // Custom rendering for multiple lines per cell

        World* w = const_cast<World*>(world);
        float cstate = getCellState(cell, world);

        // Store cell state for custom rendering
        config.customFloats["cstate"] = cstate;
        config.customFloats["x"] = static_cast<float>(x);
        config.customFloats["y"] = static_cast<float>(y);
        config.customFloats["z"] = static_cast<float>(z);

        // Base color (will be modulated in custom rendering)
        config.color = applyMapping(cstate);

        return config;
    }

    void update(float time, float dt) override {
        mTime = time;
    }
};

// ============================================================================
// Pattern18: Spherical polygon quads with modulated theta/phi (pattern31)
// ============================================================================
class Pattern18 : public Pattern {
public:
    Pattern18() : Pattern(18) {}

    string getName() const override { return "Spherical Modulated Quads"; }
    string getDescription() const override { return "Spherical coordinate quads with theta/phi modulation and per-vertex colors"; }

    bool isActive(int x, int y, int z, const Cell* cell, const World* world) const override {
        if (!mActive) return false;

        // Only render on every 4th Z layer and if cell has state > 0
        return (cell->phase > 0.0f && z % 4 == 0);
    }

    RenderConfig getRenderConfig(int x, int y, int z, const Cell* cell, const World* world,
                                 GraphicsRenderer* renderer) const override {
        RenderConfig config;
        config.mode = RenderMode::POLYGON;

        World* w = const_cast<World*>(world);
        float cellState = cell->states[w->index()];

        // Calculate unmap value
        float maxState = w->rule()->numStates() - 1;
        float unmap = 1.0f - (cellState / maxState);

        // Scale factor to match Pattern00's world extent
        float worldScale = (w->sizeX() + w->sizeY() + w->sizeZ()) / 3.0f;
        float fragSizeScale = worldScale * 0.5f;

        // Base spherical coordinates (will be modulated)
        float baseTheta = (2.0f * M_PI / w->sizeX()) * x;
        float basePhi = (2.0f * M_PI / w->sizeY()) * y;

        // Radius with state-based modulation
        float rho = z * (fragSizeScale * 0.5f) + (fragSizeScale * mapf(unmap, -2.0f, 2.0f));

        // Theta/phi modulation factor
        float modulation = mapf(unmap, 0.2f, 0.8f);

        // Subtle audio reactivity: overall amplitude affects modulation
        float audioMod = 1.0f + (getAudioAmplitude(renderer) * 0.15f);  // 0-15% boost
        modulation *= audioMod;

        // Calculate 4 vertices in spherical coords with modulated theta/phi
        // Vertex 0: (x, y)
        float theta0 = baseTheta * modulation;
        float phi0 = basePhi * modulation;
        config.polygonVertices[0].x = rho * cos(theta0) * cos(phi0);
        config.polygonVertices[0].y = rho * sin(theta0) * cos(phi0);
        config.polygonVertices[0].z = rho * sin(phi0);

        // Vertex 1: (x-1, y)
        float baseTheta1 = (2.0f * M_PI / w->sizeX()) * (x - 1);
        float theta1 = baseTheta1 * modulation;
        float phi1 = basePhi * modulation;
        config.polygonVertices[1].x = rho * cos(theta1) * cos(phi1);
        config.polygonVertices[1].y = rho * sin(theta1) * cos(phi1);
        config.polygonVertices[1].z = rho * sin(phi1);

        // Vertex 2: (x-1, y-1)
        float basePhi2 = (2.0f * M_PI / w->sizeY()) * (y - 1);
        float theta2 = baseTheta1 * modulation;
        float phi2 = basePhi2 * modulation;
        config.polygonVertices[2].x = rho * cos(theta2) * cos(phi2);
        config.polygonVertices[2].y = rho * sin(theta2) * cos(phi2);
        config.polygonVertices[2].z = rho * sin(phi2);

        // Vertex 3: (x, y-1)
        float theta3 = baseTheta * modulation;
        float phi3 = basePhi2 * modulation;
        config.polygonVertices[3].x = rho * cos(theta3) * cos(phi3);
        config.polygonVertices[3].y = rho * sin(theta3) * cos(phi3);
        config.polygonVertices[3].z = rho * sin(phi3);

        // Per-vertex colors with different modulations
        // Vertex 0: base unmap
        config.polygonColors[0] = applyMapping(unmap);

        // Vertex 1: sin(unmap * PI)
        float otunmap1 = sin(unmap * M_PI);
        config.polygonColors[1] = applyMapping(otunmap1);

        // Vertex 2: cos(unmap * PI)
        float otunmap2 = cos(unmap * M_PI);
        config.polygonColors[2] = applyMapping(otunmap2);

        // Vertex 3: sin(unmap * PI)
        config.polygonColors[3] = applyMapping(otunmap1);

        return config;
    }

    void update(float time, float dt) override {
        mTime = time;
    }
};

// ============================================================================
// Pattern19: Dynamic neighbor polygon fan (pattern35) - wireframe version
// ============================================================================
class Pattern19 : public Pattern {
public:
    Pattern19() : Pattern(19) {}

    string getName() const override { return "Neighbor Polygon Fan"; }
    string getDescription() const override { return "Dynamic polygon connecting cell to all active neighbors with per-vertex colors (wireframe)"; }

    bool isActive(int x, int y, int z, const Cell* cell, const World* world) const override {
        if (!mActive) return false;

        World* w = const_cast<World*>(world);

        // Must be inside boundaries, have state > 0, and on 4-grid
        if (cell->phase <= 0.0f) return false;
        if (x <= 0 || y <= 0 || z <= 0 ||
            x >= w->sizeX() - 1 || y >= w->sizeY() - 1 || z >= w->sizeZ() - 1) {
            return false;
        }

        // Only render on every 4th coordinate (creates sparser pattern)
        return (x % 4 == 0 || y % 4 == 0 || z % 4 == 0);
    }

    RenderConfig getRenderConfig(int x, int y, int z, const Cell* cell, const World* world,
                                 GraphicsRenderer* renderer) const override {
        RenderConfig config;
        config.mode = RenderMode::CUSTOM;  // Custom rendering for dynamic polygon

        World* w = const_cast<World*>(world);
        float cellState = cell->states[w->index()];

        // Calculate unmap value for center vertex
        float maxState = w->rule()->numStates() - 1;
        float unmap = 1.0f - (cell->phase / maxState);

        // Store data for custom rendering
        config.customFloats["unmap"] = unmap;
        config.customFloats["x"] = static_cast<float>(x);
        config.customFloats["y"] = static_cast<float>(y);
        config.customFloats["z"] = static_cast<float>(z);
        config.customFloats["filled"] = 0.0f;  // Wireframe

        // Base color for center vertex
        config.color = applyMapping(unmap);

        return config;
    }

    void update(float time, float dt) override {
        mTime = time;
    }
};

// ============================================================================
// Pattern20: Dynamic neighbor polygon fan (pattern35) - filled version
// ============================================================================
class Pattern20 : public Pattern {
public:
    Pattern20() : Pattern(20) {}

    string getName() const override { return "Neighbor Polygon Fan (Filled)"; }
    string getDescription() const override { return "Dynamic filled polygon connecting cell to all active neighbors with per-vertex colors"; }

    bool isActive(int x, int y, int z, const Cell* cell, const World* world) const override {
        if (!mActive) return false;

        World* w = const_cast<World*>(world);

        // Must be inside boundaries, have state > 0, and on 4-grid
        if (cell->phase <= 0.0f) return false;
        if (x <= 0 || y <= 0 || z <= 0 ||
            x >= w->sizeX() - 1 || y >= w->sizeY() - 1 || z >= w->sizeZ() - 1) {
            return false;
        }

        // Only render on every 4th coordinate (creates sparser pattern)
        return (x % 4 == 0 || y % 4 == 0 || z % 4 == 0);
    }

    RenderConfig getRenderConfig(int x, int y, int z, const Cell* cell, const World* world,
                                 GraphicsRenderer* renderer) const override {
        RenderConfig config;
        config.mode = RenderMode::CUSTOM;  // Custom rendering for dynamic polygon

        World* w = const_cast<World*>(world);
        float cellState = cell->states[w->index()];

        // Calculate unmap value for center vertex
        float maxState = w->rule()->numStates() - 1;
        float unmap = 1.0f - (cell->phase / maxState);

        // Store data for custom rendering
        config.customFloats["unmap"] = unmap;
        config.customFloats["x"] = static_cast<float>(x);
        config.customFloats["y"] = static_cast<float>(y);
        config.customFloats["z"] = static_cast<float>(z);
        config.customFloats["filled"] = 1.0f;  // Filled

        // Base color for center vertex
        config.color = applyMapping(unmap);

        return config;
    }

    void update(float time, float dt) override {
        mTime = time;
    }
};

// ============================================================================
// Pattern21: Elongated cubes on grid planes (pattern34)
// ============================================================================
class Pattern21 : public Pattern {
public:
    Pattern21() : Pattern(21) {}

    string getName() const override { return "Grid Plane Bars"; }
    string getDescription() const override { return "Elongated cubes on x%5, y%5, z%5 grid planes with state-based sizing"; }

    bool isActive(int x, int y, int z, const Cell* cell, const World* world) const override {
        if (!mActive) return false;

        // Must have state > 0 and be on 5-grid
        if (cell->phase <= 0.0f) return false;
        return (x % 5 == 0 || y % 5 == 0 || z % 5 == 0);
    }

    RenderConfig getRenderConfig(int x, int y, int z, const Cell* cell, const World* world,
                                 GraphicsRenderer* renderer) const override {
        RenderConfig config;
        config.mode = RenderMode::CUBES;

        World* w = const_cast<World*>(world);
        float cellState = cell->states[w->index()];

        // Calculate unmap value
        float maxState = w->rule()->numStates() - 1;
        float unmap = 1.0f - (cell->phase / maxState);

        // Base cube size
        float baseSize = 0.33f;

        // Elongated size based on unmap (mapf(fragSizeX * unmap, 1.0, 4.0) in original)
        // Simplified: elongation factor from 1.0 to 4.0 based on unmap
        float elongation = mapf(unmap, 1.0f, 4.0f);

        // Determine scale based on which grid plane
        vec3 scale(baseSize, baseSize, baseSize);

        if (x % 5 == 0) {
            // Elongate in Z
            scale.z = baseSize + elongation;
        }
        if (y % 5 == 0) {
            // Elongate in X
            scale.x = baseSize + elongation;
        }
        if (z % 5 == 0) {
            // Elongate in Y
            scale.y = baseSize + elongation;
        }

        config.scale = scale;

        // Color with subtle audio reactivity
        ColorA baseColor = applyMapping(unmap);

        // Subtle audio reactivity: low frequencies modulate brightness
        float audioMod = 1.0f + (getAudioBand(renderer, 0) * 0.25f);  // 0-25% boost from bass
        baseColor.r = std::min(baseColor.r * audioMod, 1.0f);
        baseColor.g = std::min(baseColor.g * audioMod, 1.0f);
        baseColor.b = std::min(baseColor.b * audioMod, 1.0f);

        config.color = baseColor;

        return config;
    }

    void update(float time, float dt) override {
        mTime = time;
    }
};

// ============================================================================
// Pattern22: Textured cubes with pattern15 mappings (using 03.png)
// ============================================================================
class Pattern22 : public Pattern {
private:
    mutable gl::TextureRef mTexture;  // Mutable to allow lazy loading

public:
    Pattern22() : Pattern(22) {}

    string getName() const override { return "Grid Textured Cubes"; }
    string getDescription() const override { return "Textured cubes on boundaries with inverse state sizing using 08.png"; }

    bool isActive(int x, int y, int z, const Cell* cell, const World* world) const override {
        if (!mActive) return false;
        if (cell->phase <= 0.0f) return false;

        World* w = const_cast<World*>(world);

        // Only render on boundaries to reduce texture overhead
        return (x == 0 || y == 0 || z == 0 ||
                (x == w->sizeX() - 1 && y < w->sizeY() - 1 && z < w->sizeZ() - 1) ||
                (y == w->sizeY() - 1 && z < w->sizeZ() - 1 && x < w->sizeX() - 1) ||
                (z == w->sizeZ() - 1 && x < w->sizeX() - 1 && y < w->sizeY() - 1));
    }

    RenderConfig getRenderConfig(int x, int y, int z, const Cell* cell, const World* world,
                                 GraphicsRenderer* renderer) const override {
        RenderConfig config;
        config.mode = RenderMode::CUBES;

        World* w = const_cast<World*>(world);

        // Lazy load texture
        if (!mTexture) {
            try {
                auto imgSource = loadImage(app::loadAsset("08.png"));
                gl::Texture::Format fmt;
                fmt.setWrap(GL_REPEAT, GL_REPEAT);
                fmt.setMinFilter(GL_LINEAR);
                fmt.setMagFilter(GL_LINEAR);
                mTexture = gl::Texture::create(imgSource, fmt);
                console() << "Pattern22: Loaded texture from 08.png successfully!" << std::endl;
            }
            catch (const std::exception& e) {
                console() << "Pattern22: Failed to load texture: " << e.what() << std::endl;
            }
        }

        config.texture = mTexture;

        // Pattern15 state calculation
        float cstate;
        if (w->ruleType() == CONT) {
            cstate = cell->phase;
        } else {
            if (cell->phase != 0.0f) {
                cstate = 1.0f / cell->phase;  // Inverse of state
            } else {
                cstate = 0.0f;
            }
        }

        // Map to 0.75-1.3 range
        cstate = mapf(cstate, 0.75f, 1.3f);

        // Size: fragSizeX * cstate (from pattern15)
        // In normalized coords, this becomes just cstate
        config.scale = vec3(cstate, cstate, cstate);

        // Color mapping using cstate
        ColorA baseColor = applyMapping(cstate);

        // Subtle audio reactivity: mid frequencies modulate brightness
        float audioMod = 1.0f + (getAudioBand(renderer, 1) * 0.25f);  // 0-25% boost from mids
        baseColor.r = std::min(baseColor.r * audioMod, 1.0f);
        baseColor.g = std::min(baseColor.g * audioMod, 1.0f);
        baseColor.b = std::min(baseColor.b * audioMod, 1.0f);

        config.color = baseColor;

        return config;
    }

    void update(float time, float dt) override {
        mTime = time;
    }
};

// ============================================================================
// Pattern23: Hexagonal boundary cells (like Pattern00 but hexagons)
// ============================================================================
class Pattern23 : public Pattern {
private:
    float mBrightness = 2.5f;  // Overall brightness multiplier

public:
    Pattern23() : Pattern(23) {}

    string getName() const override { return "Hexagonal Boundary Cells"; }
    string getDescription() const override { return "Draws hexagonal cells on boundaries with nested scaling"; }

    bool isActive(int x, int y, int z, const Cell* cell, const World* world) const override {
        if (!mActive) return false;
        if (cell->phase <= 0.0f) return false;

        World* w = const_cast<World*>(world);

        // Only render on boundaries (same as Pattern00)
        return (x == 0 || y == 0 || z == 0 ||
                (x == w->sizeX() - 1 && y < w->sizeY() - 1 && z < w->sizeZ() - 1) ||
                (y == w->sizeY() - 1 && z < w->sizeZ() - 1 && x < w->sizeX() - 1) ||
                (z == w->sizeZ() - 1 && x < w->sizeX() - 1 && y < w->sizeY() - 1));
    }

    RenderConfig getRenderConfig(int x, int y, int z, const Cell* cell, const World* world,
                                 GraphicsRenderer* renderer) const override {
        RenderConfig config;
        config.mode = RenderMode::CUSTOM;  // Custom rendering for hexagons

        World* w = const_cast<World*>(world);
        float cstate = getCellState(cell, world);
        float unmap = 1.0f - cstate;

        // Determine which boundaries we're on
        bool onXNeg = (x == 0);
        bool onYNeg = (y == 0);
        bool onZNeg = (z == 0);
        bool onXPos = (x == w->sizeX() - 1 && y < w->sizeY() - 1 && z < w->sizeZ() - 1);
        bool onYPos = (y == w->sizeY() - 1 && z < w->sizeZ() - 1 && x < w->sizeX() - 1);
        bool onZPos = (z == w->sizeZ() - 1 && x < w->sizeX() - 1 && y < w->sizeY() - 1);

        // Store rendering info for custom hexagonal cells
        config.customFloats["unmap"] = unmap;
        config.customFloats["drawHexagons"] = 1.0f;

        config.customFloats["onXNeg"] = onXNeg ? 1.0f : 0.0f;
        config.customFloats["onYNeg"] = onYNeg ? 1.0f : 0.0f;
        config.customFloats["onZNeg"] = onZNeg ? 1.0f : 0.0f;
        config.customFloats["onXPos"] = onXPos ? 1.0f : 0.0f;
        config.customFloats["onYPos"] = onYPos ? 1.0f : 0.0f;
        config.customFloats["onZPos"] = onZPos ? 1.0f : 0.0f;

        // Base color with brightness affecting alpha only
        ColorA baseColor = applyMapping(unmap);

        // Apply overall brightness and audio reactivity to alpha only
        float audioMod = 1.0f + (getAudioAmplitude(renderer) * 0.2f);
        float totalBrightness = mBrightness * audioMod;

        baseColor.a = std::min(baseColor.a * totalBrightness, 1.0f);
        config.color = baseColor;

        return config;
    }

    void update(float time, float dt) override {
        mTime = time;
    }
};

// ============================================================================
// Pattern Factory
// ============================================================================
std::unique_ptr<Pattern> PatternFactory::create(int id) {
    switch (id) {
        case 0: return std::unique_ptr<Pattern>(new Pattern00());
        case 1: return std::unique_ptr<Pattern>(new Pattern01());
        case 2: return std::unique_ptr<Pattern>(new Pattern02());
        case 3: return std::unique_ptr<Pattern>(new Pattern03());
        case 4: return std::unique_ptr<Pattern>(new Pattern04());
        case 5: return std::unique_ptr<Pattern>(new Pattern05());
        case 6: return std::unique_ptr<Pattern>(new Pattern06());
        case 7: return std::unique_ptr<Pattern>(new Pattern07());
        case 8: return std::unique_ptr<Pattern>(new Pattern08());
        case 9: return std::unique_ptr<Pattern>(new Pattern09());
        case 10: return std::unique_ptr<Pattern>(new Pattern10());
        case 11: return std::unique_ptr<Pattern>(new Pattern11());
        case 12: return std::unique_ptr<Pattern>(new Pattern12());
        case 13: return std::unique_ptr<Pattern>(new Pattern13());
        case 14: return std::unique_ptr<Pattern>(new Pattern14());
        case 15: return std::unique_ptr<Pattern>(new Pattern15());
        case 16: return std::unique_ptr<Pattern>(new Pattern16());
        case 17: return std::unique_ptr<Pattern>(new Pattern17());
        case 18: return std::unique_ptr<Pattern>(new Pattern18());
        case 19: return std::unique_ptr<Pattern>(new Pattern19());
        case 20: return std::unique_ptr<Pattern>(new Pattern20());
        case 21: return std::unique_ptr<Pattern>(new Pattern21());
        case 22: return std::unique_ptr<Pattern>(new Pattern22());
        case 23: return std::unique_ptr<Pattern>(new Pattern23());
        default: return nullptr;
    }
}
