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

#include "../rx_common_lib/version.h"
#include "string.h"

const char version[32]="1.0.1.2314";

void get_version(int ver[4])
{
	int i;
	const char *ch;
	memset(ver, 0, 4*sizeof(ver[0]));
	for (i=0,ch=version; *ch; ch++)
	{
		if (*ch=='.') i++;
		if (*ch>='0' && *ch<='9') ver[i] = 10*ver[i]+*ch-'0';
	}
};