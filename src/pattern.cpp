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

        float cstate = getCellState(cell, world);
        config.color = applyMapping(cstate);

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
        config.uniformScale = mapf(cstate, 0.5f, 2.0f);
        config.color = applyMapping(cstate);

        return config;
    }
};

// ============================================================================
// Pattern02: Color only (no geometry)
// ============================================================================
class Pattern02 : public Pattern {
public:
    Pattern02() : Pattern(2) {}

    string getName() const override { return "Color Field"; }
    string getDescription() const override { return "Applies color mapping without geometry"; }

    bool isActive(int x, int y, int z, const Cell* cell, const World* world) const override {
        // This pattern doesn't render geometry, just modifies color state
        return false;
    }

    RenderConfig getRenderConfig(int x, int y, int z, const Cell* cell, const World* world,
                                 GraphicsRenderer* renderer) const override {
        // Not used, but required by interface
        return RenderConfig();
    }
};

// ============================================================================
// Pattern03: Spheres based on cell history
// ============================================================================
class Pattern03 : public Pattern {
public:
    Pattern03() : Pattern(3) {}

    string getName() const override { return "History Spheres"; }
    string getDescription() const override { return "Draws spheres sized by cell state history"; }

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
        float maxState = w->rule()->numStates() - 1;
        float mapState = (maxState - cell->states[w->index()]) * (1.0f / maxState);

        config.uniformScale = mapf(mapState, 0.25f, 0.75f);
        config.color = applyMapping(mapState);

        return config;
    }
};

// ============================================================================
// Pattern04: Boundary cubes with state-based sizing
// ============================================================================
class Pattern04 : public Pattern {
public:
    Pattern04() : Pattern(4) {}

    string getName() const override { return "State Boundary Cubes"; }
    string getDescription() const override { return "Draws cubes on boundaries sized by state"; }

    bool isActive(int x, int y, int z, const Cell* cell, const World* world) const override {
        if (!mActive) return false;

        World* w = const_cast<World*>(world);
        float cstate = cell->states[w->index()];
        if (w->ruleType() != CONT && cstate == 0.0f) return false;

        // Only draw on boundaries
        return (x == 0 || y == 0 || z == 0 ||
                x == w->sizeX() - 1 ||
                y == w->sizeY() - 1 ||
                z == w->sizeZ() - 1);
    }

    RenderConfig getRenderConfig(int x, int y, int z, const Cell* cell, const World* world,
                                 GraphicsRenderer* renderer) const override {
        RenderConfig config;
        config.mode = RenderMode::CUBES;

        World* w = const_cast<World*>(world);
        float unmap = 1.0f - unmapf(cell->phase, 0, w->rule()->numStates() - 1);

        // Determine which plane and set appropriate scale
        vec3 scale(1.0f);
        if (x == 0 || x == w->sizeX() - 1) {
            scale.x = 0.1f;
        }
        if (y == 0 || y == w->sizeY() - 1) {
            scale.y = 0.1f;
        }
        if (z == 0 || z == w->sizeZ() - 1) {
            scale.z = 0.1f;
        }

        config.scale = scale;
        config.uniformScale = unmap * 2.0f;
        config.color = applyMapping(unmap);

        return config;
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
        default: return nullptr;
    }
}
