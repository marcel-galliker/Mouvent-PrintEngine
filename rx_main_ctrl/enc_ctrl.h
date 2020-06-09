// ****************************************************************************
//
//	enc_ctrl.h
//
//	main controller tasks.
//
// ****************************************************************************
//
//	Copyright 2016 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

#include "rx_sok.h"
#include "print_queue.h"

void enc_init(void);
void enc_end(void);

void enc_tick(void);

int	 enc_set_config(int restart);
int  enc_start_printing(SPrintQueueItem *pitem, int restart);
void enc_sent_document(int pages, SPageId *pId);
int	 enc_set_pg(SPrintQueueItem *pitem, SPageId *pId);
int enc_change(void);
int  enc_stop_pg(char *reason);
int  enc_restart_pg(void);
int  enc_stop_printing(void);
int	 enc_abort_printing(void);
int	 enc_enable_printing(int enable);
int  enc_ready(void);
int  enc_speed(void);
int	 enc_simu_encoder(int khz);
void enc_error_reset(void);
void enc_reply_stat(RX_SOCKET socket);
void enc_save_par(int no);
int	 enc_pg_stop_cnt(void);

int  enc_uv_on(void);
int  enc_uv_off(void);
int  enc_is_uv_on(void);
int  enc_is_uv_ready(void);
int	 enc_is_analog(void);
int	 enc_is_connected(void);