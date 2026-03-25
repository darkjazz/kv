#include "watersim.h"
#include "cinder/app/App.h"

using namespace ci;
using namespace ci::app;

// Cymatics: 13 MFCC bands mapped to fixed positions on the water grid.
// Arranged as 1 centre + 4 inner cardinal + 8 outer octagonal.
static const vec2 CYMATICS_POS[13] = {
    { 0.50f, 0.50f },  // 0 — centre (overall energy)
    { 0.75f, 0.50f },  // 1 — inner E
    { 0.50f, 0.75f },  // 2 — inner N
    { 0.25f, 0.50f },  // 3 — inner W
    { 0.50f, 0.25f },  // 4 — inner S
    { 0.75f, 0.75f },  // 5 — outer NE
    { 0.50f, 0.85f },  // 6 — outer N
    { 0.25f, 0.75f },  // 7 — outer NW
    { 0.15f, 0.50f },  // 8 — outer W
    { 0.25f, 0.25f },  // 9 — outer SW
    { 0.50f, 0.15f },  // 10 — outer S
    { 0.75f, 0.25f },  // 11 — outer SE
    { 0.85f, 0.50f },  // 12 — outer E
};

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

        // Clear both to zero (calm water)
        for (int i = 0; i < 2; i++) {
            gl::ScopedFramebuffer fboScope(mSimFbo[i]);
            gl::clear(Color(0, 0, 0));
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

    static float timers[13] = {};
    static float lastTime   = 0.0f;
    float now = (float)app::getElapsedSeconds();
    float dt  = now - lastTime;
    lastTime  = now;
    if (dt > 0.1f) dt = 0.1f;  // clamp on first frame / lag spike

    int numBands = (int)std::min((int)bands.size(), 13);

    for (int i = 0; i < numBands; i++) {
        timers[i] -= dt;
        if (timers[i] > 0.0f) continue;

        float strength = bands[i] * amplitude * 0.04f;
        if (strength > 0.004f) {
            // Drop radius: bass bands bigger, treble bands smaller
            float t      = (float)i / 12.0f;
            float radius = glm::mix(0.045f, 0.020f, t);
            addDrop(CYMATICS_POS[i].x, CYMATICS_POS[i].y, radius, strength);
            // Cooldown: louder band fires more often
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
