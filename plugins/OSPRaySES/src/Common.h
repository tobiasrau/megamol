#ifndef _SES_COMMON_H_
#define _SES_COMMON_H_

#include <algorithm>

#include "ospray/ospcommon/vec.h"
#include "ospray/ospcommon/Quaternion.h"
#include "ospray/ospcommon/box.h"
#include "ospray/ospcommon/AffineSpace.h"

namespace osp {
  typedef ospcommon::vec2f vec2f;
  typedef ospcommon::vec2d vec2d;
  typedef ospcommon::vec2i vec2i;

  typedef ospcommon::vec3f vec3f;
  typedef ospcommon::vec3d vec3d;
  typedef ospcommon::vec3i vec3i;

  typedef ospcommon::vec4f vec4f;
  typedef ospcommon::vec4d vec4d;
  typedef ospcommon::vec4i vec4i;

  typedef ospcommon::box3f box3f;
  typedef ospcommon::box3i box3i;

  typedef ospcommon::Quaternion3f quaternion3f;

  typedef ospcommon::affine3f affine3f;
}

#define OSPRAY_EXTERNAL_VECTOR_TYPES
#include "ospray/ospray.h"

namespace megamol {
	namespace ospray {
		namespace ses {
      typedef osp::vec2f vec2f;
      typedef osp::vec2d vec2d;
      typedef osp::vec2i vec2i;

			typedef osp::vec3f vec3f;
      typedef osp::vec3d vec3d;
			typedef osp::vec3i vec3i;

			typedef osp::vec4f vec4f;
      typedef osp::vec4d vec4d;
      typedef osp::vec4i vec4i;
      typedef osp::quaternion3f Quaternion;

      struct CirclePseudoAngleHelper {
        vec3d v_0;
        vec3d v_0_cross_n;
        double v_0_dot_v_0;
      };

      inline Quaternion CreateQuaternion(const float rotationAngle, const vec3f &rotationAxis) {
        return Quaternion(cos(rotationAngle / 2.0f), sin(rotationAngle / 2.0f)*normalize(rotationAxis));
      }

      inline vec4f CreateOSPRayQuaternion(Quaternion &q) {
				return vec4f{ q.r, q.i, q.j, q.k };
			}

      inline float unitCircleAtan2(float y, float x) {
        float theta = std::atan2(y, x);
        return theta >= 0.0f ? theta : 2 * static_cast<float>(M_PI) + theta;
      }

      inline bool areParallel(const vec3d &v1, const vec3d &v2, float epsilon) {
        return std::abs(1.0f - std::abs(dot((v1 / length(v1)),(v2 / length(v2))))) < epsilon;
      }

      inline bool areParallel(const vec3f &v1, const vec3f &v2, float epsilon) {
        return std::fabs(1.0f - std::fabs(dot((v1 / length(v1)), (v2 / length(v2))))) < epsilon;
      }

      inline float angle(const vec3f &v1, const vec3f &v2) {
        return acos(dot(normalize(v1), normalize(v2)));
      }

      inline double angle(const vec3d &v1, const vec3d &v2) {
        return acos(dot(normalize(v1), normalize(v2)));
      }

      inline vec3f min(const vec3f &a, const vec3f &b) {
        return vec3f(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z));
      }

      inline vec3f max(const vec3f &a, const vec3f &b) {
        return vec3f(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z));
      }

      inline vec3d min(const vec3d &a, const vec3d &b) {
        return vec3d(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z));
      }

      inline vec3d max(const vec3d &a, const vec3d &b) {
        return vec3d(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z));
      }

      inline void CirclePseudoAngleHelper_init(CirclePseudoAngleHelper &cpah, const vec3d &v_0, const vec3d &n) {
        cpah.v_0 = v_0;
        cpah.v_0_cross_n = cross(v_0, n);
        cpah.v_0_dot_v_0 = dot(v_0,v_0);
      }

      inline double CirclePseudoAngleHelper_computePseudoAngle(CirclePseudoAngleHelper &cpah, const vec3d &p) {
        // Calculate angles, this is done by constructing two planes:
        // The first plane is has the first arcs vector (v_0) as normal, the second the cross product of v_0 and circle j's normal
        // Now every point on the circle lies in one of the four quadrants, for which a pseudo angle is computed
        // The pseudo angle is always between [0, v_0 dot v_0) in the quadrant, because all points relative to the circle center v are of same length 
        // and |a cross b| = |a||b| times sin theta = a dot a times sin theta, therefore we move a dot a times sin theta in each quadrant
        const double u = dot(cpah.v_0, p);
        const double v = dot(cpah.v_0_cross_n,p);
        const double w = length(cross(cpah.v_0, p)); // w can be in [0, v_0 dot v_0)
        if (u > 0.0) {
          if (v > 0.0) {
            // 1st quadrant
            return w;
          } else { // v <= 0.0f
            return 4 * cpah.v_0_dot_v_0 - w;
          }
        } else { // u <= 0.0f
          if (v > 0.0) {
            // 2nd quadrant
            return 2 * cpah.v_0_dot_v_0 - w;
          } else { // v <= 0.0f
                   // 3rd quadrant
            return 2 * cpah.v_0_dot_v_0 + w;
          }
        }
      }

      inline vec4f ComputeCuttingPlane(const vec3f &center, const vec3f &v1, const vec3f &v2, const vec3f &witness) {
        vec3f v1_os = v1 - center;
        vec3f v2_os = v2 - center;

        // hesse normal form
        vec3f normal = cross(v1_os,v2_os);
        vec3f n_0 = normalize(dot(v1, normal) >= 0 ? normal : (-1.0f)*normal);
        float d = dot(v1, n_0);

        // make sure the normal looks inward
        if (dot(n_0, witness) - d < 0.0f) {
          d = -d;
          n_0 = (-1.0f)*n_0;
        }

        return vec4f(n_0.x, n_0.y, n_0.z, d );
      }


		}
	}
}

#endif
