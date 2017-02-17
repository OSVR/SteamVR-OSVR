/** @file
    @brief Unit tests for PoseUtil.h functions.

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
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "PoseUtil.h"

// Library/third-party includes
#include <osvr/Util/ClientReportTypesC.h> // for OSVR_VelocityState

// Standard includes
#include <iostream>
#include <array>

std::ostream& operator<<(std::ostream& os, const std::array<double, 3>& value)
{
    os << "[";
    bool first = true;
    for (const auto x : value) {
        if (!first) os << ", ";
        os << x;
        first = false;
    }
    os << "]";

    return os;
}

TEST_CASE("zero linear velocity when invalid", "[getPoseLinearVelocity]")
{
    OSVR_VelocityState velocity_state;

    {
        velocity_state.linearVelocity = {{0.0, 0.0, 0.0}};
        velocity_state.linearVelocityValid = false;

        const auto expected = std::array<double, 3>{{0.0, 0.0, 0.0}};

        CHECK(expected == getPoseLinearVelocity(velocity_state));
    }

    {
        velocity_state.linearVelocity = {{1.0, 2.0, 3.0}};
        velocity_state.linearVelocityValid = false;

        const auto expected = std::array<double, 3>{{0.0, 0.0, 0.0}};

        CHECK(expected == getPoseLinearVelocity(velocity_state));
    }
}

TEST_CASE("linear velocity when valid", "[getPoseLinearVelocity]")
{
    OSVR_VelocityState velocity_state;

    {
        velocity_state.linearVelocity = {{0.0, 0.0, 0.0}};
        velocity_state.linearVelocityValid = true;

        const auto expected = std::array<double, 3>{{0.0, 0.0, 0.0}};

        CHECK(expected == getPoseLinearVelocity(velocity_state));
    }

    {
        velocity_state.linearVelocity = {{1.0, 2.0, 3.0}};
        velocity_state.linearVelocityValid = true;

        const auto expected = std::array<double, 3>{{1.0, 2.0, 3.0}};

        CHECK(expected == getPoseLinearVelocity(velocity_state));
    }
}

TEST_CASE("zero angular velocity when invalid", "[getPoseAngularVelocity]")
{
    OSVR_VelocityState velocity_state;

    {
        velocity_state.angularVelocity = {{{0.0, 0.0, 0.0, 0.0}}, 0.0};
        velocity_state.angularVelocityValid = false;

        const auto expected = std::array<double, 3>{{0.0, 0.0, 0.0}};

        CHECK(expected == getPoseAngularVelocity(velocity_state));
    }

    {
        velocity_state.angularVelocity = {{{1.0, 2.0, 3.0, 1.0}}, 1.0};
        velocity_state.angularVelocityValid = false;

        const auto expected = std::array<double, 3>{{0.0, 0.0, 0.0}};

        CHECK(expected == getPoseAngularVelocity(velocity_state));
    }
}

TEST_CASE("angular velocity when valid", "[getPoseAngularVelocity]")
{
    OSVR_VelocityState velocity_state;

    {
        velocity_state.angularVelocity = {{{1.0, 0.0, 0.0, 0.0}}, 1.0};
        velocity_state.angularVelocityValid = true;

        const auto expected = std::array<double, 3>{{0.0, 0.0, 0.0}};

        CHECK(expected == getPoseAngularVelocity(velocity_state));
    }

    {
        // 90 degrees about the x axis per second
        velocity_state.angularVelocity = {{{std::sqrt(2.0)/2.0, std::sqrt(2.0)/2.0, 0.0, 0.0}}, 1.0};
        velocity_state.angularVelocityValid = true;

        const auto expected = std::array<double, 3>{{M_PI/2.0, 0.0, 0.0}};

        CHECK(expected == getPoseAngularVelocity(velocity_state));
    }
}

