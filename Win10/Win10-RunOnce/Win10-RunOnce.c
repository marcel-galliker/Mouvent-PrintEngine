// Win10-RunOnce.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "windows.h"


//--- _disable_task ---------------------------------------
void _disable_task(char *taskname)
{
	char cmd[256];
	printf("Disable >>%s<<\n", taskname);
	sprintf(cmd, "schtasks /change /tn \"%s\" /disable", taskname);
	system(cmd);
	printf("\n");
}

//--- main -----------------------------------------------
int main(int argc, char* argv[])
{
	/*
	char name[MAX_PATH];
	printf("SYSTEM NAME:");
	scanf("%s", name);
	SetComputerNameA(name);
	*/

	//--- disable tasks ------------------------------------------------
	_disable_task("OneDrive Standalone Update Task v2");		
	_disable_task("\\Microsoft\\Office\\Office ClickToRun Service Monitor");		
	_disable_task("\\Microsoft\\Office\\Office Automatic Updates");		
	_disable_task("\\Microsoft\\Windows\\WindowsUpdate\\Automatic App Update");		
	_disable_task("\\Microsoft\\Windows\\WindowsUpdate\\Scheduled Start");	
	_disable_task("\\Microsoft\\Windows\\WindowsUpdate\\sih");		
	_disable_task("\\Microsoft\\Windows\\WindowsUpdate\\sihboot");		

	system("powercfg -change -monitor-timeout-ac 0");
	system("powercfg -change -standby-timeout-ac 0");

	system("powercfg -setacvalueindex SCHEME_CURRENT SUB_BUTTONS PBUTTONACTION 0");
	system("powercfg -setacvalueindex SCHEME_CURRENT SUB_VIDEO aded5e82-b909-4619-9949-f5d71dac0bcb 100"); // Display Brightness
	system("powercfg -setdcvalueindex SCHEME_CURRENT SUB_VIDEO aded5e82-b909-4619-9949-f5d71dac0bcb 100"); // Display Brightness

	system("reg add \"HKCU\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\" /f /v Shell /d \"%ProgramFiles(x86)%\\mouvent\\Win10-Startup.exe\"");
	system("reg add \"HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\" /f /v HideFileExt /t REG_DWORD /d 0");

	system("reg add \"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\AutoRotation\" /f /v Enable /t REG_DWORD /d 0");

	system("netsh advfirewall firewall add rule name=\"ICMP Allow incoming V4 echo request\" protocol=icmpv4:8,any dir=in action=allow");
	system("netsh advfirewall firewall add rule name=\"ICMP Allow incoming V6 echo request\" protocol=icmpv6:8,any dir=in action=allow");

//	system("netsh interface ipv4 set address name=\"Ethernet 2\" static 192.168.200.2 255.255.255.0");

//	HKEY_LOCAL_MACHINE \SOFTWARE \Microsoft \Windows \CurrentVersion \Policies \System.

	printf("Install TeamViewer\n");
//	getchar();
	system("cmd /c \"%ProgramFiles(x86)%\\mouvent\\TeamViewer_Setup.exe\"");
//	system("sc config TeamViewer start=demand");

	printf("Install WinPk Filter \n");
//	getchar();
	system("cmd /c \"%ProgramFiles(x86)%\\mouvent\\WinpkFilter.exe\"");

	printf("Restart\n");
//	getchar();
	system("shutdown /r /t 0");
	return 0;
}

