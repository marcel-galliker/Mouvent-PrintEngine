#include "stdafx.h"

#include "lib.h"

//--- rx_process_execute --------------------------------------------------------------------
int rx_process_execute(const char *process, const char *outPath, int timeout)
{
	SECURITY_ATTRIBUTES		sa;
	STARTUPINFOA			startuopInfo;
	PROCESS_INFORMATION		processInfo;

	int		ret;
	ULONG	code;
	HANDLE	hfile=NULL;
	
	memset(&startuopInfo, 0, sizeof(startuopInfo));
	startuopInfo.cb			= sizeof(startuopInfo);

	if (outPath && *outPath)
	{
		memset(&sa, 0, sizeof(sa));
		sa.nLength			= sizeof(sa);
		sa.bInheritHandle	= TRUE;

		hfile = CreateFileA(outPath, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &sa, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, 0);

		startuopInfo.dwFlags	= STARTF_USESTDHANDLES;
		startuopInfo.hStdOutput = hfile;
		startuopInfo.hStdError  = hfile;
	}

	ret = CreateProcessA(NULL, (char*)process, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, NULL, NULL, &startuopInfo, &processInfo);
	if (!ret) 
	{
		if (hfile) CloseHandle(hfile);
		return GetLastError();
	}
	do
	{

		Sleep(10);
		if (timeout)
		{
			timeout-=10;
			if (timeout<=0) 
			{
				TerminateProcess(processInfo.hProcess, WAIT_TIMEOUT);
			}
		}
		GetExitCodeProcess(processInfo.hProcess, &code);
	} while (code==STILL_ACTIVE);
	if (hfile) CloseHandle(hfile);
	return code;
}
