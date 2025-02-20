@echo off
setlocal enabledelayedexpansion

echo Starting WinFlexBison installation...

:: Create directories
if not exist "external" (
    mkdir external
    echo Created external directory
)
if not exist "external\winflex" (
    mkdir external\winflex
    echo Created winflex directory
)

:: Navigate to the directory
cd external\winflex

:: Clean any existing files
if exist "*.exe" del *.exe
if exist "*.dll" del *.dll

:: Download WinFlexBison using direct link
echo Downloading WinFlexBison...
powershell -Command "& {[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; Invoke-WebRequest -Uri 'https://github.com/lexxmark/winflexbison/releases/download/v2.5.24/win_flex_bison-2.5.24.zip' -OutFile 'win_flex_bison.zip'}"

if not exist "win_flex_bison.zip" (
    echo Failed to download WinFlexBison
    exit /b 1
)

:: Extract the archive using tar (available in modern Windows)
echo Extracting files...
tar -xf win_flex_bison.zip

:: Clean up
echo Cleaning up...
del win_flex_bison.zip

:: Copy executables and necessary files from data directory
echo Moving files...
if exist "data\*" (
    echo Copying executable files...
    for %%f in (data\*.exe data\*.dll) do (
        if exist "%%f" (
            echo Moving %%f
            move /Y "%%f" . > nul
        )
    )
    :: Remove data directory and its contents
    echo Removing data directory...
    rd /s /q data
)

:: Add to PATH if not already present
echo Checking PATH...
set "flexPath=%CD%"
powershell -Command "& {if ($env:Path -split ';' -notcontains '%flexPath%') { [Environment]::SetEnvironmentVariable('Path', $env:Path + ';%flexPath%', 'User'); }}"

:: Create test batch files to ensure proper execution
echo @echo off > test_flex.bat
echo win_flex.exe --version >> test_flex.bat
echo @echo off > test_bison.bat
echo win_bison.exe --version >> test_bison.bat

echo Testing installation...
call test_flex.bat
if errorlevel 1 (
    echo WARNING: Flex not accessible from command line. Please add %CD% to your PATH manually.
)
call test_bison.bat
if errorlevel 1 (
    echo WARNING: Bison not accessible from command line. Please add %CD% to your PATH manually.
)

:: Clean up test files
del test_flex.bat
del test_bison.bat

echo Installation complete!
echo Please restart your command prompt for PATH changes to take effect.

pause 