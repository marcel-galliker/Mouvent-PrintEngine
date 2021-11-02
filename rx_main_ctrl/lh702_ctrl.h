// ****************************************************************************
//
//	lh702_ctrl.h
//
//	main controller tasks.
//
// ****************************************************************************
//
//	Copyright 2019 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

void lh702_init(void);
void lh702_end(void);

void lh702_load_material(char *name);
void lh702_save_material(char *varList);
int lh702_set_printpar(SPrintQueueItem *pitem);

int lh702_stop_printing();
int lh702_pause_printing();

void lh702_error_reset(void);

void lh702_ctr_init();
void lh702_ctr_add(int mm, UINT32 colors);
void lh702_ctr_save(int reset, char *machineName);

int opcua_get_plc_value(char* name, char* answer);
void opcua_set_plc_value(char *name, char *answer);

int opcua_get_speed();
