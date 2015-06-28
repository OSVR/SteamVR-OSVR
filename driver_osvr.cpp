/** @file
    @brief OSVR driver provider

    @date 2015

    @author
    Sensics, Inc.
    <http://sensics.com>

*/

// Copyright 2015 Sensics, Inc.
//
// All rights reserved.
//
// (Final version intended to be licensed under
// the Apache License, Version 2.0)

// Internal Includes
#include "osvr_compiler_detection.h"
#include "osvr_tracked_device.h"

#include "stringhasprefix.h"
#include "osvr_dll_export.h"

// Library/third-party includes
#include <openvr_driver.h>

#include <osvr/ClientKit/Context.h>
#include <osvr/ClientKit/Interface.h>

// Standard includes
#include <vector>
#include <cstring>
#include <string>
#include <iostream>   // for debug output

class CDriver_OSVR : public vr::IServerTrackedDeviceProvider
{
public:
    /**
     * Initializes the driver.
     *
     * This is called when the driver is first loaded.
     *
     * @param user_config_dir the absoluate path of the directory where the
     *     driver should store any user configuration files.
     * @param driver_install_dir the absolute path of the driver's root
     *     directory.
     *
     * If Init() returns anything other than \c HmdError_None the driver will be
     * unloaded.
     *
     * @returns HmdError_None on success.
     */
    virtual vr::HmdError Init(vr::IDriverLog* driver_log, vr::IServerDriverHost* driver_host, const char* user_driver_config_dir, const char* driver_install_dir) OSVR_OVERRIDE;

    /**
     * Performs any cleanup prior to the driver being unloaded.
     */
    virtual void Cleanup() OSVR_OVERRIDE;

    /**
     * Returns the number of tracked devices.
     */
    virtual uint32_t GetTrackedDeviceCount() OSVR_OVERRIDE;

    /**
     * Returns a single tracked device by its index.
     *
     * @param index the index of the tracked device to return.
     */
    virtual vr::ITrackedDeviceServerDriver* GetTrackedDeviceDriver(uint32_t index) OSVR_OVERRIDE;

    /**
     * Returns a single HMD by its name.
     *
     * @param hmd_id the C string name of the HMD.
     */
    virtual vr::ITrackedDeviceServerDriver* FindTrackedDeviceDriver(const char* id) OSVR_OVERRIDE;

    /**
     * Allows the driver do to some work in the main loop of the server.
     */
    virtual void RunFrame() OSVR_OVERRIDE;

private:
    std::vector<std::unique_ptr<OSVRTrackedDevice>> trackedDevices_;
    std::unique_ptr<osvr::clientkit::ClientContext> context_;
	vr::IDriverLog* logger_;
};

static CDriver_OSVR g_driverOSVR;

vr::HmdError CDriver_OSVR::Init(vr::IDriverLog* driver_log, vr::IServerDriverHost* driver_host, const char* user_driver_config_dir, const char* driver_install_dir)
{
	logger_ = driver_log;

	logger_->Log("CDriver_OSVR::Init() called.");
	std::cout << "CDriver_OSVR::Init() called." << std::endl;
    context_ = std::make_unique<osvr::clientkit::ClientContext>("com.osvr.SteamVR");

    const std::string display_description = context_->getStringParameter("/display");
    trackedDevices_.emplace_back(std::make_unique<OSVRTrackedDevice>(display_description, *(context_.get())));

    return vr::HmdError_None;
}

void CDriver_OSVR::Cleanup()
{
    trackedDevices_.clear();
    context_.reset();
}

uint32_t CDriver_OSVR::GetTrackedDeviceCount()
{
	std::cout << "CDriver_OSVR::GetTrackedDeviceCount(): Returning " << trackedDevices_.size() << std::endl;
    return trackedDevices_.size();
}

vr::ITrackedDeviceServerDriver* CDriver_OSVR::GetTrackedDeviceDriver(uint32_t index)
{
	if (index >= trackedDevices_.size()) {
		std::cerr << "CDriver_OSVR::GetTrackedDeviceDriver(): Index " << index << " is out of range [0.." << trackedDevices_.size() << "]." << std::endl;
		return NULL;
	}

	std::cout << "CDriver_OSVR::GetTrackedDeviceDriver(): Returning tracked device " << index << "." << std::endl;
    return trackedDevices_[index].get();
}

vr::ITrackedDeviceServerDriver* CDriver_OSVR::FindTrackedDeviceDriver(const char* id)
{
    for (auto& tracked_device : trackedDevices_) {
		if (0 == std::strcmp(id, tracked_device->GetId())) {
			std::cout << "CDriver_OSVR::FindTrackedDeviceDriver(): Returning tracked device " << id << "." << std::endl;
			return tracked_device.get();
		}
    }

	std::cerr << "CDriver_OSVR::FindTrackedDeviceDriver(): Failed to locate device named '" << id << "'." << std::endl;
    return NULL;
}

void CDriver_OSVR::RunFrame()
{
    context_->update();
}

static const char* IHmdDriverProvider_Prefix = "IHmdDriverProvider_";

OSVR_DLL_EXPORT void* TrackedDeviceDriverFactory(const char* pInterfaceName, int* pReturnCode)
{
    if (!StringHasPrefix(pInterfaceName, IHmdDriverProvider_Prefix)) {
		std::string msg = "TrackedDeviceDriverFactory(): Invalid interface. Interface name '" + std::string(pInterfaceName) + "' does not match prefix '" + std::string(IHmdDriverProvider_Prefix) + "'.";
		std::cerr << msg << std::endl;
		if (pReturnCode) {
			*pReturnCode = vr::HmdError_Init_InvalidInterface;
		}
        return NULL;
    }

    if (0 != strcmp(vr::IServerTrackedDeviceProvider_Version, pInterfaceName)) {
		std::cerr << "TrackedDeviceDriverFactory(): Interface not found. The requested interface name '" << pInterfaceName << "' does not match the tracked device provider version '" << vr::IServerTrackedDeviceProvider_Version << "'." << std::endl;
		if (pReturnCode) {
			*pReturnCode = vr::HmdError_Init_InterfaceNotFound;
		}
        return NULL;
    }

    return &g_driverOSVR;
}
