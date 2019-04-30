// ****************************************************************************
//
//	spool_rip.c
//
// ****************************************************************************
//
//	Copyright 2015 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************


//--- includes ---------------------------------------------------------------
#include "rx_def.h"
#include "rx_tif.h"
#include "tcp_ip.h"
#include "rx_mem.h"
#include "data.h"
#include "test.h"


//	mkdir /mnt/ramdisk
//	mount -t tmpfs -o size=512m tmpfs /mnt/ramdisk
//  sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches"

static char *_FileNames[] = 
{
//	"TestA",
//	"TestB",
//	"TestC",
//	"TestD",
	"N1",
//	"N2",
//	"N34",
//	"N5",
//	"N67",
};

static UINT64	_BufferSize=0;
static BYTE*	_Buffer[MAX_COLORS];

static void _loadFiles(char *printEnv, char *fname);

//--- test_do ----------------------------------------------------------------
void test_do(void)
{
	int i, n;

	memset(RX_Color, 0, sizeof(RX_Color));
	RX_Color[0].color.colorCode =  0;	// black
	RX_Color[1].color.colorCode =  1;	// cyan
	RX_Color[2].color.colorCode =  2;	// magenta
	RX_Color[3].color.colorCode =  3;	// yellow

	for (i=0; i<SIZEOF(RX_Color); i++)
	{
		if (i==0 || RX_Color[i].color.colorCode) 
		{
			strcpy(RX_Color[i].color.name, RX_ColorNameLong(RX_Color[i].color.colorCode));
			RX_Color[i].lastLine = 0xffffffff;
		}
	}
	for (n=0; n<100; n++)
	{
		printf("***** LOOP %d ***************\n", n+1);
		for (i=0; _FileNames[i]; i++)
		{
			_loadFiles("Radex 4C Tiff", _FileNames[i]);
		}
	}
}

//--- _loadFiles ---------------------------------------------------------------
static void _loadFiles(char *printEnv, char *fname)
{
	char			filePath[MAX_PATH];
	UINT32			total, free1, free2;
	int				ret, i;
	int				time0;
	SPrintFileRep	reply;
	SBmpInfo		bmpInfo;

	total = rx_mem_get_totalMB();
	free1 = rx_mem_get_freeMB();

	printf("Loading Files >>%s<<		memory: total=%04d  free=%04d MB\n", fname, total, rx_mem_get_freeMB());
	sprintf(filePath, "%s%s/%s/%s.tif", "/opt/PrintData/", printEnv, fname, fname);
//	sprintf(filePath, "%s%s.tif", "/mnt/ramdisk/", fname);

	time0 = rx_get_ticks();

	ret = data_get_size(filePath, 0, 0, &reply.widthPx, &reply.lengthPx, &reply.bitsPerPixel);
	if (ret==REPLY_OK) ret = data_malloc (FALSE, reply.widthPx, reply.lengthPx, reply.bitsPerPixel, RX_Color, SIZEOF(RX_Color), &_BufferSize, _Buffer);
	if (ret==REPLY_OK) ret = tif_load(NULL, filePath, "", PM_SINGLE_PASS, 0, RX_Color, SIZEOF(RX_Color), _Buffer, &bmpInfo, NULL);
	free2 = rx_mem_get_freeMB();
	for (i=0; i<SIZEOF(_Buffer); i++) rx_mem_unuse(&_Buffer[i]);
	printf("Memory: total=%04d  free1=%04d  free2=%04d  free3=%04d(MB)\n", total, free1, free2, rx_mem_get_freeMB());

	printf("Load time = %d ms\n\n", rx_get_ticks()-time0);
}