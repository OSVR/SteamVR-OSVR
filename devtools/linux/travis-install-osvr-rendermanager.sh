#!/bin/bash

# This script installs OSVR-RenderManager on the Travis CI Linux server.

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

# If already installed, skip the rest
if [ -e "${PREFIX}/lib/libosvrRenderManager.so" ]; then
    echo "OSVR-RenderManager is already installed."
    exit 0
fi

# Dependency source directories
mkdir -p ~/source/"${CONFIG}"
pushd ~/source/"${CONFIG}"

# Download and build OSVR-RenderManager
if [ ! -d OSVR-RenderManager ]; then
    git clone https://github.com/sensics/OSVR-RenderManager.git
fi
pushd OSVR-RenderManager
git pull
git submodule init vendor/vrpn
git submodule update
popd

# Build OSVR-RenderManager
mkdir -p OSVR-RenderManager/build
pushd OSVR-RenderManager/build
cmake .. -DCMAKE_PREFIX_PATH="${PREFIX}" -DCMAKE_INSTALL_PREFIX="${PREFIX}"
make -j2 install
popd

popd

