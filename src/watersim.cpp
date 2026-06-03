#include "watersim.h"
#include "cinder/app/App.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;

// Cymatics: 13 walkers that drift across the water surface.
// Each band drives a drop at its walker's current position.
// Walkers bounce off the edges and steer randomly each frame.
struct CymaticWalker {
    vec2 pos;
    vec2 vel;
};
static CymaticWalker sWalkers[13];
static bool sWalkersInit = false;

static void initWalkers() {
    for (int i = 0; i < 13; i++) {
        float angle = ci::randFloat(0.0f, float(M_PI * 2.0));
        float speed = ci::randFloat(0.03f, 0.08f);
        sWalkers[i].pos = vec2(ci::randFloat(0.1f, 0.9f), ci::randFloat(0.1f, 0.9f));
        sWalkers[i].vel = vec2(std::cos(angle), std::sin(angle)) * speed;
    }
    sWalkersInit = true;
}

WaterSim::WaterSim() {}

void WaterSim::setup(int size) {
    mSize    = size;
    mCurrent = 0;

    // RGBA32F — full precision for height/velocity accumulation
    gl::Texture2d::Format texFmt;
    texFmt.setInternalFormat(GL_RGBA32F);
    texFmt.setMinFilter(GL_LINEAR);
    texFmt.setMagFilter(GL_LINEAR);
    texFmt.setWrapS(GL_CLAMP_TO_EDGE);
    texFmt.setWrapT(GL_CLAMP_TO_EDGE);

    gl::Fbo::Format fboFmt;
    fboFmt.setColorTextureFormat(texFmt);
    fboFmt.disableDepth();

    try {
        mSimFbo[0] = gl::Fbo::create(mSize, mSize, fboFmt);
        mSimFbo[1] = gl::Fbo::create(mSize, mSize, fboFmt);

        // Clear both to zero (calm water).
        // Must use glClearBufferfv for RGBA32F — glClear leaves float FBOs undefined.
        // Alpha=1 so blending (if accidentally active) doesn't zero out writes.
        const float zeros[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        for (int i = 0; i < 2; i++) {
            gl::ScopedFramebuffer fboScope(mSimFbo[i]);
            glClearBufferfv(GL_COLOR, 0, zeros);
        }
        console() << "WaterSim: FBOs created (" << mSize << "x" << mSize << ")" << std::endl;
    }
    catch (const std::exception& e) {
        console() << "WaterSim: FBO creation failed: " << e.what() << std::endl;
        return;
    }

    try {
        auto vert = app::loadAsset("blur.vert");   // reuse existing passthrough vert
        mUpdateShader = gl::GlslProg::create(vert, app::loadAsset("water_update.frag"));
        mDropShader   = gl::GlslProg::create(vert, app::loadAsset("water_drop.frag"));
        console() << "WaterSim: shaders loaded" << std::endl;
    }
    catch (const std::exception& e) {
        console() << "WaterSim: shader load failed: " << e.what() << std::endl;
        return;
    }

    mInitialized = true;
}

void WaterSim::renderFullQuad(gl::GlslProgRef shader) {
    auto rect = geom::Rect(Rectf(-1, -1, 1, 1))
                    .texCoords(vec2(0, 0), vec2(1, 0), vec2(1, 1), vec2(0, 1));
    gl::Batch::create(rect, shader)->draw();
}

void WaterSim::update() {
    if (!mInitialized) return;

    int next = 1 - mCurrent;

    gl::ScopedFramebuffer fboScope(mSimFbo[next]);
    gl::ScopedViewport    vpScope(ivec2(0), mSimFbo[next]->getSize());
    gl::ScopedMatrices    matScope;
    gl::setMatrices(CameraOrtho(-1, 1, -1, 1, -1, 1));
    gl::ScopedDepth       depthScope(false);
    gl::ScopedBlend       blendOff(false);   // compute pass — blending must be off

    gl::ScopedTextureBind texScope(mSimFbo[mCurrent]->getColorTexture(), 0);
    gl::ScopedGlslProg    shaderScope(mUpdateShader);

    float delta = 1.0f / (float)mSize;
    mUpdateShader->uniform("uHeightTex", 0);
    mUpdateShader->uniform("uDelta",     vec2(delta, delta));
    mUpdateShader->uniform("uDamping",   damping);
    mUpdateShader->uniform("uSpeed",     waveSpeed);

    renderFullQuad(mUpdateShader);

    mCurrent = next;
}

void WaterSim::addDrop(float x, float y, float radius, float strength) {
    if (!mInitialized) return;

    int next = 1 - mCurrent;

    gl::ScopedFramebuffer fboScope(mSimFbo[next]);
    gl::ScopedViewport    vpScope(ivec2(0), mSimFbo[next]->getSize());
    gl::ScopedMatrices    matScope;
    gl::setMatrices(CameraOrtho(-1, 1, -1, 1, -1, 1));
    gl::ScopedDepth       depthScope(false);
    gl::ScopedBlend       blendOff(false);   // compute pass — blending must be off

    gl::ScopedTextureBind texScope(mSimFbo[mCurrent]->getColorTexture(), 0);
    gl::ScopedGlslProg    shaderScope(mDropShader);

    mDropShader->uniform("uHeightTex", 0);
    mDropShader->uniform("uCenter",    vec2(x, y));
    mDropShader->uniform("uRadius",    radius);
    mDropShader->uniform("uStrength",  strength);

    renderFullQuad(mDropShader);

    mCurrent = next;
}

void WaterSim::addCymaticDrops(const std::vector<float>& bands, float amplitude) {
    if (!mInitialized) return;
    if (!sWalkersInit) initWalkers();

    static float timers[13] = {};
    static float lastTime   = 0.0f;
    float now = (float)app::getElapsedSeconds();
    float dt  = now - lastTime;
    lastTime  = now;
    if (dt > 0.1f) dt = 0.1f;

    int numBands = (int)std::min((int)bands.size(), 13);

    // Advance walkers — speed scales with overall amplitude so loud audio moves faster
    float baseSpeed = 0.04f + amplitude * 0.12f;
    for (int i = 0; i < numBands; i++) {
        CymaticWalker& w = sWalkers[i];

        // Gentle random steering each frame
        float steer = ci::randFloat(-0.8f, 0.8f);
        float cs = std::cos(steer * dt), sn = std::sin(steer * dt);
        w.vel = vec2(cs * w.vel.x - sn * w.vel.y,
                     sn * w.vel.x + cs * w.vel.y);

        // Maintain target speed
        float spd = glm::length(w.vel);
        if (spd > 1e-5f) w.vel = (w.vel / spd) * baseSpeed;

        w.pos += w.vel * dt;

        // Constrain to a circle of cymaticRadius around center (0.5, 0.5)
        vec2  toCenter = vec2(0.5f, 0.5f) - w.pos;
        float dist     = glm::length(toCenter);
        if (dist > cymaticRadius) {
            w.pos = vec2(0.5f, 0.5f) - (toCenter / dist) * cymaticRadius;
            // Reflect velocity inward
            vec2 n = toCenter / dist;
            w.vel = w.vel - 2.0f * glm::dot(w.vel, -n) * (-n);
        }
    }

    for (int i = 0; i < numBands; i++) {
        timers[i] -= dt;
        if (timers[i] > 0.0f) continue;

        float band     = glm::clamp(std::abs(bands[i]), 0.0f, 1.0f);
        float strength = band * amplitude * cymaticGain;
        if (strength > 0.004f) {
            float t      = (float)i / 12.0f;
            float radius = glm::mix(0.045f, 0.020f, t);
            addDrop(sWalkers[i].pos.x, sWalkers[i].pos.y, radius, strength);
            timers[i] = 0.05f + (1.0f - glm::clamp(strength * 6.0f, 0.0f, 1.0f)) * 0.25f;
        } else {
            timers[i] = 0.05f;
        }
    }
}

gl::TextureRef WaterSim::getHeightTexture() {
    if (!mInitialized) return nullptr;
    return mSimFbo[mCurrent]->getColorTexture();
}

float WaterSim::readHeightAt(float u, float v) {
    if (!mInitialized) return 0.0f;
    int px = (int)(u * mSize);
    int py = (int)(v * mSize);
    px = glm::clamp(px, 0, mSize - 1);
    py = glm::clamp(py, 0, mSize - 1);
    float pixel[4] = {};
    gl::ScopedFramebuffer fboScope(mSimFbo[mCurrent]);
    glReadPixels(px, py, 1, 1, GL_RGBA, GL_FLOAT, pixel);
    return pixel[0];  // R = height
}
