#include "stdafx.h"
#include "mmcore/moldyn/BindingSiteCall.h"

using namespace megamol;
using namespace megamol::core;
using namespace megamol::core::moldyn;

/*
 * BindingSiteCall::BindingSiteCall
 */
const unsigned int BindingSiteCall::CallForGetData = 0;


BindingSiteCall::BindingSiteCall(void) : megamol::core::Call(),
        bindingSites(NULL), bindingSiteResNames(NULL), bindingSiteNames(NULL),
        bindingSiteDescriptions(NULL), bindingSiteColors(NULL) {
}


BindingSiteCall::~BindingSiteCall(void) {
    this->bindingSites = NULL;
    this->bindingSiteResNames = NULL;
    this->bindingSiteNames = NULL;
    this->bindingSiteDescriptions = NULL;
    this->bindingSiteColors = NULL;
}
