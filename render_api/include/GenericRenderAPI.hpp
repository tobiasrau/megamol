/*
 * GenericRenderAPI.hpp
 *
 * Copyright (C) 2019 by MegaMol Team
 * Alle Rechte vorbehalten.
 */

#pragma once

#include "AbstractRenderAPI.hpp"
#include "AbstractUILayer.h"

#include <string>
#include <tuple>
#include <vector>
#include <functional>

namespace megamol {
namespace render_api {

struct WindowPlacement {
    int x = 100, y = 100, w = 800, h = 600, mon = 0;
    bool pos = false;
    bool size = false;
};

class GenericRenderAPI : public AbstractRenderAPI {
public:
    // make capabilities of OpenGL/GLFW RAPI statically query-able (we would like to force this from the abstract
    // class, but this is not possible?)
    std::string getAPIName() const override { return std::string{"Generic"}; };
    RenderAPIVersion getAPIVersion() const override { return RenderAPIVersion{0, 0}; };

    // how to force RAPI subclasses to implement a Config struct which should be passed to constructor??
    // set sane defaults for all options here, so usage is as simple as possible
    struct Config {
        void* sharedContextPtr = nullptr;
        std::string viewInstanceName = "";
        WindowPlacement windowPlacement{}; // window position, glfw creation hints // TODO: sane defaults??
    };

    GenericRenderAPI() = default;
    ~GenericRenderAPI() override;
    // TODO: delete copy/move/assign?

    // init API, e.g. init GLFW with OpenGL and open window with certain decorations/hints
    bool initAPI(const Config& config);
    bool initAPI(void* configPtr) override;
    void closeAPI() override;

    void preViewRender() override;  // prepare rendering with API, e.g. set OpenGL context, frame-timers, etc
    void postViewRender() override; // clean up after rendering, e.g. stop and show frame-timers in GLFW window

    const void* getAPISharedDataPtr() const override; // ptr non-owning, share data should be only borrowed

    // from AbstractRenderAPI:
    // int setPriority(const int p) // priority initially 0
    // int getPriority() const;
    // bool shouldShutdown() const; // shutdown initially false
    // void setShutdown(const bool s = true);

private:
    struct PimplData;
    std::unique_ptr<PimplData, std::function<void(PimplData*)>>
        m_pimpl; // abstract away GLFW library details behind pointer-to-implementation
};

} // namespace render_api
} // namespace megamol
