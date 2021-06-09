// ****************************************************************************
//
//	DIGITAL PRINTING - head_ctrl.h
//
//	head-control-client
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once

int ctrl_init(void);
int ctrl_end(void);

int ctrl_main(int ticks, int menu);

char *ctrl_cmd_name(int cmd);

int ctrl_send_2(INT32 id, UINT32 dataLen, void *data);
void ctrl_send_status(void);