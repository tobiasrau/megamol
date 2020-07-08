/*
 * AbstractOSPRayRenderer.h
 * Copyright (C) 2009-2015 by MegaMol Team
 * Alle Rechte vorbehalten.
 */
#pragma once

#include <map>
#include <stdint.h>
#include "OSPRay_plugin/CallOSPRayStructure.h"
#include "mmcore/CallerSlot.h"
#include "mmcore/param/ParamSlot.h"
#include "mmcore/view/CallRender3D_2.h"
#include "mmcore/view/Renderer3DModule_2.h"
#include "mmcore/view/light/CallLight.h"
#include "ospray/ospray.h"

namespace megamol {
namespace ospray {

class AbstractOSPRayRenderer : public core::view::Renderer3DModule_2 {
protected:
    // Ctor
    AbstractOSPRayRenderer(void);

    // Dtor
    ~AbstractOSPRayRenderer(void);

    /**
     * initializes OSPRay
     */
    void initOSPRay(OSPDevice& dvce);

    /**
     * helper function for initializing OSPRay
     * @param OSPRay renderer object
     * @param OSPRay camera object
     * @param OSPRay world object
     * @param volume name/type
     * @param renderer type
     */
    void setupOSPRay(OSPRenderer& renderer, OSPCamera& camera, OSPModel& world, const char* renderer_name);

    /**
     * helper function for initializing OSPRays Camera
     * @param OSPRay camera object
     * @param CallRenderer3D object
     */
    void setupOSPRayCamera(OSPCamera& ospcam, megamol::core::view::Camera_2& mmcam);


    /**
     * Texture from file importer
     * @param file path
     * @return 2
     */
    OSPTexture2D TextureFromFile(vislib::TString fileName);
    // helper function to write the rendered image as PPM file
    void writePPM(const char* fileName, const osp::vec2i& size, const uint32_t* pixel);

    // TODO: Documentation

    bool AbstractIsDirty();
    void AbstractResetDirty();
    void RendererSettings(OSPRenderer& renderer);
    OSPFrameBuffer newFrameBuffer(osp::vec2i& imgSize, const OSPFrameBufferFormat format = OSP_FB_RGBA8,
        const uint32_t frameBufferChannels = OSP_FB_COLOR);



    /**
     * Reads the structure map and uses its parameteres to
     * create geometries and volumes.
     *
     */
    bool fillWorld();

    void changeMaterial();

    void changeTransformation();

    /**
     * Releases the created geometries and volumes.
     *
     */
    void releaseOSPRayStuff();

    // Interface Variables
    core::param::ParamSlot AOtransparencyEnabled;
    core::param::ParamSlot AOsamples;
    core::param::ParamSlot AOdistance;
    core::param::ParamSlot accumulateSlot;

    core::param::ParamSlot rd_epsilon;
    core::param::ParamSlot rd_spp;
    core::param::ParamSlot rd_maxRecursion;
    core::param::ParamSlot rd_type;
    core::param::ParamSlot rd_ptBackground;
    core::param::ParamSlot shadows;
    core::param::ParamSlot useDB;
    core::param::ParamSlot numThreads;

    // Fix for deprecated material (ospNewMaterial2 now)
    std::string rd_type_string;

    megamol::core::param::ParamSlot deviceTypeSlot;

    // device type
    enum deviceType { DEFAULT, MPI_DISTRIBUTED };

    // renderer type
    enum rdenum { SCIVIS, PATHTRACER, MPI_RAYCAST };

    // light
    std::vector<OSPLight> lightArray;
    OSPData lightsToRender;

    // framebuffer dirtyness
    bool framebufferIsDirty;

    // OSP objects
    OSPFrameBuffer framebuffer;
    OSPCamera camera;
    OSPModel world;
    // device
    OSPDevice device;
    // renderer
    OSPRenderer renderer;
    OSPTexture2D maxDepthTexture;

    // structure vectors
    std::map<CallOSPRayStructure*, std::vector<std::variant<OSPGeometry,OSPVolume>>> baseStructures;
    std::map<CallOSPRayStructure*, OSPModel> instancedModels;
    std::map<CallOSPRayStructure*, OSPGeometry> instances;
    std::map<CallOSPRayStructure*, OSPMaterial> materials;


    // Structure map
    OSPRayStrcutrureMap structureMap;
    // extend map
    OSPRayExtendMap extendMap;

    void fillLightArray(glm::vec4& eyeDir);

    long long int ispcLimit = 1ULL << 30;
    long long int numCreateGeo;
};

} // end namespace ospray
} // end namespace megamol
