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
#include "RenderManagerD3DBase.h"
#include <SDL.h>

namespace osvr {
namespace renderkit {

    class RenderManagerD3D11 : public RenderManagerD3D11Base {
      public:
        virtual ~RenderManagerD3D11();

        // Opens the D3D renderer we're going to use.
        OpenResults OpenDisplay() override;

        bool OSVR_RENDERMANAGER_EXPORT
          GetTimingInfo(size_t whichEye, OSVR_RenderTimingInfo& info) override;

      protected:
        /// Construct a D3D RenderManager.
        RenderManagerD3D11(
            OSVR_ClientContext context,
            ConstructorParameters p);

        // Classes and structures needed to do our rendering.
        class DisplayInfo {
          public:
            /// @todo Release these pointers in destructor
            SDL_Window* m_window = nullptr; ///< SDL window pointer
            HWND m_rawWindow = nullptr; ///< Raw Windows handle for the window
            IDXGISwapChain* m_swapChain =
                nullptr; ///< Pointer to the swap chain to use
            ID3D11Texture2D* m_renderTarget =
                nullptr; ///< Pointer to render target texture
            ID3D11RenderTargetView* m_renderTargetView =
                nullptr; ///< Pointer to our render target view
        };
        std::vector<DisplayInfo> m_displays;

        //===================================================================
        // Overloaded render functions from the base class.
        bool RenderFrameInitialize() override { return true; }
        bool RenderDisplayInitialize(size_t display) override { return true; }
        bool RenderEyeFinalize(size_t eye) override { return true; }
        bool RenderDisplayFinalize(size_t display) override { return true; }

        bool PresentDisplayInitialize(size_t display) override;
        bool PresentDisplayFinalize(size_t display) override;
        bool PresentFrameFinalize() override;

        bool SolidColorEye(size_t eye, const RGBColorf &color) override;

        friend RenderManager OSVR_RENDERMANAGER_EXPORT*
        createRenderManager(OSVR_ClientContext context,
                            const std::string& renderLibraryName,
                            GraphicsLibrary graphicsLibrary);

      private:
    };

} // namespace renderkit
} // namespace osvr
