// ****************************************************************************
//
//	DIGITAL PRINTING - ctr.c
//
// ****************************************************************************
//
//	Copyright 2018 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#include "rx_def.h"
#include "rx_error.h"
#include "rx_file.h"
#include "rx_setup_file.h"
#include "rx_hash.h"
#include "rx_sok.h"
#include "rx_trace.h"
#include "gui_svr.h"
#include "lh702_ctrl.h"
#include "ctr.h"

#include <time.h>

//--- structs ------------------------------
typedef struct
{
	INT64 act;
	INT64 total;
	UINT64	macAddr;
	time_t time;
} _sctr;

//--- static variables -------------------
static int   _Time;
static INT64 _counterAct, _counterTotal;
static int   _jobLen;

//--- prototypes ---------------------------------------
static void _ctr_save(int reset, char *machineName);

//--- ctr_init --------------------------------------
void ctr_init(void)
{
	//--- init ---
	RX_PrinterStatus.counterAct = 0;
	RX_PrinterStatus.counterTotal = 0;
	_jobLen = 0;

	//--- read file ------------	
	HANDLE file = setup_create();
	setup_load(file, PATH_USER FILENAME_COUNTERS);
	
	if (setup_chapter(file, "Counters", -1, READ)==REPLY_OK)
	{
		setup_int64(file, "actual", READ, &RX_PrinterStatus.counterAct,   0);
		setup_int64(file, "total",  READ, &RX_PrinterStatus.counterTotal, 0);
		_counterAct = RX_PrinterStatus.counterAct;
		_counterTotal = RX_PrinterStatus.counterTotal;
	}
	setup_destroy(file);
		
	lh702_ctr_init();

	_ctr_save(FALSE, NULL);	
}

//--- ctr_tick -----------------------------
void ctr_tick(void)
{
	if (++_Time>30)
	{
		_Time = 0;
		if (RX_PrinterStatus.counterAct != _counterAct || RX_PrinterStatus.counterTotal != _counterTotal)
		{
			_ctr_save(FALSE, NULL);
			_counterAct = RX_PrinterStatus.counterAct;
			_counterTotal = RX_PrinterStatus.counterTotal;
		}
	}
}

//--- ctr_add -------------------------------------------
void ctr_add(int mm, UINT32 colors)
{
	if (rx_def_is_tx(RX_Config.printer.type)) // for tx, do not count offset
	{
		int encoderOffset = RX_Spooler.maxOffsetPx;
		_jobLen += mm;
		if (_jobLen >= encoderOffset / 1000)
		{
			if ((_jobLen - encoderOffset/1000) <= mm)
			{
				mm = (_jobLen - (encoderOffset)/1000);
			}
			RX_PrinterStatus.counterTotal	+= mm;
			RX_PrinterStatus.counterAct		+= mm;
		}
	}
	else
	{
		RX_PrinterStatus.counterTotal += mm;
		RX_PrinterStatus.counterAct += mm;
	}

	lh702_ctr_add(mm, colors);
}

//--- ctr_reset_jobLen ---------------
void ctr_reset_jobLen(void)
{
	_jobLen = 0;
}

//--- ctr_reset ---------------------------------------------
void ctr_reset(void)
{
	RX_PrinterStatus.counterAct = 0;
}

//--- _ctr_save --------------------------------------------------
static void _ctr_save(int reset, char *machineName)
{
	char   name[64];
	time_t time=rx_file_get_mtime(PATH_USER FILENAME_COUNTERS);

	HANDLE file = setup_create();
	if (reset) 
	{	
		RX_PrinterStatus.counterTotal = 0;
		strncpy(name, machineName, sizeof(name)-1);
	}
	else strncpy(name, RX_Hostname, sizeof(name)-1);

	if (setup_chapter(file, "Counters", -1, WRITE)==REPLY_OK)
	{
		setup_str	(file, "machine", WRITE, name, sizeof(name), "");
		setup_int64 (file, "actual", WRITE, &RX_PrinterStatus.counterAct, 0);
		setup_int64 (file, "total",  WRITE, &RX_PrinterStatus.counterTotal, 0);
	}

	setup_save(file, PATH_USER FILENAME_COUNTERS);
	setup_destroy(file);

	rx_file_set_mtime(PATH_USER FILENAME_COUNTERS, time);

	lh702_ctr_save(reset, machineName);
}