#pragma once

#ifndef _ABSTRACT_OSP_RAY_SES_RENDERER_
#define _ABSTRACT_OSP_RAY_SES_RENDERER_

#include <vector>

#include "vislib/graphics/gl/GLSLShader.h"
#include "vislib/graphics/CameraParameters.h"
#include "AbstractOSPRayTestRenderer.h"

#include "Common.h"
#include "OSPRayPrimitives.h"

namespace megamol {
  namespace ospray {
    namespace ses {

      class AbstractOSPRaySESRenderer : public AbstractOSPRayTestRenderer {
      public:

        /**
        * Answers whether this module is available on the current system.
        *
        * @return 'true' if the module is available, 'false' otherwise.
        */
        static bool IsAvailable(void) {
          return vislib::graphics::gl::GLSLShader::AreExtensionsAvailable();
        }

        AbstractOSPRaySESRenderer();

        virtual ~AbstractOSPRaySESRenderer();

      protected:

        /**
        * The render callback.
        *
        * @param call The calling call.
        *
        * @return The return value of the function.
        */
        virtual bool Render(megamol::core::Call& call);

        void DestroyOSP();

        /**
        * Implement to get spheres from somewhere.
        */
        virtual bool LoadData() = 0;

        /**
        * Indicates recomputation of the surface should happen.
        */
        bool recompute;

        bool dataLoaded;

        std::vector<OSPRaySESSphere> sesSpheres;
        std::vector<vec4f> colors;


      };
    }
  }
}

#endif
