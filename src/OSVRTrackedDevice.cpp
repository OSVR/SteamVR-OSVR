/** @file
    @brief OSVR tracked device

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

// Internal Includes
#include "OSVRTrackedDevice.h"
#include "Logging.h"

#include "osvr_compiler_detection.h"
#include "make_unique.h"
#include "matrix_cast.h"
#include "ValveStrCpy.h"
#include "platform_fixes.h" // strcasecmp
#include "make_unique.h"
#include "osvr_platform.h"
#include "display/DisplayEnumerator.h"

// OpenVR includes
#include <openvr_driver.h>

// Library/third-party includes
#include <osvr/ClientKit/Display.h>
#include <osvr/Util/EigenInterop.h>
#include <osvr/Client/RenderManagerConfig.h>
#include <util/FixedLengthStringFunctions.h>
#include <osvr/RenderKit/DistortionCorrectTextureCoordinate.h>

// Standard includes
#include <cstring>
#include <ctime>
#include <string>
#include <iostream>
#include <exception>
#include <fstream>
#include <algorithm>        // for std::find

OSVRTrackedDevice::OSVRTrackedDevice(osvr::clientkit::ClientContext& context, vr::IServerDriverHost* driver_host, vr::ETrackedDeviceClass device_class) : context_(context), driverHost_(driver_host), pose_(), deviceClass_(device_class)
{
    settings_ = std::make_unique<Settings>(driverHost_->GetSettings(vr::IVRSettings_Version));
}

OSVRTrackedDevice::~OSVRTrackedDevice()
{
    driverHost_ = nullptr;
}

vr::EVRInitError OSVRTrackedDevice::Activate(uint32_t object_id)
{
    objectId_ = object_id;
    return vr::VRInitError_None;
}

void OSVRTrackedDevice::Deactivate()
{
    // do nothing
}

void OSVRTrackedDevice::PowerOff()
{
    // do nothing
}

void* OSVRTrackedDevice::GetComponent(const char* component_name_and_version)
{
    if (!strcasecmp(component_name_and_version, vr::IVRDisplayComponent_Version)) {
        return dynamic_cast<vr::IVRDisplayComponent*>(this);
    } else if (!strcasecmp(component_name_and_version, vr::IVRDriverDirectModeComponent_Version)) {
        return dynamic_cast<vr::IVRDriverDirectModeComponent*>(this);
    } else if (!strcasecmp(component_name_and_version, vr::IVRControllerComponent_Version)) {
        return dynamic_cast<vr::IVRControllerComponent*>(this);
    } else if (!strcasecmp(component_name_and_version, vr::IVRCameraComponent_Version)) {
        return dynamic_cast<vr::IVRCameraComponent*>(this);
    } else {
        OSVR_LOG(warn) << "Unknown component [" << component_name_and_version << "] requested.";
        return nullptr;
    }
}

void OSVRTrackedDevice::DebugRequest(const char* request, char* response_buffer, uint32_t response_buffer_size)
{
    // Log the requests just to see what info clients are looking for
    OSVR_LOG(debug) << "Received debug request [" << request << "] with response buffer size of " << response_buffer_size << "].";

    // make use of (from vrtypes.h) static const uint32_t k_unMaxDriverDebugResponseSize = 32768;
    // return empty string for now
    if (response_buffer_size > 0) {
        response_buffer[0] = '\0';
    }
}

vr::DriverPose_t OSVRTrackedDevice::GetPose()
{
    return pose_;
}

bool OSVRTrackedDevice::GetBoolTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* error)
{
    return GetTrackedDeviceProperty(prop, error, false);
}

float OSVRTrackedDevice::GetFloatTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* error)
{
    return GetTrackedDeviceProperty(prop, error, 0.0f);
}

int32_t OSVRTrackedDevice::GetInt32TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* error)
{
    return GetTrackedDeviceProperty(prop, error, static_cast<int32_t>(0));
}

uint64_t OSVRTrackedDevice::GetUint64TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* error)
{
    return GetTrackedDeviceProperty(prop, error, static_cast<uint64_t>(0));
}

vr::HmdMatrix34_t OSVRTrackedDevice::GetMatrix34TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* error)
{
    // Default value is identity matrix
    vr::HmdMatrix34_t default_value;
    map(default_value) = Matrix34f::Identity();
    return GetTrackedDeviceProperty(prop, error, default_value);
}

uint32_t OSVRTrackedDevice::GetStringTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, char* value, uint32_t buffer_size, vr::ETrackedPropertyError *error)
{
    uint32_t default_value = 0;

    const auto result = checkProperty(prop, value);
    if (vr::TrackedProp_Success != result) {
        if (error)
            *error = result;
        return default_value;
    }

    const auto str = GetStringTrackedDeviceProperty(prop, error);
    if (*error == vr::TrackedProp_Success) {
        if (str.size() + 1 > buffer_size) {
            *error = vr::TrackedProp_BufferTooSmall;
        } else {
            valveStrCpy(str, value, buffer_size);
        }
        return static_cast<uint32_t>(str.size()) + 1;
    }

    return 0;
}

// ------------------------------------
// Protected Methods
// ------------------------------------

std::string OSVRTrackedDevice::GetStringTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError *error)
{
    return GetTrackedDeviceProperty(prop, error, std::string{""});
}

