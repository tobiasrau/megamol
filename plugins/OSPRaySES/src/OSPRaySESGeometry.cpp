#include "stdafx.h"
#include "OSPRaySESGeometry.h"
#include "protein_calls/MolecularDataCall.h"
#include "mmcore/param/Vector3fParam.h"
#include "mmcore/param/ButtonParam.h"
#include "mmcore/param/FloatParam.h"
#include "mmcore/param/BoolParam.h"
#include "mmcore/param/IntParam.h"
#include "mmcore/param/EnumParam.h"
#include "OSPRay_plugin/CallOSPRayAPIObject.h"
#include "Common.h"
#include "OSPRayPrimitives.h"

using namespace megamol::ospray::ses;

OSPRaySESGeometry::OSPRaySESGeometry(void) :
  megamol::core::Module(),
  getDataSlot("getdata", "Connects to the data source"),
  deployOSPRayGeometrySlot("deployOSPRayGeometrySlot", "Connects to Handler of contained geometry"),
  modeSlot("mode", "ses computation mode"),
  probeRadiusSlot("probeRadius", "Probe radius"),
  skipContourSingularitiesSlot("skipContourSingularities", "Skip singular cases"),
  resizeVectorsSlot("resizeVectors", "Flag indicates if vectors used in contour computations should be resized to save memory"),
  computeSingularitiesSlot("computeSingularities", "Flag indicating if spherical triangle singularities are considered"),
  debugFlagSlot("debugFlag", "Flag indicating debug mode"),
  renderContourArcsSlot("renderContourArcs", "Flag indicating contour arc rendering") {

  this->currentTime = 0.0f;

  // Slots
  this->getDataSlot.SetCompatibleCall<megamol::protein_calls::MolecularDataCallDescription>();
  this->MakeSlotAvailable(&this->getDataSlot);

  this->deployOSPRayGeometrySlot.SetCallback(CallOSPRayAPIObject::ClassName(), CallOSPRayAPIObject::FunctionName(0), &OSPRaySESGeometry::GetData);
  this->deployOSPRayGeometrySlot.SetCallback(CallOSPRayAPIObject::ClassName(), CallOSPRayAPIObject::FunctionName(1), &OSPRaySESGeometry::GetExtends);
  this->deployOSPRayGeometrySlot.SetCallback(CallOSPRayAPIObject::ClassName(), CallOSPRayAPIObject::FunctionName(2), &OSPRaySESGeometry::GetDirty);
  this->MakeSlotAvailable(&this->deployOSPRayGeometrySlot);

  this->probeRadiusSlot << new megamol::core::param::FloatParam(1.4f, 0.01f);
  this->MakeSlotAvailable(&this->probeRadiusSlot);

  this->skipContourSingularitiesSlot << new megamol::core::param::BoolParam(true);
  this->MakeSlotAvailable(&this->skipContourSingularitiesSlot);

  this->resizeVectorsSlot << new megamol::core::param::BoolParam(false);
  this->MakeSlotAvailable(&this->resizeVectorsSlot);

  this->computeSingularitiesSlot << new megamol::core::param::BoolParam(false);
  this->MakeSlotAvailable(&this->computeSingularitiesSlot);

  this->debugFlagSlot << new megamol::core::param::BoolParam(false);
  this->MakeSlotAvailable(&this->debugFlagSlot);

  this->renderContourArcsSlot << new megamol::core::param::BoolParam(false);
  this->MakeSlotAvailable(&this->renderContourArcsSlot);

  megamol::core::param::EnumParam *modeParam = new megamol::core::param::EnumParam(ComputationMode::CPU);
  modeParam->SetTypePair(ComputationMode::CPU, "CPU");
  modeParam->SetTypePair(ComputationMode::VECTORIZED, "Vectorized");
  modeParam->SetTypePair(ComputationMode::PARTIALLY_VECTORIZED, "Partially vectorized");
  this->modeSlot << modeParam;
  this->MakeSlotAvailable(&this->modeSlot);
}

OSPRaySESGeometry::~OSPRaySESGeometry(void){
  this->Release();
}

bool OSPRaySESGeometry::GetData(megamol::core::Call &call) {

  CallOSPRayAPIObject *cd = dynamic_cast<CallOSPRayAPIObject *>(&call);

  if (cd == nullptr) {
    return false;
  }
  
  // load data, these must be copied as they go out of scope!
  std::vector<OSPRaySESSphere> spheres;
  std::vector<vec4f> colors;

  megamol::protein_calls::MolecularDataCall *mol = this->getDataSlot.CallAs<megamol::protein_calls::MolecularDataCall>();
  if (mol == nullptr) return false;
  const float timeStamp = cd->GetTimeStamp();
  mol->SetCalltime(timeStamp);
  mol->SetFrameID(static_cast<int>(timeStamp));
  if (!(*mol)(megamol::protein_calls::MolecularDataCall::CallForGetData)) return false;
  this->currentTime = timeStamp;

  // load the data from mol
  spheres.reserve(mol->AtomCount());

  for (unsigned int i = 0; i < mol->AtomTypeCount(); i++) {
    const megamol::protein_calls::MolecularDataCall::AtomType &atomType = mol->AtomTypes()[i];
    const unsigned char* color = atomType.Colour();
    float r = static_cast<float>(color[0]) / 255.0f;
    float g = static_cast<float>(color[1]) / 255.0f;
    float b = static_cast<float>(color[2]) / 255.0f;

    colors.push_back(vec4f{ r, g, b, 1.0f });
  }

  for (unsigned int i = 0; i < mol->AtomCount(); i++) {
    const float *positions = mol->AtomPositions();
    const megamol::protein_calls::MolecularDataCall::AtomType &atomType = mol->AtomTypes()[mol->AtomTypeIndices()[i]];
    OSPRaySESSphere sphere;

    sphere.center = vec3f(positions[i * 3], positions[i * 3 + 1], positions[i * 3 + 2]);
    sphere.radius = atomType.Radius();
    sphere.colorID = mol->AtomTypeIndices()[i];
    spheres.push_back(sphere);
  }
  
  float probeRadius = this->probeRadiusSlot.Param<megamol::core::param::FloatParam>()->Value();
  bool skipContourSingularities = this->skipContourSingularitiesSlot.Param<megamol::core::param::BoolParam>()->Value();
  bool computeSingularities = this->computeSingularitiesSlot.Param<megamol::core::param::BoolParam>()->Value();
  bool resizeVectors = this->resizeVectorsSlot.Param<megamol::core::param::BoolParam>()->Value();
  bool debug = this->debugFlagSlot.Param<megamol::core::param::BoolParam>()->Value();
  bool renderContourArcs = this->renderContourArcsSlot.Param<megamol::core::param::BoolParam>()->Value();

  // setup ospray geometry from data
  OSPGeometry sesGeometry = ospNewGeometry("ses_surface");

  const ComputationMode mode = static_cast<ComputationMode>(this->modeSlot.Param<megamol::core::param::EnumParam>()->Value());

  switch (mode) {
  case ComputationMode::CPU:
    ospSetString(sesGeometry, "mode", "cpu");
    break;
  case ComputationMode::VECTORIZED:
    ospSetString(sesGeometry, "mode", "vectorized");
    break;
  case ComputationMode::PARTIALLY_VECTORIZED:
    ospSetString(sesGeometry, "mode", "partially_vectorized");
    break;
  }

  ospSet1i(sesGeometry, "debug", debug);
  ospSet1f(sesGeometry, "probe_radius", static_cast<float>(probeRadius));
  ospSet1i(sesGeometry, "compute_spherical_triangle_singularities", static_cast<int>(computeSingularities));
  ospSet1i(sesGeometry, "skip_contour_singularities", static_cast<int>(skipContourSingularities));
  ospSet1i(sesGeometry, "resize_vectors", static_cast<int>(resizeVectors));
  ospSet1i(sesGeometry, "compute_contour_arcs", static_cast<int>(renderContourArcs));
  
  OSPData sphereData = ospNewData(sizeof(OSPRaySESSphere)*spheres.size(), OSP_UCHAR, spheres.data());
  OSPData sphereColorData = ospNewData(colors.size(), OSP_FLOAT4, colors.data());
  // were copied, we may need the memory to compute the surface
  spheres.clear();
  colors.clear();
  spheres.shrink_to_fit();
  colors.shrink_to_fit();

  ospSet1i(sesGeometry, "bytes_per_ses_sphere", sizeof(OSPRaySESSphere));
  ospSet1i(sesGeometry, "ses_spheres_offset_center", offsetof(OSPRaySESSphere, center));
  ospSet1i(sesGeometry, "ses_spheres_offset_radius", offsetof(OSPRaySESSphere, radius));
  ospSet1i(sesGeometry, "ses_spheres_offset_colorID", offsetof(OSPRaySESSphere, colorID));
  ospSetData(sesGeometry, "ses_spheres", sphereData);
  ospSetData(sesGeometry, "colors", sphereColorData);

  ospCommit(sesGeometry);
  //ospRelease(sphereData);
  //ospRelease(sphereColorData);

  std::vector<void*> geo_transfer(1);
  geo_transfer[0] = reinterpret_cast<void*>(sesGeometry);

  cd->setAPIObjects(std::move(geo_transfer));
  cd->SetDataHash(mol->DataHash());
  cd->setStructureType(structureTypeEnum::GEOMETRY);
  return true;
}

bool OSPRaySESGeometry::GetExtends(megamol::core::Call &call) {
  megamol::protein_calls::MolecularDataCall *mol = this->getDataSlot.CallAs<megamol::protein_calls::MolecularDataCall>();
  CallOSPRayAPIObject *cd = dynamic_cast<CallOSPRayAPIObject *>(&call);
  if (mol == nullptr || cd == nullptr) {
    return false;
  }

  if (!(*mol)(1)) return false;
  //cd->AccessBoundingBoxes() = mol->AccessBoundingBoxes();
  //cd->AccessBoundingBoxes().MakeScaledWorld(1.0f);
  cd->SetExtent(mol->FrameCount(), mol->AccessBoundingBoxes());
  return true;
}

bool OSPRaySESGeometry::GetDirty(megamol::core::Call& call) {
    auto os = dynamic_cast<CallOSPRayAPIObject*>(&call);
    auto cd = this->getDataSlot.CallAs<megamol::protein_calls::MolecularDataCall>();

    if (cd == nullptr) return false;
    if (this->InterfaceIsDirtyNoReset()) {
        os->setDirty();
    }
    os->SetDataHash(cd->DataHash());
    return true;
}

bool OSPRaySESGeometry::InterfaceIsDirty() {
    if (this->modeSlot.IsDirty() || this->probeRadiusSlot.IsDirty() || this->skipContourSingularitiesSlot.IsDirty() ||
        this->resizeVectorsSlot.IsDirty() || this->computeSingularitiesSlot.IsDirty() ||
        this->debugFlagSlot.IsDirty()) {
        this->modeSlot.ResetDirty();
        this->probeRadiusSlot.ResetDirty();
        this->skipContourSingularitiesSlot.ResetDirty();
        this->resizeVectorsSlot.ResetDirty();
        this->computeSingularitiesSlot.ResetDirty();
        this->debugFlagSlot.ResetDirty();
        return true;
    } else {
        return false;
    }
}

bool OSPRaySESGeometry::InterfaceIsDirtyNoReset() const {
    if (this->modeSlot.IsDirty() || this->probeRadiusSlot.IsDirty() || this->skipContourSingularitiesSlot.IsDirty() ||
        this->resizeVectorsSlot.IsDirty() || this->computeSingularitiesSlot.IsDirty() ||
        this->debugFlagSlot.IsDirty()) {
        return true;
    } else {
        return false;
    }
}