#!/bin/bash

set -e

# Define the target triplet (for 64-bit Windows)
TARGET=x86_64-w64-mingw32

# Output and build dirs
BUILD_DIR="build-windows"
INSTALL_DIR="install-windows"

# Clean previous builds
rm -rf $BUILD_DIR
mkdir -p $BUILD_DIR
cd $BUILD_DIR

# Configure CMake with MinGW toolchain
cmake .. \
  -DCMAKE_SYSTEM_NAME=Windows \
  -DCMAKE_C_COMPILER=${TARGET}-gcc \
  -DCMAKE_CXX_COMPILER=${TARGET}-g++ \
  -DCMAKE_RC_COMPILER=${TARGET}-windres \
  -DCMAKE_FIND_ROOT_PATH="/usr/${TARGET}" \
  -DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER \
  -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY \
  -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_STANDARD=17 \
  -DUSE_VALIDATION_LAYERS=OFF \
  -DUSE_TRACY=ON \
  -DGLFW_BUILD_WAYLAND=OFF \
  -DGLFW_USE_WAYLAND=OFF \
  -DGLFW_BUILD_X11=OFF \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DOP_DISABLE_HTTP=ON

# Build the project
make -j$(nproc)

# Copy binary to install dir
mkdir -p ../$INSTALL_DIR
cp nexcorp.exe ../$INSTALL_DIR/
