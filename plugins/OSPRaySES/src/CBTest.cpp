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
#include "mmcore/param/ButtonParam.h"
#include "mmcore/param/FloatParam.h"
#include "mmcore/param/BoolParam.h"
#include "CBTest.h"

using namespace megamol::ospray::ses;


CBTest::CBTest() :
  AbstractOSPRaySESRenderer(),
  sphereParamSlot("sphereParam", "Sphere param"),
  radiusParamSlot("radiusParam", "Radius param"),
  pushSphereSlot("pushSphere", "Push sphere"),
  popSphereSlot("popSphere", "Pop sphere"),
  pushRandomSphereSlot("pushRandomSphere", "Push random sphere"),
  randPositionParamSlot("randPosition", "Random position domain"),
  distribution(0.0f, 1.0f) {

  vislib::math::Vector<float, 3> defaultSphere(0.0f, 0.0f, 0.0f);

  this->sphereParamSlot << new megamol::core::param::Vector3fParam(defaultSphere);
  this->MakeSlotAvailable(&this->sphereParamSlot);

  this->radiusParamSlot << new megamol::core::param::FloatParam(1.0f, 0.01f);
  this->MakeSlotAvailable(&this->radiusParamSlot);

  this->pushSphereSlot << new megamol::core::param::ButtonParam();
  this->pushSphereSlot.SetUpdateCallback(&CBTest::PushSphereCallback);
  this->MakeSlotAvailable(&this->pushSphereSlot);

  this->popSphereSlot << new megamol::core::param::ButtonParam();
  this->popSphereSlot.SetUpdateCallback(&CBTest::PopSphereCallback);
  this->MakeSlotAvailable(&this->popSphereSlot);

  this->pushRandomSphereSlot << new megamol::core::param::ButtonParam();
  this->pushRandomSphereSlot.SetUpdateCallback(&CBTest::PushRandomSphereCallback);
  this->MakeSlotAvailable(&this->pushRandomSphereSlot);

  this->randPositionParamSlot << new megamol::core::param::FloatParam(1.0f, 0.01f);
  this->MakeSlotAvailable(&this->randPositionParamSlot);

  generator.seed({ 42 });

  this->colors.push_back(vec4f{ 1.0f, 1.0f, 1.0f, 1.0f });
}

CBTest::~CBTest() {
  this->Release();
}

bool CBTest::LoadData() {
  this->sesSpheres.clear();
  this->sesSpheres.insert(this->sesSpheres.end(), this->spheres.begin(), this->spheres.end());
  return true;
}

bool CBTest::PushSphereCallback(megamol::core::param::ParamSlot &paramSlot) {

  vislib::math::Vector<float, 3> param = this->sphereParamSlot.Param<megamol::core::param::Vector3fParam>()->Value();
  OSPRaySESSphere sphere;
  sphere.center = vec3f(param.X(), param.Y(), param.Z());
  sphere.radius = this->radiusParamSlot.Param<megamol::core::param::FloatParam>()->Value();
  sphere.colorID = 0;
  this->spheres.push_back(sphere);
  this->recompute = true;
  this->dataLoaded = false;
  return true;
}

bool CBTest::PopSphereCallback(megamol::core::param::ParamSlot &paramSlot) {

  if (this->spheres.size() == 0) {
    return true;
  }

  this->spheres.pop_back();
  this->recompute = true;
  this->dataLoaded = false;
  return true;
}

bool CBTest::PushRandomSphereCallback(megamol::core::param::ParamSlot &paramSlot) {
  const float rand = this->randPositionParamSlot.Param<megamol::core::param::FloatParam>()->Value();
  OSPRaySESSphere sphere;
  sphere.center = vec3f(this->distribution(generator) * 2 * rand - rand, this->distribution(generator) * 2 * rand - rand, this->distribution(generator) * 2 * rand - rand);
  sphere.radius = this->radiusParamSlot.Param<megamol::core::param::FloatParam>()->Value();
  sphere.colorID = 0;
  this->spheres.push_back(sphere);

  vislib::math::Vector<float, 3> _sphere(sphere.center.x, sphere.center.y, sphere.center.z);
  this->sphereParamSlot.Param<megamol::core::param::Vector3fParam>()->SetValue(_sphere);

  this->recompute = true;
  this->dataLoaded = false;
  return true;
}

bool CBTest::GetExtents(megamol::core::Call & call) {
  // TODO something is not right here
  megamol::core::view::CallRender3D *cr = dynamic_cast<megamol::core::view::CallRender3D*>(&call);
  if (cr == nullptr) return false;

  cr->AccessBoundingBoxes().SetObjectSpaceBBox(-1.0, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f);
  cr->AccessBoundingBoxes().MakeScaledWorld(1.0f);
  cr->SetTime(0);
  cr->SetTimeFramesCount(1);
  return true;

}