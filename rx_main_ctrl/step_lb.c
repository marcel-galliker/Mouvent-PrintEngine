// ****************************************************************************
//
//	step_lb.c	Standrd Stepper Control		
//
// ****************************************************************************
//
//	Copyright 2016 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#include <string.h>
#include <stdlib.h>

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
#include "ctrl_svr.h"
#include "step_lb.h"
#include "fluid_ctrl.h"
#include "setup.h"

#define STEPPER_CNT		    4

#define VACUUM_DELAY        10 * 1000       // ms -> 10 seconds
static RX_SOCKET		    _step_socket[STEPPER_CNT]={0};

static SStepperStat		    _Status[STEPPER_CNT];
static int				    _AbortPrinting=FALSE;
static int                  _WashStarted;
static UINT32			    _Flushed = 0x00;		// For capping function which is same than flushing (need to purge after cap)
static int                  _ScrewCommandSend[STEPPER_CNT] = {FALSE};
static int                  _OldVacuum_Cleaner_State = FALSE;
static double                _Vacuum_Cleaner_Time = 0;
static int                  _AutoCapMode = FALSE;

static int                  _AutoCapTimer = 0;


static int				    _StatReadCnt[STEPPER_CNT];

static EnFluidCtrlMode	    _RobotCtrlMode[STEPPER_CNT] = {ctrl_undef};

static SHeadAdjustmentMsg   _HeadAdjustment[STEPPER_CNT] = {0};

static SHeadAdjustmentMsg   _HeadAdjustmentBuffer[STEPPER_CNT][MAX_HEAD_DIST];

static void _steplb_rob_do_reference(int no);

static void _check_screwer(void);
static void _check_fluid_back_pump(void);
static void _send_ctrlMode(EnFluidCtrlMode ctrlMode, int no);
static int  _rob_get_printbar(int rob, int printbar);
static int  _set_screw_pos(int stepperNo);

//--- steplb_init ---------------------------------------------------
void steplb_init(int no, RX_SOCKET socket)
{	
	setup_fluid_system(PATH_USER FILENAME_FLUID_STATE, &_Flushed, READ);
	if (no>=0 && no<STEPPER_CNT)
	{
		_step_socket[no] = socket;
		memset(&_Status[no], 0, sizeof(_Status[no]));
	}
	memset(_Status, 0, sizeof(_Status));
	// All steppers board variables reset
	for (int i = 0; i < STEPPER_CNT; i++) _StatReadCnt[i] = 0;
    RX_StepperStatus.robinfo.auto_cap = TRUE;

    for (int i = 0; i < STEPPER_CNT; i++)
    {
        for (int j = 0; j < MAX_HEAD_DIST; j++)
        {
            _HeadAdjustmentBuffer[i][j].printbarNo = -1;
        }
    }
}

//--- steplb_handle_gui_msg------------------------------------------------------------------
int	 steplb_handle_gui_msg(RX_SOCKET socket, UINT32 cmd, void *data, int dataLen)
{	
	int no;
	for (no=0; no<SIZEOF(_step_socket); no++)
	{
		if (_step_socket[no] && _step_socket[no]!=INVALID_SOCKET)
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
						sok_send_2(&_step_socket[no], cmd, 0, NULL);
						break;

			case CMD_TT_SCAN:
						{
							STestTableScanPar par;
							par.speed	= 5;
							par.scanCnt = 5;
							par.scanMode= PQ_SCAN_STD;
							par.yStep   = 10000;

							sok_send_2(&_step_socket[no], CMD_TT_SCAN, sizeof(par), &par);
						}
						break;

			//--- cappping ---------------------------------------------------------
			case CMD_LIFT_STOP:
			case CMD_LIFT_UP_POS:
			case CMD_LIFT_CAPPING_POS:
			case CMD_LIFT_REFERENCE:
			case CMD_ROB_REFERENCE:
            case CMD_ROB_SERVICE:
						sok_send_2(&_step_socket[no], cmd, 0, NULL);
						break;
		
			case CMD_LIFT_PRINT_POS:
						_AbortPrinting=FALSE;
						int height=RX_Config.stepper.print_height+RX_Config.stepper.material_thickness;
						sok_send_2(&_step_socket[no], CMD_LIFT_PRINT_POS, sizeof(height), &height);
						break;
            }
		}
	}
	return REPLY_OK;
}

//--- _rob_get_printbar ------------------------------------------
static int _rob_get_printbar(int rob, int printbar)
{
    switch(RX_Config.printer.type)
    {
    case printer_LB702_WB:  return rob*2+printbar;
    default:                Error(ERR_CONT, 0, "Not implemented");
                            return printbar;
    }
}

//--- steplb_handle_status ----------------------------------------------------------------------
int steplb_handle_status(int no, SStepperStat *pStatus)
{
    int i;
    int robot_used = FALSE;
    ETestTableInfo info;
    ERobotInfo robinfo;

    SStepperStat oldStatus[STEPPER_CNT];

    for (i = 0; i < STEPPER_CNT; i++)
    {
        memcpy(&oldStatus[i], &_Status[i], sizeof(oldStatus[i]));
    }
    memcpy(&_Status[no], pStatus, sizeof(_Status[no]));
    
    _Status[no].no = no;
    gui_send_msg_2(0, REP_STEPPER_STAT, sizeof(RX_StepperStatus), &_Status[no]);

    // Don't refresh the main variable until all steppers board status have been
    // received (after  a call of steplb_init())
    _StatReadCnt[no]++;

    for (i = 0; i < STEPPER_CNT; i++)
    {
        if (_step_socket[i] && _step_socket[i] != INVALID_SOCKET)
        {
            if (_StatReadCnt[i] == 0) return REPLY_OK;
        }
    }

    memset(&info, 0, sizeof(info));
    memset(&robinfo, 0, sizeof(robinfo));
    info.printhead_en = TRUE;
    info.ref_done = TRUE;
    info.z_in_ref = TRUE;
    info.z_in_print = TRUE;
    info.z_in_cap = TRUE;
    info.x_in_cap = TRUE;
    info.x_in_ref = TRUE;
    robinfo.rob_in_cap = TRUE;
    robinfo.ref_done = TRUE;
    robinfo.purge_ready = TRUE;
    
    for (i = 0; i < STEPPER_CNT; i++)
    {
        if (_step_socket[i] && _step_socket[i] != INVALID_SOCKET)
        {
            if (_Status[i].robot_used && !oldStatus[i].robot_used)
            {
                _set_screw_pos(i);
            }

            info.ref_done &= _Status[i].info.ref_done;
            info.printhead_en &= _Status[i].info.printhead_en;
            info.moving |= _Status[i].info.moving;
            info.z_in_ref &= _Status[i].info.z_in_ref;
            info.z_in_print &= _Status[i].info.z_in_print;
            info.z_in_cap &= _Status[i].info.z_in_cap;
            info.z_in_screw |= _Status[i].info.z_in_screw;
            info.x_in_cap &= _Status[i].info.x_in_cap;
            info.x_in_ref &= _Status[i].info.x_in_ref;
            robot_used |= _Status[i].robot_used;
            robinfo.rob_in_cap &= _Status[i].robinfo.rob_in_cap;
            robinfo.ref_done &= _Status[i].robinfo.ref_done;
            robinfo.moving |= _Status[i].robinfo.moving;
            robinfo.purge_ready &= _Status[i].robinfo.purge_ready;
            RX_StepperStatus.posY[i] = _Status[i].posY[1];
            if (_Status[i].info.moving)
            {
                RX_StepperStatus.posX = _Status[i].posX;
                RX_StepperStatus.posZ = _Status[i].posZ;
                RX_StepperStatus.posZ_back = _Status[i].posZ_back;
            }
        }
    };
    if (_step_socket[0] == INVALID_SOCKET || _step_socket[0] == 0)
        robinfo.auto_cap = TRUE;
    else
        robinfo.auto_cap = _Status[0].robinfo.auto_cap;
    
    if (RX_Config.printer.type == printer_LB701)
    {
        info.headUpInput_0 = _Status[0].info.headUpInput_0;
        info.headUpInput_1 = _Status[1].info.headUpInput_0;
        info.headUpInput_2 = _Status[2].info.headUpInput_0;
        info.headUpInput_3 = _Status[3].info.headUpInput_0;
    }
    else if (RX_Config.printer.type == printer_LB702_WB ||
             RX_Config.printer.type == printer_LB702_UV)
    {
        info.headUpInput_0 = _Status[0].info.z_in_ref;      
        info.headUpInput_1 = _Status[1].info.z_in_ref; 
        info.headUpInput_2 = _Status[2].info.z_in_ref; 
        info.headUpInput_3 = _Status[3].info.z_in_ref; 
    }
    RX_StepperStatus.robot_used = robot_used;

    if (!info.moving)
    {
        RX_StepperStatus.posX = _Status[no].posX;
        RX_StepperStatus.posZ = _Status[no].posZ;
        RX_StepperStatus.posZ_back = _Status[no].posZ_back;
    }

    if (_AbortPrinting && RX_StepperStatus.info.z_in_print)
        steplb_lift_to_up_pos();

    memcpy(&RX_StepperStatus.info, &info, sizeof(RX_StepperStatus.info));
    memcpy(&RX_StepperStatus.robinfo, &robinfo, sizeof(RX_StepperStatus.robinfo));
    RX_StepperStatus.robinfo.rob_in_cap = robinfo.rob_in_cap;

    if (_step_socket[no] && _step_socket[no] != INVALID_SOCKET)
        steplb_rob_control(_RobotCtrlMode[no], no);
    

    for (i = 0; i < STEPPER_CNT; i++)
    {
        // Vacuum Cleaner Timer Start -------------------------------------------------------------------------------
        // Just needed for testing phase -> Need to be taken out before it goes to customers
        if (_OldVacuum_Cleaner_State == FALSE && _Status[i].info.vacuum_running && i == 1)
        {
            _Vacuum_Cleaner_Time = rx_get_ticks()/1000;
        }
        else if (_OldVacuum_Cleaner_State == TRUE && !_Status[i].info.vacuum_running && i == 1)
        {
            double _OldVacuum_Cleaner_Time = 0;
            setup_vacuum_cleaner(PATH_USER FILENAME_VACUUM_TIME, &_OldVacuum_Cleaner_Time, READ);
            _OldVacuum_Cleaner_Time = _OldVacuum_Cleaner_Time + rx_get_ticks()/1000 - _Vacuum_Cleaner_Time;
            setup_vacuum_cleaner(PATH_USER FILENAME_VACUUM_TIME, &_OldVacuum_Cleaner_Time, WRITE);
        }
        if (i == 1) _OldVacuum_Cleaner_State = _Status[i].info.vacuum_running;
        
        // Vacuum Cleaner Timer End ---------------------------------------------------------------------------------
    }
    
    if (rx_get_ticks() >= _AutoCapTimer && _AutoCapTimer)
    {
        _AutoCapTimer = 0;
        _AutoCapMode = FALSE;
    }

    if (no == 0)
    {
        _check_screwer();
        _check_fluid_back_pump();
    }
    return REPLY_OK;
}

//--- _set_screw_pos -----------------------------------------
static int _set_screw_pos(int stepperNo)
{
    SScrewPositions pos[STEPPER_CNT];
    memset(&pos, 0, sizeof(pos));

    setup_screw_positions(PATH_USER FILENAME_SCREW_POS, pos, READ);
    for (int printbar=0; printbar<2; printbar++)
    {
        for (int head=0; head<RX_Config.headsPerColor; head++)
        {
            int no = _rob_get_printbar(stepperNo, printbar)*RX_Config.headsPerColor+head;
            pos[stepperNo].printbar[printbar].head[head][AXE_ANGLE].turns  = RX_HBStatus[no/HEAD_CNT].head[no%HEAD_CNT].eeprom_mvt.robot.angle;
            pos[stepperNo].printbar[printbar].head[head][AXE_STITCH].turns = RX_HBStatus[no/HEAD_CNT].head[no%HEAD_CNT].eeprom_mvt.robot.stitch;
        }
    }
    sok_send_2(&_step_socket[stepperNo], CMD_SET_SCREW_POS, sizeof(pos[stepperNo]), &pos[stepperNo]);
    return REPLY_OK;
}

//--- steplb_set_ScrewPos -----------------------------------------
int	 steplb_set_ScrewPos(int no, SScrewPositions *ppos)
{
    SScrewPositions pos[STEPPER_CNT];
    memset(&pos, 0, sizeof(pos));

    setup_screw_positions(PATH_USER FILENAME_SCREW_POS, pos, READ);
    memcpy(&pos[no], ppos, sizeof(pos[no]));
    
    setup_screw_positions(PATH_USER FILENAME_SCREW_POS, pos, WRITE);
    for (int printbar=0; printbar<2; printbar++)
    {
        for (int head=0; head<RX_Config.headsPerColor; head++)
        {
            int headNo=_rob_get_printbar(no, printbar)*RX_Config.headsPerColor+head;
            ctrl_set_rob_pos(headNo, ppos->printbar[printbar].head[head][AXE_ANGLE].turns, ppos->printbar[printbar].head[head][AXE_STITCH].turns);
        }
    }

    return REPLY_OK;
}

//--- steplb_to_print_pos --------------------------------
int	 steplb_to_print_pos(void)
{
	_AbortPrinting = FALSE;
	for (int no=0; no<SIZEOF(_step_socket); no++)
	{
		int height=RX_Config.stepper.print_height+RX_Config.stepper.material_thickness;
		sok_send_2(&_step_socket[no], CMD_LIFT_PRINT_POS, sizeof(height), &height);
	}
	return REPLY_OK;									
}

//--- steplb_abort_printing -----------------------------------------
void  steplb_abort_printing(void)
{
	if(RX_StepperStatus.info.z_in_print) steplb_lift_to_up_pos();
	else _AbortPrinting = TRUE;
}

//--- steplb_lift_to_top_pos ---------------------------
void steplb_lift_to_top_pos(void)
{
    if (_AutoCapTimer == 0 && RX_StepperStatus.info.z_in_cap && _AutoCapMode)
    {
        _AutoCapTimer = rx_get_ticks() + 5000;
    }
    else if (!(_AutoCapMode && RX_StepperStatus.info.z_in_cap))
    {
        for (int no = 0; no < SIZEOF(_step_socket); no++)
        {
            if (!_Status[no].info.z_in_screw || _Status[no].screwerinfo.screwer_ready || !RX_StepperStatus.robot_used)
                sok_send_2(&_step_socket[no], CMD_LIFT_REFERENCE, 0, NULL);
        }
        _AbortPrinting = FALSE;
    }
}

//--- steplb_lift_in_top_pos --------------
int	 steplb_lift_in_top_pos(void)
{
	return RX_StepperStatus.info.z_in_ref;
}

//--- steplb_lift_to_up_pos ---------------------------
void steplb_lift_to_up_pos(void)
{
	for (int no=0; no<SIZEOF(_step_socket); no++)
	{
        sok_send_2(&_step_socket[no], CMD_LIFT_UP_POS, 0, NULL);
	}
	_AbortPrinting = FALSE;
}

//--- steplb_lift_is_up --------------
int	 steplb_lift_in_up_pos(void)
{
	return RX_StepperStatus.info.z_in_ref;
}

//--- steplb_lift_to_up_pos_individually -------------------
void steplb_lift_to_up_pos_individually(int no)
{
	sok_send_2(&_step_socket[no], CMD_LIFT_UP_POS, 0, NULL);
}

//--- steplb_lift_in_up_pos_individually -------------------
int	 steplb_lift_in_up_pos_individually(int no)
{
	if (no == -1) return steplb_lift_in_up_pos();
	if (_step_socket[no] == INVALID_SOCKET) return TRUE;
	return _Status[no].info.z_in_ref;
}

//--- steplb_rob_to_fct_pos --------------------------
void steplb_rob_to_fct_pos(int no, ERobotFunctions rob_function)
{
    if (_step_socket[no] == INVALID_SOCKET) return;
	sok_send_2(&_step_socket[no], CMD_ROB_MOVE_POS, sizeof(rob_function), &rob_function);		
}

//--- steplb_rob_to_fct_pos_all ----------------------------------------------
void steplb_rob_to_fct_pos_all(ERobotFunctions rob_function)
{
    int i;
    for (i = 0; i < STEPPER_CNT; i++)
    {
        if (_step_socket[i] != INVALID_SOCKET)
			sok_send_2(&_step_socket[i], CMD_ROB_MOVE_POS, sizeof(rob_function), &rob_function);
    }
}

//--- steplb_rob_in_fct_pos --------------------------
int	 steplb_rob_in_fct_pos(int no, ERobotFunctions rob_function)
{
	if (_step_socket[no]==INVALID_SOCKET) return TRUE;
	
	switch (rob_function)
	{
	case rob_fct_cap: 			return _Status[no].robinfo.rob_in_cap		&& _Status[no].robinfo.moving == FALSE; break;
	case rob_fct_wash: 			return _Status[no].robinfo.wash_ready		&& _Status[no].robinfo.moving == FALSE; break;
	case rob_fct_vacuum: 		return _Status[no].robinfo.vacuum_ready		&& _Status[no].robinfo.moving == FALSE; break;
	case rob_fct_wipe: 			return _Status[no].robinfo.wipe_ready		&& _Status[no].robinfo.moving == FALSE; break;
	case rob_fct_vacuum_change: return _Status[no].robinfo.vacuum_in_change	&& _Status[no].robinfo.moving == FALSE; break;
	case rob_fct_purge_all:		return _Status[no].robinfo.purge_ready		&& _Status[no].robinfo.moving == FALSE; break; // && _Status[no].robinfo.rob_in_cap
	case rob_fct_purge_head0:
	case rob_fct_purge_head1:
	case rob_fct_purge_head2:
	case rob_fct_purge_head3:
	case rob_fct_purge_head4:
	case rob_fct_purge_head5:
	case rob_fct_purge_head6:
	case rob_fct_purge_head7:	return _Status[no].robinfo.purge_ready		&& _Status[no].robinfo.moving == FALSE; break;
    case rob_fct_purge4ever:    return _Status[no].robinfo.x_in_purge4ever     && _Status[no].robinfo.moving == FALSE; break;
	default: return FALSE; break;
	}
}

//--- steplb_rob_in_fct_pos_all ------------------------
int steplb_rob_in_fct_pos_all(ERobotFunctions rob_function)
{
	switch (rob_function)
	{
	case rob_fct_cap:			return RX_StepperStatus.robinfo.rob_in_cap	&& RX_StepperStatus.robinfo.moving == FALSE; break;
    case rob_fct_purge_head7:	return RX_StepperStatus.robinfo.purge_ready && RX_StepperStatus.robinfo.moving == FALSE; break;
	default: return FALSE; break;
	}
}

//--- steplb_rob_fct_start -------------------------------------
void steplb_rob_fct_start(int no, ERobotFunctions rob_function)
{
	if (_step_socket[no]==INVALID_SOCKET) return;
	sok_send_2(&_step_socket[no], CMD_ROB_FILL_CAP, sizeof(rob_function), &rob_function);
}

//--- steplb_rob_fct_done --------------------------------------
int	 steplb_rob_fct_done(int no, ERobotFunctions rob_function)
{
	if (_step_socket[no]==INVALID_SOCKET) return TRUE;
	switch (rob_function)
	{
	case rob_fct_wash:		return _Status[no].robinfo.wash_done;
	case rob_fct_vacuum:	return _Status[no].robinfo.vacuum_done;
	case rob_fct_cap:		return _Status[no].robinfo.cap_ready;
	default:				return FALSE;
	}	
}

//--- steplb_rob_stop_all ------------------------------
void steplb_rob_stop_all(void)
{
	for (int no = 0; no<SIZEOF(_step_socket); no++)
	{
		if (_Status[no].robot_used) sok_send_2(&_step_socket[no], CMD_ROB_STOP, 0, NULL);
		sok_send_2(&_step_socket[no], CMD_LIFT_STOP, 0, NULL);
	}	
}

//--- steplb_rob_stop -------------------------------------------------
void steplb_rob_stop(int no)
{
    if (_step_socket[no] == INVALID_SOCKET) return;
    if (_Status[no].robot_used) sok_send_2(&_step_socket[no], CMD_ROB_STOP, 0, NULL);
	sok_send_2(&_step_socket[no], CMD_LIFT_STOP, 0, NULL);
}

//--- steplb_rob_do_reference -----------------------
void steplb_rob_do_reference(void)
{
	for (int no = 0; no < SIZEOF(_step_socket); no++)
	{
		if (_step_socket[no] != INVALID_SOCKET)
			sok_send_2(&_step_socket[no], CMD_ROB_REFERENCE, 0, NULL);
	}
}

//--- steplb_rob_empty_waste ---------------------------------------
/*
 * If time <= 0, then a defined time on the Stepperboard will used for the pump time.
 * The pump time can't be decreased , if it gets set a time here and it has already
 * a time left running, it will take the longer time.
 *
 * */
void steplb_rob_empty_waste(int time)
{
    for (int i = 0; i < STEPPER_CNT; i++)
    {
        if (_step_socket[i] != INVALID_SOCKET)
            sok_send_2(&_step_socket[i], CMD_ROB_VACUUM, sizeof(time), &time);
    }
    
}

//--- _steplb_rob_do_reference ---------------------------------------------------
static void _steplb_rob_do_reference(int no)
{
    sok_send_2(&_step_socket[no], CMD_ROB_REFERENCE, 0, NULL);
}

//--- steplb_rob_reference_done ---------------------
int steplb_rob_reference_done(void)
{
	return RX_StepperStatus.robinfo.ref_done;
}

//--- steplb_lift_to_fct_pos ----------------------------
void steplb_lift_to_fct_pos(int no, ERobotFunctions rob_function)
{
	switch (rob_function)
	{
	case rob_fct_cap: sok_send_2(&_step_socket[no], CMD_LIFT_CAPPING_POS, 0, NULL); break;
	default: break;
	}
}

//--- steplb_lift_in_fct_pos -------------------------------
int  steplb_lift_in_fct_pos(int no, ERobotFunctions rob_function)
{
	if (_step_socket[no] == INVALID_SOCKET) return TRUE;
	switch (rob_function)
	{
	case rob_fct_cap: return _Status[no].info.z_in_cap; break;
	default: return FALSE; break;
	}
}

//--- steplb_rob_control -------------------------------
void steplb_rob_control_all(EnFluidCtrlMode ctrlMode)
{
	for (int no = 0; no < SIZEOF(_step_socket); no++)
	{
		if (_step_socket[no] != INVALID_SOCKET)		steplb_rob_control(ctrlMode, no);
    }
}


//--- steplb_rob_start_cap_all -------------------------------
void steplb_rob_start_cap_all(void)
{
	for (int no = 0; no < SIZEOF(_step_socket); no++)
	{
		if (_step_socket[no] != INVALID_SOCKET)		_RobotCtrlMode[no] = ctrl_cap;
	}
}

//--- steplb_pump_back_fluid ----------------------------------
void steplb_pump_back_fluid(int fluidNo, int state)
{
    int even_number_of_colors = RX_Config.inkSupplyCnt % 2 == 0;
    int val = 0;
    if (fluidNo == -1)
    {
        for (int no = 0; no < SIZEOF(_step_socket); no++)
        {
            if (_step_socket[no] != INVALID_SOCKET && state == FALSE) 
                sok_send_2(&_step_socket[no], CMD_ROB_EMPTY_WASTE, sizeof(val), &val);
            else if (_step_socket[no] != INVALID_SOCKET && state == TRUE)
            {
                val = 5;
                sok_send_2(&_step_socket[no], CMD_ROB_EMPTY_WASTE, sizeof(val), &val);
            }
        }  
    }
    else if (even_number_of_colors)
    {
        if (state == FALSE)
            val = ((fluidNo % 2) + 1) * 2;
        else
            val = ((fluidNo % 2) + 1) * 2 - 1;
        sok_send_2(&_step_socket[fluidNo/2], CMD_ROB_EMPTY_WASTE, sizeof(val), &val);
    }
    else
    {
        if (state == FALSE)
        {
            if (fluidNo % 2 == 0)
                val = 4;
            else
                val = 2;
        }
        else
        {
            if (fluidNo % 2 == 0)
                val = 3;
            else
                val = 1;
        }
        sok_send_2(&_step_socket[(fluidNo+1)/2], CMD_ROB_EMPTY_WASTE, sizeof(val), &val);
    }
}

//--- steplb_rob_control ------------------------------
void steplb_rob_control(EnFluidCtrlMode ctrlMode, int no)
{		
    static int _risingEdge[STEPPER_CNT] = {0};
    static int _time[STEPPER_CNT] = {0};
    ERobotFunctions function;

    if (ctrlMode != ctrl_vacuum && ctrlMode != ctrl_vacuum_step1) _time[no] = 0;
    
	if (_Status[no].robot_used)
	{
		EnFluidCtrlMode	old = _RobotCtrlMode[no];
		switch (ctrlMode)
		{
		case ctrl_cap:				if (!_Status[no].robinfo.ref_done) _steplb_rob_do_reference(no);
									_RobotCtrlMode[no] = ctrl_cap_step1;
                                    break;
		
		case ctrl_cap_step1:		if (_Status[no].robinfo.ref_done && !_Status[no].robinfo.moving)
									{
										steplb_rob_to_fct_pos(no, rob_fct_cap);
                                        if (_AutoCapMode)   _RobotCtrlMode[no] = ctrl_cap_step3;
                                        else                _RobotCtrlMode[no] = ctrl_cap_step2;
									}
                                    _risingEdge[no] = FALSE;
                                    break;
		
		case ctrl_cap_step2:		if (steplb_rob_in_fct_pos(no, rob_fct_cap) && _risingEdge[no])
									{
										steplb_rob_fct_start(no, rob_fct_cap);
										_RobotCtrlMode[no] = ctrl_cap_step3;
                                    }
                                    else if (!steplb_rob_in_fct_pos(no, rob_fct_cap))
                                        _risingEdge[no] = TRUE;
									break;
		
		case ctrl_cap_step3:		if (steplb_rob_fct_done(no, rob_fct_cap) || (steplb_rob_in_fct_pos(no, rob_fct_cap) && _risingEdge[no] && _AutoCapMode))
									{
										steplb_lift_to_fct_pos(no, rob_fct_cap);
										_RobotCtrlMode[no] = ctrl_cap_step4;
                                    }
                                    else if (!steplb_rob_in_fct_pos(no, rob_fct_cap))
                                        _risingEdge[no] = TRUE;
									break;
		
		case ctrl_cap_step4:		if (_Status[no].info.z_in_cap)
									{
                                        if (!rx_def_is_lb(RX_Config.printer.type))
                                        {
                                            _Flushed |= (0x3 << (no*2));
											Error(LOG, 0, "ctrl_cap_step4 OK, no=%d, _Flushed=%d",no,_Flushed);
                                            setup_fluid_system(PATH_USER FILENAME_FLUID_STATE, &_Flushed, WRITE);
											fluid_init_flushed();
                                        }
                                        if (_AutoCapMode)
                                        {
                                            _RobotCtrlMode[no] = ctrl_print;
                                        }
                                        else			
                                        {
                                            _RobotCtrlMode[no] = ctrl_off;
                                            _send_ctrlMode(_RobotCtrlMode[no], no);
                                        }
                                    }										 
									break;
		
        case ctrl_wash:				if (!_Status[no].robinfo.moving)
                                    {
                                        function = rob_fct_wash;
									    sok_send_2(&_step_socket[no], CMD_ROB_MOVE_POS, sizeof(function), &function);
                                        _RobotCtrlMode[no] = ctrl_wash_step1;
                                        _WashStarted = FALSE;
                                    }
                                    else _RobotCtrlMode[no] = ctrl_wash;
                                    
                                    break;
                                    
        case ctrl_wash_step1:		if (_Status[no].robinfo.moving && !_Status[no].robinfo.wash_done) _WashStarted = TRUE;
                                    if (_Status[no].robinfo.wash_done && _WashStarted) 
                                    {
                                        _RobotCtrlMode[no] = ctrl_wash_step2;
                                        steplb_lift_to_top_pos();
                                    }
									break;

        case ctrl_wash_step2:		if (steplb_lift_in_top_pos())
                                    {
                                        _RobotCtrlMode[no] = ctrl_wash_step3; 
                                    }
                                    _WashStarted = FALSE;
									break;
                                    
        case ctrl_wash_step3:		_RobotCtrlMode[no] = ctrl_off;
									break;
                                    
        case ctrl_vacuum:           _RobotCtrlMode[no] = ctrl_vacuum_step1;
                                    _time[no] = rx_get_ticks() + VACUUM_DELAY;
                                    break;

        case ctrl_vacuum_step1:		if (!_Status[no].robinfo.moving && rx_get_ticks() >= _time[no])
                                    {
                                        function = rob_fct_vacuum;
									    if (ctrl_vacuum_step2 != _RobotCtrlMode[no]) sok_send_2(&_step_socket[no], CMD_ROB_MOVE_POS, sizeof(function), &function);
                                        _RobotCtrlMode[no] = ctrl_vacuum_step2;
                                    }
                                    else if (_RobotCtrlMode[no] != ctrl_vacuum_step2) _RobotCtrlMode[no] = ctrl_vacuum_step1;
                                    break;
                                    
        case ctrl_vacuum_step2:     if (_Status[no].robinfo.vacuum_done)
                                    {
                                        _RobotCtrlMode[no] = ctrl_vacuum_step3;
                                        steplb_rob_to_fct_pos(no, rob_fct_cap);
                                    }
                                    break;
                                    
        case ctrl_vacuum_step3:		if (steplb_rob_in_fct_pos(no, rob_fct_cap)) _RobotCtrlMode[no] = ctrl_vacuum_step4;
									break;
                                    
        case ctrl_vacuum_step4:		 _RobotCtrlMode[no] = ctrl_off;
									break;

		case ctrl_off:              _RobotCtrlMode[no] = ctrl_off;
                                    break;
		default: return;
		
		}
		if (_RobotCtrlMode[no] != old && _RobotCtrlMode[no] != ctrl_off)
		{
            _send_ctrlMode(_RobotCtrlMode[no], no);
        }		
	}
}

//--- _send_ctrlMode ---------------------------------------------
static void _send_ctrlMode(EnFluidCtrlMode ctrlMode, int no)
{
    if (fluid_get_ctrlMode(2*no) != ctrlMode)
        fluid_send_ctrlMode(2 * no, ctrlMode, TRUE);

    if (RX_Config.inkSupplyCnt % 2 == 0)
    {
        if (fluid_get_ctrlMode(2 * no + 1) != ctrlMode)
            fluid_send_ctrlMode(2 * no + 1, ctrlMode, TRUE);
    }
    else if (no != 0)
    {
        if (fluid_get_ctrlMode(2 * no - 1) != ctrlMode)
        fluid_send_ctrlMode(2 * no - 1, ctrlMode, TRUE);
    }
    
}

//--- steplb_adjust_heads ------------------------------------------------
void steplb_adjust_heads(RX_SOCKET socket, SHeadAdjustmentMsg *headAdjustment)
{
//    SHeadAdjustment msg;
    int stepperno=0;    // Variable is not set!!!

    if (RX_Config.inkSupplyCnt % 2 == 0)
        stepperno = headAdjustment->printbarNo / 2;
    else
        stepperno = (headAdjustment->printbarNo + 1) / 2;

    if (headAdjustment->printbarNo < 0 || headAdjustment->printbarNo >= RX_Config.colorCnt)
    {
        Error(ERR_CONT, 0, "Printbar %d is not existing", headAdjustment->printbarNo+1);
        return;
    }
    if (headAdjustment->headNo == -1 && headAdjustment->axis == AXE_ANGLE)
    {
        Error(ERR_CONT, 0, "Angle-Screw is not existing at Head-No: %d", headAdjustment->headNo+1);
        return;
    }
    if (headAdjustment->headNo == RX_Config.headsPerColor-1 && headAdjustment->axis >= AXE_STITCH)
    {
        Error(ERR_CONT, 0, "Last screw of each color is pointless to turn");
        return;
    }
    
    if (_Status[stepperno].screwerinfo.screwer_ready && !(_HeadAdjustmentBuffer[stepperno][0].printbarNo != -1 && _Status[stepperno].info.z_in_screw))
    {
        _set_screw_pos(stepperno);
        _HeadAdjustment[stepperno] = *headAdjustment;
        if (RX_Config.inkSupplyCnt % 2 == 0 || (RX_Config.inkSupplyCnt == 7 && headAdjustment->printbarNo == 0))
            headAdjustment->printbarNo %= 2;
        else
            headAdjustment->printbarNo = (headAdjustment->printbarNo + 1) % 2;
        sok_send(&_step_socket[stepperno], headAdjustment);
    }   
    else
    {
        int i;
        
        for (i = 0; i < SIZEOF(_HeadAdjustmentBuffer[stepperno]); i++)
        {
            if (_HeadAdjustmentBuffer[stepperno][i].printbarNo == -1 || (_HeadAdjustmentBuffer[stepperno][i].axis == headAdjustment->axis && _HeadAdjustmentBuffer[stepperno][i].headNo == headAdjustment->headNo &&
                     _HeadAdjustmentBuffer[stepperno][i].printbarNo == headAdjustment->printbarNo))
            {
                if (_HeadAdjustmentBuffer[stepperno][i].printbarNo != -1)
                    Error(LOG, 0, "Delete Screw-Movement of Printbar %d, Head %d and Axis %d with %d Steps", headAdjustment->printbarNo+1, headAdjustment->headNo+1, headAdjustment->axis, _HeadAdjustmentBuffer[stepperno][i].steps);
                _HeadAdjustmentBuffer[stepperno][i].axis = headAdjustment->axis;
                _HeadAdjustmentBuffer[stepperno][i].headNo = headAdjustment->headNo;
                _HeadAdjustmentBuffer[stepperno][i].printbarNo = headAdjustment->printbarNo;
                _HeadAdjustmentBuffer[stepperno][i].steps = headAdjustment->steps;
                _HeadAdjustmentBuffer[stepperno][i].hdr = headAdjustment->hdr;
                Error(LOG, 0, "Save Screw-Movement of Printbar %d, Head %d and Axis %d with %d Steps", headAdjustment->printbarNo+1, headAdjustment->headNo+1, headAdjustment->axis, headAdjustment->steps);
                i = SIZEOF(_HeadAdjustmentBuffer[stepperno]);
            }
        }
    }
}

//--- _check_screwer --------------------------------------------------
static void _check_screwer(void)
{
    int i, j;

    SHeadAdjustmentMsg headAdjustment;
    for (i = 0; i < SIZEOF(_HeadAdjustmentBuffer); i++)
    {
        for (j = 1; j < SIZEOF(_HeadAdjustmentBuffer[i]); j++)
        {
            if (_HeadAdjustmentBuffer[i][j-1].printbarNo == -1 && _HeadAdjustmentBuffer[i][j].printbarNo != -1)
            {
                _HeadAdjustmentBuffer[i][j - 1] = _HeadAdjustmentBuffer[i][j];
                _HeadAdjustmentBuffer[i][j].printbarNo = -1;
            }
        } 
    }
    
    for (i = 0; i < SIZEOF(_HeadAdjustmentBuffer); i++)
    {
        if (_ScrewCommandSend[i] == TRUE && _Status[i].screwerinfo.screwer_ready == FALSE)
            _ScrewCommandSend[i] = FALSE;
        
        if (_HeadAdjustmentBuffer[i][0].printbarNo != -1 && RX_PrinterStatus.printState == ps_ready_power && (_RobotCtrlMode[i] == ctrl_off || _RobotCtrlMode[i] == ctrl_undef) &&
                _Status[i].info.z_in_screw && _Status[i].info.ref_done && _Status[i].screwerinfo.screwer_ready && _ScrewCommandSend[i] == FALSE)
        {
            headAdjustment = _HeadAdjustmentBuffer[i][0];
            _HeadAdjustmentBuffer[i][0].printbarNo = -1;
             steplb_adjust_heads(INVALID_SOCKET, &headAdjustment);
            _ScrewCommandSend[i] = TRUE;
        }
    }
}

//--- _check_fluid_back_pump ---------------------------------------
static void _check_fluid_back_pump(void)
{
    if (RX_Config.inkSupplyCnt % 2 == 0)
    {
        for (int i = 0; i < INK_SUPPLY_CNT; i+=2)
        {
            if (fluid_get_ctrlMode(i) != ctrl_off && _Status[i/2].inkinfo.ink_pump_error_left)
                fluid_send_ctrlMode(i, ctrl_off, TRUE);
            if (fluid_get_ctrlMode(i+1) != ctrl_off && _Status[i/2].inkinfo.ink_pump_error_right)
                fluid_send_ctrlMode(i+1, ctrl_off, TRUE);
        }
    }
    else if (RX_Config.inkSupplyCnt % 2 == 1)
    {
        for (int i = 0; i < INK_SUPPLY_CNT; i+=2)
        {
            if (i == 0 && RX_Config.colorCnt == 7)
            {
                if (fluid_get_ctrlMode(i) != ctrl_off && _Status[i/2].inkinfo.ink_pump_error_left)
                    fluid_send_ctrlMode(i, ctrl_off, TRUE); 
            }
            else if (i == 0 && RX_Config.colorCnt != 7)
            {
                if (fluid_get_ctrlMode(i) != ctrl_off && _Status[i/2].inkinfo.ink_pump_error_right)
                    fluid_send_ctrlMode(i, ctrl_off, TRUE);
            }
            else if ( i > 0)
            {
                if (fluid_get_ctrlMode(i) != ctrl_off && _Status[i/2].inkinfo.ink_pump_error_right)
                    fluid_send_ctrlMode(i, ctrl_off, TRUE);
                if (fluid_get_ctrlMode(i-1) != ctrl_off && _Status[(i+1)/2].inkinfo.ink_pump_error_left)
                    fluid_send_ctrlMode(i-1, ctrl_off, TRUE);
            }
        }
    }
}

//--- steplb_set_autocapMode --------------------------------------------------------
void steplb_set_autocapMode(int state)
{
    _AutoCapMode = state;
}

void steplb_set_fluid_off(int no)
{
    if (_RobotCtrlMode[no/2] != ctrl_off && RX_Config.inkSupplyCnt % 2 == 0 && 
            ((no %2 == 0 && (fluid_get_ctrlMode(no+1) < ctrl_cap || fluid_get_ctrlMode(no+1) > ctrl_wash_step6 || (fluid_get_ctrlMode(no+1) >= ctrl_cap && fluid_get_ctrlMode(no+1) <= ctrl_cap_step6))) || 
            (no %2 == 1 && (fluid_get_ctrlMode(no-1) < ctrl_cap || fluid_get_ctrlMode(no-1) > ctrl_wash_step6 || (fluid_get_ctrlMode(no-1) >= ctrl_cap && fluid_get_ctrlMode(no-1) <= ctrl_cap_step6)))))
    {
        _RobotCtrlMode[no / 2] = ctrl_off;
        _send_ctrlMode(ctrl_off, no/2);
		steplb_rob_control(ctrl_off, no / 2);
        steplb_rob_stop(no / 2);
        
        
    }
	else if (_RobotCtrlMode[(no+1)/2] != ctrl_off && RX_Config.inkSupplyCnt % 2 == 1 && 
                 (no == 0 || (no %2 == 0 && (fluid_get_ctrlMode(no-1) < ctrl_cap || fluid_get_ctrlMode(no-1) > ctrl_wash_step6 || (fluid_get_ctrlMode(no-1) >= ctrl_cap && fluid_get_ctrlMode(no-1) <= ctrl_cap_step6))) || 
            (no %2 == 1 && (fluid_get_ctrlMode(no+1) < ctrl_cap || fluid_get_ctrlMode(no+1) > ctrl_wash_step6 || (fluid_get_ctrlMode(no+1) >= ctrl_cap && fluid_get_ctrlMode(no+1) <= ctrl_cap_step6)))))
    {
        _RobotCtrlMode[(no+1) / 2] = ctrl_off;
        _send_ctrlMode(ctrl_off, (no+1) / 2);
        steplb_rob_control(ctrl_off, (no + 1) / 2);
        steplb_rob_stop((no + 1) / 2);
    }
}
