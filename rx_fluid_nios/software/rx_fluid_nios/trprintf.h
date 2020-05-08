// ****************************************************************************
//
//	DIGITAL PRINTING - trprintf.c
//
// ****************************************************************************
//
//	Copyright 2016 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#ifndef _TRPRINTF_H_
#define _TRPRINTF_H_

void tr_debug(void);
int  tr_debug_on(void);

int trprintf(const char *format, ...);

#endif
