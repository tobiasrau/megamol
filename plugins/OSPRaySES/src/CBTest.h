#pragma once

#ifndef _CB_TEST_H_
#define _CB_TEST_H_

#include <vector>
#include <random>

#include "vislib/graphics/gl/GLSLShader.h"
#include "vislib/graphics/CameraParameters.h"
#include "vislib/math/Quaternion.h"
#include "mmcore/param/ParamSlot.h"
#include "AbstractOSPRaySESRenderer.h"

namespace megamol {
  namespace ospray {
    namespace ses {

      class CBTest : public AbstractOSPRaySESRenderer {
      public:

        /**
        * Answer the name of this module.
        *
        * @return The name of this module.
        */
        static const char *ClassName(void) {
          return "CBTest";
        }

        /**
        * Answer a human readable description of this module.
        *
        * @return A human readable description of this module.
        */
        static const char *Description(void) {
          return "Debug CB Algorithmn.";
        }

        /**
        * Answers whether this module is available on the current system.
        *
        * @return 'true' if the module is available, 'false' otherwise.
        */
        static bool IsAvailable(void) {
          return vislib::graphics::gl::GLSLShader::AreExtensionsAvailable();
        }

        CBTest();

        virtual ~CBTest();

        virtual bool LoadData();

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
        virtual bool GetExtents(megamol::core::Call& call);

        bool PushSphereCallback(megamol::core::param::ParamSlot &paramSlot);

        bool PopSphereCallback(megamol::core::param::ParamSlot &paramSlot);

        bool PushRandomSphereCallback(megamol::core::param::ParamSlot &paramSlot);

        std::vector<OSPRaySESSphere> spheres;

        std::default_random_engine generator;
        std::uniform_real_distribution<float> distribution;

        megamol::core::param::ParamSlot sphereParamSlot;
        megamol::core::param::ParamSlot radiusParamSlot;
        megamol::core::param::ParamSlot randPositionParamSlot;
        megamol::core::param::ParamSlot pushSphereSlot;
        megamol::core::param::ParamSlot popSphereSlot;
        megamol::core::param::ParamSlot pushRandomSphereSlot;
      };
    }
  }
}

#endif
