/*
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 */
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef linux
	#include <arpa/inet.h>
	#include <linux/if_packet.h>
	#include <sys/ioctl.h>
	#include <sys/socket.h>
	#include <net/if.h>
	#include <netinet/ether.h>
	#include <linux/ip.h>
	#include <netinet/udp.h>
#endif

// Stefan 06.04.16 ad this as there are build errors without this
#ifndef MSG_NOSIGNAL
	#define MSG_NOSIGNAL	0	// for windows!
#endif
//

#include "rx_common.h"
#include "rx_def.h"
#include "rx_error.h"
#include "rx_mac_address.h"
#include "rx_sok.h"
#include "rx_threads.h"
#include "rx_tif.h"
#include "rx_trace.h"
#include "args.h"
#include "bmp.h"
#include "tcp_ip.h"
#include "fpga.h"
#include "udp_test.h"
 
//--- static vairables ---------------------

static RX_RAW_SOCKET	_Socket=NULL;

static BYTE				*_Data=NULL;
static int				_Size = 256*1024*1024;
static int				_BlkNo[4]={0,0,0,0};
static int				_BlkSentCnt=0;
static int				_BlkAliveSent[MAX_PATH];
static char				_FName[MAX_PATH]="";
static RX_SOCKET		_UdpSocket[2]={INVALID_SOCKET, INVALID_SOCKET};
static int				_Copy=0;

static int				_FlipHorizontal = TRUE;
static int				_Backwards = TRUE;

//--- prototyes -----------------------------------------
static void _send_image(int head, SBmpInfo *bmpInfo, int blkNo, int backwards);

//--- udp_test_init -------------------------------
void udp_test_init(void)
{
}


//--- udp_test_send_block ---------------------------------
void udp_test_send_block(int headCnt, int blkCnt)
{
	SFpgaImageCmd img;
	int dataLen;
	int widthPx = 1024;
	static UINT32 test=0;
	UINT32 *dst;
	int head,  i, blk;

	dataLen=fpga_udp_block_size();
	_Copy++;
	for (head=0; head<headCnt; head++)
	{
		memset(&img, 0, sizeof(img));
		img.hdr.msgId = CMD_FPGA_IMAGE;
		img.hdr.msgLen = sizeof(img);
		img.id.id	= 1;
		img.id.page = 1;
		img.id.scan = 1;
		img.id.copy = _Copy;

		img.head				= head;
		img.image.blkNo			= RX_HBConfig.head[head].blkNo0+_BlkNo[head];
		img.image.blkCnt		= blkCnt;	// not iused in FPGA
		img.image.bitPerPixel	= 1;
		img.image.widthPx		= widthPx;			
		img.image.widthBytes	= widthPx/8;		
		img.image.lengthPx		= (blkCnt*dataLen)/img.image.widthBytes;			
		img.image.jetPx0		= 0;		
		img.image.flags			= 0;	
		img.image.flipHorizontal= _FlipHorizontal;
		img.image.clearBlockUsed= FALSE;

		if (TRUE || _BlkNo[head]==0)
		{
			SUDPDataBlockMsg msg;
			for (blk=0; blk<blkCnt; blk++)
			{		
				msg.blkNo = RX_HBConfig.head[head].blkNo0+_BlkNo[head];
				for (i=0, dst=(UINT32*)msg.blkData; i<dataLen; i+=sizeof(UINT32)) 
					*dst++ = (head*0x10000000)+test++;
				udp_test_send(&msg, dataLen+4);
				if (++_BlkNo[head]==RX_HBConfig.head[head].blkCnt) 
					_BlkNo[head]=0;
			}
		}
		rx_sleep(100);
		fpga_image	(&img);
	}
//	if (arg_simu_machine) fpga_manual_pg();
}

//--- _send_image --------------------------------------------
static void _send_image(int head, SBmpInfo *bmpInfo, int blkNo, int backwards)
{
	SFpgaImageCmd img;

	img.hdr.msgId  = CMD_FPGA_IMAGE;
	img.hdr.msgLen = sizeof(img);
	img.id.id	= 1;
	img.id.page = 1;
	img.id.scan = 1;

	img.head	= head;

	img.image.bitPerPixel	= bmpInfo->bitsPerPixel;		
	img.image.blkNo			= blkNo;			
	img.image.widthPx		= bmpInfo->srcWidthPx;			
	img.image.widthBytes	= bmpInfo->lineLen;
	img.image.lengthPx		= bmpInfo->lengthPx;			
	img.image.jetPx0		= 0;		
	img.image.flags			= backwards;	
	img.image.flipHorizontal= _FlipHorizontal;		
	img.image.blkCnt        = (img.image.widthBytes*img.image.lengthPx+RX_HBConfig.dataBlkSize-1) / RX_HBConfig.dataBlkSize; // for tests
	/*
	//--- test: print many images out of memory ---
	if (TRUE)
	{	
		int i;
		img.image.clearBlockUsed= FALSE;
		for (i=0; i<100; i++) 
			fpga_image	(&img);
	}
	else
	*/
	{
		img.image.clearBlockUsed= TRUE;
		fpga_image	(&img);
	}
}

//--- udp_test_print -----------------------------------------
void udp_test_print(char *fname)
{
	UINT32	width, height;
	UINT8	bppx;
	char	filePathBmp[MAX_PATH];

	SPageId id;
	SColorSplitCfg split;
	SBmpInfo bmpInfo;
	int size;
	int  blkNo;
	ULONG memsize;
	int head = 0;

	if (*fname) 
	{
		strcpy(_FName, fname);
		if (!strstr(_FName, ".bmp")) strcat(_FName, ".bmp");
	}
	else strcpy(_FName, "angle.bmp");

//	sprintf(filePathBmp, "%s%s", PATH_BIN_LX, _FName);
	sprintf(filePathBmp, "%s%s", PATH_BIN, _FName);

	if (_Data==NULL)
	{
		Error(WARN, 0, "Allocate _DATA for tests");
		_Data = (BYTE*)malloc(_Size);
		memset(_Data, 0, sizeof(_Data));	
	}

	if (_Data==NULL)
	{
		Error(ERR_CONT, 0, "Could not allocate memory >>_Data<<");
		return;
	}

	bppx = 0;
	bmp_get_size(filePathBmp, &width, &height, &bppx, &memsize);
	if (width+height)
	{
		id.id   = 1;
		id.page = 1;
		id.scan = 1;
		id.copy = 1;

	//	_Backwards = ! _Backwards;

		if (arg_fhnw) _Backwards = TRUE;
		else          _Backwards = FALSE;

		memset(&split, 0, sizeof(split));
		split.color.colorCode = 0;
		strcpy(split.color.name, "Black");
		split.firstLine		  = 0;
		split.lastLine		  = 2048-2*128;

		size = (width*bppx+7)/8*height;
		if (size<=_Size) 
		{
			bmp_load	(filePathBmp, &_Data, _Size, &bmpInfo);
			if (bmpInfo.inverseColor)
			{
				UINT16 *pdat;
				int    len;
				for (pdat=(UINT16*)_Data, len=bmpInfo.dataSize; len; len-=sizeof(*pdat)) *pdat++ ^= 0xffffffff;
			}
		}
		else Error(ERR_CONT, 0, "File larger than buffer >>_Data<<");	
		for (head=0; head<4; head++)
		{
			int		dataLen;
			int		lineLen, blkLen, len;
			UINT32	h;
			BYTE	*src, *dst;

			dataLen = fpga_udp_block_size();

			src=_Data;
			SUDPDataBlockMsg msg;
			memset(&msg, 0, sizeof(msg));
			dst = msg.blkData;
			blkNo = _BlkNo[head];
			blkLen = 0;
			for (h=0; h<bmpInfo.lengthPx; h++)
			{
				for (lineLen=bmpInfo.lineLen; lineLen;)
				{	
					if (lineLen<dataLen-blkLen) len=lineLen;
					else len = dataLen-blkLen;
					memcpy(dst, src, len);
					src     += len;
					dst     += len;
					blkLen  += len;
					lineLen -= len;
					if (blkLen==dataLen)
					{
						msg.blkNo = RX_HBConfig.head[head].blkNo0+blkNo;
						blkNo = (blkNo+1)%RX_HBConfig.head[head].blkCnt;
						udp_test_send(&msg, dataLen+4);
						memset(&msg, 0, sizeof(msg));
						dst = msg.blkData;
						blkLen = 0;
					}
				}
				blkLen = (blkLen+31) & ~31; // align to 256 bits
				dst = &msg.blkData[blkLen];
			}
			if (blkLen)
			{
				msg.blkNo = RX_HBConfig.head[head].blkNo0+blkNo;
				blkNo = (blkNo+1)%RX_HBConfig.head[head].blkCnt;
				udp_test_send(&msg, dataLen+4);
			}
			TrPrintfL(TRUE, "Sent UDP Blocks: %d..%d cnt=%d\n", _BlkNo[head], blkNo-1, blkNo-_BlkNo[head]);
			_send_image(head, &bmpInfo, RX_HBConfig.head[head].blkNo0+_BlkNo[head], _Backwards);
			_BlkNo[head] = blkNo;
		}

		/*
		//--- define empty heads -------------------------------
		{
			SUDPDataBlockMsg msg;
			int dataLen = fpga_udp_block_size();
			int i;
			memset(&msg, 0, sizeof(msg));
			for (i=0; i<4; i++)
			{
				bmpInfo.srcWidthPx=1;
				bmpInfo.lengthPx=1;
				if (RX_HBConfig.head[i].enabled && i!=head) 
				{
					msg.blkNo = RX_HBConfig.head[i].blkNo0;
					udp_test_send(&msg, dataLen+4);
					_send_image(i, &bmpInfo, RX_HBConfig.head[i].blkNo0);
				}
			}
		}
		*/
	}
}

//--- udp_test_print_tif -----------------------------------------
void udp_test_print_tif(char *fname)
{
	int				h, head, headCnt=4;
	UINT32			width, height;
	UINT32			x0, x1, y;
	int				fill;
	int				blkLen, len, lineLen;
	UINT8			bppx;
	int				blkNo;
	char			filePath[MAX_PATH];
	UINT32			bufSize;
	BYTE			*buffer, *src, *src0, *dst;
	BYTE			*test, *tst;
	SBmpInfo		info;
	SColorSplitCfg	split;
	SUDPDataBlockMsg msg;
	int				 dataLen = fpga_udp_block_size();

	if (FALSE)
	{	// activate TCP/IP
		memset(&msg, 0, sizeof(msg));
		udp_test_send(&msg, dataLen+4);
		rx_sleep(1000);
	}

	if (!*fname) strcpy(fname, "maserati");
	strcpy(_FName, fname);
	if (!strstr(_FName, ".tif")) strcat(_FName, "_K.tif");
	sprintf(filePath, "%s%s", PATH_BIN_LX, _FName);
	
	if (tif_get_size(filePath, 0, 0, &width, &height, &bppx)==REPLY_OK)
	{
		bufSize = ((width+32)*height*bppx)/8;
		buffer = (BYTE*)malloc(bufSize);
		test   = (BYTE*)malloc(bufSize);

	//	_Backwards = ! _Backwards;

		if (arg_fhnw) _Backwards = TRUE;
		else          _Backwards = FALSE;

		memset(&split, 0, sizeof(split));
		split.firstLine = 0;
		split.lastLine  = HEAD_WIDTH_SAMBA*headCnt-HEAD_OVERLAP_SAMBA;
		strcpy(split.color.name, RX_ColorName[0].name);
		split.color.colorCode = 0;//RX_ColorName[0].code;

		if (bppx==8) 
		{
			tif_load_8bppx(filePath, FALSE, &split, 1, &buffer, &info);
			fill=0;
		}
		else         
		{
			tif_load(NULL, PATH_BIN_LX, fname, FALSE, 0, 0, &split, 1, &buffer, &info, NULL);
			fill=HEAD_OVERLAP_SAMBA;
		}

//		for (head=headCnt-1, x0=0; head>=0; head--, x0+=HEAD_WIDTH_SAMBA-fill, fill=0)
		for (h=0, x0=0; h<headCnt; h++, x0+=HEAD_WIDTH_SAMBA-fill, fill=0)
		{
			if (_FlipHorizontal) head=headCnt-1-h;
			else head=h;
			tst = test;
			x1=x0+HEAD_WIDTH_SAMBA;
			if (!fill) x1+=HEAD_OVERLAP_SAMBA;
		//	x1 += 8; // add data for shifting!
			if (x1>info.srcWidthPx) x1=info.srcWidthPx;
			lineLen = ((x1-x0) * info.bitsPerPixel+7)/8;
			src0 = buffer + (x0*info.bitsPerPixel)/8;
			memset(&msg, 0, sizeof(msg));
			blkNo = _BlkNo[head];
			dst = msg.blkData;
			blkLen=0;
			for (y=0; y<info.lengthPx; y++, src0+=info.lineLen)
			{
				src=src0;
				if (fill)
				{
					len = (fill*info.bitsPerPixel)/8;
					if (blkLen+len>dataLen) 
					{
						memset(dst, 0x00, dataLen-blkLen);
						len -= (dataLen-blkLen);
						msg.blkNo = RX_HBConfig.head[head].blkNo0+blkNo;
						blkNo     = (blkNo+1)%RX_HBConfig.head[head].blkCnt;

			//			if (head==1) memset(msg.blkData, 0x00, dataLen);

						udp_test_send(&msg, dataLen+4);
						memcpy(tst, msg.blkData, dataLen); tst+=dataLen;
						memset(&msg, 0, sizeof(msg));
						dst = msg.blkData;
						blkLen = 0;
					}
					memset(dst, 0x00, len);
					dst += len;
					blkLen += len;
				}
				len=lineLen;
				if (blkLen+lineLen>=dataLen) 
				{
					len = dataLen-blkLen;
					memcpy(dst, src, len);
					memcpy(tst, src, len);
					src+=len;
					msg.blkNo = RX_HBConfig.head[head].blkNo0+blkNo;
					blkNo     = (blkNo+1)%RX_HBConfig.head[head].blkCnt;
					udp_test_send(&msg, dataLen+4);
					memcpy(tst, msg.blkData, dataLen); tst+=dataLen;
					memset(&msg, 0, sizeof(msg));
					dst = msg.blkData;
					blkLen = 0;
					len=lineLen-len;
				}
				memcpy(dst, src, len);
				dst     += len;
				blkLen  += len;
				//--- fill line to 256 bits ------------
				if ((len=(blkLen%32)))
				{
					len = 32-len;
					memset(dst, 0x00, len);
					dst+=len;
					blkLen  += len;
				}
			}

			//--- send final bllock -----------------------
			if (blkLen)
			{
				msg.blkNo = RX_HBConfig.head[head].blkNo0+blkNo;
				blkNo     = (blkNo+1)%RX_HBConfig.head[head].blkCnt;

			//	if (head==1) memset(msg.blkData, 0x00, dataLen);
				
				udp_test_send(&msg, dataLen+4);
				memcpy(tst, msg.blkData, dataLen); tst+=dataLen;
			}

			TrPrintfL(TRUE, "Head[%d]: size=%d, blkCnt=%d\n", head, info.lengthPx*lineLen, (info.lengthPx*lineLen+1439)/1440);

			//--- send img -------------------------------------------
			{
				SBmpInfo inf;
				char str[MAX_PATH];
				memcpy(&inf, &info, sizeof(info));
				inf.srcWidthPx = x1-x0+fill;
				inf.lineLen	   = (inf.srcWidthPx*inf.bitsPerPixel)/8;
				inf.lineLen    = (inf.lineLen+31)&~31;

				if (FALSE)
				{
					sprintf(str, "/tmp/head_%d.bmp", head);
					bmp_write(str, test, inf.bitsPerPixel, inf.srcWidthPx, inf.lengthPx, inf.lineLen, FALSE);
				}

				_send_image(head, &inf, RX_HBConfig.head[head].blkNo0+_BlkNo[head], _Backwards);
			}
			_BlkNo[head] = blkNo;
		}
		free(buffer);
		free(test);
	}
}

//--- udp_test_fname ------------------------------
const char *udp_test_fname()
{
	return _FName;
}

//--- udp_test_sent_blocks -------------------------------------
int  udp_test_sent_blocks(void)
{
	return _BlkSentCnt;
}

//--- udp_test_sent_blocks -------------------------------------
int  udp_test_sent_alive(int no)
{
	return _BlkAliveSent[no];
}

//--- udp_test_send -----------------------------------------
int udp_test_send(void *data, int dataLen)
{
	int		ret;
	UINT32 addr;
	static int	udpNo=0;
	char str[100];
	SUDPDataBlockMsg *pmsg = (SUDPDataBlockMsg*)data;	

	if (FALSE)
	{
		int i;
		BYTE *dat = (BYTE*)data;
		printf("data: ");
		for (i=0; i<32; i++) printf(" %02x", dat[i]);
		printf("\n");
	}

	if (_UdpSocket[udpNo]==0 || _UdpSocket[udpNo]==INVALID_SOCKET)
	{
		fpga_get_ip_addr(udpNo, &addr);
		ret=sok_open_client(&_UdpSocket[udpNo], sok_addr_str(addr, str), PORT_UDP_DATA, SOCK_DGRAM);
	}
	send(_UdpSocket[udpNo], (char*)data, dataLen, MSG_NOSIGNAL);
	if (pmsg->blkNo==0xffffffff) _BlkAliveSent[udpNo]++;
	else						 _BlkSentCnt++;
	udpNo = 1-udpNo;
	return ret;
}
