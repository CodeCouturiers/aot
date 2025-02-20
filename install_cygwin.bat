@echo off
setlocal enabledelayedexpansion

echo Starting Cygwin installation...

:: Create directories
if not exist "external" (
    mkdir external
    echo Created external directory
)
if not exist "external\cygwin" (
    mkdir external\cygwin
    echo Created cygwin directory
)

:: Check if we have a mirror
if not exist "external\cygwin\mirror.txt" (
    echo No mirror found. Please run get_cygwin_mirror.bat or find_best_cygwin_mirror.bat first
    exit /b 1
)

:: Get mirror from file
set /p MIRROR=<external\cygwin\mirror.txt

:: Navigate to the directory
cd external\cygwin

:: Download Cygwin setup
echo Downloading Cygwin setup...
powershell -Command "& {[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; Invoke-WebRequest -Uri 'https://www.cygwin.com/setup-x86_64.exe' -OutFile 'setup-x86_64.exe'}"

if not exist "setup-x86_64.exe" (
    echo Failed to download Cygwin setup
    exit /b 1
)

:: Create installation directory if it doesn't exist
if not exist "C:\cygwin64" (
    mkdir "C:\cygwin64"
)

echo Installing Cygwin with required packages...
echo Using mirror: %MIRROR%
setup-x86_64.exe ^
    --site "%MIRROR%" ^
    --root "C:\cygwin64" ^
    --packages wget,gzip,tar,unzip,bash ^
    --quiet-mode ^
    --only-site ^
    --no-desktop ^
    --no-shortcuts ^
    --no-startmenu ^
    --no-verify ^
    --no-admin

:: Check if installation succeeded
if errorlevel 1 (
    echo Cygwin installation failed
    exit /b 1
)

:: Add Cygwin to PATH if not already present
echo Checking PATH...
set "cygPath=C:\cygwin64\bin"
powershell -Command "& {if ($env:Path -split ';' -notcontains '%cygPath%') { [Environment]::SetEnvironmentVariable('Path', $env:Path + ';%cygPath%', 'User'); }}"

:: Test installation
echo Testing Cygwin installation...
C:\cygwin64\bin\wget.exe --version > nul 2>&1
if errorlevel 1 (
    echo WARNING: wget not accessible. Please add C:\cygwin64\bin to your PATH manually.
) else (
    echo wget test successful
)

C:\cygwin64\bin\gzip.exe --version > nul 2>&1
if errorlevel 1 (
    echo WARNING: gzip not accessible. Please add C:\cygwin64\bin to your PATH manually.
) else (
    echo gzip test successful
)

echo Installation complete!
echo Please restart your command prompt for PATH changes to take effect.

pause 