// ****************************************************************************
//
//	step_cleaf.c		Stepper Control for CLEAF project
//
// ****************************************************************************
//
//	Copyright 2016 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#include "rx_error.h"
#include "rx_def.h"
#include "rx_error.h"
#include "rx_sok.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "tcp_ip.h"
#include "network.h"
#include "ctrl_msg.h"
#include "print_ctrl.h"
#include "plc_ctrl.h"
#include "enc_ctrl.h"
#include "gui_svr.h"
#include "args.h"
#include "cleaf_orders.h"
#include "step_cleaf.h"
#include "step_ctrl.h"

//--- defines -----------------------------------------------------------------------------------------------------------------------------

#define trace TRUE // FALSE

#define STEPPER_CNT		4

//--- dimensions -----------------------
static int belt_ref_height = 100000; // absolut from mech ref to belt
//static int robot_ref_height = 8500; // absolut from mech ref to belt
static int robot_ref_height[4] = { 7500 }; // absolut from mech ref to belt
static int printer_height = 1000; // rel from belt_ref
static int wipe_height = 1000; // rel from belt_ref-robot_ref
static int capping_height = -1000; // rel from belt_ref-robot_ref

static int capping_pos = 630000; 

//--- printhead ------------------------
#define POS_HEADSCREW_DELTA	16400 // delta between the two screws on the same head in mm
#define POS_HEAD_DELTA		43446.2 // 43140 // 48924 // 48854+500/7 about +70 // 43140 // statt 125 85 -> delta 40mm von screw1 to screw 7
//#define POS_CAP2SCREW_DELTA	25628

//--- position lift --------------------
#define LIFT_CAP_DIST				1000 // Last 1mm slow capping
#define LIFT_POS_PLACE_PRINTHEADS	(belt_ref_height-robot_ref_height[no]+capping_height+LIFT_CAP_DIST) // (belt_ref_height-capping_height)  // DELAT VALUE //LIFT_POS_START_WIPE // capping position
#define LIFT_POS_PRINTING			printer_height // DELAT VALUE // RX_StepperCfg.print_height
#define LIFT_POS_START_WIPE			(belt_ref_height-robot_ref_height[no]+wipe_height) // DELAT VALUE // RX_StepperCfg.wipe_height
#define LIFT_POS_START_WIPE_VAC		(belt_ref_height-robot_ref_height[no]+wipe_height-1500) // DELAT VALUE // RX_StepperCfg.wipe_height
#define LIFT_POS_STORED				(belt_ref_height-1000) // robot_ref_height // LIFT_POS_PLACE_PRINTHEADS // 1000  // DELAT VALUE // "UP" position above belt
//#define LIFT_POS_UP				1000
#define LIFT_POS_PURGE				LIFT_POS_START_WIPE // LIFT_POS_PLACE_PRINTHEADS // 1000  // DELAT VALUE // - 3000 um Purge height
#define LIFT_POS_SCREW				(belt_ref_height-robot_ref_height[no]) // 0 // wipe_height // -6000 // -5000 // -2000 // DELAT VALUE //-7000 // maximal value=> -7100   // DELAT VALUE // TODO: adjust distance for screwing from wiping pos (+1000)
//#define LIFT_CAP_SECURITY_DELTA		1000

// one head along long edge 43446.2 um , long edge with angle 43425.1 um
// one head along short edge 17587.870 , short edge with angle 17755.0 um
//--- position cleaning ------------------
#define CLN_POS_PRINTHEADS		capping_pos //780000 // max. 782000 
#define CLN_POS_STORED			0 // 23000 // Ref 0
#define CLN_DIST_WIPE_ONE_HEAD  (POS_HEAD_DELTA+1000) // 49350 // 48150+1000+200 // 53500 // 43140 // -> 4cm bis 9.7   41.5 bis 47.2 // 43140+5mm=48150 // ziel 4-44cm // 43 ist 40 soll
#define CLN_DIST_WIPEBLADE_FLEX  2000 // 20000 // 8000 // ca 10% of CLN_DIST_WIPE_ONE_HEAD plus 5mm
#define CLN_POS_START_WIPE		600000 // 755000 // 710000 // 700000 // 509000 // + CLN_DIST_WIPEBLADE_FLEX/2 // CLN_POS_PRINTHEADS //  - 20000 // Wipe start // TODO: adjust distance form capping to wiping (-1000)
#define CLN_POS_END_WIPE        195000 //(81000 - CLN_DIST_WIPEBLADE_FLEX)
#define CLN_POS_END_WET_WIPE    (CLN_POS_END_WIPE+25000) //(81000 - CLN_DIST_WIPEBLADE_FLEX)
#define POS_SCREW_0				204000 // 172000 // 85500 // 85300 // 84800 // 85800 // 83800 // 28800+55000 // - POS_CAP2SCREW_DELTA // 28800 + 23000
//#define POS_SCREW_BAR			173000 // Bar shift position
#define CLN_SCREW_SEARCH_DELTA	7500 // 10000
#define CLN_SCREW_TO_SENSOR		(12000+1000) // physical distancec screw center to detector center
//#define CLN_DET_SCREW_TO_SENSOR		(17500-1000-1000+800+500+100+100+100-150+1000+500+400) // detector detection edge to screw center

//--- Errors ----------------------------------------
#define ERR_0	0x00000001
#define ERR_1	0x00000002
#define ERR_2	0x00000004
#define ERR_3	0x00000008
#define ERR_4	0x00000010
#define ERR_5	0x00000020
#define ERR_6	0x00000040
#define ERR_7	0x00000080
#define ERR_8	0x00000100
#define ERR_9	0x00000200
#define ERR_10	0x00000400
#define ERR_11	0x00000800
#define ERR_12	0x00001000
#define ERR_13	0x00002000
#define ERR_14	0x00004000
#define ERR_15	0x00008000

static INT32 _ErrorFlags[STEPPER_CNT];

//--- screwing data ------------------
#define SCREW_JAW_REV_PER_UM	1.0/5.0  // 1/5 rev per um
#define SCREW_STITCH_REV_PER_UM	1.0/11.0  // 1/11 rev per um

typedef struct SPosItem
{
	int pos_cln;
	int pos_lift;
} SPosItem;
typedef struct SPosName
{
	SPosItem reset;
	SPosItem printing;
	SPosItem capping;
	SPosItem wiping;
	SPosItem wiping_vac;
	SPosItem screw0;
	SPosItem purge;
} SPosName;

//--- global  variables -----------------------------------------------------------------------------------------------------------------

static RX_SOCKET		*_step_socket[STEPPER_CNT] = { 0 };
static SStepperStat	_status[STEPPER_CNT];
static SClnStateEnv		_cln_state_env[STEPPER_CNT];
static int				_reset_pos = 100000-7500; //  2000; //  27000; //  7000; // 7000; // 5000; // not negative ! // position to which he goes up after every job // slightly above robot
//static int				_reset_cln = 0; // save position for cleaning
static enum cln_state_code _capCurrentState = ST_INIT;
static int				_WipeReady=0;
static int				_printhead_en=FALSE;
static int				_splicing=FALSE;
static int				_enc_disabled=FALSE;
static int				_allow_stepper_move_down = FALSE;	// if drip pans below the heads, disable the move down of the steppers motors

//--- prorotypes -------------------------------------------------
//static void _do_tt_status(SStepperStat *pStatus);
//static void _do_cln_status(SStepperStat *pStatus);
static int  _do_turn_screw(int no, SHeadAdjustment *pHeadAdjust);
static void _check_ref(int no, SClnStateEnv *senv);
static void _stepc_reset_fluid_sys(int no, SClnStateEnv *senv);
static void _stepc_statemachine(int no, SClnStateEnv *_state_env);


//--- stepc_init ---------------------------------------------------
void stepc_init(int no, RX_SOCKET *psocket)
{
	if (no >= 0 && no < STEPPER_CNT)
	{
		_step_socket[no] = psocket;
	}
	memset(_status, 0, sizeof(_status));
}

//--- stepc_error_reset ----------------------------------------------------
void stepc_error_reset(void)
{
	memset(_ErrorFlags, 0, sizeof(_ErrorFlags));
}

//--- stepc_handle_gui_msg------------------------------------------------------------------
int	 stepc_handle_gui_msg(RX_SOCKET socket, UINT32 cmd, void *data, int dataLen)
{
	int no;
	for (no = 0; no < SIZEOF(_step_socket); no++)
	{
		if (_step_socket[no] && *_step_socket[no] != INVALID_SOCKET)
		{
			switch (cmd)
			{
			case CMD_TT_STOP:
			case CMD_TT_START_REF:
			case CMD_TT_MOVE_TABLE:
			case CMD_TT_MOVE_LOAD:
			case CMD_TT_MOVE_CAP:
			case CMD_TT_MOVE_PURGE:
			case CMD_TT_MOVE_ADJUST:
			case CMD_TT_SCAN_RIGHT:
			case CMD_TT_SCAN_LEFT:
			case CMD_TT_VACUUM:
				sok_send_2(_step_socket[no], cmd, 0, NULL);
				break;

			case CMD_TT_SCAN:
				{
								STestTableScanPar par;
								par.speed = 5;
								par.scanCnt = 5;
								par.scanMode = PQ_SCAN_STD;
								par.yStep = 10000;
								sok_send_2(_step_socket[no], CMD_TT_SCAN, sizeof(par), &par);
				}
				break;

				//--- cappping ---------------------------------------------------------	

			case CMD_CAP_STOP:
				_cln_state_env[no].st_cmd = cmd;
				break;

			case CMD_CAP_REFERENCE:
				if (!arg_simuPLC) _cln_state_env[no].st_cmd = cmd;
				if (no == 0) sok_send_2(_step_socket[0], CMD_CLN_DRIP_PANS_EN, 0, NULL);
				break;

			case CMD_CAP_UP_POS:
				if (!arg_simuPLC && RX_StepperStatus.info.DripPans_InfeedDOWN && RX_StepperStatus.info.DripPans_OutfeedDOWN) sok_send_2(_step_socket[no], CMD_CAP_UP_POS, 0, NULL);
				break;

			case CMD_CAP_PRINT_POS:
				if ( RX_StepperStatus.info.DripPans_InfeedDOWN && RX_StepperStatus.info.DripPans_OutfeedDOWN)sok_send_2(_step_socket[no], CMD_CAP_PRINT_POS, sizeof(RX_Config.stepper.print_height), &RX_Config.stepper.print_height);			
				break;

			case CMD_CAP_CAPPING_POS:
				sok_send_2(_step_socket[no], CMD_CAP_CAPPING_POS, 0, NULL);
				break;

				//--- cleaning ---------------------------------------------------------
			case CMD_CLN_WIPE:
				/*if (_cln_state_env[no].capCurrentState == ST_INIT || _cln_state_env[no].capCurrentState == ST_IDLE)
				{
					_cln_state_env[no].st_cmd = cmd;
				}*/
				break;
			
				// Drip pans moves
			case CMD_CLN_DRIP_PANS:
				if (no == 0) sok_send_2(_step_socket[0], CMD_CLN_DRIP_PANS, 0, NULL);
				break;
				
			case CMD_CLN_WIPE_VACUUM:
				if (_cln_state_env[no].capCurrentState == ST_INIT || _cln_state_env[no].capCurrentState == ST_IDLE)
				{
					if (_cln_state_env[no].flag_cap_empty == FALSE && _cln_state_env[no].capCurrentState == ST_IDLE) 
					{
						_cln_state_env[no].st_cmd = cmd;
						//_cln_state_env[no].st_cmd = CMD_CLN_WIPE; // Enable for capping that can fill
					}
					else 
					{
						_cln_state_env[no].st_cmd = cmd;
					}
				}
				break;

			case CMD_CLN_FLUSH_TEST:
				if (_cln_state_env[no].capCurrentState == ST_INIT || _cln_state_env[no].capCurrentState == ST_IDLE)
				{
					if (_cln_state_env[no].flag_cap_empty == FALSE && _cln_state_env[no].capCurrentState == ST_IDLE) 
					{
						_cln_state_env[no].st_cmd = cmd;
					}
					else 
					{
						_cln_state_env[no].st_cmd = cmd;
					}
				}
				break;

			case CMD_CLN_ADJUST:		_do_turn_screw(no, (SHeadAdjustment*)data);
				break;

			}
		}
	}
	return REPLY_OK;
}

//--- stepc_handle_status ----------------------------------------------------------------------
int stepc_handle_status(int no, SStepperStat *pStatus)
{
	int i, cnt;
	int change=FALSE;
	int allowed = FALSE;
	ETestTableInfo info;
	
	if (_status[no].info.splicing	  != pStatus->info.splicing)	 {TrPrintfL(TRUE, "Stepper[%d]: new info.splice=%d",	   no, pStatus->info.splicing); change=TRUE;}
	if (_status[no].info.printhead_en != pStatus->info.printhead_en) {TrPrintfL(TRUE, "Stepper[%d]: new info.printhead_en=%d", no, pStatus->info.printhead_en); change=TRUE;}
	
	memcpy(&_status[no], pStatus, sizeof(_status[no]));

	memset(&info, 0, sizeof(info));
	info.ref_done		= TRUE;
	info.z_in_ref		= TRUE;
	info.headUpInput_0	= TRUE;
	info.headUpInput_1	= TRUE;
	info.DripPans_InfeedDOWN	= TRUE;
	info.DripPans_InfeedUP		= TRUE;
	info.DripPans_OutfeedDOWN	= TRUE;
	info.DripPans_OutfeedUP		= TRUE;
	info.z_in_print		= TRUE;
	info.z_in_cap		= TRUE;
	info.x_in_ref		= TRUE;
	info.printhead_en	= TRUE;
	//	TrPrintf(TRUE, "steptest_handle_status(%d)", no);

	robot_ref_height[0] = 6500;
	robot_ref_height[1] = 7500;
	robot_ref_height[2] = 7500;
	robot_ref_height[3] = 7500;

	// Init position matrix
	printer_height = RX_Config.stepper.print_height; // um // 151000 steps *1000000 /1600000
	wipe_height = RX_Config.stepper.wipe_height; // 20000 ; capping 28000; // um // 45000 steps *1000000 /1600000
	capping_pos = RX_Config.stepper.cap_pos; // um // 69801 steps *1000000 /139581.3
	capping_height = RX_Config.stepper.cap_height;
	belt_ref_height = RX_Config.stepper.ref_height;
	_reset_pos = (belt_ref_height - robot_ref_height[no]);
	//robot_ref_height = RX_Config.stepper.rob_height;

	RX_StepperStatus.posX = 0;
	RX_StepperStatus.posY = 0;
	RX_StepperStatus.posZ = 0;
	
	info.DripPans_InfeedDOWN	= _status[0].info.DripPans_InfeedDOWN;
	info.DripPans_InfeedUP		= _status[0].info.DripPans_InfeedUP;
	info.DripPans_OutfeedDOWN	= _status[0].info.DripPans_OutfeedDOWN;
	info.DripPans_OutfeedUP		= _status[0].info.DripPans_OutfeedUP;
	allowed = info.DripPans_InfeedDOWN && info.DripPans_OutfeedDOWN && !info.DripPans_InfeedUP && !info.DripPans_OutfeedUP;

	for (i = 0, cnt=0; i < STEPPER_CNT; i++)
	{
		if (_step_socket[i] && *_step_socket[i] != INVALID_SOCKET)
		{
			//			TrPrintf(TRUE, "Stepper[%d]: ref_done=%d moving=%d  in_print=%d  up=%d", i, _status[i].info.ref_done, _status[i].info.moving, _status[i].info.z_in_print, _status[i].info.z_in_ref);
			cnt++;
			info.ref_done		&= _status[i].info.ref_done;
			info.headUpInput_0	&= _status[i].info.headUpInput_0;
			info.headUpInput_1	&= _status[i].info.headUpInput_1;
			info.moving			|= _status[i].info.moving;
			info.z_in_ref		&= _status[i].info.z_in_ref;
			info.z_in_print		&= _status[i].info.z_in_print;
			info.z_in_cap		&= _status[i].info.z_in_cap;
			info.x_in_ref		&= _status[i].info.x_in_ref;
			info.printhead_en	&= _status[i].info.printhead_en;
			info.splicing		|= _status[i].info.splicing;
			info.x_in_cap		= TRUE;

			RX_StepperStatus.posX += _status[i].posX;
			RX_StepperStatus.posY += _status[i].posY;
			RX_StepperStatus.posZ += _status[i].posZ;
			
			if (allowed != _allow_stepper_move_down)
			{
				if (allowed) sok_send_2(_step_socket[i], CMD_CAP_ALLOW_MOVE_DOWN, 0, NULL);
				else sok_send_2(_step_socket[i], CMD_CAP_NOT_ALLOW_MOVE_DOWN, 0, NULL);
			}
		}
	}
	_allow_stepper_move_down = allowed;

	if (cnt)
	{
		RX_StepperStatus.posX /= cnt;
		RX_StepperStatus.posY /= cnt;
		RX_StepperStatus.posZ /= cnt;		
	}
	
	if (change) TrPrintfL(TRUE, "Splice changed: _splicing=%d, info.splicing=%d", _splicing, info.splicing);
	
	if (!_splicing && info.splicing)
	{
		enc_enable_printing(FALSE);
		_enc_disabled = TRUE;
	}
	if (info.z_in_print && _enc_disabled)
	{
		enc_enable_printing(TRUE);
		_enc_disabled=FALSE;
		if (plc_is_splicing()) co_roll_end();
	}

	if (_splicing && !info.splicing) 
	{
		TrPrintfL(TRUE, "Splice end");
		if (RX_PrinterStatus.printState==ps_printing) 
		{
			TrPrintfL(TRUE, "stepc_to_print_pos ");
			stepc_to_print_pos();
		}
	}
	_splicing = info.splicing;
	
	if (_step_socket[no] && *_step_socket[no] != INVALID_SOCKET)
	{
		_stepc_statemachine(no, &_cln_state_env[no]);
	}

	//	TrPrintf(TRUE, "STEPPER: ref_done=%d moving=%d  in_print=%d  up=%d", info.ref_done, info.moving, info.z_in_print, info.z_in_ref);

	memcpy(&RX_StepperStatus.info, &info, sizeof(RX_StepperStatus.info));

	_capCurrentState = _cln_state_env[0].capCurrentState;
	RX_StepperStatus.state = _cln_state_env[0].capCurrentState;

	gui_send_msg_2(0, REP_TT_STATUS, sizeof(RX_StepperStatus), &RX_StepperStatus);

	return REPLY_OK;
}

//--- stepc_abort_printing --------------------
int stepc_abort_printing(void)
{
	stepc_handle_gui_msg(INVALID_SOCKET, CMD_CAP_UP_POS, NULL, 0);
	return REPLY_OK;
}

//--- stepc_to_print_pos --------------------------------
int	 stepc_to_print_pos(void)
{
	stepc_handle_gui_msg(INVALID_SOCKET, CMD_CAP_PRINT_POS, NULL, 0);
	return REPLY_OK;
}

//--- stepc_to_up_pos --------------------------------
int	 stepc_to_up_pos(void)
{
	stepc_handle_gui_msg(INVALID_SOCKET, CMD_CAP_UP_POS, NULL, 0);
	return REPLY_OK;
}

//--- stepc_slide_enabled ---------------------
int	 stepc_slide_enabled(int no)
{
	if (_status[no].posZ >= 0)
	{
		return TRUE;
	}
	return FALSE;
}

//--- stepc_gui_enabled ---------------------
int	 stepc_gui_enabled(int no)
{
	if (_cln_state_env[no].capCurrentState == ST_IDLE && (_status[no].posZ >= 0) && _cln_state_env[no].slide_enable)
	{
		return TRUE;
	}
	return FALSE;
}

//--- stepc_to_purge_pos ---------------------
int	 stepc_to_purge_pos(int no, int head)
{
	if (trace) Error(LOG, 0, "fluid_cmd start purge");
	if (_cln_state_env[no].capCurrentState == ST_INIT || _cln_state_env[no].capCurrentState == ST_IDLE)
	{
		_cln_state_env[no].clean_wipe_head_nr = head;
		_cln_state_env[no].slide_enable = FALSE;
		if (trace) Error(LOG, 0, "Flag: SLIDE disable");
		if (trace) Error(LOG, 0, "stepc_to_purge_pos: CMD_CAP_PURGE start");
		_cln_state_env[no].st_cmd = CMD_CAP_PURGE;
		return REPLY_OK;
	}
	return FALSE;
}

//--- stepc_in_purge_pos ---------------
int  stepc_in_purge_pos(int no)
{
	static int _old = -1;
	if ((_cln_state_env[no].flag_purge_done == TRUE) && (plc_in_purge_pos() == TRUE)) // && (abs(_status[no].posZ - _lift_pos) <= 10) && (abs(RX_ClnStatus.posZ - senv->clean_pos) <= 10)
	{
		if (_old != TRUE) { if (trace) Error(LOG, 0, "in purge"); }
		return _old = TRUE;
	}
	return _old = FALSE;
}

//--- stepc_wipe_start --------------------------------------------
int	 stepc_wipe_start(int no, int head)
{
	if (trace) Error(LOG, 0, "fluid_cmd start wipe");
	_cln_state_env[no].clean_wipe_head_nr = head;
	if (_cln_state_env[no].capCurrentState == ST_INIT || _cln_state_env[no].capCurrentState == ST_IDLE)
	{
		_cln_state_env[no].st_cmd = CMD_CLN_WIPE;
		if (trace) Error(LOG, 0, "stepc_wipe_start: CMD_CLN_WIPE start");
		return REPLY_OK;
	}
	return FALSE;
}

//--- stepc_wipe_done --------------------------------------------
int	 stepc_wipe_done(int no)
{
	if ((_cln_state_env[no].flag_wipe_done == TRUE) && (plc_in_wipe_pos() == TRUE)) // && (abs(_status[no].posZ - _lift_pos) <= 10) && (abs(RX_ClnStatus.posZ - senv->clean_pos) <= 10)
	{
		if (trace) Error(LOG, 0, "wipe done");
		return TRUE;
	}
	return FALSE;
}

//--- _stepc_statemachine --------------------------------------------------
static void _stepc_statemachine(int no, SClnStateEnv *senv)
{
	int i;

	// --- Position Matrix ---
	struct SPosName	positions =
	{
		{ CLN_POS_STORED, LIFT_POS_STORED },				//  0: reset
		{ CLN_POS_STORED, LIFT_POS_PRINTING },				//  1: printing
		{ CLN_POS_PRINTHEADS, (LIFT_POS_PLACE_PRINTHEADS) },	//  2: capping
		{ CLN_POS_START_WIPE, LIFT_POS_START_WIPE },		//  3: wiping
		{ CLN_POS_START_WIPE, LIFT_POS_START_WIPE_VAC },	//  4: wiping vacuum blade
		{ POS_SCREW_0, LIFT_POS_SCREW },					//  5: screw0
		{ CLN_POS_START_WIPE, LIFT_POS_PURGE }				//  6: purge
	};

	// --- Stop State Machine ---
	if ((senv->st_cmd == CMD_CAP_STOP) || (senv->st_cmd == CMD_CLN_STOP))
	{
		sok_send_2(_step_socket[no], CMD_CAP_STOP, 0, NULL);
		senv->st_cmd = FALSE;
		senv->st_cmd_stored = FALSE;
		_stepc_reset_fluid_sys(no, senv);
		Error(LOG, 0, "MAIN: State_Command 0x%08x: Manual STOP", senv->st_cmd_stored);
		if (_status[no].info.ref_done == TRUE && _status[no].info.ref_done == TRUE) // do not go to Idle if not once referenced
		{
			senv->capCurrentState = ST_IDLE;
			printf("Stop State Machine and set to Idle\n");
		}
		else
		{
			senv->capCurrentState = ST_INIT;
			printf("Stop State Machine and set to Init\n");
		}
	}

	// --- Reference State Machine ---
	if (senv->st_cmd == CMD_CAP_REFERENCE)
	{
		//sok_send_2(_step_socket[no], CMD_CAP_STOP, 0, NULL);
		senv->capCurrentState = ST_INIT;
		printf("Reference State Machine \n");
	}


	//_capCurrentState = senv->capCurrentState;
	// --- State Machine CLN & LIFT ctrl ---
	// First: must reference lift and cleaningstation (cln) to get to idle
	// Idle has 5 Options: capping: move lift to capping
	//                     printing: move lift to printing
	//                     up: move lift up to reference
	//                     wipe: move lift up, move cleaning blade to position, move down to cleaning height, wipe
	//                     screw: move lift up, move screwer to position, move down to screwing height, screw
	switch (senv->capCurrentState) {

	case ST_INIT:
		if (senv->st_cmd != FALSE)
		{
			//init Values
			senv->screw_0_pos = 0;
			senv->screw_1_pos = 0;
			senv->test_loop_cnt = 0;
			senv->flag_wipe_done = FALSE;
			senv->flag_purge_done = FALSE;
			//if (trace) Error(LOG, 0, "Flag: WIPE DONE disable");
			//if (trace) Error(LOG, 0, "Flag: PURGE DONE disable");
			printf("state %d REFERENCE\n", senv->capCurrentState);
			_stepc_reset_fluid_sys(no, senv);
			senv->cln_move_tgl = _status[no].info.move_tgl;
			//if (((_status[no].info.cln_in_stored == TRUE) && (_status[no].info.cln_in_stored_1 == TRUE)) || (senv->flag_cap_empty == TRUE)) {  // Enable for capping that can fill
			senv->flag_cap_empty = TRUE;
			senv->dry_wipe_done = TRUE;
			//senv->st_cmd = FALSE;
			senv->lift_move_tgl = _status[no].info.move_tgl;
			if (senv->st_cmd == CMD_CAP_REFERENCE)
			{
				senv->st_cmd = FALSE;
				sok_send_2(_step_socket[no], CMD_CAP_REFERENCE, 0, NULL); // ref Lift
				senv->capCurrentState = ST_REF_LIFT;
			}
			//}
			//else
			//			{
			//				int set_flag = 2;
			//				sok_send_2(_step_socket[no], CMD_CLN_EMPTY_CAP, sizeof(set_flag), &set_flag); // Clean Cap valve inside
			//				//senv->st_cmd = CMD_CLN_WIPE;
			//				senv->dry_wipe_done = FALSE;
			//				set_flag = 2;
			//				senv->lift_move_tgl = _status[no].info.move_tgl;
			//				sok_send_2(_step_socket[no], CMD_CAP_EMPTY_CAP, sizeof(set_flag), &set_flag); // Empty Cap
			//				senv->capCurrentState = ST_CAP_CLEAN_REF_IN;
			//			}

		}
		break;

		//	case ST_CAP_CLEAN_REF_IN:
		//		if (senv->lift_move_tgl != _status[no].info.move_tgl)
		//		{
		//			int set_flag = 1;
		//			sok_send_2(_step_socket[no], CMD_CLN_EMPTY_CAP, sizeof(set_flag), &set_flag); // Clean Cap valve outside
		//			set_flag = 1;
		//			senv->lift_move_tgl = _status[no].info.move_tgl;
		//			sok_send_2(_step_socket[no], CMD_CAP_EMPTY_CAP, sizeof(set_flag), &set_flag); // Empty Cap
		//			senv->capCurrentState = ST_CAP_CLEAN_REF;
		//		}
		//		break;
		//
		//	case ST_CAP_CLEAN_REF:
		//		if (senv->lift_move_tgl != _status[no].info.move_tgl)
		//		{
		//			senv->cln_move_tgl = _status[no].info.move_tgl;
		//			sok_send_2(_step_socket[0], CMD_CLN_DRAIN_WASTE, 0, NULL); // Empty Waste
		//			senv->capCurrentState = ST_DRAIN_WASTE_REF;
		//		}
		//		break;
		//
		//	case ST_DRAIN_WASTE_REF:
		//		if (senv->cln_move_tgl != _status[no].info.move_tgl)
		//		{
		//			int set_flag = 3;
		//			sok_send_2(_step_socket[no], CMD_CLN_EMPTY_CAP, sizeof(set_flag), &set_flag); // Clean Cap valve
		//
		//			senv->lift_move_tgl = _status[no].info.move_tgl;
		//			sok_send_2(_step_socket[no], CMD_CAP_REFERENCE, 0, NULL); // ref Lift
		//			senv->capCurrentState = ST_REF_LIFT;
		//		}
		//		break;

	case ST_REF_LIFT:
		if (senv->lift_move_tgl != _status[no].info.move_tgl)
		{
			if (_status[no].info.z_in_ref == TRUE)
			{
				//				if (senv->flag_cap_empty == FALSE)   // Enable for capping that can fill
				//				{
				//					int set_flag = 1;
				//					sok_send_2(_step_socket[no], CMD_CAP_SET_VAC, sizeof(set_flag), &set_flag); // Start Vacuum
				//				}
				senv->cln_move_tgl = _status[no].info.move_tgl;
				sok_send_2(_step_socket[no], CMD_CLN_REFERENCE, 0, NULL); // ref Cleaning
				senv->slide_enable = TRUE;
				senv->capCurrentState = ST_REF_CLEAN;
			}
			else
			{
				ErrorExFlag(dev_stepper, no, ERR_CONT, &_ErrorFlags[no], ERR_5, 0, "MAIN: State_Command 0x%08x: Reference not done, retry reference", senv->st_cmd_stored);					
			}
		}
		break;


	case ST_REF_CLEAN:
		if (senv->cln_move_tgl != _status[no].info.move_tgl)
		{
			if (_status[no].info.x_in_ref == TRUE)
			{
				senv->capCurrentState = ST_IDLE;
			}
			else
			{
			//	ErrorEx(dev_stepper, no, ERR_CONT, 0, "MAIN: State_Command 0x%08x: Reference not done, retry reference", senv->st_cmd_stored);
				ErrorEx(dev_stepper, no, ERR_CONT, 0, "MAIN: State_Command 0x%08x: Reference not done", senv->st_cmd_stored);
				senv->capCurrentState = ST_IDLE;
			}
		}
		break;

	case ST_CLEAN_PRE_POS:
		if (senv->lift_move_tgl != _status[no].info.move_tgl)
		{
			if (_status[no].info.z_ref_done == TRUE) // && ((_status[no].posZ - positions.reset.pos_lift) <= 10))
			{
				senv->capCurrentState = ST_IDLE;
				senv->flag_cap_empty = TRUE;
				if (senv->dry_wipe_done == TRUE)
				{
					//senv->st_cmd = FALSE;
				}
				else
				{
					//senv->st_cmd = CMD_CLN_WIPE; // Enable for capping that can fill
				}
			}
			else
			{
				Error(ERR_CONT, 0, "MAIN: State_Command 0x%08x: Reference not done, retry reference", senv->st_cmd_stored);
			}
		}
		break;

	case ST_IDLE:
		senv->cycle_sok_send = 0;
		if (_status[no].info.ref_done == FALSE || _status[no].info.ref_done == FALSE)
		{
			senv->st_cmd = senv->st_cmd_stored;
//			senv->capCurrentState = ST_IDLE;
//			Error(LOG, 0, "MAIN: State_Command 0x%08x: Reference not done, retry reference", senv->st_cmd_stored);
			break;
		}
		if (senv->st_cmd != FALSE)
		{
			senv->flag_wipe_done = FALSE;
			senv->flag_purge_done = FALSE;
		}
		senv->cln_move_tgl = _status[no].info.move_tgl;
		senv->lift_move_tgl = _status[no].info.move_tgl;
		senv->st_cmd_stored = senv->st_cmd;
		
		if (senv->st_cmd == CMD_CLN_FLUSH_TEST)
		{
			senv->slide_enable = FALSE;
			senv->st_cmd = FALSE;
			senv->lift_pos = positions.capping.pos_lift;
			senv->clean_pos = positions.capping.pos_cln; // CLN_POS_PRINTHEADS
			
		}
		
		if (senv->st_cmd == CMD_CLN_WIPE)
		{
			senv->slide_enable = TRUE;
			senv->lift_pos = positions.wiping.pos_lift;
			//senv->clean_pos = positions.wiping.pos_cln - CLN_DIST_WIPE_ONE_HEAD * senv->clean_wipe_head_nr; //x;
			//senv->clean_wipe_pos = positions.wiping.pos_cln - CLN_DIST_WIPE_ONE_HEAD * (senv->clean_wipe_head_nr + 1) - CLN_DIST_WIPEBLADE_FLEX; //x;
			senv->clean_pos = positions.wiping.pos_cln; // CLN_POS_PRINTHEADS; //  positions.wiping.pos_cln; //x;
			senv->clean_wipe_pos = CLN_POS_END_WIPE; //  positions.wiping.pos_cln - CLN_DIST_WIPE_ONE_HEAD * 12 - CLN_DIST_WIPEBLADE_FLEX; //x;
			senv->st_cmd_stored = senv->st_cmd;
			senv->st_cmd = FALSE;
			if ((senv->clean_wipe_pos < CLN_POS_END_WIPE) || (senv->clean_pos < CLN_POS_END_WIPE)) break; // printf("desired wipe wipes longer than POS_END_WIPE, cancel wiping"); 
			_WipeReady &= ~(1<<no);
			_check_ref(no, senv); // Next State ST_PRE_MOVE_POS OR ST_CLEAN_MOVE_POS
		}
		if (senv->st_cmd == CMD_CLN_WIPE_VACUUM)
		{
			senv->slide_enable = TRUE;
			senv->lift_pos = positions.wiping_vac.pos_lift;
			senv->clean_pos = positions.wiping_vac.pos_cln; // CLN_POS_PRINTHEADS; //  positions.wiping.pos_cln; //x;
			senv->clean_wipe_pos = CLN_POS_END_WIPE; //  positions.wiping.pos_cln - CLN_DIST_WIPE_ONE_HEAD * 12 - CLN_DIST_WIPEBLADE_FLEX; //x;
			senv->st_cmd_stored = senv->st_cmd;
			senv->st_cmd = FALSE;
			if ((senv->clean_wipe_pos < CLN_POS_END_WIPE) || (senv->clean_pos < CLN_POS_END_WIPE)) break; // printf("desired wipe wipes longer than POS_END_WIPE, cancel wiping"); 
			_check_ref(no, senv); // Next State ST_PRE_MOVE_POS OR ST_CLEAN_MOVE_POS
		}
		//		if (senv->st_cmd == CMD_CAP_PURGE)
		//		{
		//			senv->slide_enable = FALSE;
		//			if (trace) Error(LOG, 0, "Flag: SLIDE disable");
		//			senv->st_cmd = FALSE;
		//			senv->lift_pos = positions.purge.pos_lift;
		//			//senv->clean_pos = positions.purge.pos_cln;
		//			senv->clean_pos = positions.purge.pos_cln - CLN_DIST_WIPE_ONE_HEAD * senv->clean_wipe_head_nr;
		//			//senv->clean_pos = positions.purge.pos_cln - CLN_DIST_WIPE_ONE_HEAD * 0;
		//			printf("start purge \n");
		//			if ((abs(_status[no].posZ - senv->lift_pos) <= 10) && (plc_in_purge_pos() == TRUE)) // && (abs(RX_ClnStatus.posX - senv->clean_pos) <= 10)
		//			{
		//				senv->capCurrentState = ST_PURGE;
		//			}
		//			else
		//			{
		//				sok_send_2(_step_socket[no], CMD_CAP_CAPPING_POS, 0, NULL); 
		//				senv->capCurrentState = ST_PRE_MOVE_POS;
		//			}
		//		}
		if (senv->st_cmd == CMD_CLN_SCREW_0_POS)
		{
			senv->st_cmd = FALSE;
			if (!(_cln_state_env[no].screw_data.bar == no)) break;
			senv->slide_enable = TRUE;
			printf("start screwing of screw nr %ld \n", (long)senv->screw_data.nr);
			if (senv->screw_data.nr > 8 * 12){ break; }										// limit the number of heads to maximum 12 heads in a row
			int screw_dist = (senv->screw_data.nr % 2) * POS_HEADSCREW_DELTA;
			int head_dist = (INT32)((senv->screw_data.nr / 2) * POS_HEAD_DELTA);					// count from zero
			senv->lift_pos = positions.screw0.pos_lift;
			senv->clean_pos = positions.screw0.pos_cln + screw_dist + head_dist + CLN_SCREW_SEARCH_DELTA + CLN_SCREW_TO_SENSOR;		// break;
			senv->clean_wipe_pos = positions.screw0.pos_cln + screw_dist + head_dist - CLN_SCREW_SEARCH_DELTA + CLN_SCREW_TO_SENSOR; //x;
			if ((senv->clean_pos < senv->screw_0_pos) || (senv->clean_wipe_pos > senv->screw_0_pos)){ senv->screw_0_pos = 0; }
			if ((abs(_status[no].posZ - senv->lift_pos) <= 10) && ((abs(_status[no].posX - senv->screw_0_pos) <= 10) || (abs(_status[no].posX - senv->screw_1_pos) <= 10)) && (_status[no].posX > (POS_SCREW_0 - CLN_SCREW_SEARCH_DELTA))) // && (plc_in_wipe_pos() == TRUE))
			{
				int srew_mot_nr = senv->screw_data.bar + 1;
				sok_send_2(_step_socket[no], CMD_CLN_SCREW_ATTACH, sizeof(srew_mot_nr), &srew_mot_nr); // execute ref of screws
				senv->capCurrentState = ST_SCREW_REF;
			}
			else
			{
				_check_ref(no, senv); // Next State ST_PRE_MOVE_POS OR ST_CLEAN_MOVE_POS
			}
		}
		break;

	case ST_CAP_CLEAN_START:
		//if (((_status[no].info.cln_in_stored == TRUE) && (_status[no].info.cln_in_stored_1 == TRUE)) || (senv->flag_cap_empty == TRUE))  // Enable for capping that can fill
		//{
		senv->flag_cap_empty = TRUE;
		senv->lift_move_tgl = _status[no].info.move_tgl;
		Error(LOG, 0, "send CMD_CAP_ROB_Z_POS to stepper[%d]", no);
		sok_send_2(_step_socket[no], CMD_CAP_ROB_Z_POS, sizeof(_reset_pos), &_reset_pos); // Lift up to zero position
		senv->capCurrentState = ST_PRE_MOVE_POS;
		//		}
		//		else {
		//			int set_flag = 2; // 3
		//			sok_send_2(_step_socket[no], CMD_CLN_EMPTY_CAP, sizeof(set_flag), &set_flag); // Clean Cap valve inside
		//			set_flag = 2; // 1;
		//			senv->lift_move_tgl = _status[no].info.move_tgl;
		//			sok_send_2(_step_socket[no], CMD_CAP_EMPTY_CAP, sizeof(set_flag), &set_flag); // Empty Cap
		//			senv->capCurrentState = ST_CAP_CLEAN_IN;
		//			senv->dry_wipe_done = FALSE;
		//			//senv->capCurrentState = ST_CAP_CLEAN_OUT;
		//		}
		break;

		//	case ST_CAP_CLEAN_IN:  // Enable for capping that can fill
		//		if (senv->lift_move_tgl != _status[no].info.move_tgl)
		//		{
		//			int set_flag = 1;
		//			sok_send_2(_step_socket[no], CMD_CLN_EMPTY_CAP, sizeof(set_flag), &set_flag); // Clean Cap valve outside
		//			set_flag = 1;
		//			senv->lift_move_tgl = _status[no].info.move_tgl;
		//			sok_send_2(_step_socket[no], CMD_CAP_EMPTY_CAP, sizeof(set_flag), &set_flag); // Empty Cap
		//			senv->capCurrentState = ST_CAP_CLEAN_OUT;
		//		}
		//		break;
		//
		//	case ST_CAP_CLEAN_OUT:
		//		if (senv->lift_move_tgl != _status[no].info.move_tgl)
		//		{
		//			int set_flag = 0;
		//			//sok_send_2(_step_socket[no], CMD_CLN_EMPTY_CAP, sizeof(set_flag), &set_flag); // Clean Cap valve off
		//			senv->cln_move_tgl = _status[no].info.move_tgl;
		//			sok_send_2(_step_socket[0], CMD_CLN_DRAIN_WASTE, 0, NULL); // Clear Valves Drain Cap Outside
		//			senv->capCurrentState = ST_DRAIN_WASTE;
		//		}
		//		break;
		//
		//	case ST_DRAIN_WASTE:
		//		if (senv->cln_move_tgl != _status[no].info.move_tgl)
		//		{
		//			int set_flag = 0;
		//			sok_send_2(_step_socket[no], CMD_CLN_EMPTY_CAP, sizeof(set_flag), &set_flag); // Clean Cap valve off
		//			senv->lift_move_tgl = _status[no].info.move_tgl;
		//			sok_send_2(_step_socket[no], CMD_CAP_PRINT_POS, sizeof(_reset_pos), &_reset_pos); // Lift up to zero position
		//			senv->capCurrentState = ST_PRE_MOVE_POS;
		//		}
		//		break;

	case ST_PRE_MOVE_POS: // Move Lift to capping pos // move_ok // z_in_print
		if (senv->lift_move_tgl != _status[no].info.move_tgl)
		{
			if (abs(_status[no].posZ -  _reset_pos) <= 10) //  && (_status[no].info.z_in_print == TRUE)) // && (_status[no].info.ref_done == TRUE) // if (_status[no].info.z_in_cap == TRUE) // if (_status[no].info.z_in_print == TRUE) // _status[no].info.z_in_ref == TRUE)  || _status[no].info.z_in_print == TRUE)
			{
				//if (senv->flag_cap_full == TRUE)
				//{
				//	int set_flag = 3;
				//	sok_send_2(_step_socket[no], CMD_CLN_EMPTY_CAP, sizeof(set_flag), &set_flag); // Clean Cap valve
				//	set_flag = 1;
				//	sok_send_2(_step_socket[no], CMD_CAP_SET_VAC, sizeof(set_flag), &set_flag); // Start Vacuum
				//}
				senv->cln_move_tgl = _status[no].info.move_tgl;
				sok_send_2(_step_socket[no], CMD_CLN_MOVE_POS, sizeof(senv->clean_pos), &senv->clean_pos); // Cleaning move to position
				senv->capCurrentState = ST_CLEAN_MOVE_POS;
			}
			else
			{
				senv->st_cmd = senv->st_cmd_stored;
				senv->capCurrentState = ST_INIT;
				Error(LOG, 0, "MAIN: State_Command 0x%08x: ST_PRE_MOVE_POS retry reference", senv->st_cmd_stored);
			}
		}
		break;

	case ST_CLEAN_MOVE_POS:
		if (senv->cln_move_tgl != _status[no].info.move_tgl)
		{
			int set_flag = 0;
			//sok_send_2(_step_socket[no], CMD_CAP_SET_VAC, sizeof(set_flag), &set_flag); // Stop Vacuum
			//sok_send_2(_step_socket[no], CMD_CLN_EMPTY_CAP, sizeof(set_flag), &set_flag); // Clean Cap valve off
			if (_status[no].info.move_ok == FALSE)
			{
				senv->capCurrentState = ST_INIT;
				Error(LOG, 0, "MAIN: State_Command 0x%08x: ST_CLEAN_MOVE_POS retry reference", senv->st_cmd_stored);
				break;
			}
			//if ((abs(_status[no].posZ - (belt_ref_height - senv->lift_pos)) <= 10)){
			//	senv->lift_move_tgl = !_status[no].info.move_tgl;
			//}
			//else{
			senv->lift_move_tgl = _status[no].info.move_tgl;
			if (senv->st_cmd_stored == CMD_CAP_PRINT_POS)
			{
				Error(LOG, 0, "send CMD_CAP_PRINT_POS to stepper[%d]", no);
				sok_send_2(_step_socket[no], CMD_CAP_PRINT_POS, sizeof(senv->lift_pos), &senv->lift_pos); // Lift move to position
			}
			else
				sok_send_2(_step_socket[no], CMD_CAP_ROB_Z_POS, sizeof(senv->lift_pos), &senv->lift_pos); // Lift move to position
			//}
			senv->set_io_cnt = _status[no].set_io_cnt;
			i = 0;
			senv->capCurrentState = ST_LIFT_MOVE_POS;
		}
		break;

	case ST_LIFT_MOVE_POS:
		if (senv->lift_move_tgl != _status[no].info.move_tgl)
		{
			if (_status[no].info.z_ref_done == FALSE)
			{
				senv->capCurrentState = ST_INIT;
				Error(ERR_CONT, 0, "MAIN: State_Command 0x%08x: ST_LIFT_MOVE_POS retry reference", senv->st_cmd_stored);
				break;
			}
			if ((senv->st_cmd_stored == CMD_CAP_CAPPING_POS) || (senv->st_cmd_stored == CMD_CLN_FLUSH_TEST))
			{
				int set_flag = 1;
				senv->lift_move_tgl = _status[no].info.move_tgl;
				sok_send_2(_step_socket[no], CMD_CAP_CAPPING_POS, 0, NULL); // Lift move to capping
				senv->capCurrentState = ST_LIFT_CAPPING;
			}
			else if (senv->st_cmd_stored == CMD_CLN_WIPE)
			{
				int set_flag = 1;
				//				if (senv->dry_wipe_done == FALSE)
				//				{
				//					sok_send_2(_step_socket[no], CMD_CLN_DRY_WIPE, sizeof(set_flag), &set_flag); // start dry wipe to clean capping
				//					sok_send_2(_step_socket[0], CMD_CAP_SET_VAC, sizeof(set_flag), &set_flag); // start vacuum
				//					//sok_send_2(_step_socket[0], CMD_CLN_SET_WASTE_PUMP, sizeof(set_flag), &set_flag); // start waste pump
				//					//sok_send_2(_step_socket[0], CMD_CAP_SET_VAC, sizeof(set_flag), &set_flag); // start vacuum
				//				}
				//				else {
				//sok_send_2(_step_socket[no], CMD_CLN_WET_WIPE, sizeof(set_flag), &set_flag); // start wet wipe
				//sok_send_2(_step_socket[0], CMD_CAP_SET_PUMP, sizeof(set_flag), &set_flag); // start flush pump
				//sok_send_2(_step_socket[0], CMD_CLN_SET_WASTE_PUMP, sizeof(set_flag), &set_flag); // start waste pump
				//sok_send_2(_step_socket[0], CMD_CAP_SET_PUMP, sizeof(set_flag), &set_flag); // start flush pump dummy
				//}
				if (_status[no].set_io_cnt == senv->set_io_cnt) 
				{
					switch (senv->cycle_sok_send)
					{
					case 0: sok_send_2(_step_socket[no], CMD_CLN_WET_WIPE, sizeof(set_flag), &set_flag); // start wet wipe
						senv->set_io_cnt++;
						senv->cycle_sok_send++;
						break;
					case 1: if (no == 0)
							{
										sok_send_2(_step_socket[0], CMD_CAP_SET_PUMP, sizeof(set_flag), &set_flag); // start flush pump
										senv->set_io_cnt++;
										senv->cycle_sok_send++;
							}
							else
							{
								senv->cycle_sok_send++;
							}
							break;
					case 2: if (no == 0)
							{
								sok_send_2(_step_socket[0], CMD_CLN_SET_WASTE_PUMP, sizeof(set_flag), &set_flag); // start waste pump
								senv->set_io_cnt++;
								senv->cycle_sok_send++;
							}
							else
							{
								senv->cycle_sok_send++;
							}
							break;
					case 3: senv->dry_wipe_done = TRUE;
						senv->flag_cap_empty = TRUE;
						senv->cln_move_tgl = _status[no].info.move_tgl;
						sok_send_2(_step_socket[no], CMD_CLN_WIPE, sizeof(senv->clean_wipe_pos), &senv->clean_wipe_pos); // Wipe move to position
						senv->set_io_cnt = _status[no].set_io_cnt;
						senv->cycle_sok_send = 0;
						senv->capCurrentState = ST_WIPE;
						break;
					}
				}
			}
			else if (senv->st_cmd_stored == CMD_CLN_WIPE_VACUUM)
			{
				int set_flag;
				_WipeReady |= (1<<no);
				if ((_status[no].set_io_cnt == senv->set_io_cnt) && _WipeReady == (2^STEPPER_CNT-1)) // && (_status[0].set_io_cnt == senv->set_io_cnt))
				{
					switch (senv->cycle_sok_send)
					{
					case 0: 				set_flag = 2;
						sok_send_2(_step_socket[no], CMD_CLN_DRY_WIPE, sizeof(set_flag), &set_flag); // start vacuum blade wipe
						senv->set_io_cnt++;
						senv->cycle_sok_send++;
						break;
					case 1: 				set_flag = 1;
						if (no == 0){
							sok_send_2(_step_socket[0], CMD_CAP_SET_VAC, sizeof(set_flag), &set_flag); // start vacuum
							senv->set_io_cnt++;
							senv->cycle_sok_send++;
						}
						else
						{
							senv->cycle_sok_send++;
						}
						break;
					case 2: 				//sok_send_2(_step_socket[0], CMD_CLN_SET_WASTE_PUMP, sizeof(set_flag), &set_flag); // start waste pump
						//sok_send_2(_step_socket[0], CMD_CAP_SET_VAC, sizeof(set_flag), &set_flag); // start vacuum dummy
						senv->dry_wipe_done = TRUE;
						senv->flag_cap_empty = TRUE;
						senv->cln_move_tgl = _status[no].info.move_tgl;
						sok_send_2(_step_socket[no], CMD_CLN_WIPE_VAC, sizeof(senv->clean_wipe_pos), &senv->clean_wipe_pos); // Wipe slow for Vacuum move to position
						senv->set_io_cnt = _status[no].set_io_cnt;
						senv->cycle_sok_send = 0;
						senv->capCurrentState = ST_WIPE;
						break;
					}
				}
			}
			else if (senv->st_cmd_stored == CMD_CLN_SCREW_0_POS)
			{
				senv->cln_move_tgl = _status[no].info.move_tgl;
				senv->capCurrentState = ST_DETECT_SCREW;
				sok_send_2(_step_socket[no], CMD_CLN_DETECT_SCREW_0, sizeof(senv->clean_wipe_pos), &senv->clean_wipe_pos); // detect screw
			}
			else
			{
				senv->capCurrentState = ST_IDLE;
				senv->st_cmd_stored = FALSE;
			}
			//senv->cln_move_tgl = _status[no].info.move_tgl;
			//sok_send_2(_step_socket[no], CMD_CLN_MOVE_POS, sizeof(senv->clean_pos), &senv->clean_pos); // Cleaning move to position
			//senv->capCurrentState = ST_CLEAN_MOVE_POS;
			//if ((_status[no].info.z_in_print == TRUE) || (_status[no].info.z_in_cap == TRUE)) // New: check also for Lift beeing in capping
			//{
			//	printf("state %d ST_LIFT_MOVE_POS move ok\n", senv->capCurrentState);
			//	if (senv->st_cmd_stored == CMD_CAP_PURGE)
			//	{
			//		senv->capCurrentState = ST_PURGE;
			//	}
			//	else
			//	{
			//printf("state %d ST_LIFT_MOVE_POS else ok\n", senv->capCurrentState);
			//senv->lift_move_tgl = _status[no].info.move_tgl;
			//senv->capCurrentState = ST_IDLE;
			//	}
			//}
			//}
		}
		else
		{
			senv->set_io_cnt = _status[no].set_io_cnt;
		}
		break;

	case ST_LIFT_CAPPING:
		if (senv->lift_move_tgl != _status[no].info.move_tgl)
		{
			if (_status[no].info.z_in_cap == TRUE)
			{
				//				if (senv->st_cmd_stored == CMD_CLN_FLUSH_TEST)   // Enable for capping that can fill
				//				{
				//					int set_flag = 3; // empty inside and outside // 2 empty inside // 1 empty outside
				//					sok_send_2(_step_socket[no], CMD_CLN_EMPTY_CAP, sizeof(set_flag), &set_flag); // Set Valves to empty cap
				//					set_flag = 2;
				//					senv->lift_move_tgl = _status[no].info.move_tgl;
				//					sok_send_2(_step_socket[no], CMD_CAP_EMPTY_CAP, sizeof(set_flag), &set_flag); // Empty Cap
				//				}
				//				else // (senv->st_cmd_stored == CMD_CAP_CAPPING_POS)
				//				{
				//					int set_flag = 1;
				//					sok_send_2(_step_socket[no], CMD_CLN_FILL_CAP, sizeof(set_flag), &set_flag); // Set Valves to fill cap
				//					senv->lift_move_tgl = _status[no].info.move_tgl;
				//					sok_send_2(_step_socket[no], CMD_CAP_FILL_CAP, 0, NULL); // Fill Cap
				//				}
				//				senv->capCurrentState = ST_CAP_FILL;
				senv->flag_cap_empty = FALSE;
				senv->capCurrentState = ST_IDLE;   // Disable for capping that can fill
				senv->st_cmd_stored = FALSE;       // Disable for capping that can fill
			}
			else {
				Error(LOG, 1, "MAIN: State_Command 0x%08x: Could not reach capping, please reference", senv->st_cmd_stored);
			}
		}
		break;

		//	case ST_CAP_FILL:
		//		if (senv->lift_move_tgl != _status[no].info.move_tgl)
		//		{
		//			int set_flag = 0;
		//			sok_send_2(_step_socket[no], CMD_CLN_FILL_CAP, sizeof(set_flag), &set_flag); // Clear Valves to fill cap
		//			sok_send_2(_step_socket[0], CMD_CAP_SET_PUMP, sizeof(set_flag), &set_flag); // Dummy Send
		//			senv->cln_move_tgl = _status[no].info.move_tgl;
		//			sok_send_2(_step_socket[0], CMD_CLN_DRAIN_WASTE, 0, NULL); // Clear Valves Drain Cap Outside
		//			senv->capCurrentState = ST_DRAIN_WASTE_IDLE;
		//		}
		//		break;
		//
		//	case ST_DRAIN_WASTE_IDLE:
		//		if (senv->cln_move_tgl != _status[no].info.move_tgl)
		//		{
		//			senv->capCurrentState = ST_IDLE;
		//			senv->st_cmd_stored = FALSE;
		//		}
		//		break;

		//case  ST_PURGE:
		//	printf("state %d Purge done\n", senv->capCurrentState);
		//	senv->flag_purge_done = TRUE;
		//	if (trace) Error(LOG, 0, "Flag: PURGE DONE enable");
		//	senv->capCurrentState = ST_IDLE;
		//	break;

	case ST_WIPE:
		if ((_status[no].posX < CLN_POS_END_WET_WIPE) && (senv->flag_wet_wipe_done == FALSE) && (no == 0)) {
			senv->flag_wet_wipe_done = TRUE;
			int set_flag = 0;
			sok_send_2(_step_socket[0], CMD_CAP_SET_PUMP, sizeof(set_flag), &set_flag); // stop wet wipe pump
		}
		if (senv->cln_move_tgl != _status[no].info.move_tgl)
		{
			int set_flag = 0;

			if ((_status[no].set_io_cnt == senv->set_io_cnt)) // && (_status[0].set_io_cnt == senv->set_io_cnt))
			{
				switch (senv->cycle_sok_send)
				{
				case 0: if (no == 0)
				{
							sok_send_2(_step_socket[0], CMD_CAP_SET_PUMP, sizeof(set_flag), &set_flag); // stop wet wipe pump
							senv->set_io_cnt++;
							senv->cycle_sok_send++;
				}
						else
						{
							senv->cycle_sok_send++;
						}
						break;
				case 1: if (no == 0)
				{
							sok_send_2(_step_socket[0], CMD_CAP_SET_VAC, sizeof(set_flag), &set_flag); // stop wet wipe vac
							senv->set_io_cnt++;
							senv->cycle_sok_send++;
				}
						else
						{
							senv->cycle_sok_send++;
						}
						break;
				case 2: if (no == 0)
				{
							sok_send_2(_step_socket[0], CMD_CLN_SET_WASTE_PUMP, sizeof(set_flag), &set_flag); // stop waste pump
							senv->set_io_cnt++;
							senv->cycle_sok_send++;
				}
						else
						{
							senv->cycle_sok_send++;
						}
						break;
				case 3: sok_send_2(_step_socket[no], CMD_CLN_WET_WIPE, sizeof(set_flag), &set_flag); // stop wet wipe
					senv->set_io_cnt++;
					senv->cycle_sok_send++;
					break;
				case 4: sok_send_2(_step_socket[no], CMD_CLN_DRY_WIPE, sizeof(set_flag), &set_flag); // stop dry wipe
					senv->set_io_cnt++;
					senv->cycle_sok_send++;
					break;
				case 5: 			senv->lift_move_tgl = _status[no].info.move_tgl;
					Error(LOG, 0, "send CMD_CAP_PRINT_POS to stepper[%d]", no);
					sok_send_2(_step_socket[no], CMD_CAP_PRINT_POS, sizeof(_reset_pos), &_reset_pos); // up to reset
					senv->capCurrentState = ST_LAST_LIFT;
					// -------------------------------------------------------------------------  senv->st_cmd = CMD_CLN_SCREW_0_POS; // REMOVE FOR MACHINE // ONLY TEST // ENDLESS LOOP !!!
					senv->test_loop_cnt++;
					senv->cycle_sok_send = 0;
					senv->set_io_cnt = _status[no].set_io_cnt;
					senv->flag_wipe_done = TRUE;
					senv->flag_wet_wipe_done = FALSE;
					senv->clean_wipe_head_nr = 0;
					//Error(LOG, 0, "TEST v9: Loop count done = %d", senv->test_loop_cnt);
					break;
				}
			}

		}
		else
		{
			senv->set_io_cnt = _status[no].set_io_cnt;
		}
		break;

		//case  ST_CABLE_CLEAN:

		//	if (senv->cln_move_tgl != _status[no].info.move_tgl)
		//	{
		//			senv->lift_move_tgl = _status[no].info.move_tgl;
		//			sok_send_2(_step_socket[no], CMD_CAP_PRINT_POS, sizeof(_reset_pos), &_reset_pos); // up to reset
		//			senv->capCurrentState = ST_LAST_LIFT;
		//	}
		//	break;

	case  ST_LAST_LIFT:

		if (senv->lift_move_tgl != _status[no].info.move_tgl)
		{
			//if (_status[no].info.z_in_print == TRUE) // _status[no].info.z_in_ref == TRUE)  || _status[no].info.z_in_print == TRUE)
			{
				senv->capCurrentState = ST_IDLE;
				senv->st_cmd_stored = FALSE;
			}
		}
		break;

	case ST_DETECT_SCREW:
		if (senv->cln_move_tgl != _status[no].info.move_tgl)
		{
			if (_status[no].info.move_ok == TRUE)
			{
				senv->cln_move_tgl = !_status[no].info.move_tgl;
				senv->capCurrentState = ST_REPOS_SCREW;
			}
			else
			{
				senv->cln_move_tgl = _status[no].info.move_tgl;
				senv->capCurrentState = ST_IDLE;
				ErrorFlag(ERR_CONT, &_ErrorFlags[no], ERR_0, 0, "MAIN: State_Command 0x%08x: ST_SCREW screw detecting screw failed, adjust screw hwight position", senv->st_cmd_stored);
			}
			//if (senv->screw_data.bar == 0)
			//{
			//	if (_status[no].info.cln_screw_2 == TRUE)
			//	{
			//		senv->cln_move_tgl = _status[no].info.move_tgl;
			//		int move = _status[no].posX - CLN_DET_SCREW_TO_SENSOR;
			//		sok_send_2(_step_socket[no], CMD_CLN_MOVE_POS, sizeof(move), &move); // move to screw
			//		senv->capCurrentState = ST_REPOS_SCREW;
			//	}
			//	else
			//	{
			//		ErrorFlag(ERR_CONT, &_ErrorFlags[no], ERR_6, 0, "MAIN: State_Command 0x%08x: no screw 0 found. Check Head and sensor", senv->st_cmd_stored);
			//		senv->capCurrentState = ST_IDLE;
			//		senv->st_cmd_stored = FALSE;
			//		//sok_send_2(_step_socket[no], CMD_CAP_STOP, 0, NULL);
			//		// -------------------------------------------------------------------------  senv->st_cmd = CMD_CLN_WIPE; // REMOVE FOR MACHINE // ONLY TEST // ENDLESS LOOP !!!
			//	}
			//}
			//else if (senv->screw_data.bar == 1)
			//{
			//	if (_status[no].info.cln_screw_3 == TRUE)
			//	{
			//		senv->cln_move_tgl = _status[no].info.move_tgl;
			//		int move = _status[no].posX - CLN_DET_SCREW_TO_SENSOR;
			//		sok_send_2(_step_socket[no], CMD_CLN_MOVE_POS, sizeof(move), &move); // move to screw
			//		senv->capCurrentState = ST_REPOS_SCREW;
			//	}
			//	else
			//	{
			//		ErrorFlag(ERR_CONT, &_ErrorFlags[no], ERR_7, 0, "MAIN: State_Command 0x%08x: no screw 1 found. Check Head and sensor", senv->st_cmd_stored);
			//		senv->capCurrentState = ST_IDLE;
			//		senv->st_cmd_stored = FALSE;
			//		//sok_send_2(_step_socket[no], CMD_CAP_STOP, 0, NULL);
			//		// -------------------------------------------------------------------------  senv->st_cmd = CMD_CLN_WIPE; // REMOVE FOR MACHINE // ONLY TEST // ENDLESS LOOP !!!
			//	}
			//}
		}
		break;

	case ST_REPOS_SCREW:
		if (senv->cln_move_tgl != _status[no].info.move_tgl)
		{
				senv->screw_0_pos = _status[no].posX;
				senv->screw_1_pos = 0;
			senv->cln_move_tgl = _status[no].info.move_tgl;
			int srew_mot_nr = senv->screw_data.bar + 1;
			sok_send_2(_step_socket[no], CMD_CLN_SCREW_ATTACH, sizeof(srew_mot_nr), &srew_mot_nr); // execute ref of screws
			senv->capCurrentState = ST_SCREW_REF;
		}

	case ST_SCREW_REF:
		// int mov_rel = 200;
		//sok_send_2(_step_socket[no], CMD_CLN_MOVE_POS_REL, sizeof(mov_rel), &mov_rel);
		if (senv->cln_move_tgl != _status[no].info.move_tgl)
		{
			if (_status[no].info.screw_in_ref == TRUE)
			{
				senv->cln_move_tgl = _status[no].info.move_tgl;
				// Define Screw to turn
					sok_send_2(_step_socket[no], CMD_CLN_SCREW_0_POS, sizeof(senv->screw_data.rot), &senv->screw_data.rot);
					// -------------------------------------------------------------------------  senv->screw_data.rot = -(senv->screw_data.rot); // REMOVE FOR MACHINE // ONLY TEST // ENDLESS LOOP !!!
				senv->capCurrentState = ST_SCREW;
			}
			else
			{
				ErrorFlag(ERR_CONT, &_ErrorFlags[no], ERR_1, 0, "MAIN: State_Command 0x%08x: screw referencing failed", senv->st_cmd_stored);
				senv->capCurrentState = ST_DRAIN_WASTE_IDLE;
				senv->st_cmd_stored = FALSE;
				sok_send_2(_step_socket[no], CMD_CLN_SCREW_REF, 0, NULL); // move to screw
			}
		}
		break;

	case ST_SCREW:
		if (senv->cln_move_tgl != _status[no].info.move_tgl)
		{
			if (_status[no].info.screw_done == TRUE)
			{
				printf("state %d ST_SCREW_REF screwing done\n", senv->capCurrentState);
			}
			else
			{
				ErrorFlag(ERR_CONT, &_ErrorFlags[no], ERR_2, 0, "MAIN: State_Command 0x%08x: ST_SCREW screw adjusting failed, screw might be at maximum or minimum", senv->st_cmd_stored);
			}
			// Detach Screwer from Screw anyway
			senv->cln_move_tgl = _status[no].info.move_tgl;
			sok_send_2(_step_socket[no], CMD_CLN_SCREW_DETACH, 0, NULL);
			senv->capCurrentState = ST_DETACH_SCREW;
		}
		break;

	case ST_DETACH_SCREW:
		if (senv->cln_move_tgl != _status[no].info.move_tgl)
		{
			if (_status[no].info.cln_screw_0 == TRUE)
			{
				//senv->lift_move_tgl = _status[no].info.move_tgl;
				//senv->cln_move_tgl = _status[no].info.move_tgl;
				//senv->capCurrentState = ST_LAST_LIFT;
				//sok_send_2(_step_socket[no], CMD_CAP_PRINT_POS, sizeof(_reset_pos), &_reset_pos); // up to reset
				// -------------------------------------------------------------------------  senv->st_cmd = CMD_CLN_WIPE; // REMOVE FOR MACHINE // ONLY TEST // ENDLESS LOOP !!!
			}
			else
			{
				senv->capCurrentState = ST_INIT;
				senv->st_cmd_stored = FALSE;
				senv->st_cmd = FALSE;
				ErrorFlag(ERR_CONT, &_ErrorFlags[no], ERR_3, 0, "MAIN: State_Command 0x%08x: ST_DETACH_SCREW screw detach failed", senv->st_cmd_stored);
				//sok_send_2(_step_socket[no], CMD_CAP_STOP, 0, NULL);
			}
			senv->capCurrentState = ST_IDLE;
			senv->st_cmd_stored = FALSE;
		}
		break;

	}

}

//--- stepc_reset_fluid_sys --------------------------------
static void	 _stepc_reset_fluid_sys(int no, SClnStateEnv *senv)
{
	int set_flag = 0;
	if (no == 0)sok_send_2(_step_socket[0], CMD_CAP_SET_PUMP, sizeof(set_flag), &set_flag); // stop wet wipe pump
	sok_send_2(_step_socket[no], CMD_CLN_WET_WIPE, sizeof(set_flag), &set_flag); // stop wet wipe
	//sok_send_2(_step_socket[no], CMD_CLN_FILL_CAP, sizeof(set_flag), &set_flag); // Clear Valves to fill cap
	if (no == 0)sok_send_2(_step_socket[0], CMD_CAP_SET_VAC, sizeof(set_flag), &set_flag); // stop vacc
	//sok_send_2(_step_socket[no], CMD_CLN_EMPTY_CAP, sizeof(set_flag), &set_flag); // Clear Cap valve inside
	//sok_send_2(_step_socket[0], CMD_CAP_SET_VAC, sizeof(set_flag), &set_flag); // stop vacc
	//sok_send_2(_step_socket[no], CMD_CLN_WET_WIPE, sizeof(set_flag), &set_flag); // stop wet wipe
}

//--- _check_ref --------------------------------------------------
static void _check_ref(int no, SClnStateEnv *senv)
{
	if ((abs(_status[no].posZ -  _reset_pos) <= 10) && (_status[no].info.ref_done == TRUE) && (senv->flag_cap_empty == TRUE))
	{
		sok_send_2(_step_socket[no], CMD_CLN_MOVE_POS, sizeof(senv->clean_pos), &senv->clean_pos); // Cleaning move to position
		senv->capCurrentState = ST_CLEAN_MOVE_POS;
	}
	else 
	{
		//sok_send_2(_step_socket[no], CMD_CAP_PRINT_POS, sizeof(_reset_pos), &_reset_pos); // Lift up to zero position
		//senv->capCurrentState = ST_PRE_MOVE_POS;
		senv->capCurrentState = ST_CAP_CLEAN_START;
	}
}


//--- _do_turn_screw --------------------------------------------------
static int _do_turn_screw(int no, SHeadAdjustment *pHeadAdjust)
{
	if (_cln_state_env[no].capCurrentState == ST_IDLE || _cln_state_env[no].capCurrentState == ST_INIT)
	{
		if (pHeadAdjust->angle == 0)
		{
			//			_screw_nr = (pHeadAdjust->headNo-1) * 2 + 0; // head nr von 0-3 ?? welche Schraube ist stitch, welche angle ? zuerst angle, dann stitch
			//			_screw_rot = (int)(pHeadAdjust->stitch * 1000 * SCREW_STITCH_REV_PER_UM);
			//			_screw_cluster = pHeadAdjust->inkSupplyNo; // 0-3 ??
			//_cln_state_env[no].screw_data.nr = (7 - pHeadAdjust->inkSupplyNo) * 2 + 0; // head nr von 0-3 ?? welche Schraube ist stitch, welche angle ? zuerst angle, dann stitch
			_cln_state_env[no].screw_data.nr = (pHeadAdjust->headNo - 1) * 2 + 1;
			_cln_state_env[no].screw_data.rot = (int)(pHeadAdjust->stitch * 1000 * SCREW_STITCH_REV_PER_UM);
			_cln_state_env[no].screw_data.bar = pHeadAdjust->inkSupplyNo; // 0-3 ??
		}
		else
		{
			//			_screw_nr = (pHeadAdjust->headNo-1) * 2 + 1; // head nr von 0-3 ?? welche Schraube ist stitch, welche angle ? zuerst angle, dann stitch
			//			_screw_rot = (int)(pHeadAdjust->angle * 1000 * SCREW_JAW_REV_PER_UM);
			//			_screw_cluster = pHeadAdjust->inkSupplyNo; // 0-3 ??
			//_cln_state_env[no].screw_data.nr = (7 - pHeadAdjust->inkSupplyNo) * 2 + 1; // head nr von 0-3 ?? welche Schraube ist stitch, welche angle ? zuerst angle, dann stitch
			_cln_state_env[no].screw_data.nr = (pHeadAdjust->headNo - 1) * 2 + 0;
			_cln_state_env[no].screw_data.rot = (int)(pHeadAdjust->angle * 1000 * SCREW_JAW_REV_PER_UM);
			_cln_state_env[no].screw_data.bar = pHeadAdjust->inkSupplyNo; // 0-3 ??
		}

		if (_cln_state_env[no].screw_data.bar <= 1)
		{
			_cln_state_env[no].st_cmd = CMD_CLN_SCREW_0_POS;
			return TRUE;
		}
		else
		{
			ErrorFlag(ERR_CONT, &_ErrorFlags[no], ERR_4, 0, "MAIN: State_Command ST_SCREW: unknown bar number");
			return FALSE;
		}

	}
	else
	{
		Error(WARN, 0, "MAIN: State_Command 0x%08x busy", _cln_state_env[no].st_cmd_stored);
		return FALSE;
	}
}


