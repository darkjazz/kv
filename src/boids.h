/*
 *  boids.h
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

#ifndef BOIDS_H
#define BOIDS_H

#include "cinder/Vector.h"
#include <vector>
#include "util.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace std;

enum CenterType { ABS, MEAN, SET };

class Boid{
	
public:
	Boid() {};
    Boid(vec3, vec3, int);
	~Boid() {};
	
	void init();
	void reset();
	float distance(Boid*);
	
	vec3 pos, vec, separation;
	Vec3fRS cohesion, alignment;
	
	int lifeExpectancy;
	
};

class Boids{

public:
	Boids() {};
	Boids(int, vec3, double, double, double, double, double );
	~Boids();
	
	void init();
	void update();
	
	void setCenter(vec3);
	void setAutoCenter();
	void setNoCenter();
	Boid* getBoidAtIndex(int index) { return &_boids[index]; };
	int numBoids() { return _boids.size(); }
	void addBoid(vec3);
	void removeBoid(int);
    vec3 dimensions() { return _dim; }
	vec3 centroid();
	double speed, cohesion, alignment, separation, center;
	
	int maxNumBoids;

private:
	void calc(Boid*);
	void compare(Boid*, Boid*);
	
	Vec3fRS _ctr;
	vec3 _dim, _ctrvec;
	int _num;
	vector<Boid> _boids;
	CenterType _ctype;
	int _minLifeExpectancy, _maxLifeExpectancy;

};

#endif
