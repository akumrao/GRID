#!/usr/bin/env bash

# Exit immediately if a command exits with a non-zero status
set -e

echo "=========================================================="
# Step 1: Pre-flight check for dependencies
echo "[1/4] Checking build tools..."
if ! command -v conan &> /dev/null; then
    echo "ERROR: Conan 2.x package manager is not installed or added to PATH."
    exit 1
fi

if ! command -v cmake &> /dev/null; then
    echo "ERROR: CMake is not installed or added to PATH."
    exit 1
fi

# Step 2: Establish isolated directory spaces
echo "[2/4] Cleaning old workspaces and building fresh build layout..."
rm -rf build

# Step 3: Run Conan 2.x profile installation
echo "[3/4] Installing third-party modules (IXWebSocket, nlohmann_json) via ConanCenter..."
# conan install .. --output-folder=. --build=missing -s build_type=Release


# # Step 4: Configure CMake project matching structural Conan output configurations
# echo "[4/4] Configuring toolchains and executing build compilation mapping..."
# cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
# cmake --build . --config Release


conan install . -s '&:build_type=Debug' -s build_type=Release --output-folder=build

cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug

cmake --build build




echo "=========================================================="
echo "BUILD SUCCESSFUL! Executable target location: build/supabase_host"
echo "=========================================================="
