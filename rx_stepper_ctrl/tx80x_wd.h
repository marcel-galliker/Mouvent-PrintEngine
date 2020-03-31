// ****************************************************************************
//
//	DIGITAL PRINTING - tx80x_wd.h
//
//	Communication between ARM processor and FPGA
//
// ****************************************************************************
//
//	Copyright 2016 by radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

void tx80x_wd_init(void);
void tx80x_wd_main(void);
void tx80x_wd_handle_menu(char *str);
void tx80x_wd_menu(int help);
void tx80x_wd_display_status(void);
int tx80x_wd_handle_ctrl_msg(RX_SOCKET socket, int msgId, void *pdata);