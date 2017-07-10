# SteamVR-OSVR Configuration

The SteamVR-OSVR driver supports a number of configuration options. While the
defaults are tuned to work well with the OSVR HDK HMDs, you may use
SteamVR-OSVR to support other HMDs by adjusting the proper configuration
settings.


## Settings

### `activeWaitPeriod`

The delay (in milliseconds) between subsequent polls of the OSVR server for new poses and data while the driver is active. Increasing this value reduces CPU usage but also potentially increases judder.

**Default: 1**

*See also:* `standbyWaitPeriod`

### `cameraFOVBottomDegrees`
### `cameraFOVLeftDegrees`
### `cameraFOVRightDegrees`
### `cameraFOVTopDegrees`

The field of view parameters of the tracking camera.

### `cameraPath`

The OSVR device path for the camera pose.

Default: `/trackingCamera`

### `cameraRenderModel`

The 3D model used to represent the tracking camera.

To use a model within the SteamVR-OSVR driver's `resources/rendermodels` folder, use the `{osvr}` prefix followed by the base name of the `.obj` filename.
To use a model installed in SteamVR's `resources/rendermodels` folder instead, omit the `{osvr}` prefix and just use the basename of the model file.

Default: `{osvr}osvr_camera`

### `displayName`

A substring which matches the name of the display to render to. If you're using an HMD other than the OSVR HDK, you'll want to change this value to match your HMD's name.

Since it matches a substring, you can use `OSVR` to match both `OSVR HDK` and `OSVR HDK 2`, for example.

Default: `OSVR`

### `edidProductId`

The EDID product ID of your HMD. This is used to enable SteamVR's direct mode.

Default: `4121`

### `edidVendorId`

The EDID vendor ID of your HMD. This is used to enable SteamVR's direct mode.

Default: `53838`

### `ignoreVelocityReports`

When set to true, it instructs the OSVR driver to ignore any velocity reports and not send them to SteamVR. For OSVR plugins providing wonky velocity reports, it may be better to ignore them. This has the side-effect of disabling SteamVR's tracking prediction routines.

Setting this to `false` (the default value) causes the OSVR driver to pass along the velocity reports, enabling SteamVR's tracking prediction routines.

Default: `false`

### `manufacturer`

The manufacturer name to report to SteamVR applications. This value normally isn't important, but some games may use it to only run on particular HMDs. You can override this value to pretend to be a different HMD.

Default: hmd.device.vendor value in the OSVR display descriptor

### `maxTrackingRangeMeters`
### `minTrackingRangeMeters`

The minimum and maximum range (in meters) of the tracking camera's frustum along its z-axis.

Defaults: min: `0.15`; max: `1.5`

### `modelNumber`

The model number to report to SteamVR applications. This value normally isn't important, but some games may use it to only run on particular HMDs. You can override this value to pretend to be a different HMD.

Default: concatenation of the `hmd.device.model` and `hmd.device.version` values in the OSVR display descriptor

### `scanoutOrigin`

The scan-out origin of the HMD. The scan-out origin is the location of the first pixel sent to the display while the HMD is being worn. Possible values are `upper-left`, `upper-right`, `lower-right`, and `lower-left`. An empty string allows the driver to attempt to determine the proper value algorithmically.

Example: The scan-out origin of the OSVR HDK 2 is `lower-right` because when the display is not rotated by the operating system, the first pixel sent to the display (the upper-left corner of the display in its native orientation) appears in the lower-right corner of the HMD while being worn by the user.

Default: "" (determined algorithmically)

### `serialNumber`

The unique name of the display.

Default: the name of the detected display or `OSVR HMD`

### `serverTimeout`

The amount of time (in seconds) to wait for a connection to the OSVR server.

Default: 5

### `standbyWaitPeriod`

The delay (in milliseconds) between subsequent polls of the OSVR server for new poses and data while the driver is in standby mode. Increasing this value reduces CPU usage but also potentially increases judder.

Default: 100

*See also:* `activeWaitPeriod`

### `verbose`

Whether verbose logging should be enabled or disabled.

Default: `false`

### `verticalRefreshRate`

The vertical refresh rate of the HMD (in Hertz).

Default: 0.0 (values are autodetected or determined algorithmically)

