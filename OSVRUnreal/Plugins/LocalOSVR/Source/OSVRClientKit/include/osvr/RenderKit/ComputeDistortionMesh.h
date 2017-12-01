/** @file
    @brief Header

    @date 2016

    @author
    Sensics, Inc.
    <http://sensics.com>

*/

// Copyright 2016 Sensics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// 	http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef INCLUDED_ComputeDistortionMesh_h_GUID_EEB1F254_03BE_407A_B30B_CC201C5F44DF
#define INCLUDED_ComputeDistortionMesh_h_GUID_EEB1F254_03BE_407A_B30B_CC201C5F44DF

// Internal Includes
#include "DistortionMesh.h"
#include "DistortionParameters.h"

// Library/third-party includes
// - none

// Standard includes
#include <cstddef>      // for size_t

namespace osvr {
namespace renderkit {

    /// @brief Constructs a mesh to correct lens distortions
    ///
    /// Constructs a set of vertices in the range (-1,-1) to (1,1),
    /// with (-1,-1) at the lower-left corner and (1,1) at the upper
    /// right and the first coordinate in X.  The number and
    /// arrangement of these vertices can be specified.
    ///
    /// NOTE: The vertices only have two coordinates; you need
    /// to fill in a default Z and homogenous value (maybe 0,1)
    /// when you use them.
    ///
    /// Attaches to each vertex three pairs of texture coordinates
    /// that are the texture location that will be mapped onto this
    /// screen location by the lens distortion.  These coordinates
    /// will be in the subset of the range (0,0) to (1,1) that falls
    /// onto the screen after distortion is applied.  There is one
    /// pair for red, one for green, and one for blue.
    ///
    /// There are sets of 3 vertices produced, suitable for sending
    /// as a set of triangles to the rendering system.
    ///
    ///  @todo Consider switching to an indexed-based mesh.
    ///
    ///  @param eye which eye
    ///  @param type type of mesh to produce
    ///  @param distort distortion parameters
    ///  @param overfillFactor overfill factor
    ///
    ///  @return Vector of triangles (sets of 3 vertices), empty on failure.
    DistortionMesh OSVR_RENDERMANAGER_EXPORT ComputeDistortionMesh(
      size_t eye, DistortionMeshType type, DistortionParameters distort,
      float overfillFactor);

} // namespace osvr
} // namespace renderkit

#endif // INCLUDED_ComputeDistortionMesh_h_GUID_EEB1F254_03BE_407A_B30B_CC201C5F44DF

