// ****************************************************************************
//
//	DIGITAL PRINTING - robot_client.h
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

void rc_init(void);
void rc_config(int boardNo);
void rc_main(int ticks, int menu);
void rc_display_status(void);	
void rc_menu(int help);
void rc_handle_menu(char *str);
int	rc_isConnected(void);

void rc_reference(void);
void rc_moveto_xy(int x, int y);
void rc_move_done(void);
void rc_move_top(void);
void rc_move_bottom(void);
void rc_move_up(void);
void rc_turn(int steps); 


