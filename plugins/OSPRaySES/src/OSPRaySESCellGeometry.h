#pragma once

#include <vector>

#include <map>
#include "Common.h"
#include "OSPRayPrimitives.h"
#include "OSPRay_plugin/AbstractOSPRayStructure.h"
#include "mmcore/param/ParamSlot.h"
#include "protein_calls/CellDataCall.h"

namespace megamol {
namespace ospray {
namespace ses {

class OSPRaySESCellGeometry : public core::Module {
public:
    /**
     * Answer the name of this module.
     *
     * @return The name of this module.
     */
    static const char* ClassName(void) { return "OSPRaySESCellGeometry"; }

    /**
     * Answer a human readable description of this module.
     *
     * @return A human readable description of this module.
     */
    static const char* Description(void) { return "SES Rendering of whole cells as Geometry"; }
    /**
     * Answers whether this module is available on the current system.
     *
     * @return 'true' if the module is available, 'false' otherwise.
     */
    static bool IsAvailable(void) { return true; }

    OSPRaySESCellGeometry();

    virtual ~OSPRaySESCellGeometry();

protected:
    virtual bool create() { return true; }
    virtual void release() {
        // empty
    }

    bool getDataCallback(core::Call& call);
    bool getExtendsCallback(core::Call& call);
    bool getDirtyCallback(core::Call& call);
    void SetSurfaceParams(OSPGeometry sesGeometry);

    bool InterfaceIsDirty();

    bool InterfaceIsDirtyNoReset() const;

    /**
     * Implement to get spheres from somewhere.
     */
    bool LoadMolecule(std::string);

    /**
     * Indicates recomputation of the surface should happen.
     */
    bool recompute;
    bool initialData;
    bool triggered;

    std::map<std::string, std::vector<OSPRaySESSphere>> sesSpheres;

    std::map<std::string, std::vector<vec4f>> colors;
    // SES or SPHERES
    core::param::ParamSlot renderAsSlot;
    // Molecule selector
    core::param::ParamSlot removeSlot;
    core::param::ParamSlot addSlot;
    core::param::ParamSlot triggerRemoveSlot;
    core::param::ParamSlot triggerAddSlot;
    core::param::ParamSlot triggerShowAllSlot;
    core::param::ParamSlot triggerHideAllSot;
    // Chunkyfier
    core::param::ParamSlot numChunksSlot;
    core::param::ParamSlot chunkIDSlot;
    core::param::ParamSlot onlyChunkSlot;
    // SES PARAMETERS
    core::param::ParamSlot epsilonCuttingThresholdSlot;
    core::param::ParamSlot epsilonSelfIntersectSlot;


    /** MolecularDataCall caller slot */
    core::CallerSlot molDataCallerSlot;
    core::CalleeSlot deployStructureSlot;

private:

    bool triggerRemoveCallback(megamol::core::param::ParamSlot& slot);
    bool triggerAddCallback(megamol::core::param::ParamSlot& slot);
    bool triggerShowAllCallback(megamol::core::param::ParamSlot &slot);
    bool triggerHideAllCallback(megamol::core::param::ParamSlot &slot);
    void createChunks();

    std::map<std::string, OSPModel> model;
    std::map<std::string, OSPGeometry> sesGeometry;
    std::map<std::string, OSPData> sphereData;
    std::map<std::string, OSPData> sphereColorData;
    std::map<std::string, std::vector<OSPGeometry>> instances;
    size_t datahash;
    std::vector<vislib::math::Cuboid<float>> chunks;
    vislib::math::Cuboid<float> bbox;

    std::map<std::string, bool> show;

};
} // namespace ses
} // namespace ospray
} // namespace megamol
