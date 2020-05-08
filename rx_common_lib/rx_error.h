// ****************************************************************************
//
//	DIGITAL PRINTING - main
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
#include "rx_sok.h"

#ifdef __cplusplus
extern "C"{
#endif

typedef HANDLE err_Handle;
// typedef HANDLE log_Handle;

#define LOG			LOG_TYPE_LOG,	__FILE__,__LINE__
#define WARN		LOG_TYPE_WARN,	__FILE__,__LINE__
#define ERR_CONT	LOG_TYPE_ERROR_CONT,	__FILE__,__LINE__
#define ERR_STOP	LOG_TYPE_ERROR_STOP,	__FILE__,__LINE__
#define ERR_ABORT	LOG_TYPE_ERROR_ABORT,	__FILE__,__LINE__
#define ERR(TYPE)	TYPE,	                __FILE__,__LINE__ 
	
typedef void (*ErrorHandler)(ELogItemType errType, char *deviceStr, int no, char *msg);

void err_SetHandler(ErrorHandler OnError);

void err_init(int log, int listSize);
void err_end(void);
void err_set_device(EDevice device, int no);
void err_set_server(HANDLE hServer);
void err_set_client(RX_SOCKET socket);
char *err_system_error(int no, char *buffer, int size);

SLogItem  *err_get_log_item(int no);

void err_clear_all(void);

int  Error	    (						 ELogItemType type, const char *file, int line, int errNo, const char *format, ...);
int  ErrorFlag  (						 ELogItemType type, const char *file, int line, UINT32 *flags, UINT32 flag, int errNo, const char *format, ...);
int  ErrorEx	(EDevice device, int no, ELogItemType type, const char *file, int line, int errNo, const char *format, ...);
int  ErrorExFlag(EDevice device, int no, ELogItemType type, const char *file, int line, UINT32 *flags, UINT32 flag, int errNo, const char *format, ...);

int  SlaveError(EDevice device, int no, SLogItem *msg);

void err_get_state(ELogItemType *pState, char *text, int size);
	
void compose_message(EDevice deviceType, int deviceNo, int errNo, char *str, int strSize, const char *format, const BYTE *arg);


#ifdef __cplusplus
}
#endif
