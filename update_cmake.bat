@echo off
setlocal enabledelayedexpansion

echo Checking Visual Studio 2019 Community installation...

:: Check for VS2019 Community installation directory
set "VS2019_DIR=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community"

if not exist "%VS2019_DIR%" (
    echo Visual Studio 2019 Community not found
    echo Please make sure Visual Studio 2019 Community is installed
    exit /b 1
)

:: Check current CMake version
echo Checking CMake version...
"%VS2019_DIR%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" --version > cmake_version.txt
set /p CMAKE_VERSION=<cmake_version.txt
del cmake_version.txt

echo Current CMake version: %CMAKE_VERSION%
echo Required CMake version: 3.24 or higher

echo.
echo To update CMake in Visual Studio 2019 Community:
echo 1. Open Visual Studio Installer
echo 2. Select "Modify" for Visual Studio 2019 Community
echo 3. Go to "Individual Components"
echo 4. Find and check "CMake 3.24 or later"
echo 5. Click "Modify" to update

echo.
echo Would you like to open Visual Studio Installer now? (Y/N)
choice /C YN /M "Open Visual Studio Installer"
if errorlevel 2 goto :end
if errorlevel 1 goto :open_installer

:open_installer
start "" "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vs_installer.exe"

:end
echo.
echo After updating:
echo 1. Restart Visual Studio
echo 2. Clear CMake cache: Project -^> Clear CMake Cache
echo 3. Regenerate CMake project

pause 