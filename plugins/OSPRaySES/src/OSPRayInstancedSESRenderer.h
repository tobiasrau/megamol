#pragma once

#ifndef _OSP_RAY_SES_INSTANCED_SES_RENDERER_H_
#define _OSP_RAY_SES_INSTANCED_SES_RENDERER_H_

#include <vector>

#include "vislib/graphics/gl/GLSLShader.h"
#include "vislib/graphics/CameraParameters.h"
#include "AbstractOSPRayTestRenderer.h"

#include "Common.h"
#include "OSPRayPrimitives.h"

namespace megamol {
  namespace ospray {
    namespace ses {

      class OSPRayInstancedSESRenderer : public AbstractOSPRayTestRenderer {
      public:

        /**
        * Answer the name of this module.
        *
        * @return The name of this module.
        */
        static const char *ClassName(void) {
          return "OSPRayInstancedSESRenderer";
        }

        /**
        * Answer a human readable description of this module.
        *
        * @return A human readable description of this module.
        */
        static const char *Description(void) {
          return "SES Rendering with ospray-internal contour computation.";
        }
        /**
        * Answers whether this module is available on the current system.
        *
        * @return 'true' if the module is available, 'false' otherwise.
        */
        static bool IsAvailable(void) {
          return vislib::graphics::gl::GLSLShader::AreExtensionsAvailable();
        }

        OSPRayInstancedSESRenderer();

        ~OSPRayInstancedSESRenderer();

      protected:

        enum ComputationMode {
          CPU = 0,
          VECTORIZED,
          PARTIALLY_VECTORIZED
        };

        /**
        * The render callback.
        *
        * @param call The calling call.
        *
        * @return The return value of the function.
        */
        bool Render(megamol::core::Call& call);

        void DestroyOSP();

        /**
        * Implement to get spheres from somewhere.
        */
        bool LoadData();

        /**
        * Indicates recomputation of the surface should happen.
        */
        bool recompute;

        bool dataLoaded;

        std::vector<OSPRaySESSphere> sesSpheres;

        std::vector<vec4f> colors;

        megamol::core::param::ParamSlot numInstancesPerDimensionSlot;
        megamol::core::param::ParamSlot instanceSpacingSlot;
      private:

        /**
        * The get extents callback. The module should set the members of
        * 'call' to tell the caller the extents of its data (bounding boxes
        * and times).
        *
        * @param call The calling call.
        *
        * @return The return value of the function.
        */
        bool GetExtents(megamol::core::Call& call);

        /** MolecularDataCall caller slot */
        megamol::core::CallerSlot molDataCallerSlot;
      };
    }
  }
}

#endif
