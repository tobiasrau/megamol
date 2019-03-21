#include <stdio.h>
#include <iostream>

#include "vislib/graphics/gl/IncludeAllGL.h"
#include "vislib/graphics/gl/ShaderSource.h"
#include "vislib/math/Cuboid.h"
#include "vislib/math/Quaternion.h"
#include "vislib/math/Vector.h"
#include "mmcore/CoreInstance.h"
#include "mmcore/view/CallRender3D.h"
#include "mmcore/param/Vector3fParam.h"
#include "mmcore/param/Vector4fParam.h"
#include "mmcore/param/ButtonParam.h"
#include "mmcore/param/FloatParam.h"
#include "mmcore/param/BoolParam.h"
#include "mmcore/param/IntParam.h"
#include "mmcore/param/EnumParam.h"
#include "protein_calls/MolecularDataCall.h"
#include "OSPRayInstancedSESRenderer.h"

using namespace megamol::ospray::ses;

OSPRayInstancedSESRenderer::OSPRayInstancedSESRenderer() :
  AbstractOSPRayTestRenderer(),
  numInstancesPerDimensionSlot("numInstancesPerDimension", "Number of instances along one dimension."),
  instanceSpacingSlot("instanceSpacingSlot", "Spacing for instances"),
  molDataCallerSlot("getData", "Protein data input.")
  {

  this->recompute = true;

  this->dataLoaded = false;

  this->numInstancesPerDimensionSlot << new megamol::core::param::IntParam(10, 1);
  this->MakeSlotAvailable(&this->numInstancesPerDimensionSlot);

  this->instanceSpacingSlot << new megamol::core::param::FloatParam(100.0f, 1.0f);
  this->MakeSlotAvailable(&this->instanceSpacingSlot);

  // Slots
  this->molDataCallerSlot.SetCompatibleCall<megamol::protein_calls::MolecularDataCallDescription>();
  this->MakeSlotAvailable(&this->molDataCallerSlot);
}

OSPRayInstancedSESRenderer::~OSPRayInstancedSESRenderer() {
  this->Release();
}

void OSPRayInstancedSESRenderer::DestroyOSP() {
  AbstractOSPRayTestRenderer::DestroyOSP();
}

bool OSPRayInstancedSESRenderer::Render(megamol::core::Call & call) {

  this->RenderPre();

  core::view::CallRender3D *cr = dynamic_cast<core::view::CallRender3D*>(&call);
  if (cr == nullptr) return false;

  if (!this->dataLoaded) {
    this->dataLoaded = true;
    if (!this->LoadData()) {
      return false;
    }
  }

  this->recompute = this->recompute || this->IsSurfaceInterfaceDirty();

  if (this->numInstancesPerDimensionSlot.IsDirty() || this->instanceSpacingSlot.IsDirty()) {
    this->recompute = true;
    this->numInstancesPerDimensionSlot.ResetDirty();
    this->instanceSpacingSlot.ResetDirty();
  }

  if (this->recompute) {
    this->ClearGeometry();
    const int preferredNumInstances = this->numInstancesPerDimensionSlot.Param<megamol::core::param::IntParam>()->Value();
    // int numInstances = static_cast<int>(std::ceil(std::pow(static_cast<double>(preferredNumInstances), 1.0 / 3.0)));
    int numInstances = preferredNumInstances;
    float spacing = this->instanceSpacingSlot.Param<megamol::core::param::FloatParam>()->Value();

    OSPModel model = ospNewModel();
    OSPGeometry sesGeometry = this->CreateSurface();

    this->SetSurfaceParams(sesGeometry);
   
    // spheres
    OSPData sphereData = ospNewData(sizeof(OSPRaySESSphere)*this->sesSpheres.size(), OSP_UCHAR, this->sesSpheres.data());
    OSPData sphereColorData = ospNewData(this->colors.size(), OSP_FLOAT4, this->colors.data());
    ospSet1i(sesGeometry, "bytes_per_ses_sphere", sizeof(OSPRaySESSphere));
    ospSet1i(sesGeometry, "ses_spheres_offset_center", offsetof(OSPRaySESSphere, center));
    ospSet1i(sesGeometry, "ses_spheres_offset_radius", offsetof(OSPRaySESSphere, radius));
    ospSet1i(sesGeometry, "ses_spheres_offset_colorID", offsetof(OSPRaySESSphere, colorID));
    ospSetData(sesGeometry, "ses_spheres", sphereData);
    ospSetData(sesGeometry, "colors", sphereColorData);
    ospCommit(sesGeometry);
    ospAddGeometry(model, sesGeometry);

    for (int z = 0; z < numInstances; z++) {
      for (int y = 0; y < numInstances; y++) {
        for (int x = 0; x < numInstances; x++) {
          osp::affine3f transf = ospcommon::AffineSpace3f::translate(spacing*vec3f(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)));
          OSPGeometry inst = ospNewInstance(model, transf);
          this->AddGeometry(inst);
        }
      }
    }
    this->FinalizeGeometry();
    vislib::sys::Log::DefaultLog.WriteInfo("OSPRayInstancedSESRenderer: NumInstances = %d\n", numInstances*numInstances*numInstances);

    ospRelease(sphereData);
    ospRelease(sphereColorData);

    ospRemoveGeometry(model, sesGeometry);
    ospRelease(model);
    this->recompute = false;
  }

  this->UpdateCameraAndRenderer(cr->GetCameraParameters());
  
  return this->RenderPost();
}

bool OSPRayInstancedSESRenderer::GetExtents(megamol::core::Call & call) {
  megamol::core::view::CallRender3D *cr = dynamic_cast<megamol::core::view::CallRender3D*>(&call);
  if (cr == nullptr) return false;
  megamol::protein_calls::MolecularDataCall *mol = this->molDataCallerSlot.CallAs<megamol::protein_calls::MolecularDataCall>();
  if (mol == nullptr) {
    return false;
  }

  const int preferredNumInstances = this->numInstancesPerDimensionSlot.Param<megamol::core::param::IntParam>()->Value();
  // int numInstances = static_cast<int>(std::ceil(std::pow(static_cast<double>(preferredNumInstances), 1.0 / 3.0)));
  int numInstances = preferredNumInstances;
  float spacing = this->instanceSpacingSlot.Param<megamol::core::param::FloatParam>()->Value();

  if (!(*mol)(1)) return false;
  megamol::core::BoundingBoxes bboxes = mol->AccessBoundingBoxes();


  vislib::math::Cuboid<float> bbox = bboxes.ObjectSpaceBBox();
  // HACK
  if (numInstances != 1) {
    bbox.GrowToPoint(vislib::math::Point<float, 3>(bbox.Right() + (numInstances - 1)*spacing,
          bbox.Top() + (numInstances - 1) * spacing,
		bbox.Back() + (numInstances - 1) * spacing));
  }
  
  bboxes.SetObjectSpaceBBox(bbox);
  this->scale = 10.0f / bbox.LongestEdge();
  //bboxes.MakeScaledWorld(scale);
  cr->AccessBoundingBoxes() = bboxes;
  cr->AccessBoundingBoxes().MakeScaledWorld(this->scale);
  cr->SetTimeFramesCount(mol->FrameCount());

  return true;

}

bool OSPRayInstancedSESRenderer::LoadData() {
  this->sesSpheres.clear();
  this->colors.clear();
  megamol::protein_calls::MolecularDataCall *mol = this->molDataCallerSlot.CallAs<megamol::protein_calls::MolecularDataCall>();
  if (mol == nullptr) return false;
  if (!(*mol)(megamol::protein_calls::MolecularDataCall::CallForGetData)) return false;
  if (this->sesSpheres.empty()) {
    this->sesSpheres.reserve(mol->AtomCount());


    for (unsigned int i = 0; i < mol->AtomTypeCount(); i++) {
      const megamol::protein_calls::MolecularDataCall::AtomType &atomType = mol->AtomTypes()[i];
      const unsigned char* color = atomType.Colour();
      float r = static_cast<float>(color[0]) / 255.0f;
      float g = static_cast<float>(color[1]) / 255.0f;
      float b = static_cast<float>(color[2]) / 255.0f;

      this->colors.push_back(vec4f{ r, g, b, 1.0f });
    }

    for (unsigned int i = 0; i < mol->AtomCount(); i++) {
      const float *positions = mol->AtomPositions();
      const megamol::protein_calls::MolecularDataCall::AtomType &atomType = mol->AtomTypes()[mol->AtomTypeIndices()[i]];
      OSPRaySESSphere sphere;

      sphere.center = vec3f(positions[i * 3], positions[i * 3 + 1], positions[i * 3 + 2]);
      sphere.radius = atomType.Radius();
      sphere.colorID = mol->AtomTypeIndices()[i];
      this->sesSpheres.push_back(sphere);
    }
  }

  return true;
}