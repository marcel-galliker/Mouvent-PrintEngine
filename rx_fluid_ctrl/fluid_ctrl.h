// ****************************************************************************
//
//	DIGITAL PRINTING - fluid_ctrl.h
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

#include "rx_sok.h"

int ctrl_init(void);
int ctrl_end(void);
int ctrl_connected(void);

RX_SOCKET ctrl_socket(void);	

int ctrl_main(int ticks, int menu);

