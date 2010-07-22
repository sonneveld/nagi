#!/bin/bash

mkdir build
mkdir build/Release
mkdir build/Debug
mkdir build/Windows

pushd build/Release
cmake -DCMAKE_BUILD_TYPE=Release ../../src
popd

pushd build/Debug
cmake -DCMAKE_BUILD_TYPE=Debug ../../src
popd

# see http://www.itk.org/Wiki/CmakeMingw
# and http://www.cmake.org/Wiki/CMake_Cross_Compiling
pushd build/Windows
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../../src/Toolchain-mingw32.cmake ../../src
popd
