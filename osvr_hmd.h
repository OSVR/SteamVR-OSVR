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

// Library/third-party includes
#include <steamvr.h>
#include <ihmddriver.h>

// Standard includes
// - none

class OSVRHmd : public vr::IHmdDriver {
public:
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
     * Gets the viewport in the frame buffer to draw the output of the disortion
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

};

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
vr::HmdError OSVRHmd::Activate(vr::IPoseListener* pPoseListener)
{
	//m_pPoseListener = pPoseListener;

	// Retrieve display parameters
	osvr::clientkit::ClientContext context("com.osvr.SteamVR");
	const std::string display_description = context.getStringParameter("/display");
	m_DisplayConfiguration = OSVRDisplayConfiguration(display_description);

	// Register tracker callback
	const std::string tracker_interface = context.getInterface("/me/head");
	tracker_interface.registerCallback(&OSVRHmd::HmdTrackerCallback, NULL);

	return HmdError_None;
}

/**
 * This is called when The VR system is switching from this Hmd being the
 * active display to another Hmd being the active display. The driver should
 * clean whatever memory and thread use it can when it is deactivated.
 */
void OSVRHmd::Deactivate()
{
	m_pPoseListener = NULL;
}


/**
 * Returns the ID of this particular HMD. This value is opaque to the VR
 * system itself, but should be unique within the driver because it will be
 * passed back in via FindHmd
 */
const char* OSVRHmd::GetId()
{
	return m_sensorInfo.SerialNumber; // FIXME
}


// ------------------------------------
// Display Methods
// ------------------------------------

/**
 * Size and position that the window needs to be on the VR display.
 */
void OSVRHmd::GetWindowBounds(int32_t* pnX, int32_t* pnY, uint32_t* pnWidth, uint32_t* pnHeight)
{
	// KMG: DONE!
	*pnX = m_DisplayConfiguration.getDisplayLeft();
	*pnY = m_DisplayConfiguration.getDisplayTop();
	*pnWidth = m_DisplayConfiguration.getDisplayWidth();
	*pnHeight = m_DisplayConfiguration.getDisplayHeight();
}


/**
 * Suggested size for the intermediate render target that the distortion
 * pulls from.
 */
void OSVRHmd::GetRecommendedRenderTargetSize(uint32_t* pnWidth, uint32_t* pnHeight)
{
	// KMG: DONE!
	*pnWidth = m_DisplayConfiguration.getDisplayWidth();
	*pnHeight = m_DisplayConfiguration.getDisplayHeight();
}


/**
 * Gets the viewport in the frame buffer to draw the output of the disortion
 * into
 */
void OSVRHmd::GetEyeOutputViewport(vr::Hmd_Eye eEye, uint32_t* pnX, uint32_t* pnY, uint32_t* pnWidth, uint32_t* pnHeight)
{
	// KMG: DONE!
	switch (m_DisplayMode) {
	case HORIZONTAL_SIDE_BY_SIDE:
		*pnWidth = m_DisplayConfiguration.getDisplayWidth() / 2;
		*pnHeight = m_DisplayConfiguration.getDisplayHeight();
		*pnX = (vr::Eye_Left == eEye) ? 0 : *pnWidth;
		*pnY = 0;
		break;
	case VERTICAL_SIDE_BY_SIDE:
		*pnWidth = m_DisplayConfiguration.getDisplayWidth();
		*pnHeight = m_DisplayConfiguration.getDisplayHeight() / 2;
		*pnX = 0;
		*pnY = (vr::Eye_Left == eEye) ? 0 : *pnHeight;
		break;
	case FULL_SCREEN:
		*pnWidth = m_DisplayConfiguration.getDisplayWidth();
		*pnHeight = m_DisplayConfiguration.getDisplayHeight();
		*pnX = 0;
		*pnY = 0;
		break;
	default:
		*pnWidth = m_DisplayConfiguration.getDisplayWidth();
		*pnHeight = m_DisplayConfiguration.getDisplayHeight();
		*pnX = 0;
		*pnY = 0;
		std::cerr << "ERROR: Unexpected display mode type: " << m_DisplayMode << ".\n";
	}
}


/**
 * The components necessary to build your own projection matrix in case your
 * application is doing something fancy like infinite Z
 */
void OSVRHmd::GetProjectionRaw(vr::Hmd_Eye eEye, float* pfLeft, float* pfRight, float* pfTop, float* pfBottom)
{
	// FIXME ?
	const float (*p)[4] = mat.M;

	float dx  = 2.0f / p[0][0];
	float sx  = p[0][2] * dx;
	*pfRight  = (sx + dx) * 0.5f;
	*pfLeft   = sx - *pfRight;

	float dy   = 2.0f / p[1][1];
	float sy   = p[1][2] * dy;
	*pfBottom  = (sy + dy) * 0.5f;
	*pfTop     = sy - *pfBottom;
}


/**
 * Returns the transform between the view space and eye space. Eye space is
 * the per-eye flavor of view space that provides stereo disparity. Instead
 * of Model * View * Projection the model is Model * View * Eye *
 * Projection.  Normally View and Eye will be multiplied together and
 * treated as View in your application. 
 */
vr::HmdMatrix44_t OSVRHmd::GetEyeMatrix(vr::Hmd_Eye eEye)
{
	// KMG: DONE!
	HmdMatrix44_t mat;
	HmdMatrix_SetIdentity( &mat );
	if( eEye == vr::Eye_Left )
		mat.m[0][3] = -m_DisplayConfiguration.getIPDMeters() / 2.0;
	else
		mat.m[0][3] = m_DisplayConfiguration.getIPDMeters() / 2.0;
	return mat;
}


/**
 * Returns the result of the distortion function for the specified eye and
 * input UVs. UVs go from 0,0 in the upper left of that eye's viewport and
 * 1,1 in the lower right of that eye's viewport.
 */
vr::DistortionCoordinates_t OSVRHmd::ComputeDistortion(vr::Hmd_Eye eEye, float fU, float fV)
{
	// FIXME
	OVR::Util::Render::DistortionConfig distConfig = m_stereoConfig.GetDistortionConfig();
	OVR::Util::Render::Viewport vp = m_stereoConfig.GetFullViewport();

	vp.w = m_hmdInfo.HResolution/2;

	float fXCenterOffset = distConfig.XCenterOffset;
	float fUOutOffset = 0;
	if( eEye == vr::Eye_Right )
	{
		fXCenterOffset = -fXCenterOffset;

		vp.x = m_hmdInfo.HResolution/2;
		fUOutOffset = -0.5f;
	}

	float x = (float)vp.x / (float)m_hmdInfo.HResolution;
	float y = (float)vp.y / (float)m_hmdInfo.VResolution;
	float w = (float)vp.w / (float)m_hmdInfo.HResolution;
	float h = (float)vp.h / (float)m_hmdInfo.VResolution;

	// pre-munge the UVs the way that Oculus does in their vertex shader
	float fMungedU = fU * w + x;
	float fMungedV = fV * h + y;

	float as = (float)vp.w / (float)vp.h;

	float fLensCenterX = x + (w + fXCenterOffset * 0.5f)*0.5f;
	float fLensCenterY = y + h *0.5f;

	float scaleFactor = 1.0f / distConfig.Scale;

	float scaleU = (w/2)*scaleFactor; 
	float scaleV = (h/2)*scaleFactor * as;

	float scaleInU = 2/w;
	float scaleInV = (2/h)/as;

	DistortionCoordinates_t coords;
	float thetaU = ( fMungedU - fLensCenterX ) *scaleInU; // Scales to [-1, 1]
	float thetaV = ( fMungedV - fLensCenterY ) * scaleInV; // Scales to [-1, 1]

	float  rSq= thetaU * thetaU + thetaV * thetaV;
	float theta1U = thetaU * (distConfig.K[0]+ distConfig.K[1] * rSq +
		distConfig.K[2] * rSq * rSq + distConfig.K[3] * rSq * rSq * rSq);
	float theta1V = thetaV * (distConfig.K[0]+ distConfig.K[1] * rSq +
		distConfig.K[2] * rSq * rSq + distConfig.K[3] * rSq * rSq * rSq);

	// The x2 on U scale of the output coords is because the input texture in the VR API is 
	// single eye instead of two-eye like is standard in the Rift samples.

	// Do blue scale and lookup
	float thetaBlueU = theta1U * (distConfig.ChromaticAberration[2] + distConfig.ChromaticAberration[3] * rSq);
	float thetaBlueV = theta1V * (distConfig.ChromaticAberration[2] + distConfig.ChromaticAberration[3] * rSq);
	coords.rfBlue[0] = 2.f * (fLensCenterX + scaleU * thetaBlueU + fUOutOffset );
	coords.rfBlue[1] = fLensCenterY + scaleV * thetaBlueV;

	// Do green lookup (no scaling).
	coords.rfGreen[0] = 2.f * (fLensCenterX + scaleU * theta1U + fUOutOffset);
	coords.rfGreen[1] = fLensCenterY + scaleV * theta1V;

	// Do red scale and lookup.
	float thetaRedU = theta1U * (distConfig.ChromaticAberration[0] + distConfig.ChromaticAberration[1] * rSq);
	float thetaRedV = theta1V * (distConfig.ChromaticAberration[0] + distConfig.ChromaticAberration[1] * rSq);
	coords.rfRed[0] = 2.f * (fLensCenterX + scaleU * thetaRedU + fUOutOffset);
	coords.rfRed[1] = fLensCenterY + scaleV * thetaRedV;

	//float r = sqrtf( rSq );
	//if( r > 0.19f && r < 0.2f )
	//{
	//	memset( &coords, 0, sizeof(coords) );
	//}

	return coords;
}


// -----------------------------------
// Administrative Methods
// -----------------------------------

/**
 * Returns the model number of this HMD
 */
const char* OSVRHmd::GetModelNumber()
{
	// FIXME
	return m_hmdInfo.ProductName;
}


/**
 * Returns the serial number of this HMD
 */
const char* OSVRHmd::GetSerialNumber()
{
	// FIXME
	return m_sensorInfo.SerialNumber;
}

void OSVRHmd::HmdTrackerCallback(void* /*userdata*/, const OSVR_TimeValue* /*timestamp*/, const OSVR_PoseReport* report) {
{
	if (!m_pPoseListener)
		return;

	std::cout << "Got POSE report: Position = ("
		<< report->pose.translation.data[0] << ", "
		<< report->pose.translation.data[1] << ", "
		<< report->pose.translation.data[2] << "), orientation = ("
		<< osvrQuatGetW(&(report->pose.rotation)) << ", ("
		<< osvrQuatGetX(&(report->pose.rotation)) << ", "
		<< osvrQuatGetY(&(report->pose.rotation)) << ", "
		<< osvrQuatGetZ(&(report->pose.rotation)) << "))" << std::endl;


	vr::DriverPose_t pose;
	pose.poseTimeOffset = 0;
	pose.defaultPredictionTime = 0;

	// Set WorldFromDriver and DriverFromHead poses to identity
	for (int i = 0; i < 3; i++)
	{
		pose.vecWorldFromDriverTranslation[ i ] = 0.0;
		pose.vecDriverFromHeadTranslation[ i ] = 0.0;
	}

	pose.qWorldFromDriverRotation.w = 1;
	pose.qWorldFromDriverRotation.x = 0;
	pose.qWorldFromDriverRotation.y = 0;
	pose.qWorldFromDriverRotation.z = 0;

	pose.qDriverFromHeadRotation.w = 1;
	pose.qDriverFromHeadRotation.x = 0;
	pose.qDriverFromHeadRotation.y = 0;
	pose.qDriverFromHeadRotation.z = 0;

	// some things are always true
	pose.shouldApplyHeadModel = true;
	pose.willDriftInYaw = true;

	// we don't do position, so these are easy
	for( int i=0; i < 3; i++ )
	{
		pose.vecPosition[ i ] = 0.0;
		pose.vecVelocity[ i ] = 0.0;
		pose.vecAcceleration[ i ] = 0.0;

		// we also don't know the angular velocity or acceleration
		pose.vecAngularVelocity[ i ] = 0.0;
		pose.vecAngularAcceleration[ i ] = 0.0;

	}

	// now get the rotation and turn it into axis-angle format
	OVR::Quatf qOculus = m_sensorFusion.GetPredictedOrientation();
	if( qOculus.w == 0 && qOculus.x == 0 && qOculus.y == 0 && qOculus.z == 0 )
	{
		pose.qRotation.w = 1;
		pose.qRotation.x = 0;
		pose.qRotation.y = 0;
		pose.qRotation.z = 0;
		pose.poseIsValid = false;
		pose.result = TrackingResult_Uninitialized;
	}
	else
	{
		pose.qRotation.w = qOculus.w;
		pose.qRotation.x = qOculus.x;
		pose.qRotation.y = qOculus.y;
		pose.qRotation.z = qOculus.z;

		//TODO: Fold in msgBodyFrame.RotationRate for vecAngularVelocity
		//NOTE: If you do this in order to perform prediction in vrclient,
		// then use GetOrientation above instead of GetPredictedOrientation.

		pose.poseIsValid = true;
		pose.result = TrackingResult_Running_OK;
	}
	m_pPoseListener->PoseUpdated( this, pose );
}




#endif // INCLUDED_osvr_hmd_h_GUID_128E3B29_F5FC_4221_9B38_14E3F402E645

