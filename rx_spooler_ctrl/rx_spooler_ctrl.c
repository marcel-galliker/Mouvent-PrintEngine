// ****************************************************************************
//
//	RxSpoolCtrl.c
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#include "tcp_ip.h"
#include "rx_common.h"
#include "rx_error.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "args.h"
#include "ctrl_client.h"
#include "data_client.h"
#include "head_client.h"	
#include "rs_tec-it.h"

//--- global variables -----------------------------

int				RX_SpoolerNo = 0;
SSpoolerCfg		RX_Spooler;
SColorSplitCfg	RX_Color[MAX_COLORS];
char			RX_TestData[MAX_COLORS*MAX_HEADS_COLOR][MAX_TEST_DATA_SIZE];
static char _IpAddrMain[64] = RX_CTRL_MAIN;

//---  test_read_file ---------------------------------
static void test_read_file(char *path, int *psize, int *ptime, BYTE *buffer, int bufsize)
{
	FILE *file;
	int len, time;
	
	*psize=0;
	*ptime=0;
	time = rx_get_ticks();
	file = fopen(path,"rb");
	if (file)
	{
		do 
		{
			len=(int)fread(buffer, 1, bufsize, file);
			*psize+=len;
		} while (len);
		fclose(file);
		*ptime = rx_get_ticks()-time;
	}
}

//--- test_disk_speed --------------------------------
static void test_disk_speed(char *root)
{
	int time, i, size, ttot;
	int bufsize = 0x1000;
	char path[MAX_PATH];
	BYTE *buffer = (BYTE*)malloc (bufsize);
	
	/*
	SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
	*/

	#ifdef linux
		printf("test_disk_speed linux Blocksize=0x%x, root >>%s<<\n", bufsize, root);
	#else
		printf("test_disk_speed Windows Blocksize=0x%x\n", bufsize);
	#endif

	ttot = 0;
	for (i=0; i<=9; i++)
	{
		sprintf(path, "%sfile%d.usf", root, i);
		test_read_file(path, &size, &time, buffer, bufsize);
		if (size>0)
		{
			ttot += time;
			printf("File %d:\tSize=%d MB\tTime=%d ms\tSpeed=%d MB/s\n", i, size/1024/1024, time, (int)(size*1000.0/time/1024/1024));
		}
		else printf("File Read Error\n");
	}
	printf("---------------------------------------------------------\n");
	printf("Average:\t\t\tTime=%d ms\tSpeed=%d MB/s\n", ttot/i, (int)(size*1000.0/(ttot/i)/1024/1024));
	
	//--- cache -----------------------------------------------------------
	printf("\nTest cache\n");
	ttot=0;
	for (i=0; i<=9; i++)
	{
		test_read_file(path, &size, &time, buffer, bufsize);
		if (size>0)
		{
			ttot += time;
			printf("File %d:\tSize=%d MB\tTime=%d ms\tSpeed=%d MB/s\n", i, size/1024/1024, time, (int)(size*1000.0/time/1024/1024));
		}
		else printf("File Read Error\n");
	}
	printf("---------------------------------------------------------\n");
	printf("Average:\t\t\tTime=%d ms\tSpeed=%d MB/s\n", ttot/i, (int)(size*1000.0/(ttot/i)/1024/1024));
}

//--- _spoolerNo -------------------------------------------------------
int _spoolerNo(void) 
{
	UINT32 ipaddr;
	UINT32 no;

	sok_get_ip_address_num("eth0", &ipaddr);
	if (!ipaddr) sok_get_ip_address_num("em2", &ipaddr);

	if (ipaddr == sok_addr_32(RX_CTRL_MAIN)) return 0;

	no = (ipaddr>>24)-(sok_addr_32(RX_CTRL_SPOOL)>>24);
	if (no>1 && no<50) return no;
	return 1;
}

//--- main ------------------------------------------------------------------
int main(int argc, char* argv[])
{
	// agruments
	//	-ipaddr <addr>

	char ch;

	rx_process_name(argv[0]);
	args_init(argc, argv);
	Trace_init(argv[0]);
	rx_startup(argv[0], arg_debug);

//	rx_set_process_priority(-10);

	RX_SpoolerNo = _spoolerNo();

//	Trace_to_screen(FALSE);
//	Trace_to_file(FALSE);

	printf("Process >>%s<< started\n", argv[0]);
	err_set_device(dev_spooler, RX_SpoolerNo);
	err_init(FALSE, 100);
	rx_init();
	rx_def_init();

	tecit_version();	// is used to link the library (linux) !!!

	#ifdef linux
//		test_do();
	#endif

	#ifdef WIN32
		strcpy(_IpAddrMain, "localhost");	
	#endif
	if (*arg_ipAddr) strcpy(_IpAddrMain, arg_ipAddr);
	ctrl_start(_IpAddrMain);

	printf("press \"X\" to stop\n");
	while (TRUE)
	{
		rx_sleep(1000);
		hc_check();
		ch=getchar_nolock();
		if (ch=='X') break;
	}

	//--- end libraries ----------------------------
	ctrl_end();
	rx_end();

	printf("process stopped\n");
	return 0;
}


