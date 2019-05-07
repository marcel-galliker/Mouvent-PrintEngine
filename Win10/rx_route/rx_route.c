// ****************************************************************************
//
//	DIGITAL PRINTING - rx_net_bridge
//
// ****************************************************************************
//
//	Copyright 2017 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

//	finds the correct IP-Address for the Teamviewer VPN (7.xxx.xxx.xxx) and adds the route

//--- includes -------------------------------------------------------
#include <Windows.h>
#include <stdio.h>

#include "lib.h"

//--- prototypes -----------------------------------------------------
static void _scan_ipAddr(char *str, int ipAddr[4])
{
	int i;
	for (i=0; i<4; i++) ipAddr[i]=0;
	sscanf(str, "%d.%d.%d.%d", &ipAddr[0], &ipAddr[1], &ipAddr[2], &ipAddr[3]);
}

//--- _route_add -----------------------------------------------------
static void _route_add(void)
{
	char path[256];
	GetTempPath(sizeof(path), path);
	strcat(path, "arp.txt");
	rx_process_execute("arp.exe -a", path, 0);

	FILE *file;
	char str[256];
	char *pos;
	int	  ipAddrLocal[4];
	int	  ipAddr[4];	

	file = fopen(path, "r");
	if (file!=NULL)
	{
		while(fgets(str, sizeof(str), file))
		{
			printf(str);
			if (pos=strstr(str, ": 7."))
			{				
				_scan_ipAddr(&pos[2], ipAddrLocal);
				printf("*** INTERFACE *** >>%s<<\n", pos);
				while (fgets(str, sizeof(str), file) && strlen(str)>2)
				{
					printf("* %s", str);
					_scan_ipAddr(&str[2], ipAddr);
					if (ipAddr[0]==ipAddrLocal[0] && (ipAddr[1]!=ipAddrLocal[1] || ipAddr[2]!=ipAddrLocal[2] || ipAddr[3]!=ipAddrLocal[3]))
					{
						if (ipAddr[1]!=255 && ipAddr[2]!=255 && ipAddr[3]!=255)
						{
							sprintf(str, "route add 192.168.200.0 mask 255.255.255.0 %d.%d.%d.%d metric 10", ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);
							printf("%s\n", str);
							rx_process_execute(str, NULL, 0);
							goto done;
						}
					}
				}
			}
		}
	done:
		fclose(file);
		DeleteFile(path);
	} 
}

///--- main ---------------------------------------------------------------
int main(int argc, char** argv)
{
	_route_add();

	return 0;
}

