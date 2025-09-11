#!/bin/bash
# Alternative macOS build script using create-dmg

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

# Check if create-dmg is available
if command -v create-dmg >/dev/null 2>&1; then
    echo "Using create-dmg to create DMG..."
    create-dmg \
        --volname "cpman 0.1.0" \
        --window-pos 200 120 \
        --window-size 600 300 \
        --icon-size 100 \
        --icon "cpman.app" 175 120 \
        --hide-extension "cpman.app" \
        --app-drop-link 425 120 \
        "${BUILD_DIR}/cpman-0.1.0-macOS.dmg" \
        "${BUILD_DIR}/app/cpman.app"
else
    echo "create-dmg not found, using hdiutil..."
    # Create a temporary directory for DMG contents
    DMG_DIR="${BUILD_DIR}/dmg_contents"
    rm -rf "$DMG_DIR"
    mkdir -p "$DMG_DIR"
    
    # Copy the app bundle
    cp -R "${BUILD_DIR}/app/cpman.app" "$DMG_DIR/"
    
    # Create Applications symlink
    ln -s /Applications "$DMG_DIR/Applications"
    
    # Create DMG
    hdiutil create -volname "cpman 0.1.0" -srcfolder "$DMG_DIR" -ov -format UDZO "${BUILD_DIR}/cpman-0.1.0-macOS.dmg"
    
    # Clean up
    rm -rf "$DMG_DIR"
fi

echo "Build complete! DMG file created in: $BUILD_DIR"
echo "Available DMG files:"
ls -la "$BUILD_DIR"/*.dmg