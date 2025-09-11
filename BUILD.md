# 构建说明

## 快速开始

### macOS DMG 构建
```bash
# 直接运行 macOS 构建脚本
./scripts/macos/build.sh

# 或使用通用构建脚本
./scripts/build.sh macos
```

### Windows 安装包构建
```bash
# 在 Windows 上运行
scripts\windows\build.bat

# 或使用通用构建脚本
./scripts/build.sh windows
```

### Linux AppImage 构建
```bash
# 直接运行 Linux 构建脚本
./scripts/linux/build.sh

# 或使用通用构建脚本
./scripts/build.sh linux
```

### 构建所有平台
```bash
./scripts/build.sh all
```

### 清理构建文件
```bash
./scripts/build.sh clean
```

## 系统要求

### macOS
- macOS 12.0 或更高版本
- Xcode 命令行工具
- CMake 3.20 或更高版本
- Qt 6.5 或更高版本

### Windows
- Windows 10 或更高版本
- Visual Studio 2022
- CMake 3.20 或更高版本
- Qt 6.5 或更高版本
- NSIS（用于创建安装包）

### Linux
- Ubuntu 22.04 或其他现代 Linux 发行版
- GCC 9 或更高版本
- CMake 3.20 或更高版本
- Qt 6.5 或更高版本
- linuxdeploy（可选，用于创建 AppImage）
- Flatpak（可选，用于创建 Flatpak 包）

## 输出文件

构建完成后，您可以在以下位置找到生成的包：

- **macOS**: `build-macos/cpman-0.1.0-macOS.dmg`
- **Windows**: `build-windows/cpman-0.1.0-Windows.exe`
- **Linux**: `build-linux/cpman-0.1.0-Linux.AppImage`

## 故障排除

### 常见问题

1. **找不到 Qt**
   - 确保 Qt 6.5 或更高版本已安装
   - 设置 `Qt6_DIR` 环境变量指向 Qt 安装目录

2. **Windows 上找不到 Visual Studio**
   - 安装 Visual Studio 2022 并确保 C++ 工具链已安装
   - 在开发人员命令提示符中运行构建脚本

3. **Linux 上缺少依赖**
   - 安装必要的开发包：
     ```bash
     sudo apt-get install build-essential cmake qt6-base-dev libqt6sql6-sqlite
     ```

4. **macOS 上代码签名问题**
   - 如果需要代码签名，请在构建前配置开发者证书

## 跨平台构建

如果您需要在非 Windows 系统上构建 Windows 版本，可以使用 MinGW 工具链：

```bash
# 安装 MinGW 工具链
sudo apt-get install mingw-w64

# 使用工具链文件构建
mkdir build-windows-cross
cd build-windows-cross
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../scripts/windows/toolchain.cmake
make -j$(nproc)
```

## 自定义构建

您可以通过以下环境变量自定义构建：

- `CMAKE_BUILD_TYPE`: Debug, Release, RelWithDebInfo, MinSizeRel
- `Qt6_DIR`: Qt 6 安装目录
- `CMAKE_PREFIX_PATH`: CMake 搜索路径

例如：
```bash
export CMAKE_BUILD_TYPE=Debug
export Qt6_DIR=/path/to/qt6
./scripts/build.sh macos
```