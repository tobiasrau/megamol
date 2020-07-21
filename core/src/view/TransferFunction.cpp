/*
 * TransferFunction.cpp
 *
 * Copyright (C) 2008 by Universitaet Stuttgart (VIS).
 * Alle Rechte vorbehalten.
 */

#include "stdafx.h"
#include "mmcore/view/TransferFunction.h"

#include "mmcore/param/TransferFunctionParam.h"

#ifdef HAS_OPENGL
#include "mmcore/view/TransferFunctionUtility_gl.h"
#else
#include "mmcore/view/TransferFunctionUtility.h"
#endif


namespace megamol::core::view {


/*
 * TransferFunction::TransferFunction
 */
TransferFunction::TransferFunction(void)
    : Module()
    , getTFSlot("gettransferfunction", "Provides the transfer function")
    , tfParam("TransferFunction", "The transfer function serialized as JSON string.")
    , texID(0)
    , texSize(1)
    , tex()
    , texFormat(CallGetTransferFunction::TEXTURE_FORMAT_RGBA)
    , interpolMode(param::TransferFunctionParam::InterpolationMode::LINEAR)
    , range({0.0f, 1.0f})
    , tfparam_check_init_value(true)
    , tfparam_skip_changes_once(false)
    , version(0)
    , last_frame_id(0) {

    CallGetTransferFunctionDescription cgtfd;
    this->getTFSlot.SetCallback(cgtfd.ClassName(), cgtfd.FunctionName(0), &TransferFunction::requestTF);
    this->MakeSlotAvailable(&this->getTFSlot);

    this->tfParam << new param::TransferFunctionParam("");
    this->MakeSlotAvailable(&this->tfParam);
}


/*
 * TransferFunction::~TransferFunction
 */
TransferFunction::~TransferFunction(void) { this->Release(); }


/*
 * TransferFunction::create
 */
bool TransferFunction::create(void) {

    return true;
}


/*
 * TransferFunction::release
 */
void TransferFunction::release(void) {

}


/*
 * TransferFunction::requestTF
 */
bool TransferFunction::requestTF(Call& call) {

    CallGetTransferFunction* cgtf = dynamic_cast<CallGetTransferFunction*>(&call);
    if (cgtf == nullptr) return false;

    // Skip changes propagated by call once to apply initial value of transfer function parameter set from project file.
    this->tfparam_skip_changes_once = false;
    if (this->tfparam_check_init_value) {
        if (!this->tfParam.Param<param::TransferFunctionParam>()->Value().empty()) {
            this->tfparam_skip_changes_once = true;
        }
        this->tfparam_check_init_value = false;
    }
    // Update changed data set range for transfer function parameter
    if (!this->tfparam_skip_changes_once) {
        if (cgtf->ConsumeRangeUpdate()) {
            // Get current values from parameter string 
            auto tmp_range = this->range;
            auto tmp_interpol = this->interpolMode;
            auto tmp_tex_size = this->texSize;
            param::TransferFunctionParam::TFNodeType tmp_nodes;
            if (megamol::core::param::TransferFunctionParam::ParseTransferFunction(this->tfParam.Param<param::TransferFunctionParam>()->Value(), tmp_nodes, tmp_interpol, tmp_tex_size, tmp_range)) {
                std::string tf_str;
                // Set transfer function parameter value using updated range
                if (megamol::core::param::TransferFunctionParam::DumpTransferFunction(tf_str, tmp_nodes, tmp_interpol, tmp_tex_size, cgtf->Range())) {
                    this->tfParam.Param<param::TransferFunctionParam>()->SetValue(tf_str);
                }
            }
        }
    }

    if ((this->texID == 0) || this->tfParam.IsDirty()) {
        this->tfParam.ResetDirty();

        // Get current values from parameter string (Values are checked, too).
        param::TransferFunctionParam::TFNodeType tfnodes;
        if (!megamol::core::param::TransferFunctionParam::ParseTransferFunction(
            this->tfParam.Param<param::TransferFunctionParam>()->Value(), tfnodes, this->interpolMode, this->texSize, this->range)) {
            return false;
        }

        // Apply interpolation and generate texture data.
        if (this->interpolMode == param::TransferFunctionParam::InterpolationMode::LINEAR) {
            param::TransferFunctionParam::LinearInterpolation(this->tex, this->texSize, tfnodes);
        }
        else if (this->interpolMode == param::TransferFunctionParam::InterpolationMode::GAUSS) {
            param::TransferFunctionParam::GaussInterpolation(this->tex, this->texSize, tfnodes);
        }

        // use utility for gl or CPU calls
        auto tfu = TransferFunctionUtility(texFormat, tex, texID, texSize);
        tfu.requestTF();

        ++this->version;
    }

    cgtf->SetTexture(this->texID, this->texSize, this->tex.data(), this->texFormat,
        this->range, this->version);

    return true;
}
} // namespace megamol::core::view