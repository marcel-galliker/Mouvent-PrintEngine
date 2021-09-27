// ****************************************************************************
//
//	rx_Threads.cpp
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#include "rx_trace.h"
#include "rx_error.h"
#include "rx_threads.h"

static int _rx_core_cnt();
#define THREAD_CNT 256

//--- rx_get_maxinumthreads -------------------------
int rx_get_maxinumthreads()
{
	// use the number of core but not more than a quarter of the Thread Cnt
	return min(THREAD_CNT / 4, _rx_core_cnt());
}

#ifdef WIN32
#include <Windows.h>
#include <psapi.h>


//--- rx_thread_start(WIN32)----------------------------------------------
HANDLE rx_thread_start(void *(*thread_routine) (void *), void *param, UINT32 stacksize, const char *name)
{
	ULONG id;

	return CreateThread(NULL,					// pThreadAttributes 
		(SIZE_T)stacksize,						// dwStackSize
		(LPTHREAD_START_ROUTINE)thread_routine,	// lpStartAddress 
		param,									// lpParameter 
		0,										// dwCreationFlags 
		&id										// lpThreadId
		);
}

//--- rx_get_thread_id --------------------------------------------
int rx_get_thread_id(void)	
{
	return (int) GetCurrentThreadId();
}

//--- rx_mutex_create(WIN32) ---------------------------------------------
HANDLE rx_mutex_create()
{
	return CreateMutex(NULL, FALSE, NULL);
}

//--- rx_mutex_destroy --------------------------------------------------
int    rx_mutex_destroy(HANDLE *mutex)
{
	CloseHandle(*mutex);
	*mutex = NULL;
	return REPLY_OK;
}

//--- rx_mutex_lock(WIN32) ----------------------------------------------
int    rx_mutex_lock(HANDLE mutex)
{
	WaitForSingleObject(mutex, INFINITE);
	return REPLY_OK;
}

//--- rx_mutex_unlock(WIN32) -------------------------------------------
int    rx_mutex_unlock(HANDLE mutex)
{
	ReleaseMutex(mutex);
	return REPLY_OK;
}

//--- rx_sem_create(linux) ---------------------------------------------
HANDLE rx_sem_create()
{
	return CreateSemaphore(NULL, 0, 100, NULL);
}

//--- rx_sem_destroy ---------------------------------------------------
int rx_sem_destroy(HANDLE *sem)
{
	CloseHandle(*sem);
	*sem=NULL;
	return REPLY_OK;
}
//--- rx_sem_wait(linux) ----------------------------------------------
int    rx_sem_wait(HANDLE sem, int time)
{
	int ret;
	if (!time) time=INFINITE;
	ret= WaitForSingleObject(sem, time);
	if (ret==WAIT_OBJECT_0) return REPLY_OK;
	else if (ret==-1) return GetLastError();
	else return ret;
}

//--- rx_sem_post(linux) -------------------------------------------
int    rx_sem_post(HANDLE sem)
{
	long old;
	if (ReleaseSemaphore(sem, 1, &old)) return REPLY_OK;
	else return GetLastError();
}

//--- rx_sleep ----------------------------------------
void rx_sleep(UINT32 ms)
{
	Sleep(ms);
}

//--- rx_process_start -------------------------------
#ifdef _WIN64_unused
int rx_process_start	(const char *process, const char *params)
{
	int ret;
	printf("Strating >>%s<< >>%s<<\n", process, params);
	ret=(int)ShellExecute(NULL,  "runas", (char*)process, params, NULL, SW_SHOW);
	if (ret>32) return REPLY_OK;
	else return ret;
}
#else

int rx_process_start	(const char *process, const char *params)
{
	STARTUPINFO			startuopInfo;
	PROCESS_INFORMATION	processInfo;
	char				*workingDir=NULL;

	memset(&startuopInfo, 0, sizeof(startuopInfo));
	startuopInfo.cb = sizeof(startuopInfo);
	memset(&processInfo, 0, sizeof(processInfo));

	if (params) return CreateProcess((char*)process, (char*)params, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS | DETACHED_PROCESS, NULL, workingDir, &startuopInfo, &processInfo);
	else	    return CreateProcess(NULL, (char*)process,          NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS | DETACHED_PROCESS, NULL, workingDir, &startuopInfo, &processInfo);
}
#endif

//--- rx_process_get_id -------------------------------
int rx_process_get_id	(const char *process, const char *arg)
{
	return 0;
}

//--- rx_process_kill --------------------------------
int rx_process_kill 	(const char *process, const char *arg)
{
	unsigned int	i;
	int				count, start;
	DWORD			aProcesses[1024], cbNeeded, cProcesses;
	HMODULE			hMod;
	HANDLE			hProcess;
	char			szProcessName[50];

	for (start = (int)strlen(process); start > 0; start--)
	{
		if (process[start] == '\\' || process[start] == '/')
		{
			start++;
			break;
		}
	}

	if (EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
	{
		cProcesses = cbNeeded / sizeof(DWORD);

		// Print the name and process identifier for each process.
		count = 0;
		for (i = 0; i < cProcesses; i++) 
		{
			strcpy_s(szProcessName, sizeof(szProcessName), "unknown process");
			hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aProcesses[i]);
			if (hProcess)
			{
				if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
					GetModuleBaseName(hProcess, hMod, szProcessName, sizeof(szProcessName));
				CloseHandle( hProcess );
				if (_strnicmp(szProcessName, &process[start], strlen(&process[start])) == 0)
				{
					hProcess = OpenProcess (PROCESS_ALL_ACCESS | PROCESS_TERMINATE, FALSE, aProcesses[i]);
					if (hProcess)
					{
						int err = TerminateProcess(hProcess, 0);
						if (err==0) err=GetLastError();
						CloseHandle( hProcess );
						count++;
					}
				}
			}
		}
	}
	return count;
}

//--- rx_exit --------------------------------
void   rx_exit(int exitCode)
{
	TrPrintf(TRUE, "rx_exit(%d)", exitCode);
	ExitProcess(exitCode);
}

//--- rx_process_running_cnt -------------------------------------------
int rx_process_running_cnt(const char *process, const char *arg)//, HANDLE *phandle, HMODULE *pmodule)
{
	int start, count=0;
	FILE *file;
	char str[100];

	for (start = (int)strlen(process); start > 0; start--)
	{
		if (process[start] == '\\' || process[start] == '/')
		{
			start++;
			break;
		}
	}

	file = _popen("tasklist.exe", "r");
	if (file)
	{
		while (fgets(str, sizeof(str), file))
		{
			if (str_start(str, &process[start])) count++;
		}
		fclose(file);
	}
	return count;
}

/* !!! this does not list x64 processes !!!! ----------------------------------------------------
int rx_process_running_cnt(const char *process, const char *arg)//, HANDLE *phandle, HMODULE *pmodule)
{
		unsigned int	i;
		int				count, start;
		DWORD			aProcesses[1024], cbNeeded, cProcesses;
		HMODULE			hMod;
		HANDLE			hProcess;
		char			szProcessName[50];
	char			szFileName[MAX_PATH];

		for (start = (int)strlen(process); start > 0; start--)
		{
			if (process[start] == '\\' || process[start] == '/')
			{
				start++;
				break;
			}
		}

		if (EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
		{
			cProcesses = cbNeeded / sizeof(DWORD);

			// Print the name and process identifier for each process.
			count = 0;
			for (i = 0; i < cProcesses; i++) 
			{
				strcpy_s(szProcessName, sizeof(szProcessName), "unknown process");
				hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aProcesses[i]);
				if (hProcess)
				{
				strcpy(szFileName, "unknown");
				if (EnumProcessModulesEx(hProcess, &hMod, sizeof(hMod), &cbNeeded, LIST_MODULES_ALL))
						GetModuleBaseName(hProcess, hMod, szProcessName, sizeof(szProcessName));
					if (_strnicmp(szProcessName, &process[start], strlen(&process[start])) == 0)
					{
				//		if (phandle) *phandle = hProcess;
				//		if (pmodule) *pmodule = hMod;
						count++;
					}
				}
			}
		}

		return count;
}
*/

//--- rx_run_in_backgrund -------------------------------
void rx_run_in_backgrund()
{
	return;
}

//--- rx_startup -----------------------------------
int rx_startup(const char* name, int debug)
{
	int cnt;
	cnt = rx_process_running_cnt(name, NULL);

	if (!debug)
	{
		if (cnt>1)
		{
			return REPLY_ERROR;
		}
		rx_run_in_backgrund();
	}
	return REPLY_OK;
}

//--- rx_set_process_priority -----------------------
void rx_set_process_priority(int priority)
{
}

//--- rx_set_tread_priority -----------------------
int rx_set_tread_priority(int priority)
{
	return REPLY_OK;
}

//--- rx_process_execute --------------------------------------------------------------------
int rx_process_execute(const char *process, const char *outPath, int timeout)
{
	SECURITY_ATTRIBUTES		sa;
	STARTUPINFO				startuopInfo;
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

		hfile = CreateFile(outPath, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &sa, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, 0);

		startuopInfo.dwFlags	= STARTF_USESTDHANDLES;
		startuopInfo.hStdOutput = hfile;
		startuopInfo.hStdError  = hfile;
	}

	ret = CreateProcess(NULL, (char*)process, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, NULL, NULL, &startuopInfo, &processInfo);
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

//--- rx_core_cnt -----------------------
int _rx_core_cnt(void)
{
	SYSTEM_INFO sysinfo;
	GetSystemInfo( &sysinfo );
	return sysinfo.dwNumberOfProcessors;	
}

#else

#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <semaphore.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <dirent.h>


static HANDLE _ThreadMutex;

//--- globals --------------------------------------------------
typedef struct SThreadInfo
{
	int		idx;
	int		tid;
	char	name[64];
	void *(*thread_routine) (void *);
	void *param;
} SThreadInfo;

static SThreadInfo ThreadInfo[THREAD_CNT]={0};

//--- rx_get_thread_id --------------------------------------------
int rx_get_thread_id(void)	
{
	return (int) syscall (SYS_gettid);
}

//--- launcher --------------------------------------------------------------
static void *launcher(void *param)
{
	SThreadInfo *par = (SThreadInfo*)param;
	int idx = par->idx;
	void *ret;

	//---  run the thread -----------------------------------
	ret = par->thread_routine(par->param);

#ifdef linux		
	#ifndef soc
		TrPrintfL(TRUE, "end thread[%d]: tid=%d, name=>>%s<<", idx, ThreadInfo[idx].tid, ThreadInfo[idx].name);
	#endif
#endif

	//--- clean up --------------------------
	rx_mutex_lock(_ThreadMutex);
//	memset(par, 0, sizeof(SThreadInfo));
	memset(&ThreadInfo[idx], 0, sizeof(ThreadInfo[idx]));
	rx_mutex_unlock(_ThreadMutex);

	return ret;
}

//--- _rx_thread_list --------------------------------------------
static void _rx_thread_list(int mark)
{
	int idx;
	for (idx=0; idx<SIZEOF(ThreadInfo); idx++)
	{
		if (ThreadInfo[idx].tid) 
		{
			if(idx == mark) TrPrintfL(TRUE, "rx_thread_start thread[%d]: tid=%d, name=>>%s<< ****", idx, ThreadInfo[idx].tid, ThreadInfo[idx].name);			
			else			TrPrintfL(TRUE, "rx_thread_start thread[%d]: tid=%d, name=>>%s<<", idx, ThreadInfo[idx].tid, ThreadInfo[idx].name);			
		}	
	}
}

//--- rx_thread_start(linux) ----------------------------------------------------------------
HANDLE rx_thread_start(void *(*thread_routine) (void *), void *param, UINT32 stacksize, const char *name)
{
	int ret;
	pthread_t			threadId;
	char				test[32];

	if (!_ThreadMutex) _ThreadMutex=rx_mutex_create();
	rx_mutex_lock(_ThreadMutex);

	if (!ThreadInfo[0].tid)
	{
		ThreadInfo[0].tid=rx_get_thread_id();
		strcpy(ThreadInfo[0].name, "main");
	}

	//--- find list entry ----------------------------------
	int idx;
	for (idx=1; ; idx++)
	{
		if (idx==THREAD_CNT) 
		{
			rx_mutex_unlock(_ThreadMutex);
			Error(ERR_CONT, 0, "rx_thread_start TOO MANY THREADS");
			return NULL;
		}
		if (!ThreadInfo[idx].tid)
		{
			ThreadInfo[idx].tid=rx_get_thread_id();
			strncpy(ThreadInfo[idx].name, name, sizeof(ThreadInfo[idx].name));
			ThreadInfo[idx].thread_routine = thread_routine;
			ThreadInfo[idx].param          = param;
			break;
		}
/*
#ifdef linux		
	#ifndef soc
		TrPrintfL(TRUE, "rx_thread_start thread[%d]: tid=%d, name=>>%s<<", idx, ThreadInfo[idx].tid, ThreadInfo[idx].name);
	#endif
#endif
*/
	}
	ThreadInfo[idx].idx=idx;
	if (stacksize)
	{
		pthread_attr_t		attr;
		ret = pthread_attr_init(&attr);
		ret = pthread_attr_setstacksize(&attr, stacksize);
		ret = pthread_create(&threadId, &attr, launcher, &ThreadInfo[idx]);
	}
	else
	{
		ret = pthread_create(&threadId, NULL, launcher, &ThreadInfo[idx]);
	}		
#ifdef linux		
	#ifndef soc
	//	_rx_thread_list(idx);
	#endif
#endif
	
	rx_mutex_unlock(_ThreadMutex);
	
	if (ret==0) return (HANDLE)threadId;
	else		return (HANDLE)threadId;
}

//--- rx_mutex_create(linux) ---------------------------------------------
HANDLE rx_mutex_create()
{
	pthread_mutex_t 	*mutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(mutex, NULL);
	return (HANDLE)mutex;
}

//--- rx_mutex_destroy ---------------------------------------------------
int rx_mutex_destroy(HANDLE *mutex)
{
	pthread_mutex_destroy((pthread_mutex_t*)*mutex);
	*mutex=NULL;
	return REPLY_OK;
}

//--- rx_mutex_lock(linux) ----------------------------------------------
int    rx_mutex_lock(HANDLE mutex)
{
	pthread_mutex_lock((pthread_mutex_t*)mutex);
	return REPLY_OK;
}

//--- rx_mutex_unlock(linux) -------------------------------------------
int    rx_mutex_unlock(HANDLE mutex)
{
	pthread_mutex_unlock((pthread_mutex_t*)mutex);
	return REPLY_OK;
}

//--- rx_sem_create(linux) ---------------------------------------------
HANDLE rx_sem_create()
{
	sem_t 	*sem = (sem_t*) malloc(sizeof(sem_t));
	sem_init(sem, 0, 0); // used between threads of one process
	return (HANDLE)sem;
}

//--- rx_sem_destroy ---------------------------------------------------
int rx_sem_destroy(HANDLE *sem)
{
	sem_destroy((sem_t*)*sem);
	*sem=NULL;
	return REPLY_OK;
}
//--- rx_sem_wait(linux) ----------------------------------------------
int    rx_sem_wait(HANDLE sem, int time)
{
	if (time)
	{ 
		struct timespec t;
		UINT64 ms;
		int ret;
		clock_gettime(CLOCK_REALTIME, &t);
		t.tv_sec  += time/1000;
		t.tv_nsec += (time%1000)*1000000;
		if (t.tv_nsec>1000000000)
		{
			t.tv_sec++;
			t.tv_nsec-=1000000000;
		}
		
		ret = sem_timedwait((sem_t*)sem, &t);
		if (ret)
			ret=ret;
		return ret;
	}
	else if (sem==NULL) return REPLY_ERROR;
	else sem_wait((sem_t*)sem);
	return REPLY_OK;
}

//--- rx_sem_post(linux) -------------------------------------------
int    rx_sem_post(HANDLE sem)
{
	if (sem==NULL) return REPLY_ERROR;
	sem_post((sem_t*)sem);
	return REPLY_OK;
}

//--- rx_process_start --------------------------------------
int rx_process_start(const char *process, const char *params)
{
	int pid;
	const char *ch, *pname;
	pid=fork();
	if (pid==0)
	{
		pname=process;
		for (ch=process; *ch; ch++)
		{
			if (*ch=='/') pname=ch+1;
		}

		execl(process, pname, params, NULL);
		exit (0);
		return REPLY_ERROR;
	}
	else if (pid<0) return REPLY_ERROR;
	return REPLY_OK;
}

//--- rx_exit --------------------------------
void   rx_exit(int exitCode)
{
	TrPrintf(TRUE, "rx_exit(%d)", exitCode);
	exit(exitCode);
}

//--- rx_process_execute --------------------------------------------------------------------
int rx_process_execute(const char *process, const char *outPath, int timeout)
{
	return REPLY_ERROR;
/*
	SECURITY_ATTRIBUTES		sa;
	STARTUPINFO				startuopInfo;
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

		hfile = CreateFile(outPath, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &sa, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, 0);

		startuopInfo.dwFlags	= STARTF_USESTDHANDLES;
		startuopInfo.hStdOutput = hfile;
		startuopInfo.hStdError  = hfile;
	}

	ret = CreateProcess(NULL, (char*)process, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, NULL, NULL, &startuopInfo, &processInfo);
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
	*/
}

//--- rx_core_cnt -----------------------------
int _rx_core_cnt(void)
{
	char str[64];
	FILE *file = popen("nproc", "r");
	fgets(str, sizeof(str), file);
	pclose(file);
	return atoi(str);	
}

//--- rx_sleep ----------------------------------------
void	rx_sleep(UINT32 ms)
{
	usleep(1000*ms);
}

//--- _process_match --------------------------------
static int _process_match(INT32 processId, const char *pname, const char *arg)
{
	FILE	*file;
	char	fname[MAX_PATH];
	char	cmdline[1024];
	char	*a;
	size_t	size;

	// readllink("/proc/%d/cmdline", processId);
	sprintf(fname, "/proc/%d/cmdline", processId);
	if (file=fopen(fname, "r"))
	{
		memset(cmdline, 0, sizeof(cmdline));
		size=fread(cmdline, 1, sizeof(cmdline)-2, file);
		fclose(file);
		// printf("Process %d pname >>%s<<\n", processId, pname);
		// printf("cmdline >>%s<<\n", cmdline);

		if (!strstr(cmdline, pname)) return FALSE;
		if (arg==NULL) return TRUE;
		a=cmdline;
		while(1) 
		{
			a+=strlen(a)+1;
			if (!*a) return FALSE;
			// printf("arg >>%s<<\n", a);
			if (strstr(a, arg)) return TRUE;
		}
	}
	return FALSE;
}

//--- rx_process_get_id ------------------------------
int rx_process_get_id(const char *process, const char *arg )
{
	DIR *pDir;
	struct dirent *pEntry;
	INT32 process_id;
	const char *process_name;

	const char *ch;
	process_name = process;
	for (ch=process; *ch; ch++)
	{
		if (*ch=='/') process_name = ch+1;
	}

	pDir = opendir( "/proc/" );
	while(pEntry = readdir(pDir))
    {
		if ((pEntry->d_type & DT_DIR)						// directory
		&& (process_id = atoi(pEntry->d_name))				// && numbered
		&& (_process_match(process_id, process_name, arg))	// && match
		)
		{
			return process_id;
		}
    }
	closedir(pDir);
	return 0;
}

//--- rx_process_running_cnt ------------------------------
int rx_process_running_cnt(const char *process, const char *arg )
{
	int cnt=0;
	DIR *pDir;
	struct dirent *pEntry;
	INT32 process_id;
	const char *process_name;

	const char *ch;
	process_name = process;
	for (ch=process; *ch; ch++)
	{
		if (*ch=='/') process_name = ch+1;
	}

	pDir = opendir( "/proc/" );
	while(pEntry = readdir(pDir))
    {
		if ((pEntry->d_type & DT_DIR)					// directory
		&& (process_id = atoi(pEntry->d_name))			// && numbered
		&& (_process_match(process_id, process_name, arg))	// && match
		)
		{
			cnt++;
		}
    }
	closedir(pDir);
//	TrPrintfL(1, "rx_process_running_cnt(%s)=%d", process_name, cnt);
	return cnt;
}

//--- rx_process_kill --------------------------------------
int rx_process_kill(const char *process, const char *arg)
{
	int ret;

	unsigned int pid=rx_process_get_id(process, arg);
	if (pid)
	{
	//	TrPrintf(1, "Kill id=%d", pid);
        if (kill(pid, SIGTERM))
			printf("Kill >>%s<<: Error=%d >>%s<<\n", process, errno, strerror(errno));
    //    rx_sleep(2);
    //    kill(pid, SIGKILL);
	}
	return REPLY_OK;
}

//--- rx_run_in_backgrund -------------------------------
void rx_run_in_backgrund()
{
	int pid;

	pid=fork();
	if (pid==0) return;				// this is the new background process
	if (pid>0) exit (0);			// this is the main process
	printf("rx_run_in_backgrund ERROR\n\r");
}

//--- rx_startup -----------------------------------
int rx_startup(const char* name, int debug)
{
	int cnt;
	cnt = rx_process_running_cnt(name, NULL);
	TrPrintfL(1, "ProcessCnt >>%s<<=%d", name, cnt);

	if (debug)
	{
		TrPrintfL(1, "debug version");
		while(1)
		{
			cnt = rx_process_running_cnt(name, NULL);
			if (cnt<2) break;
			TrPrintfL(1, "Kill >>%s<<", name);
			rx_process_kill(name, NULL);
		}
	}
	else
	{
		TrPrintfL(1, "background version");
		if (cnt>1)
		{
			TrPrintfL(1, "Already running, cnt=%d", cnt);
			return REPLY_ERROR;
		}
		rx_run_in_backgrund();
	}
	return REPLY_OK;
}

//--- rx_set_process_priority -----------------------
void rx_set_process_priority(int priority)
{
	setpriority(PRIO_PROCESS, getpid(), priority);		
}

//--- rx_set_tread_priority -----------------------
int rx_set_tread_priority(int priority)
{
	pthread_t this_thread = pthread_self();
	struct sched_param params;
	int min=sched_get_priority_min(SCHED_FIFO);
	int max=sched_get_priority_max(SCHED_FIFO);
	if(priority < max) params.sched_priority = priority;
	else               params.sched_priority = max;
	return pthread_setschedparam(this_thread, SCHED_FIFO, &params);
}


#endif
