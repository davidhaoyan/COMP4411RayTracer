#include <cmath>

#include "light.h"

double DirectionalLight::distanceAttenuation( const vec3f& P ) const
{
	// distance to light is infinite, so f(di) goes to 0.  Return 1.

	return 1.0;
}


vec3f DirectionalLight::shadowAttenuation( const vec3f& P ) const
{
    // YOUR CODE HERE:
    // You should implement shadow-handling code here.
	vec3f lightDirection = getDirection(P).normalize();
	ray shadowRay = ray(P + RAY_EPSILON * lightDirection, lightDirection);
	isect i;

	if (scene->intersect(shadowRay, i)) {
		vec3f scale = i.getMaterial().kt;
		return scale;
	}
	else {
		return vec3f(1, 1, 1);
	}
}

vec3f DirectionalLight::getColor( const vec3f& P ) const
{
	// Color doesn't depend on P 
	return color;
}

vec3f DirectionalLight::getDirection( const vec3f& P ) const
{
	return -orientation;
}

double PointLight::distanceAttenuation( const vec3f& P ) const
{
	// YOUR CODE HERE

	// You'll need to modify this method to attenuate the intensity 
	// of the light based on the distance between the source and the 
	// point P.  For now, I assume no attenuation and just return 1.0

	float a = this->a;
	float b = this->b;
	float c = this->c;

	double scale = 50;
	double distance = (P - position).length() * scale;
	double dropOff = 1 / (a + b * distance + c * distance * distance);
	double attenuation = min((double)1, dropOff);

	return attenuation;
}

vec3f PointLight::getColor( const vec3f& P ) const
{
	// Color doesn't depend on P 
	return color;
}

vec3f PointLight::getDirection( const vec3f& P ) const
{
	return (position - P).normalize();
}


vec3f PointLight::shadowAttenuation(const vec3f& P) const
{
	// YOUR CODE HERE:
	// You should implement shadow-handling code here.

	vec3f lightDirection = getDirection(P).normalize();
	ray shadowRay = ray(P + RAY_EPSILON*lightDirection, lightDirection);
	isect i;

	if (scene->intersect(shadowRay, i)) {
		vec3f scale = i.getMaterial().kt;
		return scale;
	}
	else {
		return vec3f(1, 1, 1);
	}
}
