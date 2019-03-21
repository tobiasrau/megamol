#ifndef _OSP_RAY_PRIMITIVES_H_
#define _OSP_RAY_PRIMITIVES_H_

#include <vector>
#include "Common.h"

namespace megamol {
	namespace ospray {
		namespace ses {
			struct OSPRaySphere {
        vec3f center;
        int colorID{ 0 };
        float radius;
      };

      struct OSPRayTorus {
        vec3f center;
        float r;
        float R;
        vec4f rotQuaternion;
        float angles[2];
      };

      struct OSPRaySESSphere {
        vec3f center;
        float radius;
        int colorID;
      };

      struct ColorMapping {
        std::vector<int> colorIDs;
        std::vector<vec4f> colors;
      };

      struct OSPRaySESSpheresAdditionalData {
        std::vector<int> toroidalPatchIndicesBeginIndex;
        std::vector<int> toroidalPatchIndicesCount;
        std::vector<int> contourPointsBeginIndex;
        std::vector<int> contourPointsCount;
      };

      struct OSPRaySESSphericalTriangle {
        vec3f center;
        int cuttingPlaneIndices[3];
        int neighborIndicesBeginIndex;
        int neighborIndicesCount;
        int sphereIndices[3];
      };

      struct OSPRaySESToroidalPatch {
        vec3f center;
        float R;
        vec4f rotQuaternion;
        vec4f visibilitySphere;
        int sphereIndices[2];
        int cuttingPlaneIndices[2];
        float angle;
      };
		}
	}
}

#endif