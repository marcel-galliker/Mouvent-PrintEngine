// ****************************************************************************
//
//	DIGITAL PRINTING - main
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#include <stdio.h>
#include "rx_common.h"
#include "rx_error.h"
#include "rx_setup_file.h"
#include "rx_setup_ink.h"
#include "rx_sok.h"
#include "rx_term.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "version.h"
#include "args.h"
#include "nios.h"
#include "conditioner.h"
#include "fpga.h"
#include "fpga_simu.h"
#include "head_ctrl.h"
#include "udp_test.h"
#include "DataSrv.h"
#include "mem_test.h"
#include "putty.h"
#include "nios_def_head.h"

//--- globals ------------------------------------------------------------
int						RX_VersionLinux;

RX_SOCKET				RX_MainSocket;
SHeadBoardCfg			RX_HBConfig;
int						RX_RGB[MAX_HEADS_BOARD];
SHeadBoardStat			RX_HBStatus[1];
SFluidStateLight		RX_FluidStat[MAX_HEADS_BOARD];

UINT32					RX_FpgaCmd;
SFpgaHeadStat			RX_FpgaStat;
SFpgaHeadError			RX_FpgaError;
SFpgaDataStat			RX_FpgaData;
SFpgaPrintList			RX_FpgaPrint;
SFpgaEncoderCfg			RX_FpgaEncCfg;
UINT32					RX_BlockUsed[MAX_HEADS_BOARD];
char					RX_MacAddr[2][32];
char					RX_IpAddr[2][32];


SNiosStat				RX_NiosStat;


static int		_AppRunning;

//--- prototypes ---------------------------------------------------------
static void _mem_test(void);

//--- _do_waveform ----------------------------------------------
static void _do_waveform(const char *fname)
{
	int i;
	char path[MAX_PATH];
	SInkDefinition inkdef;
	
	if (*fname) 	sprintf(path, "%s%s", PATH_BIN_HEAD, fname);
	else            sprintf(path, "%s%s", PATH_BIN_HEAD, "waveform");
	if (_stricmp(&path[strlen(path)-4], ".wfd")) strcat(path, ".wfd");
	if (setup_ink(path, &inkdef, READ)==REPLY_OK)
	{
		for (i=0; i<4; i++) nios_setInk(i, &inkdef, 3, 100);
		printf("WaveForm >>%s<< loaded\n", path);
	}
	else printf("\nERROR WaveForm >>%s<< not found or incorrect\n", path);
}

//--- main_menu ----------------------------------------------------------
void handle_menu(char *str)
{
	int synth=FALSE;
	static int cnt=0;
	int i;
	int no;
	
	no = str[1] - '0';
	switch (str[0])
	{
	case '0': cond_resetPumpTime(atoi(&str[1])); break;
			
	case 'd': nios_fixed_grey_levels(atoi(&str[1]), 3);	break;
	case 'g': fpga_manual_pg();							break;
	case 'h': fpga_enc_config(atoi(&str[1]));			break;
	case 'p': udp_test_print(&str[1]);					break;
	//case 'P': udp_test_print_tif(&str[1]);				break;
	/*
	case 'r': fpga_load(PATH_BIN_HEAD FIELNAME_HEAD_RBF);	
				nios_load(PATH_BIN_HEAD FIELNAME_HEAD_NIOS);
				fpga_set_config(INVALID_SOCKET);			
				break;
	*/
	case 't': udp_test_send_block(4, 1);				break;
	case 'T': udp_test_send_block(4, 10000);			break;
//	case 'u': fpga_display_used_flags();				break;
//	case 'w': _do_waveform(&str[1]);					break;
	case 'x': _AppRunning=FALSE;						break;
			
	case 'o': cond_ctrlMode(atoi(&str[1]), ctrl_off);		break;
	case 'w': cond_ctrlMode(atoi(&str[1]), ctrl_warmup);	break;
	case 'R': fpga_trace_registers("registers");			break;
	case 'r': cond_ctrlMode(atoi(&str[1]), ctrl_print);		break;
		
	case 'i': cond_heater_test(atoi(&str[1]));			break;
	case 'q': cond_start_log();							break;
			
	case 'E': cond_error_reset();						break;
	case 'l': cond_start_preslog();						break;
		
	// todo remove from final software -> toggle meniscus error check	
	case 'M': cond_toggle_meniscus_check();				break;
							
	// Only for DEBUGGING purposes
	// Parameters for tuning the Conditioner's PID controller

    case 'm': 
    case 'P':
    case 'I':
    case 'D':
    case 'O':
        cond_set_param(no, str[0], atoi(&str[2]));
        break;

	case 'u': if (no<=4) cond_offset_del(no);					break;
	case 'z': if (no<=4) cond_ctrlMode2(no, ctrl_offset_cal);	break;	

	/*
	case 'z': if (no<=4) cond_ctrlMode2(no, ctrl_offset_cal);			break; // 4 for all connected heads
	case 'B': if (no<4) cond_toggle_psensor_cali(no);					break;
	case 'b': if (no<4) cond_toggle_psensor_cali_user(no);				break;
	*/
	default: putty_handle_menu(str);
	}
}

//--- _main_loop -----------------------------------------------------------------------
static void _main_loop(void)
{
	int ticks;
	int t_menu=0;
	int menu;
	int msg;
	char str[64];

	rx_set_tread_priority(90);
	_AppRunning = TRUE;
	while (_AppRunning)
	{
		ticks = rx_get_ticks();
		menu = ticks>t_menu;
		msg=ctrl_main(ticks, menu);
		fpga_main(ticks, menu);
		nios_main(ticks, menu);
		if (menu) 
		{
			putty_display_status();
			putty_input(str, sizeof(str));
			handle_menu(str);
			t_menu = 500*(1+ticks/500);
		}
		if (!msg) rx_sleep(1);
	}
}

//--- _mem_test -----------------------------------------------------------
static void _mem_test(void)
{
	int		longCnt = 1024*1024;
	int		loop=10;
	int 	t0;
	int		i, ok, n;
	long	*buffer;

	printf("RX-Test\n");

	buffer = (long*)malloc(longCnt*4);
	if (buffer)
	{
		t0=rx_get_ticks();
		printf("Fill start\n");
		for(n=0; n<loop; n++)
		{
			for (i=0; i<longCnt; i++)
				buffer[i]=i;
		}
		printf("time=%d ms\n", rx_get_ticks()-t0);

		t0=rx_get_ticks();
		printf("Test start\n");
		ok = 1;
		for(n=0; n<loop; n++)
		{
			for (i=0; i<longCnt; i++)
				if (buffer[i]!=i) ok=0;
		}
		if (ok) printf("OK, time=%d ms\n", rx_get_ticks()-t0);
		else	printf("Error, time=%d ms\n", rx_get_ticks()-t0);

		free(buffer);
	}
	else printf("Error allocating buffer\n");
}

///--- main ---------------------------------------------------------------
int main(int argc, char** argv)
{
	char ver[32];
	Trace_init(argv[0]);

	args_init(argc, argv);
	rx_startup(argv[0], arg_debug);

	memset(&RX_HBConfig, 0, sizeof(RX_HBConfig));
	memset(&RX_HBStatus, 0, sizeof(RX_HBStatus));
	sscanf(version, "d.d.d.d", &RX_HBStatus[0].swVersion.major, &RX_HBStatus[0].swVersion.minor, &RX_HBStatus[0].swVersion.revision, &RX_HBStatus[0].swVersion.build);
	
	TrPrintfL(1, "rx_head_ctrl %s (%s, %s) started", version, __DATE__, __TIME__);

	rx_init();
	err_init(0, 100);
	nios_shutdown();
	fpga_init();
	nios_init();
	cond_init();
	putty_init();
	
	TrPrintfL(1, "Version %s", version);

	if (arg_offline)
	{
		_do_waveform("test.wfd");
		Error(WARN, 0, "SPECIAL HW TEST");
		fpga_set_config(INVALID_SOCKET);
	}
	
	if (!arg_offline) ctrl_init();
	
//	DataSrv_init();
	_main_loop();
	
	TrPrintfL(1, "Closing rx_head_ctrl\n");
	ctrl_end();
	nios_end();
	fpga_end();
	rx_end();
	TrPrintfL(1, "rx_head_ctrl ended");
	return 0;
}
