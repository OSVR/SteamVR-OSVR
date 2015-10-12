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

#ifndef INCLUDED_osvr_tracked_device_h_GUID_128E3B29_F5FC_4221_9B38_14E3F402E645
#define INCLUDED_osvr_tracked_device_h_GUID_128E3B29_F5FC_4221_9B38_14E3F402E645

/// @fixme workaround OpenVR bug!
namespace vr {
    typedef unsigned short CameraVideoStreamFormat_e; /// @todo this type is used in the SteamVR drivers
    typedef unsigned short * CameraVideoStreamFrame_t; /// @todo this type is used in the SteamVR drivers
}

// Internal Includes
#include "osvr_compiler_detection.h"
#include "osvr_display_configuration.h"
#include "make_unique.h"
#include "matrix_cast.h"
#include "projection_matrix.h"
#include "osvr_device_properties.h"

// Library/third-party includes
#include <openvr_driver.h>

#include <osvr/ClientKit/Context.h>
#include <osvr/ClientKit/Interface.h>
#include <osvr/Util/EigenInterop.h>

#include <Eigen/Geometry>

#include <util/FixedLengthStringFunctions.h>

// Standard includes
#include <cstring>
#include <string>

class OSVRTrackedDevice : public vr::ITrackedDeviceServerDriver
{
public:
    OSVRTrackedDevice(const std::string& display_description, osvr::clientkit::ClientContext& context, vr::IDriverLog* driver_log = nullptr);

    // ------------------------------------
    // Management Methods
    // ------------------------------------
    /**
     * This is called before an HMD is returned to the application. It will
     * always be called before any display or tracking methods. Memory and
     * processor use by the ITrackedDeviceServerDriver object should be kept to
     * a minimum until it is activated.  The pose listener is guaranteed to be
     * valid until Deactivate is called, but should not be used after that
     * point.
     */
    virtual vr::HmdError Activate(uint32_t object_id) OSVR_OVERRIDE;

    /**
     * This is called when The VR system is switching from this Hmd being the
     * active display to another Hmd being the active display. The driver should
     * clean whatever memory and thread use it can when it is deactivated.
     */
    virtual void Deactivate() OSVR_OVERRIDE;

    /**
     * Returns the ID of this particular HMD. This value is opaque to the VR
     * system itself, but should be unique within the driver because it will be
     * passed back in via FindHmd().
     */
    virtual const char* GetId() OSVR_OVERRIDE;

    /**
     * A VR Client has made this debug request of the driver. The set of valid
     * requests is entirely up to the driver and the client to figure out, as is
     * the format of the response. Responses that exceed the length of the
     * supplied buffer should be truncated and null terminated.
     */
    virtual void DebugRequest(const char* request, char* response_buffer, uint32_t response_buffer_size) OSVR_OVERRIDE;

    // ------------------------------------
    // Display Methods
    // ------------------------------------

    /**
     * Size and position that the window needs to be on the VR display.
     */
    virtual void GetWindowBounds(int32_t* x, int32_t* y, uint32_t* width, uint32_t* height) OSVR_OVERRIDE;

    /**
     * Returns true if the display is extending the desktop.
     */
    virtual bool IsDisplayOnDesktop() OSVR_OVERRIDE;

    /**
     * Returns true if the display is real and not a fictional display.
     */
    virtual bool IsDisplayRealDisplay() OSVR_OVERRIDE;

    /**
     * Suggested size for the intermediate render target that the distortion
     * pulls from.
     */
    virtual void GetRecommendedRenderTargetSize(uint32_t* width, uint32_t* height) OSVR_OVERRIDE;

    /**
     * Gets the viewport in the frame buffer to draw the output of the distortion
     * into
     */
    virtual void GetEyeOutputViewport(vr::Hmd_Eye eye, uint32_t* x, uint32_t* y, uint32_t* width, uint32_t* height) OSVR_OVERRIDE;

    /**
     * The components necessary to build your own projection matrix in case your
     * application is doing something fancy like infinite Z
     */
    virtual void GetProjectionRaw(vr::Hmd_Eye eye, float* left, float* right, float* top, float* bottom) OSVR_OVERRIDE;

    /**
     * Returns the transform from eye space to the head space. Eye space is the
     * per-eye flavor of head space that provides stereo disparity. Instead of
     * Model * View * Projection the sequence is Model * View * Eye^-1 *
     * Projection.  Normally View and Eye^-1 will be multiplied together and
     * treated as View in your application.
     */
    virtual vr::HmdMatrix34_t GetHeadFromEyePose(vr::Hmd_Eye eye) OSVR_OVERRIDE;

#if 0 // obsolete
    /**
     * Returns the transform between the view space and eye space. Eye space is
     * the per-eye flavor of view space that provides stereo disparity. Instead
     * of Model * View * Projection the model is Model * View * Eye *
     * Projection.  Normally View and Eye will be multiplied together and
     * treated as View in your application.
     */
    virtual vr::HmdMatrix44_t GetEyeMatrix(vr::Hmd_Eye eEye) OSVR_OVERRIDE;
#endif

    /**
     * Returns the result of the distortion function for the specified eye and
     * input UVs. UVs go from 0,0 in the upper left of that eye's viewport and
     * 1,1 in the lower right of that eye's viewport.
     */
    virtual vr::DistortionCoordinates_t ComputeDistortion(vr::Hmd_Eye eye, float u, float v) OSVR_OVERRIDE;

    // -----------------------------------
    // Assorted capability methods
    // -----------------------------------

    /**
     * Returns all the static information that will be reported to the clients.
     */
    virtual vr::TrackedDeviceDriverInfo_t GetTrackedDeviceDriverInfo() OSVR_OVERRIDE;

    // -----------------------------------
    // Administrative Methods
    // -----------------------------------

    /**
     * Returns the model number of this tracked device.
     */
    virtual const char* GetModelNumber() OSVR_OVERRIDE;

    /**
     * Returns the serial number of this tracked device.
     */
    virtual const char* GetSerialNumber() OSVR_OVERRIDE;

    // ------------------------------------
    // IPD Methods
    // ------------------------------------

    /**
     * Gets the current IPD (Interpupillary Distance) in meters.
     */
    virtual float GetIPD() OSVR_OVERRIDE;

    // ------------------------------------
    // Tracking Methods
    // ------------------------------------
    virtual vr::DriverPose_t GetPose() OSVR_OVERRIDE;

    // ------------------------------------
    // Property Methods
    // ------------------------------------

    /**
     * Returns a bool property. If the property is not available this function
     * will return false.
     */
    virtual bool GetBoolTrackedDeviceProperty(vr::TrackedDeviceProperty prop, vr::TrackedPropertyError* error) OSVR_OVERRIDE;

    /**
     * Returns a float property. If the property is not available this function
     * will return 0.
     */
    virtual float GetFloatTrackedDeviceProperty(vr::TrackedDeviceProperty prop, vr::TrackedPropertyError* error) OSVR_OVERRIDE;

    /**
     * Returns an int property. If the property is not available this function
     * will return 0.
     */
    virtual int32_t GetInt32TrackedDeviceProperty(vr::TrackedDeviceProperty prop, vr::TrackedPropertyError* error) OSVR_OVERRIDE;

    /**
     * Returns a uint64 property. If the property is not available this function
     * will return 0.
     */
    virtual uint64_t GetUint64TrackedDeviceProperty(vr::TrackedDeviceProperty prop, vr::TrackedPropertyError* error) OSVR_OVERRIDE;

    /**
     * Returns a matrix property. If the device index is not valid or the
     * property is not a matrix type, this function will return identity.
     */
    virtual vr::HmdMatrix34_t GetMatrix34TrackedDeviceProperty(vr::TrackedDeviceProperty prop, vr::TrackedPropertyError* error) OSVR_OVERRIDE;

    /**
     * Returns a string property. If the property is not available this function
     * will return 0 and @p error will be set to an error. Otherwise it returns
     * the length of the number of bytes necessary to hold this string including
     * the trailing null. If the buffer is too small the error will be
     * @c TrackedProp_BufferTooSmall. Strings will generally fit in buffers of
     * @c k_unTrackingStringSize characters. Drivers may not return strings longer
     * than @c k_unMaxPropertyStringSize.
     */
    virtual uint32_t GetStringTrackedDeviceProperty(vr::TrackedDeviceProperty prop, char* value, uint32_t buffer_size, vr::TrackedPropertyError* error) OSVR_OVERRIDE;

    // ------------------------------------
    // Controller Methods
    // ------------------------------------

    /**
     * Gets the current state of a controller.
     */
    virtual vr::VRControllerState_t GetControllerState() OSVR_OVERRIDE;

    /**
     * Returns a uint64 property. If the property is not available this function will return 0.
     */
    virtual bool TriggerHapticPulse(uint32_t axis_id, uint16_t pulse_duration_microseconds) OSVR_OVERRIDE;

    // Camera Methods
    // ------------------------------------
    virtual bool HasCamera() OSVR_OVERRIDE;
    virtual bool GetCameraFrameBufferingRequirements( int *pDefaultFrameQueueSize, uint32_t *pFrameBufferDataSize ) OSVR_OVERRIDE;
    virtual bool SetCameraFrameBuffering( int nFrameBufferCount, void **ppFrameBuffers, uint32_t nFrameBufferDataSize ) OSVR_OVERRIDE;
    virtual vr::CameraVideoStreamFormat_e GetCameraVideoStreamFormat() OSVR_OVERRIDE;
    virtual const vr::CameraVideoStreamFrame_t *GetVideoStreamFrame() OSVR_OVERRIDE;
    virtual void ReleaseVideoStreamFrame( const vr::CameraVideoStreamFrame_t *pFrameImage ) OSVR_OVERRIDE;
    virtual bool StartVideoStream() OSVR_OVERRIDE;
    virtual void StopVideoStream() OSVR_OVERRIDE;
    virtual bool IsVideoStreamActive() OSVR_OVERRIDE;
    virtual float GetVideoStreamElapsedTime() OSVR_OVERRIDE;
private:
    static void HmdTrackerCallback(void* userdata, const OSVR_TimeValue* timestamp, const OSVR_PoseReport* report);
    const std::string m_DisplayDescription;
    osvr::clientkit::ClientContext& m_Context;
    vr::IDriverLog* logger_ = nullptr;
    osvr::clientkit::Interface m_TrackerInterface;
    std::unique_ptr<OSVRDisplayConfiguration> m_DisplayConfiguration;
    vr::DriverPose_t pose_;
    vr::TrackedDeviceClass deviceClass_;
};

OSVRTrackedDevice::OSVRTrackedDevice(const std::string& display_description, osvr::clientkit::ClientContext& context, vr::IDriverLog* driver_log) : m_DisplayDescription(display_description), m_Context(context), logger_(driver_log), m_DisplayConfiguration(nullptr), pose_(), deviceClass_(vr::TrackedDeviceClass_HMD)
{
    // do nothing
}

vr::HmdError OSVRTrackedDevice::Activate(uint32_t object_id)
{
    // Retrieve display parameters
    m_DisplayConfiguration = std::make_unique<OSVRDisplayConfiguration>(m_DisplayDescription);

    // Register tracker callback
    if (m_TrackerInterface.notEmpty()) {
        m_TrackerInterface.free();
    }
    m_TrackerInterface = m_Context.getInterface("/me/head");
    m_TrackerInterface.registerCallback(&OSVRTrackedDevice::HmdTrackerCallback, this);

    return vr::HmdError_None;
}

void OSVRTrackedDevice::Deactivate()
{
    /// Have to force freeing here
    if (m_TrackerInterface.notEmpty()) {
        m_TrackerInterface.free();
    }
}

const char* OSVRTrackedDevice::GetId()
{
    /// @todo When available, return the actual unique ID of the HMD
    return "OSVR HMD";
}

void OSVRTrackedDevice::DebugRequest(const char* request, char* response_buffer, uint32_t response_buffer_size)
{
    // TODO
    // make use of (from vrtypes.h) static const uint32_t k_unMaxDriverDebugResponseSize = 32768;
}

void OSVRTrackedDevice::GetWindowBounds(int32_t* x, int32_t* y, uint32_t* width, uint32_t* height)
{
    *x = m_DisplayConfiguration->getDisplayLeft();
    *y = m_DisplayConfiguration->getDisplayTop();
    *width = m_DisplayConfiguration->getDisplayWidth();
    *height = m_DisplayConfiguration->getDisplayHeight();
}

bool OSVRTrackedDevice::IsDisplayOnDesktop()
{
    // TODO get this info from display description?
    return true;
}

bool OSVRTrackedDevice::IsDisplayRealDisplay()
{
    // TODO get this info from display description?
    return true;
}

void OSVRTrackedDevice::GetRecommendedRenderTargetSize(uint32_t* width, uint32_t* height)
{
    *width = m_DisplayConfiguration->getDisplayWidth();
    *height = m_DisplayConfiguration->getDisplayHeight();
}

void OSVRTrackedDevice::GetEyeOutputViewport(vr::Hmd_Eye eye, uint32_t* x, uint32_t* y, uint32_t* width, uint32_t* height)
{
    switch (m_DisplayConfiguration->getDisplayMode()) {
    case OSVRDisplayConfiguration::HORIZONTAL_SIDE_BY_SIDE:
        *width = m_DisplayConfiguration->getDisplayWidth() / 2;
        *height = m_DisplayConfiguration->getDisplayHeight();
        *x = (vr::Eye_Left == eye) ? 0 : *width;
        *y = 0;
        break;
    case OSVRDisplayConfiguration::VERTICAL_SIDE_BY_SIDE:
        *width = m_DisplayConfiguration->getDisplayWidth();
        *height = m_DisplayConfiguration->getDisplayHeight() / 2;
        *x = 0;
        *y = (vr::Eye_Left == eye) ? 0 : *height;
        break;
    case OSVRDisplayConfiguration::FULL_SCREEN:
        *width = m_DisplayConfiguration->getDisplayWidth();
        *height = m_DisplayConfiguration->getDisplayHeight();
        *x = 0;
        *y = 0;
        break;
    default:
        *width = m_DisplayConfiguration->getDisplayWidth();
        *height = m_DisplayConfiguration->getDisplayHeight();
        *x = 0;
        *y = 0;
        std::cerr << "ERROR: Unexpected display mode type: " << m_DisplayConfiguration->getDisplayMode() << ".\n";
    }
}

void OSVRTrackedDevice::GetProjectionRaw(vr::Hmd_Eye eye, float* left, float* right, float* top, float* bottom)
{
    /// @todo this does not produce correct output

    // Projection matrix centered between the eyes
    const double z_near = 0.1;
    const double z_far = 100.0;
    const Eigen::Matrix4d center = make_projection_matrix(m_DisplayConfiguration->getHorizontalFOVRadians(), m_DisplayConfiguration->getFOVAspectRatio(), z_near, z_far);

    // Translate projection matrix to left or right eye
    double eye_translation = m_DisplayConfiguration->getIPDMeters() / 2.0;
    if (vr::Eye_Left == eye) {
        eye_translation *= -1.0;
    }
    const Eigen::Affine3d translation(Eigen::Translation3d(Eigen::Vector3d(eye_translation, 0.0, 0.0)));

    const Eigen::Matrix4d eye_matrix = translation * center;

    const double near = eye_matrix(2, 3) / (eye_matrix(2, 2) - 1.0);
    const double far = eye_matrix(2, 3) / (eye_matrix(3, 3) + 1.0);
    *bottom = static_cast<float>(near * (eye_matrix(1, 2) - 1.0) / eye_matrix(1, 1));
    *top = static_cast<float>(near * (eye_matrix(1, 2) + 1.0) / eye_matrix(1, 1));
    *left = static_cast<float>(near * (eye_matrix(0, 2) - 1.0) / eye_matrix(0, 0));
    *right = static_cast<float>(near * (eye_matrix(0, 2) + 1.0) / eye_matrix(0, 0));
}

vr::HmdMatrix34_t OSVRTrackedDevice::GetHeadFromEyePose(vr::Hmd_Eye eye)
{

    // Rotate per the display configuration
    const double horiz_fov = m_DisplayConfiguration->getHorizontalFOVRadians();
    const double overlap = m_DisplayConfiguration->getOverlapPercent() * horiz_fov;
    double angle = (horiz_fov - overlap) / 2.0;
    if (vr::Eye_Right == eye) {
        angle *= -1.0;
    }
    const Eigen::Affine3d rotation = Eigen::Affine3d(Eigen::AngleAxisd(angle, Eigen::Vector3d(0, 1, 0)));

    // Translate along x-axis by half the interpupillary distance
    double eye_translation = m_DisplayConfiguration->getIPDMeters() / 2.0;
    if (vr::Eye_Left == eye) {
        eye_translation *= -1.0;
    }
    const Eigen::Affine3d translation(Eigen::Translation3d(Eigen::Vector3d(eye_translation, 0.0, 0.0)));
    // Eye matrix
    const Matrix34f mat = (translation * rotation).matrix().block(0,0,3,4).cast<float>();

    vr::HmdMatrix34_t matrix;
    map(matrix) = (mat);
    return matrix;
}

vr::DistortionCoordinates_t OSVRTrackedDevice::ComputeDistortion(vr::Hmd_Eye eye, float u, float v)
{
    /// @todo FIXME Compute distortion using display configuration data
    vr::DistortionCoordinates_t coords;
    coords.rfRed[0] = u;
    coords.rfRed[1] = v;
    coords.rfBlue[0] = u;
    coords.rfBlue[1] = v;
    coords.rfGreen[0] = u;
    coords.rfGreen[1] = v;
    return coords;
}

vr::TrackedDeviceDriverInfo_t OSVRTrackedDevice::GetTrackedDeviceDriverInfo()
{
    vr::TrackedDeviceDriverInfo_t info;
    util::strcpy_safe(info.rchTrackingSystemId, "OSVR"); // TODO name of the underlying tracking system
    util::strcpy_safe(info.rchSerialNumber, GetSerialNumber());
    util::strcpy_safe(info.rchModelNumber, GetModelNumber());
    info.rchRenderModelName[0] = '\0';        // TODO pass this to GetRenderModel to get the mesh and texture to render this device
    info.eClass = vr::TrackedDeviceClass_HMD; // TODO adjust accordingly
    info.bDeviceIsConnected = true;           // false if user unplugs device
    info.bWillDriftInYaw = true;              // true if gyro-only tracking system
    info.bReportsTimeSinceVSync = false;
    info.fSecondsFromVsyncToPhotons = 0.0; // seconds between vsync and photons hitting wearer's eyes
    info.fDisplayFrequency = 60.0;         // fps of display

    return info;
}

const char* OSVRTrackedDevice::GetModelNumber()
{
    /// @todo When available, return the actual model number of the HMD
    return "OSVR HMD";
}

const char* OSVRTrackedDevice::GetSerialNumber()
{
    /// @todo When available, return the actual serial number of the HMD
    return "0";
}

float OSVRTrackedDevice::GetIPD()
{
    return static_cast<float>(m_DisplayConfiguration->getIPDMeters());
}

vr::DriverPose_t OSVRTrackedDevice::GetPose()
{
    return pose_;
}

bool OSVRTrackedDevice::GetBoolTrackedDeviceProperty(vr::TrackedDeviceProperty prop, vr::TrackedPropertyError* error)
{
    const bool default_value = false;

    if (isWrongDataType(prop, bool())) {
        if (error)
            *error = vr::TrackedProp_WrongDataType;
        return default_value;
    }

    if (isWrongDeviceClass(prop, deviceClass_)) {
        if (error)
            *error = vr::TrackedProp_WrongDeviceClass;
        return default_value;
    }

    if (vr::TrackedDeviceClass_Invalid == deviceClass_) {
        if (error)
            *error = vr::TrackedProp_InvalidDevice;
        return default_value;
    }

    switch (prop) {
    case vr::Prop_WillDriftInYaw_Bool: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
        break;
    case vr::Prop_ReportsTimeSinceVSync_Bool: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
        break;
    case vr::Prop_IsOnDesktop_Bool: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
        break;
    }

    if (error)
        *error = vr::TrackedProp_UnknownProperty;
    return default_value;
}

float OSVRTrackedDevice::GetFloatTrackedDeviceProperty(vr::TrackedDeviceProperty prop, vr::TrackedPropertyError* error)
{
    const float default_value = 0.0f;

    if (isWrongDataType(prop, float())) {
        if (error)
            *error = vr::TrackedProp_WrongDataType;
        return default_value;
    }

    if (isWrongDeviceClass(prop, deviceClass_)) {
        if (error)
            *error = vr::TrackedProp_WrongDeviceClass;
        return default_value;
    }

    if (vr::TrackedDeviceClass_Invalid == deviceClass_) {
        if (error)
            *error = vr::TrackedProp_InvalidDevice;
        return default_value;
    }

    switch (prop) {
    case vr::Prop_SecondsFromVsyncToPhotons_Float: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_DisplayFrequency_Float: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_UserIpdMeters_Float: // TODO
        if (error)
            *error = vr::TrackedProp_Success;
        return static_cast<float>(m_DisplayConfiguration->getIPDMeters());
    case vr::Prop_FieldOfViewLeftDegrees_Float: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_FieldOfViewRightDegrees_Float: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_FieldOfViewTopDegrees_Float: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_FieldOfViewBottomDegrees_Float: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_TrackingRangeMinimumMeters_Float: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_TrackingRangeMaximumMeters_Float: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    }

    if (error)
        *error = vr::TrackedProp_UnknownProperty;
    return default_value;
}

int32_t OSVRTrackedDevice::GetInt32TrackedDeviceProperty(vr::TrackedDeviceProperty prop, vr::TrackedPropertyError* error)
{
    const int32_t default_value = 0;

    if (isWrongDataType(prop, int32_t())) {
        if (error)
            *error = vr::TrackedProp_WrongDataType;
        return default_value;
    }

    if (isWrongDeviceClass(prop, deviceClass_)) {
        if (error)
            *error = vr::TrackedProp_WrongDeviceClass;
        return default_value;
    }

    if (vr::TrackedDeviceClass_Invalid == deviceClass_) {
        if (error)
            *error = vr::TrackedProp_InvalidDevice;
        return default_value;
    }

    switch (prop) {
    case vr::Prop_Axis0Type_Int32: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_Axis1Type_Int32: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_Axis2Type_Int32: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_Axis3Type_Int32: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_Axis4Type_Int32: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    }

    if (error)
        *error = vr::TrackedProp_UnknownProperty;
    return default_value;
}

uint64_t OSVRTrackedDevice::GetUint64TrackedDeviceProperty(vr::TrackedDeviceProperty prop, vr::TrackedPropertyError* error)
{
    const uint64_t default_value = 0;

    if (isWrongDataType(prop, uint64_t())) {
        if (error)
            *error = vr::TrackedProp_WrongDataType;
        return default_value;
    }

    if (isWrongDeviceClass(prop, deviceClass_)) {
        if (error)
            *error = vr::TrackedProp_WrongDeviceClass;
        return default_value;
    }

    if (vr::TrackedDeviceClass_Invalid == deviceClass_) {
        if (error)
            *error = vr::TrackedProp_InvalidDevice;
        return default_value;
    }

    switch (prop) {
    case vr::Prop_CurrentUniverseId_Uint64: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_PreviousUniverseId_Uint64: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_SupportedButtons_Uint64: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    }

    if (error)
        *error = vr::TrackedProp_UnknownProperty;
    return default_value;
}

vr::HmdMatrix34_t OSVRTrackedDevice::GetMatrix34TrackedDeviceProperty(vr::TrackedDeviceProperty prop, vr::TrackedPropertyError* error)
{
    // Default value is identity matrix
    vr::HmdMatrix34_t default_value;
    map(default_value) = Matrix34f::Identity();

    if (isWrongDataType(prop, vr::HmdMatrix34_t())) {
        if (error)
            *error = vr::TrackedProp_WrongDataType;
        return default_value;
    }

    if (isWrongDeviceClass(prop, deviceClass_)) {
        if (error)
            *error = vr::TrackedProp_WrongDeviceClass;
        return default_value;
    }

    if (vr::TrackedDeviceClass_Invalid == deviceClass_) {
        if (error)
            *error = vr::TrackedProp_InvalidDevice;
        return default_value;
    }

    switch (prop) {
    case vr::Prop_StatusDisplayTransform_Matrix34: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    }

    if (error)
        *error = vr::TrackedProp_UnknownProperty;
    return default_value;
}

uint32_t OSVRTrackedDevice::GetStringTrackedDeviceProperty(vr::TrackedDeviceProperty prop, char* value, uint32_t buffer_size, vr::TrackedPropertyError* error)
{
    const uint32_t default_value = 0;

    if (isWrongDataType(prop, value)) {
        if (error)
            *error = vr::TrackedProp_WrongDataType;
        return default_value;
    }

    if (isWrongDeviceClass(prop, deviceClass_)) {
        if (error)
            *error = vr::TrackedProp_WrongDeviceClass;
        return default_value;
    }

    if (vr::TrackedDeviceClass_Invalid == deviceClass_) {
        if (error)
            *error = vr::TrackedProp_InvalidDevice;
        return default_value;
    }

    switch (prop) {
    case vr::Prop_TrackingSystemName_String: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_ModelNumber_String: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_SerialNumber_String: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_RenderModelName_String: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_ManufacturerName_String: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_TrackingFirmwareVersion_String: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_HardwareRevision_String: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_DisplayFirmwareVersion_String: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_AttachedDeviceId_String: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_AllWirelessDongleDescriptions_String:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_ConnectedWirelessDongle_String:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    }

    if (error)
        *error = vr::TrackedProp_UnknownProperty;
    return default_value;
}

vr::VRControllerState_t OSVRTrackedDevice::GetControllerState()
{
    // TODO
    vr::VRControllerState_t controller_state;

#if 0
    // If packet num matches that on your prior call, then the controller state hasn't been changed since
    // your last call and there is no need to process it
    uint32_t unPacketNum;

    // bit flags for each of the buttons. Use ButtonMaskFromId to turn an ID into a mask
    uint64_t ulButtonPressed;
    uint64_t ulButtonTouched;

    // Axis data for the controller's analog inputs
    VRControllerAxis_t rAxis[ k_unControllerStateAxisCount ];

    /** contains information about one axis on the controller */
    struct VRControllerAxis_t
    {
        float x; // Ranges from -1.0 to 1.0 for joysticks and track pads. Ranges from 0.0 to 1.0 for triggers were 0 is fully released.
        float y; // Ranges from -1.0 to 1.0 for joysticks and track pads. Is always 0.0 for triggers.
    };
    /** the number of axes in the controller state */
    static const uint32_t k_unControllerStateAxisCount = 5;
#endif

    return controller_state;
}

bool OSVRTrackedDevice::TriggerHapticPulse(uint32_t axis_id, uint16_t pulse_duration_microseconds)
{
    /// @todo SteamVR drivers return false and do nothing else, doing the same here
    return false;
}

bool OSVRTrackedDevice::HasCamera()
{
    /// @todo SteamVR drivers return false and do nothing else, doing the same here
    return false;
}

bool OSVRTrackedDevice::GetCameraFrameBufferingRequirements( int *pDefaultFrameQueueSize, uint32_t *pFrameBufferDataSize )
{
    /// @todo SteamVR drivers return false and do nothing else, doing the same here
    return false;
}
bool OSVRTrackedDevice::SetCameraFrameBuffering( int nFrameBufferCount, void **ppFrameBuffers, uint32_t nFrameBufferDataSize )
{
    /// @todo SteamVR drivers return false and do nothing else, doing the same here
    return false;
}
vr::CameraVideoStreamFormat_e OSVRTrackedDevice::GetCameraVideoStreamFormat()
{
    /// @todo SteamVR drivers return NULL and do nothing else, doing the same here
    return 0;
}
const vr::CameraVideoStreamFrame_t * OSVRTrackedDevice::GetVideoStreamFrame()
{
    /// @todo SteamVR drivers return NULL and do nothing else, doing the same here
    return NULL;
}
void OSVRTrackedDevice::ReleaseVideoStreamFrame( const vr::CameraVideoStreamFrame_t *pFrameImage )
{
    /// @todo SteamVR drivers does nothing, doing the same here
}

bool OSVRTrackedDevice::StartVideoStream()
{
    /// @todo SteamVR drivers return false and do nothing else, doing the same here
    return false;
}
void OSVRTrackedDevice::StopVideoStream()
{
    /// @todo SteamVR drivers does nothing, doing the same here
}
bool OSVRTrackedDevice::IsVideoStreamActive()
{
    /// @todo SteamVR drivers return false and do nothing else, doing the same here
    return false;
}
float OSVRTrackedDevice::GetVideoStreamElapsedTime()
{
    /// @todo SteamVR drivers return 0 and do nothing else, doing the same here
    return 0;
}




void OSVRTrackedDevice::HmdTrackerCallback(void* userdata, const OSVR_TimeValue* timestamp, const OSVR_PoseReport* report)
{
    if (!userdata)
        return;

    auto* self = static_cast<OSVRTrackedDevice*>(userdata);

    vr::DriverPose_t pose;
    pose.poseTimeOffset = 0; // close enough

    Eigen::Vector3d::Map(pose.vecWorldFromDriverTranslation) = Eigen::Vector3d::Zero();
    Eigen::Vector3d::Map(pose.vecDriverFromHeadTranslation) = Eigen::Vector3d::Zero();

    map(pose.qWorldFromDriverRotation) = Eigen::Quaterniond::Identity();

    map(pose.qDriverFromHeadRotation) = Eigen::Quaterniond::Identity();

    // Position
    Eigen::Vector3d::Map(pose.vecPosition) = osvr::util::vecMap(report->pose.translation);

    // Position velocity and acceleration are not currently consistently provided
    Eigen::Vector3d::Map(pose.vecVelocity) = Eigen::Vector3d::Zero();
    Eigen::Vector3d::Map(pose.vecAcceleration) = Eigen::Vector3d::Zero();

    // Orientation
    map(pose.qRotation) = osvr::util::fromQuat(report->pose.rotation);

    // Angular velocity and acceleration are not currently consistently provided
    Eigen::Vector3d::Map(pose.vecAngularVelocity) = Eigen::Vector3d::Zero();
    Eigen::Vector3d::Map(pose.vecAngularAcceleration) = Eigen::Vector3d::Zero();

    pose.result = vr::TrackingResult_Running_OK;
    pose.poseIsValid = true;
    pose.willDriftInYaw = true;
    pose.shouldApplyHeadModel = true;

    if (self->logger_)
        self->logger_->Log("OSVRTrackedDevice::HmdTrackerCallback(): Got new pose.\n");

    self->pose_ = pose;
}

#endif // INCLUDED_osvr_tracked_device_h_GUID_128E3B29_F5FC_4221_9B38_14E3F402E645
