// ****************************************************************************
//
//	DIGITAL PRINTING - test_table.c
//
//
// ****************************************************************************
//
//	Copyright 2016 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

//--- includes ----------------------------------------------
#include "rx_error.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "rx_term.h"
#include "args.h"
#include "slide.h"
#include "tcp_ip.h"
#include "motor.h"
#include "motor_cfg.h"
#include "fpga_stepper.h"
#include "test_table.h"

//--- defines ----------------------------------------------
#define POSY_LOAD		  1000
#define POSY_PRINT		250000
#define POSY_TABLE		145000
#define POSY_CAP		145000
#define POSZ_CAP		100000 // Maximal position
#define POSY_PURGE		190000
#define POSY_ADJUST		260000

#define CURING_SPEED	3
#define CURRENT_HOLD	100

//--- static variables ---------------------------------------
static RX_SOCKET _MainSocket = INVALID_SOCKET;
static SMovePar	_ParY_ref;
static SMovePar	_ParY_print;
static SMovePar	_ParZ_up;
static SMovePar	_ParZ_down;
static int		_CmdRunning;
static int		_CmdWaiting;
static int		_ScanCnt;
static int		_ScanStep;
static int		_ScanStop;
static int		_CureCnt;
static int		_Printed;
static STestTableScanPar _ScanPar;

//--- prototypes ---------------------------------------------
static void _scan_state_machine(int menu);
static void _abort(void);
static void _stop(void);
static void _start_ref(void);
static void _start_ref1(void);
static void _start_ref2(void);
static void _start_ref3(void);
static void _move_table(void);
static void _move_load_pos(void);
static void _move_cap_pos(void);
static void _move_cap_up(void);
static void _move_purge(int head);
static void _move_adjust(int head);
static void _vacuum(void);
static void _scan_start(void);
static void _scan_left(void);
static void _scan_right(void);
static void _scan(void);
static int  _wait_move_done(int timeout);

static void _tt_motor_z_test(int steps);
static void _tt_motor_y_test(int steps);

static int _ref_started(int cmd);


//--- tt_init -----------------------------
void tt_init(void)
{
	slide_init();

	motor_config(MOTOR_Y_LEFT,  CURRENT_HOLD, 0.0, 0.0);
	motor_config(MOTOR_Y_RIGHT, CURRENT_HOLD, 0.0, 0.0);
	motor_config(MOTOR_Z,	    0,            0.0, 0.0);	

	//--- movment parameters ----------------
	_ParY_ref.speed			= 2000;
	_ParY_ref.accel			= 1000;
	_ParY_ref.current		= 200;
	_ParY_ref.stop_mux		= 0; // MOTOR_Y_BITS;
	_ParY_ref.dis_mux_in	= 0;
	_ParY_ref.stop_in		= ESTOP_UNUSED;
	_ParY_ref.stop_level	= 0;
	_ParY_ref.estop_in		= ESTOP_UNUSED;
	_ParY_ref.estop_level	= 0;
	_ParY_ref.checkEncoder  = TRUE;

	_ParY_print.speed		= 32000;
	_ParY_print.accel		= 35000;
	_ParY_print.current		= 500;
	_ParY_print.stop_mux	= MOTOR_Y_BITS;
	_ParY_print.dis_mux_in	= 0;
	_ParY_print.stop_in		= ESTOP_UNUSED;
	_ParY_print.stop_level	= 0;
	_ParY_print.estop_in	= ESTOP_UNUSED;
	_ParY_print.estop_level	= 0;
	_ParY_print.checkEncoder= TRUE;
	
	_ParZ_down.speed		= 5000;
	_ParZ_down.accel		= 2000;
	_ParZ_down.current		= 100.0;
	_ParZ_down.stop_in      = ESTOP_UNUSED;
	_ParZ_down.stop_level   = 0;
	_ParZ_down.estop_in     = 1;
	_ParZ_down.estop_level  = 1;
	_ParZ_down.checkEncoder = TRUE;

	_ParZ_up.speed			= 5000;
	_ParZ_up.accel			= 2000;
	_ParZ_up.current		= 50.0;
	_ParZ_up.stop_in		= ESTOP_UNUSED;
	_ParZ_up.stop_level		= 0;
	_ParZ_up.estop_in		= ESTOP_UNUSED;
	_ParZ_up.estop_level	= 0;
	_ParZ_up.checkEncoder	= TRUE;
}

//--- tt_end ------------------------------
void tt_end(void)
{
	slide_end();
	motor_end();
}

//--- tt_menu --------------------------------------------------
int tt_menu(void)
{
	char str[MAX_PATH];
	int synth=FALSE;
	static int cnt=0;

	if (RX_StepperCfg.printerType!=printer_test_table) return TRUE;
	
	term_printf("MENU Test Table -------------------------\n");
	if (slide_ref_done())	term_printf("r: move right\n");
	else					term_printf("r: reset motors\n");
	term_printf("L: move slide left              R: move slide right\n");
	term_printf("a: adjust <headno>              o: set output <no>\n");
	term_printf("l: move to load pos             y, z: move by <steps>\n");
	term_printf("t: move to table                v: set speed\n");
	term_printf("c: move to cap                  0: reference\n");
	term_printf("s: stop slide                   1: Scan\n");
	term_printf("x: exit\n");
	term_printf(">");
	term_flush();

	if (term_get_str(str, sizeof(str)))
	{
		switch (str[0])
		{
		case 'a':	slide_move_adjust(atoi(&str[1]));	break;
		case 'r':	motor_reset(0);
					motor_reset(1);
					break;
		case '0':	tt_start_cmd(CMD_TT_START_REF); break;
		case '1':	{
						STestTableScanPar par;
						memset(&par, 0, sizeof(par));
						par.scanMode	= PQ_SCAN_STD;
						par.scanCnt		= 5;
						par.speed		= 1;
						par.yStep		= 20000;
						tt_set_scan_par(&par);
						tt_start_cmd(CMD_TT_SCAN);
					}
					break;
			
		case 'L':	slide_scan_left();			    break;
		case 'R':	slide_scan_right();			    break;
		case 'c':	tt_start_cmd(CMD_TT_MOVE_CAP);	break;			
		case 'l':	tt_start_cmd(CMD_TT_MOVE_LOAD);	break;
		case 's':	tt_start_cmd(CMD_TT_STOP);		break;
//		case 't':	slide_move_table();				break;
//		case 't':	motor_trace_move(0);			break;
		case 't':	tt_start_cmd(CMD_TT_MOVE_TABLE);break;
		case 'v':	slide_set_speed(atoi(&str[1]));	break;
		case 'o':	tt_output(atoi(&str[1]));		break;
		case 'y':	_tt_motor_y_test(atoi(&str[1]));break;
		case 'z':	_tt_motor_z_test(atoi(&str[1]));break;
		case 'x': return FALSE;
		}
	}
	return TRUE;
}

//--- tt_handle_ctrl_msg -----------------------------------------
int tt_handle_ctrl_msg(RX_SOCKET socket, int msgId, void *pdata)
{
//	if (msgId!=CMD_TT_STATUS) 
//		RX_StepperStatus.err=0;

	_MainSocket = socket;
	switch(msgId)
	{
	case CMD_TT_STATUS:		sok_send_2(&socket, REP_TT_STATUS, sizeof(RX_StepperStatus), &RX_StepperStatus);	break;
	case CMD_TT_SCAN:		tt_set_scan_par		(pdata);
							tt_start_cmd		(msgId);
							break;
		
	default:				tt_start_cmd(msgId);
							break;
	}
	return REPLY_OK;
}

//--- _wait_move_done ------------------------
static int  _wait_move_done(int timeout)
{
	while (!motor_move_done(MOTOR_Y_LEFT) || !motor_move_done(MOTOR_Y_RIGHT))
	{
		timeout -= 100;
		if (timeout<=0) return REPLY_ERROR;
		rx_sleep(100);				
	}
	return REPLY_OK;
}

//--- tt_main -----------------------------
void tt_main(int ticks, int menu)
{
	if (RX_StepperCfg.printerType!=printer_test_table) return;

	slide_main(ticks, menu);

	RX_StepperStatus.posX = slide_get_pos();
	RX_StepperStatus.posY = motor_get_step(MOTOR_Y_LEFT);
	RX_StepperStatus.posZ = motor_get_step(MOTOR_Z);

	RX_StepperStatus.info.cover_open = FALSE; // !(Fpga.stat->input & TT_COVER_CLOSED_IN);

	if (!arg_simu)
	{
		if (slide_error())				ErrorFlag(ERR_ABORT, &RX_StepperStatus.err, TT_ERR_SLIDE,           0, "Slide ERROR.");		
		if (motor_error(MOTOR_Y_LEFT))  ErrorFlag(ERR_ABORT, &RX_StepperStatus.err, TT_ERR_STEPPER_Y_LEFT,  0, "Table Stepper Motor left  ERROR.");	
		if (motor_error(MOTOR_Y_RIGHT)) ErrorFlag(ERR_ABORT, &RX_StepperStatus.err, TT_ERR_STEPPER_Y_RIGHT, 0, "Table Stepper Motor right ERROR.");	
//		if (motor_error(MOTOR_Z)) ErrorFlag(ERR_ABORT, &RX_StepperStatus.err, TT_ERR_STEPPER_Z,		  0, "Table Stepper Motor capping ERROR.");			
	}
		
	if (menu)
	{
		term_printf("Test Table ---------------------------------\n");
		term_printf("CmdRunning:    0x%08x\n", _CmdRunning);
		term_printf("CmdWaiting:    0x%08x\n", _CmdWaiting);
		term_printf("ScanStep:      0x%08x\n", _ScanStep);		
		term_printf("Error:         0x%08x\n", RX_StepperStatus.err);
		term_printf("MoveDone:      left=%d   right=%d  slide=%d\n", motor_move_done(MOTOR_Y_LEFT), motor_move_done(MOTOR_Y_RIGHT), slide_move_done());
		
		term_printf("\n");
	}
	
	switch(_CmdRunning)
	{
	case CMD_TT_START_REF:	if (motor_move_done(MOTOR_Z))
							{
								motor_reset(MOTOR_Z);
								_start_ref1();
							}
							break;
		
	case CMD_TT_START_REF+101:
							if (motor_move_done(MOTOR_Y_LEFT) && motor_move_done(MOTOR_Y_RIGHT) && slide_ref_done())
							{
								_start_ref2();
							}
							break;

	case CMD_TT_START_REF+102:
							if (motor_move_done(MOTOR_Y_LEFT) && motor_move_done(MOTOR_Y_RIGHT) && slide_ref_done())
							{
								motor_reset(MOTOR_Y_LEFT);
								motor_reset(MOTOR_Y_RIGHT);
								_start_ref3();
							}
							break;
		
	case CMD_TT_START_REF+103:  
							if (motor_move_done(MOTOR_Y_LEFT) && motor_move_done(MOTOR_Y_RIGHT) && slide_ref_done())
							{
								RX_StepperStatus.info.ref_done = TRUE;
								RX_StepperStatus.info.moving   = FALSE;
								_CmdRunning = 0;
								if (_CmdWaiting)
								{
									tt_start_cmd(_CmdWaiting);
									_CmdWaiting = 0;
								}
							}
							break;
		
	case CMD_TT_MOVE_CAP:	if (motor_move_done(MOTOR_Y_LEFT) && motor_move_done(MOTOR_Y_RIGHT) && slide_move_done())
							{
								_move_cap_up();
							}
							break;
		
	case CMD_TT_MOVE_CAP+110: if (motor_move_done(MOTOR_Z))
							{								
								RX_StepperStatus.info.moving   = FALSE;
								_CmdRunning = 0;								
								if (_CmdWaiting)
								{
									tt_start_cmd(_CmdWaiting);
									_CmdWaiting = 0;
								}
							}
							break;
		
	case CMD_TT_MOVE_TABLE:
	case CMD_TT_MOVE_LOAD:
	case CMD_TT_MOVE_PURGE:
	case CMD_TT_MOVE_ADJUST:
							if (motor_move_done(MOTOR_Y_LEFT) && motor_move_done(MOTOR_Y_RIGHT) && slide_move_done())
							{
								RX_StepperStatus.info.moving = FALSE;									
								_CmdRunning = 0;
								Fpga.par->output &= ~TT_VACUUM_OUT;
								if (!RX_StepperStatus.info.ref_done)
									_start_ref();
							}
							break;

	case CMD_TT_SCAN:		if (RX_StepperStatus.info.cover_open ) ErrorFlag(ERR_ABORT, &RX_StepperStatus.err, TT_ERR_COVER_OPEN, 0, "Cover open.");	
							if (menu) _scan_state_machine(menu); 
							break;
		
	case CMD_TT_SCAN_RIGHT:	
	case CMD_TT_SCAN_LEFT:	if (slide_move_done()) _CmdRunning = 0; 
							break;
	}
}

//--- tt_start_cmd ----------------------
int  tt_start_cmd(int cmd)
{	
	switch(cmd)
	{
	case CMD_TT_ABORT:				_abort();			return REPLY_OK;
	case CMD_TT_STOP:				_stop();			return REPLY_OK;
	case CMD_TT_START_REF:			if (!_CmdRunning) {RX_StepperStatus.err=0; _start_ref();	  }	return REPLY_OK;
	case CMD_TT_MOVE_TABLE:			if (!_CmdRunning) {RX_StepperStatus.err=0; _move_table();	  } return REPLY_OK;
	case CMD_TT_VACUUM:				_vacuum();														return REPLY_OK;
	case CMD_TT_SCAN:				if (!_CmdRunning) {RX_StepperStatus.err=0; _scan();		  }	return REPLY_OK;
	case CMD_TT_MOVE_LOAD:			if (!_CmdRunning) {RX_StepperStatus.err=0; _move_load_pos();}	return REPLY_OK;
	case CMD_TT_MOVE_CAP:			if (!_CmdRunning) {RX_StepperStatus.err=0; _move_cap_pos(); }	return REPLY_OK;
	case CMD_TT_MOVE_PURGE:		    if (!_CmdRunning) {RX_StepperStatus.err=0; _move_purge(0);  }	return REPLY_OK;
	case CMD_TT_MOVE_ADJUST:		if (!_CmdRunning) {RX_StepperStatus.err=0; _move_adjust(0); }	return REPLY_OK;
	case CMD_TT_SCAN_RIGHT:			if (!_CmdRunning) _scan_right();								return REPLY_OK;
	case CMD_TT_SCAN_LEFT:			if (!_CmdRunning) _scan_left();									return REPLY_OK;
	}	
	return REPLY_ERROR;
}

//--- tt_ref_done --------------------------
int  tt_ref_done(void)
{
	return RX_StepperStatus.info.ref_done;
}

//--- _abort -----------------------------------
static void _abort(void)
{
	motors_stop(MOTOR_Y_BITS);
	_CureCnt  = 1000;
	_ScanStop = TRUE;
}

//--- _stop -----------------------------
static void _stop(void)
{
	slide_stop();
	motors_stop(MOTOR_Y_BITS);
	_CmdRunning = 0;
	_CmdWaiting = 0;
}

//--- _ref_started -------------------------------------------
static int _ref_started(int cmd)
{
	// start reference if needed -------------------------
	if (arg_simu) 
	{
		RX_StepperStatus.info.ref_done=TRUE;
		return FALSE;
	}
	
	if (!RX_StepperStatus.info.ref_done)
	{
		_CmdWaiting = cmd;
		_start_ref();
		return TRUE;
	}
	if (!(Fpga.stat->input & TT_CAP_DOWN_IN))
	{
		_CmdRunning = CMD_TT_MOVE_CAP+110;
		_CmdWaiting = cmd;
		if (motor_error(MOTOR_Z)) motor_reset(MOTOR_Z);
		motor_move_by_step	(MOTOR_Z,  &_ParZ_down, -100000);
		motors_start(MOTOR_Z_BITS, FALSE);
		return TRUE;
	}
	return FALSE;
}

//--- _start_ref -------------------------
static void _start_ref(void)
{
	if (arg_simu) 
	{
		RX_StepperStatus.info.ref_done=TRUE;
		return;
	}
	_CmdRunning = CMD_TT_START_REF;
	RX_StepperStatus.info.moving=TRUE;
	
	motor_reset			(MOTOR_Z);
	motors_move_by_step	(MOTOR_Z_BITS,  &_ParZ_down, -100000, TRUE);
}

//--- _start_ref1 -------------------------
static void _start_ref1(void)
{
	_CmdRunning = CMD_TT_START_REF+101;
	
	slide_start_ref();
	motor_reset			(MOTOR_Y_LEFT);
	motor_reset			(MOTOR_Y_RIGHT);

	if (Fpga.stat->input & (TT_END_LEFT_IN | TT_END_RIGHT_IN))
	{
		//--- move off sensors ---
		motors_move_by_step	(MOTOR_Y_BITS,  &_ParY_ref, 3000, TRUE);

		//--- wait moved off sensors -----------------
		_wait_move_done(4000);
	}
	
	{
		SMovePar parLeft;
		parLeft.speed			= _ParY_print.speed;
		parLeft.accel			= _ParY_print.accel;
		parLeft.current			= _ParY_print.current;
		parLeft.stop_mux		= 0;
		parLeft.dis_mux_in		= 0;
		parLeft.stop_in			= ESTOP_UNUSED;
		parLeft.stop_level		= 1;
		parLeft.estop_in		= 10;
		parLeft.estop_level		= 1;
		parLeft.checkEncoder	= TRUE;
		
		SMovePar parRight;
		parRight.speed			= _ParY_print.speed;
		parRight.accel			= _ParY_print.accel;
		parRight.current		= _ParY_print.current;
		parRight.stop_mux		= 0;
		parRight.dis_mux_in		= 0;
		parRight.stop_in		= ESTOP_UNUSED;
		parRight.stop_level		= 0;
		parRight.estop_in		= 11;
		parRight.estop_level	= 1;
		parRight.checkEncoder	= TRUE;

		motor_reset			(MOTOR_Y_LEFT);
		motor_reset			(MOTOR_Y_RIGHT);

		motors_move_by_step	(MOTOR_Y_BITS,  &parLeft,  -500000, FALSE);
	}
}

//--- _start_ref2 -------------------------
static void _start_ref2(void)
{
	_CmdRunning = CMD_TT_START_REF+102;
	motor_reset			(MOTOR_Y_LEFT);
	motor_reset			(MOTOR_Y_RIGHT);

	if (Fpga.stat->input & (TT_END_LEFT_IN | TT_END_RIGHT_IN))
	{
		//--- move off sensors ---
		SMovePar parLeft;
		parLeft.speed		= _ParY_ref.speed;
		parLeft.accel		= _ParY_ref.accel;
		parLeft.current		= _ParY_ref.current;
		parLeft.stop_mux	= 0;
		parLeft.dis_mux_in	= 0;
		parLeft.stop_in		= ESTOP_UNUSED;
		parLeft.stop_level	= 0;
		parLeft.estop_in	= 10;
		parLeft.estop_level = 0;
		parLeft.checkEncoder= TRUE;

		SMovePar parRight;
		parRight.speed		= _ParY_ref.speed;
		parRight.accel		= _ParY_ref.accel;
		parRight.current	= _ParY_ref.current;
		parRight.stop_mux	= 0;
		parRight.dis_mux_in = 0;
		parRight.stop_in	= ESTOP_UNUSED;
		parRight.stop_level = 0;
		parRight.estop_in	= 11;
		parRight.estop_level = 0;
		parRight.checkEncoder = TRUE;

		motors_move_by_step	(MOTOR_Y_BITS,  &parLeft,  3000, FALSE);
	}
}

//--- _start_ref3 -------------------------
static void _start_ref3(void)
{
	_CmdRunning = CMD_TT_START_REF+103;
	RX_StepperStatus.info.moving = TRUE;

	slide_scan_left();
	motors_move_to_step	(MOTOR_Y_BITS,  &_ParY_print, POSY_LOAD);
}

//--- _move_table -----------------------
static void _move_table(void)
{
	if (_ref_started(CMD_TT_MOVE_TABLE)) return;
	
	_CmdRunning = CMD_TT_MOVE_TABLE;
	RX_StepperStatus.info.moving = TRUE;

	slide_move_table();
	motors_move_to_step (MOTOR_Y_BITS,  &_ParY_print, POSY_TABLE);
}

//--- _move_load_pos -----------------------
static void _move_load_pos(void)
{	
	if (_ref_started(CMD_TT_MOVE_LOAD)) return;

	_CmdRunning = CMD_TT_MOVE_LOAD;
	RX_StepperStatus.info.moving=TRUE;

	slide_scan_left();
	motors_move_to_step	(MOTOR_Y_BITS,  &_ParY_print, POSY_LOAD);
}

//--- _move_cap_pos -----------------------
static void _move_cap_pos(void)
{	
	if (_ref_started(CMD_TT_MOVE_CAP)) return;

	_CmdRunning = CMD_TT_MOVE_CAP;
	RX_StepperStatus.info.moving=TRUE;

	slide_scan_left();
	motors_move_to_step	(MOTOR_Y_BITS,  &_ParY_print, POSY_CAP+RX_StepperCfg.cap_pos/1000);
}

//--- _move_cap_up --------------------------------------------------
static void _move_cap_up(void)
{
	_CmdRunning = CMD_TT_MOVE_CAP+110;
	motors_reset(MOTOR_Z_BITS);
	motors_move_to_step	(MOTOR_Z_BITS, &_ParZ_up, POSZ_CAP);
}

//--- _scan_left ---------------------------------------
static void _scan_left(void)
{
	if (_ref_started(CMD_TT_SCAN_LEFT)) return;

	_CmdRunning = CMD_TT_SCAN_LEFT;
	slide_scan_left();			
}

//--- _scan_right ---------------------------------------
static void _scan_right(void)
{
	if (_ref_started(CMD_TT_SCAN_RIGHT)) return;

	_CmdRunning = CMD_TT_SCAN_RIGHT;
	slide_scan_right();						
}

//--- _move_purge ---------------------
static void _move_purge(int head)
{
	if (_ref_started(CMD_TT_MOVE_PURGE)) return;

	_CmdRunning = CMD_TT_MOVE_PURGE;
	RX_StepperStatus.info.moving=TRUE;

	slide_scan_left();
	motors_move_to_step	(MOTOR_Y_BITS,  &_ParY_print, POSY_PURGE);
}

//--- _move_adjust ---------------------
static void _move_adjust(int head)
{
	if (_ref_started(CMD_TT_MOVE_ADJUST)) return;

	_CmdRunning = CMD_TT_MOVE_ADJUST;
	RX_StepperStatus.info.moving=TRUE;

	slide_scan_left();
	motors_move_to_step	(MOTOR_Y_BITS,  &_ParY_print, POSY_ADJUST);
}

//--- _vacuum ---------------------------
static void _vacuum(void)
{	
	if (Fpga.par->output&TT_VACUUM_OUT) Fpga.par->output &= ~TT_VACUUM_OUT;
	else                                Fpga.par->output |=  TT_VACUUM_OUT;
}

//--- tt_set_scan_par -----------------------------------
void tt_set_scan_par(STestTableScanPar *par)
{
	memcpy(&_ScanPar, par, sizeof(_ScanPar));
}

//--- _scan -----------------------------------
static void _scan(void)
{
	if (_ref_started(CMD_TT_SCAN)) return;
	
	if (_CmdRunning) return;
	_CmdRunning = CMD_TT_SCAN;
	RX_StepperStatus.err = 0;
	_CureCnt  = 0;
	_scan_start();
}

//--- _scan_start ------------------------------------------------------
static void _scan_start(void)
{
	//--- head is in park - pos -------------
	_ScanStep = 1;
	_ScanCnt  = 0;
	_ScanStop = FALSE;
	RX_StepperStatus.info.printing = _CureCnt==0;
	RX_StepperStatus.info.curing   = _CureCnt>0;	
	slide_set_speed(_ScanPar.speed);
	if (_ScanPar.scanMode==PQ_SCAN_RTL) slide_scan_right();
	else							    slide_scan_left();
	Fpga.par->output |= TT_VACUUM_OUT;	
	motor_move_to_step	(MOTOR_Y_LEFT,  &_ParY_print, POSY_PRINT);
	motor_move_to_step	(MOTOR_Y_RIGHT, &_ParY_print, POSY_PRINT+_ScanPar.offsetAngle);
	motors_start(MOTOR_Y_BITS, TRUE);
}

//--- _scan_state_machine -------------------------
static void _scan_state_machine(int menu)
{		
	//----- wait until motors stopped ---------------
	if ( motor_move_done(MOTOR_Y_LEFT) && motor_move_done(MOTOR_Y_RIGHT) && slide_move_done())
	{ 
		if (_Printed)
		{
			_ScanCnt++;
			_Printed=FALSE;
		}

		if (motor_error(MOTOR_Y_LEFT) || motor_error(MOTOR_Y_RIGHT))
		{
			rx_sleep(100);
			_move_load_pos();
			RX_StepperStatus.info.ref_done=FALSE;
			_ScanStep = 100;
			return;
		}

		if (_ScanCnt>=_ScanPar.scanCnt && _ScanStep<100)
		{
			_CureCnt++;
			if (_CureCnt < _ScanPar.curingPasses) 
			{
				_ScanPar.scanMode=PQ_SCAN_BIDIR;
				_scan_start();
				slide_set_speed(CURING_SPEED);
				return;
			}
			else  _ScanStop = TRUE;
		}
		
		/*
		if (motor_get_step(MOTOR_Y_LEFT) != motor_get_end_step(MOTOR_Y_LEFT) || motor_get_step(MOTOR_Y_RIGHT) != motor_get_end_step(MOTOR_Y_RIGHT))
		{
			BIT_CLR(RX_StepperStatus.info, TT_INFO_MOVING);
			BIT_CLR(RX_StepperStatus.info, TT_INFO_REF_DONE);
			Error(ERR_ABORT, 0, "Position failed");
			return;			
		}
		*/
		
		if (_ScanStop)
		{
			if (_Printed && slide_get_pos()) slide_scan_left();
			else
			{
				slide_scan_left();
				motors_move_to_step(MOTOR_Y_BITS,  &_ParY_print, POSY_LOAD);
				Fpga.par->output &= ~TT_VACUUM_OUT;
				_ScanStep = 100;				
				_ScanStop = FALSE;
			}
			return;
		}

		switch (_ScanStep)
		{
		case 1:		//--- slide is left -----------
					_Printed = TRUE;
					switch (_ScanPar.scanMode)
					{
					case PQ_SCAN_STD:	slide_scan_right(); _ScanStep=2; break;
					case PQ_SCAN_RTL:	slide_scan_left();	_ScanStep=2; break;
					case PQ_SCAN_BIDIR:	slide_scan_right(); _ScanStep=10; break;
					}
					break;

		case 2:		//--- slide is right (UNI_DIR) ---------------------
					motors_move_by_step	(MOTOR_Y_BITS,  &_ParY_print, -_ScanPar.yStep, TRUE);
					
					if (_ScanPar.scanMode==PQ_SCAN_RTL) 
						slide_scan_right();
					else		   
						slide_scan_left();
					_ScanStep=1;
					break;

		case 10:	//--- slide is right (BI_DIR) ---------------------
					motors_move_by_step	(MOTOR_Y_BITS,  &_ParY_print, -_ScanPar.yStep, TRUE);
					_ScanStep++;
					break;

		case 11:	//--- slide is right, Y-Movement done ----
					_Printed = TRUE;
					slide_scan_left();
					_ScanStep++;
					break;

		case 12:	//--- slide is left (BI_DIR) ---------------------
					motors_move_by_step	(MOTOR_Y_BITS,  &_ParY_print, -_ScanPar.yStep, TRUE);
					_ScanStep=1;
					break;

		case 100: 	//--- done ------------------
					_CmdRunning = 0;
					RX_StepperStatus.info.printing = FALSE;
					RX_StepperStatus.info.curing   = FALSE;	

					RX_StepperStatus.info.moving = FALSE;
					if (_MainSocket!=INVALID_SOCKET) sok_send_2(&_MainSocket, CMD_PRINT_ABORT, 0, NULL);
					break;
		}
	}
}

//--- tt_output -------------------------------------------------
void tt_output(int no)
{
	Fpga.par->output ^= (1<<no);
}

//--- _tt_motor_y_test -------------------------------
static void _tt_motor_y_test(int steps)
{
	int motor = 0x03;
	SMovePar par;
	int i;

	par.speed		 = 5000;
	par.accel		 = 2000;
	par.current		 = 318.0;
	par.stop_mux	 = motor;
	par.dis_mux_in	 = 0;
	par.stop_in      = ESTOP_UNUSED;
	par.stop_level   = 0;
	par.estop_in     = ESTOP_UNUSED;
	par.estop_level  = 0;
	par.checkEncoder = TRUE;
	
	for (i=0; i<2; i++) motor_config(i, CURRENT_HOLD, 0.0, 0.0);
	motors_move_by_step(motor, &par, steps, TRUE);
}

//--- _tt_motor_z_test -------------------------------
static void _tt_motor_z_test(int steps)
{
	SMovePar par;

	par.speed		 = 5000;
	par.accel		 = 2000;
	par.current		 = 100.0;
	par.stop_in      = ESTOP_UNUSED;
	par.stop_level	 = 1;
	par.estop_in     = 0;
	par.estop_level	 = 1;
	par.checkEncoder = TRUE;
	motor_config(MOTOR_Z,  CURRENT_HOLD, 0.0, 0.0);
	motor_move_by_step(MOTOR_Z, &par, steps);
}
