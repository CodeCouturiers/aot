@echo off
setlocal enabledelayedexpansion

:: Get current directory
set "RML_PATH=%CD%"

:: Set RML environment variable for current session
set "RML=%RML_PATH%"
echo Set RML environment variable to: %RML%

:: Set RML environment variable permanently for user
powershell -Command "[Environment]::SetEnvironmentVariable('RML', '%RML_PATH%', 'User')"

:: Verify the setting
echo.
echo Testing RML environment variable:
echo %RML%

echo.
echo RML environment variable has been set.
echo Please restart your command prompt or IDE for the changes to take effect.

pause 