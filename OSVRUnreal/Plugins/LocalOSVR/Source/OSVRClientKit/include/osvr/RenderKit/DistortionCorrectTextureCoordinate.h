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

#ifndef INCLUDED_DistortionCorrectTextureCoordinate_h_GUID_B37CBBBD_F2E1_4931_B9AC_B508C4D54F27
#define INCLUDED_DistortionCorrectTextureCoordinate_h_GUID_B37CBBBD_F2E1_4931_B9AC_B508C4D54F27

// Internal Includes
#include "DistortionCorrectTextureCoordinate.h"
#include "DistortionParameters.h"
#include "UnstructuredMeshInterpolator.h"
#include "Float2.h"

// Library/third-party includes
#include <Eigen/Core>

// Standard includes
#include <cstddef>      // for size_t
#include <array>


namespace osvr {
namespace renderkit {

    /// @name Forward declarations
    //@{
    Float2 OSVR_RENDERMANAGER_EXPORT DistortionCorrectTextureCoordinate(
        const size_t eye, Float2 const& inCoords,
        const DistortionParameters& distort, const size_t color,
        const float overfillFactor,
        const std::vector< std::unique_ptr<UnstructuredMeshInterpolator> >& interpolators);

    Float2 OSVR_RENDERMANAGER_EXPORT DistortionCorrectNormalizedTextureCoordinate(
        const size_t eye, Float2 const& inCoords,
        const DistortionParameters& distort, const size_t color,
        const std::vector< std::unique_ptr<UnstructuredMeshInterpolator> >& interpolators);

    Float2 OSVR_RENDERMANAGER_EXPORT DistortionCorrectRGBSymmetricPolynomials(
        Float2 const& inCoords, const DistortionParameters& distort,
        const size_t color);

    Float2 OSVR_RENDERMANAGER_EXPORT DistortionCorrectMonoPointSamples(
        const size_t eye, Float2 const& inCoords,
        const DistortionParameters& distort,
        const std::vector< std::unique_ptr<UnstructuredMeshInterpolator> >& interpolators);

    Float2 OSVR_RENDERMANAGER_EXPORT DistortionCorrectRGBPointSamples(
        const size_t eye, Float2 const& inCoords,
        const DistortionParameters& distort, const size_t color,
        const std::vector< std::unique_ptr<UnstructuredMeshInterpolator> >& interpolators);
    //@}

    /// @brief Distortion-correct a texture coordinate in PresentMode
    ///
    /// Takes a texture coordinate that is specified in the coordinate system of
    /// a Presented texture for a given eye, which has (0,0) at the lower left
    /// and (1,1) at the upper right.  The lower left and upper right are at the
    /// boundaries specified by the overfill rectangle, which are not visible
    /// for overfill factors > 1.
    ///
    /// Returns the distorted location, in the same coordinate system.
    ///
    /// Uses the specified distortion parameters to convert to the distortion
    /// space, distort, and convert back (also taking into account that the
    /// distortion parameters are specified for a window that has no overfill).
    ///
    /// @param eye eye to get coordinates for
    /// @param inCoords coordinates to modify
    /// @param distort distortion parameters
    /// @param color red=0, green=1, blue=2
    /// @param overfillFactor scaling factor to allow for timewarp
    /// @param interpolators list of unstructured mesh interpolators
    ///
    /// @return New coordinates on success, unchanged coordinates on
    /// failure.
    inline Float2 OSVR_RENDERMANAGER_EXPORT DistortionCorrectTextureCoordinate(
        const size_t eye, Float2 const& inCoords,
        const DistortionParameters& distort, const size_t color,
        const float overfillFactor,
        const std::vector< std::unique_ptr<UnstructuredMeshInterpolator> >& interpolators) {
        // Check for invalid parameters
        if (color > 2) {
            return inCoords;
        }

        // Convert from coordinates in the overfilled texture to coordinates
        // that will cover the range (0,0) to (1,1) on the screen.  This is
        // done by scaling around (0.5,0.5) to push the edges of the screen
        // out to the (0,0) and (1,1) boundaries.
        using Eigen::Vector2f;
        using Eigen::Map;
        const auto inMap = Map<const Vector2f>(inCoords.data());

        Vector2f xyN = (inMap - Vector2f::Constant(0.5f)) * overfillFactor +
                       Vector2f::Constant(0.5f);
        const float xN = xyN.x();
        const float yN = xyN.y();

        const auto normalized_inCoords = Float2{xN, yN};

        Float2 ret = DistortionCorrectNormalizedTextureCoordinate(
            eye, normalized_inCoords, distort, color, interpolators);

        // Convert from unit (normalized) space back into overfill space.
        ret[0] = (ret[0] - 0.5f) / overfillFactor + 0.5f;
        ret[1] = (ret[1] - 0.5f) / overfillFactor + 0.5f;

        return ret;
    }

    /// @brief Distortion-correct a normalized texture coordinate
    ///
    /// Takes a normalized texture coordinate that is specified in the
    /// coordinate system for a given eye, which has (0,0) at the lower left
    /// and (1,1) at the upper right.  for overfill factors > 1.
    ///
    /// Returns the distorted location, in the same coordinate system.
    ///
    /// Uses the specified distortion parameters to convert to the
    /// distortion space, distort, and convert back.
    ///
    /// @param eye eye to get coordinates for
    /// @param inCoords coordinates to modify
    /// @param distort distortion parameters
    /// @param color red=0, green=1, blue=2
    /// @param interpolators list of unstructured mesh interpolators
    ///
    /// @return New coordinates on success, unchanged coordinates on
    /// failure.
    inline Float2 OSVR_RENDERMANAGER_EXPORT DistortionCorrectNormalizedTextureCoordinate(
        const size_t eye, Float2 const& inCoords,
        const DistortionParameters& distort, const size_t color,
        const std::vector< std::unique_ptr<UnstructuredMeshInterpolator> >& interpolators) {
        // Check for invalid parameters
        if (color > 2) {
            return inCoords;
        }

        switch (distort.m_type) {
        case DistortionParameters::rgb_symmetric_polynomials:
            return DistortionCorrectRGBSymmetricPolynomials(inCoords, distort,
                                                            color);
        case DistortionParameters::mono_point_samples:
            return DistortionCorrectMonoPointSamples(eye, inCoords, distort,
                                                     interpolators);
        case DistortionParameters::rgb_point_samples:
            return DistortionCorrectRGBPointSamples(eye, inCoords, distort,
                                                    color, interpolators);
        default:
            return inCoords;
        }
    }

    inline Float2 OSVR_RENDERMANAGER_EXPORT DistortionCorrectRGBSymmetricPolynomials(
        Float2 const& inCoords, const DistortionParameters& distort,
        const size_t color) {
        Float2 ret = inCoords;

        // rgb_symmetric_polynomials
        if (distort.m_distortionPolynomialRed.size() < 2) {
            return ret;
        }
        if (distort.m_distortionPolynomialGreen.size() < 2) {
            return ret;
        }
        if (distort.m_distortionPolynomialBlue.size() < 2) {
            return ret;
        }
        if (distort.m_distortionCOP.size() != 2) {
            return ret;
        }
        if (distort.m_distortionD.size() != 2) {
            return ret;
        }
        if (distort.m_distortionD[0] <= 0) {
            return ret;
        }
        if (distort.m_distortionD[1] <= 0) {
            return ret;
        }

        // Convert from normalized range to (D[0], D[1]) range.  Here,
        // both coordinate systems share a common (0,0) boundary so we
        // can just scale around the origin.
        using Eigen::Vector2f;
        using Eigen::Map;
        const Vector2f xyN = Map<const Vector2f>(inCoords.data());
        const Vector2f xyD =
            xyN.cwiseProduct(Vector2f::Map(distort.m_distortionD.data()));

        // Compute the distance from the COP in D space
        // (direction and squared magnitude).  First convert from a COP
        // that ranges from 0-1 across the entire viewport into D space
        // by multiplying it by D on each axis.
        Eigen::Vector2f COPinD =
          Vector2f::Map(distort.m_distortionD.data()).cwiseProduct(
          Vector2f::Map(distort.m_distortionCOP.data()));
        const Vector2f xyDDiff = xyD - COPinD;
        const float rMag2 = xyDDiff.squaredNorm();
        if (rMag2 == 0) { // We're at the center -- no distortion
            ret = inCoords;
            return ret;
        }
        const float rMag = sqrt(rMag2);
        const Vector2f xyDNorm = xyDDiff / rMag;

        // Compute the new location in D space based on the distortion
        // parameters
        std::vector<float> const& params =
            color == 0 ? distort.m_distortionPolynomialRed
                        : color == 1 ? distort.m_distortionPolynomialGreen
                                    : distort.m_distortionPolynomialBlue;

        // The distance scaling factor needs to be applied as many times
        // as the degree of the polynomial we are using.  For the constant
        // term, the factor is 1.
        float rFactor = 1;
        float rNew = params[0];

        auto n = params.size();
        using index_type = decltype(n);
        for (index_type i = 1; i < n; i++) {
            rFactor *= rMag;
            rNew += params[i] * rFactor;
        }
        const Vector2f xyDNew = COPinD + rNew * xyDNorm;

        // Convert from D space back to unit space
        const Vector2f xyNNew = xyDNew.cwiseQuotient(
            Vector2f::Map(distort.m_distortionD.data()));

        Vector2f::Map(ret.data()) = xyNNew;

        return ret;
    }

    inline Float2 OSVR_RENDERMANAGER_EXPORT DistortionCorrectMonoPointSamples(
        const size_t eye, Float2 const& inCoords,
        const DistortionParameters& distort,
        const std::vector< std::unique_ptr<UnstructuredMeshInterpolator> >& interpolators) {
        if (eye >= distort.m_monoPointSamples.size()) {
            return inCoords;
        }

        // Find the three non-collinear points in the mesh that are nearest
        // to the normalized point we are trying to look up.  We start by
        // sorting the points based on distance from our location, selecting
        // the first two, and then looking through the rest until we find
        // one that is not collinear with the first two (normalized dot
        // product magnitude far enough from 1).  If we don't find such
        // points, we just go with the values from the closest point.
        return interpolators[0]->interpolateNearestPoints(inCoords[0],
                                                          inCoords[1]);
    }

    inline Float2 OSVR_RENDERMANAGER_EXPORT DistortionCorrectRGBPointSamples(
        const size_t eye, Float2 const& inCoords,
        const DistortionParameters& distort, const size_t color,
        const std::vector< std::unique_ptr<UnstructuredMeshInterpolator> >& interpolators) {
        if (color >= 3) {
            return inCoords;
        }

        if (eye >= distort.m_rgbPointSamples[color].size()) {
            return inCoords;
        }

        // Find the three non-collinear points in the mesh that are nearest
        // to the normalized point we are trying to look up.  We start by
        // sorting the points based on distance from our location, selecting
        // the first two, and then looking through the rest until we find
        // one that is not collinear with the first two (normalized dot
        // product magnitude far enough from 1).  If we don't find such
        // points, we just go with the values from the closest point.
        return interpolators[color]->interpolateNearestPoints(inCoords[0],
                                                              inCoords[1]);
    }

} // end namespace renderkit
} // end namespace osvr

#endif // INCLUDED_DistortionCorrectTextureCoordinate_h_GUID_B37CBBBD_F2E1_4931_B9AC_B508C4D54F27
