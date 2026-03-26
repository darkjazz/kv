#include "waterscene.h"
#include "cinder/app/App.h"
#include "cinder/GeomIo.h"

using namespace ci;
using namespace ci::app;

// ---------------------------------------------------------------------------
// Mesh helpers
// ---------------------------------------------------------------------------

// Build an (N+1)×(N+1) UV grid as a triangle mesh.
// Attribute: ciTexCoord0 (geom::TEX_COORD_0) — vec2, row-major.
gl::VboMeshRef WaterScene::buildUVGridMesh(int N) {
    std::vector<vec2> uvs;
    uvs.reserve((N + 1) * (N + 1));
    for (int j = 0; j <= N; j++) {
        for (int i = 0; i <= N; i++) {
            uvs.push_back(vec2(i / (float)N, j / (float)N));
        }
    }

    std::vector<uint32_t> indices;
    indices.reserve(N * N * 6);
    for (int j = 0; j < N; j++) {
        for (int i = 0; i < N; i++) {
            uint32_t tl = (uint32_t)(j * (N + 1) + i);
            uint32_t tr = tl + 1;
            uint32_t bl = tl + (uint32_t)(N + 1);
            uint32_t br = bl + 1;
            indices.push_back(tl); indices.push_back(bl); indices.push_back(tr);
            indices.push_back(tr); indices.push_back(bl); indices.push_back(br);
        }
    }

    auto vbo = gl::Vbo::create(GL_ARRAY_BUFFER,
                               uvs.size() * sizeof(vec2),
                               uvs.data(), GL_STATIC_DRAW);
    auto ibo = gl::Vbo::create(GL_ELEMENT_ARRAY_BUFFER,
                               indices.size() * sizeof(uint32_t),
                               indices.data(), GL_STATIC_DRAW);

    geom::BufferLayout layout;
    layout.append(geom::Attrib::TEX_COORD_0, 2, sizeof(vec2), 0);

    return gl::VboMesh::create(
        (uint32_t)uvs.size(), GL_TRIANGLES,
        { { layout, vbo } },
        (uint32_t)indices.size(), GL_UNSIGNED_INT, ibo
    );
}

// Pool geometry vertex: position + normal packed together.
struct PoolVertex {
    vec3 pos;
    vec3 normal;
};

// Build floor: horizontal quad at Y = -poolSize, XZ in [-P, +P].
void WaterScene::buildFloorMesh() {
    float P = mPoolSize;
    float D = mPoolSize;  // floor at Y = -D

    std::vector<PoolVertex> verts = {
        { { -P, -D, -P }, { 0,1,0 } },
        { {  P, -D, -P }, { 0,1,0 } },
        { {  P, -D,  P }, { 0,1,0 } },
        { { -P, -D,  P }, { 0,1,0 } },
    };
    std::vector<uint32_t> idx = { 0,1,2, 0,2,3 };

    auto vbo = gl::Vbo::create(GL_ARRAY_BUFFER,
                               verts.size() * sizeof(PoolVertex),
                               verts.data(), GL_STATIC_DRAW);
    auto ibo = gl::Vbo::create(GL_ELEMENT_ARRAY_BUFFER,
                               idx.size() * sizeof(uint32_t),
                               idx.data(), GL_STATIC_DRAW);

    geom::BufferLayout layout;
    layout.append(geom::Attrib::POSITION, 3, sizeof(PoolVertex), offsetof(PoolVertex, pos));
    layout.append(geom::Attrib::NORMAL,   3, sizeof(PoolVertex), offsetof(PoolVertex, normal));

    mFloorMesh = gl::VboMesh::create(
        (uint32_t)verts.size(), GL_TRIANGLES,
        { { layout, vbo } },
        (uint32_t)idx.size(), GL_UNSIGNED_INT, ibo
    );
}

// Build one pool wall (index 0-3: +Z, -Z, +X, -X).
void WaterScene::buildWallMesh(int w) {
    float P = mPoolSize;
    float D = mPoolSize;

    std::vector<PoolVertex> verts;
    if (w < 2) {
        // Z walls
        float z  = (w == 0) ? P : -P;
        vec3  N  = (w == 0) ? vec3(0,0,-1) : vec3(0,0,1);
        verts = {
            { { -P, -D, z }, N },
            { {  P, -D, z }, N },
            { {  P,  0, z }, N },
            { { -P,  0, z }, N },
        };
    } else {
        // X walls
        float x  = (w == 2) ? P : -P;
        vec3  N  = (w == 2) ? vec3(-1,0,0) : vec3(1,0,0);
        verts = {
            { { x, -D, -P }, N },
            { { x, -D,  P }, N },
            { { x,  0,  P }, N },
            { { x,  0, -P }, N },
        };
    }
    std::vector<uint32_t> idx = { 0,1,2, 0,2,3 };

    auto vbo = gl::Vbo::create(GL_ARRAY_BUFFER,
                               verts.size() * sizeof(PoolVertex),
                               verts.data(), GL_STATIC_DRAW);
    auto ibo = gl::Vbo::create(GL_ELEMENT_ARRAY_BUFFER,
                               idx.size() * sizeof(uint32_t),
                               idx.data(), GL_STATIC_DRAW);

    geom::BufferLayout layout;
    layout.append(geom::Attrib::POSITION, 3, sizeof(PoolVertex), offsetof(PoolVertex, pos));
    layout.append(geom::Attrib::NORMAL,   3, sizeof(PoolVertex), offsetof(PoolVertex, normal));

    mWallMeshes[w] = gl::VboMesh::create(
        (uint32_t)verts.size(), GL_TRIANGLES,
        { { layout, vbo } },
        (uint32_t)idx.size(), GL_UNSIGNED_INT, ibo
    );
}

// ---------------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------------

void WaterScene::setup(float pSize, int simRes, int causticRes, int meshRes) {
    mPoolSize   = pSize;
    poolSize    = pSize;
    mCausticRes = causticRes;
    mMeshRes    = meshRes;

    // Water simulation
    mWaterSim.setup(simRes);
    if (!mWaterSim.isReady()) {
        console() << "WaterScene: WaterSim setup failed" << std::endl;
        return;
    }

    // Caustic FBO (RGBA32F, no depth)
    {
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
            mCausticFbo = gl::Fbo::create(mCausticRes, mCausticRes, fboFmt);
            gl::ScopedFramebuffer fboScope(mCausticFbo);
            gl::clear(Color(0, 0, 0));
        }
        catch (const std::exception& e) {
            console() << "WaterScene: caustic FBO failed: " << e.what() << std::endl;
            return;
        }
    }

    // Shaders — load each separately so failures are identified precisely
    auto loadShader = [&](const char* vert, const char* frag) -> gl::GlslProgRef {
        try {
            auto prog = gl::GlslProg::create(app::loadAsset(vert), app::loadAsset(frag));
            console() << "WaterScene: loaded " << vert << std::endl;
            return prog;
        }
        catch (const std::exception& e) {
            console() << "WaterScene: FAILED " << vert << " — " << e.what() << std::endl;
            return nullptr;
        }
    };

    mCausticShader = loadShader("water_caustic.vert", "water_caustic.frag");
    mPoolShader    = loadShader("pool.vert",           "pool.frag");
    mSurfaceShader = loadShader("water_surface.vert",  "water_surface.frag");

    if (!mCausticShader || !mPoolShader || !mSurfaceShader) {
        console() << "WaterScene: aborting — shader(s) failed" << std::endl;
        return;
    }

    // Meshes and batches
    try {
        mCausticMesh = buildUVGridMesh(mMeshRes);
        mSurfaceMesh = buildUVGridMesh(mMeshRes);
        buildFloorMesh();
        for (int i = 0; i < 4; i++) buildWallMesh(i);

        mCausticBatch = gl::Batch::create(mCausticMesh, mCausticShader);
        console() << "WaterScene: caustic batch OK" << std::endl;
        mSurfaceBatch = gl::Batch::create(mSurfaceMesh, mSurfaceShader);
        console() << "WaterScene: surface batch OK" << std::endl;
        mFloorBatch   = gl::Batch::create(mFloorMesh,   mPoolShader);
        console() << "WaterScene: floor batch OK" << std::endl;
        for (int i = 0; i < 4; i++) {
            mWallBatches[i] = gl::Batch::create(mWallMeshes[i], mPoolShader);
        }
        console() << "WaterScene: wall batches OK" << std::endl;
    }
    catch (const std::exception& e) {
        console() << "WaterScene: mesh/batch setup failed: " << e.what() << std::endl;
        return;
    }

    // Dedicated camera: positioned above and in front, looking down into the pool
    float aspect = app::getWindowAspectRatio();
    mPoolCam.setPerspective(55.0f, aspect, 0.1f, 500.0f);
    mPoolCam.lookAt(
        vec3(0.0f,  mPoolSize * 1.4f, mPoolSize * 1.8f),   // eye: above-front
        vec3(0.0f, -mPoolSize * 0.25f, 0.0f),              // look at: center of pool
        vec3(0.0f,  1.0f, 0.0f)
    );

    mInitialized = true;
    console() << "WaterScene: ready (pool=" << mPoolSize
              << " caustic=" << mCausticRes << "x" << mCausticRes << ")" << std::endl;
}

void WaterScene::reshape(float aspectRatio) {
    mPoolCam.setAspectRatio(aspectRatio);
}

// ---------------------------------------------------------------------------
// Update
// ---------------------------------------------------------------------------

void WaterScene::update() {
    if (!mInitialized) return;

    // Sync controllable params to the inner sim
    mWaterSim.damping   = damping;
    mWaterSim.waveSpeed = waveSpeed;

    mWaterSim.update();
}

// ---------------------------------------------------------------------------
// Drop interface
// ---------------------------------------------------------------------------

void WaterScene::addDrop(float worldX, float worldZ, float radius, float strength) {
    if (!mInitialized) return;
    // Convert world XZ → simulation UV [0,1]
    float u = worldX / mPoolSize * 0.5f + 0.5f;
    float v = worldZ / mPoolSize * 0.5f + 0.5f;
    // Clamp radius from world units to UV fraction
    float rUV = radius / (2.0f * mPoolSize);
    mWaterSim.addDrop(u, v, rUV, strength);
}

void WaterScene::addCymaticDrops(const std::vector<float>& bands, float amplitude) {
    if (!mInitialized) return;
    mWaterSim.addCymaticDrops(bands, amplitude);
}

// ---------------------------------------------------------------------------
// Caustic FBO render
// ---------------------------------------------------------------------------

void WaterScene::renderCausticFbo() {
    auto heightTex = mWaterSim.getHeightTexture();
    if (!heightTex) return;

    gl::ScopedFramebuffer fboScope(mCausticFbo);
    gl::ScopedViewport    vpScope(ivec2(0), mCausticFbo->getSize());
    gl::ScopedDepth       depthOff(false);
    gl::ScopedMatrices    matScope;

    // Small ambient fill so pool looks lit even on calm water
    gl::clear(ColorA(0.04f, 0.04f, 0.04f, 1.0f));

    // Additive blending: caustic brightness accumulates
    gl::ScopedBlend blend(GL_ONE, GL_ONE);

    gl::ScopedTextureBind texBind(heightTex, 0);
    gl::ScopedGlslProg    shaderScope(mCausticShader);

    mCausticShader->uniform("uHeightTex",   0);
    mCausticShader->uniform("uPoolSize",    mPoolSize);
    mCausticShader->uniform("uNormalScale", normalScale);
    mCausticShader->uniform("uCausticScale", causticScale);

    mCausticBatch->draw();
}

// ---------------------------------------------------------------------------
// Draw
// ---------------------------------------------------------------------------

void WaterScene::drawPoolGeometry(const CameraPersp& cam) {
    auto causticTex = mCausticFbo->getColorTexture();
    if (!causticTex) return;

    gl::ScopedTextureBind texBind(causticTex, 0);
    gl::ScopedGlslProg    shaderScope(mPoolShader);
    gl::ScopedDepth       depthOn(true);

    mPoolShader->uniform("uCausticTex",     0);
    mPoolShader->uniform("uPoolColor",      poolColor);
    mPoolShader->uniform("uPoolSize",       mPoolSize);
    mPoolShader->uniform("uCausticStrength", causticStrength);
    mPoolShader->uniform("uAmbient",        poolAmbient);

    mFloorBatch->draw();
    for (int i = 0; i < 4; i++) {
        mWallBatches[i]->draw();
    }
}

void WaterScene::drawWaterSurface(const CameraPersp& cam) {
    auto heightTex = mWaterSim.getHeightTexture();
    if (!heightTex) return;

    gl::ScopedTextureBind texBind(heightTex, 0);
    gl::ScopedGlslProg    shaderScope(mSurfaceShader);
    gl::ScopedDepth       depthOn(true);
    gl::ScopedBlend       blend(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    mSurfaceShader->uniform("uHeightTex",   0);
    mSurfaceShader->uniform("uPoolSize",    mPoolSize);
    mSurfaceShader->uniform("uHeightScale", heightScale);
    mSurfaceShader->uniform("uNormalScale", normalScale);
    mSurfaceShader->uniform("uWaterColor",  waterColor);
    mSurfaceShader->uniform("uAlpha",       waterAlpha);
    mSurfaceShader->uniform("uCameraPos",   cam.getEyePoint());

    mSurfaceBatch->draw();
}

void WaterScene::draw(const CameraPersp&) {
    if (!mInitialized || !isVisible) return;

    // Step 1: project refracted light to caustic FBO (no camera needed)
    renderCausticFbo();

    // Step 2: clear the screen and draw pool with the dedicated pool camera.
    // This replaces the CA world view when the pool is active.
    gl::clear(Color(0, 0, 0));
    gl::ScopedDepth depthScope(true);

    gl::ScopedMatrices matScope;
    gl::setMatrices(mPoolCam);

    if (drawPool)    drawPoolGeometry(mPoolCam);
    if (drawSurface) drawWaterSurface(mPoolCam);
}
