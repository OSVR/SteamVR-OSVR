/** @file
    @brief Standalone program for testing a SteamVR driver.

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

// Internal Includes
#include "osvr_compiler_detection.h"    // for OSVR_OVERRIDE
#include "ServerDriver_OSVR.h"          // for ServerDriver_OSVR
#include "driver_osvr.h"                // for factories

// Library/third-party includes
#include <openvr_driver.h>              // for vr::IDriverLog

// Standard includes
#include <cstdlib> // for EXIT_SUCCESS
#include <iostream>

namespace vr {

/**
 * Log messages to the console by default.
 */
class Logger : public IVRDriverLog {
public:
    void Log(const char* message) OSVR_OVERRIDE
    {
        std::cout << message << std::endl;
    }

    virtual ~Logger()
    {
        // do nothing
    }
};

class ServerDriverHost : public IVRServerDriverHost
{
public:
    virtual bool TrackedDeviceAdded(const char *pchDeviceSerialNumber, ETrackedDeviceClass eDeviceClass, ITrackedDeviceServerDriver *pDriver) OSVR_OVERRIDE
    {
        std::cout << "Detected tracker: " << pchDeviceSerialNumber << "." << std::endl;
        trackerCount_++;
        return true;
    }

    virtual void TrackedDevicePoseUpdated(uint32_t unWhichDevice, const DriverPose_t & newPose, uint32_t unPoseStructSize) OSVR_OVERRIDE { }
    virtual void TrackedDevicePropertiesChanged(uint32_t unWhichDevice) OSVR_OVERRIDE { }
    virtual void VsyncEvent(double vsyncTimeOffsetSeconds) OSVR_OVERRIDE { }
    virtual void TrackedDeviceButtonPressed(uint32_t unWhichDevice, EVRButtonId eButtonId, double eventTimeOffset) OSVR_OVERRIDE { }
    virtual void TrackedDeviceButtonUnpressed(uint32_t unWhichDevice, EVRButtonId eButtonId, double eventTimeOffset) OSVR_OVERRIDE { }
    virtual void TrackedDeviceButtonTouched(uint32_t unWhichDevice, EVRButtonId eButtonId, double eventTimeOffset) OSVR_OVERRIDE { }
    virtual void TrackedDeviceButtonUntouched(uint32_t unWhichDevice, EVRButtonId eButtonId, double eventTimeOffset) OSVR_OVERRIDE { }
    virtual void TrackedDeviceAxisUpdated(uint32_t unWhichDevice, uint32_t unWhichAxis, const VRControllerAxis_t & axisState) OSVR_OVERRIDE { }
    virtual void MCImageUpdated() OSVR_OVERRIDE { }
    virtual IVRSettings *GetSettings(const char *pchInterfaceVersion) OSVR_OVERRIDE { return nullptr; }
    virtual void PhysicalIpdSet(uint32_t unWhichDevice, float fPhysicalIpdMeters) OSVR_OVERRIDE { }
    virtual void ProximitySensorState(uint32_t unWhichDevice, bool bProximitySensorTriggered) OSVR_OVERRIDE { }
    virtual void VendorSpecificEvent(uint32_t unWhichDevice, vr::EVREventType eventType, const VREvent_Data_t & eventData, double eventTimeOffset) OSVR_OVERRIDE { }
    virtual bool IsExiting() OSVR_OVERRIDE { return false; }
    virtual bool PollNextEvent(VREvent_t *pEvent, uint32_t uncbVREvent) OSVR_OVERRIDE { return false; }

    uint32_t GetTrackedDeviceCount() { return trackerCount_; }

private:
    uint32_t trackerCount_ = 0;
};

class Properties : public IVRProperties
{
public:
    virtual ETrackedPropertyError ReadPropertyBatch(PropertyContainerHandle_t ulContainerHandle, PropertyRead_t *pBatch, uint32_t unBatchEntryCount) OSVR_OVERRIDE { return TrackedProp_Success; }
    virtual ETrackedPropertyError WritePropertyBatch(PropertyContainerHandle_t ulContainerHandle, PropertyWrite_t *pBatch, uint32_t unBatchEntryCount) OSVR_OVERRIDE { return TrackedProp_Success; }
    virtual const char *GetPropErrorNameFromEnum(ETrackedPropertyError error) OSVR_OVERRIDE { return nullptr; }
    virtual PropertyContainerHandle_t TrackedDeviceToPropertyContainer(TrackedDeviceIndex_t nDevice) OSVR_OVERRIDE { return 0; }

};

class Settings : public IVRSettings
{
public:
    virtual const char *GetSettingsErrorNameFromEnum(EVRSettingsError eError) OSVR_OVERRIDE { return nullptr; }
    virtual bool Sync(bool bForce = false, EVRSettingsError *peError = nullptr) OSVR_OVERRIDE { return false; }
    virtual void SetBool(const char *pchSection, const char *pchSettingsKey, bool bValue, EVRSettingsError *peError = nullptr) OSVR_OVERRIDE { }
    virtual void SetInt32(const char *pchSection, const char *pchSettingsKey, int32_t nValue, EVRSettingsError *peError = nullptr) OSVR_OVERRIDE { }
    virtual void SetFloat(const char *pchSection, const char *pchSettingsKey, float flValue, EVRSettingsError *peError = nullptr) OSVR_OVERRIDE { }
    virtual void SetString(const char *pchSection, const char *pchSettingsKey, const char *pchValue, EVRSettingsError *peError = nullptr) OSVR_OVERRIDE { }

    virtual bool GetBool(const char *pchSection, const char *pchSettingsKey, EVRSettingsError *peError = nullptr) OSVR_OVERRIDE
    {
        if (peError)
            *peError = VRSettingsError_UnsetSettingHasNoDefault;
        return false;
    }
    virtual int32_t GetInt32(const char *pchSection, const char *pchSettingsKey, EVRSettingsError *peError = nullptr) OSVR_OVERRIDE
    {
        if (peError)
            *peError = VRSettingsError_UnsetSettingHasNoDefault;
        return 0;
    }
    virtual float GetFloat(const char *pchSection, const char *pchSettingsKey, EVRSettingsError *peError = nullptr) OSVR_OVERRIDE
    {
        if (peError)
            *peError = VRSettingsError_UnsetSettingHasNoDefault;
        return 0.0f;
    }
    virtual void GetString(const char *pchSection, const char *pchSettingsKey, VR_OUT_STRING() char *pchValue, uint32_t unValueLen, EVRSettingsError *peError = nullptr) OSVR_OVERRIDE
    {
        if (peError)
            *peError = VRSettingsError_UnsetSettingHasNoDefault;
    }

    virtual void RemoveSection(const char *pchSection, EVRSettingsError *peError = nullptr) OSVR_OVERRIDE { }
    virtual void RemoveKeyInSection(const char *pchSection, const char *pchSettingsKey, EVRSettingsError *peError = nullptr) OSVR_OVERRIDE { }
};

class DriverContext : public vr::IVRDriverContext
{
public:
    /** Returns the requested interface. If the interface was not available it will return NULL and fill
    * out the error. */
    virtual void *GetGenericInterface(const char *pchInterfaceVersion, EVRInitError *peError = nullptr) OSVR_OVERRIDE
    {
        if (0 == strcmp(IVRDriverLog_Version, pchInterfaceVersion))
        {
            return &logger;
        }
        if (0 == strcmp(IVRServerDriverHost_Version, pchInterfaceVersion))
        {
            return &serverHost;
        }
        if (0 == strcmp(IVRProperties_Version, pchInterfaceVersion))
        {
            return &properties;
        }
        if (0 == strcmp(IVRSettings_Version, pchInterfaceVersion))
        {
            return &settings;
        }

        if (peError)
            *peError = VRInitError_Init_InterfaceNotFound;

        return nullptr;
    }

    /** Returns the property container handle for this driver */
    virtual DriverHandle_t GetDriverHandle() OSVR_OVERRIDE { return 0; };

    Logger logger;
    ServerDriverHost serverHost;
    Properties properties;
    Settings settings;
};

} // namespace vr

int main(int argc, char* argv[])
{
    // Instantiate the tracker driver
    std::cout << "Instantiating tracker driver..." << std::endl;
    int driver_init_return = 0;
    auto tracker_driver = static_cast<ServerDriver_OSVR*>(TrackedDeviceDriverFactory(vr::IServerTrackedDeviceProvider_Version, &driver_init_return));
    if (!tracker_driver) {
        std::cerr << "! Error creating tracker driver. ";
        switch (driver_init_return) {
        case vr::VRInitError_Init_InvalidInterface:
            std::cerr << "Invalid interface.";
            break;
        case vr::VRInitError_Init_InterfaceNotFound:
            std::cerr << "Interface not found.";
            break;
        default:
            std::cerr << "Unexpected error: " << driver_init_return << ".";
            break;
        }
        std::cerr << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << " - Tracker driver instantiated successfully." << std::endl;

    vr::DriverContext context;

    // Initialize the tracker driver
    std::cout << "Initializing the tracker driver..." << std::endl;
    vr::EVRInitError error = tracker_driver->Init(&context);
    if (vr::VRInitError_None != error) {
        std::cerr << "! Error initializing tracker driver: " << error << "." << std::endl;
        tracker_driver->Cleanup();
        return EXIT_FAILURE;
    }
    std::cout << " - Tracker driver initialized successfully." << std::endl;

    // Get the number of trackers
    const auto tracker_count = context.serverHost.GetTrackedDeviceCount();
    std::cout << "Detected " << tracker_count << " trackers." << std::endl;
    if (tracker_count < 1) {
        std::cerr << "! No trackers were detected." << std::endl;
        tracker_driver->Cleanup();
        return EXIT_FAILURE;
    }

    tracker_driver->Cleanup();

    return EXIT_SUCCESS;
}
