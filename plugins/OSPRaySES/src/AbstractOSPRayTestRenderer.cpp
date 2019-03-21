#include <stdio.h>
#include <iostream>
#include "vislib/graphics/gl/IncludeAllGL.h"
#include "vislib/graphics/gl/ShaderSource.h"
#include "vislib/math/Vector.h"
#include "mmcore/CoreInstance.h"
#include "mmcore/param/Vector3fParam.h"
#include "mmcore/param/Vector4fParam.h"
#include "mmcore/param/ButtonParam.h"
#include "mmcore/param/FloatParam.h"
#include "mmcore/param/BoolParam.h"
#include "mmcore/param/IntParam.h"
#include "mmcore/param/EnumParam.h"
#include "AbstractOSPRayTestRenderer.h"
#include "OSPRaySESModuleLoader.h"
//const float M_PI = 3.14159277f;

using namespace megamol::ospray::ses;

AbstractOSPRayTestRenderer::AbstractOSPRayTestRenderer() :
  core::view::Renderer3DModule(),
  // Renderer
  epsilonSlot("epsilon", "Renderer epsilon"),
  useHeadlightSlot("useHeadlight", "Use headlight lighting"),
  lightDirSlot("lightDir", "Directional light"),
  lightDiffuseWeightSlot("lightDiffuseWeight", "Diffuse color weight"),
  lightAmbientWeightSlot("lightAmbientWeight", "Ambient color weight"),
  lightExponentSlot("lightExponent", "Exponent of the light"),
  alphaSlot("alpha", "Genernal transparancy"),
  useCuttingPlaneSlot("useCuttingPlane", "Whether to use a cutting plane"),
  cuttingPlaneSlot("cuttingPlane", "The cutting plane in Hesse-Normal form"),
  accumulateSlot("accumulate", "Wether to use accumulation"),
  // AOOM
  useAOOMSlot("useAOOM", "Flag indicates if AOOM is to be used."),
  smallCavitiesSlot("smallCavities", "Flag indicates which visibility function to use."),
  rhoSlot("rho", "AOOM rho"),
  tauSlot("tau", "AOOM tau"),
  aoDistanceSlot("aoDistance", "AO hemisphere radius"),
  aoSamplesSlot("aoSamples", "AO samples count"),
  blendingColorSlot("blendingColor", "Occluded areas are blended with this color"),
  blendingWeightSlot("blendingWeight", "Weight of occlusion blending"),
  // Surface
  modeSlot("mode", "ses computation mode"),
  probeRadiusSlot("probeRadius", "Probe radius"),
  skipContourSingularitiesSlot("skipContourSingularities", "Skip singular cases"),
  resizeVectorsSlot("resizeVectors", "Flag indicates if vectors used in contour computations should be resized to save memory"),
  computeSingularitiesSlot("computeSingularities", "Flag indicating if spherical triangle singularities are considered"),
  debugFlagSlot("debugFlag", "Flag indicating debug mode"),
  useIterativeMethodSlot("useIterativeMethod", "Enable iterative method for toroidal patch rendering"),
  epsilonCuttingThresholdSlot("epsilonCuttingThreshold", "Secondary epsilon"),
  epsilonSelfIntersectSlot("epsilonSelfIntersect", "Third epsilon"),
  renderContourArcsSlot("renderContourArcs", "Flag indicating contour arc rendering") {

  this->device = nullptr;
  this->camera = nullptr;
  this->scene = nullptr;
  this->renderer = nullptr;
  this->framebuffer = nullptr;
  this->image = vec2i{ 0, 0 };

  // renderer params

  this->epsilonSlot << new megamol::core::param::FloatParam(0.001f, 0.00f, 1.0f);
  this->MakeSlotAvailable(&this->epsilonSlot);

  this->useHeadlightSlot << new megamol::core::param::BoolParam(true);
  this->MakeSlotAvailable(&this->useHeadlightSlot);

  this->lightDirSlot << new megamol::core::param::Vector3fParam(vislib::math::Vector<float, 3>(1.0f, 0.0f, 0.0f));
  this->MakeSlotAvailable(&this->lightDirSlot);

  this->lightDiffuseWeightSlot << new megamol::core::param::FloatParam(0.55f, 0.00f, 1.0f);
  this->MakeSlotAvailable(&this->lightDiffuseWeightSlot);

  this->lightAmbientWeightSlot << new megamol::core::param::FloatParam(0.1f, 0.00f, 1.0f);
  this->MakeSlotAvailable(&this->lightAmbientWeightSlot);

  this->lightExponentSlot << new megamol::core::param::FloatParam(50.0f, 0.0f);
  this->MakeSlotAvailable(&this->lightExponentSlot);

  this->alphaSlot.SetParameter(new megamol::core::param::FloatParam(1.0f, 0.0f, 1.0f));
  this->MakeSlotAvailable(&this->alphaSlot);

  vislib::math::Vector<float, 4> defaultPlane(1.0f, 0.0f, 0.0f, 0.0f);

  this->cuttingPlaneSlot << new megamol::core::param::Vector4fParam(defaultPlane);
  this->MakeSlotAvailable(&this->cuttingPlaneSlot);

  this->useCuttingPlaneSlot << new megamol::core::param::BoolParam(false);
  this->MakeSlotAvailable(&this->useCuttingPlaneSlot);

  this->accumulateSlot << new megamol::core::param::BoolParam(false);
  this->MakeSlotAvailable(&this->accumulateSlot);

  // AOOM params
  this->useAOOMSlot.SetParameter(new megamol::core::param::BoolParam(false));
  this->MakeSlotAvailable(&this->useAOOMSlot);

  this->smallCavitiesSlot.SetParameter(new megamol::core::param::BoolParam(true));
  this->MakeSlotAvailable(&this->smallCavitiesSlot);

  this->rhoSlot.SetParameter(new megamol::core::param::FloatParam(1.0f, 0.0f));
  this->MakeSlotAvailable(&this->rhoSlot);

  this->tauSlot.SetParameter(new megamol::core::param::FloatParam(0.7f, 0.01f, 1.0f));
  this->MakeSlotAvailable(&this->tauSlot);

  this->aoDistanceSlot << new megamol::core::param::FloatParam(10.0f, 0.01f);
  this->MakeSlotAvailable(&this->aoDistanceSlot);

  this->aoSamplesSlot << new megamol::core::param::IntParam(1, 0);
  this->MakeSlotAvailable(&this->aoSamplesSlot);

  vislib::math::Vector<float, 3> minVec(0.0f, 0.0f, 0.0f);
  vislib::math::Vector<float, 3> maxVec(1.0f, 1.0f, 1.0f);
  this->blendingColorSlot << new megamol::core::param::Vector3fParam(maxVec, minVec, maxVec);
  this->MakeSlotAvailable(&this->blendingColorSlot);

  this->blendingWeightSlot << new megamol::core::param::FloatParam(0.0f, 0.0f, 1.0f);
  this->MakeSlotAvailable(&this->blendingWeightSlot);

  // surface params
  megamol::core::param::EnumParam *modeParam = new megamol::core::param::EnumParam(ComputationMode::CPU);
  modeParam->SetTypePair(ComputationMode::CPU, "CPU");
  modeParam->SetTypePair(ComputationMode::VECTORIZED, "Vectorized");
  modeParam->SetTypePair(ComputationMode::PARTIALLY_VECTORIZED, "Partially vectorized");
  this->modeSlot << modeParam;
  this->MakeSlotAvailable(&this->modeSlot);

  this->probeRadiusSlot << new megamol::core::param::FloatParam(1.4f, 0.01f);
  this->MakeSlotAvailable(&this->probeRadiusSlot);

  this->skipContourSingularitiesSlot << new megamol::core::param::BoolParam(false);
  this->MakeSlotAvailable(&this->skipContourSingularitiesSlot);

  this->resizeVectorsSlot << new megamol::core::param::BoolParam(false);
  this->MakeSlotAvailable(&this->resizeVectorsSlot);

  this->computeSingularitiesSlot << new megamol::core::param::BoolParam(true);
  this->MakeSlotAvailable(&this->computeSingularitiesSlot);

  this->debugFlagSlot << new megamol::core::param::BoolParam(false);
  this->MakeSlotAvailable(&this->debugFlagSlot);

  this->useIterativeMethodSlot << new megamol::core::param::BoolParam(false);
  this->MakeSlotAvailable(&this->useIterativeMethodSlot);

  this->epsilonCuttingThresholdSlot << new megamol::core::param::FloatParam(0.001f, 0.00f, 1.0f);
  this->MakeSlotAvailable(&this->epsilonCuttingThresholdSlot);

  this->epsilonSelfIntersectSlot << new megamol::core::param::FloatParam(0.001f, 0.00f, 1.0f);
  this->MakeSlotAvailable(&this->epsilonSelfIntersectSlot);

  this->renderContourArcsSlot << new megamol::core::param::BoolParam(false);
  this->MakeSlotAvailable(&this->renderContourArcsSlot);
}

AbstractOSPRayTestRenderer::~AbstractOSPRayTestRenderer() {
  this->Release();
}

bool AbstractOSPRayTestRenderer::InitOSP() {
  // TODO shouldn't this be loaded by default???
  ospLoadModule("ispc");
  if (this->device == nullptr) {
    this->device = ospNewDevice("default");
    ospDeviceCommit(this->device);
  };
  ospSetCurrentDevice(this->device);
  ospDeviceSetErrorFunc(this->device,
    [](OSPError e, const char *msg) {
    std::cout << "OSPRAY ERROR [" << e << "]: "
      << msg << std::endl;
  });

  bool success = false;
  if (OSPRaySESModuleLoader::LoadModule()) {
    this->renderer = ospNewRenderer("ses_renderer");
    this->scene = ospNewModel();
    this->camera = ospNewCamera("perspective");
    if (this->renderer != nullptr) {
      success = true;
    }
  }

  return success;
}

bool AbstractOSPRayTestRenderer::create() {

  vislib::graphics::gl::ShaderSource vert, frag;

  if (!instance()->ShaderSourceFactory().MakeShaderSource("ospray_ses::vertex", vert)) {
    return false;
  }

  if (!instance()->ShaderSourceFactory().MakeShaderSource("ospray_ses::fragment", frag)) {
    return false;
  }

  try {
    if (!this->osprayShader.Create(vert.Code(), vert.Count(), frag.Code(), frag.Count())) {
      vislib::sys::Log::DefaultLog.WriteMsg(vislib::sys::Log::LEVEL_ERROR,
        "Unable to compile ospray ses shader: Unknown error\n");
      return false;
    }
  }
  catch (vislib::graphics::gl::AbstractOpenGLShader::CompileException ce) {
    vislib::sys::Log::DefaultLog.WriteMsg(vislib::sys::Log::LEVEL_ERROR,
      "Unable to compile ospray ses shader: (@%s): %s\n",
      vislib::graphics::gl::AbstractOpenGLShader::CompileException::CompileActionName(
        ce.FailedAction()), ce.GetMsgA());
    return false;
  }
  catch (vislib::Exception e) {
    vislib::sys::Log::DefaultLog.WriteMsg(vislib::sys::Log::LEVEL_ERROR,
      "Unable to compile ospray ses shader: %s\n", e.GetMsgA());
    return false;
  }
  catch (...) {
    vislib::sys::Log::DefaultLog.WriteMsg(vislib::sys::Log::LEVEL_ERROR,
      "Unable to compile ospray ses shader: Unknown exception\n");
    return false;
  }

  if (!this->InitOSP()) {
    return false;
  }
  this->InitGL();

  this->surfaceParamValues.dirty = false;
  this->rendererParamValues.dirty = false;
  this->ReadInterfaceSurface();
  this->ReadInterfaceRenderer();

  return true;
}

void AbstractOSPRayTestRenderer::release(void) {
  this->DestroyGL();
  this->DestroyOSP();

  this->osprayShader.Release();
}

void AbstractOSPRayTestRenderer::DestroyOSP() {
  // leads to crash
  /*if (this->device != nullptr) {
    ospRelease(this->device);
    this->device = nullptr;
  }*/
  if (this->camera != nullptr) {
    ospRelease(this->camera);
    this->camera = nullptr;
  }
  this->ClearGeometry();
  if (this->scene != nullptr) {
    ospRelease(this->scene);
    this->scene = nullptr;
  }
  if (this->renderer != nullptr) {
    ospRelease(this->renderer);
    this->renderer = nullptr;
  }
  if (this->framebuffer != nullptr) {
    ospFreeFrameBuffer(this->framebuffer);
    this->framebuffer = nullptr;
  }

  ospShutdown();
}

void AbstractOSPRayTestRenderer::InitGL() {

  GLfloat verts[] = { 0.0f,0.0f, 1.0f,0.0f, 0.0f,1.0f, 1.0f,1.0f };

  glGenVertexArrays(1, &this->vaRect);
  glGenBuffers(1, &this->vbo);

  glBindVertexArray(this->vaRect);
  glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
  glEnableVertexAttribArray(0);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * 4, verts, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glEnable(GL_TEXTURE_2D);
  glGenTextures(1, &this->ospFbTex);
  glBindTexture(GL_TEXTURE_2D, this->ospFbTex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);
  glDisable(GL_TEXTURE_2D);
}

void AbstractOSPRayTestRenderer::DestroyGL() {
  glDeleteBuffers(1, &this->vbo);
  glDeleteVertexArrays(1, &this->vaRect);
  glDeleteTextures(1, &this->ospFbTex);
}

void AbstractOSPRayTestRenderer::UpdateCameraAndRenderer(const vislib::SmartPtr<vislib::graphics::CameraParameters> &camParams, bool clearBuffer) {
  const vislib::math::Point<vislib::graphics::SceneSpaceType, 3U> pos = camParams->EyePosition();
  vec3f position;
  position.x = pos.GetX() / this->scale;
  position.y = pos.GetY() / this->scale;
  position.z = pos.GetZ() / this->scale;
  ospSetVec3f(this->camera, "pos", position);

  float aspect = static_cast<float>(camParams->TileRect().AspectRatio());
  ospSetf(this->camera, "aspect", aspect);

  const vislib::math::Vector<vislib::graphics::SceneSpaceType, 3U> up = camParams->EyeUpVector();
  vec3f upVec;
  upVec.x = up.GetX();
  upVec.y = up.GetY();
  upVec.z = up.GetZ();
  ospSetVec3f(this->camera, "up", upVec);

  const vislib::math::Vector<vislib::graphics::SceneSpaceType, 3U> dir = camParams->EyeDirection();
  vec3f direction;
  direction.x = dir.GetX();
  direction.y = dir.GetY();
  direction.z = dir.GetZ();
  ospSetVec3f(this->camera, "dir", direction);

  ospCommit(this->camera);
  ospSetObject(this->renderer, "camera", this->camera);
  vec2i _image{ static_cast<int>(camParams->TileRect().Width()), static_cast<int>(camParams->TileRect().Height()) };

  // framebuffer accumulation control
  const bool accumulate = this->accumulateSlot.Param<megamol::core::param::BoolParam>()->Value();
  const bool accumChanged = this->previousAccumulateFlag != accumulate;

  const uint32_t framebufferFlags = accumulate ? OSP_FB_COLOR | OSP_FB_ACCUM : OSP_FB_COLOR;

  // init/renew framebuffer
  if (this->framebuffer == nullptr || accumChanged || this->image.x != _image.x || this->image.y != _image.y) {
    // store old camera params to check if they changed
    //if (this->framebuffer == nullptr) {
    //  ospFreeFrameBuffer(this->framebuffer);
    //}

    this->framebuffer = ospNewFrameBuffer(_image, OSP_FB_SRGBA, framebufferFlags);
    this->image = _image;
  }

  const bool camChanged = previousCamParams.EyeDirection().GetX() != camParams->EyeDirection().GetX()
    || previousCamParams.EyeDirection().GetY() != camParams->EyeDirection().GetY()
    || previousCamParams.EyeDirection().GetZ() != camParams->EyeDirection().GetZ();


  if (camChanged || accumChanged || clearBuffer) {
    ospFrameBufferClear(this->framebuffer, framebufferFlags);
  }
  this->previousCamParams.CopyFrom(camParams);
  this->previousAccumulateFlag = accumulate;

  this->SetRenderParams(camParams);
}

OSPModel AbstractOSPRayTestRenderer::GetScene() {
  return this->scene;
}

OSPRenderer AbstractOSPRayTestRenderer::GetRenderer() {
  return this->renderer;
}

OSPGeometry AbstractOSPRayTestRenderer::CreateSurface() {
  return ospNewGeometry("ses_surface");
}

void AbstractOSPRayTestRenderer::AddGeometry(OSPGeometry geometry) {
  this->geometries.push_back(geometry);
}

void AbstractOSPRayTestRenderer::FinalizeGeometry() {
  for (OSPGeometry g : this->geometries) {
    ospAddGeometry(this->scene, g);
  }
  ospCommit(this->scene);
  ospSetObject(this->renderer, "model", this->scene);
  ospCommit(this->renderer);
}

int AbstractOSPRayTestRenderer::GetGeometryCnt() {
  return static_cast<int>(this->geometries.size());
}

void AbstractOSPRayTestRenderer::ClearGeometry() {
  for (OSPGeometry g : this->geometries) {
    ospRemoveGeometry(this->scene, g);
    //ospRelease(g);
  }
  this->geometries.clear();
}

void AbstractOSPRayTestRenderer::ReadInterfaceRenderer() {

  this->rendererParamValues.epsilon = this->epsilonSlot.Param<megamol::core::param::FloatParam>()->Value();
  this->rendererParamValues.useHeadLight = this->useHeadlightSlot.Param<megamol::core::param::BoolParam>()->Value();
  const vislib::math::Vector<float, 3> lightDir(this->lightDirSlot.Param<megamol::core::param::Vector3fParam>()->Value());

  this->rendererParamValues.lightDir = normalize(vec3f{ lightDir.X(), lightDir.Y(), lightDir.Z() });
  this->rendererParamValues.lightDiffuseWeight = this->lightDiffuseWeightSlot.Param<megamol::core::param::FloatParam>()->Value();
  this->rendererParamValues.lightAmbientWeight = this->lightAmbientWeightSlot.Param<megamol::core::param::FloatParam>()->Value();
  this->rendererParamValues.lightExponent = this->lightExponentSlot.Param<megamol::core::param::FloatParam>()->Value();
  this->rendererParamValues.alpha = this->alphaSlot.Param<megamol::core::param::FloatParam>()->Value();

  this->rendererParamValues.useCuttingPlane = this->useCuttingPlaneSlot.Param<megamol::core::param::BoolParam>()->Value();
  const vislib::math::Vector<float, 4> plane(this->cuttingPlaneSlot.Param<megamol::core::param::Vector4fParam>()->Value());
  const vec3f normal = normalize(vec3f{ plane.X(), plane.Y(), plane.Z() });
  this->rendererParamValues.cuttingPlane = vec4f{ normal.x, normal.y, normal.z, plane.W() };

  this->rendererParamValues.useAOOM = this->useAOOMSlot.Param<megamol::core::param::BoolParam>()->Value();
  this->rendererParamValues.smallCavities = this->smallCavitiesSlot.Param<megamol::core::param::BoolParam>()->Value();
  this->rendererParamValues.rho = this->rhoSlot.Param<megamol::core::param::FloatParam>()->Value();
  this->rendererParamValues.tau = this->tauSlot.Param<megamol::core::param::FloatParam>()->Value();
  this->rendererParamValues.aoDistance = this->aoDistanceSlot.Param<megamol::core::param::FloatParam>()->Value();
  this->rendererParamValues.aoSamples = this->aoSamplesSlot.Param<megamol::core::param::IntParam>()->Value();

  const vislib::math::Vector<float, 3> blendingColor = this->blendingColorSlot.Param<megamol::core::param::Vector3fParam>()->Value();

  this->rendererParamValues.blendingColor = vec3f{ blendingColor.X(), blendingColor.Y(), blendingColor.Z() };
  this->rendererParamValues.blendingWeight = this->blendingWeightSlot.Param<megamol::core::param::FloatParam>()->Value();

}

void AbstractOSPRayTestRenderer::SetRenderParams(const vislib::SmartPtr<vislib::graphics::CameraParameters> &camParams) {

  if (this->rendererParamValues.useHeadLight) {
    const vislib::math::Vector<float, 3> eyeDir = camParams->EyeDirection();
    ospSetVec3f(this->renderer, "lightDir", normalize(vec3f{ eyeDir.X(), eyeDir.Y(), eyeDir.Z() }));
  } else {
    ospSetVec3f(this->renderer, "lightDir", this->rendererParamValues.lightDir);
  }

  ospSet1f(this->renderer, "diffWeight", this->rendererParamValues.lightDiffuseWeight);
  ospSet1f(this->renderer, "ambWeigth", this->rendererParamValues.lightAmbientWeight);
  ospSet1f(this->renderer, "exponent", this->rendererParamValues.lightExponent);
  ospSet1f(this->renderer, "epsilon", this->rendererParamValues.epsilon);
  ospSet1f(this->renderer, "alpha", this->rendererParamValues.alpha);

  ospSet1i(this->renderer, "use_cutting_plane", static_cast<int>(this->rendererParamValues.useCuttingPlane));
  ospSetVec4f(this->renderer, "cutting_plane", this->rendererParamValues.cuttingPlane);

  ospSet1i(this->renderer, "use_aoom", static_cast<int>(this->rendererParamValues.useAOOM));
  ospSet1i(this->renderer, "small_cavities", static_cast<int>(this->rendererParamValues.smallCavities));
  ospSet1f(this->renderer, "rho", this->rendererParamValues.rho);
  ospSet1f(this->renderer, "tau", this->rendererParamValues.tau);
  ospSet1f(this->renderer, "aoDistance", this->rendererParamValues.aoDistance);
  ospSet1i(this->renderer, "aoSamples", this->rendererParamValues.aoSamples);
  ospSetVec3f(this->renderer, "blendColor", this->rendererParamValues.blendingColor);
  ospSet1f(this->renderer, "blendWeight", this->rendererParamValues.blendingWeight);
  
  // commit renderer
  ospCommit(this->renderer);
}

void AbstractOSPRayTestRenderer::SetSurfaceParams(OSPGeometry sesGeometry) {
  switch (this->surfaceParamValues.mode) {
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

  ospSet1i(sesGeometry, "debug", this->surfaceParamValues.debugFlag);
  ospSet1f(sesGeometry, "probe_radius", static_cast<float>(this->surfaceParamValues.probeRadius));
  ospSet1i(sesGeometry, "compute_spherical_triangle_singularities", static_cast<int>(this->surfaceParamValues.computeSingularities));
  ospSet1i(sesGeometry, "skip_contour_singularities", static_cast<int>(this->surfaceParamValues.skipContourSingularities));
  ospSet1i(sesGeometry, "resize_vectors", static_cast<int>(this->surfaceParamValues.resizeVectors));
  ospSet1i(sesGeometry, "toroidal_patches_use_iterative_method", static_cast<int>(this->surfaceParamValues.useIterativeMethod));
  ospSet1f(sesGeometry, "epsilon_cutting_threshold", this->surfaceParamValues.epsilonCuttingThreshold);
  ospSet1f(sesGeometry, "epsilon_self_intersect", this->surfaceParamValues.epsilonSelfIntersect);
  ospSet1i(sesGeometry, "compute_contour_arcs", static_cast<int>(this->surfaceParamValues.renderContourArcs));
}


void AbstractOSPRayTestRenderer::ReadInterfaceSurface() {

  // surface
  this->surfaceParamValues.probeRadius = this->probeRadiusSlot.Param<megamol::core::param::FloatParam>()->Value();
  this->surfaceParamValues.skipContourSingularities = this->skipContourSingularitiesSlot.Param<megamol::core::param::BoolParam>()->Value();
  this->surfaceParamValues.computeSingularities = this->computeSingularitiesSlot.Param<megamol::core::param::BoolParam>()->Value();
  this->surfaceParamValues.resizeVectors = this->resizeVectorsSlot.Param<megamol::core::param::BoolParam>()->Value();
  this->surfaceParamValues.debugFlag = this->debugFlagSlot.Param<megamol::core::param::BoolParam>()->Value();
  this->surfaceParamValues.useIterativeMethod = this->useIterativeMethodSlot.Param<megamol::core::param::BoolParam>()->Value();

  this->surfaceParamValues.epsilonCuttingThreshold = this->epsilonCuttingThresholdSlot.Param<megamol::core::param::FloatParam>()->Value();
  this->surfaceParamValues.epsilonSelfIntersect = this->epsilonSelfIntersectSlot.Param<megamol::core::param::FloatParam>()->Value();

  this->surfaceParamValues.renderContourArcs = this->renderContourArcsSlot.Param<megamol::core::param::BoolParam>()->Value();
  this->surfaceParamValues.mode = static_cast<ComputationMode>(this->modeSlot.Param<megamol::core::param::EnumParam>()->Value());

}

bool AbstractOSPRayTestRenderer::RenderPre() {

  if (this->scene == nullptr) {
    this->scene = ospNewModel();
  }

  this->surfaceParamValues.dirty = false;
  if (this->modeSlot.IsDirty() || this->probeRadiusSlot.IsDirty() || this->skipContourSingularitiesSlot.IsDirty() || this->resizeVectorsSlot.IsDirty() || this->computeSingularitiesSlot.IsDirty()
    || this->debugFlagSlot.IsDirty() || this->useIterativeMethodSlot.IsDirty() || this->epsilonCuttingThresholdSlot.IsDirty() || this->epsilonSelfIntersectSlot.IsDirty() || this->renderContourArcsSlot.IsDirty()) {
    this->modeSlot.ResetDirty();
    this->probeRadiusSlot.ResetDirty();
    this->skipContourSingularitiesSlot.ResetDirty();
    this->resizeVectorsSlot.ResetDirty();
    this->computeSingularitiesSlot.ResetDirty();
    this->debugFlagSlot.ResetDirty();
    this->useIterativeMethodSlot.ResetDirty();
    this->epsilonCuttingThresholdSlot.ResetDirty();
    this->epsilonSelfIntersectSlot.ResetDirty();
    this->renderContourArcsSlot.ResetDirty();
    this->surfaceParamValues.dirty = true;
  }

  this->rendererParamValues.dirty = false;
  if (this->epsilonSlot.IsDirty() || this->alphaSlot.IsDirty() || this->rhoSlot.IsDirty() || this->tauSlot.IsDirty()
    || this->useCuttingPlaneSlot.IsDirty() || this->cuttingPlaneSlot.IsDirty()
    || this->lightDirSlot.IsDirty() || this->useHeadlightSlot.IsDirty() || this->lightDiffuseWeightSlot.IsDirty()
    || this->lightAmbientWeightSlot.IsDirty() || this->lightExponentSlot.IsDirty()
    || this->useAOOMSlot.IsDirty() || this->smallCavitiesSlot.IsDirty() || this->aoDistanceSlot.IsDirty() || this->aoSamplesSlot.IsDirty()
    || this->blendingColorSlot.IsDirty() || this->blendingWeightSlot.IsDirty()) {
    this->alphaSlot.ResetDirty();
    this->rhoSlot.ResetDirty();
    this->tauSlot.ResetDirty();
    this->epsilonSlot.ResetDirty();
    this->useCuttingPlaneSlot.ResetDirty();
    this->cuttingPlaneSlot.ResetDirty();
    this->lightDirSlot.ResetDirty();
    this->lightDiffuseWeightSlot.ResetDirty();
    this->lightAmbientWeightSlot.ResetDirty();
    this->lightExponentSlot.ResetDirty();
    this->useHeadlightSlot.ResetDirty();
    this->useAOOMSlot.ResetDirty();
    this->smallCavitiesSlot.ResetDirty();
    this->aoDistanceSlot.ResetDirty();
    this->aoSamplesSlot.ResetDirty();
    this->blendingColorSlot.ResetDirty();
    this->blendingWeightSlot.ResetDirty();
    this->rendererParamValues.dirty = true;
  }

  if (this->surfaceParamValues.dirty) {
    this->ReadInterfaceSurface();
  }

  if (this->rendererParamValues.dirty) {
    this->ReadInterfaceRenderer();
  }

  return true;
}

bool AbstractOSPRayTestRenderer::RenderPost() {
  ospRenderFrame(this->framebuffer, this->renderer, OSP_FB_COLOR);
  const uint32_t *fb = (uint32_t*)ospMapFrameBuffer(framebuffer, OSP_FB_COLOR);
  this->osprayShader.Enable();
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, this->ospFbTex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->image.x, this->image.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, fb);

  glUniform1i(this->osprayShader.ParameterLocation("tex"), 0);
  glBindVertexArray(this->vaRect);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
  this->osprayShader.Disable();
  ospUnmapFrameBuffer(fb, this->framebuffer);
  this->surfaceParamValues.dirty = false;
  this->rendererParamValues.dirty = false;
  return true;
}
