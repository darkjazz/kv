#pragma once
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"
#include "watersim.h"
#include <vector>
#include <array>
#include <mutex>

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
    void addCornerDrops(float sub, float low, float mid, float high);
    void setBoundaryValues(float sub, float low, float mid, float high);


    bool isReady() const { return mInitialized; }

    // --- Visible knobs ---
    bool  isVisible       = false;

    // Pool geometry
    float poolSize        = 8.0f;
    vec3  poolColor       = { 0.06f, 0.07f, 0.10f };
    float poolAmbient     = 0.35f;

    // Water surface
    vec3  waterColor      = { 0.0f, 0.6f, 1.0f };
    float waterAlpha      = 0.72f;
    float heightScale     = 2.0f;    // visual displacement of surface mesh
    bool  drawSurface     = true;
    bool  drawPool        = true;

    // Caustics
    float causticStrength = 0.45f;
    float causticScale    = 1.2f;    // oldArea/newArea multiplier
    float normalScale     = 8.0f;    // height gradient → surface normal magnitude

    // Camera — updated every frame in draw()
    vec3  cameraEye    = { 0.0f, 11.2f, 14.4f };
    vec3  cameraTarget = { 0.0f, -2.0f,  0.0f };

    // Caustic light direction (world space, need not be normalised)
    vec3  lightDir  = { 0.2f, -1.0f, 0.2f };

    // Diagnostic: fire a drop every 2s so waves are visible without OSC
    bool  autoDrop  = false;

    // WaterSim passthrough
    bool  cymatics      = false;
    bool  cornerMode    = false;  // continuous boundary forcing at 4 corners from audio bands
    float damping       = 0.995f;
    float waveSpeed     = 0.5f;   // 2.0 was at the stability boundary → Nyquist oscillation
    float cymaticJitter = 0.5f;   // spatial randomness around walker positions
    float cymaticGain   = 0.2f;   // drop strength multiplier
    float cymaticRadius = 0.15f;  // max distance walkers roam from center (0=center, 0.45=full pool)

private:
    bool        mInitialized = false;
    float       mPoolSize    = 8.0f;
    CameraPersp mPoolCam;             // dedicated camera, angled above the pool
    int   mMeshRes     = 128;
    int   mCausticRes  = 512;

    WaterSim mWaterSim;

    // Thread-safe drop queue (OSC callbacks run on background thread)
    struct PendingDrop { float worldX, worldZ, radius, strength; };
    std::vector<PendingDrop> mPendingDrops;
    std::mutex               mDropMutex;

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
