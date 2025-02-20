@echo off
setlocal enabledelayedexpansion

echo Finding fastest Cygwin mirror...

:: Create directories if they don't exist
if not exist "external" (
    mkdir external
)
if not exist "external\cygwin" (
    mkdir external\cygwin
)

:: Download mirror list
echo Downloading mirror list...
powershell -Command "[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; Invoke-WebRequest -Uri 'https://cygwin.com/mirrors.lst' -OutFile 'external\cygwin\mirrors.lst'"

if not exist "external\cygwin\mirrors.lst" (
    echo Failed to download mirror list
    exit /b 1
)

:: Create temporary file for results
echo Testing mirrors speed...
type nul > external\cygwin\mirror_speeds.txt

:: Test each mirror multiple times with different files
echo Testing mirrors (this may take a few minutes)...
for /f "tokens=2 delims=;" %%a in (external\cygwin\mirrors.lst) do (
    echo Testing %%a
    powershell -Command "$ErrorActionPreference = 'SilentlyContinue'; $totalMs = 0; $successCount = 0; $urls = @('%%a/x86_64/setup.ini', '%%a/x86_64/setup.bz2', '%%a/x86_64/wget/wget-1.21.4-1.tar.xz'); foreach ($url in $urls) { try { $start = Get-Date; $response = Invoke-WebRequest -Uri $url -Method Head -TimeoutSec 3; if ($response.StatusCode -eq 200) { $end = Get-Date; $totalMs += ($end - $start).TotalMilliseconds; $successCount++ } } catch {} }; if ($successCount -gt 0) { $avgTime = $totalMs / $successCount; Add-Content -Path 'external\cygwin\mirror_speeds.txt' -Value ('%%a;' + $avgTime + ';' + $successCount) }"
)

:: Find best mirror (considering both speed and reliability)
echo Finding the best mirror...
powershell -Command "$ErrorActionPreference = 'SilentlyContinue'; $results = Get-Content 'external\cygwin\mirror_speeds.txt' | Where-Object {$_.trim() -ne ''} | ForEach-Object { $parts = $_ -split ';'; if ($parts.Count -eq 3) { [PSCustomObject]@{ Url = $parts[0]; Time = [double]$parts[1]; Success = [int]$parts[2] } } } | Where-Object {$_ -ne $null} | Sort-Object -Property Success -Descending | Sort-Object {$_.Time / $_.Success}; if ($results) { $results[0].Url | Set-Content 'external\cygwin\mirror.txt' } else { 'https://mirrors.kernel.org/cygwin/' | Set-Content 'external\cygwin\mirror.txt' }"

:: Display result
set /p MIRROR=<external\cygwin\mirror.txt
echo.
echo Best mirror found: %MIRROR%
echo Mirror saved to external\cygwin\mirror.txt
echo Success rate and average response time were considered in the selection.

:: Cleanup
del external\cygwin\mirrors.lst
del external\cygwin\mirror_speeds.txt

pause 