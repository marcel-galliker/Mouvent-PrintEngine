// ****************************************************************************
//
//	rx_csv.h
//
// ****************************************************************************
//
//	Copyright 2016 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#include <stdlib.h>
#include <stdio.h>
#include "rx_csv.h"

#define SEPARATOR	';'

//--- csv_get_int -------------------------------------------
int  csv_get_int(const char *str, int col)
{
	const char *ch;
	int value=0;
	for (ch=str; *ch; ch++)
	{
		if (col==1)
		{
			if (*ch==SEPARATOR) return -1;
			while (*ch!=SEPARATOR)
			{
				if (*ch>='0' && *ch<='9') value=10*value+*ch-'0';
				ch++; 
			}
			return value;
		}
		if (*ch==SEPARATOR) col--;
	}
	return -1;
}

//--- csv_put -------------------------------------
void csv_put(const char *instr, int col, const char *value, char *outstr)
{
	const char *inch;
	char	   *outch;
	
	for (inch=instr, outch=outstr; *inch; inch++)
	{
		if (col==1)
		{
			sprintf(outch, "%s%s", value, inch);
			return;
		}
		*outch++=*inch;
		if (*inch==SEPARATOR) col--;
	}
}