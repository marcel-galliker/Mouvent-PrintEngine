// ****************************************************************************
//
//	DIGITAL PRINTING - tts_ink.h
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
void tts_ink_init(void);
void tts_ink_main(int ticks, int menu);
int tts_ink_menu(void);
int tts_ink_handle_ctrl_msg(RX_SOCKET socket, int msgId, void* pdata);