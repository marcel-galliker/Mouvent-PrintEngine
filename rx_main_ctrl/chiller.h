// ****************************************************************************
//
//	chiller.h		
//
// ****************************************************************************
//
//	Copyright 2017 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once


int	 chiller_init(void);
int  chiller_end(void);
void chiller_tick(void);

void chiller_enable(int enable);
void chiller_error_reset(void);
void chiller_reply_stat(RX_SOCKET socket);	// to GUI
int  chiller_is_running(void);
