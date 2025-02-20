@echo off
setlocal enabledelayedexpansion

echo Upgrading win_flex_bison...

:: Create temporary directory
if not exist temp mkdir temp
cd temp

:: Download win_flex_bison
echo Downloading win_flex_bison-2.5.25...
powershell -Command "& {Invoke-WebRequest -Uri 'https://github.com/lexxmark/winflexbison/releases/download/v2.5.25/win_flex_bison-2.5.25.zip' -OutFile 'win_flex_bison.zip'}"
if %ERRORLEVEL% neq 0 (
    echo Failed to download win_flex_bison
    exit /b 1
)

:: Extract the archive
echo Extracting files...
powershell -Command "& {Expand-Archive -Path 'win_flex_bison.zip' -DestinationPath '.' -Force}"
if %ERRORLEVEL% neq 0 (
    echo Failed to extract archive
    exit /b 1
)

cd ..

:: Copy necessary files
echo Copying files...
xcopy /Y /Q "temp\win_flex_bison-2.5.25\win_bison.exe" "." >nul
xcopy /Y /Q "temp\win_flex_bison-2.5.25\win_flex.exe" "." >nul
xcopy /Y /Q "temp\win_flex_bison-2.5.25\FlexLexer.h" "." >nul
xcopy /Y /Q "temp\win_flex_bison-2.5.25\README.md" "." >nul
xcopy /Y /Q "temp\win_flex_bison-2.5.25\changelog.md" "." >nul

:: Copy data directory with all subdirectories
echo Copying data directory...
if exist data rd /S /Q data
xcopy /E /I /Y "temp\win_flex_bison-2.5.25\data" "data" >nul

:: Clean up
echo Cleaning up...
rd /S /Q temp

echo.
echo Upgrade completed successfully!
echo win_flex_bison has been updated to version 2.5.25

endlocal 