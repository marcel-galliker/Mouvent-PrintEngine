@echo off
setlocal EnableDelayedExpansion

REM if started directly by double-click the script should pause at the end
if not "%~2"=="" set NO_PAUSE=1

REM vcvars batch file has to be executed first to be able to compile from CLI
set VCVARS_PATH="C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat"
if exist %VCVARS_PATH% (
	call %VCVARS_PATH%
) else (
	echo %VCVARS_PATH% could not be found!
	echo Are you sure Visual Studio Professional 2019 is installed?
	goto EOF
)

REM Special case to restore dotnet package
if "%1"=="dotnet" (
	dotnet restore
	SET RETURNCODE=0
	goto :EOF
)

REM cmd /c "bash -c 'sudo ssh-restart.sh'"
REM call "C:\utils\start-ssh.bat"

set BATCH_PATH=%~dp0
echo %DATE% %TIME%
echo Start building projects in %BATCH_PATH%

set LOG_PATH=%BATCH_PATH%log\
set LOG_PASS=%BATCH_PATH%log\passed\
set LOG_FAIL=%BATCH_PATH%log\failed\
REM set NIOS_HEAD_PATH=%BATCH_PATH%..\rx_head_nios
REM set NIOS_FLUID_PATH=%BATCH_PATH%..\rx_fluid_nios
set BIN_PATH=%BATCH_PATH%..\bin\
set GUI_PATH=%BIN_PATH%gui\

if not exist %LOG_PATH% (
	md %LOG_PATH%
	echo log directory created
) else (
	if not "%2"=="-no-delete" (
		del /q %LOG_PATH%\*
		echo old log files deleted
	) else (
		echo keeping old log files
	)
)

if not exist %LOG_PASS% (
	md %LOG_PASS%
) else (
	if not "%2"=="-no-delete" del /q %LOG_PASS%\*
)

if not exist %LOG_FAIL% (
	md %LOG_FAIL%
) else (
	if not "%2"=="-no-delete" del /q %LOG_FAIL%\*
)

if not exist %BIN_PATH% (
	REM powershell -Command "New-Item -ItemType Directory -Path %BIN_PATH% -Force | Out-Null"
	md %BIN_PATH%
	md %GUI_PATH%
)

if not exist %GUI_PATH% (
	md %GUI_PATH%
)

echo.

set FAILED_BUILDS=
set /A FAIL_CNT=0
set /A PASS_CNT=0
set /a BIN_CNT=0
set /A i=0
set BINFILES[0]=
set TARGETS=/t:Build
REM build sequence

echo Build %1
echo ========================

if "%1"=="other" (
  dotnet restore %BATCH_PATH%..
  call :LIB_X64
  call :BIN_X64
  call :LIB_SOC
  call :BIN_SOC
  call :LIB_LX
  call :BIN_LX
) else if "%1"=="nios" (
  call :NIOS
) else if "%1"=="keil" (
  call :KEIL
) else (
  REM Default Action to run partial build (X64, LX, SOC or X32)
  call :LIB_%1
  call :BIN_%1
)

echo ========================
goto :EVAL

REM ----------------------------------------------------------------------------
:NIOS
	set BUILD=32
	set FLAGS=/m /property:Configuration=Release-soc /property:Platform=Win32
	call :BUILD_PROJECT rx_fluid_nios, vcxproj
	call :BUILD_PROJECT rx_head_nios, vcxproj
	goto :EOF
REM ----------------------------------------------------------------------------

:KEIL
	set BUILD=32
	set FLAGS=/m /property:Configuration=Release /property:Platform=Win32
	call :BUILD_PROJECT rx_head_cond, vcxproj
	goto :EOF

REM Build a special debug rx_main_ctrl for test (local IP)
:LIB_TEST
	set BUILD=64
	set FLAGS=/m /property:Configuration=Debug /property:Platform=x64 /property:SolutionDir=%BATCH_PATH%..\
	call :BUILD_PROJECT rx_common_lib, vcxproj
	call :BUILD_PROJECT rx_tif_lib, vcxproj
	call :BUILD_PROJECT rx_pecore_lib, vcxproj
	call :BUILD_PROJECT TinyXML, vcxproj, Externals\
	call :BUILD_PROJECT rx_rip_lib, vcxproj
	goto :EOF

:BIN_TEST
	set BUILD=64
	set FLAGS=/m /p:DefineConstants="RX_CTRL_SUBNET=\"127.168.200.\"" /property:Configuration=Debug /property:Platform=x64 /property:SolutionDir=%BATCH_PATH%..\
	call :BUILD_PROJECT rx_main_ctrl, vcxproj
	goto :EOF


:LIB_X64
	set BUILD=64
	set FLAGS=/m /property:Configuration=Release /property:Platform=x64 /property:SolutionDir=%BATCH_PATH%..\
	call :BUILD_PROJECT rx_common_lib, vcxproj
	call :BUILD_PROJECT rx_common_lib_cs, csproj
	call :BUILD_PROJECT rx_tif_lib, vcxproj
	call :BUILD_PROJECT rx_pecore_lib, vcxproj
	call :BUILD_PROJECT ScanCheckParser, sln
	call :BUILD_PROJECT TinyXML, vcxproj, Externals\
	REM rx_rip_lib must be built after TinyXML
	call :BUILD_PROJECT rx_rip_lib, vcxproj
	call :BUILD_PROJECT rx_slicescreen_lib, vcxproj
	goto :EOF

:BIN_X64
	set BUILD=64
	set FLAGS=/m /property:Configuration=Release /property:Platform=x64 /property:SolutionDir=%BATCH_PATH%..\
	call :BUILD_PROJECT rx_spooler_ctrl, vcxproj
	call :BUILD_PROJECT rx_main_ctrl, vcxproj
	REM call :BUILD_PROJECT rx_digiprint_gui, sln
	call :BUILD_PROJECT mvt_digiprint_gui, sln
	call :BUILD_PROJECT Win10-Install, vcxproj, Win10\
	call :BUILD_PROJECT Win10-Startup, vcxproj, Win10\
	goto :EOF
REM ----------------------------------------------------------------------------

:LIB_SOC
	set BUILD=SOC
	set FLAGS=/m /property:Configuration=Release-soc /property:Platform=Win32
	call :BUILD_PROJECT rx_common_lib, vcxproj
	call :BUILD_PROJECT TinyXML, vcxproj, Externals\
	call :BUILD_PROJECT rx_tif_lib, vcxproj
	goto :EOF

:BIN_SOC
	set BUILD=SOC
	set FLAGS=/m /property:Configuration=Release-soc /property:Platform=Win32
	call :BUILD_PROJECT rx_boot, sln
	call :BUILD_PROJECT rx_encoder_ctrl, sln
	call :BUILD_PROJECT rx_fluid_ctrl, sln
	call :BUILD_PROJECT rx_head_ctrl, sln
	call :BUILD_PROJECT rx_stepper_ctrl, sln
	goto :EOF
REM ----------------------------------------------------------------------------

:LIB_LX
	REM no lib as we are building only solution and clean before build
	goto :EOF

:BIN_LX
	set BUILD=LX
	REM Important to clean to ensure the Linux folder is empty
	set TARGETS=/t:Clean,Build
	set FLAGS=/m:1 /property:Configuration=Release-lx /property:Platform=Win32
	call :BUILD_PROJECT rx_dhcp_server, sln
	call :BUILD_PROJECT rx_spooler_ctrl, sln
	call :BUILD_PROJECT rx_boot, sln
	call :BUILD_PROJECT rx_main_ctrl, sln
	set TARGETS=/t:Build
	goto :EOF
REM ----------------------------------------------------------------------------

REM Check creation time of built files.
REM Print all files older than %TIME_DIFF%
:CHECK_FILE_TIMES
	set TIME_DIFF=30

	REM loop through all files in bin folder
	for /r %BATCH_PATH%\..\bin %%a in (*) do (
		REM loop through all generated binary files
		for /l %%b in (0,1,%BIN_CNT%) do (
			REM check if file is one that has been built
			if %%~na==!BINFILES[%%b]! (
				set FILE_NAME=%%~a
				set FILE_DATE_TIME=%%~ta

				set psCommand=powershell -command "Test-Path !FILE_NAME! -OlderThan (Get-Date).AddMinutes(-!TIME_DIFF!)"
				for /f "delims=" %%I in ('!psCommand!') do set "RET=%%I"

				if "!RET!"=="True" (
					echo Following file was created more than !TIME_DIFF! minutes ago^^!
					echo !FILE_NAME!
					echo !FILE_DATE_TIME!
					echo ---
				)
			)
		)
	)

	goto :EOF
REM ----------------------------------------------------------------------------

REM Build each project.
REM Parameter 1: Projectname
REM Parameter 2: Projectextension
REM Parameter [3]: optional subpath of projectfile ending with '\'
:BUILD_PROJECT
	set EXIT_MISSING_PARAM=2
	if "%~1"=="" goto :MISSING_PARAMS
	if "%~2"=="" goto :MISSING_PARAMS

	set PROJ_FILE=%~3%~1\%~1.%~2
	set LOG_FILE=%LOG_PATH%%BUILD%_%~1.log
	echo === compiling %~1 {%BUILD%} ===
	msbuild.exe -nodeReuse:False %FLAGS% %BATCH_PATH%..\%PROJ_FILE% %TARGETS% > %LOG_FILE%
	if errorlevel 1 (
		type %LOG_FILE%
		echo [91mbuild FAILED[0m
		set FAILED_BUILDS=%FAILED_BUILDS%  %~1 {%BUILD%}
		set LOGPATH_2=%LOG_FAIL%%BUILD%_%~1.log
		set /a FAIL_CNT+=1
	) else (
		echo [92mbuild succeeded[0m
		set LOGPATH_2=%LOG_PASS%%BUILD%_%~1.log
		set /a PASS_CNT+=1
	)

	set /a BIN_CNT+=1

	REM get warnings and errors from logfile
	FOR /F "tokens=* USEBACKQ" %%F IN (`powershell -Command "Select-String -Path %LOG_FILE% -Pattern '(\d*) (Warning\(s\)|Warnung\(en\)|Avertissement\(s\))$' | %% { $_.Matches.groups[1] } | %% { $_.Value }"`) DO SET WARN=%%F
	FOR /F "tokens=* USEBACKQ" %%F IN (`powershell -Command "Select-String -Path %LOG_FILE% -Pattern '(\d*) (Error\(s\)|Fehler|Erreur\(s\))$' | %% { $_.Matches.groups[1] } | %% { $_.Value }"`) DO SET ERR=%%F

	set WARN_STR=Warnings: %WARN%
	set ERR_STR=Errors:   %ERR%

	if %WARN% gtr 0 (
		echo [93m%WARN_STR%[0m
	) else (
		echo %WARN_STR%
	)

	if %ERR% gtr 0 (
		echo [91m%ERR_STR%[0m
	) else (
		echo %ERR_STR%
	)

	move %LOG_FILE% %LOGPATH_2% > nul
	REM echo LOG: %LOGPATH_2%
	echo ========================
	echo.


	set BINFILES[%i%]=%~1
	set /a i+=1

	goto :EOF
REM ----------------------------------------------------------------------------

REM Missing parameters to build project > exit script
:MISSING_PARAMS
	echo Missing parameters on function call: %~1
	exit /b EXIT_MISSING_PARAM
REM ----------------------------------------------------------------------------

REM Summarize all passed and failed builds
:EVAL

	echo ****************************************************
	echo %DATE% %TIME%
	echo Finished building projects in %BATCH_PATH%

	REM set the error code to 2 for no run done
	set RETURNCODE=2
	if "%FAILED_BUILDS%"=="" (
		echo [92m
		echo ALL BUILDS SUCCEEDED [%PASS_CNT%]
		IF "%PASS_CNT%" neq "0" set RETURNCODE=0
	) else (
		echo [92m
		echo SUCCEEDED BUILDS [%PASS_CNT%/%BIN_CNT%]
		echo [91m
		echo FAILED BUILDS [%FAIL_CNT%/%BIN_CNT%]:
		echo %FAILED_BUILDS%
		set RETURNCODE=1
	)
	echo [0m
	echo.

	call :CHECK_FILE_TIMES

	if "%NO_PAUSE%"=="" (
		echo.
		echo Press any key to close this window
		pause > nul
	)

REM End of file
:EOF
	echo ========================
	exit /b %RETURNCODE%
