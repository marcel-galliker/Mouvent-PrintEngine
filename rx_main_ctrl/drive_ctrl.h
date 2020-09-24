// ****************************************************************************
//
//	plc_ctrl.h
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once

int drive_init(void);
void drive_error_reset(void);
int drive_set_printbar(SPrintQueueItem *pItem);
UINT32 drive_get_scanner_pos(void);
int drive_start_printing(void);
void drive_stop_printing(void);
void drive_abort_printing(void);
int drive_handle_status(void);
void drive_move_homing(void);
void drive_move_waste(void);
void drive_move_start(void);
void drive_move_jet(void);
void drive_move_back(void);
void drive_move_table(void);
int drive_in_waste(void);
void drive_jog_cmd(int cmd);
