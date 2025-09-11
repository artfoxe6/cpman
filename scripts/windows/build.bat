@echo off
REM Windows build script

setlocal enabledelayedexpansion

REM Get the directory where this script is located
set SCRIPT_DIR=%~dp0
set PROJECT_DIR=%SCRIPT_DIR%..\..\
set BUILD_DIR=%PROJECT_DIR%build-windows

echo Building cpman for Windows...
echo Project directory: %PROJECT_DIR%
echo Build directory: %BUILD_DIR%

REM Clean and create build directory
if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
mkdir "%BUILD_DIR%"

REM Configure and build
cd "%BUILD_DIR%"
cmake .. -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release --parallel %NUMBER_OF_PROCESSORS%

REM Create installer
echo Creating NSIS installer...
cpack -G NSIS

echo Build complete! Installer created in: %BUILD_DIR%
echo Available installer files:
dir /b "%BUILD_DIR%\*.exe"

pause