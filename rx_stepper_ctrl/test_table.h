// ****************************************************************************
//
//	DIGITAL PRINTING - test_table.h
//
//
// ****************************************************************************
//
//	Copyright 2016 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once

#include "rx_sok.h"

//--- test table I/O -----------------------------------

#define TT_CAP_UP_IN			0x00000001	//	in00
#define TT_CAP_DOWN_IN			0x00000002	//	in01
#define TT_MOTOR_Z_REF_IN		0x00000004	//	in02
#define TT_HEAD_DOWN_IN			0x00000008	//	in03
#define TT_COVER_CLOSED_IN		0x00000010	//	in04
#define TT_ESTOP_IN				0x00000020	//	in05

#define SLIDE_RUNNIGN_IN		0x00000040	//	in06
#define SLIDE_ERROR_IN			0x00000080	//	in07
#define SLIDE_REF_DONE_IN		0x00000100	//	in08
#define SLIDE_MOVING_IN			0x00000200	//	in09
#define TT_END_LEFT_IN			0x00000400	//	in10
#define TT_END_RIGHT_IN			0x00000800	//	in11

#define TT_VACUUM_OUT			0x00000001	//	out00
#define TT_COOLING_OUT			0x00000002	//	out01

#define SLIDE_ENABLE			0x00000004	//	out02
#define SLIDE_SPEED0_OUT		0x00000008	//	out03
#define SLIDE_SPEED1_OUT		0x00000010	//	out04
#define SLIDE_SPEED2_OUT		0x00000020	//	out05

#define SLIDE_SPEED3_OUT		0x00000040	//	out06
#define SLIDE_CMD0_OUT			0x00000080	//	out07
#define SLIDE_CMD1_OUT			0x00000100	//	out08
#define SLIDE_CMD2_OUT			0x00000200	//	out09
#define SLIDE_CMD3_OUT			0x00000400	//	out10
#define SLIDE_CMD_EXEC_OUT		0x00000800	//	out11

//--- functions ----------------------------------------------------

void tt_init(void);
void tt_end(void);
int  tt_menu(void);
void tt_main(int ticks, int menu);

int tt_handle_ctrl_msg(RX_SOCKET socket, int msgId, void *pdata);

int  tt_ref_done(void);

void tt_set_scan_par(STestTableScanPar *par);

int  tt_start_cmd(int cmd);

void tt_output(int no);

