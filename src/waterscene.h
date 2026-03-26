#pragma once
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"
#include "watersim.h"
#include <vector>
#include <array>

using namespace ci;

// Standalone pool scene: water surface + pool geometry + GPU caustics.
// Independent of the CA world — owns its own WaterSim.
class WaterScene {
public:
    WaterScene()  = default;
    ~WaterScene() = default;

    // poolSize  : half-extent of pool in world units (water surface ±poolSize in XZ)
    // simRes    : water simulation FBO resolution (default 256)
    // causticRes: caustic projection FBO resolution (default 512)
    // meshRes   : subdivisions for caustic / surface meshes (default 128)
    void setup(float poolSize = 8.0f, int simRes = 256,
               int causticRes = 512, int meshRes = 128);
    void update();
    // cam is ignored — WaterScene uses its own dedicated camera.
    // Call draw() with no argument or any CameraPersp.
    void draw(const CameraPersp& cam = CameraPersp());
    void reshape(float aspectRatio);  // call on window resize

    // Drop in world XZ coordinates
    void addDrop(float worldX, float worldZ, float radius, float strength);
    void addCymaticDrops(const std::vector<float>& bands, float amplitude);

    bool isReady() const { return mInitialized; }

    // --- Visible knobs ---
    bool  isVisible       = false;

    // Pool geometry
    float poolSize        = 8.0f;
    vec3  poolColor       = { 0.06f, 0.07f, 0.10f };
    float poolAmbient     = 0.18f;

    // Water surface
    vec3  waterColor      = { 0.10f, 0.38f, 0.65f };
    float waterAlpha      = 0.60f;
    float heightScale     = 0.40f;   // visual displacement of surface mesh
    bool  drawSurface     = true;
    bool  drawPool        = true;

    // Caustics
    float causticStrength = 0.9f;
    float causticScale    = 1.2f;    // oldArea/newArea multiplier
    float normalScale     = 8.0f;    // height gradient → surface normal magnitude

    // WaterSim passthrough
    bool  cymatics  = false;
    float damping   = 0.995f;
    float waveSpeed = 2.0f;

private:
    bool        mInitialized = false;
    float       mPoolSize    = 8.0f;
    CameraPersp mPoolCam;             // dedicated camera, angled above the pool
    int   mMeshRes     = 128;
    int   mCausticRes  = 512;

    WaterSim mWaterSim;

    // Caustic projection (top-down, additive)
    gl::FboRef      mCausticFbo;
    gl::GlslProgRef mCausticShader;
    gl::VboMeshRef  mCausticMesh;   // UV grid, ciTexCoord0
    gl::BatchRef    mCausticBatch;

    // Pool geometry (floor + 4 walls)
    gl::GlslProgRef              mPoolShader;
    gl::VboMeshRef               mFloorMesh;
    gl::BatchRef                 mFloorBatch;
    std::array<gl::VboMeshRef,4> mWallMeshes;
    std::array<gl::BatchRef, 4>  mWallBatches;

    // Water surface
    gl::GlslProgRef mSurfaceShader;
    gl::VboMeshRef  mSurfaceMesh;   // UV grid, ciTexCoord0
    gl::BatchRef    mSurfaceBatch;

    // ---- build helpers ----
    gl::VboMeshRef buildUVGridMesh(int N);      // (N+1)^2 verts, ciTexCoord0
    void buildFloorMesh();
    void buildWallMesh(int index);

    void renderCausticFbo();
    void drawPoolGeometry(const CameraPersp& cam);
    void drawWaterSurface(const CameraPersp& cam);
};
