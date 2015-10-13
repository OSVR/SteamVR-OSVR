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
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef INCLUDED_projection_matrix_h_GUID_7B4C45E9_A1D7_4D41_9F4E_68B5730AEA37
#define INCLUDED_projection_matrix_h_GUID_7B4C45E9_A1D7_4D41_9F4E_68B5730AEA37

// Internal Includes
// - none

// Library/third-party includes
#include <Eigen/Geometry>

// Standard includes
#include <cmath>

Eigen::Matrix4d make_projection_matrix(double vertical_fov, double aspect_ratio, double near_plane, double far_plane)
{
    // References:
    // http://www.scratchapixel.com/lessons/3d-basic-rendering/perspective-and-orthographic-projection-matrix/building-basic-perspective-projection-matrix
    // http://schabby.de/projection-matrix/
    const double y_scale = 1.0 / std::tan(vertical_fov / 2.0);
    const double x_scale = y_scale / aspect_ratio;

    const double alpha = -(far_plane + near_plane) / (near_plane - far_plane);
    const double beta = (2 * near_plane * far_plane) / (near_plane - far_plane);

    Eigen::Matrix4d mat;
    mat << x_scale, 0.0, 0.0, 0.0,
        0.0, y_scale, 0.0, 0.0,
        0.0, 0.0, alpha, -1.0,
        0.0, 0.0, beta, 0.0;

    return mat;
}

#endif // INCLUDED_projection_matrix_h_GUID_7B4C45E9_A1D7_4D41_9F4E_68B5730AEA37

