// ****************************************************************************
//
//	DIGITAL PRINTING - web.c
//
//	Communication between ARM processor and FPGA
//
// ****************************************************************************
//
//	Copyright 2016 by radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#include "rx_term.h"
#include "tcp_ip.h"
#include "fpga_stepper.h"
#include "power_step.h"
#include "motor_cfg.h"
#include "motor.h"
#include "cleaf.h"

#define CURRENT_HOLD	10


//--- prototypes --------------------------------------------
static void _test_motor_test  (int motor, int steps);

//--- test_init --------------------------------------
void test_init(void)
{
}

//--- test_main ------------------------------------------------------------------
void test_main(int ticks, int menu)
{
	motor_main(ticks, menu);
}

//--- test_display_status ---------------------------------------------------------
void test_display_status(void)
{
}

//--- test_menu --------------------------------------------------
int test_menu(void)
{
	char str[MAX_PATH];
	int synth=FALSE;
	static int cnt=0;
	int i;
	int pos=10000;

	test_display_status();
	
	term_printf("MENU TEST -------------------------\n");
	term_printf("o: toggle output <no>\n");
	term_printf("s: STOP\n");
	term_printf("r<n>: reset motor<n>\n");	
	term_printf("m<n><steps>: move Motor<n> by <steps>\n");	
	term_printf("x: exit\n");
	term_printf(">");
	term_flush();

	if (term_get_str(str, sizeof(str)))
	{
		switch (str[0])
		{
		case 'o': Fpga.par->output ^= (1<<atoi(&str[1]));							break;
		case 's': cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_STOP,			NULL);	break;
		case 'r': if (str[1]==0) for (i=0; i<MOTOR_CNT; i++) motor_reset(i);
				  else           motor_reset(atoi(&str[1])); 
				  break;
		case 'm': _test_motor_test(str[1]-'0', atoi(&str[2]));break;			
		case 'x': return FALSE;
		}
	}
	return TRUE;
}

//--- _test_motor_test ---------------------------------
static void _test_motor_test(int motorNo, int steps)
{
	int motors = 1<<motorNo;
	SMovePar par;
	int i;

	par.speed		= 1000;
	par.accel		= 1000;
	par.current		= 300.0;
	par.stop_mux	= 0;
	par.dis_mux_in	= 0;
	par.stop_in		= ESTOP_UNUSED;
	par.stop_level	= 0;
	par.estop_in    = ESTOP_UNUSED;
	par.estop_level = 0;
	par.checkEncoder= FALSE;
	RX_TestTableStatus.info.moving = TRUE;
	
	motors_config(motors,  CURRENT_HOLD, 0.0, 0.0);
	motors_move_by_step(motors, &par, steps, FALSE);			
}