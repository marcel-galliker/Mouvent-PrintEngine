@echo off
echo Update AssemblyInfo.cs
powershell -Command "$ver = %~dp0\..\automated_build\gitversion.exe /config %~dp0\..\automated_build\GitVersion.yml | ConvertFrom-Json; Get-Content -Encoding UTF8 %1 | foreach { $ExecutionContext.InvokeCommand.ExpandString($_) } | Set-Content -Encoding UTF8 %2"