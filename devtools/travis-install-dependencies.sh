#!/bin/bash

# This script is executed by Travis CI during the before-install phase.

set -o errexit # exit on first error
set -o nounset # report unset variables
set -o xtrace  # show commands

before_install_linux()
{
    if [ $CC = 'clang' ]; then
        export CXX="clang++-3.7" CC="clang-3.7"
    fi

    export PATH="${PREFIX}"/bin:$PATH

    # Install OSVR-Core, OSVR-RenderManager, and their dependencies
    pushd linux
    ./travis-install-osvr-core.sh "$1"
    ./travis-install-osvr-rendermanager.sh "$1"
    popd
}

before_install_osx()
{
    brew update
    brew tap homebrew/science
    brew tap osvr/osvr

    brew uninstall json-c
    brew install jsoncpp --HEAD
    brew install osvr-core --HEAD
    brew install eigen
    brew install glew

    # Install OSVR-RenderManager
    pushd osx
    ./travis-install-osvr-rendermanager.sh "$1"
    popd
}

if [ $# -ne 1 ]; then
    echo "Usage: $0 <installation prefix>"
    exit 1
fi

export PREFIX="$1"
mkdir -p "${PREFIX}"


if [ $CC = gcc ]; then
    export CXX="g++-5" CC="gcc-5"
fi

case "$(uname)" in
    Linux)
        before_install_linux "${PREFIX}"
        ;;
    Darwin)
        before_install_osx "${PREFIX}"
        ;;
    *)
        echo "Unknown operating system: $(uname). Only Linux and Darwin are supported."
        exit 1
        ;;
esac

