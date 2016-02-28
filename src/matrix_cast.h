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
// 	http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef INCLUDED_matrix_cast_h_GUID_27799A61_A16D_4B3A_AFA8_A2A4336D40AD
#define INCLUDED_matrix_cast_h_GUID_27799A61_A16D_4B3A_AFA8_A2A4336D40AD

// Internal Includes
#include "identity.h"

// Library/third-party includes
#include <Eigen/Geometry>
#include <openvr_driver.h>

// Standard includes
// - none

typedef Eigen::Matrix<float, 3, 4> Matrix34f;

namespace detail {

/**
 * Traits class to specify the Eigen equivalent of a given type, and also to
 * provide a static get() method to get the data pointer.
 */
template <typename OpenVRType>
struct ConvertOpenVRToEigenTraits {
};

/**
 * Base of ConvertOpenVRToEigenTraits specializations containing shared get()
 * implementation for matrices.
 */
template <typename OpenVRType, typename Scalar>
struct ConvertMatrixBase {
    using scalar = Scalar;

    static Scalar* get(OpenVRType& mat)
    {
        return &(mat.m[0][0]);
    }

    static Scalar const* get(OpenVRType const& mat)
    {
        return &(mat.m[0][0]);
    }
};

/**
 * Base of ConvertOpenVRToEigenTraits specializations containing shared get()
 * implementation for vectors.
 */
template <typename OpenVRType, typename Scalar>
struct ConvertVectorBase {
    using scalar = Scalar;

    static Scalar* get(OpenVRType& vec)
    {
        return &(vec.v[0]);
    }

    static Scalar const* get(OpenVRType const& vec)
    {
        return &(vec.v[0]);
    }
};

template <>
struct ConvertOpenVRToEigenTraits<vr::HmdMatrix34_t> : ConvertMatrixBase<vr::HmdMatrix34_t, float> {
    using type = Eigen::Matrix<float, 3, 4, Eigen::RowMajor>;
};

template <>
struct ConvertOpenVRToEigenTraits<vr::HmdMatrix44_t> : ConvertMatrixBase<vr::HmdMatrix44_t, float> {
    using type = Eigen::Matrix<float, 4, 4, Eigen::RowMajor>;
};

template <>
struct ConvertOpenVRToEigenTraits<vr::HmdVector3_t> : ConvertVectorBase<vr::HmdVector3_t, float> {
    using type = Eigen::Vector3f;
};

template <>
struct ConvertOpenVRToEigenTraits<vr::HmdVector3d_t> : ConvertVectorBase<vr::HmdVector3d_t, double> {
    using type = Eigen::Vector3d;
};

template <>
struct ConvertOpenVRToEigenTraits<vr::HmdVector2_t> : ConvertVectorBase<vr::HmdVector2_t, float> {
    using type = Eigen::Vector2f;
};

template <typename OpenVRType>
using GetEigenType = typename ConvertOpenVRToEigenTraits<OpenVRType>::type;

template <typename OpenVRType>
using GetEigenMap = Eigen::Map<GetEigenType<OpenVRType>>;

template <typename OpenVRType>
using GetEigenConstMap = Eigen::Map<const GetEigenType<OpenVRType>>;

template <typename OpenVRType>
using GetScalar = typename ConvertOpenVRToEigenTraits<OpenVRType>::scalar;

} // namespace detail

/**
 * Generic implementation to create an Eigen map object for a non-const OpenVR
 * type (matrix or vector).
 */
template <typename OpenVRType>
inline detail::GetEigenMap<OpenVRType> map(OpenVRType& v)
{
    return detail::GetEigenMap<OpenVRType>(detail::ConvertOpenVRToEigenTraits<OpenVRType>::get(v));
}

/**
 * Generic implementation to create an Eigen const map object for a const OpenVR
 * type (matrix or vector).
 */
template <typename OpenVRType>
inline detail::GetEigenConstMap<OpenVRType> map(OpenVRType const& v)
{
    return detail::GetEigenConstMap<OpenVRType>(detail::ConvertOpenVRToEigenTraits<OpenVRType>::get(v));
}

namespace detail {

/**
 * Helper function for HmdQuaternionMap
 */
inline Eigen::Quaterniond constructEigenQuat(vr::HmdQuaternion_t const& q)
{
    return Eigen::Quaterniond(q.w, q.x, q.y, q.z);
}

/**
 * Class wrapping a reference to a vr::HmdQuaternion_t for conversion to/from
 * Eigen.
 */
template <bool IsConst = false>
class HmdQuaternionMap {
public:
    typedef vr::HmdQuaternion_t Held;
    typedef Held& HeldRef;

    /**
     * Construct from a ref.
     */
    HmdQuaternionMap(HeldRef q) : m_held(q) {}

    /**
     * No copying
     */
    HmdQuaternionMap(HmdQuaternionMap const&) = delete;

    /**
     * OK to move
     */
    HmdQuaternionMap(HmdQuaternionMap<false>&& other) : m_held(other.m_held) {}

    /**
     * Assign from another map: OK
     */
    template <bool OtherConst>
    HmdQuaternionMap& operator=(HmdQuaternionMap<OtherConst> const& other)
    {
        if (this == &other) {
            return *this;
        }
        m_held = other.get();
        return *this;
    }

    /**
     * Assign from an Eigen Quat CRTP
     */
    template <typename Derived>
    HmdQuaternionMap& operator=(Eigen::QuaternionBase<Derived> const& q)
    {
        return doAssign(q.derived());
    }

    /**
     * Convert to Eigen quaternion
     */
    operator Eigen::Quaterniond() const
    {
        return constructEigenQuat(m_held);
    }

    /**
     * Access the ref.
     */
    HeldRef get() const
    {
        return m_held;
    }

private:
    template <typename T>
    HmdQuaternionMap& doAssign(T const& q)
    {
        m_held.x = q.x();
        m_held.y = q.y();
        m_held.z = q.z();
        m_held.w = q.w();
        return *this;
    }

    HeldRef m_held;
};

template <>
class HmdQuaternionMap<true> {
public:
    typedef vr::HmdQuaternion_t const Held;
    typedef Held& HeldRef;

    /**
     * Construct from a const ref
     */
    HmdQuaternionMap(HeldRef q) : m_held(q) {}

    /**
     * No copying
     */
    HmdQuaternionMap(HmdQuaternionMap const&) = delete;

    /**
     * OK to move from const or non-const
     */
    template <bool OtherConst>
    HmdQuaternionMap(HmdQuaternionMap<OtherConst>&& other) : m_held(other.get()) {}

    /**
     * Convert to Eigen quaternion
     */
    operator Eigen::Quaterniond() const
    {
        return constructEigenQuat(m_held);
    }

    /**
     * Access the ref.
     */
    HeldRef get() const
    {
        return m_held;
    }

private:
    HeldRef m_held;
};

} // namespace detail

/**
 * Overload to create a custom const map object for vr::HmdQuaternion_t
 */
inline detail::HmdQuaternionMap<true> map(vr::HmdQuaternion_t const& q)
{
    return detail::HmdQuaternionMap<true>(q);
}

/**
 * Overload to create a custom map object for vr::HmdQuaternion_t
 */
inline detail::HmdQuaternionMap<false> map(vr::HmdQuaternion_t& q)
{
    return detail::HmdQuaternionMap<false>(q);
}

#endif // INCLUDED_matrix_cast_h_GUID_27799A61_A16D_4B3A_AFA8_A2A4336D40AD

