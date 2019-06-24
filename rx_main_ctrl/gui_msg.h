// ****************************************************************************
//
//	GuiMsg.h
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

int handle_gui_msg(RX_SOCKET socket, void *pmsg, int len, struct sockaddr *sender, void *par);

