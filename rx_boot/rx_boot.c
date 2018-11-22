// ****************************************************************************
//
//	DIGITAL PRINTING - rx_boot
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#include <stdio.h>
#include "rx_common.h"
#include "rx_def.h"
#include "rx_error.h"
#include "rx_file.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "rx_mac_address.h"
#include "tcp_ip.h"
#include "boot_client.h"
#include "file_server.h"


#ifdef soc
	#define DEVICE_NAME "eth0"
#else
	#define DEVICE_NAME "em2"
#endif

//--- _read_info -----------------------------------------------------------
static void _read_info(char *filePath, char *deviceTypeStr, int len, UINT32 *serialNo)
{
	FILE *file;

	*deviceTypeStr=0;
	*serialNo=0;
	file = fopen(filePath, "r");
	if (file)
	{
		char str[256];
		int i, n;

		while (fgets(str, sizeof(str), file))
		{
			//---  argument ---
			i=0;
			while (str[i] && str[i++]!='=');
			while (str[i] && str[i]==' ') i++;
			for (n = strlen(str); n && str[n]<=' '; n--);
			str[n+1]=0;
			if (str_start(str, "DeviceType"))
			{
				strncpy(deviceTypeStr, &str[i], len);
			}
			if (str_start(str, "SerialNo"))
			{
				*serialNo=atoi(&str[i]);
			}
			if (str_start(str, "DeviceType"))
			{
				strncpy(deviceTypeStr, &str[i], len);
			}
		}

		fclose(file);
	}

	if (!*deviceTypeStr) TrPrintfL(1, "DeviceType is missing in file >>%s<<", filePath);
	if (!*serialNo)      TrPrintfL(1, "SerialNo is missing in file >>%s<<", filePath);
}

//--- info from MAC-Address -----------------------------------------------
void _info_from_mac(UINT64 macAddr, char *deviceTypeStr, int len, UINT32 *serialNo)
{
	*deviceTypeStr=0;
	*serialNo=0;

	#ifdef soc
	UINT64 device;
	sok_get_mac_address(DEVICE_NAME, &macAddr);
	if ((macAddr & MAC_OUI_MASK) == MAC_OUI)
	{
		device = macAddr & MAC_ID_MASK;
		if		(device==MAC_HEAD_CTRL_ARM) strcpy(deviceTypeStr, "Head");
		else if (device==MAC_ENCODER_CTRL)  strcpy(deviceTypeStr, "Encoder");
		else if (device==MAC_FLUID_CTRL)	strcpy(deviceTypeStr, "Fluid");
		else if (device==MAC_STEPPER_CTRL)	strcpy(deviceTypeStr, "Stepper");
		else								sprintf(deviceTypeStr,"Device %d", device);	

		if (*deviceTypeStr) 
		{
			UINT64 no = swap_uint64(macAddr & MAC_NO_MASK);
			*serialNo = (no>>16);
		}
	}
	#else
		strcpy(deviceTypeStr, "Spooler");
	#endif
}

///--- main ---------------------------------------------------------------
int main(int argc, char** argv)
{
	char	deviceTypeStr[32];
	UINT32	serialNo;
	UINT64	macAddr;
	int		cnt1, cnt;
	char str[64];

	//--- do not start on MAIN -----------------
	sok_get_ip_address_str(DEVICE_NAME, str, sizeof(str));
	if (!strcmp(str, RX_CTRL_MAIN)) 
	{
		printf("PC is MAIN-CTRL\n");
		return 0;
	}

	//--- wait until interface is up -----------------
	sok_get_mac_address(DEVICE_NAME, &macAddr);
	_info_from_mac(macAddr, deviceTypeStr, sizeof(deviceTypeStr), &serialNo);
	
	if (!*deviceTypeStr) _read_info(PATH_ROOT "info", deviceTypeStr, sizeof(deviceTypeStr), &serialNo);

	TrPrintfL(1, "rx_boot >>%s %d<< starting",  deviceTypeStr, serialNo);

	cnt1 = rx_process_running_cnt(argv[0], NULL);
	TrPrintfL(1, "ProcessCnt >>%s<<=%d", argv[0], cnt1);

	if (argc==2 && !strcmp(argv[1], "-debug"))
	{
		TrPrintfL(1, "debug version");
		while(1)
		{
			cnt = rx_process_running_cnt(argv[0], NULL);
			if (cnt<2) break;
			TrPrintfL(1, "Kill >>%s<<", argv[0]);
			rx_process_kill(argv[0], NULL);
		}
	}
	else
	{
		TrPrintfL(1, "background version");
		if (cnt1>1)
		{
			TrPrintfL(1, "rx_boot already running, cnt=%d", cnt1);
			return 0;
		}
		rx_run_in_backgrund();
		rx_sleep(5000);
	}

	rx_set_process_priority(0);

	err_init(0, 0);
	rx_init();
	Trace_init(argv[0]);

	rx_mkdir(PATH_ROOT);

	boot_init(DEVICE_NAME, deviceTypeStr, serialNo);
	
	boot_start();

	while (1)
	{
		rx_sleep(1000);
	}

	return 0;
}
