#!/bin/bash

# This script is executed by Travis CI during the script (build) phase.

set -o errexit # exit on first error
set -o nounset # report unset variables
set -o xtrace  # show commands

if [ $# -ne 1 ]; then
    echo "Usage: $0 <installation prefix>"
    exit 1
fi

export PREFIX="$1"
mkdir -p "${PREFIX}"


if [ $CC = gcc ]; then
    export CXX="g++-5" CC="gcc-5"
fi

git submodule update --init --recursive

export PATH="${PREFIX}"/bin:$PATH

mkdir -p build
pushd build

if [ "$(uname)" = "Darwin" ]; then
    cmake .. \
        -DCMAKE_C_COMPILER=$(which ${CC}) \
        -DCMAKE_CXX_COMPILER=$(which ${CXX}) \
        -DCMAKE_BUILD_TYPE=${CONFIG} \
        -DCMAKE_PREFIX_PATH=${PREFIX} \
        -DCMAKE_OSX_ARCHITECTURES=x86_64

elif [ "$(uname)" = "Linux" ]; then
    if [ $CC = 'clang' ]; then
        export CXX="clang++-3.7" CC="clang-3.7"
    fi

    cmake .. \
        -DCMAKE_C_COMPILER=$(which ${CC}) \
        -DCMAKE_CXX_COMPILER=$(which ${CXX}) \
        -DCMAKE_BUILD_TYPE=${CONFIG} \
        -DCMAKE_PREFIX_PATH=${PREFIX}

fi

make -j2

popd # build

