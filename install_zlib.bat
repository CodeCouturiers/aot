@echo off
setlocal enabledelayedexpansion

:: Check if running as administrator
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo This script requires administrator privileges.
    echo Please run as administrator.
    pause
    exit /b 1
)

:: Set paths
if not defined RML (
    echo Error: RML environment variable is not set
    pause
    exit /b 1
)

set EXTERNAL_DIR=%RML%\external
set ZLIB_DIR=%EXTERNAL_DIR%\zlib
set ZLIB_BUILD=%ZLIB_DIR%\build
set ZLIB_INSTALL=%ZLIB_DIR%\install
set ZLIB_VERSION=1.3

:: Create directories
if not exist "%EXTERNAL_DIR%" mkdir "%EXTERNAL_DIR%"
if not exist "%ZLIB_DIR%" mkdir "%ZLIB_DIR%"
if not exist "%ZLIB_INSTALL%" mkdir "%ZLIB_INSTALL%"

:: Download ZLIB if not exists
if not exist "%ZLIB_DIR%\zlib-%ZLIB_VERSION%.zip" (
    echo Downloading ZLIB %ZLIB_VERSION%...
    powershell -Command "(New-Object Net.WebClient).DownloadFile('https://github.com/madler/zlib/archive/refs/tags/v%ZLIB_VERSION%.zip', '%ZLIB_DIR%\zlib-%ZLIB_VERSION%.zip')"
)

:: Extract ZLIB
cd "%ZLIB_DIR%"
if not exist "%ZLIB_DIR%\zlib-%ZLIB_VERSION%" (
    echo Extracting ZLIB...
    powershell -Command "Expand-Archive -Force -Path 'zlib-%ZLIB_VERSION%.zip' -DestinationPath '%ZLIB_DIR%'"
    ren "zlib-%ZLIB_VERSION%" "zlib-src"
)

:: Create build directory
if not exist "%ZLIB_BUILD%" mkdir "%ZLIB_BUILD%"
cd "%ZLIB_BUILD%"

:: Configure and build ZLIB
echo Configuring ZLIB...
cmake -G "Visual Studio 16 2019" -A x64 ^
    -DCMAKE_INSTALL_PREFIX="%ZLIB_INSTALL%" ^
    -DCMAKE_BUILD_TYPE=Release ^
    "%ZLIB_DIR%\zlib-src"

echo Building ZLIB...
cmake --build . --config Release
cmake --install . --config Release

:: Set ZLIB_ROOT environment variable
setx ZLIB_ROOT "%ZLIB_INSTALL%" /M

echo.
echo Installation complete!
echo ZLIB_ROOT has been set to: %ZLIB_INSTALL%
echo.
echo Please restart Visual Studio and any command prompts for the changes to take effect.
pause