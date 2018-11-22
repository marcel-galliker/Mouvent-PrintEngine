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

static HANDLE	 _Mutex		= NULL;
static HANDLE	 _hServer	= NULL;
static FILE		 *_TraceFile	= NULL;
static  char	 _sAppName_org[32];
static  char	 _sAppName[32];
static int		 _TraceToScreen = TRUE;
static int		 _TraceToFile   = TRUE;
static int		 _TraceFileSize;

//--- prototypes --------------------------------------------
static void _TraceFileOpen	(const char *path, const char *appName);

//--- Trace_init ----------------------------------------------
void Trace_init(const char *appName)
{
	strcpy(_sAppName_org, appName);
	_TraceFileOpen(PATH_TRACE, appName);
	if (appName)
	{
		split_path(appName, NULL, _sAppName, NULL);
		strcat(_sAppName, ": ");
	}
	else *_sAppName=0;
	if (!_Mutex) _Mutex = rx_mutex_create();
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
			sprintf(str,  "%s%s.txt", path, name);
			_TraceFile = fopen(str, "w");
		#else
			struct tm		tm;
			__time64_t		time;
			_time64(&time);
			_localtime64_s(&tm, &time);
			sprintf(str,"%s%s*.txt", path, name);
			no = rx_remove_old_files(str, 2);
			if (no)	no++;
			else no = 1;
			sprintf(str, "%s%s-%04d-%s-%02d_%03d.txt", path, name, 1900 + tm.tm_year, MonthStr[tm.tm_mon], tm.tm_mday, no);
			_TraceFile = rx_fopen(str, "wt", _SH_DENYNO);
		#endif
		_TraceFileSize = 0;
	}
}

//--- TrPrintf ------------------------------------------------------
void TrPrintf(int level, const char *format, ...)
{
	va_list ap;
	int len;

	if (level && (_TraceToScreen || _TraceToFile))
	{
		if (_Mutex==NULL) _Mutex=rx_mutex_create();
		rx_mutex_lock(_Mutex);
		STraceMsg msg;
		msg.hdr.msgId = EVT_TRACE;

		// get time
		msg.time = rx_get_ticks();

		va_start(ap, format);
		len = vsnprintf(msg.message, sizeof(msg.message)-3, format, ap);
		va_end(ap);
		msg.hdr.msgLen = sizeof(msg);//sizeof(msg.hdr) + strlen(msg.message)+1;
		sok_send_to_clients(_hServer, &msg);
		msg.message[len++]   = '\n';
		msg.message[len] = 0;
		if (_TraceFile && _TraceToFile)
		{
			_TraceFileSize += (int)fwrite(msg.message, 1, len+2, _TraceFile);
			if(_TraceFileSize < TRACE_FILE_SIZE_MAX) fflush(_TraceFile);
			else 
			{
				fclose(_TraceFile);
				_TraceFile = NULL;
				_TraceFileOpen(PATH_TRACE, _sAppName_org);					
			}
		}
		#ifdef SOC
			msg.message[len++]   = '\r';
		#endif
		if (_TraceToScreen) printf("%s", msg.message);
		msg.message[len]=0;
		rx_mutex_unlock(_Mutex);
	}
} // end TrPrintf

//--- TrPrintfL ------------------------------------------------------
void TrPrintfL(int level, const char *format, ...)
{
	va_list ap;
	int		len;
	char	str[270];

	if (level && (_TraceToScreen || _TraceToFile))
	{
		if (_Mutex==NULL) _Mutex=rx_mutex_create();
		rx_mutex_lock(_Mutex);
		long t = rx_get_ticks();
		memset(str, 0, sizeof(str));
		len = sprintf(str, "%s%ld.%03ld: ", _sAppName, t/1000, t%1000);
		va_start(ap, format);
		vsnprintf(&str[len], sizeof(str)-len-3, format, ap);
		#ifdef SOC
			strcat(str, "\r\n");
		#else
			strcat(str, "\n");
		#endif
		if (_TraceToScreen) printf("%s", str);
		if (_TraceFile && _TraceToFile)
		{
			fwrite(str, 1, strlen(str), _TraceFile);
			fflush(_TraceFile);
		}
		va_end(ap);
		rx_mutex_unlock(_Mutex);
	}
} // end TrPrintf