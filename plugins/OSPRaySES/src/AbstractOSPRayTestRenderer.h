#pragma once

#ifndef _ABSTRACT_OSP_RAY_TEST_RENDERER_H_
#define _ABSTRACT_OSP_RAY_TEST_RENDERER_H_

#include <vector>

#include "vislib/graphics/gl/GLSLShader.h"
#include "vislib/graphics/CameraParameters.h"
#include "vislib/graphics/CameraParamsStore.h"
#include "vislib/math/Quaternion.h"

#include "mmcore/view/Renderer3DModule.h"
#include "mmcore/CallerSlot.h"

#include "Common.h"

namespace megamol {
	namespace ospray {
		namespace ses {


			class AbstractOSPRayTestRenderer : public core::view::Renderer3DModule {
			public:

				/**
				* Answers whether this module is available on the current system.
				*
				* @return 'true' if the module is available, 'false' otherwise.
				*/
				static bool IsAvailable(void) {
					return vislib::graphics::gl::GLSLShader::AreExtensionsAvailable();
				}

				AbstractOSPRayTestRenderer();

				virtual ~AbstractOSPRayTestRenderer();

			protected:

        enum ComputationMode {
          CPU = 0,
          VECTORIZED,
          PARTIALLY_VECTORIZED
        };

				/**
				* Implementation of 'Create'.
				*
				* @return 'true' on success, 'false' otherwise.
				*/
				virtual bool create(void);

				/**
				* Implementation of 'Release'.
				*/
				virtual void release(void);

				virtual bool InitOSP();

				virtual void DestroyOSP();

				virtual void UpdateCameraAndRenderer(const vislib::SmartPtr<vislib::graphics::CameraParameters> &camParams, bool clearBuffer = false);

				virtual void InitGL();

				virtual void DestroyGL();

        bool RenderPre();

				bool RenderPost();

        void ReadInterfaceRenderer();

        void ReadInterfaceSurface();

        void SetRenderParams(const vislib::SmartPtr<vislib::graphics::CameraParameters> &camParams);

        void SetSurfaceParams(OSPGeometry sesGeometry);

        inline bool IsSurfaceInterfaceDirty() {
          return this->surfaceParamValues.dirty;
        }

        inline bool IsRendererInterfaceDirty() {
          return this->rendererParamValues.dirty;
        }

        OSPModel GetScene();

        OSPRenderer GetRenderer();

        OSPGeometry CreateSurface();

        void AddGeometry(OSPGeometry geometry);

        void FinalizeGeometry();

        int GetGeometryCnt();

        void ClearGeometry();

        // OSP to MM scaling
        float scale = 1.0f;

      private:

        /** OSP members */
        OSPDevice device;
        OSPCamera camera;
        std::vector<OSPGeometry> geometries;
        OSPModel scene;

        OSPRenderer renderer;
        OSPFrameBuffer framebuffer;

        vec2i image;

        /** GL stuff */
        vislib::graphics::gl::GLSLShader osprayShader;

        /** old camera parameters */
        vislib::graphics::CameraParamsStore previousCamParams;
        bool previousAccumulateFlag;

        GLuint vaRect;
        GLuint vbo;
        GLuint ospFbTex;

        // renderer params
        megamol::core::param::ParamSlot epsilonSlot;
        megamol::core::param::ParamSlot useHeadlightSlot;
        megamol::core::param::ParamSlot lightDirSlot;
        megamol::core::param::ParamSlot lightDiffuseWeightSlot;
        megamol::core::param::ParamSlot lightAmbientWeightSlot;
        megamol::core::param::ParamSlot lightExponentSlot;

        megamol::core::param::ParamSlot alphaSlot;
        megamol::core::param::ParamSlot useCuttingPlaneSlot;
        megamol::core::param::ParamSlot cuttingPlaneSlot;
        megamol::core::param::ParamSlot accumulateSlot;

        // AOOM params
        megamol::core::param::ParamSlot useAOOMSlot;
        megamol::core::param::ParamSlot smallCavitiesSlot;
        megamol::core::param::ParamSlot rhoSlot;
        megamol::core::param::ParamSlot tauSlot;
        megamol::core::param::ParamSlot aoDistanceSlot;
        megamol::core::param::ParamSlot aoSamplesSlot;
        megamol::core::param::ParamSlot blendingColorSlot;
        megamol::core::param::ParamSlot blendingWeightSlot;

        // surface params
        megamol::core::param::ParamSlot modeSlot;
        megamol::core::param::ParamSlot probeRadiusSlot;
        megamol::core::param::ParamSlot skipContourSingularitiesSlot;
        megamol::core::param::ParamSlot computeSingularitiesSlot;
        megamol::core::param::ParamSlot resizeVectorsSlot;
        megamol::core::param::ParamSlot debugFlagSlot;
        megamol::core::param::ParamSlot useIterativeMethodSlot;
        megamol::core::param::ParamSlot epsilonCuttingThresholdSlot;
        megamol::core::param::ParamSlot epsilonSelfIntersectSlot;
        megamol::core::param::ParamSlot renderContourArcsSlot;

        // current surface param values
        struct SurfaceParamValues {
          ComputationMode mode;
          float probeRadius;
          bool skipContourSingularities;
          bool computeSingularities;
          bool resizeVectors;
          bool debugFlag;
          bool useIterativeMethod;
          float epsilonCuttingThreshold;
          float epsilonSelfIntersect;
          bool renderContourArcs;

          // set by RenderPre/RenderPost!
          bool dirty;
        } surfaceParamValues;

        struct RendererParamValues {
          float epsilon;
          bool useHeadLight;
          vec3f lightDir;
          float lightDiffuseWeight;
          float lightAmbientWeight;
          float lightExponent;
          float alpha;
          bool useCuttingPlane;
          vec4f cuttingPlane;

          bool useAOOM;
          bool smallCavities;
          float rho;
          float tau;
          float aoDistance;
          int aoSamples;
          vec3f blendingColor;
          float blendingWeight;

          // set by RenderPre/RenderPost!
          bool dirty;
        } rendererParamValues;

			};
		}
	}
}

#endif
