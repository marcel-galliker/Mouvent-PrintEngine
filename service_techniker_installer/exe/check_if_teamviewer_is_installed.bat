@echo off
IF EXIST "C:\Program Files (x86)\TeamViewer\TeamViewer.exe" (
echo "Please uninstall TeamViewer before running this Installer"
pause
exit 0
) else (
	start /WAIT "" "C:\Program Files (x86)\mouvent\TeamViewer_Setup.exe"
	start /WAIT "" "C:\Program Files (x86)\mouvent\WinpkFilter.exe"
)
