// ****************************************************************************
//
//	step_tts.h		Test Table Seon Control
//
// ****************************************************************************
//
//	Copyright 2016 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

#include "rx_sok.h"

void steptts_init(int no, RX_SOCKET psocket);
int steptts_in_top_pos(void);
int steptts_handle_status(int no, SStepperStat* pstatus);
int steptts_handle_gui_msg(RX_SOCKET socket, UINT32 cmd, void *data, int dataLen);
void steptts_to_print_pos(void);
void steptts_to_top_pos(void);
void do_fluid_tts(RX_SOCKET socket, SValue* msg);
void steptts_rob_stop(void);
void tts_error_reset(void);