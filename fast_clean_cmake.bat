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

echo Done! Visual Studio solution has been regenerated.
cd ..
pause 