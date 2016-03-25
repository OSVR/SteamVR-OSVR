#!/bin/bash

# This script installs OpenCV on the Travis CI Linux server.

# Parameters:
#  $1  Installation prefix

set -o errexit # exit on first error
set -o nounset # report unset variables
set -o xtrace  # show commands

export OPENCV_VERSION=3.1.0

if [ $# -ne 1 ]; then
    echo "Usage: $0 <installation prefix>"
    exit 1
fi

export PREFIX="$1"
mkdir -p "${PREFIX}"

# If already installed, skip the rest
if [ -e "${PREFIX}/lib/libopencv_core.so" ]; then
    echo "OpenCV is already installed."
    exit 0
fi

# Dependency source directories
mkdir -p ~/source/"${CONFIG}"
pushd ~/source/"${CONFIG}"

# Download and build OpenCV
if [ -e "opencv-${OPENCV_VERSION}/build/lib/libopencv_core.so" ]; then
    echo "OpenCV $OPENCV_VERSION has already been installed."
    exit 0
fi

# Build OpenCV
curl -LR https://github.com/Itseez/opencv/archive/${OPENCV_VERSION}.zip -o opencv-${OPENCV_VERSION}.zip
unzip -qo opencv-${OPENCV_VERSION}.zip
mkdir -p opencv-${OPENCV_VERSION}/build
pushd opencv-${OPENCV_VERSION}/build
cmake .. -DCMAKE_INSTALL_PREFIX="${PREFIX}" -DCMAKE_BUILD_TYPE=${CONFIG} -DWITH_IPP=OFF -DWITH_1394=OFF -DWITH_FFMPEG=OFF
make -j2 install
popd

popd

