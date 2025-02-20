@echo off
setlocal enabledelayedexpansion

echo Finding best Cygwin mirror for Kiev...

:: Create directories if they don't exist
if not exist "external" (
    mkdir external
)
if not exist "external\cygwin" (
    mkdir external\cygwin
)

:: List of fast mirrors for Kiev (European mirrors)
set "mirrors=https://mirror.datacenter.by/pub/mirrors/cygwin/;https://ftp.gwdg.de/pub/linux/sources.redhat.com/cygwin/;https://mirrors.netix.net/cygwin/;https://mirror.mangohost.net/cygwin/;https://ftp.acc.umu.se/mirror/cygwin/"

:: Test each mirror
echo Testing mirrors for best connection...
type nul > external\cygwin\mirror_speeds.txt

for %%m in (%mirrors%) do (
    echo Testing %%m
    powershell -Command "$ErrorActionPreference = 'SilentlyContinue'; try { $start = Get-Date; $response = Invoke-WebRequest -Uri '%%m/x86_64/setup.ini' -Method Head -TimeoutSec 3; if ($response.StatusCode -eq 200) { $end = Get-Date; $time = ($end - $start).TotalMilliseconds; Add-Content -Path 'external\cygwin\mirror_speeds.txt' -Value ('%%m' + ';' + $time) } } catch {}"
)

:: Find fastest mirror
powershell -Command "$ErrorActionPreference = 'SilentlyContinue'; $fastest = Get-Content 'external\cygwin\mirror_speeds.txt' | ForEach-Object { $split = $_ -split ';'; [PSCustomObject]@{ Url = $split[0]; Time = [double]$split[1] } } | Sort-Object Time | Select-Object -First 1; if ($fastest) { $fastest.Url | Set-Content 'external\cygwin\mirror.txt' } else { 'https://mirror.datacenter.by/pub/mirrors/cygwin/' | Set-Content 'external\cygwin\mirror.txt' }"

:: Display result
set /p MIRROR=<external\cygwin\mirror.txt
echo.
echo Best mirror for Kiev: %MIRROR%
echo Mirror saved to external\cygwin\mirror.txt

:: Cleanup
if exist external\cygwin\mirror_speeds.txt del external\cygwin\mirror_speeds.txt

pause 