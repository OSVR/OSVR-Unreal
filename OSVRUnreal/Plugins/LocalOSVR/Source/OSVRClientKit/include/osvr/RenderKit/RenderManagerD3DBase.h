/** @file
@brief Header file describing the OSVR direct-to-device rendering interface for
D3D

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
#include "RenderManager.h"
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>
#endif

#include <vector>
#include <string>

namespace osvr {
namespace renderkit {

    class RenderManagerD3D11Base : public RenderManager {
      public:
        virtual OSVR_RENDERMANAGER_EXPORT ~RenderManagerD3D11Base();

        // Creates the D3D11 device and context to be used
        // to draw things into our window unless they have
        // already been filled in.
        virtual bool OSVR_RENDERMANAGER_EXPORT SetDeviceAndContext();

        // Opens the D3D renderer we're going to use.
        OpenResults OSVR_RENDERMANAGER_EXPORT OpenDisplay() override;

      protected:

       virtual bool OSVR_RENDERMANAGER_EXPORT PresentRenderBuffersInternal(
              const std::vector<RenderBuffer>& buffers,
              const std::vector<RenderInfo>& renderInfoUsed,
              const RenderParams& renderParams = RenderParams(),
              const std::vector<OSVR_ViewportDescription>&
              normalizedCroppingViewports =
              std::vector<OSVR_ViewportDescription>(),
              bool flipInY = false) override;

        /// Construct a D3D RenderManager.
        OSVR_RENDERMANAGER_EXPORT RenderManagerD3D11Base(
            OSVR_ClientContext context,
            ConstructorParameters p);

        virtual bool OSVR_RENDERMANAGER_EXPORT UpdateDistortionMeshesInternal(
            DistortionMeshType type ///< Type of mesh to produce
            ,
            std::vector<DistortionParameters> const&
                distort ///< Distortion parameters
            ) override;

        /// Call before calling OpenDisplay() to set the DXGIAdapter if you
        /// don't want the default one.
        void OSVR_RENDERMANAGER_EXPORT setAdapter(
          Microsoft::WRL::ComPtr<IDXGIAdapter> const& adapter);

        /// Get the D3D11 Device as a IDXGIDevice
        Microsoft::WRL::ComPtr<IDXGIDevice> OSVR_RENDERMANAGER_EXPORT getDXGIDevice();

        /// Get the adapter, whether manually specified or automatically
        /// determined.
        Microsoft::WRL::ComPtr<IDXGIAdapter> OSVR_RENDERMANAGER_EXPORT getDXGIAdapter();

        /// Get the DXGIFactor1 corresponding to the adapter.
        Microsoft::WRL::ComPtr<IDXGIFactory1> OSVR_RENDERMANAGER_EXPORT getDXGIFactory();

        bool m_displayOpen; ///< Has our display been opened?

		/// Block until vsync after a framebuffer or direct mode present, even if
		/// parameters say not to. This is used by the ATW wrapper to do a more
		/// fine-grained blocking (no blocking before present, but yes blocking after)
		/// TODO: Could this be added as a new parameter? Might be a breaking change if so.
		bool m_vsyncBlockAfterFrameBufferPresent = false;

        /// The adapter, if and only if explicitly set.
        Microsoft::WRL::ComPtr<IDXGIAdapter> m_adapter;

        // D3D-related state information
        /// @todo Release these pointers in destructor
        ID3D11Device* m_D3D11device; ///< Pointer to the D3D11 device to use.
        ID3D11DeviceContext*
            m_D3D11Context; ///< Pointer to the D3D11 context to use.

        //============================================================================
        // Information needed to provide render and depth/stencil buffers for
        // each of the eyes we give to the user to use when rendering.  This is
        // for user code to render into.
        //   This is only used in the non-present-mode interface.
        std::vector<osvr::renderkit::RenderBuffer> m_renderBuffers;
        ID3D11DepthStencilState* m_depthStencilStateForRender;

        /// Construct the buffers we're going to use in Render() mode, which
        /// we use to actually use the Presentation mode.  This gives us the
        /// main Presentation path as the basic approach which we can build on
        /// top of, and also lets us make the intermediate buffers the correct
        /// size we need for Asychronous Time Warp and distortion, and keeps
        /// them from being in the same window and so bleeding together.
        bool OSVR_RENDERMANAGER_EXPORT constructRenderBuffers();

        //============================================================================
        // Information needed to render to the final output buffer.  Render
        // state and geometries needed to go from the presented buffers to the
        // screen.
        struct XMFLOAT3 {
            float x;
            float y;
            float z;
        };
        struct XMFLOAT2 {
            float x;
            float y;
        };
        struct DistortionVertex {
            XMFLOAT3 Pos;
            XMFLOAT2 TexR;
            XMFLOAT2 TexG;
            XMFLOAT2 TexB;
        };
        // We do not need to release these in destructor because they auto-release.
        Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader = nullptr;
        Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader = nullptr;
        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_renderTextureSamplerState = nullptr;
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState = nullptr;
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_cbPerObjectBuffer = nullptr;
        // @todo We need to release this in the destructor, or change to a WRL pointer
        ID3D11InputLayout* m_vertexLayout = nullptr;

        struct DistortionMeshBuffer {
            /// Used to render quads for present mode
            Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
            /// Backing data for vertexBuffer
            std::vector<DistortionVertex> vertices;
            /// Vertex indices, for DrawIndexed
            Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
            /// Backing data for indexBuffer
            std::vector<UINT16> indices;
        };

        /// @todo One per eye/display combination in case of multiple displays
        /// per eye
        std::vector<DistortionMeshBuffer> m_distortionMeshBuffer;

        ID3D11DepthStencilState* m_depthStencilStateForPresent; // Depth/stencil
                                                                // state that
                                                                // disables both

        /// Type of matrices that we pass as uniform parameters to the shader.
        struct cbPerObject {
            DirectX::XMMATRIX projection;
            DirectX::XMMATRIX modelView;
            DirectX::XMMATRIX texture;
        };

        /// @brief Wait for rendering completion on our D3D11 context and device.
        ///
        /// This function is used to avoid tearing caused by the presentation
        /// of a buffer in DirectMode before rendering has completed into
        /// that buffer.  This forces us to hold off on presentation until
        /// rendering has completed.
        virtual bool OSVR_RENDERMANAGER_EXPORT WaitForRenderCompletion();

        //===================================================================
        // Overloaded render functions from the base class.  Not all of the
        // ones that need overloading are here; derived classes must decide
        // what to do for those.
        bool OSVR_RENDERMANAGER_EXPORT RenderPathSetup() override;
        bool OSVR_RENDERMANAGER_EXPORT RenderEyeInitialize(size_t eye) override;
        bool OSVR_RENDERMANAGER_EXPORT 
        RenderSpace(size_t whichSpace, ///< Index into m_callbacks vector
                         size_t whichEye, ///< Which eye are we rendering for?
                         OSVR_PoseState pose, ///< ModelView transform to use
                         OSVR_ViewportDescription viewport, ///< Viewport to use
                         OSVR_ProjectionMatrix projection ///< Projection to use
                         ) override;

        bool OSVR_RENDERMANAGER_EXPORT RenderFrameInitialize() override { return true; }
        bool OSVR_RENDERMANAGER_EXPORT RenderDisplayFinalize(size_t display) override;
        bool OSVR_RENDERMANAGER_EXPORT RenderFrameFinalize() override;

        bool OSVR_RENDERMANAGER_EXPORT PresentFrameInitialize() override { return true; }
        bool OSVR_RENDERMANAGER_EXPORT PresentEye(PresentEyeParameters params) override;

        /// Used to keep track of when rendering has completed so we can hand
        /// our buffers over to the ATW thread.
        ID3D11Query* m_completionQuery = nullptr;

        friend class RenderManagerD3D11OpenGL;
        friend class RenderManagerD3D11ATW;
        friend class ::sensics::compositor::DisplayServerInterfaceD3D11Singleton;

		// needed to access m_vsyncBlockAfterFrameBufferPresent from createRenderManager
		// TODO: if this is added as a parameter (breaking change), then this isn't needed anymore and should be removed.
		friend RenderManager* createRenderManager(OSVR_ClientContext contextParameter,
			const std::string& renderLibraryName,
			GraphicsLibrary graphicsLibrary);
    };

} // namespace renderkit
} // namespace osvr
