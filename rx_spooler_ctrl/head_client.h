// ****************************************************************************
//
//	head_client.h
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

#include "rx_sok.h"

int hc_start(void);
int hc_end(void);

int  hc_in_simu(void);
int  hc_head_board_cfg(RX_SOCKET socket, SHeadBoardCfg* cfg);
void hc_start_printing(void);
void hc_abort_printing(void);
int hc_send_next(void);

void hc_check(void);
void hc_stress_test(int board);

