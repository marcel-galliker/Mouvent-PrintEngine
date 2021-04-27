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
int  rc_moveto_xy(int x, int y, const char *file, int line);
int  rc_moveto_z(int z, const char *file, int line);
int  rc_move_top(const char *file, int line);
int  rc_move_bottom(const char *file, int line);
int  rc_move_up(const char *file, int line);
int  rc_turn(int steps, const char *file, int line);
void rc_stop(int motors);
int rc_in_ref(void);
int rc_screwer_in_ref(void);
void rc_turn_ticks_left(int ticks);
void rc_turn_ticks_right(int ticks);
void rc_turn_steps(int steps);
void rc_set_screw_current(int current);
int rc_screwer_stalled(void);
int rc_get_screw_current(void);
int rc_move_started(void);
int rc_move_done(void);