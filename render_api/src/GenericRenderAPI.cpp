/*
 * GenericRenderAPI.cpp
 *
 * Copyright (C) 2019 by MegaMol Team
 * Alle Rechte vorbehalten.
 */

#include "GenericRenderAPI.hpp"

#include <array>
#include <chrono>
#include <functional>
#include <vector>

#include "UILayersCollection.hpp"
#include "vislib/graphics/FpsCounter.h"
#include "vislib/sys/Log.h"


namespace megamol {
namespace render_api {

namespace {
// the idea is that we only borrow, but _do not_ manipulate shared data somebody else gave us
struct SharedData {
    void* contextPtr{nullptr}; //
    // The SharedData idea is a bit broken here: on one hand, each window needs its own handle and GL context, on the
    // other hand a GL context can share its resources with others when its glfwWindow* handle gets passed to creation
    // of another Window/OpenGL context. So this handle is private, but can be used by other RAPI instances too.
};

void initSharedContext(SharedData& context) {
    context.contextPtr = nullptr; // stays null since nobody shared his GL context with us
}

} // namespace

// helpers to simplify data access
#define m_data (*m_pimpl)
#define m_sharedData ((m_data.sharedDataPtr) ? (*m_data.sharedDataPtr) : (m_data.sharedData))
#define m_contextPtr (m_data.sharedContextPtr)

struct GenericRenderAPI::PimplData {
    SharedData sharedData;              // personal data we share with other RAPIs
    SharedData* sharedDataPtr{nullptr}; // if we get shared data from another API object, we access it using this
                                        // ptr and leave our own shared data un-initialized

    void* contexPtr{nullptr};               // context dummy
    GenericRenderAPI::Config initialConfig; // keep copy of user-provided config

    std::string fullWindowTitle;

    int currentWidth = 0, currentHeight = 0;
    int currentPosX = 0, currentPosY = 0;

    UILayersCollection uiLayers;
    std::shared_ptr<AbstractUILayer> mouseCapture{nullptr};

    // TODO: move into 'FrameStatisticsCalculator'
    vislib::graphics::FpsCounter fpsCntr;
    float fps = 0.0f;
    std::array<float, 20> fpsList = {0.0f};
    bool showFpsInTitle = true;
    std::chrono::system_clock::time_point fpsSyncTime;
    bool showFragmentsInTitle;
    bool showPrimsInTitle;
};


GenericRenderAPI::~GenericRenderAPI() {
    if (m_pimpl) this->closeAPI(); // cleans up pimpl
}

bool GenericRenderAPI::initAPI(void* configPtr) {
    if (configPtr == nullptr) return false;

    return initAPI(*static_cast<Config*>(configPtr));
}

bool GenericRenderAPI::initAPI(const Config& config) {
    if (m_pimpl) return false; // this API object is already initialized

    // TODO: check config for sanity?
    // if(!sane(config))
    //	return false

    m_pimpl =
        std::unique_ptr<PimplData, std::function<void(PimplData*)>>(new PimplData, [](PimplData* ptr) { delete ptr; });
    m_data.initialConfig = config;
    // from here on, access pimpl data using "m_data.member", as in m_pimpl->member minus the  void-ptr casting

    // init (shared) context data for this object or use provided
    if (config.sharedContextPtr) {
        m_data.sharedDataPtr = reinterpret_cast<SharedData*>(config.sharedContextPtr);
    } else {
        initSharedContext(m_data.sharedData);
    }
    // from here on, use m_sharedData to access reference to SharedData for RAPI objects; the owner will clean it up
    // correctly
    if (m_data.sharedDataPtr) {
        // generic context already initialized by other render api
    } else {
        // TODO: implement initialization
    }

    // window size for windowed mode
    if (config.windowPlacement.size && (config.windowPlacement.w > 0) && (config.windowPlacement.h > 0)) {
        m_data.currentWidth = config.windowPlacement.w;
        m_data.currentHeight = config.windowPlacement.h;
    }

    vislib::sys::Log::DefaultLog.WriteInfo(
        "GenericRenderAPI: Rendering in a context with size w: %d, h: %d\n", m_data.currentWidth, m_data.currentHeight);

    if (config.windowPlacement.pos && (config.windowPlacement.x > 0) && (config.windowPlacement.y > 0)) {
        m_data.currentPosX = config.windowPlacement.x;
        m_data.currentPosY = config.windowPlacement.y;
    }

    // TODO implement context swapping

    return true;
}

void GenericRenderAPI::closeAPI() {
    if (!m_pimpl) // this API object is not initialized
        return;

    const bool close_glfw = (m_data.sharedDataPtr == nullptr);

    m_data.sharedDataPtr = nullptr;
    m_data.contexPtr = nullptr;
    this->m_pimpl.release();
}

void GenericRenderAPI::preViewRender() {

    // TODO: implement make context current

    // start frame timer

    // rendering via MegaMol View is called after this function finishes
    // in the end this calls something like
    //::mmcRenderView(hView, &renderContext);
}

void GenericRenderAPI::postViewRender() {

    // end frame timer
    // update window name

    m_data.uiLayers.OnDraw();

    // TODO: implement context swap
}

const void* GenericRenderAPI::getAPISharedDataPtr() const { return &m_sharedData; }


// TODO: how to force/allow subclasses to interop with other APIs?
// TODO: API-specific options in subclasses?

} // namespace render_api
} // namespace megamol
