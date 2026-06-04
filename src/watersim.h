#pragma once
#include "cinder/gl/gl.h"
#include <vector>

using namespace ci;

class WaterSim {
public:
    WaterSim();

    void setup(int size = 256);
    void update();
    void addDrop(float x, float y, float radius, float strength);
    void addCymaticDrops(const std::vector<float>& bands, float amplitude);
    // Fire drops at 4 corners driven by frequency bands (sub, low, mid, high)
    void addCornerDrops(float sub, float low, float mid, float high);

    gl::TextureRef getHeightTexture();
    bool isReady() const { return mInitialized; }

    // Read the height value at a UV position (0-1) — CPU readback, use sparingly
    float readHeightAt(float u, float v);

    // Controllable parameters
    bool   cymatics      = false;
    float  damping       = 0.995f;
    float  waveSpeed     = 2.0f;
    float  cymaticJitter = 0.5f;  // spatial randomness around walker positions
    float  cymaticGain   = 0.2f;  // drop strength multiplier
    float  cymaticRadius   = 0.15f; // max distance walkers roam from center (0=center, 0.45=full pool)

    // Continuous corner boundary forcing
    bool   boundaryForcing = false;
    float  boundaryValues[4] = {0,0,0,0}; // sub, low, mid, high — set each frame from audio
    float  boundaryRadius  = 0.06f;        // corner region size in UV space

private:
    int            mSize    = 256;
    int            mCurrent = 0;
    gl::FboRef     mSimFbo[2];
    gl::GlslProgRef mUpdateShader;
    gl::GlslProgRef mDropShader;
    bool           mInitialized = false;

    void renderFullQuad(gl::GlslProgRef shader);
};
