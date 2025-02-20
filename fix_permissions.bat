@echo off
echo Setting permissions for morphology dictionaries...

rem Create directories if they don't exist
mkdir "C:\RML\Dicts\Morph\English" 2>nul
mkdir "C:\RML\Dicts\Morph\German" 2>nul
mkdir "C:\RML\Dicts\Morph\Russian" 2>nul

rem Take ownership and grant full permissions for Dicts folder and all subfolders
echo Taking ownership of Dicts folder...
takeown /f "C:\RML\Dicts" /r /d y

echo Setting full permissions for the current user...
icacls "C:\RML\Dicts" /grant:r "%USERNAME%":(OI)(CI)F /T

echo Setting full permissions for Administrators...
icacls "C:\RML\Dicts" /grant:r "Administrators":(OI)(CI)F /T

echo Setting full permissions for SYSTEM...
icacls "C:\RML\Dicts" /grant:r "SYSTEM":(OI)(CI)F /T

echo Done! Press any key to exit...
pause 