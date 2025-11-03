/*
 *  boids.cpp
 *  lambdaApp
 *
 *  Created by alo on 13/07/2012.
 *  
 *	This file is part of lambdaApp.
 *
 *	lambdaApp is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 
 *	lambdaApp is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 
 *	You should have received a copy of the GNU General Public License
 *	along with lambdaApp.  If not, see <http://www.gnu.org/licenses/>. 
 *
 */

#include "boids.h"

Boid::Boid(vec3 apos, vec3 avec, int lifeExp) {
	pos = apos;
	vec = avec;
	lifeExpectancy = lifeExp;
	init();
}

void Boid::init() {
	separation = vec3(0.0f, 0.0f, 0.0f);
	cohesion = Vec3fRS();
	alignment = Vec3fRS();
}

void Boid::reset() {
	separation = vec3(0.0f, 0.0f, 0.0f);
	cohesion.reset();
	alignment.reset();	
}

float Boid::distance(Boid* other) {
	return glm::distance(pos, other->pos);
}

Boids::Boids(int numboids, vec3 dimensions, double speed, double cohesion, double alignment, double separation, double center) {
	_num = numboids;
	_dim = dimensions;
	this->speed = speed;
	this->cohesion = cohesion;
	this->alignment = alignment;
	this->separation = separation;
	this->center = center;

	// Initialize audio reactivity
	baseCohesion = cohesion;
	baseSeparation = separation;
	baseAlignment = alignment;
	audioReactivityCohesion = 0.0f;
	audioReactivitySeparation = 0.0f;
	audioReactivityAlignment = 0.0f;

	_ctype = ABS;

	maxNumBoids = 100;
	_minLifeExpectancy = 100;
	_maxLifeExpectancy = 1000;

	init();

}

Boids::~Boids() { _boids.clear(); }

void Boids::init() {
	for (int i = 0; i < _num; i++) {
		_boids.push_back(Boid(randVec3(), vec3(0.0f, 0.0f, 0.0f), Rand::randInt(_minLifeExpectancy, _maxLifeExpectancy)));
	}
	
	_ctr = Vec3fRS();
}

void Boids::update() {
	
	int itX, itY;
	
//	_ctr.reset();
	
	for ( itX=0 ; itX < _boids.size(); itX++ ) {
		for ( itY=0 ; itY < _boids.size(); itY++ ) {
			if (itX != itY) compare(&_boids[itX], &_boids[itY]);
		}
		calc(&_boids[itX]);
		_ctr.push(_boids[itX].pos);
	}
}

void Boids::setCenter(vec3 value) { _ctype = SET; _ctrvec = value; }

void Boids::setAutoCenter() { _ctype = MEAN; }

void Boids::setNoCenter() { _ctype = ABS; }

void Boids::compare(Boid* boidX, Boid* boidY) {
	boidX->cohesion.push(boidY->pos);
	boidX->alignment.push(boidY->vec);
	if (boidX->distance(boidY) < separation) {
		boidX->separation -= (boidY->pos - boidX->pos);
	}
}

void Boids::calc(Boid* boid) {
	vec3 centerpoint;
	switch (_ctype) {
		case MEAN:
			centerpoint = _ctr.mean();
			break;
		case SET:
			centerpoint = _ctrvec;
			break;
		default:
			centerpoint = _dim / 2.0f;
			break;
	}
	
	boid->vec *= speed;
	boid->vec += ((boid->cohesion.mean() - boid->pos) / vec3(cohesion, cohesion, cohesion));
	boid->vec += ((boid->alignment.mean() - boid->vec) / vec3(alignment, alignment, alignment));
	boid->vec += boid->separation;	
	boid->vec += ((centerpoint - boid->pos) / vec3(center, center, center));	
	boid->pos += boid->vec;
	
	boid->pos = foldvec3(boid->pos, vec3(0.0f, 0.0f, 0.0f), _dim);
	
	boid->reset();
	
}

void Boids::addBoid(vec3 loc) {
	if (_boids.size() < maxNumBoids) 
		_boids.push_back(Boid(loc, vec3(0.0f, 0.0f, 0.0f), Rand::randInt(_minLifeExpectancy, _maxLifeExpectancy)));
}

void Boids::removeBoid(int index) {
	_boids.erase(_boids.begin() + index);
}

vec3 Boids::centroid() {
	switch (_ctype) {
		case MEAN:
			return _ctr.mean();
			break;
		case SET:
			return _ctrvec;
			break;
		default:
			return _dim / 2.0f;
			break;
	}
	
}
