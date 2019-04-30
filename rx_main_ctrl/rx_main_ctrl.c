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
#include "rx_trace.h"
#include "rx_threads.h"
#include "version.h"
#include "tcp_ip.h"
#include "args.h"
#include "rfs.h"
#include "balance_def.h"
#include "chiller.h"
#include "setup.h"
#include "network.h"
#include "print_queue.h"
#include "boot_svr.h"
#include "gui_svr.h"
#include "ctrl_svr.h"
#include "plc_ctrl.h"
#include "pe_main.h"
#include "machine_ctrl.h"
#include "spool_svr.h"
#include "print_ctrl.h"
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
STestTableStat	RX_TestTableStatus;
STestTableStat	RX_ClnStatus;
SPrintQueueItem RX_TestImage;
SHeadBoardStat	RX_HBStatus[HEAD_BOARD_CNT];
char			RX_Hostname[64];

SScalesCalibration RX_ScalesCalibration[SCALES_CALIBRATION_CNT];

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
		
	TrPrintfL(TRUE, "rx_main_ctrl V %s", version);
	
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

//	dl_test();

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
	
	spool_auto(TRUE);
	
	while (running)
	{
		rx_sleep(1000);
		ctrl_tick();
		enc_tick();
		fluid_tick();
		step_tick();
		machine_tick();
		net_tick();
		chiller_tick();
		co_tick();
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

