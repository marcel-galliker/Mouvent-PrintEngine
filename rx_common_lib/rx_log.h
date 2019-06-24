// ****************************************************************************
//
//	DIGITAL PRINTING - rx_log.h
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once

#include "rx_common.h"
#include "rx_def.h"


#ifdef __cplusplus
extern "C"{
#endif

typedef HANDLE log_Handle;

void log_open(const char *filename, log_Handle *handle, int write);
// openes a file for reading or writing


void log_add_item(log_Handle handle, SLogItem *item);
// adds an item at the end of the file 

int  log_get_item_cnt(log_Handle handle);
int	 log_get_last_item_no(log_Handle handle);
int  log_eof(log_Handle handle, int itemNo);

int  log_get_item(log_Handle handle, UINT32 itemNo, SLogItem *item);
// reads item[itemno]

void log_save_as_xml(log_Handle handle, UINT32 itemNo, int no, HANDLE hsetup);
		
void log_close(log_Handle *handle);
// close the file

#ifdef __cplusplus
}
#endif
