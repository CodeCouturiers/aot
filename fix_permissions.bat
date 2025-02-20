@echo off
setlocal enabledelayedexpansion

echo Fixing permissions for RML directories...

REM Create base directories if they don't exist
mkdir "C:\RML\Dicts" 2>nul
mkdir "C:\RML\Dicts\Morph" 2>nul
mkdir "C:\RML\out\build\x64-Debug\Source\morph_dict\data\CMakeFiles" 2>nul

REM Create language-specific directories
mkdir "C:\RML\Dicts\Morph\Russian" 2>nul
mkdir "C:\RML\Dicts\Morph\German" 2>nul
mkdir "C:\RML\Dicts\Morph\English" 2>nul

REM Take ownership of directories
takeown /F "C:\RML\Dicts" /R /D Y
takeown /F "C:\RML\out" /R /D Y

REM Reset permissions
icacls "C:\RML\Dicts" /reset /T
icacls "C:\RML\out" /reset /T

REM Grant full permissions to Administrators and SYSTEM
icacls "C:\RML\Dicts" /grant:r Administrators:(OI)(CI)F /T
icacls "C:\RML\Dicts" /grant:r SYSTEM:(OI)(CI)F /T
icacls "C:\RML\out" /grant:r Administrators:(OI)(CI)F /T
icacls "C:\RML\out" /grant:r SYSTEM:(OI)(CI)F /T

REM Grant full permissions to the current user
for /f "tokens=* delims=" %%a in ('whoami') do (
    set CURRENT_USER=%%a
    echo Granting permissions to !CURRENT_USER!
    icacls "C:\RML\Dicts" /grant:r "!CURRENT_USER!:(OI)(CI)F" /T
    icacls "C:\RML\out" /grant:r "!CURRENT_USER!:(OI)(CI)F" /T
)

REM Remove inheritance and copy existing permissions
icacls "C:\RML\Dicts" /inheritance:r
icacls "C:\RML\out" /inheritance:r

echo Done. Please try building the project again.
pause 