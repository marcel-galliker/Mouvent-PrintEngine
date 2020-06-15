// ****************************************************************************
//
//	DIGITAL PRINTING - mem_test.c
//
//	Communication between ARM processor and FPGA
//
// ****************************************************************************
//
//	Copyright 2015 by radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

//--- includes ----------------------------------------------------------------
#include <stdio.h>
#include "rx_common.h"
#include "rx_trace.h"
#include "mem_test.h"


//--- defines -----------------------------------------------------------

#define MAX_LIST_LEN	6
#define INTERVAL		60	// intervall in sec

typedef struct 
{
	UINT32	time;
	UINT32	memTotal;
	UINT32	memFree;
	UINT32	memAvailable;	
} SMemSize;

static SMemSize _MemSizeList[MAX_LIST_LEN]={0};
static int      _Counter=0;
static int		_Idx=0;
static int		_Time=0;

//--- mem_test --------------------------------
void mem_test()
{
	if (--_Counter<0)
	{
		_Counter = 2*INTERVAL;

		FILE *f;
		
		f = fopen("/proc/meminfo", "r");
		if (f)
		{
			_MemSizeList[_Idx].time = _Time++;
			fscanf(f, "MemTotal: %d kB\n",		&_MemSizeList[_Idx].memTotal);
			fscanf(f, "MemFree: %d kB\n",		&_MemSizeList[_Idx].memFree);
			fscanf(f, "MemAvailable: %d kB\n",	&_MemSizeList[_Idx].memAvailable);
			fclose(f);
			_Idx++;
			if (_Idx>=MAX_LIST_LEN) _Idx=3;
		}
	}
	int i;
	TrPrintfL(TRUE,  "Time:        "); for (i=0; i<MAX_LIST_LEN; i++) TrPrintfL(TRUE, "%10d", _MemSizeList[i].time);
	TrPrintfL(TRUE, "MemTotal:    "); for (i=0; i<MAX_LIST_LEN; i++) TrPrintfL(TRUE, "%10d", _MemSizeList[i].memTotal);
	TrPrintfL(TRUE, "MemFree:     "); for (i=0; i<MAX_LIST_LEN; i++) TrPrintfL(TRUE, "%10d", _MemSizeList[i].memFree);
	TrPrintfL(TRUE, "MemAvailable:"); for (i=0; i<MAX_LIST_LEN; i++) TrPrintfL(TRUE, "%10d", _MemSizeList[i].memAvailable);
	TrPrintfL(TRUE, "\n");
}
