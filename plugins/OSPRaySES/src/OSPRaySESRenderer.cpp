#include <iostream>
#include <stdio.h>

#include "OSPRaySESRenderer.h"
#include "mmcore/CoreInstance.h"
#include "mmcore/param/BoolParam.h"
#include "mmcore/param/ButtonParam.h"
#include "mmcore/param/EnumParam.h"
#include "mmcore/param/FloatParam.h"
#include "mmcore/param/IntParam.h"
#include "mmcore/param/Vector3fParam.h"
#include "mmcore/param/Vector4fParam.h"
#include "mmcore/view/CallRender3D.h"
#include "protein_calls/MolecularDataCall.h"
#include "vislib/graphics/gl/IncludeAllGL.h"
#include "vislib/graphics/gl/ShaderSource.h"
#include "vislib/math/Cuboid.h"
#include "vislib/math/Quaternion.h"
#include "vislib/math/Vector.h"
#include "mmcore/param/ParamSlot.h"

using namespace megamol::ospray::ses;

OSPRaySESRenderer::OSPRaySESRenderer()
    : AbstractOSPRayTestRenderer()
    , molDataCallerSlot("getData", "Protein data input.")
    , triggerDirtySlot("refresh", "refreshes"){

    this->dirty = false;
    this->currentTime = 0.0f;
    this->dataHash = 0;

    // Slots
    this->molDataCallerSlot.SetCompatibleCall<megamol::protein_calls::MolecularDataCallDescription>();
    this->MakeSlotAvailable(&this->molDataCallerSlot);

    this->triggerDirtySlot << new core::param::BoolParam(false);
    this->MakeSlotAvailable(&this->triggerDirtySlot);
}

OSPRaySESRenderer::~OSPRaySESRenderer() { this->Release(); }

void OSPRaySESRenderer::DestroyOSP() { AbstractOSPRayTestRenderer::DestroyOSP(); }

bool OSPRaySESRenderer::Render(megamol::core::Call& call) {
    this->RenderPre();

    core::view::CallRender3D* cr = dynamic_cast<core::view::CallRender3D*>(&call);
    if (cr == nullptr) return false;

    megamol::protein_calls::MolecularDataCall* mol =
        this->molDataCallerSlot.CallAs<megamol::protein_calls::MolecularDataCall>();
    if (mol == nullptr) {
        return false;
    }

    this->dirty = this->triggerDirtySlot.Param<core::param::BoolParam>()->Value();
    // check for changed time/data
    const float reqTime = cr->Time();
    if (this->currentTime != reqTime) {
        this->dirty = true;
    }

    if (this->dataHash != mol->DataHash()) {
        this->dirty = true;
    }

    //bool geometryDirty = this->IsSurfaceInterfaceDirty() || this->dirty || this->GetGeometryCnt() == 0;
    bool geometryDirty = this->dirty;
    bool rendererDirty = this->IsRendererInterfaceDirty();

    // load data...
    if (this->dirty) {
        if (!this->LoadData(reqTime)) {
            return false;
        }
    }

    if (geometryDirty) {
        this->ClearGeometry();
        OSPGeometry sesGeometry = this->CreateSurface();

        this->SetSurfaceParams(sesGeometry);
        // spheres
        OSPData sphereData =
            ospNewData(sizeof(OSPRaySESSphere) * this->sesSpheres.size(), OSP_UCHAR, this->sesSpheres.data());
        OSPData sphereColorData = ospNewData(this->colors.size(), OSP_FLOAT4, this->colors.data());
        ospSet1i(sesGeometry, "bytes_per_ses_sphere", sizeof(OSPRaySESSphere));
        ospSet1i(sesGeometry, "ses_spheres_offset_center", offsetof(OSPRaySESSphere, center));
        ospSet1i(sesGeometry, "ses_spheres_offset_radius", offsetof(OSPRaySESSphere, radius));
        ospSet1i(sesGeometry, "ses_spheres_offset_colorID", offsetof(OSPRaySESSphere, colorID));
        ospSetData(sesGeometry, "ses_spheres", sphereData);
        ospSetData(sesGeometry, "colors", sphereColorData);
        ospCommit(sesGeometry);

        this->AddGeometry(sesGeometry);
        ospRelease(sphereData);
        ospRelease(sphereColorData);
        this->FinalizeGeometry();
    }

    this->UpdateCameraAndRenderer(cr->GetCameraParameters(), geometryDirty || rendererDirty);

    return this->RenderPost();
}

bool OSPRaySESRenderer::GetExtents(megamol::core::Call& call) {
    // TODO something is not right here
    megamol::core::view::CallRender3D* cr = dynamic_cast<megamol::core::view::CallRender3D*>(&call);
    if (cr == nullptr) return false;
    megamol::protein_calls::MolecularDataCall* mol =
        this->molDataCallerSlot.CallAs<megamol::protein_calls::MolecularDataCall>();
    if (mol == nullptr) {
        return false;
    }

    if (!(*mol)(1)) return false;
    float scale;
    if (!vislib::math::IsEqual(mol->AccessBoundingBoxes().ObjectSpaceBBox().LongestEdge(), 0.0f)) {
        scale = 1.0f / mol->AccessBoundingBoxes().ObjectSpaceBBox().LongestEdge();
    } else {
        scale = 1.0f;
    }
    megamol::core::BoundingBoxes bboxes = mol->AccessBoundingBoxes();
    bboxes.MakeScaledWorld(scale);
    cr->AccessBoundingBoxes() = bboxes;
    cr->AccessBoundingBoxes().MakeScaledWorld(1.0f);
    cr->SetTimeFramesCount(mol->FrameCount());

    return true;
}

bool OSPRaySESRenderer::LoadData(float time) {
    this->sesSpheres.clear();
    this->colors.clear();
    megamol::protein_calls::MolecularDataCall* mol =
        this->molDataCallerSlot.CallAs<megamol::protein_calls::MolecularDataCall>();
    if (mol == nullptr) return false;
    mol->SetCalltime(this->currentTime);
    mol->SetFrameID(static_cast<int>(time));
    this->currentTime = time;
    this->dataHash = mol->DataHash();
    if (!(*mol)(megamol::protein_calls::MolecularDataCall::CallForGetData)) return false;

    this->sesSpheres.clear();
    this->sesSpheres.reserve(mol->AtomCount());

    this->colors.clear();

    for (unsigned int i = 0; i < mol->AtomTypeCount(); i++) {
        const megamol::protein_calls::MolecularDataCall::AtomType& atomType = mol->AtomTypes()[i];
        const unsigned char* color = atomType.Colour();
        float r = static_cast<float>(color[0]) / 255.0f;
        float g = static_cast<float>(color[1]) / 255.0f;
        float b = static_cast<float>(color[2]) / 255.0f;

        this->colors.push_back(vec4f{r, g, b, 1.0f});
    }

    for (unsigned int i = 0; i < mol->AtomCount(); i++) {
        const float* positions = mol->AtomPositions();
        const megamol::protein_calls::MolecularDataCall::AtomType& atomType =
            mol->AtomTypes()[mol->AtomTypeIndices()[i]];
        OSPRaySESSphere sphere;

        sphere.center = vec3f(positions[i * 3], positions[i * 3 + 1], positions[i * 3 + 2]);
        sphere.radius = atomType.Radius();
        sphere.colorID = mol->AtomTypeIndices()[i];
        this->sesSpheres.push_back(sphere);
    }

    this->dirty = false;

    return true;
}

