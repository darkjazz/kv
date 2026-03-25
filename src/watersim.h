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

    gl::TextureRef getHeightTexture();
    bool isReady() const { return mInitialized; }

    // Controllable parameters
    bool   cymatics  = false;
    float  damping   = 0.995f;
    float  waveSpeed = 2.0f;

private:
    int            mSize    = 256;
    int            mCurrent = 0;
    gl::FboRef     mSimFbo[2];
    gl::GlslProgRef mUpdateShader;
    gl::GlslProgRef mDropShader;
    bool           mInitialized = false;

    void renderFullQuad(gl::GlslProgRef shader);
};
