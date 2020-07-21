/*
 * ModuleClassRegistry.cpp
 * Copyright (C) 2008 - 2015 by MegaMol Consortium
 * All rights reserved. Alle Rechte vorbehalten.
 */

#include "stdafx.h"
#include "factories/ModuleClassRegistry.h"

#include "mmcore/factories/ModuleDescriptionManager.h"
#include "mmcore/factories/LoaderADModuleAutoDescription.h"
#include "mmcore/factories/ModuleAutoDescription.h"
#include "mmcore/factories/ModuleDescription.h"
#include "mmcore/cluster/mpi/MpiProvider.h"
#include "mmcore/misc/SiffCSplineFitter.h"
#include "mmcore/misc/TestSpheresDataSource.h"
#include "mmcore/moldyn/MMPLDDataSource.h"
#include "mmcore/moldyn/MMPLDWriter.h"
#include "mmcore/moldyn/DirPartColModulate.h"
#include "mmcore/moldyn/DirPartFilter.h"
#include "mmcore/moldyn/ParticleListFilter.h"
#include "mmcore/special/StubModule.h"
#include "mmcore/view/ClipPlane.h"
#include "mmcore/view/TileView.h"
#include "mmcore/view/View3D_2.h"
#include "mmcore/view/SharedCameraParameters.h"
#include "mmcore/job/DataWriterJob.h"
#include "mmcore/job/JobThread.h"
#include "mmcore/moldyn/DynDensityGradientEstimator.h"
#include "job/PluginsStateFileGeneratorJob.h"
#include "job/TickSwitch.h"
#include "mmcore/utility/LuaHostSettingsModule.h"
#include "mmcore/view/light/AmbientLight.h"
#include "mmcore/view/light/DistantLight.h"
#include "mmcore/view/light/HDRILight.h"
#include "mmcore/view/light/PointLight.h"
#include "mmcore/view/light/QuadLight.h"
#include "mmcore/view/light/SpotLight.h"
#include "mmcore/FileStreamProvider.h"
#include "mmcore/FlagStorage.h"
#ifdef HAS_OPENGL
#include "mmcore/FlagStorage_GL.h"
#include "mmcore/DeferredShading.h"
#include "mmcore/view/special/CallbackScreenShooter.h"
#include "mmcore/view/SplitView.h"
#include "mmcore/view/HeadView.h"
#include "mmcore/moldyn/AddParticleColours.h"
#include "mmcore/view/TransferFunction.h"
#include "mmcore/moldyn/AddClusterColours.h"
#include "mmcore/view/View2D.h"
#include "mmcore/view/BoundingBoxRenderer.h"
#endif

using namespace megamol::core;

/*

 * factories::register_module_classes
 */
void factories::register_module_classes(factories::ModuleDescriptionManager& instance) {

    //////////////////////////////////////////////////////////////////////
    // Register all rendering graph module descriptions here
    //////////////////////////////////////////////////////////////////////

    instance.RegisterAutoDescription<misc::SiffCSplineFitter>();
    instance.RegisterAutoDescription<misc::TestSpheresDataSource>();
    instance.RegisterAutoDescription<moldyn::MMPLDDataSource>();
    instance.RegisterAutoDescription<moldyn::MMPLDWriter>();
    instance.RegisterAutoDescription<moldyn::DirPartColModulate>();
    instance.RegisterAutoDescription<moldyn::ParticleListFilter>();
    instance.RegisterAutoDescription<moldyn::DirPartFilter>();
    instance.RegisterAutoDescription<special::StubModule>();
    instance.RegisterAutoDescription<view::ClipPlane>();
    instance.RegisterAutoDescription<view::TileView>();
	instance.RegisterAutoDescription<view::View3D_2>();
    instance.RegisterAutoDescription<view::SharedCameraParameters>();
    instance.RegisterAutoDescription<job::DataWriterJob>();
    instance.RegisterAutoDescription<job::JobThread>();
    instance.RegisterAutoDescription<moldyn::DynDensityGradientEstimator>();
    instance.RegisterAutoDescription<job::PluginsStateFileGeneratorJob>();
    instance.RegisterAutoDescription<core::utility::LuaHostSettingsModule>();
    instance.RegisterAutoDescription<core::FileStreamProvider>();
    instance.RegisterAutoDescription<view::light::AmbientLight>();
    instance.RegisterAutoDescription<view::light::DistantLight>();
    instance.RegisterAutoDescription<view::light::HDRILight>();
    instance.RegisterAutoDescription<view::light::PointLight>();
    instance.RegisterAutoDescription<view::light::QuadLight>();
    instance.RegisterAutoDescription<view::light::SpotLight>();
    instance.RegisterAutoDescription<FlagStorage>();
    instance.RegisterAutoDescription<job::TickSwitch>();

    #ifdef HAS_OPENGL
    instance.RegisterAutoDescription<FlagStorage_GL>();
    instance.RegisterAutoDescription<DeferredShading>();
    instance.RegisterAutoDescription<view::special::CallbackScreenShooter>();
    instance.RegisterAutoDescription<view::SplitView>();
    instance.RegisterAutoDescription<view::HeadView>();
    instance.RegisterAutoDescription<moldyn::AddParticleColours>();
    instance.RegisterAutoDescription<view::TransferFunction>();
    instance.RegisterAutoDescription<view::special::ScreenShooter>();
    instance.RegisterAutoDescription<view::View2D>();
    instance.RegisterAutoDescription<moldyn::AddClusterColours>();
    instance.RegisterAutoDescription<view::BoundingBoxRenderer>();
    #endif
}
