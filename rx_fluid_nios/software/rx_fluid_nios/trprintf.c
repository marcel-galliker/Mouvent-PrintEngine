// ****************************************************************************
//
//	DIGITAL PRINTING - version.c
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#include <stdio.h>
#include <stdarg.h>
#include "trprintf.h"

static int _DEBUG = 0;

//--- tr_debug --------------------
void tr_debug(void)
{
	_DEBUG = 1;
}

//--- tr_debug_on ----------------
int  tr_debug_on(void)
{
	return _DEBUG;
}

//--- trprintf --------------------------
int trprintf(const char *format, ...)
{
	if (_DEBUG)
	{
		int ret;
		va_list ap;
		va_start(ap, format);
		ret = vprintf(format, ap);
		va_end(ap);
		return ret;
	}
	return 0;
}

