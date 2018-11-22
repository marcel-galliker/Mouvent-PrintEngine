// ****************************************************************************
//
//	print_queue.h	
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

#include "rx_common.h"
#include "rx_def.h"

int	pq_init(void);
int pq_end(void);

int pq_load(const char *filepath);
int pq_save(const char *filepath);

SPrintQueueItem *pq_get_item_n(int i);
SPrintQueueItem *pq_get_item(SPrintQueueItem *pitem);
SPrintQueueItem *pq_add_item(SPrintQueueItem *pitem);
SPrintQueueItem *pq_set_item(SPrintQueueItem *pitem);
int				 pq_del_item(SPrintQueueItem *pitem);
int				 pq_move_item(SPrintQueueItem *pitem, int d);

int	pq_start(void);
int pq_stop(void);
int pq_abort(void);

SPrintQueueItem *pq_get_next_item(void);

int pq_preflight(SPrintQueueItem *pitem);
int pq_ripping(SPrintQueueItem *pitem);
int pq_loading(int spoolerNo, SPageId *pid, char *txt);
int pq_sending(int spoolerNo, SPageId *pid);
int pq_printed(SPageId *pid, int *pageDone, int *jobDone, SPrintQueueItem **pnextItem);
int pq_stopping(SPrintQueueItem *pitem);
SPrintQueueItem *pq_sent(SPageId *pid);
SPrintQueueItem *pq_got_printGo(SPageId *pid);
int  pq_is_ready2print(SPrintQueueItem *pitem);
int  pq_is_ready(void);


