#include <iostream>
#include <stdio.h>

#include <limits>

#include "OSPRaySESCellGeometry.h"
#include "OSPRaySESModuleLoader.h"
#include "OSPRay_plugin/CallOSPRayAPIObject.h"
#include "mmcore/CoreInstance.h"
#include "mmcore/param/ButtonParam.h"
#include "mmcore/param/BoolParam.h"
#include "mmcore/param/EnumParam.h"
#include "mmcore/param/FlexEnumParam.h"
#include "mmcore/param/FloatParam.h"
#include "mmcore/param/IntParam.h"
#include "mmcore/param/ParamSlot.h"
#include "mmcore/view/CallRender3D.h"
#include "protein_calls/CellDataCall.h"
#include "vislib/math/Cuboid.h"
#include "vislib/math/Quaternion.h"
#include "vislib/math/Vector.h"

using namespace megamol::ospray::ses;

OSPRaySESCellGeometry::OSPRaySESCellGeometry()
    : renderAsSlot("renderAs", "Spheres or SES")
    , molDataCallerSlot("getData", "Protein data input.")
    , deployStructureSlot("deployStructureSlot", "Connects to an OSPRayAPIStructure")
    , addSlot("add", "Select molecule to add")
    , removeSlot("remove", "Select molecule to remove")
    , triggerAddSlot("triggerAdd", "Triggers the add action")
    , triggerRemoveSlot("triggerRemove", "Triggers the remove acion")
    , triggerShowAllSlot("showAll", "Shows all molecules")
    , triggerHideAllSot("hideAll", "Hides all molecules")
    , numChunksSlot("numChunks", "Defines the spacial splitting")
    , chunkIDSlot("chunkID", "Sets the chunk ID to display")
    , onlyChunkSlot("onlyChunk", "Sets if only a single chunk should be displayed")
    , epsilonCuttingThresholdSlot("SES::epsilonCuttingThreshold", "Secondary epsilon")
    , epsilonSelfIntersectSlot("SES::epsilonSelfIntersect", "Third epsilon") {

    this->recompute = true;
    this->initialData = true;
    this->triggered = false;
    // ParameterSlots
    // Render as slot
    enum ras { SES = 0, SPHERES = 1 };
    megamol::core::param::EnumParam* rasParam = new megamol::core::param::EnumParam(ras::SES);
    rasParam->SetTypePair(ras::SES, "SES");
    rasParam->SetTypePair(ras::SPHERES, "SPHERES");
    this->renderAsSlot << rasParam;
    this->MakeSlotAvailable(&this->renderAsSlot);

    // Molecule type display mechanic
    this->addSlot << new core::param::FlexEnumParam("undef");
    this->MakeSlotAvailable(&this->addSlot);

    this->removeSlot << new core::param::FlexEnumParam("undef");
    this->MakeSlotAvailable(&this->removeSlot);

    this->triggerAddSlot << new core::param::ButtonParam();
    this->triggerAddSlot.SetUpdateCallback(this, &OSPRaySESCellGeometry::triggerAddCallback);
    this->MakeSlotAvailable(&this->triggerAddSlot);

    this->triggerRemoveSlot << new core::param::ButtonParam();
    this->triggerRemoveSlot.SetUpdateCallback(this, &OSPRaySESCellGeometry::triggerRemoveCallback);
    this->MakeSlotAvailable(&this->triggerRemoveSlot);

    this->triggerShowAllSlot << new core::param::ButtonParam();
    this->triggerShowAllSlot.SetUpdateCallback(this, &OSPRaySESCellGeometry::triggerShowAllCallback);
    this->MakeSlotAvailable(&this->triggerShowAllSlot);

    this->triggerHideAllSot << new core::param::ButtonParam();
    this->triggerHideAllSot.SetUpdateCallback(this, &OSPRaySESCellGeometry::triggerHideAllCallback);
    this->MakeSlotAvailable(&this->triggerHideAllSot);

    // Chunk display mechanic
    enum numc { num8 = 8, num27 = 27, num64 = 64, num125 = 125, num216 = 216, num343 = 343, num512 = 512 };
    megamol::core::param::EnumParam* numcParam = new megamol::core::param::EnumParam(numc::num8);
    numcParam->SetTypePair(num8, "8");
    numcParam->SetTypePair(num27, "27");
    numcParam->SetTypePair(num64, "64");
    numcParam->SetTypePair(num125, "125");
    numcParam->SetTypePair(num216, "216");
    numcParam->SetTypePair(num343, "343");
    numcParam->SetTypePair(num512, "512");
    this->numChunksSlot << numcParam;
    this->MakeSlotAvailable(&this->numChunksSlot);

    this->chunkIDSlot << new core::param::IntParam(0);
    this->MakeSlotAvailable(&this->chunkIDSlot);

    this->onlyChunkSlot << new core::param::BoolParam(false);
    this->MakeSlotAvailable(&this->onlyChunkSlot);

    // SES parameter
    this->epsilonCuttingThresholdSlot << new megamol::core::param::FloatParam(0.001f, 0.00f, 1.0f);
    this->MakeSlotAvailable(&this->epsilonCuttingThresholdSlot);

    this->epsilonSelfIntersectSlot << new megamol::core::param::FloatParam(0.001f, 0.00f, 1.0f);
    this->MakeSlotAvailable(&this->epsilonSelfIntersectSlot);


    // CallSlots
    this->molDataCallerSlot.SetCompatibleCall<megamol::protein_calls::CellDataCallDescription>();
    this->MakeSlotAvailable(&this->molDataCallerSlot);

    this->deployStructureSlot.SetCallback(CallOSPRayAPIObject::ClassName(), CallOSPRayAPIObject::FunctionName(0),
        &OSPRaySESCellGeometry::getDataCallback);
    this->deployStructureSlot.SetCallback(CallOSPRayAPIObject::ClassName(), CallOSPRayAPIObject::FunctionName(1),
        &OSPRaySESCellGeometry::getExtendsCallback);
    this->deployStructureSlot.SetCallback(CallOSPRayAPIObject::ClassName(), CallOSPRayAPIObject::FunctionName(2),
        &OSPRaySESCellGeometry::getDirtyCallback);
    this->MakeSlotAvailable(&this->deployStructureSlot);
}

OSPRaySESCellGeometry::~OSPRaySESCellGeometry() {
    for (auto& ses : sesGeometry) {
        ospRelease(sphereData[ses.first]);
        ospRelease(sphereColorData[ses.first]);

        ospRemoveGeometry(model[ses.first], sesGeometry[ses.first]);
        ospRelease(model[ses.first]);
    }

    model.clear();
    sesGeometry.clear();
    sphereData.clear();
    sphereColorData.clear();
    instances.clear();
    this->Release();
}

bool OSPRaySESCellGeometry::getDataCallback(megamol::core::Call& call) {

    auto os = dynamic_cast<CallOSPRayAPIObject*>(&call);
    if (os == nullptr) return false;

    protein_calls::CellDataCall* cd = this->molDataCallerSlot.CallAs<protein_calls::CellDataCall>();
    if (cd == nullptr) return false;

    OSPRaySESModuleLoader::LoadModule();

    // Check dirtyness
    this->recompute = this->recompute || this->InterfaceIsDirty() || this->triggered;

    if (this->recompute) {
        this->triggered = false;

        if (this->onlyChunkSlot.Param<core::param::BoolParam>()->Value()) {
            this->createChunks();
        }

        if (this->initialData) {
            this->addSlot.Param<core::param::FlexEnumParam>()->ClearValues();
            this->removeSlot.Param<core::param::FlexEnumParam>()->ClearValues();
            this->addSlot.Param<core::param::FlexEnumParam>()->ParseValue("");
            this->removeSlot.Param<core::param::FlexEnumParam>()->ParseValue("");

            for (auto& mol : *cd->data) {
                this->removeSlot.Param<core::param::FlexEnumParam>()->AddValue(mol.first);
                this->show[mol.first] = true;
            }
            this->initialData = false;
        }

        for (auto& ses : sesGeometry) {
            ospRelease(sphereData[ses.first]);
            ospRelease(sphereColorData[ses.first]);

            ospRemoveGeometry(model[ses.first], sesGeometry[ses.first]);
            ospRelease(model[ses.first]);
        }
        size_t totalInstances = 0;
        size_t totalAtoms = 0;
        model.clear();
        sesGeometry.clear();
        sphereData.clear();
        sphereColorData.clear();
        instances.clear();

        std::vector<OSPGeometry> geo;
        // Loop over all molecules
        size_t index = 0;
        for (auto& mol : *cd->data) {

            auto key = mol.first;

            if (!this->show[key]) continue;

            // if (!mol.second.transformCenter) continue;
            // if (mol.second.transformTranslate[0] == 0.0f && mol.second.transformTranslate[1] == 0.0f &&
            // mol.second.transformTranslate[2] == 0.0f) continue;

            if (!this->LoadMolecule(key)) {
                return false;
            }
            if (this->sesSpheres[key].empty()) {
                vislib::sys::Log::DefaultLog.WriteError("Skipping molecule %s. No atoms could be loaded.", key.c_str());
                continue;
            }

            model[key] = ospNewModel();

            if (this->renderAsSlot.Param<core::param::EnumParam>()->ValueString() == "SES") {

                sesGeometry[key] = ospNewGeometry("ses_surface");

                this->SetSurfaceParams(sesGeometry[key]);
                auto epsilonCuttingThreshold =
                    this->epsilonCuttingThresholdSlot.Param<megamol::core::param::FloatParam>()->Value();
                auto epsilonSelfIntersect =
                    this->epsilonSelfIntersectSlot.Param<megamol::core::param::FloatParam>()->Value();

                // spheres
                sphereData[key] = ospNewData(sizeof(OSPRaySESSphere) * this->sesSpheres[key].size(), OSP_UCHAR,
                    this->sesSpheres[key].data(), OSPDataCreationFlags::OSP_DATA_SHARED_BUFFER);
                sphereColorData[key] = ospNewData(this->colors[key].size(), OSP_FLOAT4, this->colors[key].data(),
                    OSPDataCreationFlags::OSP_DATA_SHARED_BUFFER);
                ospSet1i(sesGeometry[key], "bytes_per_ses_sphere", sizeof(OSPRaySESSphere));
                ospSet1i(sesGeometry[key], "ses_spheres_offset_center", offsetof(OSPRaySESSphere, center));
                ospSet1i(sesGeometry[key], "ses_spheres_offset_radius", offsetof(OSPRaySESSphere, radius));
                ospSet1i(sesGeometry[key], "ses_spheres_offset_colorID", offsetof(OSPRaySESSphere, colorID));
                ospSet1f(sesGeometry[key], "epsilon_cutting_threshold", epsilonCuttingThreshold);
                ospSet1f(sesGeometry[key], "epsilon_self_intersect", epsilonSelfIntersect);
                ospSetData(sesGeometry[key], "ses_spheres", sphereData[key]);
                ospSetData(sesGeometry[key], "colors", sphereColorData[key]);
            } else {
                sesGeometry[key] = ospNewGeometry("spheres");
                sphereData[key] = ospNewData(
                    sizeof(OSPRaySESSphere) * this->sesSpheres[key].size(), OSP_FLOAT3, this->sesSpheres[key].data());
                ospSet1i(sesGeometry[key], "bytes_per_ses_sphere", sizeof(OSPRaySESSphere));
                ospSetData(sesGeometry[key], "spheres", sphereData[key]);
                ospSet1i(sesGeometry[key], "offset_radius", offsetof(OSPRaySESSphere, radius));
                // ospSet1f(sesGeometry[key], "radius", 1);
                // ospSet3f(sesGeometry[key], "color", 0.8, 0.8, 0.8);
            }
            ospCommit(sesGeometry[key]);
            ospAddGeometry(model[key], sesGeometry[key]);
            ospCommit(model[key]);

            for (auto& trans : mol.second.transformations) {
                if (this->onlyChunkSlot.Param<core::param::BoolParam>()->Value()) {
                    int chunkID = this->chunkIDSlot.Param<core::param::IntParam>()->Value();
                    if (this->chunks[chunkID].Left() > trans.pos[0] || this->chunks[chunkID].Right() < trans.pos[0] ||
                        this->chunks[chunkID].Bottom() > trans.pos[1] || this->chunks[chunkID].Top() < trans.pos[1] || 
                        this->chunks[chunkID].Back() > trans.pos[2] || this->chunks[chunkID].Front() < trans.pos[2]
						) {

                        continue;
                        }
                    }
                osp::affine3f xfm;
                xfm.p.x = trans.pos[0];
                xfm.p.y = trans.pos[1];
                xfm.p.z = trans.pos[2];
                xfm.l.vx.x = trans.v[0][0];
                xfm.l.vx.y = trans.v[0][1];
                xfm.l.vx.z = trans.v[0][2];
                xfm.l.vy.x = trans.v[1][0];
                xfm.l.vy.y = trans.v[1][1];
                xfm.l.vy.z = trans.v[1][2];
                xfm.l.vz.x = trans.v[2][0];
                xfm.l.vz.y = trans.v[2][1];
                xfm.l.vz.z = trans.v[2][2];

                OSPGeometry inst = ospNewInstance(model[key], xfm);
                geo.push_back(inst);
                totalInstances++;
            }
            vislib::sys::Log::DefaultLog.WriteInfo(
                "OSPRaySESCellGeometry: Created %d instances of %s\n", mol.second.transformations.size(), key.c_str());
            totalAtoms += mol.second.transformations.size() * this->sesSpheres[key].size();
        }
        this->recompute = false;
        vislib::sys::Log::DefaultLog.WriteInfo("# different Molecules: %d\n", sesGeometry.size());
        vislib::sys::Log::DefaultLog.WriteInfo("# total Instances: %d", totalInstances);
        vislib::sys::Log::DefaultLog.WriteInfo("# total Atoms: %d", totalAtoms);


        std::vector<void*> geo_transfer(geo.size());
        for (auto i = 0; i < geo.size(); i++) {
            geo_transfer[i] = reinterpret_cast<void*>(geo[i]);
        }
        os->setStructureType(GEOMETRY);
        os->setAPIObjects(std::move(geo_transfer));
    }

    return true;
}

bool OSPRaySESCellGeometry::getExtendsCallback(megamol::core::Call& call) {
    auto os = dynamic_cast<CallOSPRayAPIObject*>(&call);
    if (os == nullptr) return false;
    megamol::protein_calls::CellDataCall* cd = this->molDataCallerSlot.CallAs<megamol::protein_calls::CellDataCall>();
    if (cd == nullptr) {
        return false;
    }

    if (!(*cd)(1)) return false;

    // Loop over all molecules
    for (auto& mol : *cd->data) {
        //megamol::core::BoundingBoxes bboxes = mol.second.call->AccessBoundingBoxes();
        //bbox.Union(bboxes.ObjectSpaceBBox());

        for (auto& trans : mol.second.transformations) {
            bbox.GrowToPoint(vislib::math::Point<float, 3>(trans.pos[0], trans.pos[1], trans.pos[2]));
        }
    }

    megamol::core::BoundingBoxes bounding;
    bounding.SetObjectSpaceBBox(bbox);

    os->SetExtent(1, bounding);

    return true;
}

bool OSPRaySESCellGeometry::LoadMolecule(std::string molKey) {
    this->sesSpheres[molKey].clear();
    this->colors[molKey].clear();
    protein_calls::CellDataCall* cd = this->molDataCallerSlot.CallAs<protein_calls::CellDataCall>();
    if (cd == nullptr) return false;
    auto& data = *cd->data;
    protein_calls::MolecularDataCall* mol = data[molKey].call;


    if (this->sesSpheres[molKey].empty()) {
        this->sesSpheres[molKey].reserve(mol->AtomCount());

        // for (unsigned int i = 0; i < mol->AtomTypeCount(); i++) {
        //    const megamol::protein_calls::MolecularDataCall::AtomType& atomType = mol->AtomTypes()[i];
        //    const unsigned char* color = atomType.Colour();
        //    float r = static_cast<float>(color[0]) / 255.0f;
        //    float g = static_cast<float>(color[1]) / 255.0f;
        //    float b = static_cast<float>(color[2]) / 255.0f;

        //    this->colors[molKey].push_back(vec4f{r, g, b, 1.0f});
        //}

        this->colors[molKey].clear();
        this->colors[molKey].resize(1);
        this->colors[molKey][0] =
            vec4f(data[molKey].color[0], data[molKey].color[1], data[molKey].color[2], data[molKey].color[3]);

        for (unsigned int i = 0; i < mol->AtomCount(); i++) {
            const float* positions = mol->AtomPositions();
            const megamol::protein_calls::MolecularDataCall::AtomType& atomType =
                mol->AtomTypes()[mol->AtomTypeIndices()[i]];
            OSPRaySESSphere sphere;

            // Transform molecule in origin
            sphere.center = vec3f(positions[i * 3] - data[molKey].center[0],
                positions[i * 3 + 1] - data[molKey].center[1], positions[i * 3 + 2] - data[molKey].center[2]);
            sphere.radius = atomType.Radius();
            // sphere.colorID = mol->AtomTypeIndices()[i];
            sphere.colorID = 0;
            this->sesSpheres[molKey].push_back(sphere);
        }
    }

    return true;
}

bool OSPRaySESCellGeometry::InterfaceIsDirty() {
    if (this->renderAsSlot.IsDirty() ||
        // this->addSlot.IsDirty() ||
        // this->removeSlot.IsDirty() ||
        this->triggerAddSlot.IsDirty() || 
		this->triggerRemoveSlot.IsDirty() ||
		this->chunkIDSlot.IsDirty() ||
		this->onlyChunkSlot.IsDirty() || 
		this->numChunksSlot.IsDirty() ||
		this->epsilonCuttingThresholdSlot.IsDirty() ||
		this->epsilonSelfIntersectSlot.IsDirty()) {
        this->renderAsSlot.ResetDirty();
        this->addSlot.ResetDirty();
        this->removeSlot.ResetDirty();
        this->triggerAddSlot.ResetDirty();
        this->triggerRemoveSlot.ResetDirty();
        this->chunkIDSlot.ResetDirty();
        this->onlyChunkSlot.ResetDirty();
        this->numChunksSlot.ResetDirty();
        this->epsilonCuttingThresholdSlot.ResetDirty();
        this->epsilonSelfIntersectSlot.ResetDirty();
        return true;
    } else {
        return false;
    }
}


bool OSPRaySESCellGeometry::InterfaceIsDirtyNoReset() const {
    if (this->renderAsSlot.IsDirty() ||
        // this->addSlot.IsDirty() ||
        // this->removeSlot.IsDirty() ||
        this->triggerAddSlot.IsDirty() || this->triggerRemoveSlot.IsDirty() || this->chunkIDSlot.IsDirty() ||
        this->onlyChunkSlot.IsDirty() || this->numChunksSlot.IsDirty() || epsilonCuttingThresholdSlot.IsDirty() || epsilonSelfIntersectSlot.IsDirty()) {
        return true;
    } else {
        return false;
    }
}

bool OSPRaySESCellGeometry::getDirtyCallback(megamol::core::Call& call) {
    auto os = dynamic_cast<CallOSPRayAPIObject*>(&call);
    auto cd = this->molDataCallerSlot.CallAs<protein_calls::CellDataCall>();

    if (cd == nullptr) return false;
    if (this->InterfaceIsDirtyNoReset() || this->triggered) {
        os->setDirty();
    }
    if (this->datahash != cd->GetDataHash()) {
        os->SetDataHash(cd->GetDataHash());
    }
    return true;
}


void OSPRaySESCellGeometry::SetSurfaceParams(OSPGeometry sesGeometry) {
    // switch (this->surfaceParamValues.mode) {
    // case ComputationMode::CPU:
    //    ospSetString(sesGeometry, "mode", "cpu");
    //    break;
    // case ComputationMode::VECTORIZED:
    //    ospSetString(sesGeometry, "mode", "vectorized");
    //    break;
    // case ComputationMode::PARTIALLY_VECTORIZED:
    //    ospSetString(sesGeometry, "mode", "partially_vectorized");
    //    break;
    //}

    // ospSet1i(sesGeometry, "debug", this->surfaceParamValues.debugFlag);
    // ospSet1f(sesGeometry, "probe_radius", static_cast<float>(this->surfaceParamValues.probeRadius));
    // ospSet1i(sesGeometry, "compute_spherical_triangle_singularities",
    //    static_cast<int>(this->surfaceParamValues.computeSingularities));
    // ospSet1i(
    //    sesGeometry, "skip_contour_singularities",
    //    static_cast<int>(this->surfaceParamValues.skipContourSingularities));
    // ospSet1i(sesGeometry, "resize_vectors", static_cast<int>(this->surfaceParamValues.resizeVectors));
    // ospSet1i(sesGeometry, "toroidal_patches_use_iterative_method",
    //    static_cast<int>(this->surfaceParamValues.useIterativeMethod));
    // ospSet1f(sesGeometry, "epsilon_cutting_threshold", this->surfaceParamValues.epsilonCuttingThreshold);
    // ospSet1f(sesGeometry, "epsilon_self_intersect", this->surfaceParamValues.epsilonSelfIntersect);
    // ospSet1i(sesGeometry, "compute_contour_arcs", static_cast<int>(this->surfaceParamValues.renderContourArcs));
}

bool OSPRaySESCellGeometry::triggerRemoveCallback(megamol::core::param::ParamSlot& slot) {

    std::string toRemove = this->removeSlot.Param<core::param::FlexEnumParam>()->ValueString();
    if (toRemove.empty()) return true;
    this->removeSlot.Param<core::param::FlexEnumParam>()->ClearValues();
    this->addSlot.Param<core::param::FlexEnumParam>()->ClearValues();
    this->removeSlot.Param<core::param::FlexEnumParam>()->ParseValue("");

    for (auto s : this->show) {
        bool thisLoop = s.second;
        if (s.first == toRemove) {
            this->show[s.first] = false;
            thisLoop = false;
        }

        if (thisLoop) {
            this->removeSlot.Param<core::param::FlexEnumParam>()->AddValue(s.first);
        } else {
            this->addSlot.Param<core::param::FlexEnumParam>()->AddValue(s.first);
        }
    }
    this->triggered = true;

    return true;
}

bool OSPRaySESCellGeometry::triggerAddCallback(megamol::core::param::ParamSlot& slot) {

    std::string toAdd = this->addSlot.Param<core::param::FlexEnumParam>()->ValueString();
    if (toAdd.empty()) return true;
    this->removeSlot.Param<core::param::FlexEnumParam>()->ClearValues();
    this->addSlot.Param<core::param::FlexEnumParam>()->ClearValues();
    this->addSlot.Param<core::param::FlexEnumParam>()->ParseValue("");

    for (auto s : this->show) {
        bool thisLoop = s.second;
        if (s.first == toAdd) {
            this->show[s.first] = true;
            thisLoop = true;
        }

        if (thisLoop) {
            this->removeSlot.Param<core::param::FlexEnumParam>()->AddValue(s.first);
        } else {
            this->addSlot.Param<core::param::FlexEnumParam>()->AddValue(s.first);
        }
    }
    this->triggered = true;

    return true;
}

bool OSPRaySESCellGeometry::triggerShowAllCallback(megamol::core::param::ParamSlot& slot) {

    this->addSlot.Param<core::param::FlexEnumParam>()->ClearValues();
    this->removeSlot.Param<core::param::FlexEnumParam>()->ClearValues();
    this->addSlot.Param<core::param::FlexEnumParam>()->ParseValue("");
    this->removeSlot.Param<core::param::FlexEnumParam>()->ParseValue("");

    for (auto& s : this->show) {
        this->removeSlot.Param<core::param::FlexEnumParam>()->AddValue(s.first);
        this->show[s.first] = true;
    }

    this->triggered = true;
    return true;
}


bool OSPRaySESCellGeometry::triggerHideAllCallback(megamol::core::param::ParamSlot& slot) {
    this->addSlot.Param<core::param::FlexEnumParam>()->ClearValues();
    this->removeSlot.Param<core::param::FlexEnumParam>()->ClearValues();
    this->addSlot.Param<core::param::FlexEnumParam>()->ParseValue("");
    this->removeSlot.Param<core::param::FlexEnumParam>()->ParseValue("");

    for (auto& s : this->show) {
        this->addSlot.Param<core::param::FlexEnumParam>()->AddValue(s.first);
        this->show[s.first] = false;
    }

    this->triggered = true;
    return true;
}

void OSPRaySESCellGeometry::createChunks() {

    int numChunks = this->numChunksSlot.Param<core::param::EnumParam>()->Value();
    int numChunksPerDim = pow(numChunks,1.0f/3.0f);
    float chunkWidth = this->bbox.Width() / numChunksPerDim;
    float chunkHeight = this->bbox.Height() / numChunksPerDim;
    float chunkDepth = this->bbox.Depth() / numChunksPerDim;

    this->chunks.clear();
    this->chunks.reserve(numChunks);

    for (int i = 0; i < numChunksPerDim; i++) {
        for (int j = 0; j < numChunksPerDim; j++) {
            for (int k = 0; k < numChunksPerDim; k++) {
                vislib::math::Cuboid<float> localbbox;
                localbbox.SetLeft(this->bbox.Left() + i * chunkWidth);
                localbbox.SetRight(this->bbox.Left() + (i + 1) * chunkWidth);
                localbbox.SetBottom(this->bbox.Bottom() + j * chunkHeight);
                localbbox.SetTop(this->bbox.Bottom() + (j + 1) * chunkHeight);
                localbbox.SetBack(this->bbox.Back() + k * chunkDepth);
                localbbox.SetFront(this->bbox.Back() + (k + 1) * chunkDepth);
                this->chunks.push_back(localbbox);
            }
        }
    }
    vislib::sys::Log::DefaultLog.WriteInfo("Calculated the bounding boxes of %d chunks.", numChunks);
}