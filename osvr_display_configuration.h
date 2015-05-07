//
// OSVR display configuration
//

#include <string>
#include <json/reader.h>
#include <iostream>
#include <cmath> // for M_PI

#ifndef M_PI
#define M_PI		3.14159265358979323846	/* pi */
#endif

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

	double getIPDMeters() const;

private:

	struct Resolution {
		int width;
		int height;
		int video_inputs;
		DisplayMode display_mode;
	};

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

	double m_CenterProjX;
	double m_CenterProjY;

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

	// Since SteamVR only supports outputting to a single window, we will
	// traverse the resolutions array to find the first entry that supports a
	// single video input.
	const Json::Value resolutions = root["hmd"]["resolutions"];
	if (resolutions.isNull()) {
		std::cerr << "ERROR: Couldn't find resolutions array!\n";
		throw;
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
		std::cerr << "ERROR: Couldn't find any appropriate resolutions.\n";
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
		std::cerr << "WARNING: Unknown display mode: " << display_mode_str << "\n";
	}

	m_Resolutions.push_back(res);

	m_RightRoll = root["hmd"]["rendering"]["right_roll"].asDouble();
	m_LeftRoll = root["hmd"]["rendering"]["left_roll"].asDouble();

	m_CenterProjX = root["hmd"]["eyes"][0]["center_proj_x"].asDouble();
	m_CenterProjY = root["hmd"]["eyes"][0]["center_proj_y"].asDouble();

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
	std::cout << "Monocular horizontal FOV: " << m_MonocularHorizontalFOV << "\n";
	std::cout << "Monocular vertical FOV: " << m_MonocularVerticalFOV << "\n";
	std::cout << "Overlap percent: " << m_OverlapPercent << "%\n";
	std::cout << "Pitch tilt: " << m_PitchTilt << "\n";
	std::cout << "Resolution: " << m_Resolutions.at(0).width << " × " << m_Resolutions.at(0).height << "\n";
	std::cout << "Video inputs: " << m_Resolutions.at(0).video_inputs << "\n";
	std::cout << "Display mode: " << m_Resolutions.at(0).display_mode << "\n";
	std::cout << "Right roll: " << m_RightRoll << "\n";
	std::cout << "Left roll: " << m_LeftRoll << "\n";
	std::cout << "Center projection: (" << m_CenterProjX << ", " << m_CenterProjY << ")\n";
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

inline double OSVRDisplayConfiguration::getIPDMeters() const
{
	return 0.065; // 65 mm
}

