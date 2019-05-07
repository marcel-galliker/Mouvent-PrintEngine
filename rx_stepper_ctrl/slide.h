// ****************************************************************************
//
//	DIGITAL PRINTING - linear_drive.h
//
//
// ****************************************************************************
//
//	Copyright 2016 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once

void slide_init(void);
void slide_end(void);

void slide_main(int ticks, int menu);

int  slide_ref_done(void);
int  slide_move_done(void);
int  slide_error(void);

int	 slide_get_pos(void);

void slide_set_speed(int speed);
void slide_start_ref(void);
void slide_scan_right(void);
void slide_scan_left(void);
void slide_move_table(void);
void slide_move_adjust(int head);
void slide_stop(void);

