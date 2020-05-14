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
#define NEW_COUNTER	FALSE

//--- structs ------------------------------

typedef struct
{
	double act;
	double total;
	UINT64	macAddr;
	time_t time;
} _sctr;

//--- static variables -------------------
static int		_Manipulated=FALSE;
static int		_Time;
static int		_prodCnt;
static double	_prodLen;
static double	_jobLen;

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
		setup_double(file, "actual", READ, &RX_PrinterStatus.counterAct,   0);
		setup_double(file, "total",  READ, &RX_PrinterStatus.counterTotal, 0);
		setup_str   (file, "check",  READ, check1, sizeof(check1), "");
        #if NEW_COUNTER==0
			RX_PrinterStatus.counterTotal = RX_PrinterStatus.counterAct;
        #endif
	}
	setup_destroy(file);
	
	_calc_check(rx_file_get_mtime(PATH_USER FILENAME_COUNTERS), check2);
	
	_Manipulated = (strcmp(check1, check2))!=0;
    #if NEW_COUNTER==1
		if (_Manipulated)
		{
			_calc_reset_key(RX_Hostname, check2);
			if (!strcmp(check1, check2))
			{
				_Manipulated = FALSE;
				RX_PrinterStatus.counterTotal=0;
			}
		}
		if (_Manipulated) Error(ERR_CONT, 0, "Counters manipulated");
	
		_ctr_save(FALSE, NULL);	
    #endif
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

//--- ctr_set_total -----------------------------
void ctr_set_total(UINT32 machineMeters)
{
    #if NEW_COUNTER==0
		if (machineMeters>RX_PrinterStatus.counterTotal) 
		{
			RX_PrinterStatus.counterTotal=machineMeters;		
			gui_send_printer_status(&RX_PrinterStatus);		
		}
    #endif
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
void ctr_add(double m)
{
    double encoderOffset=0; 
	if (rx_def_is_tx(RX_Config.printer.type)) encoderOffset=(double)RX_Spooler.maxOffsetPx;
    _prodLen += m;
    _jobLen += m;
    _prodCnt++;
    if ((_jobLen*1000000 >= encoderOffset))
    {
        if ((_jobLen * 1000000 - encoderOffset) <= m * 1000000)
        {
            m = (_jobLen - (encoderOffset)/1000000);
        }
		RX_PrinterStatus.counterTotal	+= m;
		RX_PrinterStatus.counterAct		+= m;
    }  
}

//--- ctr_reset ---------------------------------------------
void ctr_reset(void)
{
	RX_PrinterStatus.counterAct = 0;
	_ctr_save(FALSE, NULL);
}

//--- calc_reset_key -------------------------------------------
void calc_reset_key(char *machineName)
{
	UINT64	macAddr;
	UCHAR   key[64];
	sok_get_mac_address(DEVICE_NAME, &macAddr);
	//printf("Mac-Addr: %08x%08x\n", macAddr >> 32, macAddr);

	if (macAddr==0x0000f7b6bf661fc8 || macAddr == 0x0000397db0eaf108)
	{
		_calc_reset_key(machineName, key);
		printf("KEY:\n%s\n", key);
	}
}

//--- _ctr_save --------------------------------------------------
static void _ctr_save(int reset, char *machineName)
{
//    Error(LOG, 0, "Counters: act=%d, total=%d, (products=%d, m=%d.%03d) ", (int)RX_PrinterStatus.counterAct, (int)RX_PrinterStatus.counterTotal, (int)_prodCnt, (int)_prodLen, (int)(_prodLen*1000.0)%1000);
    TrPrintfL(TRUE, "Counters: act=%d, total=%d, (products=%d, m=%d.%03d) ", (int)RX_PrinterStatus.counterAct, (int)RX_PrinterStatus.counterTotal, (int)_prodCnt, (int)_prodLen, (int)(_prodLen*1000.0)%1000);
    _prodCnt=0;
    _prodLen=0;

	{
		char	name[64];
		UCHAR check[64];
		time_t time;

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
			setup_double(file, "actual", WRITE, &RX_PrinterStatus.counterAct, 0);
			setup_double(file, "total",  WRITE, &RX_PrinterStatus.counterTotal, 0);

			#if NEW_COUNTER==0
				_Manipulated = FALSE;
				reset		 = FALSE;
			#endif
			if (reset)
			{
				_calc_reset_key(name, check);
			}
			else if (_Manipulated) 
			{
				Error(ERR_CONT, 0, "Counters manipulated");
				strcpy(check, "Manipulated");
			}
			else
			{
				#ifdef linux
					struct timespec now;
					clock_gettime( CLOCK_REALTIME, &now);
					time = now.tv_sec;
				#else
					_time64(&time);
				#endif
				_calc_check(time, check);
			}
			setup_str	(file, "check", WRITE, check, sizeof(check), "");
		}

		rx_file_set_readonly(PATH_USER FILENAME_COUNTERS, FALSE);
		setup_save(file, PATH_USER FILENAME_COUNTERS);
		setup_destroy(file);
		rx_file_set_readonly(PATH_USER FILENAME_COUNTERS, TRUE);
		rx_file_set_mtime(PATH_USER FILENAME_COUNTERS, time);
	}
}

void reset_job_Len(void)
{
        _jobLen = 0;
}
