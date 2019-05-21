// ****************************************************************************
//
//	ctrl_msg.h
//
//	main controller tasks.
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

void ctrl_msg_init(void);
int handle_headCtrl_msg(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *par);
int ctrl_do_log_evt(RX_SOCKET socket, SLogMsg *msg);
