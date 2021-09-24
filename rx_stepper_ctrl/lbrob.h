// ****************************************************************************
//
//	DIGITAL PRINTING - lbrob.h
//
//	Communication between ARM processor and FPGA
//
// ****************************************************************************
//
//	Copyright 2016 by radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once

#include "rx_sok.h"

#define SLIDE_MAINTENANCE_POS   -770000     //  um LB702
#define SLIDE_CAP_POS           -687000     //	um LB702
#define SLIDE_WASH_POS_FRONT    -634000     //	um LB702
#define SLIDE_WASH_POS_BACK     -221000     //	um LB702
#define SLIDE_PURGE_POS_BACK    -311000     //  um LB702
#define SLIDE_VACUUM_POS        -400000     //  um LB702
#define SLIDE_PURGE_POS_FRONT   -622000     //  um LB702    SLIDE_PURGE_POS_BACK - (7 * HEAD_WIDTH) - 10000 ->  HEAD_WIDTH = 43000
#define SLIDE_SCREW_POS_FRONT   -503000     //  um LB702    
#define SLIDE_SCREW_POS_BACK    -156208     //  um LB702    SLIDE_SCREW_POS_BACK + (8 * HEAD_WIDTH) ->  HEAD_WIDTH = 43349

void lbrob_init(int robotUsed);
void lbrob_display_status(void);	
void lbrob_menu(int help);
void lbrob_handle_menu(char *str);
void lbrob_main(int ticks, int menu);
	
int  lbrob_handle_ctrl_msg(RX_SOCKET socket, int msgId, void *pdata);

void lbrob_reference_slide(void);
void lbrob_cln_move_to(int pos);
void lbrob_move_to_pos(int cmd, int pos, int wipe_state);

void lbrob_reset_variables(void);
void lbrob_stop(void);

