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

void rc_init(int used);
int  rc_used(void);
void rc_config(int boardNo);
void rc_main(int ticks, int menu);
void rc_display_status(void);	
void rc_menu(int help);
void rc_handle_menu(char *str);
int rc_handle_ctrl_msg(RX_SOCKET socket, int msgId, void *pdata);

void rx_enable_stall_error(int enable);

void rc_reference(int x);
int  rc_moveto_x(int x, const char *file, int line);
int  rc_moveto_y(int y, const char *file, int line);
int  rc_moveby_z(int z, const char *file, int line);
int  rc_move_top(const char *file, int line);
int  rc_move_bottom(const char *file, int line);
int  rc_find_screw(int spets);
void rc_stop(void);
void rc_stop_motors(int motors);

int rc_in_garage(void);
void rc_screwer_to_ref(void);
void rc_turn_ticks_left(int ticks);
void rc_turn_ticks_right(int ticks);
// void rc_turn_ticks(int ticks, int stopOnUp);

void rc_turn_steps(int steps);
void rc_set_screwer_current(int high);
int rc_screwer_stalled(void);
int rc_get_screwer_current(void);
int rc_get_screwer_pos(void);
int rc_get_screwer_steps(int steps_per_turn);
int rc_move_started(void);
int rc_move_done(void);
int rc_move_screw_done(void);
int rc_move_xy_error(void);
// void rc_reset_motors(int motors);
void rc_clear_error();