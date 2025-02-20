@echo off
echo Cleaning CMake cache and rebuilding...

:: Set environment variables
set "VSCMD_START_DIR=%CD%"

:: Kill Visual Studio processes if they are running
taskkill /F /IM devenv.exe /T 2>nul
timeout /t 2 /nobreak >nul

:: Remove build directories and CMake cache
if exist out rmdir /s /q out
if exist build rmdir /s /q build
if exist CMakeCache.txt del /f /q CMakeCache.txt
if exist CMakeSettings.json del /f /q CMakeSettings.json
if exist .vs rmdir /s /q .vs

:: Create build directory and run CMake
mkdir build
cd build

:: Configure CMake for Visual Studio 2019 with Win32
cmake .. -G "Visual Studio 16 2019" -A Win32 ^
    -DCMAKE_GENERATOR_PLATFORM=Win32 ^
    -DCMAKE_BUILD_TYPE=Debug ^
    -DBUILD_WINDOWS_GUI=ON ^
    -DBUILD_DICTS=ON

echo Done! Visual Studio solution has been regenerated.
cd ..
pause 