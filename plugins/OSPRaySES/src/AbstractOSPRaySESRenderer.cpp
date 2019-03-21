#include <stdio.h>
#include <iostream>

#include "vislib/graphics/gl/IncludeAllGL.h"
#include "vislib/graphics/gl/ShaderSource.h"
#include "vislib/math/Cuboid.h"
#include "vislib/math/Quaternion.h"
#include "mmcore/CoreInstance.h"
#include "mmcore/view/CallRender3D.h"
#include "mmcore/param/Vector3fParam.h"
#include "mmcore/param/ButtonParam.h"
#include "mmcore/param/FloatParam.h"
#include "mmcore/param/BoolParam.h"
#include "mmcore/param/IntParam.h"
#include "protein_calls/MolecularDataCall.h"
#include "AbstractOSPRaySESRenderer.h"

using namespace megamol::ospray::ses;

AbstractOSPRaySESRenderer::AbstractOSPRaySESRenderer() :
  AbstractOSPRayTestRenderer() {

  this->recompute = false;
  this->dataLoaded = false;
}

AbstractOSPRaySESRenderer::~AbstractOSPRaySESRenderer() {
  this->Release();
}

void AbstractOSPRaySESRenderer::DestroyOSP() {
  AbstractOSPRayTestRenderer::DestroyOSP();
}

bool AbstractOSPRaySESRenderer::Render(megamol::core::Call & call) {
  this->RenderPre();

  core::view::CallRender3D *cr = dynamic_cast<core::view::CallRender3D*>(&call);
  if (cr == nullptr) return false;

  if (!this->dataLoaded) {
    if (!this->LoadData()) {
      return false;
    }
  }

  this->recompute = this->recompute || this->IsSurfaceInterfaceDirty() || this->GetGeometryCnt() == 0;

  if (this->recompute) {
    this->ClearGeometry();
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

    this->AddGeometry(sesGeometry);

    ospRelease(sphereData);
    ospRelease(sphereColorData);
    this->recompute = false;
    this->FinalizeGeometry();
  }

  this->UpdateCameraAndRenderer(cr->GetCameraParameters(), this->recompute || this->IsSurfaceInterfaceDirty());

  return this->RenderPost();
}