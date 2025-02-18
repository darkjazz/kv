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
	
	glEnable( GL_TEXTURE_2D );
	glDisable( GL_TEXTURE_2D );
	
	gl::enableDepthRead();
	gl::enableDepthWrite();		
	gl::enableAlphaBlending();	
    
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

void GraphicsRenderer::startDraw() {
    gl::ScopedModelMatrix scopedModelMatrix;
}

void GraphicsRenderer::endDraw() {
	counter++;
}

void GraphicsRenderer::drawFragment(Cell* cell) {
	
	int x, y, z;
	currentCell = cell;
	x = currentCell->x;
	y = currentCell->y;
	z = currentCell->z;
	
	state = currentCell->phase;

	if (patternLib[0].active) {
		pattern00(x, y, z);
	}
	if (patternLib[1].active) {
		pattern01(x, y, z);	
	}
	if (patternLib[2].active) {
		pattern02(x, y, z);	
	}
	if (patternLib[3].active) {
		pattern03(x, y, z);	
	}
	if (patternLib[4].active) {
		pattern04(x, y, z);	
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

        // Compute color and alpha
        red = patternLib[0].color.r * abs(patternLib[0].colormap - cstate);
        green = patternLib[0].color.g * abs(patternLib[0].colormap - cstate);
        blue = patternLib[0].color.b * abs(patternLib[0].colormap - cstate);
        alpha = patternLib[0].alpha * abs(patternLib[0].alphamap - cstate);

        gl::ScopedColor scopedColor(ColorA(red, green, blue, alpha));

        // Draw edges directly based on the plane
        if (x == 0) {
            drawEdges({
                vec3(xL, yB, zF), vec3(xL, yB, zF + zD),
                vec3(xL, yB, zF + zD), vec3(xL, yB + yH, zF + zD),
                vec3(xL, yB + yH, zF + zD), vec3(xL, yB + yH, zF),
                vec3(xL, yB + yH, zF), vec3(xL, yB, zF)
            });
        }
        if (y == 0) {
            drawEdges({
                vec3(xL, yB, zF), vec3(xL + xW, yB, zF),
                vec3(xL + xW, yB, zF), vec3(xL + xW, yB, zF + zD),
                vec3(xL + xW, yB, zF + zD), vec3(xL, yB, zF + zD),
                vec3(xL, yB, zF + zD), vec3(xL, yB, zF)
            });
        }
        if (z == 0) {
            drawEdges({
                vec3(xL, yB, zF), vec3(xL + xW, yB, zF),
                vec3(xL + xW, yB, zF), vec3(xL + xW, yB + yH, zF),
                vec3(xL + xW, yB + yH, zF), vec3(xL, yB + yH, zF),
                vec3(xL, yB + yH, zF), vec3(xL, yB, zF)
            });
        }

        // Additional edge cases
        if (x == ptrWorld->sizeX() - 1 && y < ptrWorld->sizeY() - 1 && z < ptrWorld->sizeZ() - 1) {
            xL += (xW * cstate);
            drawEdges({
                vec3(xL, yB, zF), vec3(xL, yB, zF + zD),
                vec3(xL, yB, zF + zD), vec3(xL, yB + yH, zF + zD),
                vec3(xL, yB + yH, zF + zD), vec3(xL, yB + yH, zF),
                vec3(xL, yB + yH, zF), vec3(xL, yB, zF)
            });
        }
        if (y == ptrWorld->sizeY() - 1 && z < ptrWorld->sizeZ() - 1 && x < ptrWorld->sizeX() - 1) {
            yB += (yH * cstate);
            drawEdges({
                vec3(xL, yB, zF), vec3(xL + xW, yB, zF),
                vec3(xL + xW, yB, zF), vec3(xL + xW, yB, zF + zD),
                vec3(xL + xW, yB, zF + zD), vec3(xL, yB, zF + zD),
                vec3(xL, yB, zF + zD), vec3(xL, yB, zF)
            });
        }
        if (z == ptrWorld->sizeZ() - 1 && x < ptrWorld->sizeX() - 1 && y < ptrWorld->sizeY() - 1) {
            zF += (zD * cstate);
            drawEdges({
                vec3(xL, yB, zF), vec3(xL + xW, yB, zF),
                vec3(xL + xW, yB, zF), vec3(xL + xW, yB + yH, zF),
                vec3(xL + xW, yB + yH, zF), vec3(xL, yB + yH, zF),
                vec3(xL, yB + yH, zF), vec3(xL, yB, zF)
            });
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
		
		red = patternLib[1].color.r * abs(patternLib[1].colormap - cstate);
		green = patternLib[1].color.g * abs(patternLib[1].colormap - cstate);
		blue = patternLib[1].color.b * abs(patternLib[1].colormap - cstate);
		alpha = patternLib[1].alpha * abs(patternLib[1].alphamap - cstate);
        
		if (x == 0) { 
			fillRect(1);
		}
		
		if (y == 0) {
			fillRect(2);
		}
		
		if (z == 0) {
			fillRect(0);
		}
		
		if (x == ptrWorld->sizeX()-1 && y < ptrWorld->sizeY()-1 && z < ptrWorld->sizeZ()-1) {
			xL += (xW * state);
			fillRect(1);
		}
		if (y == ptrWorld->sizeY()-1 && z < ptrWorld->sizeZ()-1 && x < ptrWorld->sizeX()-1) {
			yB += (yH * state);
			fillRect(2);
		}	
		if (z == ptrWorld->sizeZ()-1 && x < ptrWorld->sizeX()-1 && y < ptrWorld->sizeY()-1) {
			zF += (zD * state);
			fillRect(0);	
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
	
	red = patternLib[2].color.r * abs(patternLib[2].colormap - cstate);
	green = patternLib[2].color.g * abs(patternLib[2].colormap - cstate);
	blue = patternLib[2].color.b * abs(patternLib[2].colormap - cstate);
	alpha = patternLib[2].alpha * abs(patternLib[2].alphamap - cstate);
    
    gl::color(red, green, blue, alpha);
			
	if (x == 0) {
        gl::drawCube( vec3(xL, yB, zF), vec3( fragSizeX * 0.25, fragSizeY, fragSizeZ ) );
//		fillRect(1);
	}
	
	if (y == 0) {
        gl::drawCube( vec3(xL, yB, zF), vec3( fragSizeX, fragSizeY * 0.25, fragSizeZ ) );
//        fillRect(2);
	}
	
	if (z == 0) {
        gl::drawCube( vec3(xL, yB, zF), vec3( fragSizeX, fragSizeY, fragSizeZ * 0.25 ) );
//        fillRect(0);
	}
	
	if (x == ptrWorld->sizeX()-1) {
        gl::drawCube( vec3(xL, yB, zF), vec3( fragSizeX * 0.25, fragSizeY, fragSizeZ ) );
//		xL += (xW * cstate);
//		fillRect(1);
	}
	if (y == ptrWorld->sizeY()-1 ) {
        gl::drawCube( vec3(xL, yB, zF), vec3( fragSizeX, fragSizeY * 0.25, fragSizeZ ) );
//		yB += (yH * cstate);
//		fillRect(2);
	}
	if (z == ptrWorld->sizeZ()-1 ) {
        gl::drawCube( vec3(xL, yB, zF), vec3( fragSizeX, fragSizeY, fragSizeZ * 0.25 ) );
//		zF += (zD * cstate);
//		fillRect(0);
	}

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
        
        red = patternLib[3].color.r * abs(patternLib[3].colormap - mapState);
        green = patternLib[3].color.g * abs(patternLib[3].colormap - mapState);
        blue = patternLib[3].color.b * abs(patternLib[3].colormap - mapState);
        alpha = patternLib[3].alpha * abs(patternLib[3].alphamap - mapState);
        
        gl::color(red, green, blue, alpha);
//        gl::drawStrokedCube( vec3(xL, yB, zF), vec3(xW, yH, zD) );
        gl::drawSphere( vec3(xL, yB, zF), xW);
        
    }
}

void GraphicsRenderer::pattern04(int x, int y, int z) {

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
    
    red = patternLib[4].color.r * abs(patternLib[4].colormap - cstate);
    green = patternLib[4].color.g * abs(patternLib[4].colormap - cstate);
    blue = patternLib[4].color.b * abs(patternLib[4].colormap - cstate);
    alpha = patternLib[4].alpha * abs(patternLib[4].alphamap - cstate);
        
    gl::color(red, green, blue, alpha);
    
    if (x == 0) {
        gl::drawCube( vec3(x * fragSizeX, yB, zF), vec3( fragSizeX * 0.1, yH, zD ) );
        //		fillRect(1);
    }
    
    if (y == 0) {
        gl::drawCube( vec3(xL, y * fragSizeY, zF), vec3( xW, fragSizeY * 0.1, zD ) );
        //        fillRect(2);
    }
    
    if (z == 0) {
        gl::drawCube( vec3(xL, yB, z * fragSizeZ), vec3( xW, yH, fragSizeZ * 0.1 ) );
        //        fillRect(0);
    }
    
    if (x == ptrWorld->sizeX()-1) {
        gl::drawCube( vec3(x * fragSizeX + fragSizeX, yB, zF), vec3( fragSizeX * 0.1, yH, zD ) );
        //		xL += (xW * cstate);
        //		fillRect(1);
    }
    if (y == ptrWorld->sizeY()-1 ) {
        gl::drawCube( vec3(xL, y * fragSizeY + fragSizeY, zF), vec3( xW, fragSizeY * 0.1, zD ) );
        //		yB += (yH * cstate);
        //		fillRect(2);
    }
    if (z == ptrWorld->sizeZ()-1 ) {
        gl::drawCube( vec3(xL, yB, z * fragSizeZ + fragSizeZ), vec3( xW, yH, fragSizeZ * 0.1 ) );
        //		zF += (zD * cstate);
        //		fillRect(0);
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
    float randRed = red * mapf(randFloat(), 0.9f, 1.1f);
    float randGreen = green * mapf(randFloat(), 0.9f, 1.1f);
    float randBlue = blue * mapf(randFloat(), 0.9f, 1.1f);

    gl::ScopedColor scopedColor(ColorA(randRed, randGreen, randBlue, alpha));

    // Create geometry and draw
    gl::VboMeshRef mesh = gl::VboMesh::create(geom::Plane()
                                              .subdivisions(ivec2(1, 1))
                                              .size(vec2(xW, yH)));
    gl::ScopedModelMatrix scopedModel;
    gl::translate(vec3(xL, yB, zF));
    gl::draw(mesh);
}

void GraphicsRenderer::drawEdges(const std::vector<vec3>& vertices) {
    for (size_t i = 0; i < vertices.size(); i += 2) {
        gl::drawLine(vertices[i], vertices[i + 1]);
    }
}




