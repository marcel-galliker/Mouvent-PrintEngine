// ****************************************************************************
//
//	rx_threads.h
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

#include "rx_common.h"

#ifdef __cplusplus
extern "C" {
#endif

//--- RX_StartThread --------------------------------------------------

typedef void* (*thread_routine)	(void *par);

HANDLE rx_thread_start(thread_routine thread, void *param, UINT32 stacksize, const char *name);

HANDLE rx_mutex_create(void);
int    rx_mutex_destroy(HANDLE *mutex);
int    rx_mutex_lock(HANDLE mutex);
int    rx_mutex_unlock(HANDLE mutex);

HANDLE rx_sem_create();
int    rx_sem_destroy(HANDLE *sem);
int	   rx_sem_wait(HANDLE sem, int timeout);
int	   rx_sem_post(HANDLE sem);

int	   rx_sleep(UINT32 ms);
// int	   rx_get_ticks(void);

int    rx_process_start			(const char *process, const char *params);
int    rx_process_execute		(const char *process, const char *outPath, int timeout);
int    rx_process_get_id		(const char *process, const char *arg);
int    rx_process_kill 			(const char *process, const char *arg);
int    rx_process_running_cnt	(const char *process, const char *arg);

void   rx_run_in_backgrund();
int    rx_startup(const char* name, int debug);
void   rx_set_process_priority(int priority);	// linux: -20=highest, 19=lowest prio
int    rx_set_tread_priority(int priority);		// linux: 1=lowest, 99=highest

void   rx_exit(int exitCode);


#ifdef __cplusplus
}
#endif
