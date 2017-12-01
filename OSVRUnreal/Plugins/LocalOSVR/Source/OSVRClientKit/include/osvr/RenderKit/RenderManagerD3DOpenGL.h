/** @file
@brief Header file describing the OSVR direct-to-device rendering interface

@date 2015

@author
Sensics, Inc.
<http://sensics.com/osvr>
*/

// Copyright 2015 Sensics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once
#include <osvr/ClientKit/Context.h>
#include <osvr/ClientKit/Interface.h>
#include "RenderManagerD3DBase.h"
#include "RenderManagerOpenGL.h"
#include "GraphicsLibraryD3D11.h"

#include <vector>
#include <string>

namespace osvr {
namespace renderkit {

    class RenderManagerD3D11OpenGL : public RenderManagerOpenGL {
      public:
        virtual OSVR_RENDERMANAGER_EXPORT ~RenderManagerD3D11OpenGL();

        // Is the renderer currently working?
        bool OSVR_RENDERMANAGER_EXPORT doingOkay() override {
            // We check both our harnessed renderer and us.
            if ((m_D3D11Renderer.get() != nullptr) &&
                  m_D3D11Renderer->doingOkay() && RenderManagerOpenGL::doingOkay()) {
                return true;
            } else {
                return false;
            }
        }

        // Opens the D3D renderer we're going to use.
        OpenResults OSVR_RENDERMANAGER_EXPORT OpenDisplay() override;

        // Harnesses a D3D DirectMode renderer to do timing.
        bool OSVR_RENDERMANAGER_EXPORT
        GetTimingInfo(size_t whichEye, OSVR_RenderTimingInfo& info) override {
            return m_D3D11Renderer->GetTimingInfo(whichEye, info);
        }

      protected:
        /// Construct a D3D DirectMode renderer to do DirectMode
        // rendering, then harness it so that we can provide an OpenGL
        // library interface back to the client.  Uses the OpenGL/DX
        // interop functions to connect the two.
        // NOTE: This will call delete on the harnessed RenderManager
        // when it is destroyed.
        RenderManagerD3D11OpenGL(
            OSVR_ClientContext context,
            ConstructorParameters p,
            std::unique_ptr<RenderManagerD3D11Base>&& D3DToHarness);

        bool m_displayOpen; ///< Has our display been opened?

        // We harness a D3D11 DirectMode renderer to do our
        // DirectMode work and to handle the timing.
        std::unique_ptr<RenderManagerD3D11Base> m_D3D11Renderer;

        // Mapping from textures presented by the client using
        // PresentEye() to bound D3D texture buffers used to do
        // the rendering in our D3D host.  We cache them here so
        // that we're not re-creating and re-binding them every
        // frame.  The handle is used to
        typedef struct {
            GLuint OpenGLTexture;
            HANDLE glColorHandle;
            RenderBufferD3D11 D3DBuffer;
        } OglToD3DTexture;
        std::vector<OglToD3DTexture> m_oglToD3D;

        // OpenGL-related state information
        HANDLE m_glD3DHandle = nullptr;

        // Clean up the OpenGL-related state information.
        void cleanupGL();

        //===================================================================
        // Overloaded render functions from the base class.
        bool RenderFrameInitialize() override;
        bool RenderDisplayInitialize(size_t display) override;
        bool RenderEyeInitialize(size_t eye) override;

        OSVR_RENDERMANAGER_EXPORT bool UpdateDistortionMeshesInternal(
            DistortionMeshType type ///< Type of mesh to produce
            ,
            std::vector<DistortionParameters> const&
                distort ///< Distortion parameters
            ) override {
            return m_D3D11Renderer->UpdateDistortionMeshesInternal(type,
                                                                   distort);
        }

        // We need to flip the projection information so that our output
        // images match those used by Direct3D, so we don't need to (1)
        // flip the textures and (2) Modify the time warp calculations.
        // We flip this back again inside PresentRenderBuffers() so that
        // time warp is not confused by changing motion.
        virtual std::vector<RenderInfo>
          GetRenderInfoInternal(const RenderParams& params = RenderParams());

        // We use the render-buffer registration to construct
        // D3D buffers to be used for PresentMode, which we then map
        // our buffers to.
        bool RegisterRenderBuffersInternal(
            const std::vector<RenderBuffer>& buffers,
            bool appWillNotOverwriteBeforeNewPresent = false) override;

        bool PresentDisplayInitialize(size_t display) override;
        bool PresentDisplayFinalize(size_t display) override;
        bool PresentFrameInitialize() override;
        bool PresentFrameFinalize() override;
        bool PresentRenderBuffersInternal(const std::vector<RenderBuffer>& renderBuffers,
          const std::vector<RenderInfo>& renderInfoUsed,
          const RenderParams& renderParams = RenderParams(),
          const std::vector<OSVR_ViewportDescription>& normalizedCroppingViewports =
          std::vector<OSVR_ViewportDescription>(),
          bool flipInY = false) override;
        bool SolidColorEye(size_t eye, const RGBColorf &color) override {
          return m_D3D11Renderer->SolidColorEye(eye, color);
        }

        friend RenderManager OSVR_RENDERMANAGER_EXPORT*
        createRenderManager(OSVR_ClientContext context,
                            const std::string& renderLibraryName,
                            GraphicsLibrary graphicsLibrary);

    };

} // namespace renderkit
} // namespace osvr
