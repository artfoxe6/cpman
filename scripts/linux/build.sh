#!/bin/bash
# Linux AppImage build script

set -e

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
BUILD_DIR="$PROJECT_DIR/build-linux"
APPIMAGE_DIR="$BUILD_DIR/AppImage"

echo "Building cpman for Linux..."
echo "Project directory: $PROJECT_DIR"
echo "Build directory: $BUILD_DIR"

# Ensure Qt6 is installed
if ! command -v qmake >/dev/null 2>&1 || ! qmake -query QT_VERSION 2>/dev/null | grep -q '^6'; then
    echo "Qt6 not found, attempting to install..."
    if command -v apt-get >/dev/null 2>&1; then
        sudo apt-get update && sudo apt-get install -y qt6-base-dev
    elif command -v yum >/dev/null 2>&1; then
        sudo yum install -y qt6-qtbase-devel
    else
        echo "No supported package manager (apt or yum) found. Please install Qt6 manually."
        exit 1
    fi
fi

# Clean and create build directory
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

# Configure and build
cd "$BUILD_DIR"
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Create AppImage directory structure
mkdir -p "$APPIMAGE_DIR/usr/bin"
mkdir -p "$APPIMAGE_DIR/usr/lib"
mkdir -p "$APPIMAGE_DIR/usr/share/applications"
mkdir -p "$APPIMAGE_DIR/usr/share/icons/hicolor/256x256/apps"
mkdir -p "$APPIMAGE_DIR/usr/share/metainfo"

# Copy executable
cp "$BUILD_DIR/cpman" "$APPIMAGE_DIR/usr/bin/"

# Copy icon
cp "$PROJECT_DIR/app/icons/linux/256x256/apps/cpman.png" "$APPIMAGE_DIR/usr/share/icons/hicolor/256x256/apps/"

# Copy Qt libraries and dependencies
echo "Copying Qt dependencies..."
# Use linuxdeploy to bundle dependencies
if command -v linuxdeploy >/dev/null 2>&1; then
    echo "Using linuxdeploy to bundle dependencies..."
    wget -q https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
    chmod +x linuxdeploy-x86_64.AppImage
    
    wget -q https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
    chmod +x linuxdeploy-plugin-qt-x86_64.AppImage
    
    ./linuxdeploy-x86_64.AppImage --appdir "$APPIMAGE_DIR" --plugin qt --output appimage
else
    echo "linuxdeploy not found, creating simple AppImage..."
    # Create desktop file
    cat > "$APPIMAGE_DIR/usr/share/applications/cpman.desktop" << EOF
[Desktop Entry]
Name=cpman
Comment=Cross-platform clipboard history manager
Exec=cpman
Icon=cpman
Type=Application
Categories=Utility;
StartupNotify=false
EOF

    # Create AppRun script
    cat > "$APPIMAGE_DIR/AppRun" << EOF
#!/bin/bash
SELF=\$(readlink -f "\$0")
HERE=\dirname "\$SELF"
export LD_LIBRARY_PATH="\$HERE/usr/lib:\$LD_LIBRARY_PATH"
exec "\$HERE/usr/bin/cpman" "\$@"
EOF
    chmod +x "$APPIMAGE_DIR/AppRun"
fi

echo "AppImage build complete!"
echo "AppImage created in: $BUILD_DIR"
