/** @file
    @brief Header

    @date 2015

    @author
    Sensics, Inc.
    <http://sensics.com>

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
// - none

// Library/third-party includes
#include <Eigen/Geometry>
#include <vrtypes.h>

// Standard includes
// - none

//typedef Transform<double,3,Affine> Affine3d

template <typename T>
struct identity {
    typedef T type;
};

template <typename Target, typename Source>
inline Target cast(const Source& source)
{
    return cast(source, identity<Target>());
}

inline Eigen::Affine3d cast(const vr::HmdMatrix44_t& source, const identity<Eigen::Affine3d>&)
{
    Eigen::Affine3d m;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            m(i, j) = source.m[i][j];
        }
    }
    return m;
}

inline vr::HmdMatrix44_t cast(const Eigen::Affine3d& source, const identity<vr::HmdMatrix44_t>&)
{
    vr::HmdMatrix44_t m;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            m.m[i][j] = static_cast<float>(source(i, j));
        }
    }
    return m;
}

inline Eigen::Matrix4d cast(const vr::HmdMatrix44_t& source, const identity<Eigen::Matrix4d>&)
{
    Eigen::Matrix4d m;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            m(i, j) = source.m[i][j];
        }
    }
    return m;
}

inline vr::HmdMatrix44_t cast(const Eigen::Matrix4d& source, const identity<vr::HmdMatrix44_t>&)
{
    vr::HmdMatrix44_t m;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            m.m[i][j] = static_cast<float>(source(i, j));
        }
    }
    return m;
}

#endif // INCLUDED_matrix_cast_h_GUID_27799A61_A16D_4B3A_AFA8_A2A4336D40AD
