#!/bin/bash

# This script installs libfunctionality on the Travis CI Linux server.

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
if [ -e "${PREFIX}/lib/libfunctionality.so" ]; then
    echo "libfunctionality is already installed."
    exit 0
fi

# Dependency source directories
mkdir -p ~/source/"${CONFIG}"
pushd ~/source/"${CONFIG}"

# Download and build libfunctionality
if [ -d libfunctionality ]; then
    pushd libfunctionality
    git pull
    popd
else
    git clone --recursive https://github.com/OSVR/libfunctionality.git
fi

# Build libfunctionality
mkdir -p libfunctionality/build
pushd libfunctionality/build
cmake .. -DCMAKE_INSTALL_PREFIX="${PREFIX}" -DCMAKE_BUILD_TYPE="${CONFIG}"
make -j2 install
popd

popd

