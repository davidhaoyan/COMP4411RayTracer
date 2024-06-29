// The main ray tracer.

#include <Fl/fl_ask.h>

#include "RayTracer.h"
#include "scene/light.h"
#include "scene/material.h"
#include "scene/ray.h"
#include "fileio/read.h"
#include "fileio/parse.h"

#include <iostream>

#include <stack>

using namespace std;

// Trace a top-level ray through normalized window coordinates (x,y)
// through the projection plane, and out into the scene.  All we do is
// enter the main ray-tracing method, getting things started by plugging
// in an initial ray weight of (0.0,0.0,0.0) and an initial recursion depth of 0.
vec3f RayTracer::trace( Scene *scene, double x, double y )
{
    ray r( vec3f(0,0,0), vec3f(0,0,0) );
    scene->getCamera()->rayThrough( x,y,r );
	std::stack<double> refractiveIndices;
	refractiveIndices.push(1.0);
	return traceRay( scene, r, vec3f(1.0,1.0,1.0), 0).clamp();
}

// Do recursive ray tracing!  You'll want to insert a lot of code here
// (or places called from here) to handle reflection, refraction, etc etc.
vec3f RayTracer::traceRay( Scene *scene, const ray& r, 
	const vec3f& thresh, int depth)
{
	if (depth > this->depth) {
		return vec3f(0, 0, 0);
	}

	isect i;

	if( scene->intersect( r, i ) ) {
		// YOUR CODE HERE

		// An intersection occured!  We've got work to do.  For now,
		// this code gets the material for the surface that was intersected,
		// and asks that material to provide a color for the ray.  

		// This is a great place to insert code for recursive ray tracing.
		// Instead of just returning the result of shade(), add some
		// more steps: add in the contributions from reflected and refracted
		// rays.

		const Material& m = i.getMaterial();

		vec3f color = m.shade(scene, r, i);
		vec3f N = i.N.normalize();
		vec3f V = -r.getDirection().normalize();
		vec3f intersectionPoint = r.at(i.t);

		vec3f refractiveComponent;

		if (!m.kt.iszero()) {
			prod(color, vec3f(1,1,1) - vec3f(m.kt));


			double indexI = 1;
			double indexT = 1;

			if (N.dot(- V) > RAY_EPSILON) { // Exiting a material
				if (!refractiveIndices.empty()) {
					indexI = refractiveIndices.top();
				}
				if (!refractiveIndices.empty()) {
					refractiveIndices.pop();
				}
				indexT = m.index;
				N = -N;
			}
			else { // Entering a material
				if (!refractiveIndices.empty()) {
					indexI = refractiveIndices.top();
				}
				refractiveIndices.push(m.index);
				indexT = m.index;
				N = N;
			}


			double indexRatio = indexI / indexT;
			double cos_i = max(min(N.dot(V), 1.0), -1.0);
			double sin_i = sqrt(1 - cos_i * cos_i);
			double sin_t = sin_i * indexRatio;

			if (sin_t <= 1.0) {
				double cos_t = sqrt(1 - sin_t * sin_t);
				vec3f refractiveDirection = (indexRatio * cos_i - cos_t) * N - indexRatio * V;
				ray refractiveRay(intersectionPoint + RAY_EPSILON * refractiveDirection, refractiveDirection);
				refractiveComponent = traceRay(scene, refractiveRay, thresh, depth + 1);
			}
		}

		bool isReflective = false;
		for (int chan = 0; chan < 3; chan++) {
			if (m.kr[chan] > 0) {
				isReflective = true;
			}
		}

		vec3f reflectiveComponent;
		if (isReflective) {
			vec3f reflectiveDir = ((2 * N.dot(V) * N) - V).normalize();
			ray reflectiveRay(intersectionPoint + RAY_EPSILON * reflectiveDir, reflectiveDir);
			reflectiveComponent = traceRay(scene, reflectiveRay, thresh, depth + 1);
		}
		
		for (int chan = 0; chan < 3; chan++) {
			color[chan] += reflectiveComponent[chan] * m.kr[chan];
			color[chan] += refractiveComponent[chan] * m.kt[chan];
		}
		
		while (!refractiveIndices.empty()) {
			refractiveIndices.pop();
		}


		return color.clamp();
	
	} else {
		// No intersection.  This ray travels to infinity, so we color
		// it according to the background color, which in this (simple) case
		// is just black.

		return vec3f( 0.0, 0.0, 0.0 );
	}
}

RayTracer::RayTracer()
{
	buffer = NULL;
	buffer_width = buffer_height = 256;
	scene = NULL;

	m_bSceneLoaded = false;
}


RayTracer::~RayTracer()
{
	delete [] buffer;
	delete scene;
}

void RayTracer::getBuffer( unsigned char *&buf, int &w, int &h )
{
	buf = buffer;
	w = buffer_width;
	h = buffer_height;
}

double RayTracer::aspectRatio()
{
	return scene ? scene->getCamera()->getAspectRatio() : 1;
}

bool RayTracer::sceneLoaded()
{
	return m_bSceneLoaded;
}

bool RayTracer::loadScene( char* fn )
{
	try
	{
		scene = readScene( fn );
	}
	catch( ParseError pe )
	{
		fl_alert( "ParseError: %s\n", pe );
		return false;
	}

	if( !scene )
		return false;
	
	buffer_width = 256;
	buffer_height = (int)(buffer_width / scene->getCamera()->getAspectRatio() + 0.5);

	bufferSize = buffer_width * buffer_height * 3;
	buffer = new unsigned char[ bufferSize ];
	
	// separate objects into bounded and unbounded
	scene->initScene();
	
	// Add any specialized scene loading code here
	
	m_bSceneLoaded = true;

	return true;
}

void RayTracer::traceSetup( int w, int h )
{
	if( buffer_width != w || buffer_height != h )
	{
		buffer_width = w;
		buffer_height = h;

		bufferSize = buffer_width * buffer_height * 3;
		delete [] buffer;
		buffer = new unsigned char[ bufferSize ];
	}
	memset( buffer, 0, w*h*3 );
}

void RayTracer::setDepth(int depth) {
	this->depth = depth;
}

void RayTracer::traceLines( int start, int stop )
{
	vec3f col;
	if( !scene )
		return;

	if( stop > buffer_height )
		stop = buffer_height;

	for( int j = start; j < stop; ++j )
		for( int i = 0; i < buffer_width; ++i )
			tracePixel(i,j);
}

void RayTracer::tracePixel( int i, int j )
{
	vec3f col;

	if( !scene )
		return;

	double x = double(i)/double(buffer_width);
	double y = double(j)/double(buffer_height);

	col = trace( scene,x,y );

	unsigned char *pixel = buffer + ( i + j * buffer_width ) * 3;

	pixel[0] = (int)( 255.0 * col[0]);
	pixel[1] = (int)( 255.0 * col[1]);
	pixel[2] = (int)( 255.0 * col[2]);
}