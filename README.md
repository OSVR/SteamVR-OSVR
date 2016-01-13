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

- Upon extracting the OSVR driver, you'll get a `SteamVR-OSVR` directory, with a `drivers` directory inside it (with additional directories and files deeper inside `drivers`).

You'll want to drag the `drivers` directory from the `SteamVR-OSVR` extracted folder over into the Steam directory, to merge its contents with the existing contents of the `drivers` directory. (It should just make an `osvr` subdirectory with contents.)

To know if you got it right, do the same Windows Run (or other way of opening a folder by its name) as above, with the path `"%ProgramFiles(x86)%\Steam\steamapps\common\SteamVR\drivers\osvr\bin\win32"` instead. (Make changes as appropriate to suit your Steam install location.) If you got it right, you should see a number of files, including one called `driver_osvr.dll`.

### Usage

In all cases, hook up your hardware and launch OSVR Server first.

**Simple:** Paste `"%ProgramFiles(x86)%\Steam\steamapps\common\SteamVR\demo\bin\win32\hellovr_sdl.exe"` into the Windows Run dialog and click run to start the SteamVR simple demo app - you should get a command-prompt window with some text, the screen might flicker, then you should get a split-screen display that reacts to the movement of the HMD by changing your view of a world with infinite cubes of test patterns. (You might need to click to bring the demo window to the foreground.)

**Real game:** You may wish to open the "VRMonitor" - the SteamVR status window, that should be accessible through the Start Menu. At this time, it appears to also launch Steam in VR or "Big Picture" mode, which may or may not be what you want, but you can close that interface and keep the VR Monitor open. It might say "Not Ready", but if you hover over the HMD icon it should say "HMD Connected, but not ready" - that's normal.

Different games have different ways of turning on VR mode - some you right-click on them in Steam and specify in the options to pass `/vr` to launch in VR mode. You might find info about this on the Internet - many of these sources are outdated: for example, no command line switch is required for TF2 to run in VR.

In this example, we'll use TF2, which has it accessible in its menu: go to "Options", then "Video" and change "Virtual Reality Mode" to "Enabled". You'll have to save that setting and restart the game before you'll see a new entry on the game's main menu called "Activate Virtual Reality". Selecting "Activate Virtual Reality" should flip the game into VR mode and you're ready to play. (Might be best to use a gamepad, as WASD and mouselook don't go well with VR, the mouselook portion in particular.)


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

[CMake]: http://cmake.org
[OSVR-Core]: https://github.com/OSVR/OSVR-Core
[Boost]: http://boost.org
[jsoncpp]: https://github.com/open-source-parsers/jsoncpp
[openvr]: https://github.com/ValveSoftware/openvr
[libc++]: http://libcxx.llvm.org/
[util-headers]: https://github.com/rpavlik/util-headers
