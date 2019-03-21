#pragma once

#ifndef _OSPRAY_SES_GEOMETRY_H_
#define _OSPRAY_SES_GEOMETRY_H_

#include <vector>
#include <memory>

#include "mmcore/Module.h"
#include "mmcore/Call.h"
#include "mmcore/CallerSlot.h"
#include "mmcore/CalleeSlot.h"
#include "mmcore/param/ParamSlot.h"
#include "OSPRaySESModuleLoader.h"

namespace megamol {
  namespace ospray {
    namespace ses {

      class OSPRaySESGeometry : public megamol::core::Module {

      public:

          /**
          * Answer the name of this module.
          *
          * @return The name of this module.
          */
          static const char *ClassName(void) {
            return "OSPRaySESGeometry";
          }

          /**
          * Answer a human readable description of this module.
          *
          * @return A human readable description of this module.
          */
          static const char *Description(void) {
            return "Container for OSPRay solvent excluded surface geometries.";
          }

          /**
          * Answers whether this module is available on the current system.
          *
          * @return 'true' if the module is available, 'false' otherwise.
          */
          static bool IsAvailable(void) {
            // TODO check for module here?
            return true;
          }

          /** Dtor. */
          virtual ~OSPRaySESGeometry(void);

          /** Ctor. */
          OSPRaySESGeometry(void);

      private:

        float currentTime;

        enum ComputationMode {
          CPU = 0,
          VECTORIZED,
          PARTIALLY_VECTORIZED
        };

        /** The call for the OSPRayApiObject */
        megamol::core::CalleeSlot deployOSPRayGeometrySlot;
        /** The call for data */
        megamol::core::CallerSlot getDataSlot;

        /** Create the geometry */
        bool GetData(megamol::core::Call &call);

        bool GetExtends(megamol::core::Call &call);

        bool GetDirty(megamol::core::Call &call);
          bool InterfaceIsDirty();
          bool InterfaceIsDirtyNoReset() const;

          bool create() {
          if (!OSPRaySESModuleLoader::LoadModule()) {
            puts("Could not load ospray module");
            return false;
          }
          
          return true;
        }
        void release() { /*this->Release();*/ }

        bool ParamUpdateCallback(megamol::core::param::ParamSlot &paramSlot);

        megamol::core::param::ParamSlot modeSlot;
        megamol::core::param::ParamSlot probeRadiusSlot;
        megamol::core::param::ParamSlot skipContourSingularitiesSlot;
        megamol::core::param::ParamSlot resizeVectorsSlot;
        megamol::core::param::ParamSlot computeSingularitiesSlot;
        megamol::core::param::ParamSlot debugFlagSlot;
        megamol::core::param::ParamSlot renderContourArcsSlot;

      };

    }
  }
}

#endif