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
void lh702_on_error(ELogItemType type, char *deviceStr, int no, char *msg);
void lh702_menu(char *str);

void lh702_load_material(char *name);
void lh702_save_material(char *varList);
void lh702_set_printpar(SPrintQueueItem *pitem);
void lh702_start_printing(void);
void lh702_pause_printing(int fromGui);
void lh702_stop_printing(void);
void lh702_abort_printing(void);

void lh702_error_reset(void);


