# SteamVR Driver Using OSVR 
> Maintained at <https://github.com/OSVR/SteamVR-OSVR>
>
> For details, see <http://osvr.github.io>
>
> For support, see <http://support.osvr.com>

This is a SteamVR driver for allowing applications written against that API to work with hardware and software running with the OSVR software framework.

Note that despite similar goals, the internal models of the two systems are not the same (in most cases, OSVR has a more flexible, descriptive model), so if you're writing an application from scratch, be sure to evaluate that. This driver exists primarily for compatibility reasons (a bit like the Unity or Unreal integration, but more general), so existing software using the SteamVR system can run on OSVR. *OSVR ClientKit itself (along with its wrappers/engine integrations) is and remains the first-class preferred API for working with OSVR.*

## Build Instructions
### Prerequisites
- [CMake][] v3.1 or newer, latest version always recommended
- A compiler supporting C++11
	-	On Windows, Visual Studio 2013 recommended.
	- Recent compilers on Linux are fine too.
	- Mac OS X not tested/supported yet - feel free to pitch in and help change this!
- Somehow create or acquire a build of the following (make sure the bits match for everything that's not header-only):
	- [OSVR-Core][]
		- Client libraries are sufficient if you're building from scratch for this purpose only.
		- Windows snapshot builds are available at <http://access.osvr.com/binary/osvr-core>
	- [Boost][]
		- headers-only is fine: no compiled libraries are required
	- [jsoncpp][]
		- prebuilt Visual Studio binaries available at <http://access.osvr.com/binary/deps/jsoncpp>
- An installation of the [Valve Software OpenVR SDK][openvr] - right now the repo is a binary distribution, so you can just clone the repo.
	- If you name the directory `openvr` (which is the default when cloning with git) and place it next to or within this project's root directory, CMake will be able to find it automatically.
	- As of the moment this file is committed, the latest version works (and may be required). They're iterating the SDK and changing APIs rapidly, so if you have compile issues, you may have to look at the git history and either use an older version or update this project to match.

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

- `make_unique.h`: Dual-licensed under the MIT and University of Illinois Open Source Licenses.

- Vendored projects included in the source tree:

	- `/vendor/eigen-3.2.4` - Unpacked release from <http://eigen.tuxfamily.org/> - header-only library under the MPL2 (a file-level copyleft, compatible with proprietary software), used with the preprocessor definition `EIGEN_MPL2_ONLY` to exclude modules with other license from the build.

[CMake]: http://cmake.org
[OSVR-Core]: https://github.com/OSVR/OSVR-Core
[Boost]: http://boost.org
[jsoncpp]: https://github.com/open-source-parsers/jsoncpp
[openvr]: https://github.com/ValveSoftware/openvr
