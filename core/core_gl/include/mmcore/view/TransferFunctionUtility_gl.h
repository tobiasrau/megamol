/*
 * TransferFunctionUtility.cpp
 *
 * Copyright (C) 2020 by Universitaet Stuttgart (VISUS).
 * Alle Rechte vorbehalten.
 */

#include "stdafx.h"

#include "mmcore/param/TransferFunctionParam.h"
#include "mmcore/view/CallGetTransferFunction.h"
#include "vislib/graphics/IncludeAllGL.h"


namespace megamol::core::view {


class TransferFunctionUtility {
public:
    TransferFunctionUtility(CallGetTransferFunction::TextureFormat texFormat, std::vector<float>& tex,
        unsigned int texID, unsigned int texSize) :
    _texID(0)
    , _texSize(1)
    , _tex(nullptr)
    , _texFormat(CallGetTransferFunction::TEXTURE_FORMAT_RGBA) {
        _texFormat = texFormat, _tex = std::make_shared<std::vector<float>>(tex);
        _texID = texID;
        _texSize = texSize;
    }

    ~TransferFunctionUtility(void) {
        glDeleteTextures(1, &this->_texID);
        this->_texID = 0;
    }


    bool requestTF() {

        if (this->_texID != 0) {
            glDeleteTextures(1, &this->_texID);
        }

        bool t1de = (glIsEnabled(GL_TEXTURE_1D) == GL_TRUE);
        if (!t1de) glEnable(GL_TEXTURE_1D);
        if (this->_texID == 0) glGenTextures(1, &this->_texID);

        GLint otid = 0;
        glGetIntegerv(GL_TEXTURE_BINDING_1D, &otid);
        glBindTexture(GL_TEXTURE_1D, (GLuint)this->_texID);

        glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, _texSize, 0, _texFormat, GL_FLOAT, _tex->data());

        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);

        glBindTexture(GL_TEXTURE_1D, otid);

        if (!t1de) glDisable(GL_TEXTURE_1D);

        return true;
    }

private:
    unsigned int _texID;
    CallGetTransferFunction::TextureFormat _texFormat;
    std::shared_ptr<std::vector<float>> _tex;
    unsigned int _texSize;
};
} // namespace megamol::core::view