#include <cmath>
#include <assert.h>

#include "Box.h"

bool Box::intersectLocal( const ray& r, isect& i ) const
{
	// YOUR CODE HERE:
    // Add box intersection code here.
	// it currently ignores all boxes and just returns false.

    BoundingBox bbox = ComputeLocalBoundingBox();

    double tnear;
    double tfar;


    // No intersection  
    if (!bbox.intersect(r, tnear, tfar)) {
        return false;
    }

    // Intersection
    if (tnear > RAY_EPSILON) {
        // Intersection at tnear
        i.obj = this;
        i.setT(tnear);
        vec3f normal;
        vec3f intersectionPoint = r.at(tnear);

        for (int axis = 0; axis < 3; ++axis) {
            if (abs(intersectionPoint[axis] - bbox.min[axis]) < RAY_EPSILON) {
                normal[axis] = -1.0;
            }
            else if (abs(intersectionPoint[axis] - bbox.max[axis]) < RAY_EPSILON) {
                normal[axis] = 1.0;
            }
            else {
                normal[axis] = 0.0;
            }
        }

        i.setN(normal);
    }
    else {
        // Intersection is at tfar
        i.obj = this;
        i.setT(tfar);
        vec3f normal;
        vec3f intersectionPoint = r.at(tfar);

        for (int axis = 0; axis < 3; ++axis) {
            if (abs(intersectionPoint[axis] - bbox.min[axis]) < RAY_EPSILON) {
                normal[axis] = -1.0;
            }
            else if (abs(intersectionPoint[axis] - bbox.max[axis]) < RAY_EPSILON) {
                normal[axis] = 1.0;
            }
            else {
                normal[axis] = 0.0;
            }
        }

        i.setN(normal);
    }

    return true;
}
