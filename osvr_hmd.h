/** @file
    @brief Header

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

#ifndef INCLUDED_osvr_hmd_h_GUID_128E3B29_F5FC_4221_9B38_14E3F402E645
#define INCLUDED_osvr_hmd_h_GUID_128E3B29_F5FC_4221_9B38_14E3F402E645

// Internal Includes
#include "osvr_compiler_detection.h"
#include "osvr_display_configuration.h"
#include "make_unique.h"
#include "ClientMainloopThread.h"
#include "matrix_cast.h"
#include "projection_matrix.h"

// Library/third-party includes
#include <steamvr.h>
#include <ihmddriver.h>

#include <osvr/ClientKit/Context.h>
#include <osvr/ClientKit/Interface.h>

#include <Eigen/Geometry>

// Standard includes
// - none

class OSVRHmd : public vr::IHmdDriver {
public:
    OSVRHmd(const std::string& display_description, osvr::clientkit::ClientContext & context);

	// ------------------------------------
	// Management Methods
	// ------------------------------------
	/**
	 * This is called before an HMD is returned to the application. It will
	 * always be called before any display or tracking methods. Memory and
	 * processor use by the IHmdDriver object should be kept to a minimum until
	 * it is activated.  The pose listener is guaranteed to be valid until
	 * Deactivate is called, but should not be used after that point.
	 */
	virtual vr::HmdError Activate(vr::IPoseListener* pPoseListener) OSVR_OVERRIDE;

	/**
	 * This is called when The VR system is switching from this Hmd being the
	 * active display to another Hmd being the active display. The driver should
	 * clean whatever memory and thread use it can when it is deactivated.
	 */
	virtual void Deactivate() OSVR_OVERRIDE;

	/**
	 * returns the ID of this particular HMD. This value is opaque to the VR
	 * system itself, but should be unique within the driver because it will be
	 * passed back in via FindHmd
	 */
	virtual const char* GetId() OSVR_OVERRIDE;

	// ------------------------------------
	// Display Methods
	// ------------------------------------

	/**
	 * Size and position that the window needs to be on the VR display.
	 */
	virtual void GetWindowBounds(int32_t* pnX, int32_t* pnY, uint32_t* pnWidth, uint32_t* pnHeight) OSVR_OVERRIDE;

	/**
	 * Suggested size for the intermediate render target that the distortion
	 * pulls from.
	 */
	virtual void GetRecommendedRenderTargetSize(uint32_t* pnWidth, uint32_t* pnHeight) OSVR_OVERRIDE;

	/**
	 * Gets the viewport in the frame buffer to draw the output of the distortion
	 * into
	 */
	virtual void GetEyeOutputViewport(vr::Hmd_Eye eEye, uint32_t* pnX, uint32_t* pnY, uint32_t* pnWidth, uint32_t* pnHeight) OSVR_OVERRIDE;

	/**
	 * The components necessary to build your own projection matrix in case your
	 * application is doing something fancy like infinite Z
	 */
	virtual void GetProjectionRaw(vr::Hmd_Eye eEye, float* pfLeft, float* pfRight, float* pfTop, float* pfBottom) OSVR_OVERRIDE;

	/**
	 * Returns the transform between the view space and eye space. Eye space is
	 * the per-eye flavor of view space that provides stereo disparity. Instead
	 * of Model * View * Projection the model is Model * View * Eye *
	 * Projection.  Normally View and Eye will be multiplied together and
	 * treated as View in your application. 
	 */
	virtual vr::HmdMatrix44_t GetEyeMatrix(vr::Hmd_Eye eEye) OSVR_OVERRIDE;

	/**
	 * Returns the result of the distortion function for the specified eye and
	 * input UVs. UVs go from 0,0 in the upper left of that eye's viewport and
	 * 1,1 in the lower right of that eye's viewport.
	 */
	virtual vr::DistortionCoordinates_t ComputeDistortion(vr::Hmd_Eye eEye, float fU, float fV) OSVR_OVERRIDE;

	// -----------------------------------
	// Administrative Methods
	// -----------------------------------

	/**
	 * Returns the model number of this HMD
	 */
	virtual const char* GetModelNumber() OSVR_OVERRIDE;

	/**
	 * Returns the serial number of this HMD
	 */
	virtual const char* GetSerialNumber() OSVR_OVERRIDE;

protected:
	const std::string m_DisplayDescription;
    osvr::clientkit::ClientContext & m_Context;
    osvr::clientkit::Interface m_TrackerInterface;
	std::unique_ptr<OSVRDisplayConfiguration> m_DisplayConfiguration;
	vr::IPoseListener* m_PoseListener;
};

struct CallbackData {
	vr::IPoseListener* poseListener;
	vr::IHmdDriver* hmdDriver;
};

void HmdTrackerCallback(void* userdata, const OSVR_TimeValue* timestamp, const OSVR_PoseReport* report);

OSVRHmd::OSVRHmd(const std::string& display_description, osvr::clientkit::ClientContext & context) : m_DisplayDescription(display_description), m_Context(context), m_DisplayConfiguration(nullptr), m_PoseListener(nullptr)
{
	// do nothing
}

vr::HmdError OSVRHmd::Activate(vr::IPoseListener* pPoseListener)
{
	m_PoseListener = pPoseListener;

	// Retrieve display parameters
	m_DisplayConfiguration = std::make_unique<OSVRDisplayConfiguration>(m_DisplayDescription);

	// Register tracker callback
    if (m_TrackerInterface.notEmpty()) {
        m_TrackerInterface.free();
    }
    m_TrackerInterface = m_Context.getInterface("/me/head");
	m_TrackerInterface.registerCallback(&HmdTrackerCallback, this);

	return vr::HmdError_None;
}

void OSVRHmd::Deactivate()
{
    /// Have to force freeing here 
    if (m_TrackerInterface.notEmpty()) {
        m_TrackerInterface.free();
    }
	m_PoseListener = NULL;
}

const char* OSVRHmd::GetId()
{
	/// @todo When available, return the actual unique ID of the HMD
	return "OSVR HMD";
}

void OSVRHmd::GetWindowBounds(int32_t* pnX, int32_t* pnY, uint32_t* pnWidth, uint32_t* pnHeight)
{
	*pnX = m_DisplayConfiguration->getDisplayLeft();
	*pnY = m_DisplayConfiguration->getDisplayTop();
	*pnWidth = m_DisplayConfiguration->getDisplayWidth();
	*pnHeight = m_DisplayConfiguration->getDisplayHeight();
}

void OSVRHmd::GetRecommendedRenderTargetSize(uint32_t* pnWidth, uint32_t* pnHeight)
{
	*pnWidth = m_DisplayConfiguration->getDisplayWidth();
	*pnHeight = m_DisplayConfiguration->getDisplayHeight();
}

void OSVRHmd::GetEyeOutputViewport(vr::Hmd_Eye eEye, uint32_t* pnX, uint32_t* pnY, uint32_t* pnWidth, uint32_t* pnHeight)
{
	switch (m_DisplayConfiguration->getDisplayMode()) {
	case OSVRDisplayConfiguration::HORIZONTAL_SIDE_BY_SIDE:
		*pnWidth = m_DisplayConfiguration->getDisplayWidth() / 2;
		*pnHeight = m_DisplayConfiguration->getDisplayHeight();
		*pnX = (vr::Eye_Left == eEye) ? 0 : *pnWidth;
		*pnY = 0;
		break;
	case OSVRDisplayConfiguration::VERTICAL_SIDE_BY_SIDE:
		*pnWidth = m_DisplayConfiguration->getDisplayWidth();
		*pnHeight = m_DisplayConfiguration->getDisplayHeight() / 2;
		*pnX = 0;
		*pnY = (vr::Eye_Left == eEye) ? 0 : *pnHeight;
		break;
	case OSVRDisplayConfiguration::FULL_SCREEN:
		*pnWidth = m_DisplayConfiguration->getDisplayWidth();
		*pnHeight = m_DisplayConfiguration->getDisplayHeight();
		*pnX = 0;
		*pnY = 0;
		break;
	default:
		*pnWidth = m_DisplayConfiguration->getDisplayWidth();
		*pnHeight = m_DisplayConfiguration->getDisplayHeight();
		*pnX = 0;
		*pnY = 0;
		std::cerr << "ERROR: Unexpected display mode type: " << m_DisplayConfiguration->getDisplayMode() << ".\n";
	}
}

void OSVRHmd::GetProjectionRaw(vr::Hmd_Eye eEye, float* pfLeft, float* pfRight, float* pfTop, float* pfBottom)
{
	// Projection matrix centered between the eyes
	const double z_near = 0.1;
	const double z_far = 100.0;
	const Eigen::Matrix4d center = make_projection_matrix(m_DisplayConfiguration->getHorizontalFOVRadians(), m_DisplayConfiguration->getFOVAspectRatio(), z_near, z_far);

	// Translate projection matrix to left or right eye
	double eye_translation = m_DisplayConfiguration->getIPDMeters() / 2.0;
	if (vr::Eye_Left == eEye) {
		eye_translation *= -1.0;
	}
	const Eigen::Affine3d translation(Eigen::Translation3d(Eigen::Vector3d(eye_translation, 0.0, 0.0)));

	const Eigen::Matrix4d eye_matrix = translation * center;

	const double near = eye_matrix(2, 3) / (eye_matrix(2, 2) - 1.0);
	const double far  = eye_matrix(2, 3) / (eye_matrix(3, 3) + 1.0);
	*pfBottom = near * (eye_matrix(1, 2) - 1.0) / eye_matrix(1, 1);
	*pfTop    = near * (eye_matrix(1, 2) + 1.0) / eye_matrix(1, 1);
	*pfLeft   = near * (eye_matrix(0, 2) - 1.0) / eye_matrix(0, 0);
	*pfRight  = near * (eye_matrix(0, 2) + 1.0) / eye_matrix(0, 0);
}

vr::HmdMatrix44_t OSVRHmd::GetEyeMatrix(vr::Hmd_Eye eEye)
{
	// Rotate per the display configuration
	const double horiz_fov = m_DisplayConfiguration->getHorizontalFOVRadians();
	const double overlap = m_DisplayConfiguration->getOverlapPercent() * horiz_fov;
	double angle = (horiz_fov - overlap) / 2.0;
	if (vr::Eye_Right == eEye) {
		angle *= -1.0;
	}
	const Eigen::Affine3d rotation = Eigen::Affine3d(Eigen::AngleAxisd(angle, Eigen::Vector3d(0, 1, 0)));

	// Translate along x-axis by half the interpupillary distance
	double eye_translation = m_DisplayConfiguration->getIPDMeters() / 2.0;
	if (vr::Eye_Left == eEye) {
		eye_translation *= -1.0;
	}
	const Eigen::Affine3d translation(Eigen::Translation3d(Eigen::Vector3d(eye_translation, 0.0, 0.0)));

	// Eye matrix
	const Eigen::Matrix4d mat = (translation * rotation).matrix();

	return cast<vr::HmdMatrix44_t>(mat);
}

vr::DistortionCoordinates_t OSVRHmd::ComputeDistortion(vr::Hmd_Eye eEye, float fU, float fV)
{
    /// @todo FIXME Compute distortion using display configuration data
	vr::DistortionCoordinates_t coords;
	coords.rfRed[0] = 0.0;
	coords.rfRed[1] = 0.0;
	coords.rfBlue[0] = 0.0;
	coords.rfBlue[1] = 0.0;
	coords.rfGreen[0] = 0.0;
	coords.rfGreen[1] = 0.0;
	return coords;
}

const char* OSVRHmd::GetModelNumber()
{
	/// @todo When available, return the actual model number of the HMD
	return "OSVR HMD";
}

const char* OSVRHmd::GetSerialNumber()
{
    /// @todo When available, return the actual serial number of the HMD
	return "0";
}

void HmdTrackerCallback(void* userdata, const OSVR_TimeValue* timestamp, const OSVR_PoseReport* report)
{
	CallbackData* callback_data = static_cast<CallbackData*>(userdata);
	if (!callback_data->poseListener)
		return;

	vr::DriverPose_t pose;
	pose.poseTimeOffset = 0; // close enough
	pose.defaultPredictionTime = 0;

	for (int i = 0; i < 3; ++i) {
		pose.vecWorldFromDriverTranslation[i] = 0.0;
		pose.vecDriverFromHeadTranslation[i] = 0.0;
	}

	pose.qWorldFromDriverRotation.w = 1;
	pose.qWorldFromDriverRotation.x = 0;
	pose.qWorldFromDriverRotation.y = 0;
	pose.qWorldFromDriverRotation.z = 0;

	pose.qDriverFromHeadRotation.w = 1;
	pose.qDriverFromHeadRotation.x = 0;
	pose.qDriverFromHeadRotation.y = 0;
	pose.qDriverFromHeadRotation.z = 0;

	// Position
	for (int i = 0; i < 3; ++i) {
		pose.vecPosition[i] = report->pose.translation.data[0];
	}

	// Position velocity and acceleration are not currently consistently provided
	for (int i = 0; i < 3; ++i) {
		pose.vecVelocity[i] = 0.0;
		pose.vecAcceleration[i] = 0.0;
	}

	// Orientation
	pose.qRotation.w = osvrQuatGetW(&(report->pose.rotation));
	pose.qRotation.x = osvrQuatGetX(&(report->pose.rotation));
	pose.qRotation.y = osvrQuatGetY(&(report->pose.rotation));
	pose.qRotation.z = osvrQuatGetZ(&(report->pose.rotation));

	// Angular velocity and acceleration are not currently consistently provided
	for (int i = 0; i < 3; ++i) {
		pose.vecAngularVelocity[i] = 0.0;
		pose.vecAngularAcceleration[i] = 0.0;
	}

	pose.result = vr::TrackingResult_Running_OK;
	pose.poseIsValid = true;
	pose.willDriftInYaw = true;
	pose.shouldApplyHeadModel = true;

	callback_data->poseListener->PoseUpdated(callback_data->hmdDriver, pose);
}

#endif // INCLUDED_osvr_hmd_h_GUID_128E3B29_F5FC_4221_9B38_14E3F402E645

