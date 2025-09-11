#!/bin/bash
# macOS build script

set -e

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
BUILD_DIR="$PROJECT_DIR/build-macos"

echo "Building cpman for macOS..."
echo "Project directory: $PROJECT_DIR"
echo "Build directory: $BUILD_DIR"

# Clean and create build directory
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

# Configure and build
cd "$BUILD_DIR"
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_DEPLOYMENT_TARGET=12.0
make -j$(sysctl -n hw.ncpu)

# Create DMG
echo "Creating DMG package..."
cpack -G DragNDrop

echo "Build complete! DMG file created in: $BUILD_DIR"
echo "Available DMG files:"
ls -la "$BUILD_DIR"/*.dmg