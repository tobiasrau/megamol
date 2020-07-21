/*
 * TransferFunctionUtility.cpp
 *
 * Copyright (C) 2020 by Universitaet Stuttgart (VISUS).
 * Alle Rechte vorbehalten.
 */

#include "stdafx.h"

#include "mmcore/param/TransferFunctionParam.h"
#include "mmcore/view/CallGetTransferFunction.h"


namespace megamol::core::view {


class TransferFunctionUtility {
public:
    TransferFunctionUtility(CallGetTransferFunction::TextureFormat texFormat, std::vector<float>& tex,
        unsigned int texID, unsigned int texSize)
        : _texID(0), _texSize(1), _tex(nullptr), _texFormat(CallGetTransferFunction::TEXTURE_FORMAT_RGBA) {
        _texFormat = texFormat, _tex = std::make_shared<std::vector<float>>(tex);
        _texID = texID;
        _texSize = texSize;
    }

    ~TransferFunctionUtility(void) {
        this->_texID = 0;
    }


    bool requestTF() {

        _tex->resize(_texSize);

        return true;
    }

private:
    unsigned int _texID;
    CallGetTransferFunction::TextureFormat _texFormat;
    std::shared_ptr<std::vector<float>> _tex;
    unsigned int _texSize;
};
} // namespace megamol::core::view