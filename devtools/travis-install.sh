#!/usr/bin/env bash

# This script installs the SteamVR-OSVR dependencies
# It's designed to be used primarily by Travis CI in conjunction with the
# .travis.yml script.

set -ev

# Build and install dependences
if [ $TRAVIS_OS_NAME = 'osx' ]; then
    brew uninstall json-c
    brew install libusb opencv
    brew install jsoncpp --HEAD
    brew install libfunctionality --HEAD
    brew install osvr-core --HEAD
fi

if [ $TRAVIS_OS_NAME = 'linux' ]; then
    # Build jsoncpp
    mkdir $HOME/jsoncpp/build
    pushd $HOME/jsoncpp/build
    cmake .. -DBUILD_STATIC_LIBS=ON -DBUILD_SHARED_LIBS=ON -DJSONCPP_WITH_CMAKE_PACKAGE=ON -DCMAKE_BUILD_TYPE=${CONFIG}
    make -j2 install
    popd

    # Build libfunctionality
    mkdir -p $HOME/libfunctionality/build
    pushd $HOME/libfunctionality/build
    cmake .. -DCMAKE_BUILD_TYPE=${CONFIG}
    make -j2 install
    popd

    # Build OSVR-Core
    mkdir -p $HOME/OSVR-Core/build
    pushd $HOME/OSVR-Core/build
    cmake .. -DCMAKE_BUILD_TYPE=${CONFIG} -DCMAKE_PREFIX_PATH=/usr/local
    make -j2 install
    popd
fi

