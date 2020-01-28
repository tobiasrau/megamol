/*
 * CallClassRegistry.cpp
 * Copyright (C) 2008 - 2015 by MegaMol Consortium
 * All rights reserved. Alle Rechte vorbehalten.
 */

#include "stdafx.h"
#include "factories/CallClassRegistry.h"

#include "mmcore/factories/CallDescriptionManager.h"
#include "mmcore/factories/CallAutoDescription.h"
#include "mmcore/factories/CallDescription.h"
#include "mmcore/DataWriterCtrlCall.h"
#include "mmcore/moldyn/MultiParticleDataCall.h"
#include "mmcore/view/CallClipPlane.h"
#include "mmcore/view/CallRender3D_2.h"
#include "mmcore/view/CallRenderView.h"
#include "mmcore/view/CallTimeControl.h"
#include "mmcore/view/CallCamParams.h"
#include "mmcore/view/CallCamParamSync.h"
#include "mmcore/CallVolumeData.h"
#include "mmcore/moldyn/VolumeDataCall.h"
#include "mmcore/misc/BezierCurvesListDataCall.h"
#include "mmcore/misc/VolumetricDataCall.h"
#include "mmcore/misc/QRCodeDataCall.h"
#include "mmcore/misc/CalloutImageCall.h"
#include "mmcore/view/Call6DofInteraction.h"
#include "mmcore/cluster/mpi/MpiCall.h"
#include "mmcore/moldyn/EllipsoidalDataCall.h"
#include "mmcore/moldyn/ParticleRelistCall.h"
#include "mmcore/view/light/CallLight.h"
#include "mmcore/job/TickCall.h"
#include "mmcore/DirectDataWriterCall.h"
#include "mmcore/cluster/SyncDataSourcesCall.h"
#include "mmcore/FlagCall.h"
#include "mmcore/view/CallGetTransferFunction.h"
#ifdef HAS_OPENGL
#include "mmcore/view/CallRender2D.h"
#include "mmcore/FlagCall_GL.h"
#include "mmcore/view/CallGetTransferFunction_gl.h"
#include "mmcore/view/CallRenderDeferred3D.h"
#include "mmcore/view/special/CallbackScreenShooter.h"
#endif

using namespace megamol::core;


/*
 * factories::register_call_classes
 */
void factories::register_call_classes(factories::CallDescriptionManager& instance) {
    //////////////////////////////////////////////////////////////////////
    // Register all rendering graph call descriptions here
    //////////////////////////////////////////////////////////////////////
    instance.RegisterAutoDescription<moldyn::MultiParticleDataCall>();
    instance.RegisterAutoDescription<view::CallClipPlane>();
    instance.RegisterAutoDescription<view::CallRender3D_2>();
    instance.RegisterAutoDescription<view::CallRenderView>();
    instance.RegisterAutoDescription<view::CallTimeControl>();
    instance.RegisterAutoDescription<DataWriterCtrlCall>();
    instance.RegisterAutoDescription<CallVolumeData>();
    instance.RegisterAutoDescription<moldyn::VolumeDataCall>();
    instance.RegisterAutoDescription<misc::BezierCurvesListDataCall>();
    instance.RegisterAutoDescription<misc::VolumetricDataCall>();
    instance.RegisterAutoDescription<view::CallCamParams>();
    instance.RegisterAutoDescription<view::CallCamParamSync>();
    instance.RegisterAutoDescription<misc::QRCodeDataCall>();
    instance.RegisterAutoDescription<misc::CalloutImageCall>();
    instance.RegisterAutoDescription<view::Call6DofInteraction>();
    instance.RegisterAutoDescription<cluster::mpi::MpiCall>();
    instance.RegisterAutoDescription<moldyn::EllipsoidalParticleDataCall>();
    instance.RegisterAutoDescription<moldyn::ParticleRelistCall>();
    instance.RegisterAutoDescription<view::light::CallLight>();
    instance.RegisterAutoDescription<job::TickCall>();
    instance.RegisterAutoDescription<DirectDataWriterCall>();
    instance.RegisterAutoDescription<cluster::SyncDataSourcesCall>();
    instance.RegisterAutoDescription<view::CallGetTransferFunction>();
    instance.RegisterAutoDescription<FlagCall>();
#ifdef HAS_OPENGL
    instance.RegisterAutoDescription<view::CallRender2D>();
    instance.RegisterAutoDescription<FlagCallRead_GL>();
    instance.RegisterAutoDescription<FlagCallWrite_GL>();
    instance.RegisterAutoDescription<view::CallGetTransferFunction_gl>();
    instance.RegisterAutoDescription<view::CallRenderDeferred3D>();
    instance.RegisterAutoDescription<view::special::CallbackScreenShooterCall>();
#endif
}
