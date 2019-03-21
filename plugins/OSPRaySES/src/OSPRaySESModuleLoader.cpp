#include "OSPRaySESModuleLoader.h"
#include "Common.h"

using namespace megamol::ospray::ses;

bool OSPRaySESModuleLoader::loaded = false;

bool OSPRaySESModuleLoader::LoadModule() {
  // TODO use vislib logging mechanism
  puts("Attempting to load ses module...");
  if (OSPRaySESModuleLoader::loaded) {
    puts("Module already loaded");
    return true;
  }

  OSPError loadError = ospLoadModule("ses");

  if (loadError != OSP_NO_ERROR) {
    printf("Loading falied, ospLoadModule returned: %d\n", loadError);
    return false;
  }
  puts("Module loaded successfully");
  return true;
}