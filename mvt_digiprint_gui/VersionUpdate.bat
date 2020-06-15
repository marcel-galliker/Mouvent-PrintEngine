@echo off
FOR /F "tokens=* USEBACKQ" %%F IN (`git describe --dirty^=* --match "v[0-9]*.[0-9]*"`) DO SET DESCRIBE=%%F

for /f "tokens=1,2,3,4 delims=- " %%a in ("%DESCRIBE%") do (
set VERSION=%%a.%%b
set COMMIT=%%c
)
if "%BUILD_BUILDID%" NEQ "" set VERSION=%VERSION%.%BUILD_BUILDID%

echo Update AssemblyInfo.cs
powershell -Command "(Get-Content -Encoding UTF8 %1 ).Replace('#VERSION#','%VERSION%'.SubString(1)).Replace('#COMMIT#','%COMMIT%'.SubString(1)).Replace('#DATE#','%DATE%')  | Set-Content -Encoding UTF8 %2"