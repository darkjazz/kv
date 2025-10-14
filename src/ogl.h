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

#include <vector>
#include <memory>

using namespace ci;
using namespace ci::app;
using namespace std;

const int numPatterns = 5;
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
	};
	
	~GraphicsRenderer() {
		delete [] boidPatternLib;
	};

	// New pattern system
	std::vector<std::unique_ptr<Pattern>> mPatterns;

	// Legacy accessor for OSC compatibility
	Pattern* getPattern(int id) {
		if (id >= 0 && id < mPatterns.size()) {
			return mPatterns[id].get();
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

	// Instance rendering methods
	void clearInstanceData();
	void addCubeInstance(const vec3& position, const ColorA& color, const vec3& scale);
	void addSphereInstance(const vec3& position, const ColorA& color, float radius);
	void drawCubeInstances();
	void drawSphereInstances();
	
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

    gl::VertBatchRef    mGrid;

	GLfloat *rowVertices, *worldVertices, *rowNormals, *worldNormals, *rowColors, *worldColors;

	// Instance data collections
	std::vector<vec3> mCubePositions;
	std::vector<ColorA> mCubeColors;
	std::vector<vec3> mCubeScales;

	std::vector<vec3> mSpherePositions;
	std::vector<ColorA> mSphereColors;
	std::vector<float> mSphereRadii;

	// Instance rendering VBOs
	gl::VboRef mCubeInstanceVbo;
	gl::VboMeshRef mCubeMesh;
	gl::BatchRef mCubeBatch;

	gl::VboRef mSphereInstanceVbo;
	gl::VboMeshRef mSphereMesh;
	gl::BatchRef mSphereBatch;
	gl::GlslProgRef mInstanceShader;
	
	
	void pattern00(int, int, int);

	void pattern01(int, int, int);
	
	void pattern02(int, int, int);
	
	void pattern03(int, int, int);

	void pattern04(int, int, int);
	
	void pattern05(int, int, int);
	
	// *** basic drawing functions *** //
	
	void fillRect (int);
	
    void drawEdges(const std::vector<vec3>&);
    void strokeRect(int, float, float, float, float, float);
    
};

#endif
