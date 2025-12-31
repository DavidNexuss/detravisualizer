#!/bin/bash
set -eo pipefail # 'e' stops on error, 'o pipefail' catches errors in pipes

# --- Helper Functions ---

# Colors for minimal output
GREEN='\033[0;32m'
RED='\033[0;31m'
CYAN='\033[0;36m'
NC='\033[0m'
log() { echo -e "${CYAN}--> $1${NC}"; }
ok() { echo -e "${GREEN}✔ $1${NC}"; }
fail() { echo -e "${RED}✖ $1${NC}"; exit 1; }

# --- Setup OpenMP Compiler for macOS ---

log "Checking OpenMP dependencies (llvm/libomp)..."
if ! brew list llvm &>/dev/null; then
    log "Installing llvm and libomp via Homebrew..."
    brew install llvm libomp || fail "Homebrew installation failed."
fi

# Set Homebrew Clang as the default compiler for CMake
LLVM_PATH=$(brew --prefix llvm)
export CC="${LLVM_PATH}/bin/clang"
export CXX="${LLVM_PATH}/bin/clang++"
ok "Compilers set to Homebrew Clang."

# --- Project Setup and Configuration ---

log "Checking out LFS files..."
git lfs install >/dev/null
git lfs fetch >/dev/null
git lfs checkout >/dev/null
ok "LFS files checked out."

log "Updating git submodules..."
git submodule update --init --recursive || fail "Submodule update failed."
ok "Submodules updated."

# Simplified build directory and configuration loop
BUILD_DIRS=("build/release" "build/debug")

for dir in "${BUILD_DIRS[@]}"; do
    build_type=$(basename "$dir")
    log "Configuring $build_type build in '$dir'..."
    
    # Create directory only if it doesn't exist (no verbose check)
    mkdir -p "$dir"
    
    # Configure CMake
    cmake -S . -B "$dir" -DCMAKE_BUILD_TYPE="$build_type" \
          -DCMAKE_POLICY_VERSION_MINIMUM=3.5 >/dev/null || \
          fail "CMake configuration for $build_type failed."
    
    ok "$build_type configured."

    # Create symlinks
    log "Creating symlinks for $build_type..."
    ln -sf ../../assets "$dir/assets"
done

# Create compile_commands.json symlink from the release build
ln -sf build/release/compile_commands.json compile_commands.json
ok "All symlinks created (compile_commands.json linked)."
ok "Build setup complete! 🎉"
