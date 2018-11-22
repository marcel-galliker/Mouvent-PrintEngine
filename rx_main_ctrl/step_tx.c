// ****************************************************************************
//
//	step_tx.c		Stepper Control for CLEAF project
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
#include "tcp_ip.h"
#include "network.h"
#include "ctrl_msg.h"
#include "print_ctrl.h"
#include "plc_ctrl.h"
#include "enc_ctrl.h"
#include "gui_svr.h"
#include "step_ctrl.h"
#include "ctrl_svr.h"
#include "step_tx.h"

#define trace FALSE

static RX_SOCKET *_step_socket[2]={NULL, NULL};
static UINT32	  _step_ipaddr[2]={0,0};

static int _lift_pos = 0;
static int _clean_pos = 0;
static int _clean_wipe_pos = 0;
static int _clean_wipe_head_nr = 0;
static int _slide_enable = 0;
//static int _flag_finish_purge = 0;
static int _flag_wipe_done = 0;
static int _flag_purge_done = 0;
//static int _boardNo = 0; 
//static int _headNo = 0;
static int _reset_pos = 7000; // 5000; // not negative ! // position to which he goes up after every job
static UINT32 _st_cmd;
static UINT32 _st_cmd_stored;
static enum cln_state_code _capCurrentState = ST_INIT;
static int _lift_move_tgl = FALSE;
static int _cln_move_tgl = FALSE;
static int _head_safety_edge = FALSE;

//--- dimensions -----------------------
static int printer_height	=  10000; // 1000 um
static int wipe_height		=   0; // -1000 um // - 3000 um
static int capping_height	=   1000; // -9000 / -9300 um
static int belt_ref_height		= 16000; // 16000 um
static int capping_pos		= 509000; // 509000 um

//--- printhead ------------------------
#define POS_HEADSCREW_DELTA	16400 // delta between the two screws on the same head in mm
#define POS_HEAD_DELTA		48924 // 48854+500/7 about +70 // 43140 // statt 125 85 -> delta 40mm von screw1 to screw 7
//#define POS_CAP2SCREW_DELTA	25628

//--- position lift --------------------
#define LIFT_POS_PRINTING			printer_height // DELAT VALUE // RX_StepperCfg.print_height
#define LIFT_POS_START_WIPE			wipe_height // DELAT VALUE // RX_StepperCfg.wipe_height
#define LIFT_POS_STORED				1000  // DELAT VALUE // "UP" position above belt
//#define LIFT_POS_UP				1000
#define LIFT_POS_PLACE_PRINTHEADS	(belt_ref_height-capping_height)  // DELAT VALUE //LIFT_POS_START_WIPE // capping position
#define LIFT_POS_PURGE				1000  // DELAT VALUE // - 3000 um Purge height
#define LIFT_POS_SCREW				-6000 // -5000 // -2000 // DELAT VALUE //-7000 // maximal value=> -7100   // DELAT VALUE // TODO: adjust distance for screwing from wiping pos (+1000)
#define LIFT_CAP_SECURITY_DELTA	    1000


//--- position cleaning ------------------
#define CLN_POS_PRINTHEADS		capping_pos // RX_StepperCfg.capping_pos // capping position
#define CLN_POS_STORED			23000 // Ref 0
#define CLN_DIST_WIPE_ONE_HEAD  49350 // 48150+1000+200 // 53500 // 43140 // -> 4cm bis 9.7   41.5 bis 47.2 // 43140+5mm=48150 // ziel 4-44cm // 43 ist 40 soll
#define CLN_DIST_WIPEBLADE_FLEX  8000 // ca 10% of CLN_DIST_WIPE_ONE_HEAD plus 5mm
#define CLN_POS_START_WIPE		509000 // + CLN_DIST_WIPEBLADE_FLEX/2 // CLN_POS_PRINTHEADS //  - 20000 // Wipe start // TODO: adjust distance form capping to wiping (-1000)
#define CLN_POS_END_WIPE        (81000 - CLN_DIST_WIPEBLADE_FLEX)
#define POS_SCREW_0				85500 // 85300 // 84800 // 85800 // 83800 // 28800+55000 // - POS_CAP2SCREW_DELTA // 28800 + 23000

//85800 + 16400 = 102200
//85800 + 16400 + 7 * 48854 = 444178
	
	
// unused parameters
#define POS_SCREW_1			(POS_SCREW_0+POS_HEADSCREW_DELTA)
#define POS_SCREW_2			(POS_SCREW_0+POS_HEAD_DELTA)
#define POS_SCREW_3			(POS_SCREW_2+POS_HEADSCREW_DELTA)
#define POS_SCREW_4			(POS_SCREW_2+POS_HEAD_DELTA)
#define POS_SCREW_5			(POS_SCREW_4+POS_HEADSCREW_DELTA)
#define POS_SCREW_6			(POS_SCREW_4+POS_HEAD_DELTA)
#define POS_SCREW_7			(POS_SCREW_6+POS_HEADSCREW_DELTA) // total -171448 

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
	SPosItem purge;
} SPosName;


//--- prorotypes -------------------------------------------------
static void _do_tt_status(STestTableStat *pStatus);
//static void _do_cln_status(STestTableStat *pStatus);
static int  _do_turn_screw(SHeadAdjustment *pHeadAdjust); // or _do_adjust_head


//--- steptx_init ---------------------------------------------------
void steptx_init(int stepperNo, RX_SOCKET *psocket, UINT32 ipaddr)
{
	_step_socket[stepperNo] = psocket;
	_step_ipaddr[stepperNo] = ipaddr;
	_ErrorFlags = 0;
}

//--- steptx_error_reset ----------------------------------------------------
void steptx_error_reset(void)
{
	_ErrorFlags = 0;
}

//--- steptx_handle_gui_msg------------------------------------------------------------------
int	 steptx_handle_gui_msg(RX_SOCKET socket, UINT32 cmd, void *data, int dataLen)
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
		//if (_capCurrentState == ST_INIT || _capCurrentState == ST_IDLE)
		//{
		_st_cmd = cmd;
		//}
		printf("CMD 0x%08x REFERENCE executed\n", cmd);
		break;

	case CMD_CAP_UP_POS:
		if (_capCurrentState == ST_INIT || _capCurrentState == ST_IDLE)
		{
		_st_cmd = cmd;
		}
		// sok_send_2(_step_socket[0], CMD_CAP_PRINT_POS, sizeof(_reset_pos), &_reset_pos); // Lift up to zero position
		// sok_send_2(_step_socket[0], cmd, 0, NULL); execute up to savety
		printf("CMD 0x%08x Move to Zero executed\n", cmd);
		
		break;

	case CMD_CAP_CAPPING_POS:
		if (_capCurrentState == ST_INIT || _capCurrentState == ST_IDLE)
		{
		_st_cmd = cmd;
		}
		printf("CMD 0x%08x CAPPING executed\n", cmd);
		break;
		
	case CMD_CAP_PRINT_POS:
		if (_capCurrentState == ST_INIT || _capCurrentState == ST_IDLE)
		{
		_st_cmd = cmd;
		}
		printf("CMD 0x%08x PRINT_POS executed\n", cmd);
				break;
		
	case CMD_CLN_WIPE:
		if (_capCurrentState == ST_INIT || _capCurrentState == ST_IDLE)
		{
		_st_cmd = cmd;
		}
		printf("CMD 0x%08x CMD_CLN_WIPE executed\n", cmd);
		break;
	
	//--- cleaning ---------------------------------------------------------
	case CMD_CLN_ADJUST:		_do_turn_screw((SHeadAdjustment*)data);
								break;

	//--- Unknown Case ---------------------------------------------------------
	default:						Error(ERR_CONT, 0, "Command 0x%08x not implemented", cmd); break;
	}
	
	return REPLY_OK;
}

//--- steptx_handle_status ----------------------------------------------------------------------
int steptx_handle_status(int stepperNo, STestTableStat *pStatus)
{
	switch(stepperNo)
	{
	case 0: _do_tt_status(pStatus);		break;
	//case 1: _do_cln_status(pStatus);	break;
	}	
	return REPLY_OK;
}

//--- _do_cln_status --------------------------------------------------
//static void _do_cln_status(STestTableStat *pStatus)
//{
//	memcpy(&RX_ClnStatus, pStatus, sizeof(RX_ClnStatus));	
//}

//--- _do_tt_status ----------------------------------------------
static void _do_tt_status(STestTableStat *pStatus)
{
	int last_pos = RX_TestTableStatus.info.z_in_print;
	memcpy(&RX_TestTableStatus, pStatus, sizeof(RX_TestTableStatus));
	//RX_TestTableStatus.alive[1] = RX_ClnStatus.alive[0];
	RX_TestTableStatus.info.uv_on    = enc_is_uv_on();
	RX_TestTableStatus.info.uv_ready = enc_is_uv_ready();
	if (RX_Config.printer.type==printer_TX801 || RX_Config.printer.type==printer_TX802) RX_TestTableStatus.info.x_in_cap = plc_in_cap_pos();

	RX_TestTableStatus.state = _capCurrentState;
	
	if (RX_TestTableStatus.info.z_in_print!=last_pos) enc_enable_printing(RX_TestTableStatus.info.z_in_print);

	// Init position matrix
	printer_height = RX_Config.stepper.print_height; // um // 151000 steps *1000000 /1600000
	wipe_height = RX_Config.stepper.wipe_height; // 20000 ; capping 28000; // um // 45000 steps *1000000 /1600000
	capping_pos = RX_Config.stepper.cap_pos; // um // 69801 steps *1000000 /139581.3
	capping_height = RX_Config.stepper.cap_height;
	belt_ref_height = RX_Config.stepper.ref_height;
	
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
		{ CLN_POS_START_WIPE, LIFT_POS_PURGE}				// 12: purge
		//}
	};
		
	gui_send_msg_2(0, REP_TT_STATUS, sizeof(RX_TestTableStatus), &RX_TestTableStatus);
	
	if (RX_TestTableStatus.info.moving == FALSE) // && last command ...
	{
	}
	
	// --- Safety up&down for Splice ---
//	if (RX_TestTableStatus.info.head_in_safety == TRUE && _head_safety_edge == FALSE) //
//	{
//		if (RX_ClnStatus.info.ref_done == TRUE) // do not go to Idle if not once referenced
//		{
//			_capCurrentState = ST_IDLE; 
//		}
//	}
//	else if (RX_TestTableStatus.info.head_in_safety == TRUE 
//		&& RX_TestTableStatus.info.printhead_en == TRUE 
//		&& _capCurrentState == ST_IDLE)
//	{
//		if (RX_ClnStatus.info.ref_done == TRUE) // do not go down if not once referenced
//		{
//			_st_cmd = CMD_CAP_PRINT_POS;
//		}
//	}
//	_head_safety_edge = RX_TestTableStatus.info.head_in_safety;
	
	// --- Stop State Machine ---
	if (_st_cmd == CMD_CAP_STOP)
	{
		sok_send_2(_step_socket[0], _step_ipaddr[0], CMD_CAP_STOP, 0, NULL);
		_st_cmd = FALSE;
		if (RX_TestTableStatus.info.ref_done == TRUE) // do not go to Idle if not once referenced
		{
			_capCurrentState = ST_IDLE; 
			printf("Stop State Machine and set to Idle\n");
		}
		else
		{
			_capCurrentState = ST_INIT; 
			printf("Stop State Machine and set to Init\n");
		}
	}
	
	// --- Reference State Machine ---
	if (_st_cmd == CMD_CAP_REFERENCE)
	{
		sok_send_2(_step_socket[0], _step_ipaddr[0], CMD_CAP_STOP, 0, NULL);
		sok_send_2(_step_socket[1], _step_ipaddr[1], CMD_CLN_STOP, 0, NULL);
		_capCurrentState = ST_INIT;
		printf("Reference State Machine \n");
	}
	
	// --- State Machine CLN & LIFT ctrl ---
	// First: must reference lift and cleaningstation (cln) to get to idle
	// Idle has 5 Options: capping: move lift to capping
	//                     printing: move lift to printing
	//                     up: move lift up to reference
	//                     wipe: move lift up, move cleaning blade to position, move down to cleaning height, wipe
	//                     screw: move lift up, move screwer to position, move down to screwing height, screw
	switch (_capCurrentState) {
		
	case ST_INIT:
		if (_st_cmd != FALSE)
		{
				_flag_wipe_done = FALSE;
				_flag_purge_done = FALSE;
				if (trace) Error(LOG, 0, "Flag: WIPE DONE disable");
				if (trace) Error(LOG, 0, "Flag: PURGE DONE disable");
			_lift_move_tgl = RX_TestTableStatus.info.move_tgl;
		if (_st_cmd == CMD_CAP_REFERENCE)
		{
			_st_cmd = FALSE;
			}
			sok_send_2(_step_socket[0], _step_ipaddr[0], CMD_CAP_REFERENCE, 0, NULL); // ref Lift
			printf("state %d REFERENCE lift\n", _capCurrentState);
			_capCurrentState = ST_REF_LIFT;
		}
		break;
		
	case ST_REF_LIFT:
		if (RX_TestTableStatus.info.z_in_ref == TRUE)
		{
			_slide_enable = TRUE;
			if (trace) Error(LOG, 0, "Flag: SLIDE enable");
			//_lift_move_tgl = RX_TestTableStatus.info.move_tgl;
			//sok_send_2(_step_socket[0], _step_ipaddr[0], CMD_CAP_MECH_ADJUST, 0, NULL); // Lift ref to mech
			_capCurrentState = ST_REF_CLEAN;
		}
		break;
		
	case ST_REF_CLEAN:
		if (_lift_move_tgl != RX_TestTableStatus.info.move_tgl)
		{
				_capCurrentState = ST_IDLE;
			}
		break;
		
	case ST_IDLE:
		if (_st_cmd != FALSE)
		{
			_flag_wipe_done = FALSE;
			_flag_purge_done = FALSE;
			if (trace) Error(LOG, 0, "Flag: WIPE DONE disable");
			if (trace) Error(LOG, 0, "Flag: PURGE DONE disable");
		}
		_lift_move_tgl = RX_TestTableStatus.info.move_tgl;
		_st_cmd_stored = _st_cmd;
		if (_st_cmd == CMD_CAP_CAPPING_POS)
		{
			_slide_enable = FALSE;
			if (trace) Error(LOG, 0, "Flag: SLIDE disdable");
			_st_cmd = FALSE;
			_lift_pos = positions.capping.pos_lift;
			_clean_pos = positions.capping.pos_cln;
			if ((abs(RX_TestTableStatus.posZ - _lift_pos) <= 10) && (abs(RX_ClnStatus.posZ - _clean_pos) <= 10)) break;  // check for Head position
			printf("start capping \n");
			sok_send_2(_step_socket[0], _step_ipaddr[0], CMD_CAP_PRINT_POS, sizeof(_reset_pos), &_reset_pos); // Lift up to zero position
			_capCurrentState = ST_PRE_MOVE_POS;
		}
		if (_st_cmd == CMD_CAP_PRINT_POS)
		{
			_slide_enable = TRUE;
			if (trace) Error(LOG, 0, "Flag: SLIDE enable");
			_st_cmd = FALSE;
			_lift_pos = positions.printing.pos_lift;
			_clean_pos = positions.printing.pos_cln;
			if ((abs(RX_TestTableStatus.posZ - _lift_pos) <= 10) && (abs(RX_ClnStatus.posZ - _clean_pos) <= 10)) break;
			printf("start printing \n");
			
			if (RX_TestTableStatus.info.ref_done == TRUE)
			{
				sok_send_2(_step_socket[0], _step_ipaddr[0], CMD_CAP_PRINT_POS, sizeof(_lift_pos), &_lift_pos); // Lift move to position
				_capCurrentState = ST_LIFT_MOVE_POS;
			}
			else
			{
				ErrorFlag(WARN, &_ErrorFlags, ERR_0, 0, "MAIN: State_Command 0x%08x: Reference first Lift", CMD_CAP_REFERENCE);
			}
		}
		if (_st_cmd == CMD_CAP_UP_POS)
		{
			_slide_enable = TRUE;
			if (trace) Error(LOG, 0, "Flag: SLIDE enable");
			_st_cmd = FALSE;
			_lift_pos = positions.reset.pos_lift;
			_clean_pos = positions.reset.pos_cln;
			if ((abs(RX_TestTableStatus.posZ - _lift_pos) <= 10) ) break;
			printf("web up to reference \n");
			sok_send_2(_step_socket[0], _step_ipaddr[0], CMD_CAP_UP_POS, 0, NULL); // Lift up to zero position
			_capCurrentState = ST_LIFT_UP;
		}
		if (_st_cmd == CMD_CLN_WIPE)
		{
			_slide_enable = TRUE;
			if (trace) Error(LOG, 0, "Flag: SLIDE enable");
			_st_cmd = FALSE;
			_lift_pos = positions.wiping.pos_lift;
			_clean_pos = positions.wiping.pos_cln - CLN_DIST_WIPE_ONE_HEAD * _clean_wipe_head_nr; //x;
			_clean_wipe_pos = positions.wiping.pos_cln - CLN_DIST_WIPE_ONE_HEAD * (_clean_wipe_head_nr + 1) - CLN_DIST_WIPEBLADE_FLEX; //x;
			if (trace) Error(LOG, 0, "ctrl_start_wipe _clean_pos=%d", _clean_pos);
			if (trace) Error(LOG, 0, "ctrl_start_wipe _clean_wipe_pos=%d", _clean_wipe_pos);
			if ((_clean_wipe_pos < CLN_POS_END_WIPE) || (_clean_pos < CLN_POS_END_WIPE)) break; // printf("desired wipe wipes longer than POS_END_WIPE, cancel wiping"); 
			printf("start wiping \n");
			sok_send_2(_step_socket[0], _step_ipaddr[0], CMD_CAP_PRINT_POS, sizeof(_reset_pos), &_reset_pos);	// Lift up to zero position
			//ctrl_send_head_fluidCtrlMode(_clean_wipe_head_nr, ctrl_purge_step3, TRUE); // set fluid to print
			_capCurrentState = ST_PRE_MOVE_POS;
		}
		if (_st_cmd == CMD_CAP_PURGE)
		{
			_slide_enable = FALSE;
			if (trace) Error(LOG, 0, "Flag: SLIDE disable");
			_st_cmd = FALSE;
			_lift_pos = positions.purge.pos_lift;
			//_clean_pos = positions.purge.pos_cln;
			_clean_pos = positions.purge.pos_cln - CLN_DIST_WIPE_ONE_HEAD * _clean_wipe_head_nr;
			//_clean_pos = positions.purge.pos_cln - CLN_DIST_WIPE_ONE_HEAD * 0;
			printf("start purge \n");
			if ((abs(RX_TestTableStatus.posZ - _lift_pos) <= 10) && (plc_in_purge_pos() == TRUE)) // && (abs(RX_ClnStatus.posZ - _clean_pos) <= 10)
			{
					_capCurrentState = ST_PURGE;
				}
				else
				{
				sok_send_2(_step_socket[0], _step_ipaddr[0], CMD_CAP_PRINT_POS, sizeof(_reset_pos), &_reset_pos);	// Lift up to zero position
				_capCurrentState = ST_PRE_MOVE_POS;
			}
		}
		break;
		
	case ST_PRE_MOVE_POS:
		if (_lift_move_tgl != RX_TestTableStatus.info.move_tgl)
		{
		if (RX_TestTableStatus.info.z_in_print == TRUE) // RX_TestTableStatus.info.z_in_ref == TRUE)  || RX_TestTableStatus.info.z_in_print == TRUE)
			{
				printf("state %d PREMOVE done\n", _capCurrentState);
				_capCurrentState = ST_SLIDE_MOVE;
				
				if (_st_cmd_stored == CMD_CAP_CAPPING_POS)
				{
					plc_stop_printing(); // move printhead over capping position
				}
				else if (_st_cmd_stored == CMD_CLN_WIPE)
				{
					plc_to_wipe_pos(); // move printhead over cleaning position
			}
				else if (_st_cmd_stored == CMD_CAP_PURGE)
				{
					plc_to_purge_pos(); // move printhead over purge position
				}
				else if (_st_cmd_stored == CMD_CLN_SCREW_0_POS)
			{
					plc_to_wipe_pos(); // move printhead over cleaning position
				}
				
			}
		}
		break;
		
	case ST_SLIDE_MOVE:
		if (_st_cmd_stored == CMD_CAP_CAPPING_POS)
		{
			if (plc_in_cap_pos() == TRUE) // slide in capping position
			{
				_capCurrentState = ST_CLEAN_MOVE_POS;
			}
		}
		else if (_st_cmd_stored == CMD_CLN_WIPE)
		{
			if (plc_in_wipe_pos() == TRUE) // slide in cleaning position
			{
				_clean_wipe_head_nr = 0;
				_flag_wipe_done = TRUE;
				if (trace) Error(LOG, 0, "Flag: WIPE DONE enable");
				sok_send_2(_step_socket[0], _step_ipaddr[0], CMD_CAP_PRINT_POS, sizeof(_reset_pos), &_reset_pos);
				_lift_move_tgl = RX_TestTableStatus.info.move_tgl;
				_capCurrentState = ST_LAST_LIFT;
			}
		}
		else if (_st_cmd_stored == CMD_CAP_PURGE)
		{
			if (plc_in_purge_pos() == TRUE) // slide in cleaning position
			{
				printf("state %d in purge position, move lift now down \n", _capCurrentState);
				//sok_send_2(_step_socket[0], CMD_CAP_PRINT_POS, sizeof(_lift_pos), &_lift_pos); // Lift move to position
				//sok_send_2(_step_socket[0], CMD_CAP_CAPPING_POS, 0, NULL); // Lift move to position
				sok_send_2(_step_socket[0], _step_ipaddr[0], CMD_CAP_PRINT_POS, sizeof(_lift_pos), &_lift_pos); // Lift move to position
				//_capCurrentState = ST_LIFT_MOVE_POS; // no movement of cleaning station
				_capCurrentState = ST_CLEAN_MOVE_POS; // with cleaning station movement
				_lift_move_tgl = RX_TestTableStatus.info.move_tgl;
			}
		}
		else if (_st_cmd_stored == CMD_CLN_SCREW_0_POS)
		{
			if (plc_in_wipe_pos() == TRUE) // slide in cleaning position
			{
				_capCurrentState = ST_CLEAN_MOVE_POS;
			}
			}
			else
			{
			_capCurrentState = ST_CLEAN_MOVE_POS;
		}
		break;
		
	case ST_CLEAN_MOVE_POS:
				if (_st_cmd_stored == CMD_CAP_UP_POS)
				{
					printf("state %d ST_LIFT_MOVE_UP done\n", _capCurrentState);
					_capCurrentState = ST_IDLE;
				}
				else if (_st_cmd_stored == CMD_CAP_CAPPING_POS)
					{
					//if ((RX_ClnStatus.info.z_in_cap == TRUE) ) // && (RX_TestTableStatus.info.cln_in_capping == TRUE)) // cln_in_capping, better: z_in_cap
					//{
						printf("state %d CLEAN_MOVE done\n", _capCurrentState);
						_lift_move_tgl = RX_TestTableStatus.info.move_tgl;
						_capCurrentState = ST_LIFT_MOVE_POS;
						printf("state %d do lift to capping now\n", _capCurrentState);
						//sok_send_2(_step_socket[0], CMD_CAP_PRINT_POS, sizeof(_lift_pos), &_lift_pos); // Lift move to position
					int move = _lift_pos + LIFT_CAP_SECURITY_DELTA; //  -(RX_Config.stepper.cap_height - LIFT_CAP_SECURITY_DELTA - RX_Config.stepper.ref_height); 
						sok_send_2(_step_socket[0], _step_ipaddr[0], CMD_CAP_PRINT_POS, sizeof(move), &move);
					//sok_send_2(_step_socket[0], CMD_CAP_PRINT_POS, sizeof(move), &move);
					//}
					//else
					//{
					//	ErrorFlag(ERR_CONT, &_ErrorFlags, ERR_3, 0, "MAIN: State_Command 0x%08x: wait on CLN to be in Capping", _st_cmd_stored);
					//}

				}
				else if (_st_cmd_stored == CMD_CAP_PRINT_POS)
				{
						printf("state %d CLEAN_MOVE done\n", _capCurrentState);
						_lift_move_tgl = RX_TestTableStatus.info.move_tgl;
						_capCurrentState = ST_LIFT_MOVE_POS;
						printf("state %d do lift to printing now\n", _capCurrentState);
						sok_send_2(_step_socket[0], _step_ipaddr[0], CMD_CAP_PRINT_POS, sizeof(_lift_pos), &_lift_pos); // Lift move to position
					}
					else
					{
					printf("state %d CLEAN_MOVE done\n", _capCurrentState);
					_lift_move_tgl = RX_TestTableStatus.info.move_tgl;
					_capCurrentState = ST_LIFT_MOVE_POS;
					printf("state %d do lift move now\n", _capCurrentState);
					sok_send_2(_step_socket[0], _step_ipaddr[0], CMD_CAP_PRINT_POS, sizeof(_lift_pos), &_lift_pos); // Lift move to position
				}
		break;
		
	case ST_LIFT_MOVE_POS:
		if (_lift_move_tgl != RX_TestTableStatus.info.move_tgl)
		{
			if ((RX_TestTableStatus.info.z_in_print == TRUE) || (RX_TestTableStatus.info.z_in_cap == TRUE)) // New: check also for Lift beeing in capping
			{
				printf("state %d ST_LIFT_MOVE_POS move ok\n", _capCurrentState);
				if (_st_cmd_stored == CMD_CAP_CAPPING_POS)
				{
					printf("state %d ST_LIFT_MOVE_POS capping ok\n", _capCurrentState);
					_lift_move_tgl = RX_TestTableStatus.info.move_tgl;
					sok_send_2(_step_socket[0], _step_ipaddr[0], CMD_CAP_CAPPING_POS, 0, NULL); // Lift move to capping
					// execute capping ?
					// sok_send_2(_step_socket[step_lift], CMD_CAP_CAPPING_POS, 0, NULL); // Lift execute capping
					_capCurrentState = ST_LIFT_CAPPING;
					//_capCurrentState = ST_IDLE;
				}
				else if (_st_cmd_stored == CMD_CAP_PURGE)
				{
					_capCurrentState = ST_PURGE;
				}
				else
				{
					printf("state %d ST_LIFT_MOVE_POS else ok\n", _capCurrentState);
					_lift_move_tgl = RX_TestTableStatus.info.move_tgl;
					_capCurrentState = ST_IDLE;
				}
			}
		}
		//}
		break;
		
	case ST_LIFT_CAPPING :
			if (RX_TestTableStatus.info.z_in_cap == TRUE)
			{
				_lift_move_tgl = RX_TestTableStatus.info.move_tgl;
				_capCurrentState = ST_IDLE;
			}
		break;
		
	case ST_LIFT_UP :
		if (RX_TestTableStatus.info.z_in_ref == TRUE)
		{
			printf("state %d ST_LIFT_MOVE_POS UP ok\n", _capCurrentState);
			_capCurrentState = ST_IDLE;
		}
		break;
		
	case  ST_PURGE:
		printf("state %d Purge done\n", _capCurrentState);
		_flag_purge_done = TRUE;
		if (trace) Error(LOG, 0, "Flag: PURGE DONE enable");
		_capCurrentState = ST_IDLE;
		break;
			
	case  ST_LAST_LIFT:
		
		if (_lift_move_tgl != RX_TestTableStatus.info.move_tgl)
		{
			if (RX_TestTableStatus.info.z_in_print == TRUE) // RX_TestTableStatus.info.z_in_ref == TRUE)  || RX_TestTableStatus.info.z_in_print == TRUE)
			{
				_capCurrentState = ST_IDLE;
			}
		}
		break;
		
	}
	
}

//--- _do_turn_screw --------------------------------------------------
static int _do_turn_screw(SHeadAdjustment *pHeadAdjust)
{
	Error(WARN, 0, "MAIN: State_Command 0x%08x Cleaning Station not implemented", _st_cmd_stored);
		return FALSE;
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
//	steptx_init(no, _step_socket[no]);
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

//--- steptx_to_print_pos --------------------------------
int	 steptx_to_print_pos(void)
{
	if (_capCurrentState == ST_INIT || _capCurrentState == ST_IDLE)
	{
	_st_cmd = CMD_CAP_PRINT_POS;
		return REPLY_OK;
	}
	_do_tt_status(&RX_TestTableStatus);
	return FALSE;									
}

//--- steptx_to_up_pos --------------------------------
int	 steptx_to_up_pos(void)
{
	if (_capCurrentState == ST_INIT || _capCurrentState == ST_IDLE)
	{
	_st_cmd = CMD_CAP_UP_POS;
		return REPLY_OK;
	}
	_do_tt_status(&RX_TestTableStatus);
	return FALSE;									
}

//--- steptx_slide_enabled ---------------------
int	 steptx_slide_enabled(void)
{
	if (RX_TestTableStatus.posZ >= 0)
	{
		return TRUE;
	}
	return FALSE;
}

//--- steptx_gui_enabled ---------------------
int	 steptx_gui_enabled(void)
{
	if (_capCurrentState == ST_IDLE && (RX_TestTableStatus.posZ >= 0) && _slide_enable)
	{
		return TRUE;
	}
	return FALSE;
}

//--- steptx_to_purge_pos ---------------------
int	 steptx_to_purge_pos(int no)
{
	if (trace) Error(LOG, 0, "fluid_cmd start purge");
	if (_capCurrentState == ST_INIT || _capCurrentState == ST_IDLE)
	{
		_clean_wipe_head_nr = no;
		_slide_enable = FALSE;
		if (trace) Error(LOG, 0, "Flag: SLIDE disable");
		if (trace) Error(LOG, 0, "steptx_to_purge_pos: CMD_CAP_PURGE start");
		_st_cmd = CMD_CAP_PURGE;
		return REPLY_OK;
	}
	return FALSE;	
}

//--- steptx_in_purge_pos ---------------
int  steptx_in_purge_pos(void)
{
	static int _old=-1;
	if ((_flag_purge_done == TRUE)  && (plc_in_purge_pos() == TRUE)) // && (abs(RX_TestTableStatus.posZ - _lift_pos) <= 10) && (abs(RX_ClnStatus.posZ - _clean_pos) <= 10)
	{
		if (_old!=TRUE) {if (trace) Error(LOG, 0, "in purge");}
		return _old=TRUE;
	}
	return _old=FALSE;		
}

//--- steptx_wipe_start --------------------------------------------
int	 steptx_wipe_start(int no)
{
	if (trace) Error(LOG, 0, "fluid_cmd start wipe");
	_clean_wipe_head_nr = no;
	if (_capCurrentState == ST_INIT || _capCurrentState==ST_IDLE)
	{
		_st_cmd = CMD_CLN_WIPE;
		if (trace) Error(LOG, 0, "steptx_wipe_start: CMD_CLN_WIPE start");
	return REPLY_OK;									
	}
	return FALSE;									
}

//--- steptx_wipe_done --------------------------------------------
int	 steptx_wipe_done(void)
{
	if ((_flag_wipe_done == TRUE)  && (plc_in_wipe_pos() == TRUE)) // && (abs(RX_TestTableStatus.posZ - _lift_pos) <= 10) && (abs(RX_ClnStatus.posZ - _clean_pos) <= 10)
	{
		if (trace) Error(LOG, 0, "wipe done");
		return TRUE;
	}
	return FALSE;									
}

////--- steptx_purge_ok --------------------------------------------
//int	 steptx_purge_flag(int no)
//{
//	Error(LOG, 0, "steptx_purge_flag(no=%d)",no);
//	//_flag_no = no;
//	return REPLY_OK;									
//}
//
////--- steptx_purge_ok --------------------------------------------
//EnFluidCtrlMode	 steptx_purge_ok(int no)
//{
//	Error(LOG, 0, "steptx_purge_ok(no=%d)", no);
//	if (_flag_finish_purge)
//	{
//		Error(LOG, 0, "step3_finish_flag");
//		return ctrl_print;
//	}	
//	else
//		{
//			Error(LOG, 0, "step3_flag_start_wipe");
//			_flag_finish_purge = FALSE;
//			_clean_wipe_head_nr = no;
//			_st_cmd = CMD_CLN_WIPE;
//			return ctrl_wipe;
//		}
//}