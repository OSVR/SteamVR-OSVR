#!/bin/bash

# This script installs jsoncpp on the Travis CI Linux server.

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
if [ -e "${PREFIX}/lib/libjsoncpp.so" ]; then
    echo "jsoncpp is already installed."
    exit 0
fi

# Dependency source directories
mkdir -p ~/source/"${CONFIG}"
pushd ~/source/"${CONFIG}"

# Download and build jsoncpp
if [ -d jsoncpp ]; then
    pushd jsoncpp
    git pull
    popd
else
    git clone --recursive https://github.com/open-source-parsers/jsoncpp.git
fi

# Build jsoncpp
mkdir -p jsoncpp/build
pushd jsoncpp/build
cmake .. -DCMAKE_INSTALL_PREFIX="${PREFIX}" -DBUILD_STATIC_LIBS=ON -DBUILD_SHARED_LIBS=ON -DJSONCPP_WITH_CMAKE_PACKAGE=ON -DCMAKE_BUILD_TYPE="${CONFIG}"
make -j2 install
popd

popd

