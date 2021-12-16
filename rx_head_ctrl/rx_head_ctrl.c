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
#include "rx_file.h"
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
#include "rx_head_ctrl.h"

#include <pthread.h>
#include <sys/time.h>
#include <sys/resource.h>

//--- globals ------------------------------------------------------------
int						RX_LinuxDeployment;
RX_SOCKET				RX_MainSocket;
SHeadBoardCfg			RX_HBConfig;
int						RX_RGB[MAX_HEADS_BOARD];
SHeadBoardStat			RX_HBStatus[1];
SFluidStateLight		RX_FluidStat[MAX_HEADS_BOARD];
SColorSplitCfg			RX_Color[MAX_COLORS];

UINT32					RX_FpgaCmd;
SFpgaHeadStat			RX_FpgaStat;
SFpgaHeadStat			RX_FpgaStatRunning;
SFpgaHeadError			RX_FpgaError;
SFpgaDataStat			RX_FpgaData;
SFpgaPrintList			RX_FpgaPrint;
SFpgaEncoderCfg			RX_FpgaEncCfg;
UINT8					RX_GreyLevel[MAX_HEADS_BOARD][MAX_DROP_SIZES];

INT32					RX_UdpSpeed[2];
UINT32					RX_BlockUsed[MAX_HEADS_BOARD];
char					RX_MacAddr[2][32];
char					RX_IpAddr[2][32];

SNiosStat				RX_NiosStat;


static int		_AppRunning;
static int		_WaveFormLoaded = FALSE;

//--- prototypes ---------------------------------------------------------
static void _mem_test(void);
static void	_check_rx_boot(void);
static void _tickle_puls(void);


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
		for (i=0; i<4; i++) nios_setInk(i, &inkdef, "SML", 100);
		_WaveFormLoaded = TRUE;
	}
	else Error(WARN, 0, "ERROR WaveForm >>%s<< not found or incorrect", path);
}

//--- main_menu ----------------------------------------------------------
void handle_menu(char *str)
{
	int synth=FALSE;
	static int cnt=0;
	int i;
	int no;
	
	if      (no=str_start(str, "cluster"))		cond_set_clusterNo(atoi(&str[no]));
	else if (no=str_start(str, "resetinkctr"))	
	{
		cond_volume_printed(atoi(&str[no]), 0);
		cond_reset_droplets_printed(atoi(&str[no]));
	}
	else if (no=str_start(str,"ra"))			cond_set_rob_pos(str[no]-'0', str_to_screw(&str[no+1]), 0);
	else if (no=str_start(str,"rd"))			cond_set_rob_pos(str[no]-'0', 0, str_to_screw(&str[no+1]));
	else
	{
		no = str[1] - '0';
		switch (str[0])
		{
		case '0': cond_resetPumpTime(atoi(&str[1])); break;
			
		case 'd': nios_fixed_grey_levels(atoi(&str[1]), 3);	break;
		case 'g': fpga_manual_pg();							break;
		case 'h': fpga_enc_config(atoi(&str[1]));			break;
		case 'p': udp_test_print(&str[1]);					break;
	//  case 'p': udp_test_print_tif(&str[1]);				break;
		/*
		case 'r': fpga_load(PATH_BIN_HEAD FIELNAME_HEAD_RBF);	
					nios_load(PATH_BIN_HEAD FIELNAME_HEAD_NIOS);
					fpga_set_config(INVALID_SOCKET);			
					break;
		*/
	//	case 't': udp_test_send_block(4, 1);				break;
	//	case 'T': udp_test_send_block(4, 10000);			break;
//		case 's': fpga_overheated();						break;
		case 't': ctrl_stress_test();						break;
		
	//	case 'u': fpga_display_used_flags();				break;
	//	case 'w': _do_waveform(&str[1]);					break;
		case 'x': _AppRunning=FALSE;						break;
			
		case 'o': cond_ctrlMode(atoi(&str[1]), ctrl_off);		break;
		case 'w': cond_ctrlMode(atoi(&str[1]), ctrl_warmup);	break;
		case 'R': fpga_trace_registers("registers", FALSE);		break;
		case 'r': cond_ctrlMode(atoi(&str[1]), ctrl_print);		break;
		
		case 'i': cond_heater_test(atoi(&str[1]));			break;
		case 'q': cond_start_log();							break;
			
		case 'E': cond_error_reset();						break;
		case 'l': cond_start_preslog();						break;
		
		case 'f': cond_set_flowResistance(str[1]-'0', atoi(&str[2]));		break;
		// todo remove from final software -> toggle meniscus error check	
		case 'M': cond_toggle_meniscus_check();				break;
							
		// Only for DEBUGGING purposes
		// Parameters for tuning the Conditioner's PID controller

		//case 'z': if (no<=4) cond_ctrlMode2(no, ctrl_offset_cal);	break;	

		/*
		case 'z': if (no<=4) cond_ctrlMode2(no, ctrl_offset_cal);			break; // 4 for all connected heads
		case 'B': if (no<4) cond_toggle_psensor_cali(no);					break;
		case 'b': if (no<4) cond_toggle_psensor_cali_user(no);				break;
		*/
		default: putty_handle_menu(str);
		}
	}
}

//--- _main_loop -----------------------------------------------------------------------
static void _main_loop(void)
{
	int ticks;
	int t_menu=0;
	int menu;
	int msg;
	int cfgCnt=0, cnt;
	int printing;
	int connected;
	int shutdown_time=0;
	char str[64];
	int ctrlInit=FALSE;
	int time0=0, time1=0, time2=0, time3=0, time4=0, time5=0, time6=0;

//	rx_set_tread_priority(50);
	_AppRunning = TRUE;
	connected=0;
	while (_AppRunning)
	{
		printing = ctrl_printing();
		ticks = rx_get_ticks();
		
		cnt = fpga_cfg_cnt();		
		if (printing && cnt==cfgCnt && ticks-time0>100) Error(WARN, 0, "MAIN cycletime=%d ,t1=%d, t2=%d, t3=%d, t4=%d, t5=%d, t6=%d", ticks-time0, time1-time0, time2-time0, time3-time0, time4-time0, time5-time0, time6-time0); 
		cfgCnt = cnt;
		
		time0=ticks;
		time1 = rx_get_ticks();		
		menu = ticks>t_menu;
		msg=ctrl_main(ticks, menu);
		time2 = rx_get_ticks();
		fpga_main(ticks, menu);
		time3 = rx_get_ticks();
		nios_main(ticks, menu);
		time4 = rx_get_ticks();
		if (menu && fpga_is_init()) 
		{
			putty_input(str, sizeof(str));
			handle_menu(str);
			t_menu = 500*(1+ticks/500);
		//	_check_rx_boot();
		}
		time5 = rx_get_ticks();
		if (!ctrlInit && nios_loaded() && !arg_offline) 
		{
			ctrlInit=TRUE;
			ctrl_init();
		}

		if (ctrlInit && !RX_HBConfig.debug)
		{
			if (ctrl_connected())
			{
				connected = TRUE;
				shutdown_time = 0;
			}
			else if (connected)
			{
				shutdown_time = rx_get_ticks()+2000;
				TrPrintfL(TRUE, "shutdown_time=%d", shutdown_time);
				connected = FALSE;
			}
			else if (shutdown_time && rx_get_ticks()>shutdown_time)
			{
				Error(ERR_ABORT, 0, "Connection to rx_main_ctrl lost");
				TrPrintfL(TRUE, "shutdown_time=%d: _AppRunning=FALSE", shutdown_time);
				_AppRunning=FALSE;										
			}
		}
		_tickle_puls();
		if (!msg) rx_sleep(10);
		time6= rx_get_ticks();
	}
}

static void _tickle_puls(void)
{
#define TIME_MIN_IN_MS 60000
	static int time_ms = 0;
	int no[MAX_HEADS_BOARD];

	for (int i = 0; i < MAX_HEADS_BOARD; i++) no[i] = RX_HBConfig.reverseHeadOrder ? MAX_HEADS_BOARD - 1 - i : i;
	
	for (int i = 0; i < SIZEOF(RX_FpgaStat.enc_speed); i++)
	{
		if (RX_FpgaStat.enc_speed[i].current >= 400 || RX_HBStatus[0].head[i].ctrlMode != ctrl_print || (RX_HBStatus[0].head[no[i]].imgInCnt - RX_HBStatus[0].head[no[i]].printDoneCnt != 0))
		{
			time_ms = 0;
			// If all Heads off set fpga enc config to 0
			for (int j = 0; j < MAX_HEADS_BOARD; j++)
			{
				if (RX_HBStatus[0].head[j].ctrlMode > ctrl_off) return;
			}
			fpga_enc_config(0);
			return;
		}
	}
	if (!time_ms) time_ms = rx_get_ticks() + TIME_MIN_IN_MS;
	if (rx_get_ticks() > time_ms)
	{
		do_jetting(400, 0);
		time_ms = 0;
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

	TrPrintfL(TRUE, "RX-Test\n");

	buffer = (long*)malloc(longCnt*4);
	if (buffer)
	{
		t0=rx_get_ticks();
		TrPrintfL(TRUE, "Fill start\n");
		for(n=0; n<loop; n++)
		{
			for (i=0; i<longCnt; i++)
				buffer[i]=i;
		}
		TrPrintfL(TRUE, "time=%d ms\n", rx_get_ticks()-t0);

		t0=rx_get_ticks();
		TrPrintfL(TRUE, "Test start\n");
		ok = 1;
		for(n=0; n<loop; n++)
		{
			for (i=0; i<longCnt; i++)
				if (buffer[i]!=i) ok=0;
		}
		if (ok) TrPrintfL(TRUE, "OK, time=%d ms\n", rx_get_ticks()-t0);
		else	TrPrintfL(TRUE, "Error, time=%d ms\n", rx_get_ticks()-t0);

		free(buffer);
	}
	else TrPrintfL(TRUE, "Error allocating buffer\n");
}


//--- _check_rx_boot --------------------------------------------------
static void	_check_rx_boot(void)
{
	int cnt= rx_process_running_cnt("rx_boot_soc", NULL);
	if (cnt==0) rx_process_start(PATH_ROOT_LX "rx_boot_soc", NULL);
}

///--- main ---------------------------------------------------------------
int main(int argc, char** argv)
{
	char ver[32];
	
	rx_process_name(argv[0]);
	args_init(argc, argv);
	rx_startup(argv[0], arg_debug);

	Trace_init(argv[0]);

	memset(&RX_HBConfig, 0, sizeof(RX_HBConfig));
	memset(&RX_HBStatus, 0, sizeof(RX_HBStatus));
	sscanf(version, "d.d.d.d", &RX_HBStatus[0].swVersion.major, &RX_HBStatus[0].swVersion.minor, &RX_HBStatus[0].swVersion.revision, &RX_HBStatus[0].swVersion.build);
	
	TrPrintfL(1, "rx_head_ctrl %s (%s, %s) started", version, __DATE__, __TIME__);
	
	/* test speed
	{
		struct sched_param params;
		int policy;
		pthread_getschedparam(pthread_self(), &policy, &params);
		TrPrintfL(TRUE, "Thread Policy=%d, prio=%d", policy, params.sched_priority);
		
		TrPrintfL(TRUE, "priority User=%d, Group=%d, Process=%d", getpriority(PRIO_USER, 0), getpriority(PRIO_PGRP, 0), getpriority(PRIO_PROCESS, 0));
		TrPrintfL(TRUE, "userID=%d", geteuid());
	}
	
	setpriority(PRIO_USER,    0, -20);
	setpriority(PRIO_PGRP,    0, -20);
	setpriority(PRIO_PROCESS, 0, -20);
	*/
	
	//--- remove test bitmaps ---------
	{
		char path[100];
		sprintf(path, "%s*.bmp", PATH_TEMP);
		rx_remove_old_files(path, 0);
	}

	rx_init();
	err_init(0, 100);
//	nios_shutdown();
	fpga_init(FIELNAME_HEAD_RBF);
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
	
//	if (!arg_offline) ctrl_init();
	
//	DataSrv_init();
	Trace_to_screen(FALSE);
	_main_loop();
	Trace_to_screen(TRUE);
	
	TrPrintfL(1, "Closing rx_head_ctrl\n");
	ctrl_end();
	nios_end();
	fpga_end();
	rx_end();
	TrPrintfL(1, "rx_head_ctrl ended");
	return 0;
}

//--- do_jetting -------------------------------------
void do_jetting(int freq, int greyLevel)
{
	if (freq)
	{
		_do_waveform("test.wfd");
		Error(WARN, 0, "Cluster in Jetting Mode");
		int arg = arg_offline;
		arg_offline = TRUE;
		fpga_set_config(INVALID_SOCKET);
		arg_offline = arg;
		nios_fixed_grey_levels(greyLevel, 3);
	}
	fpga_enc_config(freq);
}