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
		mInstanceShader = gl::GlslProg::create(
			loadAsset("instance.vert"),
			loadAsset("instance.frag")
		);
	}
	catch (const std::exception& e) {
		console() << "Error loading instance shader: " << e.what() << std::endl;
	}

	// Create cube mesh geometry
	mCubeMesh = gl::VboMesh::create(geom::Cube());

	// Create sphere mesh geometry
	mSphereMesh = gl::VboMesh::create(geom::Sphere().subdivisions(16));

}

void GraphicsRenderer::reshape() {

	mCam.setPerspective(45.0, getWindowAspectRatio(), 0.1f, 2000.0f);
	gl::setMatrices( mCam );

}

void GraphicsRenderer::update() {
		
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

	mSpherePositions.clear();
	mSphereColors.clear();
	mSphereRadii.clear();
}

void GraphicsRenderer::addCubeInstance(const vec3& position, const ColorA& color, const vec3& scale) {
	mCubePositions.push_back(position);
	mCubeColors.push_back(color);
	mCubeScales.push_back(scale);
}

void GraphicsRenderer::addSphereInstance(const vec3& position, const ColorA& color, float radius) {
	mSpherePositions.push_back(position);
	mSphereColors.push_back(color);
	mSphereRadii.push_back(radius);
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
	if (mCubePositions.empty() || !mInstanceShader || !mCubeMesh) {
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
	instanceLayout.append(geom::Attrib::CUSTOM_0, 3, sizeof(InstanceData), offsetof(InstanceData, position), 1);  // iPosition
	instanceLayout.append(geom::Attrib::CUSTOM_1, 4, sizeof(InstanceData), offsetof(InstanceData, color), 1);     // iColor
	instanceLayout.append(geom::Attrib::CUSTOM_2, 3, sizeof(InstanceData), offsetof(InstanceData, scale), 1);     // iScale

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

	// Create batch and draw with instancing
	auto batch = gl::Batch::create(instancedMesh, mInstanceShader);
	batch->drawInstanced(static_cast<GLsizei>(mCubePositions.size()));
}

void GraphicsRenderer::drawSphereInstances() {
	if (mSpherePositions.empty() || !mInstanceShader || !mSphereMesh) {
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

	// Create batch and draw with instancing
	auto batch = gl::Batch::create(instancedMesh, mInstanceShader);
	batch->drawInstanced(static_cast<GLsizei>(mSpherePositions.size()));
}

void GraphicsRenderer::endDraw() {
	// Draw all collected instances
	drawCubeInstances();
	drawSphereInstances();

	// Draw lines
    mGrid->end();
    glDisable(GL_LINE_SMOOTH);
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
				addCubeInstance(position, config.color, finalScale);
				break;

			case RenderMode::SPHERES:
				addSphereInstance(position, config.color, config.uniformScale * fragSizeX);
				break;

			case RenderMode::LINES:
				// Lines handled by legacy pattern00 for now
				pattern00(x, y, z);
				break;

			default:
				break;
		}
	}

}

void GraphicsRenderer::pattern00(int x, int y, int z) {
    float cstate;

    if (ptrWorld->ruleType() == CONT) {
        cstate = state;
    } else {
        cstate = (state != 0) ? 1.0f / state : 0.0f;
    }

    if (ptrWorld->ruleType() == CONT || cstate != 0.0f) {
        // Compute dimensions and positions
        xL = (float)x * fragSizeX + (fragSizeX * 0.5) - (fragSizeX * 2.0f * cstate);
        yB = (float)y * fragSizeY + (fragSizeY * 0.5) - (fragSizeY * 2.0f * cstate);
        zF = (float)z * fragSizeZ + (fragSizeZ * 0.5) - (fragSizeZ * 2.0f * cstate);

        xW = fragSizeX * cstate * 4.0f;
        yH = fragSizeY * cstate * 4.0f;
        zD = fragSizeZ * cstate * 4.0f;

        xL -= hx;
        yB -= hx;
        zF -= hx;

        Pattern* p = getPattern(0);
        if (!p) return;
        red = p->getColor().r * abs(p->getColorMap() - cstate);
        green = p->getColor().g * abs(p->getColorMap() - cstate);
        blue = p->getColor().b * abs(p->getColorMap() - cstate);
        alpha = p->getAlpha() * abs(p->getAlphaMap() - cstate);
        

        if (x == 0) {
            strokeRect(1, 1.0, red, green, blue, alpha);
        }
        
        if (y == 0) {
            strokeRect(2, 1.0, red, green, blue, alpha);
        }
        
        if (z == 0) {
            strokeRect(0, 1.0, red, green, blue, alpha);
        }
                
        if (x == ptrWorld->sizeX()-1 && y < ptrWorld->sizeY()-1 && z < ptrWorld->sizeZ()-1) {
            xL += (xW * cstate);
            strokeRect(1, 1.0, red, green, blue, alpha);
        }
        if (y == ptrWorld->sizeY()-1 && z < ptrWorld->sizeZ()-1 && x < ptrWorld->sizeX()-1) {
            yB += (yH * cstate);
            strokeRect(2, 1.0, red, green, blue, alpha);
        }
        if (z == ptrWorld->sizeZ()-1 && x < ptrWorld->sizeX()-1 && y < ptrWorld->sizeY()-1) {
            zF += (zD * cstate);
            strokeRect(0, 1.0, red, green, blue, alpha);
        }
    }
}

void GraphicsRenderer::pattern01(int x, int y, int z) {

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
		xL = (float)x * fragSizeX + (fragSizeX * 0.5) - (fragSizeX * 2.0 * cstate);
		yB = (float)y * fragSizeX + (fragSizeY * 0.5) - (fragSizeY * 2.0 * cstate);
		zF = (float)z * fragSizeZ + (fragSizeZ * 0.5) - (fragSizeZ * 2.0 * cstate);

		xW = mapf(fragSizeX * cstate, 0.5, 2.0);
		yH = mapf(fragSizeX * cstate, 0.5, 2.0);
		zD = mapf(fragSizeX * cstate, 0.5, 2.0);

		xL -= hx;
		yB -= hx;
		zF -= hx;

		Pattern* p = getPattern(1);
		if (!p) return;
		red = p->getColor().r * abs(p->getColorMap() - cstate);
		green = p->getColor().g * abs(p->getColorMap() - cstate);
		blue = p->getColor().b * abs(p->getColorMap() - cstate);
		alpha = p->getAlpha() * abs(p->getAlphaMap() - cstate);

		ColorA color(red, green, blue, alpha);

		// Collect cube instances instead of immediate drawing
		if (x == 0) {
			// YZ plane - thin in X direction
			vec3 pos(xL + xW * 0.5f, yB + yH * 0.5f, zF + zD * 0.5f);
			vec3 scale(fragSizeX * 0.1f, yH, zD);
			addCubeInstance(pos, color, scale);
		}

		if (y == 0) {
			// XZ plane - thin in Y direction
			vec3 pos(xL + xW * 0.5f, yB + yH * 0.5f, zF + zD * 0.5f);
			vec3 scale(xW, fragSizeY * 0.1f, zD);
			addCubeInstance(pos, color, scale);
		}

		if (z == 0) {
			// XY plane - thin in Z direction
			vec3 pos(xL + xW * 0.5f, yB + yH * 0.5f, zF + zD * 0.5f);
			vec3 scale(xW, yH, fragSizeZ * 0.1f);
			addCubeInstance(pos, color, scale);
		}

		if (x == ptrWorld->sizeX()-1 && y < ptrWorld->sizeY()-1 && z < ptrWorld->sizeZ()-1) {
			float xL2 = xL + (xW * state);
			vec3 pos(xL2 + xW * 0.5f, yB + yH * 0.5f, zF + zD * 0.5f);
			vec3 scale(fragSizeX * 0.1f, yH, zD);
			addCubeInstance(pos, color, scale);
		}
		if (y == ptrWorld->sizeY()-1 && z < ptrWorld->sizeZ()-1 && x < ptrWorld->sizeX()-1) {
			float yB2 = yB + (yH * state);
			vec3 pos(xL + xW * 0.5f, yB2 + yH * 0.5f, zF + zD * 0.5f);
			vec3 scale(xW, fragSizeY * 0.1f, zD);
			addCubeInstance(pos, color, scale);
		}
		if (z == ptrWorld->sizeZ()-1 && x < ptrWorld->sizeX()-1 && y < ptrWorld->sizeY()-1) {
			float zF2 = zF + (zD * state);
			vec3 pos(xL + xW * 0.5f, yB + yH * 0.5f, zF2 + zD * 0.5f);
			vec3 scale(xW, yH, fragSizeZ * 0.1f);
			addCubeInstance(pos, color, scale);
		}

	}
}

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


void GraphicsRenderer::strokeRect(int plane, float lineWidth, float r, float g, float b, float a) {
            
    mGrid->color( ColorA( r, g, b, a ) );
    mGrid->color( ColorA( r, g, b, a ) );
    mGrid->color( ColorA( r, g, b, a ) );
    mGrid->color( ColorA( r, g, b, a ) );

    switch (plane)
    {
        case 0:
            mGrid->vertex (xL, yB, zF);
            mGrid->vertex (xL + xW, yB, zF);
            
            mGrid->vertex (xL + xW, yB, zF);
            mGrid->vertex (xL + xW, yB + yH, zF);
            
            mGrid->vertex (xL + xW, yB + yH, zF);
            mGrid->vertex (xL, yB + yH, zF);
            
            mGrid->vertex (xL, yB + yH, zF);
            mGrid->vertex (xL, yB, zF);
            
            break;
            
        case 1:
            mGrid->vertex (xL, yB, zF);
            mGrid->vertex (xL, yB, zF + zD);
            
            mGrid->vertex (xL, yB, zF + zD);
            mGrid->vertex (xL, yB + yH, zF + zD);
            
            mGrid->vertex (xL, yB + yH, zF + zD);
            mGrid->vertex (xL, yB + yH, zF);
            
            mGrid->vertex (xL, yB + yH, zF);
            mGrid->vertex (xL, yB, zF);
            
            break;
            
        case 2:
            mGrid->vertex (xL, yB, zF);
            mGrid->vertex (xL + xW, yB, zF);
            
            mGrid->vertex (xL + xW, yB, zF);
            mGrid->vertex (xL + xW, yB, zF + zD);
            
            mGrid->vertex (xL + xW, yB, zF + zD);
            mGrid->vertex (xL, yB, zF + zD);
            
            mGrid->vertex (xL, yB, zF + zD);
            mGrid->vertex (xL, yB, zF);
            
            break;
            
    }

}

void GraphicsRenderer::drawEdges(const std::vector<vec3>& vertices) {
    for (size_t i = 0; i < vertices.size(); i += 2) {
        gl::drawLine(vertices[i], vertices[i + 1]);
    }
}




