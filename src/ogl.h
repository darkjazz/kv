/*
 *  ogl.h
 *  lambda
 *
 *  Created by alo on 22/04/2011.
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
#ifndef OGL_H
#define OGL_H

#include "cinder/app/App.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"
#include "cinder/Surface.h"
#include "cinder/gl/Texture.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Rand.h"
#include "cinder/Color.h"
#include "cinder/GeomIo.h"

#include "world.h"
#include "pattern.h"
#include "boidpattern.h"

#include <vector>
#include <memory>

using namespace ci;
using namespace ci::app;
using namespace std;

const int numPatterns = 23;
const int numBoidPatterns = 3;

// Legacy struct for boid patterns (kept for compatibility)
struct boidPattern {
    bool active;
    int mapIndex;
    boidPattern(): active(false), mapIndex(0) {};
    ~boidPattern() {};
};


class GraphicsRenderer {

public:
	GraphicsRenderer(World* world) {
		// Initialize patterns using factory
		for (int i = 0; i < numPatterns; i++) {
			mPatterns.push_back(PatternFactory::create(i));
		}
		// Initialize boid patterns using factory
		for (int i = 0; i < numBoidPatterns; i++) {
			mBoidPatterns.push_back(BoidPatternFactory::create(i));
		}
        boidPatternLib = new boidPattern[numBoidPatterns];
		rotateXYZ = vec3( 1.0f, 0.0f, 0.0f);
		rotateAngle = 0.0f;
		ptrWorld = world;
		attachEyeToFirstBoid = false;
		lookAtCentroid = false;
		counter = 0;
		maxphase = 28;
		ptrBMU = NULL;
		mDirectional = 1.0f;
		mLightLoc = vec3(0.0f, 0.0f, 0.0f);
		blocx = 0.0f;
		blocy = 0.0f;
		blocz = 0.0f;
		mAudioAmplitude = 0.0f;
		mAudioLowBand = 0.0f;
		mAudioMidBand = 0.0f;
		mAudioHighBand = 0.0f;
	};
	
	~GraphicsRenderer() {
		delete [] boidPatternLib;
	};

	// New pattern system
	std::vector<std::unique_ptr<Pattern>> mPatterns;
	std::vector<std::unique_ptr<BoidPattern>> mBoidPatterns;

	// Legacy accessor for OSC compatibility
	Pattern* getPattern(int id) {
		if (id >= 0 && id < mPatterns.size()) {
			return mPatterns[id].get();
		}
		return nullptr;
	}

	// Boid pattern accessor
	BoidPattern* getBoidPattern(int id) {
		if (id >= 0 && id < mBoidPatterns.size()) {
			return mBoidPatterns[id].get();
		}
		return nullptr;
	}

    boidPattern* boidPatternLib;

	void setupOgl();
		
	void reshape();
		
	void startDraw();
	
	void endDraw();
	
	void drawFragment(Cell*);

	void update();

	// Boid rendering
	void drawBoids();

	// Instance rendering methods
	void clearInstanceData();
	void addCubeInstance(const vec3& position, const ColorA& color, const vec3& scale, const gl::TextureRef& texture = nullptr);
	void addSphereInstance(const vec3& position, const ColorA& color, float radius, int patternId = -1);
	void addCylinderInstance(const vec3& position, const ColorA& color, const vec3& scale, const quat& rotation);
	void addLineInstance(const vec3& start, const vec3& end, const ColorA& color, float width = 1.0f);
	void addSphericalQuad(float theta, float phi, float rho, const ivec2& gridPos, const ColorA cornerColors[4], float cellState = 1.0f);
	void addPlaneInstance(const vec3& position, const vec3& size, const ColorA& color, int planeType, bool wireframe);
	void addPointInstance(const vec3& position, const ColorA& color, float size = 1.0f);
	void drawCubeInstances();
	void drawSphereInstances();
	void drawCylinderInstances();
	void drawLineInstances();
	void drawSphericalQuads();
	void drawPlaneInstances();
	void addPolygonInstance(const vec3 vertices[4], const ColorA colors[4]);
	void drawPolygonInstances();
	void drawPointInstances();
	void addTriangleInstance(const vec3& v0, const vec3& v1, const vec3& v2,
	                         const ColorA& c0, const ColorA& c1, const ColorA& c2);
	void drawTriangleInstances();

	void setBackground(float r, float g, float b) {
		_bgr = r; _bgg = g; _bgb = b;
	};
	
    vec3 rotateXYZ;
	
	float rotateAngle;
	
	CameraPersp mCam;	
    mat4 mRotation;
	
	vec3 mEye, mCenter, mUp;
	
	float mDirectional;
	vec2 mMousePos;
			
	bool attachEyeToFirstBoid;
	bool lookAtCentroid;

    Boids* boids;

    vec3 mLightLoc;
	bool bLIGHT;
    int counter;

	// Audio reactivity from SOM vector
	float mAudioAmplitude;    // Overall energy (sum of MFCC coefficients)
	float mAudioLowBand;      // Low frequency band (MFCC 0-4)
	float mAudioMidBand;      // Mid frequency band (MFCC 5-9)
	float mAudioHighBand;     // High frequency band (MFCC 10+)

	void updateAudioFeatures();  // Extract features from SOM vector

private:

	double fragSizeX, fragSizeY, fragSizeZ, state;
	float xL, yB, zF, xW, yH, zD, red, green, blue, alpha, maxphase;
	int currentIndex, vectorSize;
	Cell* currentCell;
	Cell* ptrBMU;
	World* ptrWorld;
	float _bgr, _bgg, _bgb;
	float hx, hy;
	float blocx, blocy, blocz;
	float mLastTime = 0.0f;

    gl::VertBatchRef    mGrid;

	GLfloat *rowVertices, *worldVertices, *rowNormals, *worldNormals, *rowColors, *worldColors;

	// Instance data collections
	std::vector<vec3> mCubePositions;
	std::vector<ColorA> mCubeColors;
	std::vector<vec3> mCubeScales;
	std::vector<gl::TextureRef> mCubeTextures;

	std::vector<vec3> mSpherePositions;
	std::vector<ColorA> mSphereColors;
	std::vector<float> mSphereRadii;
	std::vector<int> mSpherePatternIds;  // Track which pattern each sphere belongs to

	std::vector<vec3> mCylinderPositions;
	std::vector<ColorA> mCylinderColors;
	std::vector<vec3> mCylinderScales;
	std::vector<quat> mCylinderRotations;

	std::vector<vec3> mLineStarts;
	std::vector<vec3> mLineEnds;
	std::vector<ColorA> mLineColors;
	std::vector<float> mLineWidths;

	// Plane instance data (for Pattern08)
	struct PlaneInstanceData {
		vec3 position;  // Corner position
		vec3 size;      // Width, height, depth
		ColorA color;
		int planeType;  // 0=XY, 1=YZ, 2=XZ
		bool wireframe; // true for wireframe, false for filled
	};
	std::vector<PlaneInstanceData> mPlaneInstances;

	// Polygon instance data (for Pattern09)
	struct PolygonInstanceData {
		vec3 vertices[4];     // 4 vertices in spherical coordinates
		ColorA colors[4];     // Per-vertex colors
	};
	std::vector<PolygonInstanceData> mPolygonInstances;

	// Spherical quad data
	struct SphericalQuadData {
		float theta, phi, rho;
		ivec2 gridPos;
		ColorA cornerColors[4];
		float cellState;  // Cell state value for size scaling
	};
	std::vector<SphericalQuadData> mSphericalQuads;

	// Point instance data (for Pattern15)
	std::vector<vec3> mPointPositions;
	std::vector<ColorA> mPointColors;
	std::vector<float> mPointSizes;

	// Triangle instance data (for Pattern20 - filled neighbor fans)
	std::vector<vec3> mTriangleVertices;
	std::vector<ColorA> mTriangleColors;

	// Instance rendering VBOs
	gl::VboRef mCubeInstanceVbo;
	gl::VboMeshRef mCubeMesh;
	gl::BatchRef mCubeBatch;

	gl::VboRef mSphereInstanceVbo;
	gl::VboMeshRef mSphereMesh;
	gl::BatchRef mSphereBatch;

	gl::VboRef mCylinderInstanceVbo;
	gl::VboMeshRef mCylinderMesh;
	gl::BatchRef mCylinderBatch;

	gl::GlslProgRef mInstanceShader;
	gl::GlslProgRef mInstanceTexturedShader;
	gl::GlslProgRef mEnvMapShader;

	gl::VboRef mLineInstanceVbo;
	gl::VboMeshRef mLineMesh;
	gl::BatchRef mLineBatch;
	gl::GlslProgRef mLineShader;

	gl::GlslProgRef mSphericalQuadShader;

	gl::TextureCubeMapRef mCubeMap;   // fxic_* cubemap for Pattern05
	gl::TextureCubeMapRef mCubeMap2;  // fxp_* cubemap for Pattern13
	

	// pattern00 removed - now using new pattern system in pattern.cpp
	// pattern01 removed - now using new pattern system in pattern.cpp
	
	void pattern02(int, int, int);
	
	void pattern03(int, int, int);

	void pattern04(int, int, int);
	
	void pattern05(int, int, int);
	
	// *** basic drawing functions *** //

	void fillRect (int);

    void drawEdges(const std::vector<vec3>&);
    // strokeRect removed - pattern00 now uses instanced line rendering

    // Pattern08 plane drawing helpers
    void drawPlaneFilled(float xL, float yB, float zF, float xW, float yH, float zD, int planeType, const ColorA& color);
    void drawPlaneWireframe(float xL, float yB, float zF, float xW, float yH, float zD, int planeType, const ColorA& color);

};

#endif
