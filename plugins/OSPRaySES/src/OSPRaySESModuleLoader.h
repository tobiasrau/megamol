#ifndef OSPRAY_SES_MODULE_LOADER_H_
#define OSPRAY_SES_MODULE_LOADER_H_

namespace megamol {
  namespace ospray {
    namespace ses {

      class OSPRaySESModuleLoader {

      public:

        static bool LoadModule();

      private:

        static bool loaded;

      };

    }
  }
}


#endif