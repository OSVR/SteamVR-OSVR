#!/bin/bash

# This script installs Oculus SDK on the Travis CI OS X server.

# Parameters:
#  $1  Installation prefix

set -o errexit # exit on first error
set -o nounset # report unset variables
set -o xtrace  # show commands

export OVR_VERSION=0.5.0.1

if [ $# -ne 1 ]; then
    echo "Usage: $0 <installation prefix>"
    exit 1
fi

export PREFIX="$1"
mkdir -p "${PREFIX}"

# Dependency source directories
mkdir -p ~/source/"${CONFIG}"
pushd ~/source/"${CONFIG}"

# Download and build Oculus SDK
if [ -d ovr_sdk_osx_-${OVR_VERSION} ]; then
    echo "Oculus SDK $OVR_VERSION has already been installed."
    exit 0
fi

# Build Oculus SDK
mkdir -p ovr_sdk
pushd ovr_sdk
#curl -LR https://static.oculus.com/sdk-downloads/ovr_sdk_macos_${OVR_VERSION}.tar.gz -o ovr_sdk_macos_${OVR_VERSION}.tar.gz
curl -LR https://kevin.godby.org/oculus/ovr_sdk_macos_${OVR_VERSION}.tar.gz -o ovr_sdk_macos_${OVR_VERSION}.tar.gz
tar xvf ovr_sdk_macos_${OVR_VERSION}.tar.gz
pushd OculusSDK

# Strip out the -Werror compiler flag from makefiles
for f in $(find . -iname "*.mk"); do sed -i.bak -e 's/-Werror//g' $f; done

TARGET=$(echo ${CONFIG} | tr [:upper:] [:lower:])
#make PREFIX="${PREFIX}" CC="${CC}" CXX="${CXX}" ${TARGET} install -j2
popd

cp -av OculusSDK "${PREFIX}"/

popd

popd

