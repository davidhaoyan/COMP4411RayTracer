#include "ray.h"
#include "material.h"
#include "light.h"

// Apply the phong model to this point on the surface of the object, returning
// the color of that point.
vec3f Material::shade( Scene *scene, const ray& r, const isect& i ) const
{
	// YOUR CODE HERE

	// For now, this method just returns the diffuse color of the object.
	// This gives a single matte color for every distinct surface in the
	// scene, and that's it.  Simple, but enough to get you started.
	// (It's also inconsistent with the phong model...)

	// Your mission is to fill in this method with the rest of the phong
	// shading model, including the contributions of all the light sources.
    // You will need to call both distanceAttenuation() and shadowAttenuation()
    // somewhere in your code in order to compute shadows and light falloff.

	vec3f color;
	vec3f ka = i.getMaterial().ka;
	vec3f kd = i.getMaterial().kd;
	vec3f ks = i.getMaterial().ks;
	vec3f ke = i.getMaterial().ke;
	vec3f ambientColor = scene->ambientColor;
	double shininess = i.getMaterial().shininess * 128;

	vec3f intersectionPoint = r.at(i.t);

	for (int chan = 0; chan < 3; chan++) {
		color[chan] += ka[chan] * ambientColor[chan]; // Ambience
		color[chan] += ke[chan]; // Emission
	}

	for (auto it = scene->beginLights(); it != scene->endLights(); it++) {
		Light* light = *it;

		vec3f N = i.N.normalize();
		vec3f L = light->getDirection(intersectionPoint).normalize();

		float lambertian = max(N.dot(L), 0.0);

		float specular = 0.0;

		double distanceAtten = light->distanceAttenuation(intersectionPoint);
		vec3f shadowAtten = light->shadowAttenuation(intersectionPoint);

		if (lambertian > 0) {
			vec3f R = (- L + 2 * L.dot(N) * N); // Reflected light vector
			vec3f V = -r.getDirection().normalize(); // Vector to viewer

			// Compute the specular term
			float specAngle = max(R.dot(V), 0.0);
			specular = pow(specAngle, shininess);
		}

		for (int chan = 0; chan < 3; chan++) {
			color[chan] += ((kd[chan] * lambertian * light->getColor(intersectionPoint)[chan]) + // Diffuse
				(ks[chan] * specular * light->getColor(intersectionPoint)[chan])) // Specular
				* distanceAtten * shadowAtten[0];
		}
	}


	return color;
}
