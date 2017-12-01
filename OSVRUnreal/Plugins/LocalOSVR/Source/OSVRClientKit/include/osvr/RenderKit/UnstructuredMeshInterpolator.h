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

#ifndef INCLUDED_UnstructuredMeshInterpolator_h_GUID_974F522D_824F_4599_82FE_EE7FF8F98A1D
#define INCLUDED_UnstructuredMeshInterpolator_h_GUID_974F522D_824F_4599_82FE_EE7FF8F98A1D

// Internal Includes
#include "MonoPointMeshTypes.h"
#include "Float2.h"
#include "DistortionParameters.h"

// Library/third-party includes
// - none

// Standard includes
#include <array>
#include <vector>
#include <memory>

namespace osvr {
namespace renderkit {

    /// @brief Spatial-calculation-acceleration structure.
    ///
    /// This class makes a spatial data structure that makes it faster
    /// to determine the interpolated coordinates between vertices
    /// in an unstructured mesh.  It pre-fills in a small list of the
    /// nearest unstructured vertices to each location in a regular
    /// grid and then uses these to more-rapidly identify the nearest
    /// points when a large number of interpolations need to be done.
    class UnstructuredMeshInterpolator {
    public:
        /// Constructor, provided the list of points it is to use.
        /// Fills in the acceleration structure so that calls to
        /// interpolate will be faster.
        /// @param points Unstructured mesh points to use for interpolation
        /// @param numSamplesX Optional parameter describing the size of
        ///        the acceleration mesh structure.
        /// @param numSamplesY Optional parameter describing the size of
        ///        the acceleration mesh structure.
        OSVR_RENDERMANAGER_EXPORT UnstructuredMeshInterpolator(
          const MonoPointDistortionMeshDescription& points,
          int numSamplesX = 20,
          int numSamplesY = 20
        );

        /// Find an interpolation of the value based on the three
        /// nearest non-collinear points in the unstructured mesh.
        /// Attempts to use the spatial acceleration structure to
        /// speed up the query if it can.  If there are not three
        /// points, just return the original coordinates.
        /// @param xN Normalized x coordinate
        /// @param yN Normalized y coordinate
        /// @return Normalized coordinate interpolated from
        ///  unstructured distortion map mesh.
        Float2 OSVR_RENDERMANAGER_EXPORT interpolateNearestPoints(
          float xN, float yN);

    protected:

        /// Return the three nearest non-collinear points in the
        /// unstructured mesh description passed in.  If there are
        /// not three such points, can return fewer.
        /// @param xN Normalized texture coordinate in X
        /// @param yN Normalized texture coordinate in Y
        /// @param points Vector of points to search in.
        /// @return vector of up to three points.
        MonoPointDistortionMeshDescription getNearestPoints(
          float xN, float yN,
          const MonoPointDistortionMeshDescription &points);

        const MonoPointDistortionMeshDescription m_points;

        /// Structure to store points from the m_points array
        /// in a regular mesh covering the range of
        /// normalized texture coordinates from (0,0) to (1,1).
        ///   It is filled by the constructor and is used by the
        /// interpolator to hopefully provide a fast way to get
        /// a list of the three nearest non-collinear points.
        /// If there are not three such points here, the acceleration
        /// has failed for a location and the full point list is
        /// searched.
        std::vector<    // Range in X
            std::vector<  // Range in Y
            MonoPointDistortionMeshDescription ///< Points
            >
            > m_grid;
        int m_numSamplesX = 0; ///< Size of the grid in X
        int m_numSamplesY = 0; ///< Size of the grid in Y

        /// Return the index of the closest grid point to a
        /// specified location.  Clamps to the range of
        /// the grid even for points outside it.
        /// @param xN [in] Normalized X coordinate
        /// @param yN [in] Normalized Y coordinate
        /// @param xIndexOut [out] Index of nearest grid point
        /// @param yIndexOut [out] Index of nearest grid point
        /// @return True on success, false on no samples in X,Y
        inline bool getIndex(double xN, double yN,
                             int &xIndexOut, int &yIndexOut) {
            if (m_numSamplesX * m_numSamplesY == 0) {
                return false;
            }
            int xIndex = static_cast<int>(0.5 + xN * (m_numSamplesX - 1));
            if (xIndex < 0) { xIndex = 0; }
            if (xIndex >= m_numSamplesX) { xIndex = m_numSamplesX - 1; }
            int yIndex = static_cast<int>(0.5 + yN * (m_numSamplesY - 1));
            if (yIndex < 0) { yIndex = 0; }
            if (yIndex >= m_numSamplesY) { yIndex = m_numSamplesY - 1; }
            xIndexOut = xIndex;
            yIndexOut = yIndex;
            return true;
        }
    };

    /// This function returns either an empty vector (non-mesh-based
    /// distortion correction, a vector with one entry per eye (for
    /// mono mesh-based distortion correction) or a vector with three
    /// entries per eye (for rgb mesh-based distortion correction).
    /// It checks its input to make sure that it is valid.
    /// @brief Helper function to construct an appropriate-length vector
    ///        of UnstructuredMeshInterpolator depending on distortion
    ///        type.
    /// @param params Describes the distortion correction type
    /// @param eye Which eye to build the vector for.
    /// @param interpolators Reference to a filled-in vector.
    /// @return true on success, false (with undefined vector) on failure.
    bool OSVR_RENDERMANAGER_EXPORT makeUnstructuredMeshInterpolators(
      const DistortionParameters &params, size_t eye,
      std::vector< std::unique_ptr<UnstructuredMeshInterpolator> >
      &interpolators);

} // namespace renderkit
} // namespace osvr

#endif // INCLUDED_UnstructuredMeshInterpolator_h_GUID_974F522D_824F_4599_82FE_EE7FF8F98A1D

