/*
 * Module.cpp
 *
 * Copyright (C) 2009-2015 by MegaMol Team
 * Alle Rechte vorbehalten.
 */

#include "stdafx.h"
#include "mmcore/RigRendering.h"
#include "mmcore/Module.h"
#include "mmcore/AbstractSlot.h"
#include "mmcore/CoreInstance.h"
#include <typeinfo>
#include "vislib/assert.h"
#include "vislib/sys/AutoLock.h"
#include "vislib/IllegalParamException.h"
#include "vislib/IllegalStateException.h"
#include "vislib/sys/Log.h"

#include "OpenGL_Context.h"

using namespace megamol::core;


/*
 * Module::Module
 */
Module::Module(void) : AbstractNamedObjectContainer(), created(false) {
    // intentionally empty ATM
}


/*
 * Module::~Module
 */
Module::~Module(void) {
    if (this->created == true) {
        throw vislib::IllegalStateException(
            "You must release all resources in the proper derived dtor.",
            __FILE__, __LINE__);
    }
}


/*
 * Module::Create
 */
bool Module::Create(std::vector<megamol::render_api::RenderResource> dependencies) {

	const megamol::input_events::IOpenGL_Context* opengl_context = nullptr;
    auto opengl_context_it = std::find_if(dependencies.begin(), dependencies.end(),
        [&](megamol::render_api::RenderResource& dep) { return dep.getIdentifier() == "IOpenGL_Context"; });

    if (opengl_context_it != dependencies.end() &&  opengl_context_it->getResource<megamol::input_events::IOpenGL_Context>().has_value()) {
        opengl_context = &opengl_context_it->getResource<megamol::input_events::IOpenGL_Context>().value().get();
    }

	if (opengl_context)
		opengl_context->activate();

    using vislib::sys::Log;
    ASSERT(this->instance() != NULL);
    if (!this->created) {
        this->created = this->create();
        Log::DefaultLog.WriteMsg(Log::LEVEL_INFO + 350,
            "%s module \"%s\"\n", ((this->created) ? "Created"
            : "Failed to create"), typeid(*this).name());
    }
    if (this->created) {
        // Now reregister parents at children
        this->fixParentBackreferences();
    }

	if (opengl_context)
		opengl_context->close();

    return this->created;
}


/*
 * Module::FindSlot
 */
AbstractSlot * Module::FindSlot(const vislib::StringA& name) {
    child_list_type::iterator iter, end;
    iter = this->ChildList_Begin();
    end = this->ChildList_End();
    for (; iter != end; ++iter) {
        AbstractSlot* slot = dynamic_cast<AbstractSlot*>(iter->get());
        if (slot == NULL) continue;
        if (slot->Name().Equals(name, false)) {
            return slot;
        }
    }
    return NULL;
}


/*
 * Module::GetDemiRootName
 */
vislib::StringA Module::GetDemiRootName() const {
    AbstractNamedObject::const_ptr_type tm = this->shared_from_this();
    while (tm->Parent() && !tm->Parent()->Name().IsEmpty()) {
        tm = tm->Parent();
    }
    return tm->Name();
}


/*
 * Module::Release
 */
void Module::Release(std::vector<megamol::render_api::RenderResource> dependencies) {

    auto opengl_context_it = std::find_if(dependencies.begin(), dependencies.end(),
        [&](megamol::render_api::RenderResource& dep) { return dep.getIdentifier() == "IOpenGL_Context"; });

	const megamol::input_events::IOpenGL_Context* opengl_context = nullptr;

    if (opengl_context_it != dependencies.end() &&  opengl_context_it->getResource<megamol::input_events::IOpenGL_Context>().has_value()) {
        opengl_context = &opengl_context_it->getResource<megamol::input_events::IOpenGL_Context>().value().get();
    }

	if (opengl_context)
		opengl_context->activate();

    using vislib::sys::Log;
    if (this->created) {
        this->release();
        this->created = false;
        Log::DefaultLog.WriteMsg(Log::LEVEL_INFO + 350,
            "Released module \"%s\"\n", typeid(*this).name());
    }

	if (opengl_context)
		opengl_context->close();
}


/*
 * Module::ClearCleanupMark
 */
void Module::ClearCleanupMark(void) {
    if (!this->CleanupMark()) return;

    AbstractNamedObject::ClearCleanupMark();

    child_list_type::iterator iter, end;
    iter = this->ChildList_Begin();
    end = this->ChildList_End();
    for (; iter != end; ++iter) {
        (*iter)->ClearCleanupMark();
    }

    AbstractNamedObject::ptr_type p = this->Parent();
    if (p) {
        p->ClearCleanupMark();
    }
}


/*
 * Module::PerformCleanup
 */
void Module::PerformCleanup(void) {
    // Do not proceed into the children, because they will be deleted
    // automatically
    AbstractNamedObject::PerformCleanup();

    if (!this->CleanupMark()) return;

    // clear list of children
    child_list_type::iterator b, e;
    while(true) {
        b = this->ChildList_Begin();
        e = this->ChildList_End();
        if (b == e) break;
        this->removeChild(*b);
    }

}


/*
 * Module::getRelevantConfigValue
 */
vislib::StringA Module::getRelevantConfigValue(vislib::StringA name) {
    vislib::StringA ret = vislib::StringA::EMPTY;
    const utility::Configuration& cfg = this->GetCoreInstance()->Configuration();
    vislib::StringA drn = this->GetDemiRootName();
    vislib::StringA test = drn;
    test.Append("-");
    test.Append(name);
    vislib::StringA test2("*-");
    test2.Append(name);
    if (cfg.IsConfigValueSet(test)) {
        ret = cfg.ConfigValue(test);
    } else if (cfg.IsConfigValueSet(test2)) {
        ret = cfg.ConfigValue(test2);
    } else if (cfg.IsConfigValueSet(name)) {
        ret = cfg.ConfigValue(name);
    }

    return ret;
}


/*
 * Module::MakeSlotAvailable
 */
void Module::MakeSlotAvailable(AbstractSlot *slot) {
    if (slot == NULL) {
        throw vislib::IllegalParamException("slot", __FILE__, __LINE__);
    }
    if (slot->GetStatus() != AbstractSlot::STATUS_UNAVAILABLE) {
        throw vislib::IllegalStateException("slot", __FILE__, __LINE__);
    }
    if (this->FindSlot(slot->Name()))  {
        throw vislib::IllegalParamException(
            "A slot with this name is already registered",
            __FILE__, __LINE__);
    }
    this->addChild(::std::shared_ptr<AbstractNamedObject>(slot, [](AbstractNamedObject* d){}));
    slot->SetOwner(this);
    slot->MakeAvailable();
}

void Module::SetSlotUnavailable(AbstractSlot *slot) {
    if (slot == NULL) {
        throw vislib::IllegalParamException("slot", __FILE__, __LINE__);
    }
    if (slot->GetStatus() == AbstractSlot::STATUS_CONNECTED) {
        throw vislib::IllegalStateException("slot", __FILE__, __LINE__);
    }
    if (slot->GetStatus() == AbstractSlot::STATUS_UNAVAILABLE) return;

    if (!this->FindSlot(slot->Name()))  {
        throw vislib::IllegalParamException("A slot with this name is not registered", __FILE__, __LINE__);
    }

    this->removeChild(::std::shared_ptr<AbstractNamedObject>(slot, [](AbstractNamedObject* d){}));
    slot->SetOwner(nullptr);
    slot->MakeUnavailable();

}



/*
 * Module::setModuleName
 */
void Module::setModuleName(const vislib::StringA& name) {
    this->setName(name);
}
