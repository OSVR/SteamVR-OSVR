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
#include "ihmddriverprovider.h"
#include "ihmddriver.h"
#include "steamvr.h"

#include "osvr_compiler_detection.h"
#include "osvr_hmd.h"

#include "stringhasprefix.h"
#include "osvr_dll_export.h"

// Library/third-party includes
#include <osvr/ClientKit/Context.h>
#include <osvr/ClientKit/Interface.h>

// Standard includes
#include <vector>
#include <cstring>

class CDriver_OSVR : public vr::IHmdDriverProvider {
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
	virtual vr::HmdError Init(const char* pchUserConfigDir, const char* pchDriverInstallDir) OSVR_OVERRIDE;

	/**
	 * Performs any cleanup prior to the driver being unloaded.
	 */
	virtual void Cleanup() OSVR_OVERRIDE;

	/**
	 * Returns the number of detect HMDs.
	 */
	virtual uint32_t GetHmdCount() OSVR_OVERRIDE;

	/**
	 * Returns a single HMD by its index.
	 *
	 * @param index the index of the HMD to return.
	 */
	virtual vr::IHmdDriver* GetHmd(uint32_t index) OSVR_OVERRIDE;

	/**
	 * Returns a single HMD by its name.
	 *
	 * @param hmd_id the C string name of the HMD.
	 */
	virtual vr::IHmdDriver* FindHmd(const char* hmd_id) OSVR_OVERRIDE;

private:
	std::vector<std::unique_ptr<OSVRHmd>> hmds_;
	std::unique_ptr<osvr::clientkit::ClientContext> context_;
	std::unique_ptr<ClientMainloopThread> client_;
};

CDriver_OSVR g_driverOSVR;

vr::HmdError CDriver_OSVR::Init(const char* pchUserConfigDir, const char* pchDriverInstallDir)
{
	context_ = std::make_unique<osvr::clientkit::ClientContext>("com.osvr.SteamVR");

	client_ = std::make_unique<ClientMainloopThread>(*context_);

	const std::string display_description = context_->getStringParameter("/display");
	osvr::clientkit::Interface head_tracker_interface = context_->getInterface("/me/head");
	hmds_.emplace_back(std::make_unique<OSVRHmd>(display_description, &head_tracker_interface));

	client_->start();

	return vr::HmdError_None;
}

void CDriver_OSVR::Cleanup()
{
	// do nothing
}

uint32_t CDriver_OSVR::GetHmdCount()
{
	return hmds_.size();
}

vr::IHmdDriver* CDriver_OSVR::GetHmd(uint32_t index)
{
	if (index >= hmds_.size())
		return NULL;

	return hmds_[index].get();
}

vr::IHmdDriver* CDriver_OSVR::FindHmd(const char* hmd_id)
{
	for (auto& hmd : hmds_) {
		if (0 == std::strcmp(hmd_id, hmd->GetId()))
			return hmd.get();
	}

	return NULL;
}


static const char* IHmdDriverProvider_Prefix = "IHmdDriverProvider_";

OSVR_DLL_EXPORT void* HmdDriverFactory(const char* pInterfaceName, int* pReturnCode)
{
	if (!StringHasPrefix(pInterfaceName, IHmdDriverProvider_Prefix)) {
		*pReturnCode = vr::HmdError_Init_InvalidInterface;
		return NULL;
	}

	if (0 != strcmp(vr::IHmdDriverProvider_Version, pInterfaceName)) {
		if (pReturnCode)
			*pReturnCode = vr::HmdError_Init_InterfaceNotFound;
		return NULL;
	}

	return &g_driverOSVR;
}

