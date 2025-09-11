# Windows cross-compilation toolchain file
# Used for building Windows executables on Linux/macOS

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_VERSION 10.0)

# Specify the cross compiler
set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

# Target environment
set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)

# Adjust the default behavior of the FIND_XXX() commands:
# search headers and libraries in the target environment, search 
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Qt6 configuration for Windows
set(QT6_INSTALL_PREFIX /usr/x86_64-w64-mingw32/lib/qt6)
set(Qt6_DIR ${QT6_INSTALL_PREFIX}/lib/cmake/Qt6)

# Windows specific flags
add_compile_definitions(WIN32_LEAN_AND_MEAN)
add_compile_definitions(NOMINMAX)
add_compile_definitions(CP_PLATFORM_WINDOWS)