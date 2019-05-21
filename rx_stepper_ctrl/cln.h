// ****************************************************************************
//
//	DIGITAL PRINTING - cleaning.h
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

void cln_init(void);
int  cln_menu(void);
void cln_main(int ticks, int menu);

int  cln_handle_ctrl_msg(RX_SOCKET socket, int msgId, void *pdata);

