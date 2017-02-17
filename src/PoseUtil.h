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

#ifndef INCLUDED_PoseUtil_h_GUID_42F788D5_00B8_415E_B7BC_CBBD37211480
#define INCLUDED_PoseUtil_h_GUID_42F788D5_00B8_415E_B7BC_CBBD37211480

// Internal Includes
// - none

// Library/third-party includes
// - none

// Standard includes
#include <array>        // for std::array

struct OSVR_VelocityState;

/**
 * Get the linear velocity of the pose.
 *
 * @param velocity_state The velocity state received from the tracker.
 *
 * @return The linear velocity of the pose or (0, 0, 0) if it's invalid.
 */
std::array<double, 3> getPoseLinearVelocity(const OSVR_VelocityState& velocity_state);

/**
 * Get the angular velocity of the pose.
 *
 * @param velocity_state The velocity state received from the tracker.
 *
 * @return The angular velocity of the pose or (0, 0, 0) if it's invalid.
 */
std::array<double, 3> getPoseAngularVelocity(const OSVR_VelocityState& velocity_state);

#endif // INCLUDED_PoseUtil_h_GUID_42F788D5_00B8_415E_B7BC_CBBD37211480

