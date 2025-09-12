#!/bin/bash
# Universal build script for cpman

set -e

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

echo "cpman Universal Build Script"
echo "============================"
echo "Project directory: $PROJECT_DIR"
echo ""

# Function to show usage
show_usage() {
    echo "Usage: $0 [platform]"
    echo ""
    echo "Platforms:"
    echo "  macos    - Build macOS DMG"
    echo "  windows  - Build Windows installer"
    echo "  linux    - Build Linux AppImage"
    echo "  all      - Build all platforms"
    echo "  clean    - Clean all build directories"
    echo ""
    echo "Examples:"
    echo "  $0 macos"
echo "  $0 all"
}

# Function to clean build directories
clean_all() {
    echo "Cleaning all build directories..."
    rm -rf "$PROJECT_DIR/build-macos"
    rm -rf "$PROJECT_DIR/build-windows"
    rm -rf "$PROJECT_DIR/build-linux"
    echo "Clean complete!"
}

# Ensure Qt6 is available, installing it with the appropriate package manager if missing
ensure_qt() {
    if command -v qmake >/dev/null 2>&1 && qmake -query QT_VERSION 2>/dev/null | grep -q '^6'; then
        return
    fi

    echo "Qt6 not found, attempting to install..."
    if [[ "$OSTYPE" == "darwin"* ]]; then
        if command -v brew >/dev/null 2>&1; then
            brew install qt
        else
            echo "Homebrew not found. Please install Homebrew and Qt6 manually."
            exit 1
        fi
    elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
        if command -v apt-get >/dev/null 2>&1; then
            sudo apt-get update && sudo apt-get install -y qt6-base-dev
        elif command -v yum >/dev/null 2>&1; then
            sudo yum install -y qt6-qtbase-devel
        else
            echo "No supported package manager found. Please install Qt6 manually."
            exit 1
        fi
    fi
}

# Function to build for macOS
build_macos() {
    echo "Building for macOS..."
    if [[ "$OSTYPE" == "darwin"* ]]; then
        ensure_qt
        cd "$PROJECT_DIR"
        bash scripts/macos/build_alt.sh
    else
        echo "Error: macOS build can only be run on macOS"
        exit 1
    fi
}

# Function to build for Windows
build_windows() {
    echo "Building for Windows..."
    if command -v cmake >/dev/null 2>&1; then
        cd "$PROJECT_DIR"
        if [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "win32" ]]; then
            cmd //c scripts\\windows\\build.bat
        else
            echo "Note: Windows build should ideally be run on Windows with Visual Studio"
            echo "Attempting cross-platform build..."
            mkdir -p build-windows
            cd build-windows
            cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../scripts/windows/toolchain.cmake
            make -j$(nproc)
        fi
    else
        echo "Error: CMake not found"
        exit 1
    fi
}

# Function to build for Linux
build_linux() {
    echo "Building for Linux..."
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        ensure_qt
        cd "$PROJECT_DIR"
        bash scripts/linux/build.sh
    else
        echo "Error: Linux build can only be run on Linux"
        exit 1
    fi
}

# Parse command line arguments
case "${1:-}" in
    "macos")
        build_macos
        ;;
    "windows")
        build_windows
        ;;
    "linux")
        build_linux
        ;;
    "all")
        echo "Building for all platforms..."
        if [[ "$OSTYPE" == "darwin"* ]]; then
            build_macos
        elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
            build_linux
        fi
        build_windows
        ;;
    "clean")
        clean_all
        ;;
    "help"|"--help"|"-h")
        show_usage
        ;;
    "")
        echo "Error: No platform specified"
        echo ""
        show_usage
        exit 1
        ;;
    *)
        echo "Error: Unknown platform '$1'"
        echo ""
        show_usage
        exit 1
        ;;
esac

echo ""
echo "Build process completed!"
