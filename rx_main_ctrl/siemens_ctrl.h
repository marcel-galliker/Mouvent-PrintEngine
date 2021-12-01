// ****************************************************************************
//
//	lh702_ctrl.h
//
//	main controller tasks.
//
// ****************************************************************************
#pragma once

void *siemens_thread(void *lpParameter);
int siemens_set_printpar(SPrintQueueItem *pitem);

int siemens_stop_printing();
int siemens_pause_printing();

void siemens_error_reset(void);

int opcua_get_plc_value(char* name, char* answer);
void opcua_set_plc_value(char *name, char *answer);

int opcua_get_speed();
