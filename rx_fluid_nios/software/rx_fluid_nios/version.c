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

#include "version.h"
#include "string.h"

const char version[32]="1.0.1.1105";

void get_version(SVersion *pversion)
{
	int val[4], i;
	const char *ch;
	memset(val, 0, sizeof(val));
	for (i=0,ch=version; *ch; ch++)
	{
		if (*ch=='.') i++;
		if (*ch>='0' && *ch<='9') val[i] = 10*val[i]+*ch-'0';
	}
	pversion->major 	= val[0];
	pversion->minor 	= val[1];
	pversion->revision 	= val[2];
	pversion->build 	= val[3];
};
