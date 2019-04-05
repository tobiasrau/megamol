/*
 * PDBCellLoader.h
 *
 * Copyright (C) 2019 by University of Stuttgart (VISUS).
 * All rights reserved.
 */


#pragma once

#include "mmcore/CalleeSlot.h"
#include "mmcore/CallerSlot.h"
#include "mmcore/param/ParamSlot.h"
#include "mmcore/view/AnimDataModule.h"
#include "protein_calls/CellDataCall.h"
#include "protein_calls/MolecularDataCall.h"
#include "vislib/Array.h"
#include "vislib/math/Cuboid.h"
#include <random>

namespace megamol {
namespace protein {

class PDBLoader;

/**
 * Data source for multiple PDB files
 */
class PDBCellLoader : public megamol::core::Module {
public:
    PDBCellLoader(void);
    virtual ~PDBCellLoader(void);

    /**
     * Answer the name of this module.
     *
     * @return The name of this module.
     */
    static const char* ClassName(void) { return "PDBCellLoader"; }

    /**
     * Answer a human readable description of this module.
     *
     * @return A human readable description of this module.
     */
    static const char* Description(void) { return "Offers multiple PDB data sets for whole cell visualization."; }

    /**
     * Answers whether this module is available on the current system.
     *
     * @return 'true' if the module is available, 'false' otherwise.
     */
    static bool IsAvailable(void) { return true; }

protected:
    /**
     * Implementation of 'Create'.
     *
     * @return 'true' on success, 'false' otherwise.
     */
    virtual bool create(void);

    /**
     * Implementation of 'Release'.
     */
    virtual void release(void);

    /**
     * Call callback to get the data
     *
     * @param c The calling call
     *
     * @return True on success
     */
    bool getData(core::Call& call);


    /**
     * Call callback to get the extent of the data
     *
     * @param c The calling call
     *
     * @return True on success
     */
    bool getExtent(core::Call& call);

private:
    /** Contains transformations and molecule infos (usually *.apr.json) */
    core::param::ParamSlot jsonFileSlot;

    /** Separate each pdb with a ";"  */
    core::param::ParamSlot pdbFileSlot;

    /** Does the json file contain transformation matrices or quaternions*/
    core::param::ParamSlot transKindSlot;

    /** The data callee slot */
    core::CalleeSlot cellDataOutSlot;

    /** The data hash for pdb data */
    SIZE_T dataHash;

    /* a pointer to the real data, required to change atom positions! */
    std::map<std::string, PDBLoader*> pdb;

    /* transformations */
    std::map<std::string, protein_calls::instContainer> data;

    /* map molecule name to transformation key */
    //std::string getTransformationKey(std::string mol);
};


} /* end namespace protein */
} /* end namespace megamol */
