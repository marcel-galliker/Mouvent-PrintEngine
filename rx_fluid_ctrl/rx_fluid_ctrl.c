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

// *** DENX BOARD *************************************************************
//	user: root  / pwd: ""		-> SSH over ICP/IP and WinSCP
//	user: radex / pwd: "radex"	-> VisualGDB, debugging 
//
//  add user -g root radex
//

#include <stdio.h>
#include "version.h"
#include "rx_common.h"
#include "rx_def.h"
#include "rx_error.h"
#include "rx_sok.h"
#include "rx_term.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "args.h"
#include "fluid_ctrl.h"
#include "fpga_fluid.h"
#include "daisy_chain.h"
#include "nios_fluid.h"
	
//--- globals ------------------------------------------------------------

SFluidBoardStat	RX_FluidBoardStatus;

static int _AppRunning;
static int _DisplayBalance=FALSE;
static int _DisplayTestBleedLine = FALSE;

//--- prototypes ---------------------------------------------------------
static void main_menu();

//--- main_menu ----------------------------------------------------------
static void main_menu()
{	
	static int status=TRUE;
	int i;
	char str[MAX_PATH];

	term_printf("\n");
	term_printf("\nMENU -------------------------\n");
//	if (status) term_printf("s: hide status\n");
//	else        term_printf("s: show status\n");
	term_printf("o:          switch off all tests\n");			
	term_printf("a<n>:       Air-Valve[n]: switch\n");		
	term_printf("b<n>:       Bleed-Valve[n]: switch\n");		
	term_printf("i<n><mbar>: Ink-Pump[n] on until ink pressure > [mbar]\n");
	term_printf("v<mbar>:    Vacuum to [mbar] (0=test off)\n");
	term_printf("p<mbar>:    Pressure to [mbar] (0=test off)\n");
	term_printf("l<n>:          Test bleed line\n");			
	term_printf("f<x>:       Flush\n");	
	
	if (_DisplayBalance) 
	{
		term_printf("T<n<:       Tara balance <n>   \n");
		term_printf("B:          hide balance       \n");
	}
	else         
		term_printf("B:          show balance       \n");
	
	if(nios_is_heater_connected())	term_printf("t<n><temp>: set Heater[n] Temperature to [temp] (max. 60 degree)\n");

	//term_printf("PID: %d %d %d %d\n", _Cfg->controller_P, _Cfg->controller_I, _Cfg->controller_D, _Cfg->controller_offset);
	term_printf("q: start log\n");
	term_printf("x: exit\n");
	term_printf(">");
	term_flush();

	if (term_get_str(str, sizeof(str)))
	{
		int no   = str[1]-'0';
		switch (str[0])
		{
//		case 's': status = !status;							break;
		case 'o':	nios_test_stop();						break;
		case 'a':	nios_test_air_valve(no);				break;
		case 'B':   _DisplayBalance = !_DisplayBalance;		break;
		case 'f':	nios_test_flush(atoi(&str[1]));			break;
		case 'b':	nios_test_bleed_valve(no);				break;
		case 'i':	nios_test_ink_pump(no, atoi(&str[2]));	break;
		case 'v':	nios_test_vacuum(atoi(&str[1]));		break;
		case 'p':	nios_test_air_pressure(atoi(&str[1]));	break;
		case 't':	nios_set_temp(no, atoi(&str[2]));		break;
//		case 'q':	nios_start_temp_log();					break;
		case 'q':	nios_start_log();						break;
		case 'l':	nios_test_bleed_line(no); _DisplayTestBleedLine = 1; break;
	
/*
		case 'c':	cmd.no		= no;
					cmd.weight  = atoi(&str[2]);
					scl_calibrate(&cmd);					break;
*/
		case 'x': _AppRunning=FALSE;						break;
		case 'T': daisy_chain_do_tara(no);					break;
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

	_AppRunning = TRUE;
	while (_AppRunning)
	{
		ticks = rx_get_ticks();
		menu = ticks>t_menu;
		if (menu) 
		{
			term_printf("\033[2J"); // Clear screen
			term_printf("rx_fluid_ctrl %s (%s, %s)", version, __DATE__, __TIME__);
		}

		msg=ctrl_main(ticks, menu);
		fpga_main(ticks, menu);
		nios_main(ticks, menu);
		daisy_chain_main(ticks, menu, _DisplayBalance); 
		if (menu) 
		{
			main_menu();
			t_menu = 500*(1+ticks/500);
		}
		if (!msg) rx_sleep(1);
	}
}


///--- main ---------------------------------------------------------------
int main(int argc, char** argv)
{	
	args_init(argc, argv);
	rx_startup(argv[0], arg_debug);
	Trace_init(argv[0]);

	TrPrintfL(1, "RxFluidCtrl %s started", version);

	memset(&RX_FluidBoardStatus, 0, sizeof(RX_FluidBoardStatus));

	rx_init();
	err_init(0, 100);
	fpga_init();
	nios_init();
	ctrl_init();
	daisy_chain_init(); 

	Trace_to_screen(FALSE);
	_main_loop();
	Trace_to_screen(TRUE);

	daisy_chain_end(); 
	nios_end();
	rx_end();
	TrPrintfL(1, "RxFluidCtrl ended");
	return 0;
}
