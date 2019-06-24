// ****************************************************************************
//
//	rx_trace.h
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
extern "C"{
#endif

	void Trace_init(const char *appName);
	void Trace_end(void);
	const char *Trace_get_path(void);
	void Trace_set_server(HANDLE hServer);
	void Trace_to_screen(int trace);
	void Trace_to_file(int trace);
	void TrPrintf (int level, const char *format, ...);
	void TrPrintfL(int level, const char *format, ...);
 
#ifdef __cplusplus
}
#endif
