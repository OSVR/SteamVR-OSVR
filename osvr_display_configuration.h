/** @file
    @brief OSVR display configuration

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

#ifndef INCLUDED_osvr_display_configuration_h_GUID_BF32FB3A_205A_40D0_B774_F38F6419EC54
#define INCLUDED_osvr_display_configuration_h_GUID_BF32FB3A_205A_40D0_B774_F38F6419EC54

// Internal Includes
#include "osvr_compiler_detection.h"

// Library/third-party includes
#include <json/value.h>
#include <json/reader.h>

// Standard includes
#include <string>
#include <iostream>
#include <cmath> // for M_PI
#include <exception>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846 /* pi */
#endif

class DisplayConfigurationParseException : public std::exception {
public:
    DisplayConfigurationParseException(const std::string& message) OSVR_NOEXCEPT : std::exception(), m_message(message)
    {
        // do nothing
    }

    virtual const char* what() const OSVR_NOEXCEPT OSVR_OVERRIDE
    {
        return m_message.c_str();
    }

private:
    const std::string m_message;
};

class OSVRDisplayConfiguration {
public:
    enum DisplayMode {
        HORIZONTAL_SIDE_BY_SIDE,
        VERTICAL_SIDE_BY_SIDE,
        FULL_SCREEN
    };

    OSVRDisplayConfiguration();
    OSVRDisplayConfiguration(const std::string& display_description);

    void parse(const std::string& display_description);

    void print() const;

    int getNumDisplays() const;

    int getDisplayTop() const;
    int getDisplayLeft() const;
    int getDisplayWidth() const;
    int getDisplayHeight() const;
    DisplayMode getDisplayMode() const;

    double getVerticalFOV() const;
    double getVerticalFOVRadians() const;
    double getHorizontalFOV() const;
    double getHorizontalFOVRadians() const;
    double getFOVAspectRatio() const;
    double getOverlapPercent() const;

    double getPitchTilt() const;

    double getIPDMeters() const;

    /// Structure holding the information for one eye.
    class EyeInfo {
    public:
        double m_CenterProjX = 0.5;
        double m_CenterProjY = 0.5;
        bool m_rotate180 = false;

        void print() const;
    };

    std::vector<EyeInfo> getEyes() const;

private:
    struct Resolution {
        int width;
        int height;
        int video_inputs;
        DisplayMode display_mode;
    };

    int m_NumDisplays;

    double m_MonocularHorizontalFOV;
    double m_MonocularVerticalFOV;
    double m_OverlapPercent;
    double m_PitchTilt;

    std::vector<Resolution> m_Resolutions;

    // Distortion
    double k1_red;
    double k1_green;
    double k1_blue;

    // Rendering
    double m_RightRoll;
    double m_LeftRoll;

    // Eyes
    std::vector<EyeInfo> m_eyes;
};

inline OSVRDisplayConfiguration::OSVRDisplayConfiguration()
{
    // do nothing
}

inline OSVRDisplayConfiguration::OSVRDisplayConfiguration(const std::string& display_description)
{
    parse(display_description);
}

inline void OSVRDisplayConfiguration::parse(const std::string& display_description)
{
    Json::Reader reader;
    Json::Value root;
    reader.parse(display_description, root, false);

    // Field of view
    m_MonocularHorizontalFOV = root["hmd"]["field_of_view"]["monocular_horizontal"].asDouble();
    m_MonocularVerticalFOV = root["hmd"]["field_of_view"]["monocular_vertical"].asDouble();
    m_OverlapPercent = root["hmd"]["field_of_view"]["overlap_percent"].asDouble() / 100.0;
    m_PitchTilt = root["hmd"]["field_of_view"]["pitch_tilt"].asDouble();
    m_NumDisplays = root["hmd"]["device"]["properties"]["num_displays"].asInt();

    // Since SteamVR only supports outputting to a single window, we will
    // traverse the resolutions array to find the first entry that supports a
    // single video input.
    const Json::Value resolutions = root["hmd"]["resolutions"];
    if (resolutions.isNull()) {
        std::cerr << "OSVRDisplayConfiguration::parse(): ERROR: Couldn't find resolutions array!\n";
        throw DisplayConfigurationParseException("Couldn't find resolutions array.");
    }

    Resolution res;
    Json::Value resolution;
    for (Json::Value::iterator iter = resolutions.begin(); iter != resolutions.end(); ++iter) {
        const Json::Value video_inputs = (*iter)["video_inputs"];
        if (video_inputs.isNull() || !video_inputs.isInt() || 1 != video_inputs.asInt()) {
            // Missing video_inputs entry, non-integral data type, or wrong
            // number of video inputs. Skipping entry.
            continue;
        }

        res.video_inputs = video_inputs.asInt();
        resolution = (*iter);
        break;
    }

    if (resolution.isNull()) {
        // We couldn't find any appropriate resolution entries
        std::cerr << "OSVRDisplayConfiguration::parse(): ERROR: Couldn't find any appropriate resolutions.\n";
        return;
    }

    // Window bounds
    res.width = resolution["width"].asInt();
    res.height = resolution["height"].asInt();

    // Display mode
    const std::string display_mode_str = resolution["display_mode"].asString();
    res.display_mode = HORIZONTAL_SIDE_BY_SIDE;
    if ("horz_side_by_side" == display_mode_str) {
        res.display_mode = HORIZONTAL_SIDE_BY_SIDE;
    } else if ("vert_side_by_size" == display_mode_str) {
        res.display_mode = VERTICAL_SIDE_BY_SIDE;
    } else if ("full_screen" == display_mode_str) {
        res.display_mode = FULL_SCREEN;
    } else {
        res.display_mode = HORIZONTAL_SIDE_BY_SIDE;
        std::cerr << "OSVRDisplayConfiguration::parse(): WARNING: Unknown display mode: " << display_mode_str << std::endl;
    }

    m_Resolutions.push_back(res);

    m_RightRoll = root["hmd"]["rendering"]["right_roll"].asDouble();
    m_LeftRoll = root["hmd"]["rendering"]["left_roll"].asDouble();

    const Json::Value eyes = root["hmd"]["eyes"];
    if (eyes.isNull()) {
        std::cerr << "OSVRDisplayConfiguration::parse(): ERROR: Couldn't find eyes array!\n";
        throw DisplayConfigurationParseException("Couldn't find eyes array.");
    }
    for (Json::Value::iterator iter = eyes.begin(); iter != eyes.end(); ++iter) {
        EyeInfo e;
        e.m_CenterProjX = (*iter)["center_proj_x"].asDouble();
        e.m_CenterProjY = (*iter)["center_proj_y"].asDouble();
        if ((*iter).isMember("rotate_180")) {
            e.m_rotate180 = ((*iter)["rotate_180"].asInt() != 0);
        }
        m_eyes.push_back(e);
    }

#if 0
	/** The components necessary to build your own projection matrix in case your
	* application is doing something fancy like infinite Z */
	virtual void GetProjectionRaw( Hmd_Eye eEye, float *pfLeft, float *pfRight, float *pfTop, float *pfBottom ) = 0;

	/** Returns the transform between the view space and eye space. Eye space is the per-eye flavor of view
	* space that provides stereo disparity. Instead of Model * View * Projection the model is Model * View * Eye * Projection.
	* Normally View and Eye will be multiplied together and treated as View in your application.
	*/
	virtual HmdMatrix44_t GetEyeMatrix( Hmd_Eye eEye ) = 0;

	/** Returns the result of the distortion function for the specified eye and input UVs. UVs go from 0,0 in
	* the upper left of that eye's viewport and 1,1 in the lower right of that eye's viewport. */
	virtual DistortionCoordinates_t ComputeDistortion( Hmd_Eye eEye, float fU, float fV ) = 0;
#endif
}

inline void OSVRDisplayConfiguration::print() const
{
    std::cout << "Monocular horizontal FOV: " << m_MonocularHorizontalFOV << std::endl;
    std::cout << "Monocular vertical FOV: " << m_MonocularVerticalFOV << std::endl;
    std::cout << "Overlap percent: " << m_OverlapPercent << "%" << std::endl;
    std::cout << "Pitch tilt: " << m_PitchTilt << std::endl;
    std::cout << "Resolution: " << m_Resolutions.at(0).width << " x " << m_Resolutions.at(0).height << std::endl;
    std::cout << "Video inputs: " << m_Resolutions.at(0).video_inputs << std::endl;
    std::cout << "Display mode: " << m_Resolutions.at(0).display_mode << std::endl;
    std::cout << "Right roll: " << m_RightRoll << std::endl;
    std::cout << "Left roll: " << m_LeftRoll << std::endl;
    std::cout << "Number of eyes: " << m_eyes.size() << std::endl;
    for (std::vector<EyeInfo>::size_type i = 0; i < m_eyes.size(); ++i) {
        std::cout << "Eye " << i << ": " << std::endl;
        m_eyes[i].print();
    }
}

inline int OSVRDisplayConfiguration::getNumDisplays() const
{
    return m_NumDisplays;
}

inline int OSVRDisplayConfiguration::getDisplayTop() const
{
    return 0;
}

inline int OSVRDisplayConfiguration::getDisplayLeft() const
{
    return 0;
}

inline int OSVRDisplayConfiguration::getDisplayWidth() const
{
    return m_Resolutions.at(0).width;
}

inline int OSVRDisplayConfiguration::getDisplayHeight() const
{
    return m_Resolutions.at(0).height;
}

inline OSVRDisplayConfiguration::DisplayMode OSVRDisplayConfiguration::getDisplayMode() const
{
    return m_Resolutions.at(0).display_mode;
}

inline double OSVRDisplayConfiguration::getVerticalFOV() const
{
    return m_MonocularVerticalFOV;
}

inline double OSVRDisplayConfiguration::getVerticalFOVRadians() const
{
    return m_MonocularVerticalFOV * M_PI / 180.0;
}

inline double OSVRDisplayConfiguration::getHorizontalFOV() const
{
    return m_MonocularHorizontalFOV;
}

inline double OSVRDisplayConfiguration::getHorizontalFOVRadians() const
{
    return m_MonocularHorizontalFOV * M_PI / 180.0;
}

inline double OSVRDisplayConfiguration::getFOVAspectRatio() const
{
    return m_MonocularVerticalFOV / m_MonocularHorizontalFOV;
}

inline double OSVRDisplayConfiguration::getOverlapPercent() const
{
    return m_OverlapPercent;
}

inline double OSVRDisplayConfiguration::getPitchTilt() const
{
    return m_PitchTilt;
}

inline double OSVRDisplayConfiguration::getIPDMeters() const
{
    return 0.065; // 65 mm
}

inline std::vector<OSVRDisplayConfiguration::EyeInfo> OSVRDisplayConfiguration::getEyes() const
{
    return m_eyes;
}

inline void OSVRDisplayConfiguration::EyeInfo::print() const
{
    std::cout << "Center of projection (X): " << m_CenterProjX << std::endl;
    std::cout << "Center of projection (Y): " << m_CenterProjY << std::endl;
    std::cout << "Rotate by 180: " << m_rotate180 << std::endl;
}

#endif // INCLUDED_osvr_display_configuration_h_GUID_BF32FB3A_205A_40D0_B774_F38F6419EC54

