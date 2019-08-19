// ****************************************************************************
//
//	SpoolSvr.h
//
//	main controller tasks.
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

#include "rx_rip_lib.h"
#include "rx_data.h"
#include "rx_counter.h"

int  spool_start(void);
int  spool_end(void);
void spool_tick(void);


void spool_auto(int enable);

int	spool_set_config(RX_SOCKET socket);
void spool_start_printing(void);
void spool_start_sending(void);
int spool_is_ready(void);
int spool_set_layout(SLayoutDef *playout, char *dataPath);
int spool_set_filedef(SFileDef  *pfileDef);
int spool_set_counter(SCounterDef  *pctrDef);
int spool_send_msg(void *msg);
int spool_send_msg_2(UINT32 cmd, int dataSize, void *data, int errmsg);
int spool_send_test_data(int headNo, char *str);
int spool_print_file(SPageId *pid, const char *filename, INT32 offsetWidth, INT32 lengthPx, SPrintQueueItem *pitem, int clearBlockUsed);
SPageId* spool_get_id(int no);
// int spool_print_empty(void);
int spool_abort_printing(void);
int spool_head_board_cnt(void);