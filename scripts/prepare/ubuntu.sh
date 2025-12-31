#!/bin/bash
set -e

echo "🔧 Updating package lists..."
sudo apt-get update

echo "📦 Installing system dependencies for Detramotor..."

sudo apt-get install -y \
  build-essential \
  cmake \
  git \
  ninja-build \
  libgl1-mesa-dev \
  libvulkan-dev \
  vulkan-tools \
  libx11-dev \
  libxrandr-dev \
  libxinerama-dev \
  libxcursor-dev \
  libxfixes-dev \
  libxi-dev \
  libwayland-dev \
  xorg-dev \
  xvfb \
  libopenal-dev \
  libogg-dev \
  libopus0 \
  libopus-dev \
  libxml2-dev \
  libglm-dev \
  doxygen \
  pkg-config \
  curl \
  unzip

echo "✅ Dependencies installed successfully!"
