/*
 *  boidpattern.h
 *  lambda
 *
 *  Boid rendering pattern system for GPU-instanced boid visualization
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

#ifndef BOIDPATTERN_H
#define BOIDPATTERN_H

#include "cinder/app/App.h"
#include "cinder/Color.h"
#include "boids.h"
#include <memory>

using namespace ci;
using namespace std;

// Forward declaration
class GraphicsRenderer;

// Boid rendering modes
enum class BoidRenderMode {
    ENVMAP,       // Environment mapped spheres using cubemap
    TRAILS,       // Spheres with trailing particles based on velocity
    CONNECTIONS,  // Lines connecting nearby boids
    SPLINES,      // B-spline curves through boid positions
    CUSTOM        // Custom rendering in drawBoids
};

// Configuration for boid rendering
struct BoidRenderConfig {
    BoidRenderMode mode = BoidRenderMode::ENVMAP;
    ColorA color = ColorA(1.0f, 1.0f, 1.0f, 1.0f);
    float size = 1.0f;
    float lineWidth = 1.0f;
    int trailLength = 5;              // For TRAILS mode
    float connectionRadius = 5.0f;     // For CONNECTIONS mode
    bool useVelocityColor = false;    // Color based on velocity
    bool useDepthFade = false;        // Fade alpha based on distance from camera
    bool useEnvMap = false;           // Use environment map shader
    bool useEnvMapPattern13 = false;  // Use fxp_* cubemap (true) vs fxic_* (false)
};

// Base class for boid rendering patterns
class BoidPattern {
protected:
    int mId;
    bool mActive = false;
    float mAlpha = 1.0f;
    int mColorMap = 0;
    int mAlphaMap = 0;
    Color mColor = Color(1.0f, 1.0f, 1.0f);

public:
    BoidPattern(int id) : mId(id) {}
    virtual ~BoidPattern() {}

    // Pattern identification
    virtual string getName() const = 0;
    virtual string getDescription() const = 0;
    int getId() const { return mId; }

    // Activation control
    virtual void setActive(bool active) { mActive = active; }
    virtual bool isActive() const { return mActive; }

    // Color and mapping control
    virtual void setAlpha(float alpha) { mAlpha = alpha; }
    virtual void setColorMap(int map) { mColorMap = map; }
    virtual void setAlphaMap(int map) { mAlphaMap = map; }
    virtual void setColor(const Color& color) { mColor = color; }

    virtual float getAlpha() const { return mAlpha; }
    virtual int getColorMap() const { return mColorMap; }
    virtual int getAlphaMap() const { return mAlphaMap; }
    virtual Color getColor() const { return mColor; }

    // Rendering configuration
    virtual BoidRenderConfig getRenderConfig(Boid* boid, int boidIndex, const Boids* boids,
                                             GraphicsRenderer* renderer) const = 0;

    // Color mapping helper (similar to cellular patterns)
    ColorA applyMapping(float value) const {
        float colorFactor = abs(mColorMap - value);
        float alphaFactor = abs(mAlphaMap - value);
        return ColorA(
            mColor.r * colorFactor,
            mColor.g * colorFactor,
            mColor.b * colorFactor,
            mAlpha * alphaFactor
        );
    }
};

// Factory for creating boid patterns
class BoidPatternFactory {
public:
    static unique_ptr<BoidPattern> create(int id);
};

#endif
