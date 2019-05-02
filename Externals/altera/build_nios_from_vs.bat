@echo off

REM ###################################################
REM #                                                 #
REM # Nios2 Software Build Tools (SBT) Command Shell  #
REM #                                                 #
REM # Execute this script to setup an environment for #
REM # using the Nios2eds Software Build Tools (SBT)   #
REM #                                                 #
REM # This shell uses nios2-elf-gcc version 4.1.2     #
REM #                                                 #
REM #                                                 #
REM # Copyright (c) 2010, 2012 Altera Corporation     #
REM # All Rights Reserved.                            #
REM #                                                 #
REM ###################################################

echo SOPC_KIT_NIOS2_15 %SOPC_KIT_NIOS2_15%

REM ######################################
REM # Variable to ignore <CR> in DOS line endings
set SHELLOPTS=igncr

REM ######################################
REM # Variable to ignore mixed paths
REM # i.e. G:/$SOPC_KIT_NIOS2/bin
set CYGWIN=nodosfilewarning

REM ######################################
REM # Discover the root nios2eds directory
REM set _NIOS2EDS_ROOT=%~dp0

set _NIOS2EDS_ROOT=%SOPC_KIT_NIOS2_15%
if "%_NIOS2EDS_ROOT%"=="" set _NIOS2EDS_ROOT=%SOPC_KIT_NIOS2%

if "%_NIOS2EDS_ROOT%"=="" (
	echo environment variable "SOPC_KIT_NIOS2_15" not defined
	exit 1
)

REM set _NIOS2EDS_ROOT=%SOPC_KIT_NIOS2_15%
set _NIOS2EDS_ROOT="%_NIOS2EDS_ROOT:\=/%"

echo _NIOS2EDS_ROOT %_NIOS2EDS_ROOT%

REM ######################################
REM # set root quartus directory to
REM # QUARTUS_ROOTDIR_OVERRIDE if the
REM # env var is set
if "%QUARTUS_ROOTDIR_OVERRIDE%"=="" goto set_default_quartus_root
set _QUARTUS_ROOT=%QUARTUS_ROOTDIR_OVERRIDE%
goto set_default_quartus_bin

REM ######################################
REM # Discover the root quartus directory
REM # when QUARTUS_ROOTDIR_OVERRIDE is not set

:set_default_quartus_root
set _QUARTUS_ROOT=%_NIOS2EDS_ROOT%\..\quartus

REM ######################################
REM # Discover the quartus bin directory

:set_default_quartus_bin
set _QUARTUS_BIN=%_QUARTUS_ROOT%\bin64

REM ######################################
REM # Run qreg (if it exists) to setup
REM # cygwin and jtag services

:run_qreg
if not exist "%_QUARTUS_BIN%\qreg.exe" goto run_nios2_command_shell

if "%SKIP_ACDS_QREG%"=="1" goto run_nios2_command_shell
echo qreg.exe
%_QUARTUS_BIN%\qreg.exe

if "%SKIP_ACDS_QREG_JTAG%"=="1" goto run_nios2_command_shell
echo qreg.exe --jtag
%_QUARTUS_BIN%\qreg.exe --jtag

REM ######################################
REM # Launch cygwin nios2eds command shell

:run_nios2_command_shell

set PROJECT_PATH=%1
set FOLDER="%PROJECT_PATH%\software"

REM get project name from source directory
for %%f in (%PROJECT_PATH%) do set PROJECT_NAME=%%~nxf
echo building project: %PROJECT_NAME% in %PROJECT_PATH%

set PROJECT_PATH_UNIX=%FOLDER:\=/%/%PROJECT_NAME%
set BASH_PATH=%_QUARTUS_BIN%\cygwin\bin\bash.exe
set ECLIPSE_PATH_CYGWIN=%_NIOS2EDS_ROOT%/bin/eclipse_nios2/eclipsec.exe

REM optional parameter to clean project
set CLEAN=%2

:generate_bsp:
echo === build NIOS BSP

set BSP_FOLDER=%PROJECT_PATH_UNIX%_bsp
set BSP_SETTINGS_FILE=%BSP_FOLDER%/settings.bsp

%BASH_PATH% -c '%_NIOS2EDS_ROOT%/nios2_command_shell.sh nios2-bsp-generate-files --settings="%BSP_SETTINGS_FILE%" --bsp-dir="%BSP_FOLDER%"

:build_nios
echo === build NIOS

%BASH_PATH% -c '%_NIOS2EDS_ROOT%/nios2_command_shell.sh %ECLIPSE_PATH_CYGWIN%^
 -nosplash -application org.eclipse.cdt.managedbuilder.core.headlessbuild -data %FOLDER% -importAll %FOLDER% -cleanBuild %PROJECT_NAME% %CLEAN%

:mem_init_generate:
echo === mem_init_generate

%BASH_PATH% -c '%_NIOS2EDS_ROOT%/nios2_command_shell.sh bash -c "cd %PROJECT_PATH_UNIX% && pwd && make mem_init_generate"

exit 