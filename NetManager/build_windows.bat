@echo off
setlocal

for %%I in ("%~dp0.") do set "PROJECT_ROOT=%%~fI"
set "VCPKG_PATH=%PROJECT_ROOT%\..\..\vcpkg"
set "BUILD_PRESET=windows-debug"
set "VCPKG_TRIPLET=x64-windows"

echo [NetManager] Project root: %PROJECT_ROOT%
echo [NetManager] vcpkg path: %VCPKG_PATH%
echo [NetManager] Build preset: %BUILD_PRESET%
echo [NetManager] vcpkg triplet: %VCPKG_TRIPLET%

for %%I in ("%VCPKG_PATH%") do set "VCPKG_PATH=%%~fI"
echo [NetManager] Resolved vcpkg path: %VCPKG_PATH%

if not exist "%VCPKG_PATH%" (
    echo [NetManager] vcpkg was not found. Cloning vcpkg...
    git clone https://github.com/microsoft/vcpkg "%VCPKG_PATH%"
    if errorlevel 1 goto fail
) else (
    echo [NetManager] vcpkg already exists. Skipping clone.
)

set "VCPKG_TOOLCHAIN=%VCPKG_PATH%\scripts\buildsystems\vcpkg.cmake"
if not exist "%VCPKG_PATH%\vcpkg.exe" (
    echo [NetManager] Bootstrapping vcpkg...
    call "%VCPKG_PATH%\bootstrap-vcpkg.bat"
    if errorlevel 1 goto fail
) else (
    echo [NetManager] vcpkg toolchain already exists. Skipping bootstrap.
)

if not exist "%VCPKG_TOOLCHAIN%" (
    echo [NetManager] vcpkg toolchain file not found: %VCPKG_TOOLCHAIN%
    goto fail
)

cd /d "%PROJECT_ROOT%"
if errorlevel 1 goto fail

echo [NetManager] Configuring CMake...
cmake --preset "%BUILD_PRESET%"
if errorlevel 1 goto fail

echo [NetManager] Building CMake target...
cmake --build --preset "%BUILD_PRESET%"
if errorlevel 1 goto fail

echo [NetManager] Build finished.
pause
exit /b 0

:fail
echo [NetManager] Build failed.
pause
exit /b 1
