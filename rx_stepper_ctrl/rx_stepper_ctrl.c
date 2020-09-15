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
#include "cln.h"
#include "tx801.h"
#include "tx404.h"
#include "txrob.h"
#include "tx80x_wd.h"
#include "dp803.h"
#include "cleaf.h"
#include "lb701.h"
#include "lb702.h"
#include "lbrob.h"
#include "test.h"
#include "stepper_ctrl.h"
#include "slide.h"
#include "motor.h"
#include "test_table.h"
#include "fpga_stepper.h"
	

//--- globals ------------------------------------------------------------
SStepperStat	RX_StepperStatus;
SStepperCfg		RX_StepperCfg;

static int		_AppRunning;

//--- prototypes ---------------------------------------------------------
static void _main_loop();

//--- _main_menu -------------------------------------------------------

//--- _main_loop ----------------------------------------------------------
static void _main_loop()
{	
	int ticks;
	int menu;
	int t_menu=0;
	int msg;

	_AppRunning = TRUE;
	while(_AppRunning)
	{
		ticks = rx_get_ticks();
		menu = ticks>t_menu;
		if (menu) 
		{
			term_printf("\033[2J"); // Clear screen
			term_printf("rx_stepper_ctrl %s (%s, %s)", version, __DATE__, __TIME__);
		}

		msg=ctrl_main(ticks, menu);
		fpga_main (ticks, menu);
		motor_main(ticks, menu);
		
		//--- machine interfaces ----------------------
		
		switch (RX_StepperCfg.printerType)
		{
		case printer_test_table: 	tt_main(ticks, menu); break;
		case printer_TX801:			
		case printer_TX802:			if (RX_StepperCfg.boardNo == 0)
									    tx801_main(ticks, menu);
									else
									{
									    txrob_main(ticks, menu);
									    tx80x_wd_main();
									}
									break;
			
        case printer_TX404:			if (RX_StepperCfg.boardNo == 0)
									    tx404_main(ticks, menu);
									else
									{
									    txrob_main(ticks, menu);
									    tx80x_wd_main();
									}
									break;
										
			
		case printer_cleaf:			cleaf_main(ticks, menu);	break;			
		case printer_LB701: 		lb701_main(ticks, menu);	break;			
		case printer_LB702_UV: 		
		case printer_LB702_WB:		
		case printer_LH702:			lb702_main(ticks, menu); break;
//		case printer_LBROB:			lbrob_main(ticks, menu); break;
		case printer_DP803: 		dp803_main(ticks, menu); break;
			
		default:					test_main(ticks, menu); break;			
		}				
		
		if (menu) 
		{
			
			switch (RX_StepperCfg.printerType)
			{
			case printer_test_table: 	_AppRunning = tt_menu(); break;
			case printer_TX801:			
			case printer_TX802:			if (RX_StepperCfg.boardNo == 0) _AppRunning = tx801_menu(); 
										else							_AppRunning = txrob_menu(); 
										break;
            case printer_TX404:			if (RX_StepperCfg.boardNo == 0) _AppRunning = tx404_menu(); 
										else							_AppRunning = txrob_menu(); 
										break;                            
			case printer_cleaf:			_AppRunning = cleaf_menu(); break;

			case printer_LB701: 		_AppRunning = lb701_menu();	break;

			case printer_LB702_UV:
			case printer_LB702_WB:		
			case printer_LH702:			_AppRunning = lb702_menu(); break;

			case printer_DP803: 		_AppRunning = dp803_menu();	break;
			
			default:					_AppRunning = test_menu(); break;				
			}					
			
			t_menu = 500*(1+ticks/500);
		}
		if (!msg) rx_sleep(1);
	}
}

///--- main ---------------------------------------------------------------
int main(int argc, char** argv)
{
	memset(&RX_StepperStatus, 0, sizeof(RX_StepperStatus));
	memset(&RX_StepperCfg,	  0, sizeof(RX_StepperCfg));

	args_init(argc, argv);

	rx_startup(argv[0], arg_debug);
	Trace_init(argv[0]);

	TrPrintfL(1, "RxStepperCtrl %s started", version);
	
	sscanf(version, "%ld.%ld.%ld.%ld", &RX_StepperStatus.swVersion.major, &RX_StepperStatus.swVersion.minor, &RX_StepperStatus.swVersion.revision, &RX_StepperStatus.swVersion.build);
	sok_get_mac_address("eth0", &RX_StepperStatus.macAddr);
	RX_StepperStatus.serialNo = swap_uint16((RX_StepperStatus.macAddr>>32) & 0xffff);

	rx_init();
	err_init(0, 100);
	fpga_init();
//	nios_init();
	ctrl_init();
	motor_init();
	
	if (arg_simu) Error(WARN, 0, "SIMULATION");
	
	switch (RX_StepperCfg.printerType)
	{
	case printer_test_table: 	tt_init(); break;
	case printer_TX801:			
	case printer_TX802:			if (RX_StepperCfg.boardNo == 0)	tx801_init();
								else
								{
								    txrob_init();
								    tx80x_wd_init();
								}
								break;
     case printer_TX404:			if (RX_StepperCfg.boardNo == 0)	tx404_init();
								else
								{
								    txrob_init();
								    tx80x_wd_init();
								}
								break;
		
	case printer_cleaf:			cleaf_init(); break;
		
	case printer_LB701: 		lb701_init(); break;
		
	case printer_LB702_UV: 		
	case printer_LB702_WB: 		
	case printer_LH702:			lb702_init();	break;
//	case printer_LBROB:			lbrob_init();	break;
		
	case printer_DP803: 		dp803_init();	break;

	
	default:					test_init();
	}			
	
	Trace_to_screen(FALSE);
	_main_loop();
	Trace_to_screen(TRUE);

	tt_end();

//	nios_end();
	rx_end();
	TrPrintfL(1, "RxStepperCtrl ended");
	return 0;
}
