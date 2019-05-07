// ****************************************************************************
//
//	rx_system.c
//
// ****************************************************************************
//
//	Copyright 2015 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#include <stdio.h>
#include <string.h>
#include "rx_system.h"

//--- rx_sys_cpu_id ---------------------------
#ifdef linux
void rx_sys_cpu_id(char *str, int str_size)
{
	*str=0;
	FILE  *f = popen("dmidecode -s system-uuid", "re");
	if (f!=NULL)
	{
		fread(str, 1, str_size, f);
		if (strlen(str)>0) str[strlen(str)-1]=0; 
		fclose(f);
	}
}
#endif

//--- rs_sys_disk_id ----------------------------
#ifdef linux
void rs_sys_disk_id(const char *device, char *str, int str_size)
{
	char s[128];
	char *ch, *res;
	char cmd[128];

	*str=0;
	res = str;
	sprintf(cmd, "blkid %s", device);
	FILE  *f = popen(cmd, "re");
	if (f!=NULL)
	{
		fread(s, 1, sizeof(s), f);
		for (ch=s; *ch; ch++)
		{
			if (*ch=='"')
			{
				ch++;
				while (*ch!='"')
				{
					if (--str_size<0) return;
					*res++=*ch++;
				}
				*res=0;
				return;
			}
		}
		fclose(f);
	}
}
#endif
