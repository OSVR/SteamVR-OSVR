#!/bin/bash

# This script installs OSVR-Core on the Travis CI Linux server.

# Parameters:
#  $1  Installation prefix

set -o errexit # exit on first error
set -o nounset # report unset variables
set -o xtrace  # show commands

if [ $# -ne 1 ]; then
    echo "Usage: $0 <installation prefix>"
    exit 1
fi

export PREFIX="$1"
mkdir -p "${PREFIX}"

# Build OSVR-Core dependencies
./travis-install-cmake.sh "${PREFIX}"
./travis-install-jsoncpp.sh "${PREFIX}"
./travis-install-libfunctionality.sh "${PREFIX}"
#./travis-install-opencv.sh "${PREFIX}"

# If already installed, skip the rest
if [ -e "${PREFIX}/lib/libosvrPluginKit.so" ]; then
    echo "OSVR is already installed."
    exit 0
fi

# Dependency source directories
mkdir -p ~/source/"${CONFIG}"
pushd ~/source/"${CONFIG}"

# Download and build OSVR-Core
if [ -d OSVR-Core ]; then
    pushd OSVR-Core
    git pull
    git submodule update --init --recursive
    popd
else
    git clone --recursive https://github.com/OSVR/OSVR-Core.git
fi

# Build OSVR-Core
mkdir -p OSVR-Core/build
pushd OSVR-Core/build
cmake .. -DCMAKE_PREFIX_PATH="${PREFIX}" -DCMAKE_INSTALL_PREFIX="${PREFIX}" \
    -DBUILD_CLIENT=ON \
    -DBUILD_CLIENT_APPS=OFF \
    -DBUILD_CLIENT_EXAMPLES=OFF \
    -DBUILD_DEV_VERBOSE=OFF \
    -DBUILD_EXPERIMENTAL_APPS=OFF \
    -DBUILD_HEADER_DEPENDENCY_TESTS=OFF \
    -DBUILD_SERVER=ON \
    -DBUILD_SERVER_APP=OFF \
    -DBUILD_SERVER_EXAMPLES=OFF \
    -DBUILD_SERVER_PLUGINS=OFF \
    -DBUILD_TESTING=OFF \
    -DBUILD_WITH_OPENCV=OFF
make -j2 install
popd

popd

