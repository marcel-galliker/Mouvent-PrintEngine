// ****************************************************************************
//
//	DIGITAL PRINTING - enc_ctrl.h
//
//	head-control-client
//
// ****************************************************************************
//
//	Copyright 2016 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once


int ctrl_init(void);
int ctrl_end(void);
int ctrl_main(int ticks, int menu);

int ctrl_connected(void);
int ctrl_requests(void);
int ctrl_replies(void);

void ctrl_simu(void);


