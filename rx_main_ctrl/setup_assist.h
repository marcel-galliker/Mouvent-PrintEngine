// ****************************************************************************
//
//	setup_assist.h		
//
// ****************************************************************************
//
//	Copyright 2020 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once


int	 sa_init(void);
int  sa_end(void);

int	sa_connected(void);
void sa_handle_gui_msg(RX_SOCKET socket, void *pmsg);
