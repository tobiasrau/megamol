#pragma once

#ifndef _OSP_RAY_SES_RENDERER_
#define _OSP_RAY_SES_RENDERER_

#include <vector>

#include "vislib/graphics/gl/GLSLShader.h"
#include "vislib/graphics/CameraParameters.h"
#include "AbstractOSPRayTestRenderer.h"
#include "mmcore/param/ParamSlot.h"

#include "Common.h"
#include "OSPRayPrimitives.h"

namespace megamol {
  namespace ospray {
    namespace ses {

      class OSPRaySESRenderer : public AbstractOSPRayTestRenderer {
      public:

        /**
        * Answer the name of this module.
        *
        * @return The name of this module.
        */
        static const char *ClassName(void) {
          return "OSPRaySESRenderer";
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

        OSPRaySESRenderer();

        ~OSPRaySESRenderer();



      protected:

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
        bool LoadData(float time);


          bool dirty;

        std::vector<OSPRaySESSphere> sesSpheres;

        std::vector<vec4f> colors;


                /** MolecularDataCall caller slot */
        megamol::core::CallerSlot molDataCallerSlot;
        megamol::core::param::ParamSlot triggerDirtySlot;

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

        float currentTime;

        SIZE_T dataHash;

      };
    }
  }
}

#endif
