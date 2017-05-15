# SteamVR Driver Using OSVR

[![Join the chat at https://gitter.im/OSVR/SteamVR-OSVR](https://badges.gitter.im/OSVR/SteamVR-OSVR.svg)](https://gitter.im/OSVR/SteamVR-OSVR?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
> Maintained at <https://github.com/OSVR/SteamVR-OSVR>
>
> For details, see <http://osvr.github.io>
>
> For support, see <http://support.osvr.com>
>
> Windows alpha binaries: [ ![Download Windows Alpha](https://api.bintray.com/packages/osvr/SteamVR-OSVR/SteamVR-OSVR-Win/images/download.svg) ](https://bintray.com/osvr/SteamVR-OSVR/SteamVR-OSVR-Win/_latestVersion)

This is a SteamVR driver for allowing applications written against that API to work with hardware and software running with the OSVR software framework.

Note that despite similar goals, the internal models of the two systems are not the same (in most cases, OSVR has a more flexible, descriptive model), so if you're writing an application from scratch, be sure to evaluate that. This driver exists primarily for compatibility reasons (a bit like the Unity or Unreal integration, but more general), so existing software using the SteamVR system can run on OSVR. *OSVR ClientKit itself (along with its wrappers/engine integrations) is and remains the first-class preferred API for working with OSVR.*

## Binary Usage Instructions
If you'd just like to try this out with pre-compiled binaries on Windows, the link above contains a compressed file with the 32- and 64-bit builds in it. (Other platforms will have to build from source at this time.) This is not the only way to use the binaries, but it's the simplest way to use them with commercially-released SteamVR games.

### Installation

1. Download and extract an [OSVR Core snapshot build and whatever plugins you need](http://osvr.github.io/using/) for your hardware. (Support for the OSVR HDK, among others, is bundled with the main download, no additional plugins required.) This is for the OSVR Server, and is common to using any OSVR-enabled application.
2. Install [Steam](http://steampowered.com) and optionally, a SteamVR-enabled game (Team Fortress 2 was tested).
3. Install SteamVR by hovering over the "Library" button in Steam, clicking on to "Tools" in the drop-down menu, then finding the "SteamVR" entry, right-clicking it, and clicking "Install Game". (You should **not** need any beta versions of Steam or SteamVR to use this.)
4. Download the binary snapshot, and extract it using 7-Zip.
5. Now, you'll need to put the driver where SteamVR can find it.

The default locations are as follows:

- If you accepted defaults, Steam's nested SteamVR directory is installed in `"%ProgramFiles(x86)%\Steam\steamapps\common\SteamVR"` - you can paste that into the Windows Run dialog and it should open the right folder.  You can also find SteamVR in the Library, Tools section of Steam, right-click it, choose Properties, then Local Files, then "Browse local files..."

	- "The right folder" will contain, among other things, a directory called `drivers`.

- Upon extracting the OSVR driver, you'll get a `SteamVR-OSVR` directory. Drill down through `SteamVR-OSVR/lib/openvr` until you find an `osvr` folder.

You'll want to drag the `osvr` directory `drivers` directory you found previously. If you are upgrading from an earlier version of the driver,  merge its contents with the existing contents of the `osvr` directory.

To know if you got it right, do the same Windows Run (or other way of opening a folder by its name) as above, with the path `"%ProgramFiles(x86)%\Steam\steamapps\common\SteamVR\drivers\osvr\bin\win32"` instead. (Make changes as appropriate to suit your Steam install location.) If you got it right, you should see a number of files, including one called `driver_osvr.dll`.

### Usage

In all cases, hook up your hardware and launch OSVR Server first.

**Simple:** Paste `"%ProgramFiles(x86)%\Steam\steamapps\common\SteamVR\demo\bin\win32\hellovr_sdl.exe"` into the Windows Run dialog and click run to start the SteamVR simple demo app - you should get a command-prompt window with some text, the screen might flicker, then you should get a split-screen display that reacts to the movement of the HMD by changing your view of a world with infinite cubes of test patterns. (You might need to click to bring the demo window to the foreground.)

**Real game:** After osvr server is running you can launch steamvr from steam large mode (normal desktop client) by clicking the vr button.  Pressing the "Guide" button or equivalent depending on controller will open up steamvr dashboard which gives access to settings and vroverlays which you use to launch vr applications, once steamvr is running.   The kb shortcut for this (shift + tab) should also work in steamvr to bring up vr dashboard.   Using the customisation options from dashboard settings allows you to use backgrounds, environments etc for steamvr compositor which makes for a much more interesting steamvr experience.   You need to subscribe to each individual item you want from the steamvr workshop.  There is a shortcut to workshop in steamvr dashboard and steamvr monitor (the tools that run in system tray after steamvr is launched).

Use the steamvr related discussions groups on steam to keep up to date with all the updates and latest bugs in steamvr.  Few issues in steamvr are osvr or osvr hdk specific.

https://steamcommunity.com/app/250820/discussions/

https://steamcommunity.com/app/358720/discussions/

  
Different games have different ways of initialising VR mode - some start in oculus mode, some start in vive mode, some ask you if you want to launch in osvr, oculus or vive mode, some you right-click on them in Steam and specify in the options to pass `/vr` to launch in VR mode. You might find info about this on the Internet - many of these sources are outdated: for example, no command line switch is required for TF2 to run in VR.

Depending on what vr hardware you are using a combination of some of these (possibly even all) third party apps will help to allow using your vr hardware to enjoy all openvr apps, that is any app launched from steam using steamvr runtime.

Revive allows non Oculus headsets be used for games which only support Oculus headsets when run through steamvr runtime https://github.com/LibreVR/Revive

FakeVive allows non HTC headsets be used for games which only support HTC headsets when run through steamvr runtime. https://github.com/Shockfire/FakeVive

OpenVR-AdvancedSettings gives users access to many useful openvr and openvr apps settings. Users that do not have Vive controllers will need to access these settings from desktop or rely on using the hydra drivers and six sense SDK from steam tools to emulate Vive controllers to access the settings in an overlay running as a steamvr dashboard app https://github.com/matzman666/OpenVR-AdvancedSettings

For Vive Controller Emulation:

Sixense SDK for Razer Hydra https://steamdb.info/app/42300/

SteamVR Driver for Razer Hydra http://store.steampowered.com/app/491380/SteamVR_Driver_for_Razer_Hydra/

FreePIE https://github.com/AndersMalmgren/FreePIE

Opentrack https://github.com/opentrack/opentrack

FreePIE-VR-Controls https://github.com/fmaurer/FreePIE-VR-Controls


For example sake, we'll use TF2, which has it accessible in its menu: go to "Options", then "Video" and change "Virtual Reality Mode" to "Enabled". You'll have to save that setting and restart the game before you'll see a new entry on the game's main menu called "Activate Virtual Reality". Selecting "Activate Virtual Reality" should flip the game into VR mode and you're ready to play. (Might be best to use a gamepad, as WASD and mouselook don't go well with VR, the mouselook portion in particular.)


## Build Instructions
### Prerequisites
- [CMake][] v3.1 or newer, latest version always recommended
- A compiler supporting C++11
	- On Windows, Visual Studio 2013 recommended.
	- Recent compilers on Linux are fine too.
	- XCode and clang on OS X will build this project.
- Somehow create or acquire a build of the following (make sure the bits match for everything that's not header-only):
	- [OSVR-Core][]
		- Client libraries are sufficient if you're building from scratch for this purpose only.
		- Windows snapshot builds are available at <http://access.osvr.com/binary/osvr-core>
	- [Boost][]
		- headers-only is fine: no compiled libraries are required
	- [jsoncpp][]
		- prebuilt Visual Studio binaries available at <http://access.osvr.com/binary/deps/jsoncpp>
- An installation of the [Valve Software OpenVR SDK][openvr]
    - This is currently provided as a git submodule.
    - Run `git submodule update --init --recursive` to download the OpenVR SDK.

### Building

#### Configure
Use CMake to configure/generate your build. You'll essentially use CMake just as any other project - that is, something like:

```sh
mkdir build
cd build
cmake ..
```

is a start for the command-line-oriented. Or, if you prefer a GUI, [this screencast demonstrates configurating and building with CMake's GUI](http://academic.cleardefinition.com/2012/05/07/how-to-build-software-using-cmake/).

The one detail is that you'll want to add the variable `CMAKE_PREFIX_PATH` and set it to the root directories/"prefixes" (semicolon-separated) where the dependency binaries are (the directory that immediately contains bin, include, and so on.)

Another configuration setting you'll want to consider is `CMAKE_INSTALL_PREFIX` - you'll probably want to set this to a local directory (like a directory on your desktop), or to the root of a SteamVR/OpenVR tree, since when building the `INSTALL` project (VS) or the `install` target (all other build systems), the driver will get installed into the `drivers/[platform]/` subdirectory of the install prefix. The system-wide SteamVR install directory on Windows is usually something like `C:/Program Files (x86)/Steam/steamapps/common/SteamVR` though install there **at your own risk!**

#### Build
Use your native build tool - open the generated Visual Studio solution, run `make`, etc.

#### Install
As mentioned above, the `install` target will place the driver binary in a specific directory. If that's not a standard directory, or if you don't re-configure SteamVR/OpenVR to look there, you'll need to copy the driver to such a directory manually.

## License and Vendored Projects

- This project: Licensed under the Apache License, Version 2.0.

- Vendored projects included in the source tree:

	- `/vendor/eigen-3.2.4` - Unpacked release from <http://eigen.tuxfamily.org/> - header-only library under the MPL2 (a file-level copyleft, compatible with proprietary software), used with the preprocessor definition `EIGEN_MPL2_ONLY` to exclude modules with other license from the build.

	- `/vendor/libcxx` - Extracted files from [libc++][] - dual-licensed under the MIT and the University of Illinois "BSD-Like" licenses. See that directory for full copyright, license, and credits.

	- `/vendor/util-headers` - Select header files from [util-headers][] - licensed under the Boost Software License v1.0.

	- `/vendor/OSVR-Display` - Used for detecting display parameters. Licensed under Apache 2.0.

	- `/vendor/googletest` - A unit-test framework. Licensed under the [New BSD License](http://www.opensource.org/licenses/bsd-license.php).


[CMake]: http://cmake.org
[OSVR-Core]: https://github.com/OSVR/OSVR-Core
[Boost]: http://boost.org
[jsoncpp]: https://github.com/open-source-parsers/jsoncpp
[openvr]: https://github.com/ValveSoftware/openvr
[libc++]: http://libcxx.llvm.org/
[util-headers]: https://github.com/rpavlik/util-headers
