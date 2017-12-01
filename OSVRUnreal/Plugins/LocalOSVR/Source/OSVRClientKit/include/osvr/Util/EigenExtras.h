/** @file
    @brief Header

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
//        http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef INCLUDED_EigenExtras_h_GUID_7AE6CABA_333B_408A_C898_A2CBBE5BCE5D
#define INCLUDED_EigenExtras_h_GUID_7AE6CABA_333B_408A_C898_A2CBBE5BCE5D

// Internal Includes
#include <osvr/Util/EigenCoreGeometry.h>

// Library/third-party includes
// - none

// Standard includes
#include <type_traits>

namespace osvr {
namespace util {

    using ColMatrix44d = Eigen::Matrix4d;
    using ColVector3d = Eigen::Vector3d;
    using ColVector4d = Eigen::Vector4d;
    using RowMatrix44d = Eigen::Matrix<double, 4, 4, Eigen::RowMajor>;
    using Eigen::RowVector3d;
    using Eigen::RowVector4d;

    template <typename Scalar>
    using Isometry3 = Eigen::Transform<Scalar, 3, Eigen::Isometry>;
    template <typename Scalar>
    using Translation3 = Eigen::Translation<Scalar, 3>;

    namespace detail {
        template <int Size, typename PrototypeVector>
        using SameLayoutVector =
            Eigen::Matrix<typename PrototypeVector::Scalar,
                          PrototypeVector::IsRowMajor ? Size : 1,
                          PrototypeVector::IsRowMajor ? 1 : Size>;
    } // namespace detail

    /// @brief Makes a 3D vector into a 4D homogeneous point, with the same
    /// options (scalar, row vs col vector) as the input.
    template <typename Derived>
    inline detail::SameLayoutVector<4, Derived>
    makeHomogeneousPoint(Eigen::MatrixBase<Derived> const &vec) {
        EIGEN_STATIC_ASSERT_VECTOR_SPECIFIC_SIZE(Derived, 3);
        return (detail::SameLayoutVector<4, Derived>() << vec, 1).finished();
    }

    /// @brief Makes a 3D vector into a 4D homogeneous vector, with the same
    /// options (scalar, row vs col vector) as the input.
    template <typename Derived>
    inline detail::SameLayoutVector<4, Derived>
    makeHomogeneousVector(Eigen::MatrixBase<Derived> const &vec) {
        EIGEN_STATIC_ASSERT_VECTOR_SPECIFIC_SIZE(Derived, 3);
        return (detail::SameLayoutVector<4, Derived>() << vec, 0).finished();
    }

    /// @brief Pulls the 3D point or vector from a 4D vec,
    /// performing division by `w` if it is nonzero.
    template <typename Derived>
    inline detail::SameLayoutVector<3, Derived>
    extractPoint(Eigen::MatrixBase<Derived> const &homogenous) {
        EIGEN_STATIC_ASSERT_VECTOR_SPECIFIC_SIZE(Derived, 4);
        return homogenous[3] == 0
                   ? homogenous.template head<3>().eval()
                   : (homogenous.template head<3>() / homogenous[3]).eval();
    }

    /// @brief A simpler, functional-style alternative to
    /// `.fromPositionOrientationScale` when no scaling is performed.
    template <typename Derived1, typename Derived2>
    inline Isometry3<typename Derived1::Scalar>
    makeIsometry(Eigen::MatrixBase<Derived1> const &translation,
                 Eigen::RotationBase<Derived2, 3> const &rotation) {
        EIGEN_STATIC_ASSERT_VECTOR_SPECIFIC_SIZE(Derived1, 3);
        static_assert(
            std::is_same<typename Derived1::Scalar,
                         typename Derived2::Scalar>::value,
            "Translation and rotation do not have the same scalar type.");
        using Scalar = typename Derived1::Scalar;
        return Translation3<Scalar>(translation) * Isometry3<Scalar>(rotation);
    }

    inline Eigen::Quaterniond
    flipQuatSignToMatch(Eigen::Quaterniond const &refQ,
                        Eigen::Quaterniond const &q) {
        if (refQ.dot(q) < 0) {
            return Eigen::Quaterniond(-q.coeffs());
        } else {
            return q;
        }
    }

} // namespace util
} // namespace osvr

#endif // INCLUDED_EigenExtras_h_GUID_7AE6CABA_333B_408A_C898_A2CBBE5BCE5D
