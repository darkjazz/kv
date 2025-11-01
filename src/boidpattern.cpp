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
// BoidPattern00: Simple spheres at boid positions
// ============================================================================
class BoidPattern00 : public BoidPattern {
public:
    BoidPattern00() : BoidPattern(0) {}

    string getName() const override { return "Boid Spheres"; }
    string getDescription() const override { return "Simple spheres at each boid position"; }

    BoidRenderConfig getRenderConfig(Boid* boid, int boidIndex, const Boids* boids,
                                     GraphicsRenderer* renderer) const override {
        BoidRenderConfig config;
        config.mode = BoidRenderMode::SPHERES;

        // Larger fixed size for visibility
        config.size = 5.0f;

        // Bright white color for visibility (ignore mapping initially)
        config.color = ColorA(1.0f, 1.0f, 1.0f, 1.0f);
        config.useVelocityColor = false;
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

        // Main boid size
        config.size = 1.0f;
        config.trailLength = 8;  // Number of trail particles

        // Color based on velocity
        float velocityMag = glm::length(boid->vec);
        float normalizedVel = glm::clamp(velocityMag / 2.0f, 0.0f, 1.0f);

        config.color = applyMapping(normalizedVel);
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
        config.connectionRadius = 8.0f;  // Connect boids within this distance
        config.lineWidth = 1.0f;

        // Sphere size for boid nodes
        config.size = 0.5f;

        // Color based on cohesion strength
        // cohesion is Vec3fRS type, use mean() to get vec3
        vec3 cohesionVec = boid->cohesion.mean();
        float cohesionMag = glm::length(cohesionVec);
        float normalizedCohesion = glm::clamp(cohesionMag / 5.0f, 0.0f, 1.0f);

        config.color = applyMapping(normalizedCohesion);
        config.useDepthFade = false;

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
        default: return nullptr;
    }
}
