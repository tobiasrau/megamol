#include "vislib/graphics/GLSLShader.h"
#include "vislib/graphics/GLSLShader.h"




namespace megamol {
namespace ospray {


class UploadTexture {
    /** The texture shader */
    vislib::graphics::gl::GLSLShader osprayShader;

        /**
     * helper function for rendering the OSPRay texture
     * @param GLSL shader
     * @param GL texture object
     * @param OSPRay color texture
     * @param OSPRay depth texture
     * @param GL vertex array object
     * @param image/window width
     * @param image/window heigth
     */
    void renderTexture2D(vislib::graphics::gl::GLSLShader& shader, const uint32_t* fb, const float* db, int& width,
        int& height, core::view::CallRender3D_2& cr);


            // vertex array, vertex buffer object, texture
    GLuint vaScreen, vbo, tex, depth;
    vislib::graphics::gl::FramebufferObject new_fbo;


    void AbstractOSPRayRenderer::renderTexture2D(vislib::graphics::gl::GLSLShader& shader, const uint32_t* fb,
    const float* db, int& width, int& height, megamol::core::view::CallRender3D_2& cr) {

    auto fbo = cr.FrameBufferObject();
    //if (fbo != NULL) {

    //    if (fbo->IsValid()) {
    //        if ((fbo->GetWidth() != width) || (fbo->GetHeight() != height)) {
    //            fbo->Release();
    //        }
    //    }
    //    if (!fbo->IsValid()) {
    //        fbo->Create(width, height, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE,
    //            vislib::graphics::gl::FramebufferObject::ATTACHMENT_TEXTURE, GL_DEPTH_COMPONENT);
    //    }
    //    if (fbo->IsValid() && !fbo->IsEnabled()) {
    //        fbo->Enable();
    //    }

    //    fbo->BindColourTexture();
    //    glClear(GL_COLOR_BUFFER_BIT);
    //    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, fb);
    //    glBindTexture(GL_TEXTURE_2D, 0);

    //    fbo->BindDepthTexture();
    //    glClear(GL_DEPTH_BUFFER_BIT);
    //    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, db);
    //    glBindTexture(GL_TEXTURE_2D, 0);

    //    if (fbo->IsValid()) {
    //        fbo->Disable();
    //        // fbo->DrawColourTexture();
    //        // fbo->DrawDepthTexture();
    //    }
    //} else {
        /*
        if (this->new_fbo.IsValid()) {
            if ((this->new_fbo.GetWidth() != width) || (this->new_fbo.GetHeight() != height)) {
                this->new_fbo.Release();
            }
        }
        if (!this->new_fbo.IsValid()) {
            this->new_fbo.Create(width, height, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE,
        vislib::graphics::gl::FramebufferObject::ATTACHMENT_TEXTURE, GL_DEPTH_COMPONENT);
        }
        if (this->new_fbo.IsValid() && !this->new_fbo.IsEnabled()) {
            this->new_fbo.Enable();
        }

        this->new_fbo.BindColourTexture();
        glClear(GL_COLOR_BUFFER_BIT);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, fb);
        glBindTexture(GL_TEXTURE_2D, 0);

        this->new_fbo.BindDepthTexture();
        glClear(GL_DEPTH_BUFFER_BIT);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, db);
        glBindTexture(GL_TEXTURE_2D, 0);


        glBlitNamedFramebuffer(this->new_fbo.GetID(), 0, 0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT |
        GL_DEPTH_BUFFER_BIT, GL_NEAREST);

        this->new_fbo.Disable();
        */
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, this->tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, fb);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, this->depth);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, db);
        glBindTexture(GL_TEXTURE_2D, 0);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, this->tex);
        glUniform1i(shader.ParameterLocation("tex"), 0);


        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, this->depth);
        glUniform1i(shader.ParameterLocation("depth"), 1);


        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindTexture(GL_TEXTURE_2D, 0);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
  //  }
}



    /**
     * helper function for setting up the OSPRay screen
     * @param GL vertex array
     * @param GL vertex buffer object
     * @param GL texture object
     */
    void setupTextureScreen();

    /**
     * Releases the OGL content created by setupTextureScreen
     */
    void releaseTextureScreen();

void AbstractOSPRayRenderer::setupTextureScreen() {
    // setup color texture
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &this->tex);
    glBindTexture(GL_TEXTURE_2D, this->tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    //// setup depth texture
    glGenTextures(1, &this->depth);
    glBindTexture(GL_TEXTURE_2D, this->depth);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}

void AbstractOSPRayRenderer::releaseTextureScreen() {
    glDeleteTextures(1, &this->tex);
    glDeleteTextures(1, &this->depth);
}

bool OSPRayRenderer::create() {
    ASSERT(IsAvailable());

    vislib::graphics::gl::ShaderSource vert, frag;

    if (!instance()->ShaderSourceFactory().MakeShaderSource("ospray::vertex", vert)) {
        return false;
    }
    if (!instance()->ShaderSourceFactory().MakeShaderSource("ospray::fragment", frag)) {
        return false;
    }

    try {
        if (!this->osprayShader.Create(vert.Code(), vert.Count(), frag.Code(), frag.Count())) {
            vislib::sys::Log::DefaultLog.WriteMsg(
                vislib::sys::Log::LEVEL_ERROR, "Unable to compile ospray shader: Unknown error\n");
            return false;
        }
    } catch (vislib::graphics::gl::AbstractOpenGLShader::CompileException ce) {
        vislib::sys::Log::DefaultLog.WriteMsg(vislib::sys::Log::LEVEL_ERROR,
            "Unable to compile ospray shader: (@%s): %s\n",
            vislib::graphics::gl::AbstractOpenGLShader::CompileException::CompileActionName(ce.FailedAction()),
            ce.GetMsgA());
        return false;
    } catch (vislib::Exception e) {
        vislib::sys::Log::DefaultLog.WriteMsg(
            vislib::sys::Log::LEVEL_ERROR, "Unable to compile ospray shader: %s\n", e.GetMsgA());
        return false;
    } catch (...) {
        vislib::sys::Log::DefaultLog.WriteMsg(
            vislib::sys::Log::LEVEL_ERROR, "Unable to compile ospray shader: Unknown exception\n");
        return false;
    }

    // this->initOSPRay(device);
    this->setupTextureScreen();
    // this->setupOSPRay(renderer, camera, world, "scivis");

    return true;
}


/*
ospray::OSPRayRenderer::release
*/
void OSPRayRenderer::release() {
    releaseTextureScreen();
}

}

} // end namespace ospray
} // end namespace megamol