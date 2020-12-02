/*
 * TableToProbes.h
 * Copyright (C) 2020 by MegaMol Team
 * Alle Rechte vorbehalten.
 */

#pragma once

#include "mmcore/CalleeSlot.h"
#include "mmcore/CallerSlot.h"
#include "mmcore/Module.h"
#include "mmcore/param/ParamSlot.h"
#include "mesh/MeshCalls.h"
#include "ProbeCollection.h"
#include "probe/MeshUtilities.h"
#include "adios_plugin/CallADIOSData.h"
#include "mmstd_datatools/table/TableDataCall.h"

namespace megamol {
namespace probe {

class TableToProbes : public core::Module {
public:
    /**
     * Answer the name of this module.
     *
     * @return The name of this module.
     */
    static const char* ClassName() { return "TableToProbes"; }

    /**
     * Answer a human readable description of this module.
     *
     * @return A human readable description of this module.
     */
    static const char* Description() { return "..."; }

    /**
     * Answers whether this module is available on the current system.
     *
     * @return 'true' if the module is available, 'false' otherwise.
     */
    static bool IsAvailable(void) { return true; }

    /** Ctor. */
    TableToProbes();

    /** Dtor. */
    virtual ~TableToProbes();

protected:
    virtual bool create();
    virtual void release();

    uint32_t _version;

    core::CalleeSlot _probe_slot;

    core::CallerSlot _table_slot;

    core::param::ParamSlot _accumulate_clustered_slot;

private:
    bool getData(core::Call& call);

    bool getMetaData(core::Call& call);
    bool generateProbes();

    const float* _table;
    std::vector<std::vector<float>> _accum_probes;
    const stdplugin::datatools::table::TableDataCall::ColumnInfo* _col_info;
    uint32_t _num_cols;
    uint32_t _num_rows;
    std::shared_ptr<ProbeCollection> _probes;

    std::array<float, 3> _whd;
    core::BoundingBoxes_2 _bbox;

    size_t _table_data_hash;
};


} // namespace probe
} // namespace megamol