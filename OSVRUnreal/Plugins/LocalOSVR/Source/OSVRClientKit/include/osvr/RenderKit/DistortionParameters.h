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

#ifndef INCLUDED_DistortionParameters_h_GUID_2E796EBA_C90A_4ECB_B718_83E7BCC06C9D
#define INCLUDED_DistortionParameters_h_GUID_2E796EBA_C90A_4ECB_B718_83E7BCC06C9D

// Internal Includes
#include "MonoPointMeshTypes.h"
#include "RGBPointMeshTypes.h"

// Required for DLL linkage on Windows
#include <osvr/RenderKit/Export.h>

// Library/third-party includes
// - none

// Standard includes
#include <vector>
#include <cstddef>      // for size_t

// Forward declaration so that we can avoid including this header in
// files that the client has to include.
class OSVRDisplayConfiguration;

namespace osvr {
namespace renderkit {

    ///-------------------------------------------------------------
    /// Class that stores one of a set of possible distortion parameters.
    /// The type of parameters is determined by the m_type, and which
    /// other entries are valid depends on the type.
    /// === Description of mono_point_samples parameters follows:
    /// This describes a mapping from normalized (X,Y) coordinates in
    /// the physical display ([0,0] at the lower-left corner, [1,1]
    /// at the upper right) into normalized coordinates in a canonical
    /// rectangle that has an overfill value of 1 (no additional
    /// overfill). Values may map outside the range [0,0]-[1,1] due
    /// to the distortion; they will map initially into the overfill
    /// area and then eventually beyond if there is insufficient
    /// overfill.
    ///   For some distortion-correction approaches, none of the points
    /// will map outside this range, because all visible display points
    /// maps to locations that lie within the projected area of the
    /// canonical screen even with an overfill factor of 1.  This is
    /// case for the angles_to_config program.
    ///   This is an unordered set of points, which is stored as a
    /// vector of elements, each of which has two elements, the first
    /// of which is the 2D coordinates in normalized physical-screen
    /// coordinates and the second of which is the 2D coordinates in
    /// the canonical-screen coordinates.
    /// === Description of rgb_point_samples parameters follows:
    ///   This is an array of three mono_point_sample entries, the first
    /// for red, the second for green, and the third for blue.
    /// === Description of rgb_symmetric_polynomials parameters follows:
    /// Because distortion correction depends on the lens geometry and the
    /// screen geometry, and may not be directly related to the viewport
    /// size or aspect ratio (for lenses that expand more in one direction
    /// than the other), we need to allow the specification of not only the
    /// radial distortion polynomial coefficients (which scale powers of the
    /// distance from the center of projection to the point), but also the
    /// space in which this is measured.  We specify the space by telling
    /// the number of unit radii in the space the parameters are defined
    /// in lies across the texture coordinates, which range from 0 to 1.
    ///   This can be different for X and Y, as the viewport may be
    /// non-square and the lens system may make yet a different aspect ratio.
    /// The D[0] component tells the width and D[1] tells the height.
    ///   The coefficients for R, G, and B and the Distances for X and Y
    /// may be specified in any consistent space that is desired (scaling
    /// all of them linearly will have no impact on the result), but
    /// lower-left corner of the space (as viewed on the screen)
    /// must be (0,0) and the far side of the pixels on the top and right
    /// are at the D-specified locations.
    ///   The first coefficient in each polynomial is a constant factor
    /// (multiplied by offset^0, or 1), the second is the linear factor, the
    /// third is quadratic, and so forth.
    ///   The COP is always specified in fractions of the screen width and
    /// height, independently of D.  This is internally scaled to D space.
    ///   For a display 10 pixels wide by 8 pixels high that has square
    /// pixels whose center of projection is in the middle of the image,
    /// we would get:
    /// D = (10, 8); COP = (0.5, 0.5); parameters specified in pixel-unit
    /// offsets.
    ///   For a display that is 6 units wide by 12 units high, but whose
    /// optics stretch the view horizontally to produce a square viewing
    /// image with pixels that are stretched in X, we could have:
    /// D = (12, 12); COP = (0.5, 0.5); parameters specified in vertical
    /// pixel-sized units
    ///   -or-
    /// D = (6,6); COP = (0.5, 0.5); parameters specified in horizontal
    /// pixel-sized units.
    ///   The parameters for each color specify the new radial displacement
    /// from the center of projection as a function of the original
    /// displacement.
    ///   In D-scaled space, this is:
    ///    Offset = Orig - COP*D;            // Vector, component-wise mult.
    ///    OffsetMag = sqrt(Offset.length() * Offset.length());  // Scalar
    ///    NormOffset = Offset / OffsetMag;  // Vector
    ///    Final = COP*D + (a0 + a1*OffsetMag + a2*OffsetMag*OffsetMag + ...)
    ///            * NormOffset; // Position

    /// @todo Adjust the effect of flipping Y in the OpenGL to D3D to suit
    /// (negate COP[y]) for symmetric polynomials, flip Y axis for the
    /// distortion map for sampled versions.

    class DistortionParameters {
    public:
        typedef enum {
            mono_point_samples,
            rgb_point_samples,
            rgb_symmetric_polynomials
        } Type;

        OSVR_RENDERMANAGER_EXPORT DistortionParameters(
          OSVRDisplayConfiguration& osvrParams,
          size_t eye);

        OSVR_RENDERMANAGER_EXPORT DistortionParameters();

        /** \name Parameters valid for all mesh types */
        //@{
        /// Type of parameters stored, determines which fields below are valid.
        Type m_type;

        /// How many triangles would we like in the mesh?
        size_t m_desiredTriangles;
        //@}

        /** \name Parameters valid for a mesh of type @c mono_point_samples */
        //@{
        MonoPointDistortionMeshDescriptions m_monoPointSamples;
        //@}

        /** \name Parameters valid for a mesh of type @c rgb_point_samples */
        //@{
        RGBPointDistortionMeshDescriptions m_rgbPointSamples;
        //@}

        /** \name Parameters valid for a mesh of type @c rgb_symmetric_polynomials */
        //@{
        /// Constant, linear, quadratic, ... for Red
        std::vector<float> m_distortionPolynomialRed;

        /// Constant, linear, quadratic, ... for Green
        std::vector<float> m_distortionPolynomialGreen;

        /// Constant, linear, quadratic, ... for Blue
        std::vector<float> m_distortionPolynomialBlue;

        /// (X,Y) location of center of projection in texture coords
        std::vector<float> m_distortionCOP;

        /// How many K1's wide and high is (0-1) in texture coords
        std::vector<float> m_distortionD;
        //@}
    };

} // namespace renderkit
} // namespace osvr

#endif // INCLUDED_DistortionParameters_h_GUID_2E796EBA_C90A_4ECB_B718_83E7BCC06C9D

