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

#ifndef INCLUDED_PropertyMap_h_GUID_D9C1D812_9B04_485C_BD5D_4316974D0043
#define INCLUDED_PropertyMap_h_GUID_D9C1D812_9B04_485C_BD5D_4316974D0043

// Internal Includes
// - none

// Library/third-party includes
#include <boost/variant.hpp>

#include <openvr_driver.h>

// Standard includes
#include <cstdint>
#include <string>
#include <map>
#include <boost/container/flat_map.hpp>

using Property = boost::variant<bool, float, int32_t, uint64_t, vr::HmdMatrix34_t, uint32_t, std::string>;

using PropertyMap = boost::container::flat_map<vr::ETrackedDeviceProperty, Property>;

#endif // INCLUDED_PropertyMap_h_GUID_D9C1D812_9B04_485C_BD5D_4316974D0043

