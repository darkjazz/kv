/*
 *  boidpattern.cpp
 *  lambda
 *
 *  Boid rendering pattern implementations
 *
 *	This file is part of lambda.
 */

#include "boidpattern.h"
#include "ogl.h"
#include "util.h"

// ============================================================================
// BoidPattern00: Environment mapped spheres (cubemap)
// ============================================================================
class BoidPattern00 : public BoidPattern {
public:
    BoidPattern00() : BoidPattern(0) {}

    string getName() const override { return "Boid Cubemap"; }
    string getDescription() const override { return "Environment mapped spheres using cubemap"; }

    BoidRenderConfig getRenderConfig(Boid* boid, int boidIndex, const Boids* boids,
                                     GraphicsRenderer* renderer) const override {
        BoidRenderConfig config;
        config.mode = BoidRenderMode::ENVMAP;

        // Sphere size based on distance from center - larger range
        vec3 boidDimensions = boids->dimensions();
        vec3 center = boidDimensions * 0.5f;
        float distFromCenter = glm::distance(boid->pos, center);
        float maxDist = glm::distance(boidDimensions, center);
        float normalizedDist = glm::clamp(distFromCenter / maxDist, 0.0f, 1.0f);

        config.size = mapf(normalizedDist, 2.0f, 5.0f);  // Larger spheres (was 1.0-3.0)
        config.useEnvMap = true;

        // Much brighter base color for env map mixing
        config.color = ColorA(0.9f, 0.9f, 0.9f, mAlpha);
        config.useDepthFade = false;

        return config;
    }
};

// ============================================================================
// BoidPattern01: Spheres with velocity-based trails
// ============================================================================
class BoidPattern01 : public BoidPattern {
public:
    BoidPattern01() : BoidPattern(1) {}

    string getName() const override { return "Boid Trails"; }
    string getDescription() const override { return "Spheres with trailing particles based on velocity"; }

    BoidRenderConfig getRenderConfig(Boid* boid, int boidIndex, const Boids* boids,
                                     GraphicsRenderer* renderer) const override {
        BoidRenderConfig config;
        config.mode = BoidRenderMode::TRAILS;

        // Smaller heads with LONG trails
        config.size = 1.5f;
        config.trailLength = 50;  // Long, menacing trails

        // Velocity magnitude for intensity
        float velocityMag = glm::length(boid->vec);

        // Dark purple - ominous and threatening
        vec3 rgb = vec3(0.3f, 0.1f, 0.4f);  // Deep purple

        // More velocity = slightly brighter but still dark
        float intensity = 0.8f + glm::clamp(velocityMag / 4.0f, 0.0f, 0.2f);

        config.color = ColorA(rgb.r * intensity, rgb.g * intensity, rgb.b * intensity, mAlpha * 0.9f);
        config.useVelocityColor = true;
        config.useDepthFade = true;

        return config;
    }
};

// ============================================================================
// BoidPattern02: Lines connecting nearby boids
// ============================================================================
class BoidPattern02 : public BoidPattern {
public:
    BoidPattern02() : BoidPattern(2) {}

    string getName() const override { return "Boid Network"; }
    string getDescription() const override { return "Lines connecting nearby boids in a network"; }

    BoidRenderConfig getRenderConfig(Boid* boid, int boidIndex, const Boids* boids,
                                     GraphicsRenderer* renderer) const override {
        BoidRenderConfig config;
        config.mode = BoidRenderMode::CONNECTIONS;

        // Connection parameters
        config.connectionRadius = 50.0f;  // Connect boids within this distance (increased for 200x200x200 space)
        config.lineWidth = 3.0f;  // Thicker lines for visibility

        // Small spheres with environment mapping (fxp_* cubemap - Pattern13 style)
        config.size = 0.8f;  // Small nodes
        config.useEnvMap = true;  // Enable cubemap on spheres
        config.useEnvMapPattern13 = true;  // Use fxp_* cubemap instead of fxic_*

        // Use pattern color (controllable via OSC) for both nodes and lines
        config.color = ColorA(mColor.r, mColor.g, mColor.b, mAlpha);
        config.useDepthFade = false;

        return config;
    }
};

// ============================================================================
// BoidPattern03: B-spline curves through boid positions
// ============================================================================
class BoidPattern03 : public BoidPattern {
public:
    BoidPattern03() : BoidPattern(3) {}

    string getName() const override { return "Boid Splines"; }
    string getDescription() const override { return "B-spline curves through boid positions"; }

    BoidRenderConfig getRenderConfig(Boid* boid, int boidIndex, const Boids* boids,
                                     GraphicsRenderer* renderer) const override {
        BoidRenderConfig config;
        config.mode = BoidRenderMode::SPLINES;

        // Use pattern color (controllable via OSC)
        config.color = ColorA(mColor.r, mColor.g, mColor.b, mAlpha);

        return config;
    }
};

// ============================================================================
// BoidPattern04: Global filled polygon through all boids
// ============================================================================
class BoidPattern04 : public BoidPattern {
public:
    BoidPattern04() : BoidPattern(4) {}

    string getName() const override { return "Boid Polygon"; }
    string getDescription() const override { return "Filled polygon with straight edges joining all boids"; }

    BoidRenderConfig getRenderConfig(Boid* boid, int boidIndex, const Boids* boids,
                                     GraphicsRenderer* renderer) const override {
        BoidRenderConfig config;
        config.mode = BoidRenderMode::POLYGON;
        config.lineWidth = 2.0f;

        // Use pattern color (controllable via OSC) - alpha scales both fill and outline
        config.color = ColorA(mColor.r, mColor.g, mColor.b, mAlpha);

        return config;
    }
};

// ============================================================================
// BoidPattern05: Convex hull skin wrapped around the whole flock
// ============================================================================
class BoidPattern05 : public BoidPattern {
public:
    BoidPattern05() : BoidPattern(5) {}

    string getName() const override { return "Boid Hull"; }
    string getDescription() const override { return "Translucent convex hull skin around the flock, brighter at the silhouette"; }

    BoidRenderConfig getRenderConfig(Boid* boid, int boidIndex, const Boids* boids,
                                     GraphicsRenderer* renderer) const override {
        BoidRenderConfig config;
        config.mode = BoidRenderMode::HULL;
        config.lineWidth = 1.0f;

        // Use pattern color (controllable via OSC) - alpha scales skin and edges
        config.color = ColorA(mColor.r, mColor.g, mColor.b, mAlpha);

        return config;
    }
};

// ============================================================================
// BoidPattern06: Metaballs - boids merge into one blobby surface where they cluster
// ============================================================================
class BoidPattern06 : public BoidPattern {
public:
    BoidPattern06() : BoidPattern(6) {}

    string getName() const override { return "Boid Metaballs"; }
    string getDescription() const override { return "Blobby isosurface that merges boids where they cluster"; }

    BoidRenderConfig getRenderConfig(Boid* boid, int boidIndex, const Boids* boids,
                                     GraphicsRenderer* renderer) const override {
        BoidRenderConfig config;
        config.mode = BoidRenderMode::METABALLS;

        // Multiplier on the influence radius, which the renderer derives from the
        // flock's typical neighbour spacing (controllable via OSC)
        config.size = mSize;

        // Use pattern color (controllable via OSC)
        config.color = ColorA(mColor.r, mColor.g, mColor.b, mAlpha);

        return config;
    }
};

// ============================================================================
// BoidPattern Factory
// ============================================================================
unique_ptr<BoidPattern> BoidPatternFactory::create(int id) {
    switch (id) {
        case 0: return unique_ptr<BoidPattern>(new BoidPattern00());
        case 1: return unique_ptr<BoidPattern>(new BoidPattern01());
        case 2: return unique_ptr<BoidPattern>(new BoidPattern02());
        case 3: return unique_ptr<BoidPattern>(new BoidPattern03());
        case 4: return unique_ptr<BoidPattern>(new BoidPattern04());
        case 5: return unique_ptr<BoidPattern>(new BoidPattern05());
        case 6: return unique_ptr<BoidPattern>(new BoidPattern06());
        default: return nullptr;
    }
}
