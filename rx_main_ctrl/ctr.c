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
#include "ctr.h"

#include <time.h>

#define DEVICE_NAME	"em2"

//--- structs ------------------------------

typedef struct
{
	INT64 act;
	INT64 total;
	UINT64	macAddr;
	time_t time;
} _sctr;

//--- static variables -------------------
static int		_Manipulated=FALSE;
static int		_Time;
static int		_prodCnt;
static int		_prodLen;
static int		_jobLen;

//--- prototypes ---------------------------------------
static void _calc_check(time_t time, UCHAR *check);
static void _calc_reset_key(char *machineName, UCHAR *key);
static void _ctr_save(int reset, char *machineName);

//--- ctr_init --------------------------------------
void ctr_init(void)
{	
	UCHAR	check1[64];
	UCHAR	check2[64];

	//--- init ---
	RX_PrinterStatus.counterAct = 0;
	RX_PrinterStatus.counterTotal = 0;
    _prodCnt = 0;
    _prodLen = 0;
    _jobLen = 0;
	_Time   = 0;

	//--- read file ------------	
	HANDLE file = setup_create();
	setup_load(file, PATH_USER FILENAME_COUNTERS);
	
	if (setup_chapter(file, "Counters", -1, READ)==REPLY_OK)
	{
		setup_int64(file, "actual", READ, &RX_PrinterStatus.counterAct,   0);
		setup_int64(file, "total",  READ, &RX_PrinterStatus.counterTotal, 0);
		setup_str   (file, "check",  READ, check1, sizeof(check1), "");
	}
	setup_destroy(file);
	
	_calc_check(rx_file_get_mtime(PATH_USER FILENAME_COUNTERS), check2);
	
	_Manipulated = (strcmp(check1, check2))!=0;
	if (_Manipulated)
	{
		_calc_reset_key(RX_Hostname, check2);
		if (!strcmp(check1, check2))
		{
			_Manipulated = FALSE;
			RX_PrinterStatus.counterTotal=0;
		}
	}
	if (_Manipulated) 
		Error(ERR_CONT, 0, "Counters manipulated");
	
	_ctr_save(FALSE, NULL);	
}

//--- _calc_check ---------------------------------
static void _calc_check(time_t time, UCHAR *check)
{
	_sctr ctr;
	memset(&ctr, 0, sizeof(ctr));
	sok_get_mac_address(DEVICE_NAME, &ctr.macAddr);
	ctr.time  = time;
	ctr.act   = RX_PrinterStatus.counterAct;
	ctr.total = RX_PrinterStatus.counterTotal;
	rx_hash_mem_str((UCHAR*)&ctr, sizeof(ctr), check);
}

//--- _calc_reset_key -----------------------------------------
static void _calc_reset_key(char *machineName, UCHAR *key)
{
	char str[MAX_PATH];

	time_t rawtime;
	struct tm *t;

	time (&rawtime);
	t=localtime(&rawtime);
	sprintf(str, "%s:%d-%d-%d", machineName, 1900+t->tm_year, 1+t->tm_mon, t->tm_mday);
	rx_hash_mem_str(str, strlen(str), key);
}

//--- ctr_tick -----------------------------
void ctr_tick(void)
{
	if (++_Time>60)
	{
		_ctr_save(FALSE, NULL);
		_Time=0;			
	}
}

//--- ctr_add -------------------------------------------
void ctr_add(int mm)
{
    int encoderOffset=0; 
	if (rx_def_is_tx(RX_Config.printer.type)) encoderOffset=RX_Spooler.maxOffsetPx;
    _prodLen += mm;
    _jobLen += mm;
    _prodCnt++;
    if ((_jobLen*1000 >= encoderOffset))
    {
        if ((_jobLen * 1000 - encoderOffset) <= mm * 1000)
        {
            mm = (_jobLen - (encoderOffset)/1000);
        }
		RX_PrinterStatus.counterTotal	+= mm;
		RX_PrinterStatus.counterAct		+= mm;
    }  
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
	_ctr_save(FALSE, NULL);
}

//--- ctr_calc_reset_key -------------------------------------------
void ctr_calc_reset_key(char *machineName)
{
	UINT64	macAddr;
	UCHAR   key[64];
	sok_get_mac_address(DEVICE_NAME, &macAddr);
	//printf("Mac-Addr: %08x%08x\n", macAddr >> 32, macAddr);

//	if (macAddr==0x0000f7b6bf661fc8 || macAddr == 0x0000397db0eaf108)
	{
		_calc_reset_key(machineName, key);
		printf("KEY:\n%s\n", key);
	}
}

//--- _ctr_save --------------------------------------------------
static void _ctr_save(int reset, char *machineName)
{
    TrPrintfL(TRUE, "Counters: act=%d, total=%d, (products=%d, m=%d.%03d) ", (int)RX_PrinterStatus.counterAct, (int)RX_PrinterStatus.counterTotal, (int)_prodCnt, _prodLen/1000, _prodLen%1000);
    _prodCnt=0;
    _prodLen=0;

	{
		char   name[64];
		UCHAR  check[64];
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

			if (reset)
			{
				time = rx_get_system_sec();
				_calc_reset_key(name, check);
			}
			else if (_Manipulated) 
			{
				Error(ERR_CONT, 0, "Counters manipulated");
				strcpy(check, "Manipulated");
			}
			else
			{
				time = rx_get_system_sec();
				_calc_check(time, check);
			}
			setup_str	(file, "check", WRITE, check, sizeof(check), "");
		}

	//	rx_file_set_readonly(PATH_USER FILENAME_COUNTERS, FALSE);
		setup_save(file, PATH_USER FILENAME_COUNTERS);
		setup_destroy(file);
	//	rx_file_set_readonly(PATH_USER FILENAME_COUNTERS, TRUE);
		rx_file_set_mtime(PATH_USER FILENAME_COUNTERS, time);
	}
}