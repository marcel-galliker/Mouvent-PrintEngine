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
#include "rx_sok.h"
#include "rx_term.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "args.h"
#include "fpga_enc.h"
#include "fpga_def_enc.h"
#include "enc_test.h"
#include "tw8.h"
#include "enc_ctrl.h"
#include "version.h"

//--- globals ------------------------------------------------------------
SEncoderStat	RX_EncoderStatus;
SEncoderCfg		RX_EncoderCfg;

static int		_AppRunning;
static int		_ShowCorrection=FALSE;
static int      _ShowParam = FALSE;

//--- prototypes ---------------------------------------------------------


//--- main_menu ----------------------------------------------------------
void main_menu()
{
	UINT64 addr=0;
	char str[MAX_PATH];
	char *ch;
	int cnt;
	int menu=TRUE;
	int synth=FALSE;
	SPageId id;
	SEncoderCfg cfg;

	cfg.incPerMeter = 1000000;
	term_printf("\n");
	term_printf("MENU ENCODER -------------------------\n");
	term_printf("h<xxx>: Herz Frquency in KHz\n");
	term_printf("n<0/1>: Enable Encoder 0\n");	
	term_printf("d<strokes>: Set Shift Delay <strokes>\n");	
	term_printf("r<strokes>: Reset Register\n");	
	term_printf("R: RESET encoder position\n");
	term_printf("o<x>: Toggle Output <x> (0..2)\n");
	term_printf("g<cnt> <dist>: PrintGo in x FP\n");	
	//term_printf("w: write CSV file with test data\n");
	term_printf("l: start/stop log of positions for CSV file\n");
	term_printf("c: show roll Correction\n");
	term_printf("p: show parameter\n");
	term_printf("x: exit\n");
	tw8_menu_print();
	term_printf(">");
	term_flush();
	
	if (term_get_str(str, sizeof(str)))
	{
		switch (str[0])
		{
		case 'c':   _ShowCorrection = !_ShowCorrection; break;
		case 'p':   _ShowParam = !_ShowParam; break;
		case 'g':	ch=strstr(str, " ");
					memset(&id, 0, sizeof(id));
					if (ch) fpga_pg_set_dist(&id, atoi(&str[1]), atoi(ch));	
					else    fpga_pg_set_dist(&id, 1,             atoi(&str[1])); 
					break;
			
		case 'h':	memset(&cfg, 0, sizeof(cfg));
					cfg.incPerMeter = 1000000;
					fpga_enc_config(0, &cfg, FALSE);
					fpga_enc_simu(atoi(&str[1]));
					break;
			
		case 'n':	fpga_encoder_enable(atoi(&str[1]), TRUE); break;

		case 'd':	fpga_shift_delay(atoi(&str[1])); break;
			
		case 'r':	fpga_encoder_reset_reg(); break;
			
		case 'R':	if (arg_test) fpga_enc_config_test();
					else
					{
						fpga_pg_init(FALSE);
					//	fpga_enc_config(0, &cfg, 0);
					//	fpga_pg_config_fhnw(0, 200000, 420000);						
					}
					break;
			
		case 'o':	fpga_output(atoi(&str[1]));	break;
		case 'w':	test_write_csv("encoder_test.csv");	break;
		case 'l':	fpga_start_poslog(); break;
		case 'x': _AppRunning=FALSE;	break;
		default:	tw8_menu_cmd(str);
		}
	}
}

//--- main_menu_test ----------------------------------------------------------
void main_menu_test()
{
	UINT64 addr=0;
	char str[MAX_PATH];
	int menu=TRUE;
	int synth=FALSE;
	SEncoderCfg cfg;

	cfg.incPerMeter = 1000000;
	term_printf("\n");
	term_printf("MENU ENCODER TEST -------------------------\n");
	term_printf("h<xxx>: Herz Frquency in KHz\n");
	term_printf("r:		RESET encoder positions\n");
	term_printf("o<x>:	Toggle Output <x> (0..n)\n");
	term_printf("x: exit\n");
	term_printf(">");
	term_flush();
	
	if (term_get_str(str, sizeof(str)))
	{
		switch (str[0])
		{
		case 'h':	fpga_enc_config(0, &cfg, FALSE);
					
					fpga_enc_simu(atoi(&str[1]));
					break;
		case 'r':	fpga_enc_config_test();		break;
		case 'o':	fpga_output(atoi(&str[1]));	break;
		case 'x': _AppRunning=FALSE;	break;
		}
	}
}

//--- _main_loop -----------------------------------------------------------------------
static void _main_loop(void)
{
	static int t_menu=0;
	int ticks;
	int menu;
	int msg;

	_AppRunning = TRUE;
	while (_AppRunning)
	{
		ticks = rx_get_ticks();
		menu = (ticks>t_menu);
		if (menu) 
		{
			term_printf("\033[2J"); // Clear screen
			term_printf("rx_encoder_ctrl %s (%s, %s)", version, __DATE__, __TIME__);
		}

		msg=ctrl_main(ticks, menu);
		fpga_main(ticks, menu, _ShowCorrection, _ShowParam);
		tw8_main(ticks, menu);
		if (menu) 
		{
			if (arg_test)	main_menu_test();
			else			main_menu();
			t_menu = 500*(1+ticks/500);
		}
		if (!msg) rx_sleep(1);
	}
}

///--- main ---------------------------------------------------------------
int main(int argc, char** argv)
{
	rx_process_name(argv[0]);
	args_init(argc, argv);
	rx_startup(argv[0], arg_debug);
	Trace_init(argv[0]);

	TrPrintfL(1, "rx_encoder_ctrl %s started", version);

	rx_init();
	err_init(0, 100);
	fpga_init();
	tw8_init();
	ctrl_init();
	
	fpga_stop_printing();

	if (arg_fhnw)
	{
		SEncoderCfg cfg;
		cfg.orientation = TRUE;
		cfg.incPerMeter = 1000000;
		cfg.pos_pg_fwd  = 200000; 
		cfg.pos_pg_bwd  = 420000;
		cfg.pos_actual  = 0;
		cfg.correction  = 0;
		cfg.scanning	= TRUE;
		fpga_enc_config(0, &cfg, FALSE);
		fpga_pg_config_fhnw(0, cfg.pos_pg_fwd, cfg.pos_pg_bwd);		
	}
	
	if (arg_cleaf)
	{
		SEncoderCfg cfg;
		cfg.orientation = FALSE;
		cfg.incPerMeter = 188212;
		cfg.pos_pg_fwd  = 1000; 
		cfg.pos_pg_bwd  = 0;
		cfg.pos_actual  = 0;
		cfg.correction  = 0;
		cfg.scanning	= FALSE;
		fpga_enc_config(0, &cfg, FALSE);
		fpga_pg_config(INVALID_SOCKET, &cfg, FALSE);		
	}
	
	if (arg_testslide)
	{
		SEncoderCfg cfg;
		cfg.orientation = FALSE;
		cfg.incPerMeter = 1000000;
		cfg.pos_pg_fwd  = 150*1000; 
		cfg.pos_pg_bwd  = 1000000;
		cfg.pos_actual  = 0;
		cfg.correction  = 0;
		cfg.scanning	= TRUE;
		fpga_enc_config(0, &cfg, FALSE);
		fpga_pg_config(INVALID_SOCKET, &cfg, FALSE);		
	}

	if (arg_test) fpga_enc_config_test();
			
	Trace_to_screen(FALSE);
	_main_loop();
	Trace_to_screen(TRUE);

	TrPrintfL(1, "Closing rx_encoder_ctrl");
	fpga_end();
	rx_end();
	tw8_end();
	TrPrintfL(1, "rx_encoder_ctrl ended");
	return 0;
}
