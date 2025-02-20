@echo off
chcp 1251 > nul
setlocal

echo Setting Russian locale...

:: Set system locale to Russian
reg add "HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\Language" /v InstallLanguage /t REG_SZ /d 0419 /f
reg add "HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\Language" /v Default /t REG_SZ /d 0419 /f

:: Set user locale to Russian
reg add "HKEY_CURRENT_USER\Control Panel\International" /v LocaleName /t REG_SZ /d ru-RU /f
reg add "HKEY_CURRENT_USER\Control Panel\International" /v sLanguage /t REG_SZ /d RUS /f
reg add "HKEY_CURRENT_USER\Control Panel\International" /v sCountry /t REG_SZ /d Russia /f

:: Set system locale for non-Unicode programs
reg add "HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\CodePage" /v ACP /t REG_SZ /d 1251 /f
reg add "HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\CodePage" /v OEMCP /t REG_SZ /d 866 /f

echo.
echo Russian locale has been set successfully.
echo Please restart your computer for changes to take effect.
echo.

pause
endlocal 