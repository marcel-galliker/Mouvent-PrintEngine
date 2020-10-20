echo Update AssemblyInfo.cs

@echo off
FOR /F "tokens=* USEBACKQ" %%F IN (`git describe --match "v[0-9]*.[0-9]*"`) DO SET DESCRIBE=%%F
set COMMIT=0
for /f "tokens=1,2,3,4 delims=- " %%a in ("%DESCRIBE%") do (
if defined %%c set COMMIT=%%c
set VERSION=%%a.%%b%COMMIT%)

if "%BUILD_BUILDID%" NEQ "" set VERSION=%VERSION%.%BUILD_BUILDID%
echo version=%VERSION%
echo data=%DATE%
echo commit=%COMMIT%
powershell -Command "(Get-Content -Encoding UTF8 %1 ).Replace('#VERSION#','%VERSION%'.SubString(1)).Replace('#COMMIT#','%COMMIT%'.SubString(1)).Replace('#DATE#','%DATE%')  | Set-Content -Encoding UTF8 %2"
