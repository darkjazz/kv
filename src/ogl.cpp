/*
 *  ogl.cpp
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

#include "ogl.h"

void GraphicsRenderer::setupOgl () {

	_bgr = _bgg = _bgb = 0.0;

	gl::clear( Color( _bgr, _bgg, _bgb ) );

	mEye = vec3(0.0f, 0.0f, -30.0f);
	mCenter = vec3(0.0f, 0.0f, 0.0f);
	mUp = vec3(0.0f, 1.0f, 0.0f);

	mCam.lookAt(mEye, mCenter, mUp);

//	glEnable( GL_TEXTURE_2D );
//	glDisable( GL_TEXTURE_2D );

	gl::enableDepthRead();
	gl::enableDepthWrite();
	gl::enableAlphaBlending();

	// Load instance rendering shader
	try {
		auto vertPath = app::loadAsset("instance.vert");
		auto fragPath = app::loadAsset("instance.frag");
		console() << "Loading instance shaders from assets..." << std::endl;
		mInstanceShader = gl::GlslProg::create(vertPath, fragPath);
		console() << "Instance shader loaded successfully" << std::endl;
	}
	catch (const std::exception& e) {
		console() << "Error loading instance shader: " << e.what() << std::endl;
	}

	// Load textured instance rendering shader
	try {
		auto vertPath = app::loadAsset("instance_textured.vert");
		auto fragPath = app::loadAsset("instance_textured.frag");
		console() << "Loading textured instance shaders from assets..." << std::endl;
		mInstanceTexturedShader = gl::GlslProg::create(vertPath, fragPath);
		console() << "Textured instance shader loaded successfully" << std::endl;
	}
	catch (const std::exception& e) {
		console() << "Error loading textured instance shader: " << e.what() << std::endl;
	}

	// Load environment map shader
	try {
		auto vertPath = app::loadAsset("envmap_sphere.vert");
		auto fragPath = app::loadAsset("envmap_sphere.frag");
		console() << "Loading environment map shaders from assets..." << std::endl;
		mEnvMapShader = gl::GlslProg::create(vertPath, fragPath);
		console() << "Environment map shader loaded successfully" << std::endl;
	}
	catch (const std::exception& e) {
		console() << "Error loading environment map shader: " << e.what() << std::endl;
	}

	// Load cubemap textures
	try {
		console() << "Loading cubemap textures..." << std::endl;

		// Load each face - order: +X, -X, +Y, -Y, +Z, -Z
		ImageSourceRef images[6];
		images[0] = loadImage(app::loadAsset("fxic_pos_x.png"));
		images[1] = loadImage(app::loadAsset("fxic_neg_x.png"));
		images[2] = loadImage(app::loadAsset("fxic_pos_y.png"));
		images[3] = loadImage(app::loadAsset("fxic_neg_y.png"));
		images[4] = loadImage(app::loadAsset("fxic_pos_z.png"));
		images[5] = loadImage(app::loadAsset("fxic_neg_z.png"));

		gl::TextureCubeMap::Format fmt;
		fmt.setMagFilter(GL_LINEAR);
		fmt.setMinFilter(GL_LINEAR);
		fmt.setWrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

		// Create cubemap from image array
		mCubeMap = gl::TextureCubeMap::create(images, fmt);
		console() << "Cubemap loaded successfully" << std::endl;
	}
	catch (const std::exception& e) {
		console() << "Error loading cubemap: " << e.what() << std::endl;
	}

	// Load line rendering shader
	try {
		auto vertPath = app::loadAsset("line.vert");
		auto fragPath = app::loadAsset("line.frag");
		console() << "Loading line shaders from assets..." << std::endl;
		mLineShader = gl::GlslProg::create(vertPath, fragPath);
		console() << "Line shader loaded successfully" << std::endl;
	}
	catch (const std::exception& e) {
		console() << "Error loading line shader: " << e.what() << std::endl;
		// Try loading from Resources directory directly
		try {
			console() << "Trying to load shaders from source location..." << std::endl;
			auto vertSource = ci::DataSourcePath::create(getAppPath() / ".." / "Resources" / "line.vert");
			auto fragSource = ci::DataSourcePath::create(getAppPath() / ".." / "Resources" / "line.frag");
			mLineShader = gl::GlslProg::create(vertSource, fragSource);
			console() << "Line shader loaded from Resources" << std::endl;
		}
		catch (const std::exception& e2) {
			console() << "Failed to load line shader from Resources: " << e2.what() << std::endl;
		}
	}

	// Load spherical quad shader
	try {
		auto vertPath = app::loadAsset("spherical_quad.vert");
		auto fragPath = app::loadAsset("spherical_quad.frag");
		console() << "Loading spherical quad shaders from assets..." << std::endl;
		mSphericalQuadShader = gl::GlslProg::create(vertPath, fragPath);
		console() << "Spherical quad shader loaded successfully" << std::endl;
	}
	catch (const std::exception& e) {
		console() << "Error loading spherical quad shader: " << e.what() << std::endl;
	}

	// Create cube mesh geometry with texture coordinates
	auto cubeGeom = geom::Cube().size(vec3(1.0f));
	mCubeMesh = gl::VboMesh::create(cubeGeom);

	// Check if mesh has texture coordinates by checking the geom source
	console() << "Cube geometry created - checking for texture coordinates..." << std::endl;
	if (cubeGeom.getAvailableAttribs().count(geom::Attrib::TEX_COORD_0) > 0) {
		console() << "  Cube mesh HAS texture coordinates!" << std::endl;
	} else {
		console() << "  WARNING: Cube mesh does NOT have texture coordinates!" << std::endl;
	}

	// Create sphere mesh geometry
	mSphereMesh = gl::VboMesh::create(geom::Sphere().subdivisions(16));

	// Create cylinder mesh geometry
	mCylinderMesh = gl::VboMesh::create(geom::Cylinder().subdivisionsAxis(16).subdivisionsHeight(1));

	// Create line mesh geometry (simple 2-vertex line)
	// First vertex has x=0 (start), second has x=1 (end)
	std::vector<float> lineVerts = {
		0.0f, 0.0f, 0.0f,  // First vertex (t=0)
		1.0f, 0.0f, 0.0f   // Second vertex (t=1)
	};
	auto lineGeom = geom::BufferLayout();
	lineGeom.append(geom::Attrib::POSITION, 3, sizeof(float) * 3, 0);
	auto lineVbo = gl::Vbo::create(GL_ARRAY_BUFFER, lineVerts.size() * sizeof(float), lineVerts.data(), GL_STATIC_DRAW);
	mLineMesh = gl::VboMesh::create(2, GL_LINES, { {lineGeom, lineVbo} });

}

void GraphicsRenderer::reshape() {

	mCam.setPerspective(45.0, getWindowAspectRatio(), 0.1f, 2000.0f);
	gl::setMatrices( mCam );

}

void GraphicsRenderer::updateAudioFeatures() {
	// Extract audio features from the SOM input vector via BMU activity
	if (!ptrWorld || !ptrWorld->initialized()) {
		mAudioAmplitude = 0.0f;
		mAudioLowBand = 0.0f;
		mAudioMidBand = 0.0f;
		mAudioHighBand = 0.0f;
		return;
	}

	// Use the current BMU (Best Matching Unit) as a proxy for audio energy
	// The BMU is updated when new MFCC vectors are sent via OSC
	float totalEnergy = 0.0f;
	float lowEnergy = 0.0f;
	float midEnergy = 0.0f;
	float highEnergy = 0.0f;

	if (ptrWorld->currentBMU()) {
		// Use BMU cell phase as energy indicator
		float bmuState = ptrWorld->currentBMU()->phase;
		totalEnergy = bmuState;

		// Map BMU spatial position to frequency bands
		// Lower X values = low frequencies, higher X = high frequencies
		int bmuX = ptrWorld->currentBMU()->x;
		int bmuY = ptrWorld->currentBMU()->y;
		int bmuZ = ptrWorld->currentBMU()->z;

		// Normalize positions to 0-1 range
		float normX = (float)bmuX / std::max(1, ptrWorld->sizeX() - 1);
		float normY = (float)bmuY / std::max(1, ptrWorld->sizeY() - 1);
		float normZ = (float)bmuZ / std::max(1, ptrWorld->sizeZ() - 1);

		// Map to frequency bands (weighted by overall energy)
		lowEnergy = (1.0f - normX) * bmuState;  // Low X = low freq
		midEnergy = (1.0f - normY) * bmuState;  // Mid range
		highEnergy = normZ * bmuState;          // High Z = high freq
	}

	// Smooth the values with exponential moving average for subtle, gradual changes
	float smoothing = 0.85f;  // High smoothing (0-1, higher = smoother)
	mAudioAmplitude = mAudioAmplitude * smoothing + totalEnergy * (1.0f - smoothing);
	mAudioLowBand = mAudioLowBand * smoothing + lowEnergy * (1.0f - smoothing);
	mAudioMidBand = mAudioMidBand * smoothing + midEnergy * (1.0f - smoothing);
	mAudioHighBand = mAudioHighBand * smoothing + highEnergy * (1.0f - smoothing);

	// Normalize to 0-1 range
	mAudioAmplitude = std::min(1.0f, std::max(0.0f, mAudioAmplitude));
	mAudioLowBand = std::min(1.0f, std::max(0.0f, mAudioLowBand));
	mAudioMidBand = std::min(1.0f, std::max(0.0f, mAudioMidBand));
	mAudioHighBand = std::min(1.0f, std::max(0.0f, mAudioHighBand));
}

void GraphicsRenderer::update() {
	// Update audio features from SOM BMU
	updateAudioFeatures();

	// Update pattern animations
	float currentTime = static_cast<float>(app::getElapsedSeconds());
	float dt = currentTime - mLastTime;
	mLastTime = currentTime;

	for (auto& pattern : mPatterns) {
		if (pattern) {
			pattern->update(currentTime, dt);
		}
	}

	if (ptrWorld->initialized()) {

		fragSizeX = (double)(getWindowWidth() / ptrWorld->sizeX()) * 0.1;
		fragSizeY = (double)(getWindowHeight() / ptrWorld->sizeY()) * 0.1;
		fragSizeZ = (double)(getWindowWidth() / ptrWorld->sizeZ()) * 0.1;

		hx = fragSizeX * ptrWorld->sizeX() * 0.5;
		hy = fragSizeY * ptrWorld->sizeY() * 0.5;
	}

    mRotation = glm::rotate(mRotation, glm::radians(rotateAngle), rotateXYZ);

	mCam.lookAt( mEye, mCenter, mUp );
	gl::setMatrices( mCam );

    gl::enableAdditiveBlending();
    glEnable( GL_MULTISAMPLE_ARB );
    glHint (GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);

    gl::clear( Color( _bgr, _bgg, _bgb ) );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

}

void GraphicsRenderer::clearInstanceData() {
	mCubePositions.clear();
	mCubeColors.clear();
	mCubeScales.clear();
	mCubeTextures.clear();

	mSpherePositions.clear();
	mSphereColors.clear();
	mSphereRadii.clear();
	mSpherePatternIds.clear();

	mCylinderPositions.clear();
	mCylinderColors.clear();
	mCylinderScales.clear();
	mCylinderRotations.clear();

	mLineStarts.clear();
	mLineEnds.clear();
	mLineColors.clear();
	mLineWidths.clear();

	mPlaneInstances.clear();
	mPolygonInstances.clear();
	mSphericalQuads.clear();
}

void GraphicsRenderer::addCubeInstance(const vec3& position, const ColorA& color, const vec3& scale, const gl::TextureRef& texture) {
	mCubePositions.push_back(position);
	mCubeColors.push_back(color);
	mCubeScales.push_back(scale);
	mCubeTextures.push_back(texture);
}

void GraphicsRenderer::addSphereInstance(const vec3& position, const ColorA& color, float radius, int patternId) {
	mSpherePositions.push_back(position);
	mSphereColors.push_back(color);
	mSphereRadii.push_back(radius);
	mSpherePatternIds.push_back(patternId);
}

void GraphicsRenderer::addCylinderInstance(const vec3& position, const ColorA& color, const vec3& scale, const quat& rotation) {
	mCylinderPositions.push_back(position);
	mCylinderColors.push_back(color);
	mCylinderScales.push_back(scale);
	mCylinderRotations.push_back(rotation);
}

void GraphicsRenderer::addLineInstance(const vec3& start, const vec3& end, const ColorA& color, float width) {
	mLineStarts.push_back(start);
	mLineEnds.push_back(end);
	mLineColors.push_back(color);
	mLineWidths.push_back(width);
}

void GraphicsRenderer::addSphericalQuad(float theta, float phi, float rho, const ivec2& gridPos, const ColorA cornerColors[4], float cellState) {
	SphericalQuadData quad;
	quad.theta = theta;
	quad.phi = phi;
	quad.rho = rho;
	quad.gridPos = gridPos;
	quad.cellState = cellState;
	for (int i = 0; i < 4; i++) {
		quad.cornerColors[i] = cornerColors[i];
	}
	mSphericalQuads.push_back(quad);
}

void GraphicsRenderer::addPlaneInstance(const vec3& position, const vec3& size, const ColorA& color, int planeType, bool wireframe) {
	PlaneInstanceData plane;
	plane.position = position;
	plane.size = size;
	plane.color = color;
	plane.planeType = planeType;
	plane.wireframe = wireframe;
	mPlaneInstances.push_back(plane);
}

void GraphicsRenderer::addPolygonInstance(const vec3 vertices[4], const ColorA colors[4]) {
	PolygonInstanceData polygon;
	for (int i = 0; i < 4; i++) {
		polygon.vertices[i] = vertices[i];
		polygon.colors[i] = colors[i];
	}
	mPolygonInstances.push_back(polygon);
}

void GraphicsRenderer::startDraw() {
    gl::ScopedModelMatrix scopedModelMatrix;
    glEnable(GL_LINE_SMOOTH);
    mGrid = gl::VertBatch::create( GL_LINES );
    mGrid->begin( GL_LINES );

	// Clear instance data for this frame
	clearInstanceData();
}

void GraphicsRenderer::drawCubeInstances() {
	if (mCubePositions.empty() || !mCubeMesh) {
		return;
	}

	// Check if any textures are actually valid
	bool hasValidTextures = false;
	for (const auto& tex : mCubeTextures) {
		if (tex) {
			hasValidTextures = true;
			break;
		}
	}

	static int frameCount = 0;
	if (frameCount % 60 == 0) {  // Log every 60 frames
		console() << "Drawing " << mCubePositions.size() << " cubes, hasValidTextures=" << hasValidTextures << std::endl;
	}
	frameCount++;

	if (hasValidTextures) {
		static bool loggedOnce = false;
		if (!loggedOnce) {
			console() << "Has valid textures, attempting textured instanced rendering..." << std::endl;
			console() << "mInstanceTexturedShader valid: " << (mInstanceTexturedShader ? "YES" : "NO") << std::endl;
			loggedOnce = true;
		}

		if (mInstanceTexturedShader) {
			// Use instanced rendering with texture shader
			static bool loggedOnce2 = false;
			if (!loggedOnce2) {
				console() << "Drawing " << mCubePositions.size() << " textured cubes INSTANCED with fu_00.png" << std::endl;
				// Find first valid texture to check size
				for (const auto& tex : mCubeTextures) {
					if (tex) {
						console() << "Texture size: " << tex->getWidth() << "x" << tex->getHeight() << std::endl;
						break;
					}
				}
				loggedOnce2 = true;
			}

			// Clear any previous GL errors
			while (glGetError() != GL_NO_ERROR);

			// Create interleaved instance data
			struct InstanceData {
				vec3 position;
				ColorA color;
				vec3 scale;
			};

			std::vector<InstanceData> instanceData;
			instanceData.reserve(mCubePositions.size());

			for (size_t i = 0; i < mCubePositions.size(); ++i) {
				InstanceData data;
				data.position = mCubePositions[i];
				data.color = ColorA(1.0f, 1.0f, 1.0f, 1.0f);  // White for full texture brightness
				data.scale = mCubeScales[i];
				instanceData.push_back(data);
			}

			// Create or update instance VBO
			if (!mCubeInstanceVbo || mCubeInstanceVbo->getSize() < instanceData.size() * sizeof(InstanceData)) {
				mCubeInstanceVbo = gl::Vbo::create(GL_ARRAY_BUFFER, instanceData, GL_DYNAMIC_DRAW);
			} else {
				mCubeInstanceVbo->bufferData(instanceData.size() * sizeof(InstanceData), instanceData.data(), GL_DYNAMIC_DRAW);
			}

			// Set up instance attributes
			geom::BufferLayout instanceLayout;
			instanceLayout.append(geom::Attrib::CUSTOM_0, 3, sizeof(InstanceData), offsetof(InstanceData, position), 1);
			instanceLayout.append(geom::Attrib::CUSTOM_1, 4, sizeof(InstanceData), offsetof(InstanceData, color), 1);
			instanceLayout.append(geom::Attrib::CUSTOM_2, 3, sizeof(InstanceData), offsetof(InstanceData, scale), 1);

			// Get the VBO mesh's VBOs and layouts
			std::vector<std::pair<geom::BufferLayout, gl::VboRef>> vertexArrayBuffers = mCubeMesh->getVertexArrayLayoutVbos();

			static bool loggedOnce3 = false;
			if (!loggedOnce3) {
				console() << "Original cube mesh has " << vertexArrayBuffers.size() << " VBO buffers" << std::endl;
				loggedOnce3 = true;
			}

			// Append instance data VBO
			vertexArrayBuffers.push_back(std::make_pair(instanceLayout, mCubeInstanceVbo));

			// Create new mesh with instance data
			auto instancedMesh = gl::VboMesh::create(
				mCubeMesh->getNumVertices(),
				mCubeMesh->getGlPrimitive(),
				vertexArrayBuffers,
				mCubeMesh->getNumIndices(),
				mCubeMesh->getIndexDataType(),
				mCubeMesh->getIndexVbo()
			);

			// Bind texture and draw instanced
			if (!mCubeTextures.empty() && mCubeTextures[0]) {
				try {
					// Find first valid texture
					gl::TextureRef validTex;
					for (const auto& tex : mCubeTextures) {
						if (tex) {
							validTex = tex;
							break;
						}
					}

					if (!validTex) {
						console() << "ERROR: No valid texture found!" << std::endl;
						return;
					}

					// Build attribute mapping - only custom attributes, let standard ones auto-bind
					gl::VboMesh::AttribGlslMap attributeMapping = {
						{ geom::Attrib::CUSTOM_0, "ciCustom0" },
						{ geom::Attrib::CUSTOM_1, "ciCustom1" },
						{ geom::Attrib::CUSTOM_2, "ciCustom2" }
					};

					auto batch = gl::Batch::create(instancedMesh, mInstanceTexturedShader, attributeMapping);

					static bool loggedTexInfo = false;
					if (!loggedTexInfo) {
						console() << "Texture bound to unit 0, ID=" << validTex->getId()
						          << " format=" << validTex->getInternalFormat() << std::endl;
						console() << "Setting uniform uTexture=0 via batch shader" << std::endl;
						loggedTexInfo = true;
					}

					// Set the texture uniform on the batch's shader BEFORE drawing
					batch->getGlslProg()->uniform("uTexture", 0);

					// Check pre-draw
					GLenum preErr = glGetError();
					if (preErr != GL_NO_ERROR) {
						console() << "OpenGL error BEFORE draw: 0x" << std::hex << preErr << std::dec << std::endl;
					}

					// Bind texture and draw
					gl::ScopedTextureBind texBind(validTex, 0);
					batch->drawInstanced(static_cast<GLsizei>(mCubePositions.size()));

					// Check for GL errors after draw
					GLenum err = glGetError();
					if (err != GL_NO_ERROR) {
						static bool reportedError = false;
						if (!reportedError) {
							console() << "OpenGL error after textured cube draw: 0x" << std::hex << err << std::dec << std::endl;
							reportedError = true;
						}
					} else {
						static bool reportedSuccess = false;
						if (!reportedSuccess) {
							console() << "Textured cubes drawn successfully with no GL errors!" << std::endl;
							reportedSuccess = true;
						}
					}
				}
				catch (const std::exception& e) {
					console() << "Exception during textured cube rendering: " << e.what() << std::endl;
				}
			}
		}
	} else {
		// Use instanced rendering for non-textured cubes
		if (!mInstanceShader) {
			console() << "ERROR: Instance shader not loaded!" << std::endl;
			return;
		}

		// Create interleaved instance data
		struct InstanceData {
			vec3 position;
			ColorA color;
			vec3 scale;
		};

		std::vector<InstanceData> instanceData;
		instanceData.reserve(mCubePositions.size());

		for (size_t i = 0; i < mCubePositions.size(); ++i) {
			InstanceData data;
			data.position = mCubePositions[i];
			data.color = mCubeColors[i];
			data.scale = mCubeScales[i];
			instanceData.push_back(data);
		}

		// Create or update instance VBO
		if (!mCubeInstanceVbo || mCubeInstanceVbo->getSize() < instanceData.size() * sizeof(InstanceData)) {
			mCubeInstanceVbo = gl::Vbo::create(GL_ARRAY_BUFFER, instanceData, GL_DYNAMIC_DRAW);
		} else {
			mCubeInstanceVbo->bufferData(instanceData.size() * sizeof(InstanceData), instanceData.data(), GL_DYNAMIC_DRAW);
		}

		// Set up instance attributes
		geom::BufferLayout instanceLayout;
		instanceLayout.append(geom::Attrib::CUSTOM_0, 3, sizeof(InstanceData), offsetof(InstanceData, position), 1);
		instanceLayout.append(geom::Attrib::CUSTOM_1, 4, sizeof(InstanceData), offsetof(InstanceData, color), 1);
		instanceLayout.append(geom::Attrib::CUSTOM_2, 3, sizeof(InstanceData), offsetof(InstanceData, scale), 1);

		// Get the VBO mesh's VBOs and layouts
		std::vector<std::pair<geom::BufferLayout, gl::VboRef>> vertexArrayBuffers = mCubeMesh->getVertexArrayLayoutVbos();

		// Append instance data VBO
		vertexArrayBuffers.push_back(std::make_pair(instanceLayout, mCubeInstanceVbo));

		// Create new mesh with instance data
		auto instancedMesh = gl::VboMesh::create(
			mCubeMesh->getNumVertices(),
			mCubeMesh->getGlPrimitive(),
			vertexArrayBuffers,
			mCubeMesh->getNumIndices(),
			mCubeMesh->getIndexDataType(),
			mCubeMesh->getIndexVbo()
		);

		// Build attribute mapping for custom attributes
		gl::VboMesh::AttribGlslMap attributeMapping = {
			{ geom::Attrib::CUSTOM_0, "ciCustom0" },
			{ geom::Attrib::CUSTOM_1, "ciCustom1" },
			{ geom::Attrib::CUSTOM_2, "ciCustom2" }
		};

		// Create batch and draw with instancing
		auto batch = gl::Batch::create(instancedMesh, mInstanceShader, attributeMapping);
		batch->drawInstanced(static_cast<GLsizei>(mCubePositions.size()));
	}
}

void GraphicsRenderer::drawSphereInstances() {
	if (mSpherePositions.empty() || !mSphereMesh) {
		return;
	}

	// Check if any spheres are from Pattern05 (environment mapped)
	bool hasPattern05 = false;
	for (int id : mSpherePatternIds) {
		if (id == 5) {
			hasPattern05 = true;
			break;
		}
	}

	// Use environment map shader if Pattern05 is present and shaders are loaded
	bool useEnvMap = hasPattern05 && mEnvMapShader && mCubeMap;
	auto shader = useEnvMap ? mEnvMapShader : mInstanceShader;

	if (!shader) {
		return;
	}

	// Create interleaved instance data
	struct InstanceData {
		vec3 position;
		ColorA color;
		vec3 scale;
	};

	std::vector<InstanceData> instanceData;
	instanceData.reserve(mSpherePositions.size());

	for (size_t i = 0; i < mSpherePositions.size(); ++i) {
		InstanceData data;
		data.position = mSpherePositions[i];
		data.color = mSphereColors[i];
		// Use radius for uniform scale
		data.scale = vec3(mSphereRadii[i]);
		instanceData.push_back(data);
	}

	// Create or update instance VBO
	if (!mSphereInstanceVbo || mSphereInstanceVbo->getSize() < instanceData.size() * sizeof(InstanceData)) {
		mSphereInstanceVbo = gl::Vbo::create(GL_ARRAY_BUFFER, instanceData, GL_DYNAMIC_DRAW);
	} else {
		mSphereInstanceVbo->bufferData(instanceData.size() * sizeof(InstanceData), instanceData.data(), GL_DYNAMIC_DRAW);
	}

	// Set up instance attributes
	geom::BufferLayout instanceLayout;
	instanceLayout.append(geom::Attrib::CUSTOM_0, 3, sizeof(InstanceData), offsetof(InstanceData, position), 1);  // iPosition
	instanceLayout.append(geom::Attrib::CUSTOM_1, 4, sizeof(InstanceData), offsetof(InstanceData, color), 1);     // iColor
	instanceLayout.append(geom::Attrib::CUSTOM_2, 3, sizeof(InstanceData), offsetof(InstanceData, scale), 1);     // iScale

	// Get the VBO mesh's VBOs and layouts
	std::vector<std::pair<geom::BufferLayout, gl::VboRef>> vertexArrayBuffers = mSphereMesh->getVertexArrayLayoutVbos();

	// Append instance data VBO
	vertexArrayBuffers.push_back(std::make_pair(instanceLayout, mSphereInstanceVbo));

	// Create new mesh with instance data
	auto instancedMesh = gl::VboMesh::create(
		mSphereMesh->getNumVertices(),
		mSphereMesh->getGlPrimitive(),
		vertexArrayBuffers,
		mSphereMesh->getNumIndices(),
		mSphereMesh->getIndexDataType(),
		mSphereMesh->getIndexVbo()
	);

	// Build attribute mapping for custom attributes
	gl::VboMesh::AttribGlslMap attributeMapping = {
		{ geom::Attrib::CUSTOM_0, "ciCustom0" },
		{ geom::Attrib::CUSTOM_1, "ciCustom1" },
		{ geom::Attrib::CUSTOM_2, "ciCustom2" }
	};

	// Create batch and draw with instancing
	auto batch = gl::Batch::create(instancedMesh, shader, attributeMapping);

	if (useEnvMap) {
		// Set environment map uniforms
		batch->getGlslProg()->uniform("uLightPos", vec3(0.0f, 100.0f, -100.0f));
		batch->getGlslProg()->uniform("uBaseColor", vec4(0.3f, 0.3f, 0.3f, 0.7f));
		batch->getGlslProg()->uniform("uMixRatio", 0.5f);
		batch->getGlslProg()->uniform("uEnvMap", 0);

		// Bind cubemap and draw
		gl::ScopedTextureBind texBind(mCubeMap, 0);
		batch->drawInstanced(static_cast<GLsizei>(mSpherePositions.size()));
	} else {
		batch->drawInstanced(static_cast<GLsizei>(mSpherePositions.size()));
	}
}

void GraphicsRenderer::drawCylinderInstances() {
	if (mCylinderPositions.empty() || !mCylinderMesh) {
		return;
	}

	// For cylinders with rotation, we need to draw them individually with model matrix transforms
	// since the current shader doesn't support quaternion rotation via instance attributes
	// Use Cinder's default shader (no custom attributes)
	auto shader = gl::getStockShader(gl::ShaderDef().color().lambert());

	for (size_t i = 0; i < mCylinderPositions.size(); ++i) {
		gl::ScopedModelMatrix scopedModel;
		gl::ScopedGlslProg scopedShader(shader);

		// Apply position
		gl::translate(mCylinderPositions[i]);

		// Apply rotation
		gl::rotate(mCylinderRotations[i]);

		// Apply scale
		gl::scale(mCylinderScales[i]);

		// Set color
		gl::color(mCylinderColors[i]);

		// Draw cylinder
		gl::draw(mCylinderMesh);
	}
}

void GraphicsRenderer::drawLineInstances() {
	if (mLineStarts.empty()) {
		return;
	}

	if (!mLineShader) {
		console() << "ERROR: Line shader not loaded!" << std::endl;
		return;
	}

	if (!mLineMesh) {
		console() << "ERROR: Line mesh not created!" << std::endl;
		return;
	}


	// Create interleaved instance data
	struct LineInstanceData {
		vec3 start;
		vec3 end;
		ColorA color;
		float width;
	};

	std::vector<LineInstanceData> instanceData;
	instanceData.reserve(mLineStarts.size());

	for (size_t i = 0; i < mLineStarts.size(); ++i) {
		LineInstanceData data;
		data.start = mLineStarts[i];
		data.end = mLineEnds[i];
		data.color = mLineColors[i];
		data.width = mLineWidths[i];
		instanceData.push_back(data);
	}

	// Create or update instance VBO
	if (!mLineInstanceVbo || mLineInstanceVbo->getSize() < instanceData.size() * sizeof(LineInstanceData)) {
		mLineInstanceVbo = gl::Vbo::create(GL_ARRAY_BUFFER, instanceData, GL_DYNAMIC_DRAW);
	} else {
		mLineInstanceVbo->bufferData(instanceData.size() * sizeof(LineInstanceData), instanceData.data(), GL_DYNAMIC_DRAW);
	}

	// Set up instance attributes
	geom::BufferLayout instanceLayout;
	instanceLayout.append(geom::Attrib::CUSTOM_0, 3, sizeof(LineInstanceData), offsetof(LineInstanceData, start), 1);  // iStart
	instanceLayout.append(geom::Attrib::CUSTOM_1, 3, sizeof(LineInstanceData), offsetof(LineInstanceData, end), 1);    // iEnd
	instanceLayout.append(geom::Attrib::CUSTOM_2, 4, sizeof(LineInstanceData), offsetof(LineInstanceData, color), 1);  // iColor
	instanceLayout.append(geom::Attrib::CUSTOM_3, 1, sizeof(LineInstanceData), offsetof(LineInstanceData, width), 1);  // iWidth

	// Get the VBO mesh's VBOs and layouts
	std::vector<std::pair<geom::BufferLayout, gl::VboRef>> vertexArrayBuffers = mLineMesh->getVertexArrayLayoutVbos();

	// Append instance data VBO
	vertexArrayBuffers.push_back(std::make_pair(instanceLayout, mLineInstanceVbo));

	// Create new mesh with instance data
	auto instancedMesh = gl::VboMesh::create(
		mLineMesh->getNumVertices(),
		mLineMesh->getGlPrimitive(),
		vertexArrayBuffers,
		mLineMesh->getNumIndices(),
		mLineMesh->getIndexDataType(),
		mLineMesh->getIndexVbo()
	);

	// Create batch and draw with instancing
	try {
		// Build attribute mapping for custom attributes
		gl::VboMesh::AttribGlslMap attributeMapping = {
			{ geom::Attrib::CUSTOM_0, "ciCustom0" },
			{ geom::Attrib::CUSTOM_1, "ciCustom1" },
			{ geom::Attrib::CUSTOM_2, "ciCustom2" },
			{ geom::Attrib::CUSTOM_3, "ciCustom3" }
		};

		// Clear any previous errors
		while (glGetError() != GL_NO_ERROR);

		glEnable(GL_LINE_SMOOTH);

		auto batch = gl::Batch::create(instancedMesh, mLineShader, attributeMapping);
		batch->drawInstanced(static_cast<GLsizei>(mLineStarts.size()));

		glDisable(GL_LINE_SMOOTH);

		// Check for GL errors
		GLenum err = glGetError();
		if (err != GL_NO_ERROR) {
			static int errorCount = 0;
			if (errorCount < 5) {  // Only print first 5 errors to avoid spam
				console() << "OpenGL error after drawLineInstances: 0x" << std::hex << err << std::dec << std::endl;
				errorCount++;
			}
		}
	}
	catch (const std::exception& e) {
		console() << "Exception in drawLineInstances: " << e.what() << std::endl;
	}
}

void GraphicsRenderer::drawSphericalQuads() {
	if (mSphericalQuads.empty()) return;

	// Use custom shader if available, fall back to Lambert
	auto shader = mSphericalQuadShader ? mSphericalQuadShader : gl::getStockShader(gl::ShaderDef().color().lambert());
	gl::ScopedGlslProg scopedShader(shader);

	// Build a single mesh containing all spherical quads
	std::vector<vec3> positions;
	std::vector<vec3> normals;
	std::vector<ColorA> colors;

	positions.reserve(mSphericalQuads.size() * 4);
	normals.reserve(mSphericalQuads.size() * 4);
	colors.reserve(mSphericalQuads.size() * 4);

	for (const auto& quad : mSphericalQuads) {
		float theta = quad.theta;
		float phi = quad.phi;
		float rho = quad.rho;
		float cellState = quad.cellState;

		// Scale quad size based on cell state
		// Higher state = larger quads, lower state = smaller quads
		float stateScale = 0.3f + cellState * 0.7f;  // Range from 0.3 to 1.0

		// Calculate angular spans - vary based on cell state
		float thetaStep = (2.0f * M_PI / ptrWorld->sizeX()) * stateScale;
		float phiStep = (2.0f * M_PI / ptrWorld->sizeY()) * stateScale;

		// Center the quad around the cell's angular position
		float thetaOffset = thetaStep * 0.5f;
		float phiOffset = phiStep * 0.5f;

		// Calculate the 4 corners of the quad in spherical coordinates
		vec3 corners[4];

		// Corner 0: (x+offset, y+offset)
		corners[0].x = rho * cos(theta + thetaOffset) * cos(phi + phiOffset);
		corners[0].y = rho * sin(theta + thetaOffset) * cos(phi + phiOffset);
		corners[0].z = rho * sin(phi + phiOffset);

		// Corner 1: (x-offset, y+offset)
		corners[1].x = rho * cos(theta - thetaOffset) * cos(phi + phiOffset);
		corners[1].y = rho * sin(theta - thetaOffset) * cos(phi + phiOffset);
		corners[1].z = rho * sin(phi + phiOffset);

		// Corner 2: (x-offset, y-offset)
		corners[2].x = rho * cos(theta - thetaOffset) * cos(phi - phiOffset);
		corners[2].y = rho * sin(theta - thetaOffset) * cos(phi - phiOffset);
		corners[2].z = rho * sin(phi - phiOffset);

		// Corner 3: (x+offset, y-offset)
		corners[3].x = rho * cos(theta + thetaOffset) * cos(phi - phiOffset);
		corners[3].y = rho * sin(theta + thetaOffset) * cos(phi - phiOffset);
		corners[3].z = rho * sin(phi - phiOffset);

		// Add quad as two triangles (0-1-2 and 0-2-3)
		// Triangle 1: corners 0, 1, 2
		for (int i : {0, 1, 2}) {
			positions.push_back(corners[i]);
			normals.push_back(glm::normalize(corners[i]));  // Normal points outward from sphere center
			colors.push_back(quad.cornerColors[i]);
		}

		// Triangle 2: corners 0, 2, 3
		for (int i : {0, 2, 3}) {
			positions.push_back(corners[i]);
			normals.push_back(glm::normalize(corners[i]));
			colors.push_back(quad.cornerColors[i]);
		}
	}

	// Create VboMesh from the collected vertices
	if (!positions.empty()) {
		auto vboMesh = gl::VboMesh::create(positions.size(), GL_TRIANGLES, {
			{ geom::BufferLayout({geom::AttribInfo(geom::Attrib::POSITION, 3, 0, 0)}),
			  gl::Vbo::create(GL_ARRAY_BUFFER, positions.size() * sizeof(vec3), positions.data(), GL_STATIC_DRAW) },
			{ geom::BufferLayout({geom::AttribInfo(geom::Attrib::NORMAL, 3, 0, 0)}),
			  gl::Vbo::create(GL_ARRAY_BUFFER, normals.size() * sizeof(vec3), normals.data(), GL_STATIC_DRAW) },
			{ geom::BufferLayout({geom::AttribInfo(geom::Attrib::COLOR, 4, 0, 0)}),
			  gl::Vbo::create(GL_ARRAY_BUFFER, colors.size() * sizeof(ColorA), colors.data(), GL_STATIC_DRAW) }
		});

		// Set shader uniforms if using custom shader
		if (mSphericalQuadShader) {
			shader->uniform("uTime", static_cast<float>(app::getElapsedSeconds()));
			shader->uniform("uLightPos", mLightLoc);
		}

		gl::draw(vboMesh);
	}
}

void GraphicsRenderer::drawPlaneInstances() {
	if (mPlaneInstances.empty()) return;

	// Separate filled planes from wireframe planes
	std::vector<vec3> filledPositions;
	std::vector<vec3> filledNormals;
	std::vector<ColorA> filledColors;
	std::vector<vec3> wirePositions;
	std::vector<ColorA> wireColors;

	// Collect vertices for all planes
	for (const auto& plane : mPlaneInstances) {
		vec3 v0, v1, v2, v3;
		vec3 normal;

		// Calculate corners and normal based on plane type
		switch (plane.planeType) {
			case 0: // XY plane (Z fixed) - normal points toward camera (-Z)
				v0 = plane.position;
				v1 = plane.position + vec3(plane.size.x, 0, 0);
				v2 = plane.position + vec3(plane.size.x, plane.size.y, 0);
				v3 = plane.position + vec3(0, plane.size.y, 0);
				normal = vec3(0, 0, -1);
				break;
			case 1: // YZ plane (X fixed) - normal points toward camera (-X)
				v0 = plane.position;
				v1 = plane.position + vec3(0, plane.size.y, 0);
				v2 = plane.position + vec3(0, plane.size.y, plane.size.z);
				v3 = plane.position + vec3(0, 0, plane.size.z);
				normal = vec3(-1, 0, 0);
				break;
			case 2: // XZ plane (Y fixed) - normal points toward camera (-Y)
				v0 = plane.position;
				v1 = plane.position + vec3(plane.size.x, 0, 0);
				v2 = plane.position + vec3(plane.size.x, 0, plane.size.z);
				v3 = plane.position + vec3(0, 0, plane.size.z);
				normal = vec3(0, -1, 0);
				break;
		}

		if (plane.wireframe) {
			// Add as line segments (4 edges)
			wirePositions.push_back(v0); wirePositions.push_back(v1);
			wireColors.push_back(plane.color); wireColors.push_back(plane.color);

			wirePositions.push_back(v1); wirePositions.push_back(v2);
			wireColors.push_back(plane.color); wireColors.push_back(plane.color);

			wirePositions.push_back(v2); wirePositions.push_back(v3);
			wireColors.push_back(plane.color); wireColors.push_back(plane.color);

			wirePositions.push_back(v3); wirePositions.push_back(v0);
			wireColors.push_back(plane.color); wireColors.push_back(plane.color);
		} else {
			// Add as two triangles (v0-v1-v2 and v0-v2-v3) with normals
			// Triangle 1
			filledPositions.push_back(v0);
			filledPositions.push_back(v1);
			filledPositions.push_back(v2);
			filledNormals.push_back(normal);
			filledNormals.push_back(normal);
			filledNormals.push_back(normal);
			filledColors.push_back(plane.color);
			filledColors.push_back(plane.color);
			filledColors.push_back(plane.color);

			// Triangle 2
			filledPositions.push_back(v0);
			filledPositions.push_back(v2);
			filledPositions.push_back(v3);
			filledNormals.push_back(normal);
			filledNormals.push_back(normal);
			filledNormals.push_back(normal);
			filledColors.push_back(plane.color);
			filledColors.push_back(plane.color);
			filledColors.push_back(plane.color);
		}
	}

	// Draw filled planes with additive blending (like original pattern05)
	if (!filledPositions.empty()) {
		// Boost colors significantly for visibility
		for (auto& color : filledColors) {
			color.r = std::min(color.r * 4.0f, 1.0f);
			color.g = std::min(color.g * 4.0f, 1.0f);
			color.b = std::min(color.b * 4.0f, 1.0f);
		}

		auto filledMesh = gl::VboMesh::create(filledPositions.size(), GL_TRIANGLES, {
			{ geom::BufferLayout({geom::AttribInfo(geom::Attrib::POSITION, 3, 0, 0)}),
			  gl::Vbo::create(GL_ARRAY_BUFFER, filledPositions.size() * sizeof(vec3), filledPositions.data(), GL_STATIC_DRAW) },
			{ geom::BufferLayout({geom::AttribInfo(geom::Attrib::COLOR, 4, 0, 0)}),
			  gl::Vbo::create(GL_ARRAY_BUFFER, filledColors.size() * sizeof(ColorA), filledColors.data(), GL_STATIC_DRAW) }
		});

		// Disable backface culling so planes are visible from both sides
		gl::disable(GL_CULL_FACE);

		// Use additive blending like the original pattern05 for brightness
		gl::ScopedBlendAdditive scopedBlend;

		// Use flat color shader (no lighting) - original pattern05 had no lighting
		auto shader = gl::getStockShader(gl::ShaderDef().color());
		gl::ScopedGlslProg scopedShader(shader);
		gl::draw(filledMesh);

		gl::enable(GL_CULL_FACE);
	}

	// Draw wireframe planes with additive blending
	if (!wirePositions.empty()) {
		// Boost wireframe colors significantly for visibility
		for (auto& color : wireColors) {
			color.r = std::min(color.r * 4.0f, 1.0f);
			color.g = std::min(color.g * 4.0f, 1.0f);
			color.b = std::min(color.b * 4.0f, 1.0f);
		}

		auto wireMesh = gl::VboMesh::create(wirePositions.size(), GL_LINES, {
			{ geom::BufferLayout({geom::AttribInfo(geom::Attrib::POSITION, 3, 0, 0)}),
			  gl::Vbo::create(GL_ARRAY_BUFFER, wirePositions.size() * sizeof(vec3), wirePositions.data(), GL_STATIC_DRAW) },
			{ geom::BufferLayout({geom::AttribInfo(geom::Attrib::COLOR, 4, 0, 0)}),
			  gl::Vbo::create(GL_ARRAY_BUFFER, wireColors.size() * sizeof(ColorA), wireColors.data(), GL_STATIC_DRAW) }
		});

		// Use additive blending like the original pattern05
		gl::ScopedBlendAdditive scopedBlend;

		auto shader = gl::getStockShader(gl::ShaderDef().color());
		gl::ScopedGlslProg scopedShader(shader);
		gl::enable(GL_LINE_SMOOTH);
		glLineWidth(2.0f);  // Make lines thicker for visibility
		gl::draw(wireMesh);
		glLineWidth(1.0f);  // Reset to default
		gl::disable(GL_LINE_SMOOTH);
	}
}

void GraphicsRenderer::drawPolygonInstances() {
	if (mPolygonInstances.empty()) return;

	// Build mesh with all polygons
	std::vector<vec3> positions;
	std::vector<ColorA> colors;

	positions.reserve(mPolygonInstances.size() * 6);  // 2 triangles per polygon = 6 vertices
	colors.reserve(mPolygonInstances.size() * 6);

	for (const auto& polygon : mPolygonInstances) {
		// Split quad into 2 triangles: (0,1,2) and (0,2,3)
		// Triangle 1
		positions.push_back(polygon.vertices[0]);
		positions.push_back(polygon.vertices[1]);
		positions.push_back(polygon.vertices[2]);
		colors.push_back(polygon.colors[0]);
		colors.push_back(polygon.colors[1]);
		colors.push_back(polygon.colors[2]);

		// Triangle 2
		positions.push_back(polygon.vertices[0]);
		positions.push_back(polygon.vertices[2]);
		positions.push_back(polygon.vertices[3]);
		colors.push_back(polygon.colors[0]);
		colors.push_back(polygon.colors[2]);
		colors.push_back(polygon.colors[3]);
	}

	if (!positions.empty()) {
		auto polygonMesh = gl::VboMesh::create(positions.size(), GL_TRIANGLES, {
			{ geom::BufferLayout({geom::AttribInfo(geom::Attrib::POSITION, 3, 0, 0)}),
			  gl::Vbo::create(GL_ARRAY_BUFFER, positions.size() * sizeof(vec3), positions.data(), GL_STATIC_DRAW) },
			{ geom::BufferLayout({geom::AttribInfo(geom::Attrib::COLOR, 4, 0, 0)}),
			  gl::Vbo::create(GL_ARRAY_BUFFER, colors.size() * sizeof(ColorA), colors.data(), GL_STATIC_DRAW) }
		});

		// Use additive blending and polygon smoothing like the original
		gl::ScopedBlendAdditive scopedBlend;
		glEnable(GL_POLYGON_SMOOTH);

		auto shader = gl::getStockShader(gl::ShaderDef().color());
		gl::ScopedGlslProg scopedShader(shader);
		gl::draw(polygonMesh);

		glDisable(GL_POLYGON_SMOOTH);
	}
}

void GraphicsRenderer::endDraw() {
	// Draw all collected instances
	drawCubeInstances();
	drawSphereInstances();
	drawCylinderInstances();
	drawLineInstances();
	drawSphericalQuads();
	drawPlaneInstances();
	drawPolygonInstances();

	// Draw legacy lines (will be deprecated)
    mGrid->end();
    mGrid->draw();

	counter++;
}

void GraphicsRenderer::drawFragment(Cell* cell) {

	int x, y, z;
	currentCell = cell;
	x = currentCell->x;
	y = currentCell->y;
	z = currentCell->z;

	state = currentCell->phase;

	// New pattern system: iterate through all patterns
	for (const auto& pattern : mPatterns) {
		if (!pattern->getActive()) continue;

		// Check if this pattern applies to this cell
		if (!pattern->isActive(x, y, z, cell, ptrWorld)) continue;

		// Get render configuration
		RenderConfig config = pattern->getRenderConfig(x, y, z, cell, ptrWorld, this);
		int patternId = pattern->getId();

		// Compute final position
		vec3 position;
		position.x = (float)x * fragSizeX + (fragSizeX * 0.5f) - hx;
		position.y = (float)y * fragSizeY + (fragSizeY * 0.5f) - hx;
		position.z = (float)z * fragSizeZ + (fragSizeZ * 0.5f) - hx;

		position += config.offset;

		// Compute final scale
		vec3 finalScale = config.scale * config.uniformScale;
		finalScale.x *= fragSizeX;
		finalScale.y *= fragSizeY;
		finalScale.z *= fragSizeZ;

		// Dispatch based on render mode
		switch (config.mode) {
			case RenderMode::CUBES:
				addCubeInstance(position, config.color, finalScale, config.texture);
				break;

			case RenderMode::SPHERES:
				addSphereInstance(position, config.color, config.uniformScale * fragSizeX, patternId);
				break;

			case RenderMode::CYLINDERS:
				addCylinderInstance(position, config.color, finalScale, config.rotation);
				break;

			case RenderMode::SPHERICAL_QUAD:
				addSphericalQuad(config.sphericalTheta, config.sphericalPhi, config.sphericalRho,
				                 config.sphericalGridPos, config.sphericalCornerColors, config.sphericalCellState);
				break;

			case RenderMode::LINES: {
				// For boundary wireframes, compute line segments for the cell's boundary edges
				float xL_local = (float)x * fragSizeX - hx;
				float yB_local = (float)y * fragSizeY - hx;
				float zF_local = (float)z * fragSizeZ - hx;
				float xR = xL_local + fragSizeX * config.scale.x * config.uniformScale * 4.0f;
				float yT = yB_local + fragSizeY * config.scale.y * config.uniformScale * 4.0f;
				float zB = zF_local + fragSizeZ * config.scale.z * config.uniformScale * 4.0f;

				// Draw wireframe box edges for this cell
				// Bottom face
				addLineInstance(vec3(xL_local, yB_local, zF_local), vec3(xR, yB_local, zF_local), config.color, config.lineWidth);
				addLineInstance(vec3(xR, yB_local, zF_local), vec3(xR, yT, zF_local), config.color, config.lineWidth);
				addLineInstance(vec3(xR, yT, zF_local), vec3(xL_local, yT, zF_local), config.color, config.lineWidth);
				addLineInstance(vec3(xL_local, yT, zF_local), vec3(xL_local, yB_local, zF_local), config.color, config.lineWidth);

				// Top face
				addLineInstance(vec3(xL_local, yB_local, zB), vec3(xR, yB_local, zB), config.color, config.lineWidth);
				addLineInstance(vec3(xR, yB_local, zB), vec3(xR, yT, zB), config.color, config.lineWidth);
				addLineInstance(vec3(xR, yT, zB), vec3(xL_local, yT, zB), config.color, config.lineWidth);
				addLineInstance(vec3(xL_local, yT, zB), vec3(xL_local, yB_local, zB), config.color, config.lineWidth);

				// Vertical edges
				addLineInstance(vec3(xL_local, yB_local, zF_local), vec3(xL_local, yB_local, zB), config.color, config.lineWidth);
				addLineInstance(vec3(xR, yB_local, zF_local), vec3(xR, yB_local, zB), config.color, config.lineWidth);
				addLineInstance(vec3(xR, yT, zF_local), vec3(xR, yT, zB), config.color, config.lineWidth);
				addLineInstance(vec3(xL_local, yT, zF_local), vec3(xL_local, yT, zB), config.color, config.lineWidth);
				break;
			}

			case RenderMode::PLANES: {
				// Pattern08: Center planes with nested rectangles
				float unmap = config.customFloats.at("unmap");
				bool onXPlane = config.customFloats.at("onXPlane") > 0.5f;
				bool onYPlane = config.customFloats.at("onYPlane") > 0.5f;
				bool onZPlane = config.customFloats.at("onZPlane") > 0.5f;

				// Draw initial stroked/filled rectangles
				float xL_base = (float)x * fragSizeX + (fragSizeX * 0.5f) - (fragSizeX * unmap) - hx;
				float yB_base = (float)y * fragSizeY + (fragSizeY * 0.5f) - (fragSizeY * unmap) - hx;
				float zF_base = (float)z * fragSizeZ + (fragSizeZ * 0.5f) - (fragSizeZ * unmap) - hx;

				float xW_base = fragSizeX * unmap * 2.0f;
				float yH_base = fragSizeY * unmap * 2.0f;
				float zD_base = fragSizeZ * unmap * 2.0f;

				// First set: initial rectangles (stroked for X/Z, filled for Y)
				if (onXPlane) {
					// YZ plane - stroked
					addPlaneInstance(vec3(xL_base, yB_base, zF_base), vec3(xW_base, yH_base, zD_base), config.color, 1, true);
				}
				if (onYPlane) {
					// XZ plane - filled
					addPlaneInstance(vec3(xL_base, yB_base, zF_base), vec3(xW_base, yH_base, zD_base), config.color, 2, false);
				}
				if (onZPlane) {
					// XY plane - stroked
					addPlaneInstance(vec3(xL_base, yB_base, zF_base), vec3(xW_base, yH_base, zD_base), config.color, 0, true);
				}

				// Draw nested rectangles at different scales
				float sizes[] = {1.0f, 0.5f, 0.25f, 0.125f};
				ColorA currentColor = config.color;

				for (int i = 0; i < 4; i++) {
					float xL_nest = (float)x * fragSizeX + (fragSizeX * sizes[i]) - (fragSizeX * unmap) - hx;
					float yB_nest = (float)y * fragSizeY + (fragSizeY * sizes[i]) - (fragSizeY * unmap) - hx;
					float zF_nest = (float)z * fragSizeZ + (fragSizeZ * sizes[i]) - (fragSizeZ * unmap) - hx;

					float xW_nest = fragSizeX * unmap * (1.0f / sizes[i]);
					float yH_nest = fragSizeY * unmap * (1.0f / sizes[i]);
					float zD_nest = fragSizeZ * unmap * (1.0f / sizes[i]);

					// Fade alpha for each nested level
					currentColor.a *= 0.87f;

					// Second set: nested rectangles (filled for X, stroked for Y, filled for Z)
					if (onXPlane) {
						// YZ plane - filled
						addPlaneInstance(vec3(xL_nest, yB_nest, zF_nest), vec3(xW_nest, yH_nest, zD_nest), currentColor, 1, false);
					}
					if (onYPlane) {
						// XZ plane - stroked
						addPlaneInstance(vec3(xL_nest, yB_nest, zF_nest), vec3(xW_nest, yH_nest, zD_nest), currentColor, 2, true);
					}
					if (onZPlane) {
						// XY plane - filled
						addPlaneInstance(vec3(xL_nest, yB_nest, zF_nest), vec3(xW_nest, yH_nest, zD_nest), currentColor, 0, false);
					}
				}
				break;
			}

			case RenderMode::POLYGON: {
				// Pattern09: Smooth polygons connecting neighbors
				// Vertices are already in world space from the pattern
				addPolygonInstance(config.polygonVertices, config.polygonColors);
				break;
			}

			default:
				break;
		}
	}

}

// pattern00 removed - now using new pattern system in pattern.cpp
// pattern01 removed - now using new pattern system in pattern.cpp

void GraphicsRenderer::pattern02(int x, int y, int z) {

	float cstate;
	
	if (ptrWorld->ruleType() == CONT) {
		cstate = state;
	}
	else {
		if (state != 0)
		{
			cstate = 1.0 / state;
		}
		else {
			cstate = 0.0f;
		}
	}
	
	xL = ((ptrWorld->sizeX()*fragSizeX/2) - ((float)x * fragSizeX)) * cstate + (fragSizeX * 0.5) - (fragSizeX * 2.0 * cstate);
	yB = ((ptrWorld->sizeY()*fragSizeX/2) - ((float)y * fragSizeY)) * cstate + (fragSizeY * 0.5) - (fragSizeY * 2.0 * cstate);
	zF = ((ptrWorld->sizeZ()*fragSizeX/2) - ((float)z * fragSizeZ)) * cstate + (fragSizeZ * 0.5) - (fragSizeZ * 2.0 * cstate);
	
	xW = mapf(fragSizeX * cstate, fragSizeX * 0.5, fragSizeX * 2.0);
	yH = mapf(fragSizeX * cstate, fragSizeX * 0.5, fragSizeX * 2.0);
	zD = mapf(fragSizeX * cstate, fragSizeX * 0.5, fragSizeX * 2.0);
	
	Pattern* p = getPattern(2);
	if (!p) return;
	red = p->getColor().r * abs(p->getColorMap() - cstate);
	green = p->getColor().g * abs(p->getColorMap() - cstate);
	blue = p->getColor().b * abs(p->getColorMap() - cstate);
	alpha = p->getAlpha() * abs(p->getAlphaMap() - cstate);
    
    gl::color(red, green, blue, alpha);
			
    

}

void GraphicsRenderer::pattern03(int x, int y, int z) {
    float cstate;

    if (ptrWorld->ruleType() == CONT) {
        cstate = state;
    }
    else {
        if (state != 0)
        {
            cstate = 1.0 / state;
        }
        else {
            cstate = 0.0f;
        }
    }

    if (ptrWorld->ruleType() == CONT || cstate != 0.0f) {

        float mapState, maxState;

        maxState = ptrWorld->rule()->numStates() - 1;
        mapState = (maxState - currentCell->states[ptrWorld->index()]) * (1 / maxState);

        xL = (float)x * fragSizeX + fragSizeX - (fragSizeX * 0.5);
        yB = (float)y * fragSizeX + fragSizeX - (fragSizeX * 0.5);
        zF = (float)z * fragSizeX + fragSizeX - (fragSizeX * 0.5);

        xW = yH = zD = mapf(fragSizeX * mapState, 0.25, 0.75);

        xL -= hx;
        yB -= hx;
        zF -= hx;

        Pattern* p = getPattern(3);
        if (!p) return;
        red = p->getColor().r * abs(p->getColorMap() - mapState);
        green = p->getColor().g * abs(p->getColorMap() - mapState);
        blue = p->getColor().b * abs(p->getColorMap() - mapState);
        alpha = p->getAlpha() * abs(p->getAlphaMap() - mapState);

        // Collect sphere instances instead of immediate drawing
        ColorA color(red, green, blue, alpha);
        vec3 position(xL, yB, zF);
        addSphereInstance(position, color, xW);

    }
}

void GraphicsRenderer::pattern04(int x, int y, int z) {


        float unmap, cstate;

        unmap = 1.0-unmapf(state, 0, ptrWorld->rule()->numStates()-1);
        cstate = currentCell->states[ptrWorld->index()];

    if (ptrWorld->ruleType() == CONT || cstate != 0.0f) {

        xL = (float)x * fragSizeX + (fragSizeX * 0.5) - (fragSizeX * unmap);
        yB = (float)y * fragSizeX + (fragSizeY * 0.5) - (fragSizeY * unmap);
        zF = (float)z * fragSizeZ + (fragSizeZ * 0.5) - (fragSizeZ * unmap);

        xW = fragSizeX * unmap * 2.0;
        yH = fragSizeX * unmap * 2.0;
        zD = fragSizeZ * unmap * 2.0;

        xL -= hx;
        yB -= hx;
        zF -= hx;

        Pattern* p = getPattern(4);
        if (!p) return;
        red = p->getColor().r * abs(p->getColorMap() - unmap);
        green = p->getColor().g * abs(p->getColorMap() - unmap);
        blue = p->getColor().b * abs(p->getColorMap() - unmap);
        alpha = p->getAlpha() * abs(p->getAlphaMap() - unmap);

        ColorA color(red, green, blue, alpha);

        // Collect cube instances instead of immediate drawing
        if (x == 0) {
            vec3 pos(xL + fragSizeX * 0.05f, yB + yH * 0.5f, zF + zD * 0.5f);
            vec3 scale(fragSizeX * 0.1f, yH, zD);
            addCubeInstance(pos, color, scale);
        }

        if (y == 0) {
            vec3 pos(xL + xW * 0.5f, yB + fragSizeY * 0.05f, zF + zD * 0.5f);
            vec3 scale(xW, fragSizeY * 0.1f, zD);
            addCubeInstance(pos, color, scale);
        }

        if (z == 0) {
            vec3 pos(xL + xW * 0.5f, yB + yH * 0.5f, zF + fragSizeZ * 0.05f);
            vec3 scale(xW, yH, fragSizeZ * 0.1f);
            addCubeInstance(pos, color, scale);
        }

        if (x == ptrWorld->sizeX()-1) {
            float xL2 = xL + (xW * cstate);
            vec3 pos(xL2 + fragSizeX * 0.05f, yB + yH * 0.5f, zF + zD * 0.5f);
            vec3 scale(fragSizeX * 0.1f, yH, zD);
            addCubeInstance(pos, color, scale);
        }
        if (y == ptrWorld->sizeY()-1 ) {
            float yB2 = yB + (yH * cstate);
            vec3 pos(xL + xW * 0.5f, yB2 + fragSizeY * 0.05f, zF + zD * 0.5f);
            vec3 scale(xW, fragSizeY * 0.1f, zD);
            addCubeInstance(pos, color, scale);
        }
        if (z == ptrWorld->sizeZ()-1 ) {
            float zF2 = zF + (zD * cstate);
            vec3 pos(xL + xW * 0.5f, yB + yH * 0.5f, zF2 + fragSizeZ * 0.05f);
            vec3 scale(xW, yH, fragSizeZ * 0.1f);
            addCubeInstance(pos, color, scale);
        }

    }
}



void GraphicsRenderer::fillRect(int plane) {
    // Define vertices for each plane
    std::vector<vec3> vertices;
    switch (plane) {
        case 0: // XY plane
            vertices = {
                vec3(xL, yB, zF),
                vec3(xL + xW, yB, zF),
                vec3(xL + xW, yB + yH, zF),
                vec3(xL, yB + yH, zF)
            };
            break;

        case 1: // YZ plane
            vertices = {
                vec3(xL, yB, zF),
                vec3(xL, yB, zF + zD),
                vec3(xL, yB + yH, zF + zD),
                vec3(xL, yB + yH, zF)
            };
            break;

        case 2: // XZ plane
            vertices = {
                vec3(xL, yB, zF),
                vec3(xL + xW, yB, zF),
                vec3(xL + xW, yB, zF + zD),
                vec3(xL, yB, zF + zD)
            };
            break;

        default:
            return; // Invalid plane
    }

    // Apply randomized color
//    float randRed = red * mapf(randFloat(), 0.9f, 1.1f);
//    float randGreen = green * mapf(randFloat(), 0.9f, 1.1f);
//    float randBlue = blue * mapf(randFloat(), 0.9f, 1.1f);

    gl::ScopedColor scopedColor(ColorA(red, green, blue, alpha));

    // Create geometry and draw
    gl::VboMeshRef mesh = gl::VboMesh::create(geom::Plane()
                                              .subdivisions(ivec2(1, 1))
                                              .size(vec2(xW, yH)));
    gl::ScopedModelMatrix scopedModel;
    gl::translate(vec3(xL, yB, zF));
    gl::draw(mesh);
}


// strokeRect removed - pattern00 now uses instanced line rendering

void GraphicsRenderer::drawEdges(const std::vector<vec3>& vertices) {
    for (size_t i = 0; i < vertices.size(); i += 2) {
        gl::drawLine(vertices[i], vertices[i + 1]);
    }
}

void GraphicsRenderer::drawPlaneFilled(float xL, float yB, float zF, float xW, float yH, float zD, int planeType, const ColorA& color) {
    gl::ScopedColor scopedColor(color);
    gl::ScopedModelMatrix scopedModel;

    vec3 v0, v1, v2, v3;

    switch (planeType) {
        case 0: // XY plane (Z fixed)
            v0 = vec3(xL, yB, zF);
            v1 = vec3(xL + xW, yB, zF);
            v2 = vec3(xL + xW, yB + yH, zF);
            v3 = vec3(xL, yB + yH, zF);
            break;
        case 1: // YZ plane (X fixed)
            v0 = vec3(xL, yB, zF);
            v1 = vec3(xL, yB + yH, zF);
            v2 = vec3(xL, yB + yH, zF + zD);
            v3 = vec3(xL, yB, zF + zD);
            break;
        case 2: // XZ plane (Y fixed)
            v0 = vec3(xL, yB, zF);
            v1 = vec3(xL + xW, yB, zF);
            v2 = vec3(xL + xW, yB, zF + zD);
            v3 = vec3(xL, yB, zF + zD);
            break;
    }

    // Draw filled quad
    gl::begin(GL_QUADS);
    gl::vertex(v0);
    gl::vertex(v1);
    gl::vertex(v2);
    gl::vertex(v3);
    gl::end();
}

void GraphicsRenderer::drawPlaneWireframe(float xL, float yB, float zF, float xW, float yH, float zD, int planeType, const ColorA& color) {
    gl::ScopedColor scopedColor(color);

    vec3 v0, v1, v2, v3;

    switch (planeType) {
        case 0: // XY plane (Z fixed)
            v0 = vec3(xL, yB, zF);
            v1 = vec3(xL + xW, yB, zF);
            v2 = vec3(xL + xW, yB + yH, zF);
            v3 = vec3(xL, yB + yH, zF);
            break;
        case 1: // YZ plane (X fixed)
            v0 = vec3(xL, yB, zF);
            v1 = vec3(xL, yB + yH, zF);
            v2 = vec3(xL, yB + yH, zF + zD);
            v3 = vec3(xL, yB, zF + zD);
            break;
        case 2: // XZ plane (Y fixed)
            v0 = vec3(xL, yB, zF);
            v1 = vec3(xL + xW, yB, zF);
            v2 = vec3(xL + xW, yB, zF + zD);
            v3 = vec3(xL, yB, zF + zD);
            break;
    }

    // Draw wireframe rectangle
    gl::drawLine(v0, v1);
    gl::drawLine(v1, v2);
    gl::drawLine(v2, v3);
    gl::drawLine(v3, v0);
}




