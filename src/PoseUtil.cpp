/** @file
    @brief Helper functions for setting various pose parameters.

    @date 2017

    @author
    Sensics, Inc.
    <http://sensics.com>

*/

// Copyright 2017 Sensics, Inc.
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

// Internal Includes
#include "PoseUtil.h"

// Library/third-party includes
#include <Eigen/Geometry>

#include <osvr/Util/ClientReportTypesC.h> // for OSVR_VelocityState
#include <osvr/Util/EigenInterop.h>

// Standard includes
#include <algorithm>    // for std::copy

std::array<double, 3> getPoseLinearVelocity(const OSVR_VelocityState& velocity_state)
{
    if (!velocity_state.linearVelocityValid) {
        return {{0.0, 0.0, 0.0}};
    }

    // Linear velocity of the pose in meters/second
    return {{velocity_state.linearVelocity.data[0],
        velocity_state.linearVelocity.data[1],
        velocity_state.linearVelocity.data[2]}};
}

std::array<double, 3> getPoseAngularVelocity(const OSVR_VelocityState& velocity_state)
{
    if (!velocity_state.angularVelocityValid) {
        return {{0.0, 0.0, 0.0}};
    }

    // SteamVR wants the angular velocity of the pose in axis-angle
    // representation. The direction is the angle of rotation and the magnitude
    // is the angle around that axis in radians/second.
    //
    // OSVR provides an incremental quaternion, providing the incremental
    // rotation taking place due to velocity over a period of dt seconds.
    const auto angular_velocity_quat = osvr::util::fromQuat(velocity_state.angularVelocity.incrementalRotation);
    const auto dt = velocity_state.angularVelocity.dt;

    // Convert quat to axis-angle representation.
    const auto angle_axis = Eigen::AngleAxisd{angular_velocity_quat};

    // Set magnitude of axis-angle representation to velocity in radians/second.
    const auto magnitude = angle_axis.angle() / dt;
    auto angular_velocity = angle_axis.axis() * magnitude;

    return {{angular_velocity.x(), angular_velocity.y(), angular_velocity.z()}};

#if 0
    const auto pose_rotation = osvr::util::fromQuat(report->pose.rotation);
    const auto angvel_incremental_rotation = pose_rotation.inverse() * osvr::util::fromQuat(velocitystate.angularVelocity.incrementalRotation) * pose_rotation;
    // Convert invcremental rotation to angular velocity
    const auto angular_velocity = osvr::vbtracker::incRotToAngVelVec(angvel_incremental_rotation, state.dt);

    Eigen::Vector3d::Map(pose.vecAngularVelocity) = angular_velocity;
#endif
}
