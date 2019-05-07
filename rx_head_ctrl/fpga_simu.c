// ****************************************************************************
//
//	DIGITAL PRINTING - fpga_simu.c
//
//	Communication between ARM processor and FPGA
//
// ****************************************************************************
//
//	Copyright 2013 by radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

//--- includes ----------------------------------------------------------------
#include "Fpga.h"
#include "rx_common.h"
#include "rx_error.h"
#include "rx_sok.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "tcp_ip.h"
#include "bmp.h"
#include "fpga_def_head.h"
#include "fpga_simu.h"

//--- defines -----------------------------------------------------------------
#define SIMU	FALSE

//--- globals -----------------------------------------------------------------
int simu_blockCnt;

extern SFpga				Fpga;

static int		_Trace=0;

//--- statics -----------------------------------------------------------------
static HANDLE  	_HServer[2]={0,0};
static int	   	_Test;
static int		_Init=FALSE;
static int		_BufferSize=0;
static BYTE		*_Buffer=NULL;
static SFpgaPrintData	_PrintList;
static SPageId			_PageId[256];
static UINT32			_PdCnt;
static SFpgaUdpCfg		_Udp;

//--- prototypes --------------------------------------------------------------
static void *_fpga_thread(void *par);
static int _handle_msg(RX_SOCKET socket, void *msg, int len, struct sockaddr	*sender, void *par);
static int _check_blocks(UINT16 head, int image);

//--- simu_start --------------------------------------------------------------
int simu_start(void)
{
	int i;
	int blkCnt;

	/*
	{
		//--- test -----------------------
		struct sockaddr_in addr;
		sok_sockaddr(&addr, "192.168.200.101", 1);
		memcpy(&Fpga.cfg->eth[0].ipAddr, &addr.sin_addr, 4);
		Fpga.cfg->eth[0].ipPort = PORT_UDP_DATA;
		//--- end test -------------------
	}
	*/
	if (SIMU)
	{
		TrPrintfL(TRUE, "simu_start");

		int size = RX_HBConfig.dataBlkSize*RX_HBConfig.dataBlkCntHead;
		if (size>_BufferSize)
		{
			if  (_Buffer!=NULL) free(_Buffer);
			_Buffer = (BYTE*) malloc(size);
			_BufferSize = size;
			memset(_Buffer, 0x55, _BufferSize);
		}
		simu_blockCnt = 0;

		TrPrintfL(TRUE, "Reset BlockUsed, size=%d", (RX_HBConfig.dataBlkCntHead+7)/8);
		memset(Fpga.blockUsed,	0, (RX_HBConfig.dataBlkCntHead+7)/8);
		memset(Fpga.stat,		0, sizeof(*Fpga.stat));
		memset(Fpga.print,		0, sizeof(*Fpga.print));
		memset(&_PrintList,		0, sizeof(_PrintList));
		memset(_PageId,			0, sizeof(_PageId));
		_Udp.udp_block_size = 0;

		blkCnt = 10240;

		for (i=0; i<4; i++)
		{
			_Udp.block[i].blkNo0   = i*blkCnt;
			_Udp.block[i].blkNoEnd = (i+1)*blkCnt-1;
		}

		_PdCnt = 0;
	}

	if (SIMU )
	{
		_Test = 0;
		if (!_Init && RX_HBConfig.dataPort[0])
		{
			// RX_start_thread(_fpga_thread, 0, 0);
			_fpga_thread(NULL);
			_Init = TRUE;
		}
	}
	return REPLY_OK;
}

//--- simu_end ----------------------------------------------------------------
int simu_end(void)
{
	return REPLY_OK;
}

//--- _fpga_thread --------------------------------------------------------------
static void *_fpga_thread(void *par)
{
	int i, err=0;

	TrPrintfL(TRUE, "START _fpga_thread");

	for (i=0; i<2; i++)
	{
		if (RX_HBConfig.dataPort[i])
		{
			char addr[32];
			UCHAR no[4];
			memcpy(no, &RX_HBConfig.dataAddr[i], 4);
			sprintf(addr, "%d.%d.%d.%d", no[0], no[1], no[2], no[3]);
			err = sok_start_server(&_HServer[i], addr, PORT_UDP_DATA+i, SOCK_DGRAM, 0, _handle_msg, NULL, NULL);
			if (err)
			{
				char str[256];
				TrPrintf(1, "UDP server[%d]: Error >>%s<<", i, err_system_error(err, str, sizeof(str)));
			}
			else TrPrintfL(1, "UDP server[%d] started", i);
		}
	}
	TrPrintfL(TRUE, "END _fpga_thread");
	return NULL;
}

//--- _handle_msg ----------------------------------------------------------------------------------
static int _handle_msg(RX_SOCKET socket, void *msg, int len, struct sockaddr	*sender, void *par)
{
	UINT32   blkNo;

	memcpy(&blkNo, msg, 4);
//	if (blkNo!=_Test) TrPrintfL(1, "Matching Error, exp=%d, got=%d", _Test, blkNo);
//	if (!(blkNo%1000)) TrPrintfL(1, "Received DataBlock[%d]", blkNo);
//	if (blkNo%100==0) TrPrintfL(TRUE, "Received block %d", blkNo);
//	if (Fpga.blockUsed[blkNo>>3] & (1<<(blkNo&7))) TrPrintfL(TRUE, "Double block %d", blkNo);
	memcpy(_Buffer+blkNo*RX_HBConfig.dataBlkSize, (UCHAR*)msg+4, RX_HBConfig.dataBlkSize);
	Fpga.blockUsed[blkNo>>3] |= (1<<(blkNo&7));
	if ((blkNo%100)==0) TrPrintfL(TRUE, "got Block %d", blkNo);
	_Test = blkNo+1;
	simu_blockCnt ++;
	return REPLY_OK;
}

//--- _check_blocks ---------------------------------------------
int _check_blocks(UINT16 head, int image)
{
	SFpgaImage	*info;
	int blkNo0;
	int blkCnt;
	int blk, cnt, c;
	int missing;
	int error=FALSE;

	char str[256];
	int l, start;

	if (head<MAX_HEADS_BOARD)
	{
		blkNo0 = _Udp.block[head].blkNo0;
		blkCnt = _Udp.block[head].blkNoEnd-_Udp.block[head].blkNo0+1;

		info = &_PrintList.imageList[head].image[image];

		//--- check for missing blocks -------------------------------
		cnt = (info->widthBytes*info->lengthPx + RX_HBConfig.dataBlkSize-1)/RX_HBConfig.dataBlkSize;
		TrPrintfL(_Trace, "Block [%d..%d]", info->blkNo, info->blkNo+cnt);
		missing = -1;
		memset(str, ' ', sizeof(str));
		l	  = info->blkNo%100;
		l    += l/10;
		start = (info->blkNo/10)*10;
		for (blk=info->blkNo, c=cnt; c; c--, blk=blkNo0+((blk+1)%blkCnt))
		{			
			if ((Fpga.blockUsed[blk>>3] & (1<<(blk&7)))==0)
			{
				str[l++]='.';
				error=TRUE;
				if (missing<0) missing=blk;
			}
			else 
			{
				str[l++]='*';
				if (missing>=0)
				{
					TrPrintfL(TRUE, "Blocks %d..%d missing", missing, blk);
					missing=-1;
				}
			}
			if (blk%10==9) str[l++]=' ';
			if (blk%100==99) 
			{
				str[l]=0;
				TrPrintfL(_Trace, "blk[%03d]: %s", start, str);
				start=blk+1;
				l=0;
			}
		}
		if (missing>=0)
		{
			TrPrintfL(TRUE, "Blocks %d..%d missing", missing, blk);
			missing=-1;
			return REPLY_ERROR;
		}
	}
	return REPLY_OK;
}

//--- simu_fpga_image -------------------------------------------------------
int simu_fpga_image		(SFpgaImageCmd *msg)
{
	int				idx, cnt;
	SFpgaImageList	*list;

	if (!SIMU) return REPLY_OK;

//	TrPrintfL(TRUE, "CMD_FPGA_IMAGE head=%d", msg->head);
	
	if (RX_HBConfig.dataBlkSize==0) return Error(ERR_ABORT, 0, "dataBlkSize=0");

	if (msg->head<MAX_HEADS_BOARD)
	{
		list = &_PrintList.imageList[msg->head];
		// send it to FPGA!!!
		idx = (list->inIdx+1)&0xff;
		if (idx==list->outIdx) return Error(ERR_CONT, 0, "Image List [%d] full", msg->head);
		memcpy(&_PageId[list->inIdx], &msg->id, sizeof(SPageId));
		memcpy(&list->image[list->inIdx], &msg->image, sizeof(SFpgaImage));
		cnt = (msg->image.widthBytes*msg->image.lengthPx+RX_HBConfig.dataBlkSize-1)/RX_HBConfig.dataBlkSize;
		TrPrintfL(TRUE, "CMD_FPGA_IMAGE image[%d.%d]: id=%d, page=%d, copy=%d, scan=%d, block0=%d..%d, blockCnt=%d, widthPx=%d, lengthPx=%d", msg->head, list->inIdx, msg->id.id,  msg->id.page, msg->id.copy, msg->id.scan, msg->image.blkNo, msg->image.blkNo+cnt, cnt, msg->image.widthPx, msg->image.lengthPx);

		list->inIdx = idx;
	}
	return REPLY_OK;
}


//--- _simu_write_image ---------------------------------------------
static int _simu_write_image(UINT16 head, UINT16 image, SPageId *pid)
{
	char filename[256];
	SFpgaImage	*info;
	int blkNo0;
	int blkCnt;
	int blk, cnt;

	if (head<MAX_HEADS_BOARD)
	{
		blkNo0 = _Udp.block[head].blkNo0;
		blkCnt = _Udp.block[head].blkNoEnd - _Udp.block[head].blkNo0 + 1;

		info = &_PrintList.imageList[head].image[image];
		cnt  = (info->widthBytes*info->lengthPx + RX_HBConfig.dataBlkSize-1)/RX_HBConfig.dataBlkSize;

		//--- write the bitmap --------------------------------------
		sprintf(filename, "%s/print_%d_id%d_p%d_c%d_b%d_h%d.bmp", PATH_TEMP, image, pid->id, pid->page, pid->copy, RX_HBConfig.no, head);
		blk = blkNo0+((info->blkNo+cnt)%blkCnt);
		TrPrintfL(TRUE, "Write Bitmap >>%s<<, id=%d, page=%d, copy=%d, blocks %d..%d", filename, pid->id, pid->page, pid->copy, info->blkNo, info->blkNo+cnt);
		bmp_write_head(filename, _Buffer+info->blkNo*RX_HBConfig.dataBlkSize, info->bitPerPixel, info->widthPx, info->lengthPx, FALSE, _Buffer+RX_HBConfig.head[head].blkNo0*RX_HBConfig.dataBlkSize, RX_HBConfig.head[head].blkCnt*RX_HBConfig.dataBlkSize);
		
		//--- reset the buffer ---
		memset(_Buffer+info->blkNo*RX_HBConfig.dataBlkSize, 0x55, info->widthBytes*info->lengthPx);
	}
	return REPLY_OK;
}

//--- simu_print --------------------------------------------------------------
int simu_print(void)
{
	SFpgaImage	*info;
	int			head;
	int			imageNo, i;
	int			size, cnt;
	int			blk0, blk, b;

	TrPrintfL(TRUE, "simu_print");
	for (head=0; head<MAX_HEADS_BOARD; head++)
	{
		if (head==0) TrPrintfL(TRUE, "ImageList: inIdx=%d, outIdx=%d", _PrintList.imageList[head].inIdx, _PrintList.imageList[head].outIdx);
		if (_PrintList.imageList[head].outIdx != _PrintList.imageList[head].inIdx)
		{
			imageNo = _PrintList.imageList[head].outIdx;
			Fpga.encoder->pgCnt[head]++;
			
			_check_blocks(head, imageNo);

			if (RX_HBConfig.writeTestBmp) _simu_write_image(head, imageNo, &_PageId[imageNo]);

			TrPrintfL(TRUE, "simu_print  head[%d].image[%d]", head, imageNo);
			info  = &_PrintList.imageList[head].image[imageNo];
			blk0  = _Udp.block[head].blkNo0;
			size  = _Udp.block[head].blkNoEnd - _Udp.block[head].blkNo0 +1;
			cnt   = (info->widthBytes*info->lengthPx + RX_HBConfig.dataBlkSize-1)/RX_HBConfig.dataBlkSize;
			blk   = info->blkNo-blk0;
			for (i=0; i<cnt; i++)
			{
				b = blk0+blk;
				Fpga.blockUsed[b>>3] &= ~(1<<(b&7));
				blk  = (blk+1)%size;
			}
//			Fpga.stat->blockOutIdx[head] = blk0+blk;
			_PrintList.imageList[head].outIdx++;
			TrPrintfL(TRUE, "Head[%d].BlockOutIdx=%d", head, blk0+blk);
			Fpga.encoder->pdCnt[head]++;
		}
	}
	
	TrPrintfL(TRUE, "simu_done");
	return REPLY_OK;
}


//--- simu_print_done -----------------------------
void simu_print_done(RX_SOCKET socket)
{
	int head;
	int cnt, done;
	SPrintFileMsg	msg;
	 
	if (!SIMU) return;
	Error(WARN, 0, "simu_print_done not implemented");
	return;

	while (1)
	{
		for (head=0, cnt=0, done=0; cnt==done && head<MAX_HEADS_BOARD;  head++)
		{
			if (RX_HBConfig.head[head].blkCnt) 
			{
				cnt++;
				if (fpga_is_init() && Fpga.encoder->pdCnt[head] > _PdCnt) 
					done++;
			}
		}

		if (cnt && cnt==done)
		{
			//--- print done for all heads --------------
			msg.hdr.msgLen  = sizeof(msg);
			msg.hdr.msgId	= EVT_PRINT_FILE;

			memcpy(&msg.id, &_PageId[_PdCnt&0xff], sizeof(msg.id));
			msg.evt			= DATA_PRINT_DONE;
			msg.bufReady	= 0;
			_PdCnt++;
			sok_send(&socket, &msg);
			TrPrintfL(TRUE, "PRINT DONE:  id=%d, page=%d, copy=%d, scan=%d", msg.id.id, msg.id.page, msg.id.copy, msg.id.scan);
		}
		else break;
	}
}

