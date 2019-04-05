/*
 * CellDataCall.h
 *
 * Copyright (C) 2019 by University of Stuttgart (VISUS).
 * All rights reserved.
 */

#pragma once

#include <array>
#include <map>
#include "MolecularDataCall.h"
#include "mmcore/Call.h"
#include "mmcore/factories/CallAutoDescription.h"

namespace megamol {
namespace protein_calls {

struct PROTEIN_CALLS_API transform {
    std::array<float, 3> pos;
    std::array<std::array<float, 3>, 3> v;
};

struct PROTEIN_CALLS_API instContainer {
    std::string structureName;
    std::string componentName;
    std::string pdbIdentifier;
    std::string pdbFile;
    MolecularDataCall* call;
    std::vector<transform> transformations;
    std::array<float, 3> transformTranslate = {0,0,0};
    std::array<float, 4> transformRotate = {1,0,0,0};
    bool transformCenter = false;
    std::array<float, 3> center;
    std::array<float, 4> color;
};




class PROTEIN_CALLS_API CellDataCall : public core::Call {
public:
    /**
     * Answer the name of the objects of this description.
     *
     * @return The name of the objects of this description.
     */
    static const char* ClassName(void) { return "CellDataCall"; }

    /**
     * Gets a human readable description of the module.
     *
     * @return A human readable description of the module.
     */
    static const char* Description(void) { return "Call cell data"; }

    /**
     * Answer the number of functions used for this call.
     *
     * @return The number of functions used for this call.
     */
    static unsigned int FunctionCount(void) { return 2; }

    /**
     * Answer the name of the function used for this call.
     *
     * @param idx The index of the function to return it's name.
     *
     * @return The name of the requested function.
     */
    static const char* FunctionName(unsigned int idx) {
        switch (idx) {
        case 0:
            return "GetCall";
        case 1:
            return "GetExtend";
        default:
            return NULL;
        }
    }

    /** Ctor. */
    CellDataCall() {
        // empty
    }

    /** Dtor. */
    virtual ~CellDataCall(void) {
        // empty
    }

    void SetDataHash(size_t _datahash) { this->datahash = _datahash; }
    size_t GetDataHash() const { return this->datahash; }

    std::shared_ptr<std::map<std::string, instContainer>> data;

    size_t datahash;
};

/** Description class typedef */
typedef megamol::core::factories::CallAutoDescription<CellDataCall> CellDataCallDescription;

} /* end namespace protein_calls */
} /* end namespace megamol */
