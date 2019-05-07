// ****************************************************************************
//
//	rx_trace.c
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include "rx_common.h"
#include "rx_file.h"
#include "rx_sok.h"
#include "rx_threads.h"
#include "tcp_ip.h"

#ifdef WIN32
#include <share.h>
#pragma warning( disable: 4996 )
#endif

//--- global variables -----------------------------------------
#define TRACE_FILE_CNT	5
#define TRACE_FILE_SIZE_MAX	1000000

#define TRACE_STR_LEN	270
#define TRACE_STR_CNT	128

static HANDLE	 _Mutex		= NULL;
static HANDLE	 _hServer	= NULL;
static FILE		 *_TraceFile	= NULL;
static  char	 _sAppName_org[32];
static  char	 _sAppName[32];
static int		 _TraceToScreen = TRUE;
static int		 _TraceToFile   = TRUE;
static char		 _TraceFilePath[MAX_PATH];
static int		 _TraceFileSize;

static char		_TraceStr[TRACE_STR_CNT][TRACE_STR_LEN];
static int		_TraceStrIn;
static int		_TraceStrOut;


static HANDLE	hTraceThread = NULL;
static HANDLE	hTraceSem;
static int		_TraceRunning;

//--- prototypes --------------------------------------------
static void _TraceFileOpen	(const char *path, const char *appName);
static void* _trace_thread(void* lpParameter);

//--- Trace_init ----------------------------------------------
void Trace_init(const char *appName)
{
	strcpy(_sAppName_org, appName);
#ifdef soc
	_TraceFileOpen(PATH_TRACE, appName);
#else
	_TraceFileOpen(PATH_LOG, appName);
#endif
	if (appName)
	{
		split_path(appName, NULL, _sAppName, NULL);
		strcat(_sAppName, ": ");
	}
	else *_sAppName=0;
	if (!_Mutex)
	{
		_Mutex			= rx_mutex_create();
		_TraceRunning	= TRUE;
		_TraceStrIn		= 0;
		_TraceStrOut	= 0;
		hTraceSem		= rx_sem_create();
		hTraceThread	= rx_thread_start(_trace_thread, NULL, 0, "Trace Thread");		
	}
}

void Trace_set_server(HANDLE server)
{
	_hServer = server;
}

//--- Trace_to_screen -------------------
void Trace_to_screen(int trace)
{
	_TraceToScreen = trace;
}

//--- Trace_to_file -------------------
void Trace_to_file(int trace)
{
	_TraceToFile = trace;
}

//--- Trace_end ------------------------------------------------
void Trace_end(void)
{
	if (_TraceFile)
	{
		fclose(_TraceFile);
		_TraceFile = NULL;
	}
}

//--- _TraceFileOpen ----------------------------------------------------------
static void _TraceFileOpen(const char *path, const char *appName)
{
	char *MonthStr[]= {"JAN", "FEB", "MAR", "APR", "MAI", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"}; 
	if (_TraceFile == NULL)
	{
		char			str[MAX_PATH];
		char			name[MAX_PATH];
		int				no;
		
		split_path(appName, NULL, name, NULL);
		
		#ifdef linux			
			//--- delete old files
			char	str1[MAX_PATH];
			no = TRACE_FILE_CNT;
			sprintf(str, "%s%s_%d.txt", path, name, no);
			remove(str);
			for (no=TRACE_FILE_CNT-1; no>=0;  no--)
			{
				if (no) sprintf(str,  "%s%s_%d.txt", path, name, no);
				else	sprintf(str,  "%s%s.txt", path, name);
				sprintf(str1, "%s%s_%d.txt", path, name, no+1);
				rename(str, str1);
			}
			sprintf(_TraceFilePath,  "%s%s.txt", path, name);
			_TraceFile = fopen(_TraceFilePath, "w");
		#else
			struct tm		tm;
			__time64_t		time;
			_time64(&time);
			_localtime64_s(&tm, &time);
			sprintf(str,"%s%s*.txt", path, name);
			no = rx_remove_old_files(str, 2);
			if (no)	no++;
			else no = 1;
			sprintf(_TraceFilePath, "%s%s-%04d-%s-%02d_%03d.txt", path, name, 1900 + tm.tm_year, MonthStr[tm.tm_mon], tm.tm_mday, no);
			_TraceFile = rx_fopen(_TraceFilePath, "wt", _SH_DENYNO);
		#endif
		_TraceFileSize = 0;
	}
}

//--- Trace_get_path -------------------------------------
const char *Trace_get_path(void)
{
	return 	_TraceFilePath;				
}


//--- TrPrintf ------------------------------------------------------
void TrPrintf(int level, const char *format, ...)
{
	va_list ap;
	int len;
	char str[TRACE_STR_LEN];
	
	if (level && (_TraceToScreen || _TraceToFile))
	{
		STraceMsg msg;
		msg.hdr.msgId  = EVT_TRACE;
		msg.hdr.msgLen = sizeof(msg);	//sizeof(msg.hdr) + strlen(msg.message)+1;
		msg.time	   = rx_get_ticks();

		va_start(ap, format);
		len = vsnprintf(str, sizeof(msg.message)-3, format, ap);
		va_end(ap);
		#ifdef SOC
			strcat(str, "\r\n");
		#else
			strcat(str, "\n");
		#endif
		
		if(_Mutex)
		{
			rx_mutex_lock(_Mutex);
			memcpy(_TraceStr[_TraceStrIn], str, sizeof(_TraceStr[0]));
			len = (_TraceStrIn+1) % TRACE_STR_CNT;
			if(len!=_TraceStrOut) _TraceStrIn = len;
			rx_mutex_unlock(_Mutex);
			rx_sem_post(hTraceSem);				
		}
		else printf(str, NULL);
		
		strncpy(msg.message, str, sizeof(msg.message)-3);
		sok_send_to_clients(_hServer, &msg);
	}
} // end TrPrintf

//--- TrPrintfL ------------------------------------------------------
void TrPrintfL(int level, const char *format, ...)
{
	va_list ap;
	int		len;
	char   str[TRACE_STR_LEN];
	
	if (level && (_TraceToScreen || _TraceToFile))
	{		
		long t = rx_get_ticks();
		memset(str, 0, TRACE_STR_LEN);
		len = sprintf(str, "%s%ld.%03ld: ", _sAppName, t/1000, t%1000);
		va_start(ap, format);
		vsnprintf(&str[len], sizeof(str)-len-3, format, ap);
		va_end(ap);
		#ifdef SOC
			strcat(str, "\r\n");
		#else
			strcat(str, "\n");
		#endif
		
		if(_Mutex)
		{
			rx_mutex_lock(_Mutex);
			memcpy(_TraceStr[_TraceStrIn],str,sizeof(_TraceStr[0]));
			len = (_TraceStrIn+1) % TRACE_STR_CNT;
			if(len!=_TraceStrOut) _TraceStrIn = len;
			rx_mutex_unlock(_Mutex);
			rx_sem_post(hTraceSem);
		}
		else printf(str, NULL);
	}
} // end TrPrintf

//--- _trace_thread ---------------------------------------------
static void* _trace_thread(void* lpParameter)
{
	while (_TraceRunning)
	{
		rx_sem_wait(hTraceSem, 0);
		char *str = _TraceStr[_TraceStrOut];
		if (_TraceToScreen) printf(str, NULL);
		if (_TraceFile && _TraceToFile)
		{
			fprintf(_TraceFile, str, NULL);
			fflush(_TraceFile);
		}
		_TraceStrOut = (_TraceStrOut+1) % TRACE_STR_CNT;
	}
	return NULL;
}
