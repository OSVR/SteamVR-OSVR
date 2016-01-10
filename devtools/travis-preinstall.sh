#!/usr/bin/env bash

# This script downloads, builds, and installs the SteamVR-OSVR dependencies
# It's designed to be used primarily by Travis CI in conjunction with the
# .travis.yml script.

set -ev

# Download dependencies
if [ $TRAVIS_OS_NAME = 'linux' ]; then
    pushd $HOME
    git clone --recursive https://github.com/open-source-parsers/jsoncpp.git
    git clone --recursive https://github.com/OSVR/libfunctionality.git
    git clone --recursive https://github.com/OSVR/OSVR-Core.git

    wget https://github.com/Itseez/opencv/archive/3.1.0.zip -O opencv-3.1.0.zip
    unzip opencv-3.1.0.zip

    popd # $HOME
fi

if [ $TRAVIS_OS_NAME = 'osx' ]; then
    brew update
    brew tap homebrew/science
    brew tap d235j/osvr
fi

