// ****************************************************************************
//
//	DIGITAL PRINTING - tts_lift.h
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

void tts_lift_init(void);
void tts_lift_main(int ticks, int menu);
int tts_lift_menu(void);
int tts_lift_handle_ctrl_msg(RX_SOCKET socket, int msgId, void *pdata);
