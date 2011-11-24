/*
 * TriSoupRenderer.cpp
 *
 * Copyright (C) 2010 by Sebastian Grottel
 * Copyright (C) 2008-2010 by VISUS (Universitaet Stuttgart)
 * Alle Rechte vorbehalten.
 */

#include "stdafx.h"
#include "TriSoupRenderer.h"
#include "CallTriMeshData.h"
#include "CallVolumetricData.h"
#include "param/BoolParam.h"
#include "param/EnumParam.h"
#include "view/CallRender3D.h"
#include <GL/gl.h>


#include "param/ButtonParam.h"
#include "param/StringParam.h"
#include "param/FilePathParam.h"
#include "vislib/KeyCode.h"
#include "vislib/Log.h"
#include "vislib/mathfunctions.h"
#include "vislib/MemmappedFile.h"
#include "vislib/ShallowPoint.h"
#include "vislib/Vector.h"

using namespace megamol;
using namespace megamol::trisoup;



using namespace megamol::core;


/*
 * TriSoupRenderer::TriSoupRenderer
 */
TriSoupRenderer::TriSoupRenderer(void) : Renderer3DModule(),
        getDataSlot("getData", "The slot to fetch the tri-mesh data"),
        getVolDataSlot("getVolData", "The slot to fetch the volume data (experimental)"),
        showVertices("showVertices", "Flag whether to show the verices of the object"),
        lighting("lighting", "Flag whether or not use lighting for the surface"),
        surFrontStyle("frontstyle", "The rendering style for the front surface"),
        surBackStyle("backstyle", "The rendering style for the back surface"),
        windRule("windingrule", "The triangle edge winding rule") {

    this->getDataSlot.SetCompatibleCall<CallTriMeshDataDescription>();
    this->MakeSlotAvailable(&this->getDataSlot);

    this->getVolDataSlot.SetCompatibleCall<core::CallAutoDescription<CallVolumetricData> >();
    this->MakeSlotAvailable(&this->getVolDataSlot);

    this->showVertices.SetParameter(new param::BoolParam(false));
    this->MakeSlotAvailable(&this->showVertices);

    this->lighting.SetParameter(new param::BoolParam(true));
    this->MakeSlotAvailable(&this->lighting);

    param::EnumParam *ep = new param::EnumParam(0);
    ep->SetTypePair(0, "Filled");
    ep->SetTypePair(1, "Wireframe");
    ep->SetTypePair(2, "Points");
    ep->SetTypePair(3, "None");
    this->surFrontStyle << ep;
    this->MakeSlotAvailable(&this->surFrontStyle);

    ep = new param::EnumParam(3);
    ep->SetTypePair(0, "Filled");
    ep->SetTypePair(1, "Wireframe");
    ep->SetTypePair(2, "Points");
    ep->SetTypePair(3, "None");
    this->surBackStyle << ep;
    this->MakeSlotAvailable(&this->surBackStyle);

    ep = new param::EnumParam(0);
    ep->SetTypePair(0, "Counter-Clock Wise");
    ep->SetTypePair(1, "Clock Wise");
    this->windRule << ep;
    this->MakeSlotAvailable(&this->windRule);
}


/*
 * TriSoupRenderer::~TriSoupRenderer
 */
TriSoupRenderer::~TriSoupRenderer(void) {
    this->Release();
}


/*
 * TriSoupRenderer::create
 */
bool TriSoupRenderer::create(void) {
    // intentionally empty
    return true;
}


/*
 * TriSoupRenderer::GetCapabilities
 */
bool TriSoupRenderer::GetCapabilities(Call& call) {
    view::CallRender3D *cr = dynamic_cast<view::CallRender3D*>(&call);
    if (cr == NULL) return false;

    cr->SetCapabilities(view::CallRender3D::CAP_RENDER | view::CallRender3D::CAP_LIGHTING);

    return true;
}


/*
 * TriSoupRenderer::GetExtents
 */
bool TriSoupRenderer::GetExtents(Call& call) {
    view::CallRender3D *cr = dynamic_cast<view::CallRender3D*>(&call);
    if (cr == NULL) return false;
    CallTriMeshData *ctmd = this->getDataSlot.CallAs<CallTriMeshData>();
    if (ctmd == NULL) return false;
    ctmd->SetFrameID(static_cast<int>(cr->Time()));
    if (!(*ctmd)(1)) return false;

    cr->SetTimeFramesCount(1);
    cr->AccessBoundingBoxes().Clear();
    cr->AccessBoundingBoxes() = ctmd->AccessBoundingBoxes();
    float scale = ctmd->AccessBoundingBoxes().ClipBox().LongestEdge();
    if (scale > 0.0f) scale = 2.0f / scale;
    cr->AccessBoundingBoxes().MakeScaledWorld(scale);

    return true;
}


/*
 * TriSoupRenderer::release
 */
void TriSoupRenderer::release(void) {
    // intentionally empty
}


/*
 * TriSoupRenderer::Render
 */
bool TriSoupRenderer::Render(Call& call) {
    view::CallRender3D *cr = dynamic_cast<view::CallRender3D*>(&call);
    if (cr == NULL) return false;
    CallTriMeshData *ctmd = this->getDataSlot.CallAs<CallTriMeshData>();
    if (ctmd == NULL) return false;

    ctmd->SetFrameID(static_cast<int>(cr->Time()));
    if (!(*ctmd)(1)) return false;
    float scale = ctmd->AccessBoundingBoxes().ClipBox().LongestEdge();
    if (scale > 0.0f) scale = 2.0f / scale;
    ::glScalef(scale, scale, scale);

    ctmd->SetFrameID(static_cast<int>(cr->Time()));
    if (!(*ctmd)(0)) return false;

    bool normals = false;
    bool colors = false;
    bool textures = false;

    ::glEnable(GL_DEPTH_TEST);
    if (this->lighting.Param<param::BoolParam>()->Value()) {
        ::glEnable(GL_LIGHTING);
    } else {
        ::glDisable(GL_LIGHTING);
    }
    ::glDisable(GL_BLEND);
    ::glEnableClientState(GL_VERTEX_ARRAY);
    ::glDisableClientState(GL_NORMAL_ARRAY);
    ::glDisableClientState(GL_COLOR_ARRAY);
    ::glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    ::glEnable(GL_COLOR_MATERIAL);
    ::glEnable(GL_TEXTURE_2D);
    ::glBindTexture(GL_TEXTURE_2D, 0);
    ::glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    ::glEnable(GL_NORMALIZE);

    GLint cfm;
    ::glGetIntegerv(GL_CULL_FACE_MODE, &cfm);
    GLint pm[2];
    ::glGetIntegerv(GL_POLYGON_MODE, pm);
    GLint twr;
    ::glGetIntegerv(GL_FRONT_FACE, &twr);

    if (this->windRule.Param<param::EnumParam>()->Value() == 0) {
        ::glFrontFace(GL_CCW);
    } else {
        ::glFrontFace(GL_CW);
    }

    int fpm, bpm, cf = 0;

    switch (this->surFrontStyle.Param<param::EnumParam>()->Value()) {
        default: fpm = GL_FILL; break;
        case 1: fpm = GL_LINE; break;
        case 2: fpm = GL_POINT; break;
        case 3: fpm = GL_FILL; cf = GL_FRONT; break;
    }
    switch (this->surBackStyle.Param<param::EnumParam>()->Value()) {
        default: bpm = GL_FILL; break;
        case 1: bpm = GL_LINE; break;
        case 2: bpm = GL_POINT; break;
        case 3: bpm = GL_FILL; cf = (cf == 0) ? GL_BACK : GL_FRONT_AND_BACK; break;
    }
    ::glPolygonMode(GL_FRONT, fpm);
    ::glPolygonMode(GL_BACK, bpm);
    if (cf == 0) {
        ::glDisable(GL_CULL_FACE);
    } else {
        ::glEnable(GL_CULL_FACE);
        ::glCullFace(cf);
    }

    ::glColor3f(1.0f, 1.0f, 1.0f);
    for (unsigned int i = 0; i < ctmd->Count(); i++) {
        const CallTriMeshData::Mesh& obj = ctmd->Objects()[i];

        switch (obj.GetVertexDataType()) {
            case CallTriMeshData::Mesh::DT_FLOAT:
                ::glVertexPointer(3, GL_FLOAT, 0, obj.GetVertexPointerFloat());
                break;
            case CallTriMeshData::Mesh::DT_DOUBLE:
                ::glVertexPointer(3, GL_DOUBLE, 0, obj.GetVertexPointerDouble());
                break;
            default: continue;
        }

        if (obj.HasNormalPointer() != NULL) {
            if (!normals) {
                ::glEnableClientState(GL_NORMAL_ARRAY);
                normals = true;
            }
            switch (obj.GetNormalDataType()) {
                case CallTriMeshData::Mesh::DT_FLOAT:
                    ::glNormalPointer(GL_FLOAT, 0, obj.GetNormalPointerFloat());
                    break;
                case CallTriMeshData::Mesh::DT_DOUBLE:
                    ::glNormalPointer(GL_DOUBLE, 0, obj.GetNormalPointerDouble());
                    break;
                default: continue;
            }
        } else if (normals) {
            ::glDisableClientState(GL_NORMAL_ARRAY);
            normals = false;
        }

        if (obj.HasColourPointer() != NULL) {
            if (!colors) {
                ::glEnableClientState(GL_COLOR_ARRAY);
                colors = true;
            }
            switch (obj.GetColourDataType()) {
                case CallTriMeshData::Mesh::DT_BYTE:
                    ::glColorPointer(3, GL_UNSIGNED_BYTE, 0, obj.GetColourPointerByte());
                    break;
                case CallTriMeshData::Mesh::DT_FLOAT:
                    ::glColorPointer(3, GL_FLOAT, 0, obj.GetColourPointerFloat());
                    break;
                case CallTriMeshData::Mesh::DT_DOUBLE:
                    ::glColorPointer(3, GL_DOUBLE, 0, obj.GetColourPointerDouble());
                    break;
                default: continue;
            }
        } else if (colors) {
            ::glDisableClientState(GL_COLOR_ARRAY);
            colors = false;
        }

        if (obj.HasTextureCoordinatePointer() != NULL) {
            if (!textures) {
                ::glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                textures = true;
            }
            switch (obj.GetTextureCoordinateDataType()) {
                case CallTriMeshData::Mesh::DT_FLOAT:
                    ::glTexCoordPointer(2, GL_FLOAT, 0, obj.GetTextureCoordinatePointerFloat());
                    break;
                case CallTriMeshData::Mesh::DT_DOUBLE:
                    ::glTexCoordPointer(2, GL_DOUBLE, 0, obj.GetTextureCoordinatePointerDouble());
                    break;
                default: continue;
            }
        } else if (textures) {
            ::glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            textures = false;
        }

        if (obj.GetMaterial() != NULL) {
            const CallTriMeshData::Material &mat = *obj.GetMaterial();

            ::glDisable(GL_COLOR_MATERIAL);
            GLfloat mat_ambient[4] = { mat.GetKa()[0], mat.GetKa()[1], mat.GetKa()[2], 1.0f };
            GLfloat mat_diffuse[4] = { mat.GetKd()[0], mat.GetKd()[1], mat.GetKd()[2], 1.0f };
            GLfloat mat_specular[4] = { mat.GetKs()[0], mat.GetKs()[1], mat.GetKs()[2], 1.0f };
            GLfloat mat_emission[4] = { mat.GetKe()[0], mat.GetKe()[1], mat.GetKe()[2], 1.0f };
            GLfloat mat_shininess[1] = { mat.GetNs() };
            ::glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
            ::glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
            ::glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
            ::glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, mat_emission);
            ::glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);

            GLuint mapid = mat.GetMapID();
            if (mapid > 0) {
                //::glActiveTexture(GL_TEXTURE0);
                ::glEnable(GL_COLOR_MATERIAL);
                ::glBindTexture(GL_TEXTURE_2D, mapid);
                ::glEnable(GL_TEXTURE_2D);
                ::glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                ::glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            }
        } else {
            GLfloat mat_ambient[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
            GLfloat mat_diffuse[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
            GLfloat mat_specular[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
            GLfloat mat_emission[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
            GLfloat mat_shininess[1] = { 0.0f };
            ::glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
            ::glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
            ::glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
            ::glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, mat_emission);
            ::glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
            ::glBindTexture(GL_TEXTURE_2D, 0);
            ::glEnable(GL_COLOR_MATERIAL);

        }

        if (obj.HasTriIndexPointer() != NULL) {
            switch (obj.GetTriDataType()) {
                case CallTriMeshData::Mesh::DT_BYTE:
                    ::glDrawElements(GL_TRIANGLES, obj.GetTriCount() * 3, GL_UNSIGNED_BYTE, obj.GetTriIndexPointerByte());
                    break;
                case CallTriMeshData::Mesh::DT_UINT16:
                    ::glDrawElements(GL_TRIANGLES, obj.GetTriCount() * 3, GL_UNSIGNED_SHORT, obj.GetTriIndexPointerUInt16());
                    break;
                case CallTriMeshData::Mesh::DT_UINT32:
                    ::glDrawElements(GL_TRIANGLES, obj.GetTriCount() * 3, GL_UNSIGNED_INT, obj.GetTriIndexPointerUInt32());
                    break;
                default: continue;
            }
        } else {
            ::glDrawArrays(GL_TRIANGLES, 0, obj.GetVertexCount());
        }
    }

    if (normals) ::glDisableClientState(GL_NORMAL_ARRAY);
    if (colors) ::glDisableClientState(GL_COLOR_ARRAY);
    if (textures) ::glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    {
        GLfloat mat_ambient[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
        GLfloat mat_diffuse[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
        GLfloat mat_specular[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        GLfloat mat_emission[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        GLfloat mat_shininess[1] = { 0.0f };
        ::glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
        ::glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
        ::glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
        ::glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, mat_emission);
        ::glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
        ::glBindTexture(GL_TEXTURE_2D, 0);
    }

    ::glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    if (this->showVertices.Param<param::BoolParam>()->Value()) {
        ::glEnable(GL_POINT_SIZE);
        ::glPointSize(3.0f);
        ::glDisable(GL_LIGHTING);

        ::glColor3f(1.0f, 0.0f, 0.0f);
        for (unsigned int i = 0; i < ctmd->Count(); i++) {
            switch (ctmd->Objects()[i].GetVertexDataType()) {
                case CallTriMeshData::Mesh::DT_FLOAT:
                    ::glVertexPointer(3, GL_FLOAT, 0, ctmd->Objects()[i].GetVertexPointerFloat());
                    break;
                case CallTriMeshData::Mesh::DT_DOUBLE:
                    ::glVertexPointer(3, GL_DOUBLE, 0, ctmd->Objects()[i].GetVertexPointerDouble());
                    break;
                default: continue;
            }
            ::glDrawArrays(GL_POINTS, 0, ctmd->Objects()[i].GetVertexCount());
        }

        ::glEnable(GL_POINT_SIZE);
        ::glPointSize(1.0f);
    }

    ::glCullFace(cfm);
    ::glFrontFace(twr);
    ::glPolygonMode(GL_FRONT, pm[0]);
    ::glPolygonMode(GL_BACK, pm[1]);

    ::glEnable(GL_CULL_FACE);
    ::glDisableClientState(GL_VERTEX_ARRAY);
    ::glDisable(GL_POINT_SIZE);
    ::glEnable(GL_BLEND);


    CallVolumetricData *cvd = this->getVolDataSlot.CallAs<CallVolumetricData>();
    if (cvd != NULL && (*cvd)(0)) {
        vislib::Array<CallVolumetricData::Volume>& volumes = cvd->GetVolumes();
        ::glEnable(GL_POINT_SIZE);
        ::glDisable(GL_BLEND);
        ::glDisable(GL_LIGHTING);
        ::glPointSize(2);
        ::glBegin(GL_POINTS);
        for(int i = 0; i < volumes.Count(); i++) {
            CallVolumetricData::Volume& v = volumes[i];
            if (!v.volumeData)
                continue;
            for(int x = 0; x < v.size[0]; x++) {
                for(int y = 0; y < v.size[1]; y++) {
                    //int yOffs = y*v.size[0];
                    for(int z = 0; z < v.size[2]; z++) {
                        int index = x + v.size[0]*(y + z*v.size[1]);
                        double position[3] = {v.origin[0] + (x+0.5)*v.scaling[0], v.origin[1] + (y+0.5)*v.scaling[1], v.origin[2] + (z+0.5)*v.scaling[2]};
                        CallVolumetricData::VoxelType voxel = v.volumeData[index];
                        if (voxel != 0) {
                            if (voxel > 0)
                                ::glColor4f(0, 1, 1, 1);
                            else
                                ::glColor4d(1, 0, 0, 1);
                        } else
                            ::glColor4f(1, 1, 1, 1);
                       ::glVertex3dv(position);
                     }
                }
            }
            break;
        }
        ::glEnd();
    }

    return true;
}
