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
// Pattern00: Wireframe boundary lines
// ============================================================================
class Pattern00 : public Pattern {
public:
    Pattern00() : Pattern(0) {}

    string getName() const override { return "Wireframe Boundaries"; }
    string getDescription() const override { return "Draws wireframe lines on world boundaries"; }

    bool isActive(int x, int y, int z, const Cell* cell, const World* world) const override {
        if (!mActive) return false;

        World* w = const_cast<World*>(world);
        float cstate = getCellState(cell, world);
        if (w->ruleType() != CONT && cstate == 0.0f) return false;

        // Only draw on boundaries
        return (x == 0 || y == 0 || z == 0 ||
                x == w->sizeX() - 1 || y == w->sizeY() - 1 || z == w->sizeZ() - 1);
    }

    RenderConfig getRenderConfig(int x, int y, int z, const Cell* cell, const World* world,
                                 GraphicsRenderer* renderer) const override {
        RenderConfig config;
        config.mode = RenderMode::LINES;

        World* w = const_cast<World*>(world);
        float cstate = getCellState(cell, world);
        config.color = applyMapping(cstate);

        // Subtle audio reactivity: modulate line width with low frequency band
        float audioMod = 1.0f + (getAudioBand(renderer, 0) * 0.5f);  // 0-50% boost from bass
        config.lineWidth = 1.0f * audioMod;
        config.lineSmooth = true;
        config.uniformScale = cstate * 2.0f;

        // Compute grid position and size (from renderer's fragSize)
        // This will be used by drawFragment to create line segments
        // The actual line endpoints will be computed in drawFragment based on which boundary

        return config;
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
// Pattern03: Complex animated geometry
// ============================================================================
class Pattern03 : public Pattern {
private:
    float mCenterRadius = 5.0f;
    float mRotationSpeed = 2.0f;
    float mNoiseScale = 0.5f;

public:
    Pattern03() : Pattern(3) {}

    string getName() const override { return "Complex Geometry"; }
    string getDescription() const override { return "Animated spheres with rotation, scaling, and glow effects"; }

    bool isActive(int x, int y, int z, const Cell* cell, const World* world) const override {
        if (!mActive) return false;

        float dist = distanceFromCenter(x, y, z, world);
        float cstate = getCellState(cell, world);
        return dist > mCenterRadius && cstate > 0.4f;
    }

    RenderConfig getRenderConfig(int x, int y, int z, const Cell* cell, const World* world,
                                 GraphicsRenderer* renderer) const override {
        RenderConfig config;
        config.mode = RenderMode::CYLINDERS;

        World* w = const_cast<World*>(world);
        float cstate = getCellState(cell, world);

        // Get world center and position
        vec3 center = vec3(w->sizeX(), w->sizeY(), w->sizeZ()) * 0.5f;
        vec3 pos = vec3(x, y, z);
        vec3 toCenter = glm::normalize(center - pos);

        // Rotate cylinder to point toward center
        vec3 up = vec3(0, 1, 0);
        vec3 cylinderAxis = vec3(0, 1, 0);  // Cylinder default orientation

        // Create rotation from cylinder axis to toCenter direction
        float angle = acos(glm::dot(cylinderAxis, toCenter));
        vec3 rotationAxis = glm::cross(cylinderAxis, toCenter);
        if (glm::length(rotationAxis) > 0.001f) {
            rotationAxis = glm::normalize(rotationAxis);
            config.rotation = glm::angleAxis(angle, rotationAxis);
        }

        // Add spinning animation around the toCenter axis
        quat spin = glm::angleAxis(mTime * mRotationSpeed, toCenter);
        config.rotation = spin * config.rotation;

        // Animated non-uniform scale - thin in X/Z, elongated in Y (cylinder height)
        float pulse = sin(mTime * mRotationSpeed + x * 0.3f + y * 0.2f + z * 0.1f) * 0.5f + 0.5f;

        // Subtle audio reactivity: high frequencies modulate cylinder height
        float audioMod = 1.0f + (getAudioBand(renderer, 2) * 0.4f);  // 0-40% boost from highs
        config.scale = vec3(
            0.01f + 0.01f * pulse,           // X - thin
            (4.0f + 0.5f * pulse) * audioMod,  // Y - height varies with audio
            0.01f + 0.01f * pulse            // Z - thin
        );

        // Simple noise-based offset using sin/cos approximation
        vec3 noisePos = pos * mNoiseScale + vec3(mTime * 0.1f);
        config.offset = vec3(
            (sin(noisePos.x + noisePos.y) - 0.5f) * 0.5f,
            (cos(noisePos.y + noisePos.z) - 0.5f) * 0.5f,
            (sin(noisePos.z + noisePos.x) - 0.5f) * 0.5f
        );

        // Distance-based color with HSV-like mapping
        float dist = normalizedDistance(x, y, z, world);
        float hue = fmod(dist + mTime * 0.1f, 1.0f);  // Animate hue over time

        // Simple HSV to RGB conversion for hue variation
        float r = abs(hue * 6.0f - 3.0f) - 1.0f;
        float g = 2.0f - abs(hue * 6.0f - 2.0f);
        float b = 2.0f - abs(hue * 6.0f - 4.0f);
        r = glm::clamp(r, 0.0f, 1.0f);
        g = glm::clamp(g, 0.0f, 1.0f);
        b = glm::clamp(b, 0.0f, 1.0f);
        config.color = ColorA(r * 0.8f, g * 0.8f, b, 1.0f);

        // Glow effect based on cell state
        config.glow = cstate;
        config.emissiveColor = config.color * cstate;

        // Material properties
        config.metallic = 0.8f;
        config.roughness = 0.2f + dist * 0.3f;

        // Blend mode
        config.blendMode = BlendMode::ADDITIVE;

        // Uniform scale multiplier
        config.uniformScale = mapf(cstate, 0.5f, 1.0f);

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
    string getDescription() const override { return "Draws texture-mapped cubes throughout the world"; }

    bool isActive(int x, int y, int z, const Cell* cell, const World* world) const override {
        if (!mActive) return false;

        World* w = const_cast<World*>(world);
        float cstate = getCellState(cell, world);
        return (w->ruleType() == CONT || cstate != 0.0f);
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
        config.uniformScale = mapf(cstate * (0.7f + pulse * 0.3f), 0.5f, 1.2f) * audioMod;

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
    string getDescription() const override { return "Environment-mapped spheres with cubemap reflections"; }

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

        // Base color with subtle audio reactivity on alpha
        ColorA baseColor = applyMapping(unmap);

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
        default: return nullptr;
    }
}
