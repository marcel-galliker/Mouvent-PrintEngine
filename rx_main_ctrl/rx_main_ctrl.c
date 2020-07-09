// ****************************************************************************
//
//	rx_main_ctrl.c
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#ifdef WIN32
#include <Windows.h>
#include <Dbghelp.h>
#endif

#include "rx_common.h"
#include "rx_error.h"
#include "rx_file.h"
#include "rx_term.h"
#include "rx_trace.h"
#include "rx_threads.h"
#include "version.h"
#include "tcp_ip.h"
#include "args.h"
#include "ctr.h"
#include "rfs.h"
#include "chiller.h"
#include "setup.h"
#include "network.h"
#include "print_queue.h"
#include "boot_svr.h"
#include "gui_svr.h"
#include "ctrl_svr.h"
#include "plc_ctrl.h"
#include "lh702_ctrl.h"
#include "pe_main.h"
#include "machine_ctrl.h"
#include "spool_svr.h"
#include "print_ctrl.h"
#include "prod_log.h"
#include "fluid_ctrl.h"
#include "rip_clnt.h"
#include "step_ctrl.h"
#include "datalogic.h"
#include "es_rip.h"
#include "enc_ctrl.h"
#include "errno.h"
#include "cleaf_orders.h"


//--- global variables -----------------------------

SRxConfig		RX_Config;
SSpoolerCfg		RX_Spooler;

//SRxStatus		RX_Status;
SRxNetwork		RX_Network;
SColorSplitCfg	RX_Color[MAX_COLORS];
SPrinterStatus	RX_PrinterStatus;
SStepperStat	RX_StepperStatus;
SStepperStat	RX_ClnStatus;
SPrintQueueItem RX_TestImage;
SHeadBoardStat	RX_HBStatus[HEAD_BOARD_CNT];
char			RX_Hostname[64];

//--- prototypes --------------------------------------

static void _menu(void);

#ifdef WIN32

#define FILENAME_MINIDUMP PATH_TEMP "rx_main_ctrl.dmp"

long WINAPI exception(LPEXCEPTION_POINTERS ExceptionInfo)
{
	HANDLE hFile;

	printf("Exception\n");

	hFile = CreateFile(FILENAME_MINIDUMP, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if (hFile!=INVALID_HANDLE_VALUE)
	{
		MINIDUMP_EXCEPTION_INFORMATION info;

		info.ThreadId			= GetCurrentThreadId();
		info.ExceptionPointers	= ExceptionInfo;
		info.ClientPointers		= FALSE;

		if (MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &info, NULL, NULL))
		{
			// wchar_t str[100];

			// wsprintf(str, L"An Exception occurred and a mini dump file has been created at >>%s<<", FILENAME_MINIDUMP);
			// AfxMessageBox(str);
		}
		CloseHandle(hFile);
	}

	return EXCEPTION_EXECUTE_HANDLER;
}
#endif

//--- _menu --------------------------------------------------------
static void _menu(void)
{
	char str[32];
	
	term_get_str(str, sizeof(str));
	if (str[0]) 
	{
		switch(str[0])
		{
		case 'i':	dl_identify(atoi(&str[1]));	break;
		case 't':	dl_trigger (atoi(&str[1]));	break;
        case 'g':	gui_test();break;
	#ifdef DEBUG
		case 'r':	ctr_calc_reset_key(&str[1]); break;
	#endif
		default:	break;
		}
		lh702_menu(str);
	}
}


//--- main -------------------------------------------------------------------
int main(int argc, char* argv[])
{
	int running = TRUE;

#ifdef WIN32
	SetUnhandledExceptionFilter(exception);
#endif

	rx_process_name(argv[0]);
	args_init(argc, argv);

	if (rx_startup(argv[0], arg_debug)!=REPLY_OK) return REPLY_ERROR;

	Trace_init(argv[0]);

	rx_set_process_priority(1);

	TrPrintfL(TRUE, "rx_main_ctrl V %s (%s / %s)", version, __DATE__, __TIME__);
	
	//--- initialize libraries ---------------------
	rx_mkdir(PATH_ROOT);
	rx_mkdir(PATH_TRACE);
	rx_mkdir(PATH_USER);
	rx_mkdir(PATH_RIPPED_DATA);
	rx_mkdir(PATH_SOURCE_DATA);
	rx_mkdir(PATH_EMBRIP_PRENV);
	rx_mkdir(PATH_FPGA_REGS);

	rx_mkdir(PATH_BIN);
	rx_mkdir(PATH_BIN_MAIN);
	rx_mkdir(PATH_BIN_SPOOLER);
	rx_mkdir(PATH_BIN_HEAD);
	rx_mkdir(PATH_BIN_FLUID);
	rx_mkdir(PATH_BIN_ENCODER);
	rx_mkdir(PATH_BIN_REXROTH);
	rx_mkdir(PATH_BIN_STEPPER);

	rx_mkdir(PATH_LOG);

	rx_remove_old_files(PATH_FPGA_REGS, 7);
	
	err_init(TRUE, 100);
	err_set_device(dev_main, 0);

	rx_init();
	rx_def_init();

	Error(LOG, 0, "STARTUP rx_main_ctrl V %s", version);

//	mx_test();

	/*
	if (FALSE)	
	{
		Error(WARN, 0, "TEST XML");
		HANDLE xml;
		SScreeningCfg screening;
		SRippingCfg ripping;
		xml = embrip_open_xml("D:\\PrEnv\\PrEnv1\\PrEnv1.xml");
		embrip_screening_cfg(xml, FALSE, &screening);
		embrip_ripping_cfg(xml, FALSE, &ripping);

		embrip_save_xml(xml, "D:\\PrEnv\\PrEnv1b\\PrEnv1b.xml");
	}
	*/
	
	//--- restart Spooler -------------------------------------
	if (!arg_debug)
	{
		#ifdef WIN32
			rx_process_kill(FILENAME_SPOOLER_CTRL, NULL);
			rx_process_start(PATH_BIN_WIN FILENAME_SPOOLER_CTRL".exe", NULL);
		#else
			rx_process_kill(FILENAME_SPOOLER_CTRL, NULL);
			rx_process_start(PATH_BIN_SPOOLER FILENAME_SPOOLER_CTRL, NULL);
		#endif
	}
	//--- main procedure ---------------------------
	setup_config(PATH_USER FILENAME_CFG, &RX_Config, READ);
	
	ctrl_update_hostname();
	
	if (!strcmp(RX_Hostname, "TEST-0001"))
	{
//		arg_simuPLC			= TRUE;
		arg_simuEncoder		= TRUE;
//		arg_simuHeads		= TRUE;
		arg_simuChiller		= TRUE;	
	}

	net_init();
	pq_init();
	pq_load(PATH_USER FILENAME_PQ);
	pc_init();
	machine_init();
	enc_init();
	fluid_init();
	step_init();
	chiller_init();
	pem_init();
	boot_start();
	gui_start();
	spool_start();
	ctrl_start();
	co_init();
	ctr_init();
	dl_init();
	pl_init();
	
	spool_auto(TRUE);
	
	while (running)
	{
		rx_sleep(1000);
		int t0=rx_get_ticks();
	//	TrPrintfL(TRUE, "ctrl_tick");
		ctrl_tick();
		int t1=rx_get_ticks()-t0;
	//	TrPrintfL(TRUE, "enc_tick");
		enc_tick();
		int t2=rx_get_ticks()-t1;
	//	TrPrintfL(TRUE, "pq_tick");
		pq_tick();
		int t3=rx_get_ticks()-t2;
	//	TrPrintfL(TRUE, "fluid_tick");
		fluid_tick();
		int t4=rx_get_ticks()-t3;
	//	TrPrintfL(TRUE, "step_tick");
		step_tick();
		int t5=rx_get_ticks()-t4;
	//	TrPrintfL(TRUE, "machine_tick");
		machine_tick();
		int t6=rx_get_ticks()-t5;
	//	TrPrintfL(TRUE, "spool_tick");
		spool_tick();
		int t7=rx_get_ticks()-t6;
	//	TrPrintfL(TRUE, "net_tick");
		net_tick();
		int t8=rx_get_ticks()-t7;
	//	TrPrintfL(TRUE, "chiller_tick");
		chiller_tick();
	//	co_tick();	// can hang sometimes!!!
		int t9=rx_get_ticks()-t8;
	//	TrPrintfL(TRUE, "ctr_tick");
		ctr_tick();
	//	TrPrintfL(TRUE, "gui_tick");
		_menu();
		gui_tick();
		int t10=rx_get_ticks()-t9;
	//	TrPrintfL(TRUE, "_menu");
		int t11=rx_get_ticks()-t10;
		if (rx_get_ticks()-t0>1000) 
		{
			char str[100];
			sprintf(str, "MAIN LOOP: t1=%d t2=%d t3=%d t4=%d t5=%d t6=%d t7=%d t8=%d t9=%d t10=%d t11=%d", t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11);
			Error(WARN, 0, str);
		}
	}

	//--- end libraries ----------------------------
	machine_end();
	pem_end();
	chiller_end();
	step_end();
	fluid_end();
	enc_end();
	pc_end();
	pq_end();
	rx_end();
	return 0;
}

