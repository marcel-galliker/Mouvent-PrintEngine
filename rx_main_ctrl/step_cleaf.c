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
#include "step_ctrl.h"

static RX_SOCKET *_step_socket[2]={NULL, NULL};
static UINT32	  _step_ipaddr[2]={0,0};

static int _lift_pos = 0;
static int _clean_pos = 0;
static int _clean_wipe_pos = 0;
static UINT32 _st_cmd;
static UINT32 _st_cmd_stored;
static enum cln_state_code _capCurrentState = ST_INIT;
static int _lift_move_tgl = FALSE;
static int _cln_move_tgl = FALSE;
static int _head_safety_edge = FALSE;

//--- dimensions -----------------------
static int printer_height	=  94375; // um // 151000 steps *1000000 /1600000
static int wipe_height		=  10000; // 20000 ; capping 28000; // um // 45000 steps *1000000 /1600000
static int capping_height	=  10000;
static int belt_ref_height	=  94375; // 16000 um
static int capping_pos		= 500074; // um // 69801 steps *1000000 /139581.3
static int _reset_pos = 94375; // not negative !

//--- printhead ------------------------
#define POS_HEADSCREW_DELTA	16400 // delta between the two screws on the same head in mm
#define POS_HEAD_DELTA		43140
#define POS_CAP2SCREW_DELTA	25628

//--- position lift --------------------
#define LIFT_POS_PRINTING			printer_height // RX_StepperCfg.print_height
#define LIFT_POS_START_WIPE			(belt_ref_height-wipe_height) // RX_StepperCfg.wipe_height
#define LIFT_POS_STORED				belt_ref_height // 0 // Ref
//#define LIFT_POS_UP				1000
#define LIFT_POS_PLACE_PRINTHEADS	(belt_ref_height-capping_height) //LIFT_POS_START_WIPE // capping position
#define LIFT_POS_SCREW				LIFT_POS_PLACE_PRINTHEADS  // TODO: adjust distance for screwing from wiping pos (+1000)

//--- position cleaning ------------------
#define CLN_POS_PRINTHEADS		capping_pos // RX_StepperCfg.capping_pos // capping position
#define CLN_POS_STORED			0 // Ref
#define CLN_POS_START_WIPE		CLN_POS_PRINTHEADS //  - 20000 // Wipe start // TODO: adjust distance form capping to wiping (-1000)
#define CLN_POS_END_WIPE		CLN_POS_START_WIPE - 428000 - 20000 // to avoid jumping wiper blades
#define POS_SCREW_0			CLN_POS_PRINTHEADS - POS_CAP2SCREW_DELTA

#define POS_SCREW_1			POS_SCREW_0-POS_HEADSCREW_DELTA
#define POS_SCREW_2			POS_SCREW_0-POS_HEAD_DELTA
#define POS_SCREW_3			POS_SCREW_2-POS_HEADSCREW_DELTA
#define POS_SCREW_4			POS_SCREW_2-POS_HEAD_DELTA
#define POS_SCREW_5			POS_SCREW_4-POS_HEADSCREW_DELTA
#define POS_SCREW_6			POS_SCREW_4-POS_HEAD_DELTA
#define POS_SCREW_7			POS_SCREW_6-POS_HEADSCREW_DELTA // total -171448

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

static INT32 _ErrorFlags;

//--- screwing data ------------------
#define SCREW_JAW_REV_PER_UM	1.0/5.0  // 1/5 rev per um
#define SCREW_STITCH_REV_PER_UM	1.0/11.0  // 1/11 rev per um
static int _screw_nr = FALSE;
static int _screw_rot = FALSE;
static int _screw_cluster = FALSE;

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
	SPosItem screw0;
	SPosItem screw1;
	SPosItem screw2;
	SPosItem screw3;
	SPosItem screw4;
	SPosItem screw5;
	SPosItem screw6;
	SPosItem screw7;
} SPosName;


//--- prorotypes -------------------------------------------------
static void _do_tt_status(STestTableStat *pStatus);
static void _do_cln_status(STestTableStat *pStatus);
static int  _do_turn_screw(SHeadAdjustment *pHeadAdjust); // or _do_adjust_head


//--- stepc_init ---------------------------------------------------
void stepc_init(int steppperNo, RX_SOCKET *psocket, UINT32 ipaddr)
{
	_step_socket[steppperNo] = psocket;
	_step_ipaddr[steppperNo] = ipaddr;
	
	_ErrorFlags = 0;
}

//--- stepc_error_reset ----------------------------------------------------
void stepc_error_reset(void)
{
	_ErrorFlags = 0;
}

//--- stepc_handle_gui_msg------------------------------------------------------------------
int	 stepc_handle_gui_msg(RX_SOCKET socket, UINT32 cmd, void *data, int dataLen)
{
	switch(cmd)
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
				sok_send_2(_step_socket[0], _step_ipaddr[0], cmd, 0, NULL);
				break;

	case CMD_TT_SCAN:
				{
					STestTableScanPar par;
					par.speed	= 5;
					par.scanCnt = 5;
					par.scanMode= PQ_SCAN_STD;
					par.yStep   = 10000;

					sok_send_2(_step_socket[0], _step_ipaddr[0], CMD_TT_SCAN, sizeof(par), &par);
				}
				break;

	//--- cappping ---------------------------------------------------------
	case CMD_CAP_STOP:
		_st_cmd = cmd;
		printf("CMD 0x%08x STOP executed\n", cmd);
		break;

	case CMD_CAP_REFERENCE:
		_st_cmd = cmd;
		printf("CMD 0x%08x REFERENCE executed\n", cmd);
		break;

	case CMD_CAP_UP_POS:
		_st_cmd = cmd;
		// sok_send_2(_step_socket[0], _step_ipaddr[0], CMD_CAP_PRINT_POS, sizeof(_reset_pos), &_reset_pos); // Lift up to zero position
		// sok_send_2(_step_socket[0], _step_ipaddr[0], cmd, 0, NULL); execute up to savety
		printf("CMD 0x%08x Move to Zero executed\n", cmd);
		
		break;

	case CMD_CAP_CAPPING_POS:
		_st_cmd = cmd;
		printf("CMD 0x%08x CAPPING executed\n", cmd);
		break;
		
	case CMD_CAP_PRINT_POS:
		_st_cmd = cmd;
		printf("CMD 0x%08x PRINT_POS executed\n", cmd);
		break;
		
	case CMD_CLN_WIPE:
		_st_cmd = cmd;
		printf("CMD 0x%08x CMD_CLN_WIPE executed\n", cmd);
		break;
	
//	//--- cleaning ---------------------------------------------------------
	case CMD_CLN_ADJUST:		_do_turn_screw((SHeadAdjustment*)data);
								break;

	//--- Unknown Case ---------------------------------------------------------
	default:						Error(ERR_CONT, 0, "Command 0x%08x not implemented", cmd); break;
	}
	
	return REPLY_OK;
}

//--- stepc_handle_status ----------------------------------------------------------------------
int stepc_handle_status(int stepperNo, STestTableStat *pStatus)
{
	switch(stepperNo)
	{
	case 0: _do_tt_status(pStatus);		break;
	case 1: _do_cln_status(pStatus);	break;
	}	
	return REPLY_OK;
}

//--- _do_cln_status --------------------------------------------------
static void _do_cln_status(STestTableStat *pStatus)
{
	memcpy(&RX_ClnStatus, pStatus, sizeof(RX_ClnStatus));	
}

//--- _do_tt_status ----------------------------------------------
static void _do_tt_status(STestTableStat *pStatus)
{
	int last_pos		= RX_TestTableStatus.info.z_in_print;
	int last_spliceing	=  RX_TestTableStatus.info.splicing;
	memcpy(&RX_TestTableStatus, pStatus, sizeof(RX_TestTableStatus));
	RX_TestTableStatus.alive[1] = RX_ClnStatus.alive[0];
	RX_TestTableStatus.info.uv_on    = enc_is_uv_on();
	RX_TestTableStatus.info.uv_ready = enc_is_uv_ready();
	if(RX_Config.printer.type == printer_TX801 || RX_Config.printer.type == printer_TX802) RX_TestTableStatus.info.x_in_cap = plc_in_cap_pos();
	else if (RX_Config.printer.type == printer_cleaf) RX_TestTableStatus.info.x_in_cap = RX_ClnStatus.info.cln_in_capping;

	RX_TestTableStatus.state = _capCurrentState;
	
	if (RX_TestTableStatus.info.z_in_print!=last_pos) enc_enable_printing(RX_TestTableStatus.info.z_in_print);
	
	if(!last_spliceing && RX_TestTableStatus.info.splicing) enc_enable_printing(FALSE);
	else if(last_spliceing && !RX_TestTableStatus.info.splicing && RX_PrinterStatus.printState==ps_printing)
	{
		TrPrintfL(TRUE, "CMD_CAP_PRINT_POS after splice");
		_st_cmd = CMD_CAP_PRINT_POS;
	}

	//printf("Value ref_height	= %d \n", cfg.ref_height);
	//printf("Value print_height= %d \n", cfg.print_height);
	//printf("Value wipe_height	= %d \n", cfg.wipe_height);
	//printf("Value cap_height	= %d \n", cfg.cap_height);
	//printf("Value cap_pos		= %d \n", cfg.cap_pos);
	//RX_Config.stepper.ref_height;
	//RX_Config.stepper.print_height;
	//RX_Config.stepper.wipe_height;
	//RX_Config.stepper.cap_height;
	//RX_Config.stepper.cap_pos;
	
	_clean_wipe_pos = CLN_POS_END_WIPE;
	printer_height = RX_Config.stepper.print_height; // um // 151000 steps *1000000 /1600000
	wipe_height = RX_Config.stepper.wipe_height; // 20000 ; capping 28000; // um // 45000 steps *1000000 /1600000
	capping_pos = RX_Config.stepper.cap_pos; // um // 69801 steps *1000000 /139581.3
	belt_ref_height = RX_Config.stepper.ref_height;
	capping_height = RX_Config.stepper.cap_height;
	_reset_pos = belt_ref_height;
	
	struct SPosName	positions = 
	{
		{ CLN_POS_STORED, LIFT_POS_STORED },				//  0: reset
		{ CLN_POS_STORED, LIFT_POS_PRINTING },				//  1: printing
		{ CLN_POS_PRINTHEADS, LIFT_POS_PLACE_PRINTHEADS },	//  2: capping
		{ CLN_POS_START_WIPE, LIFT_POS_START_WIPE },		//  3: wiping
		{ POS_SCREW_0, LIFT_POS_SCREW },					//  4: screw0
		{ POS_SCREW_1, LIFT_POS_SCREW },					//  5: screw1
		{ POS_SCREW_2, LIFT_POS_SCREW },					//  6: screw2
		{ POS_SCREW_3, LIFT_POS_SCREW },					//  7: screw3
		{ POS_SCREW_4, LIFT_POS_SCREW },					//  8: screw4
		{ POS_SCREW_5, LIFT_POS_SCREW },					//  9: screw5
		{ POS_SCREW_6, LIFT_POS_SCREW },					// 10: screw6
		{ POS_SCREW_7, LIFT_POS_SCREW },					// 11: screw7
	};
		
	gui_send_msg_2(0, REP_TT_STATUS, sizeof(RX_TestTableStatus), &RX_TestTableStatus);
	
	if (RX_TestTableStatus.info.moving == FALSE) // && last command ...
	{
	}
	
	// --- Safety up&down for Splice ---
	if (RX_TestTableStatus.info.head_in_safety == TRUE && _head_safety_edge == FALSE) //
	{
		if (RX_ClnStatus.info.ref_done == TRUE) // do not go to Idle if not once referenced
		{
			_capCurrentState = ST_IDLE; 
		}
	}
	else if (RX_TestTableStatus.info.head_in_safety == TRUE 
		&& RX_TestTableStatus.info.printhead_en == TRUE 
		&& _capCurrentState == ST_IDLE
		&& RX_ClnStatus.info.ref_done
		&& RX_PrinterStatus.printState==ps_printing)
	{
		TrPrintfL(TRUE, "CMD_CAP_PRINT_POS after safety");
		_st_cmd = CMD_CAP_PRINT_POS;
	}
	_head_safety_edge = RX_TestTableStatus.info.head_in_safety;
	
	// --- Stop State Machine ---
	if (_st_cmd == CMD_CAP_STOP)
	{
		sok_send_2(_step_socket[0], _step_ipaddr[0], CMD_CAP_STOP, 0, NULL);
		sok_send_2(_step_socket[1], _step_ipaddr[1], CMD_CLN_STOP, 0, NULL);
		_st_cmd = FALSE;
		if (RX_ClnStatus.info.ref_done == TRUE) // do not go to Idle if not once referenced
		{
			_capCurrentState = ST_IDLE; 
		}
		printf("Stop State Machine and set to Idle\n");
	}
	
	// --- Reference State Machine ---
	if (_st_cmd == CMD_CAP_REFERENCE)
	{
	//	sok_send_2(_step_socket[0], _step_ipaddr[0], CMD_CAP_STOP, 0, NULL);
	//	sok_send_2(_step_socket[1], _step_ipaddr[1], CMD_CLN_STOP, 0, NULL);
		_capCurrentState = ST_INIT;
		printf("Init State Machine \n");
	}
	
	// --- State Machine CLN & WEB ctrl ---
	switch (_capCurrentState) {
		
	case ST_INIT:
		if (_st_cmd != FALSE)
		{
			_lift_move_tgl = RX_TestTableStatus.info.move_tgl;
			if (_st_cmd == CMD_CAP_REFERENCE)
			{
				_st_cmd = FALSE;
			}
			sok_send_2(_step_socket[0], _step_ipaddr[0], CMD_CAP_REFERENCE, 0, NULL); // ref Lift
			printf("pos lift print %d \n", positions.printing.pos_lift);
			printf("pos clean print %d \n", positions.printing.pos_cln);
			printf("pos lift cap %d \n", positions.capping.pos_lift);
			printf("pos clean cap %d \n", positions.capping.pos_cln);
			printf("state %d REFERENCE lift\n", _capCurrentState);
			_capCurrentState = ST_REF_LIFT;
		}
		break;
		
	case ST_REF_LIFT:
		if (_lift_move_tgl != RX_TestTableStatus.info.move_tgl)
		{
			if (RX_TestTableStatus.info.z_in_ref == TRUE)
			{
				_lift_move_tgl = RX_TestTableStatus.info.move_tgl;
				_cln_move_tgl = RX_ClnStatus.info.move_tgl;
				sok_send_2(_step_socket[0], _step_ipaddr[0], CMD_CAP_MECH_ADJUST, 0, NULL); // Lift ref to mech
				sok_send_2(_step_socket[1], _step_ipaddr[1], CMD_CLN_REFERENCE, 0, NULL); // ref Cleaning
				_capCurrentState = ST_REF_CLEAN;
			}
		}
		break;
		
	case ST_REF_CLEAN:
		if (_lift_move_tgl != RX_TestTableStatus.info.move_tgl)
		{
			if (RX_ClnStatus.info.z_in_ref == TRUE && RX_TestTableStatus.info.z_in_ref == TRUE)
			{
				_capCurrentState = ST_IDLE;
			}
			else
			{
				if (_cln_move_tgl != RX_ClnStatus.info.move_tgl)
				{
					//ErrorFlag(ERR_CONT, &_ErrorFlags, ERR_0, 0, "MAIN: State_Command 0x%08x: ST_REF_CLEAN2 wait on LIFT & CLN to be in reference", CMD_CAP_REFERENCE);
				}
			}
		}
		break;
		
	case ST_IDLE:
		_lift_move_tgl = RX_TestTableStatus.info.move_tgl;
		_cln_move_tgl = RX_ClnStatus.info.move_tgl;
		_st_cmd_stored = _st_cmd;
		if (_st_cmd == CMD_CAP_CAPPING_POS)
		{
			_st_cmd = FALSE;
			_lift_pos = positions.capping.pos_lift;
			_clean_pos = positions.capping.pos_cln;
			if ((abs(RX_TestTableStatus.posZ - _lift_pos) <= 10) && (abs(RX_ClnStatus.posZ - _clean_pos) <= 10)) break;
			printf("start capping \n");
			sok_send_2(_step_socket[0], _step_ipaddr[0], CMD_CAP_PRINT_POS, sizeof(_reset_pos), &_reset_pos); // Lift up to zero position
			_capCurrentState = ST_PRE_MOVE_POS;
		}
		if (_st_cmd == CMD_CAP_PRINT_POS)
		{
			_st_cmd = FALSE;
			_lift_pos = positions.printing.pos_lift;
			_clean_pos = positions.printing.pos_cln;
			//Error(LOG, 0, "CMD_CAP_PRINT_POS: print_height=%d, pos_list=%d", RX_Config.stepper.print_height, _lift_pos);
			if (RX_TestTableStatus.info.printhead_en == FALSE) break; // do not execute printpos when printhead_en is off
			if ((abs(RX_TestTableStatus.posZ - _lift_pos) <= 10) && (abs(RX_ClnStatus.posZ - _clean_pos) <= 10)) break;
			printf("start printing \n");
			if ((RX_TestTableStatus.info.ref_done == TRUE) 
				&& (_clean_pos == 0) 
				&& (RX_ClnStatus.info.z_in_ref == TRUE) 
				&& (RX_TestTableStatus.info.cln_in_capping == FALSE)) 
			{
				sok_send_2(_step_socket[0], _step_ipaddr[0], CMD_CAP_PRINT_POS, sizeof(_lift_pos), &_lift_pos); // Lift move to position
				_capCurrentState = ST_LIFT_MOVE_POS;
			}
			else
			{
				sok_send_2(_step_socket[0], _step_ipaddr[0], CMD_CAP_PRINT_POS, sizeof(_reset_pos), &_reset_pos); // Lift up to zero position
				_capCurrentState = ST_PRE_MOVE_POS;
			}
		}
		if (_st_cmd == CMD_CAP_UP_POS)
		{
			_st_cmd = FALSE;
			_lift_pos = positions.reset.pos_lift;
			_clean_pos = positions.reset.pos_cln;
			if ((abs(RX_TestTableStatus.posZ - _lift_pos) <= 10) && (abs(RX_ClnStatus.posZ - _clean_pos) <= 10)) break;
			printf("web up to reference \n");
			sok_send_2(_step_socket[0], _step_ipaddr[0], CMD_CAP_PRINT_POS, sizeof(_reset_pos), &_reset_pos); // Lift up to zero position
			_capCurrentState = ST_PRE_MOVE_POS;
		}
		if (_st_cmd == CMD_CLN_WIPE)
		{
			_st_cmd = FALSE;
			_lift_pos = positions.wiping.pos_lift;
			_clean_pos = positions.wiping.pos_cln;
			//if (RX_TestTableStatus.info.printhead_en == FALSE) break;							// do not execute wipe when printhead_en is off
			printf("start wiping \n");
			sok_send_2(_step_socket[0], _step_ipaddr[0], CMD_CAP_PRINT_POS, sizeof(_reset_pos), &_reset_pos);	// Lift up to zero position
			_capCurrentState = ST_PRE_MOVE_POS;
		}
		if (_st_cmd == CMD_CLN_SCREW_0_POS)
		{
			_st_cmd = FALSE;
			if (RX_TestTableStatus.info.printhead_en == FALSE) break;			// do not execute screw when printhead_en is off
			printf("start screwing of screw nr %d \n", _screw_nr);
			if (_screw_nr > 8*10){break;}										// limit the number of heads to maximum 10 heads in a row
			int screw_dist = (_screw_nr % 2) * POS_HEADSCREW_DELTA;
			int head_dist = (_screw_nr / 2) * POS_HEAD_DELTA;					// count from zero
			_lift_pos = positions.screw0.pos_lift;
			_clean_pos = positions.screw0.pos_cln - screw_dist - head_dist;		// break;
			if ((abs(RX_TestTableStatus.posZ - _lift_pos) <= 10) && (abs(RX_ClnStatus.posZ - _clean_pos) <= 10))
			{
				sok_send_2(_step_socket[1], _step_ipaddr[1], CMD_CLN_SCREW_REF, 0, NULL);		// screw at same position
				_capCurrentState = ST_SCREW_REF;
			}
			else
			{
				sok_send_2(_step_socket[0], _step_ipaddr[0], CMD_CAP_PRINT_POS, sizeof(_reset_pos), &_reset_pos); // Lift up to zero position -- step_lift
				_capCurrentState = ST_PRE_MOVE_POS;
			}
		}
		break;
		
	case ST_PRE_MOVE_POS:
		if (_lift_move_tgl != RX_TestTableStatus.info.move_tgl)
		{
			if ((abs(RX_TestTableStatus.posZ - _reset_pos) <= 10) && (RX_TestTableStatus.info.ref_done == TRUE)) // (RX_TestTableStatus.info.z_in_ref == TRUE)
			{
				printf("state %d PREMOVE done\n", _capCurrentState);
				_lift_move_tgl = RX_TestTableStatus.info.move_tgl;
				_cln_move_tgl = RX_ClnStatus.info.move_tgl;
				_capCurrentState = ST_CLEAN_MOVE_POS;
				sok_send_2(_step_socket[1], _step_ipaddr[1], CMD_CLN_MOVE_POS, sizeof(_clean_pos), &_clean_pos); // Cleaning move to position
			}
			else
			{
				_st_cmd = _st_cmd_stored;
				_capCurrentState = ST_INIT;
				Error(LOG, 0, "MAIN: State_Command 0x%08x: ST_PRE_MOVE_POS retry reference", _st_cmd_stored);
			}
		}
		break;
		
	case ST_CLEAN_PRE_POS: 
		if (_cln_move_tgl != RX_ClnStatus.info.move_tgl)
		{
			if (RX_ClnStatus.info.move_ok == TRUE)
			{
				_lift_move_tgl = RX_TestTableStatus.info.move_tgl;
				_cln_move_tgl = RX_ClnStatus.info.move_tgl;
				_capCurrentState = ST_CLEAN_MOVE_POS;
				sok_send_2(_step_socket[1], _step_ipaddr[1], CMD_CLN_MOVE_POS, sizeof(_clean_pos), &_clean_pos); // Cleaning move to position
			}
			else
			{
				ErrorFlag(ERR_CONT, &_ErrorFlags, ERR_2, 0, "MAIN: State_Command 0x%08x: ST_CLEAN_PRE_POS wait on CLN for move_ok", _st_cmd_stored);
			}
		}
		break;
		
	case ST_CLEAN_MOVE_POS:
		if (_cln_move_tgl != RX_ClnStatus.info.move_tgl)
		{
			if (RX_ClnStatus.info.move_ok == TRUE)
			{
				if (_st_cmd_stored == CMD_CAP_CAPPING_POS)
				{
					if ((RX_ClnStatus.info.z_in_cap == TRUE) && (RX_TestTableStatus.info.cln_in_capping == TRUE)) // cln_in_capping, better: z_in_cap
					{
						printf("state %d CLEAN_MOVE done\n", _capCurrentState);
						_lift_move_tgl = RX_TestTableStatus.info.move_tgl;
						_cln_move_tgl = RX_ClnStatus.info.move_tgl;
						_capCurrentState = ST_LIFT_MOVE_POS;
						printf("state %d do lift to capping now\n", _capCurrentState);
						sok_send_2(_step_socket[0], _step_ipaddr[0], CMD_CAP_PRINT_POS, sizeof(_lift_pos), &_lift_pos); // Lift move to position
					}
					else
					{
						ErrorFlag(ERR_CONT, &_ErrorFlags, ERR_3, 0, "MAIN: State_Command 0x%08x: wait on CLN to be in Capping", _st_cmd_stored);
					}

				}
				else if (_st_cmd_stored == CMD_CAP_PRINT_POS)
				{
					if ((RX_ClnStatus.info.z_in_ref == TRUE) && (RX_TestTableStatus.info.cln_in_capping == FALSE)) // cln_in_stored, better: z_in_ref
					{
						printf("state %d CLEAN_MOVE done\n", _capCurrentState);
						_lift_move_tgl = RX_TestTableStatus.info.move_tgl;
						_cln_move_tgl = RX_ClnStatus.info.move_tgl;
						_capCurrentState = ST_LIFT_MOVE_POS;
						printf("state %d do lift to printing now\n", _capCurrentState);
						sok_send_2(_step_socket[0], _step_ipaddr[0], CMD_CAP_PRINT_POS, sizeof(_lift_pos), &_lift_pos); // Lift move to position
					}
					else
					{
						ErrorFlag(ERR_CONT, &_ErrorFlags, ERR_4, 0, "MAIN: State_Command 0x%08x: wait on CLN to be in Ref", _st_cmd_stored);
					}

				}
				else
				{
					printf("state %d CLEAN_MOVE done\n", _capCurrentState);
					_lift_move_tgl = RX_TestTableStatus.info.move_tgl;
					_cln_move_tgl = RX_ClnStatus.info.move_tgl;
					_capCurrentState = ST_LIFT_MOVE_POS;
					printf("state %d do lift move now\n", _capCurrentState);
					sok_send_2(_step_socket[0], _step_ipaddr[0], CMD_CAP_PRINT_POS, sizeof(_lift_pos), &_lift_pos); // Lift move to position
				}
			}
			else
			{
				ErrorFlag(ERR_CONT, &_ErrorFlags, ERR_5, 0, "MAIN: State_Command 0x%08x: ST_CLEAN_MOVE_POS wait on CLN for move_ok", _st_cmd_stored);
			}
		}
		break;
		
	case ST_LIFT_MOVE_POS:
		if (_lift_move_tgl != RX_TestTableStatus.info.move_tgl)
		{
			printf("state %d ST_LIFT_MOVE_POS toggle ok\n", _capCurrentState);
			if (RX_TestTableStatus.info.move_ok == TRUE)
			{
				printf("state %d ST_LIFT_MOVE_POS move ok\n", _capCurrentState);
				if (_st_cmd_stored == CMD_CAP_CAPPING_POS)
				{
					printf("state %d ST_LIFT_MOVE_POS capping ok\n", _capCurrentState);
					_lift_move_tgl = RX_TestTableStatus.info.move_tgl;
					_cln_move_tgl = RX_ClnStatus.info.move_tgl;
					// execute capping ?
					// sok_send_2(_step_socket[step_lift], CMD_CAP_CAPPING_POS, 0, NULL); // Lift execute capping
					//_capCurrentState = ST_LIFT_CAPPING;
					_capCurrentState = ST_IDLE;
				}
				else if(_st_cmd_stored == CMD_CLN_WIPE)
				{
					printf("state %d ST_LIFT_MOVE_POS wiping ok\n", _capCurrentState);
					_lift_move_tgl = RX_TestTableStatus.info.move_tgl;
					_cln_move_tgl = RX_ClnStatus.info.move_tgl;
					// execute cleaning station CMD_CLN_WIPE
					// sok_send_2(_step_socket[1], _step_ipaddr[1], CMD_CLN_MOVE_POS, sizeof(_clean_wipe_pos), &_clean_wipe_pos); // Cleaning move to position
					sok_send_2(_step_socket[1], _step_ipaddr[1], CMD_CLN_WIPE, sizeof(_clean_wipe_pos), &_clean_wipe_pos); // Cleaning move to position, slowly
					_capCurrentState = ST_WIPE;
				}
				else if(_st_cmd_stored == CMD_CLN_SCREW_0_POS)
				{
					printf("state %d ST_LIFT_MOVE_POS screwing ok\n", _capCurrentState);
					_lift_move_tgl = RX_TestTableStatus.info.move_tgl;
					_cln_move_tgl = RX_ClnStatus.info.move_tgl;
					sok_send_2(_step_socket[1], _step_ipaddr[1], CMD_CLN_SCREW_REF, 0, NULL); // execute ref of screws
					_capCurrentState = ST_SCREW_REF;
				}
				else
				{
					printf("state %d ST_LIFT_MOVE_POS else ok\n", _capCurrentState);
					_lift_move_tgl = RX_TestTableStatus.info.move_tgl;
					_cln_move_tgl = RX_ClnStatus.info.move_tgl;
					_capCurrentState = ST_IDLE;
				}
			}
			else
			{
				if (RX_TestTableStatus.info.printhead_en == FALSE)
				{
					printf("State_Command 0x%08x: LIFT_MOVE_POS skipped, printhead_down_en is off\n", _capCurrentState);
					_capCurrentState = ST_IDLE;
				}
				else
				{
					ErrorFlag(ERR_CONT, &_ErrorFlags, ERR_6, 0, "MAIN: State_Command 0x%08x: LIFT stopped by mechanical stopp", _st_cmd_stored);
				}
			}
		}
		break;
		
	case ST_LIFT_CAPPING :
			if (RX_TestTableStatus.info.z_in_cap == TRUE)
			{
				_lift_move_tgl = RX_TestTableStatus.info.move_tgl;
				_cln_move_tgl = RX_ClnStatus.info.move_tgl;
				_capCurrentState = ST_IDLE;
			}
		break;
		
	case ST_WIPE :
		if (_cln_move_tgl != RX_ClnStatus.info.move_tgl)
		{
			if (RX_ClnStatus.info.move_ok == TRUE)
			{
				_lift_move_tgl = RX_TestTableStatus.info.move_tgl;
				_cln_move_tgl = RX_ClnStatus.info.move_tgl;
				_capCurrentState = ST_IDLE;
				_st_cmd = CMD_CAP_UP_POS; // move up at the end
			}
			else
			{
				ErrorFlag(ERR_CONT, &_ErrorFlags, ERR_7, 0, "MAIN: State_Command 0x%08x: ST_WIPE wait on CLN for move_ok", _st_cmd_stored);
			}
		}
		break;
		
	case ST_SCREW_REF :
		if (_cln_move_tgl != RX_ClnStatus.info.move_tgl)
		{
			if (RX_ClnStatus.info.screw_in_ref == TRUE)
			{
				printf("state %d ST_SCREW_REF screwing ref ok\n", _capCurrentState);
				printf("selected cluster: nr  %d \n", _screw_cluster);
				_lift_move_tgl = RX_TestTableStatus.info.move_tgl;
				_cln_move_tgl = RX_ClnStatus.info.move_tgl;
				switch (_screw_cluster) {
				case 0: sok_send_2(_step_socket[1], _step_ipaddr[1], CMD_CLN_SCREW_0_POS, sizeof(_screw_rot), &_screw_rot); break; // turn screw
				case 1: sok_send_2(_step_socket[1], _step_ipaddr[1], CMD_CLN_SCREW_1_POS, sizeof(_screw_rot), &_screw_rot); break; // turn screw
				case 2: sok_send_2(_step_socket[1], _step_ipaddr[1], CMD_CLN_SCREW_2_POS, sizeof(_screw_rot), &_screw_rot); break; // turn screw
				case 3: sok_send_2(_step_socket[1], _step_ipaddr[1], CMD_CLN_SCREW_3_POS, sizeof(_screw_rot), &_screw_rot); break; // turn screw
				}
				_capCurrentState = ST_SCREW;
			}
			else
			{
				ErrorFlag(ERR_CONT, &_ErrorFlags, ERR_8, 0, "MAIN: State_Command 0x%08x: ST_SCREW_REF could not connect to all 4 screws, please align global head stitches", _st_cmd_stored);
			}
		}
		break;
		
	case ST_SCREW :
		if (_cln_move_tgl != RX_ClnStatus.info.move_tgl)
		{
			if (RX_ClnStatus.info.screw_done == TRUE)
			{
				printf("state %d ST_SCREW_REF screwing done\n", _capCurrentState);
				_lift_move_tgl = RX_TestTableStatus.info.move_tgl;
				_cln_move_tgl = RX_ClnStatus.info.move_tgl;
				_capCurrentState = ST_IDLE;
			}
			else
			{
				ErrorFlag(ERR_CONT, &_ErrorFlags, ERR_9, 0, "MAIN: State_Command 0x%08x: ST_SCREW wait on CLN to execute screwing", _st_cmd_stored);
			}
		}
		break;
			
	}
	
}

//--- _do_turn_screw --------------------------------------------------
static int _do_turn_screw(SHeadAdjustment *pHeadAdjust)
{
	if (_capCurrentState == ST_IDLE)
	{
		if (pHeadAdjust->angle == 0)
		{
			_screw_nr = (pHeadAdjust->headNo-1) * 2 + 1; // head nr von 0-3 ?? welche Schraube ist stitch, welche angle ? zuerst angle, dann stitch
			_screw_rot = (int)(pHeadAdjust->stitch * 1000 * SCREW_STITCH_REV_PER_UM);
			_screw_cluster = pHeadAdjust->inkSupplyNo; // 0-3 ??
		}
		else
		{
			_screw_nr = (pHeadAdjust->headNo-1) * 2 + 0; // head nr von 0-3 ?? welche Schraube ist stitch, welche angle ? zuerst angle, dann stitch
			_screw_rot = (int)(pHeadAdjust->angle * 1000 * SCREW_JAW_REV_PER_UM);
			_screw_cluster = pHeadAdjust->inkSupplyNo; // 0-3 ??
		}
		_st_cmd = CMD_CLN_SCREW_0_POS;
		return TRUE;
	}
	else
	{
		Error(WARN, 0, "MAIN: State_Command 0x%08x busy", _st_cmd_stored);
		return FALSE;
	}
}

////--- _step_set_config -------------------------------------------------
//static void _step_set_config(int no)
//{
//	SStepperCfg cfg;
//
//	memcpy(&cfg, &RX_Config.stepper, sizeof(cfg));
//	cfg.printerType = RX_Config.printer.type;
//	cfg.boardNo = no;
//
//	stepc_init(no, _step_socket[no]);
//
//	printf("Value ref_height	= %d \n", cfg.ref_height);
//	printf("Value print_height	= %d \n", cfg.print_height);
//	printf("Value wipe_height	= %d \n", cfg.wipe_height);
//	printf("Value cap_height	= %d \n", cfg.cap_height);
//	printf("Value cap_pos		= %d \n", cfg.cap_pos);
//
//	sok_send_2(_step_socket[no], _step_ipaddr[no], CMD_STEPPER_CFG, sizeof(cfg), &cfg);
//}

// RX_Config.stepper.cap_height

//--- stepc_to_print_pos --------------------------------
int	 stepc_to_print_pos(void)
{
	_st_cmd = CMD_CAP_PRINT_POS;
	_do_tt_status(&RX_TestTableStatus);
	return REPLY_OK;									
}

//--- stepc_to_up_pos --------------------------------
int	 stepc_to_up_pos(void)
{
	_st_cmd = CMD_CAP_UP_POS;
	_do_tt_status(&RX_TestTableStatus);
	return REPLY_OK;									
}

//--- stepc_abort_printing --------------------
int stepc_abort_printing(void)
{
	if (RX_TestTableStatus.info.z_in_print) stepc_to_up_pos();
	return REPLY_OK;													
}
