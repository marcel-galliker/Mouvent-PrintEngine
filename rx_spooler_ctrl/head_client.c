// ****************************************************************************
//
//	head_client.c
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

//--- includes ----------------------------------------------------------------
#include "rx_common.h"
#include "rx_def.h"
#include "rx_error.h"
#include "rx_file.h"
#include "rx_mac_address.h"
#include "rx_mem.h"
#include "rx_sok.h"
#include "rx_threads.h"
#include "rx_tif.h"
#include "rx_trace.h"
#include "spool_arp.h"
#include "tcp_ip.h"
#include "data.h"
#include "screening.h"
#include "bmp.h"
#include "ctrl_client.h"

#include "head_client.h"


//--- Defines -----------------------------------------------------------------
// static int	_Trace=2;
static int	_Trace=0;

#define SIMU_OFF	0	// no simulation
#define SIMU_WRITE	1	// write data to file
#define SIMU_READ	2	// test reading files, no sending, no writing

static int	_Simulation=SIMU_WRITE;

// #define RAW_SOCKET

#define MAX_USED_ID	16
#define BLOCK_BURST_CNT	3000

//--- Externals ---------------------------------------------------------------

//--- Structures --------------------------------------------------------------
typedef struct
{
	SHeadBoardCfg	cfg;
	RX_SOCKET		dataSocket[MAX_ETH_PORT];
	RX_SOCKET		ctrlSocket;
	HANDLE			mutex;

	UINT32			blockUsedSize;
	UINT32			blockOutIdx;
	UINT32			*blockUsed;
	UINT8			blockUsedId;
	UINT8			checkBlocksFree;
	UINT8			reqUsedId[HEAD_CNT][MAX_USED_ID];
	SBmpSplitInfo	*pBmpSplitPar[HEAD_CNT];
	SUDPDataBlockMsg msg;
	SBlockUsedCmd	 blockUsedCmd;
	int				 udpNo;
	UINT32			pdCnt;
	int				errorSent;
	int				boardNo;
	SPageId			lastId[HEAD_CNT];
	UINT8			blkData[DATA_BLOCK_SIZE_JUMBO+2];
} SHBThreadPar;

//--- static vairables --------------------------------------------------------
static int				_BufSize;
static int				_Running;
static int				_Abort;
static int				_Checking;
static int				_DeleteFiles=TRUE;
static int				_HBUsed[HEAD_BOARD_CNT];
static SHBThreadPar*	_HBPar[HEAD_BOARD_CNT];
static int				_SimuNo[MAX_COLORS];
static int				_TestImgNo[HEAD_BOARD_CNT][MAX_HEADS_BOARD];
static UINT16			_TestBlockSent[MAX_HEADS_BOARD][140000];
static int				_TestBlockUsedReqTime[MAX_HEADS_BOARD][MAX_USED_ID];
static int				_TestLastBlock;
static int				_ScreeningTime;
static int				_StressTestRunning;

static char				_send_image_cmd_flags[MAX_PATH];

//--- prototypes --------------------------------------------------------------
static void *_head_board_ctrl_thread(void *par);
static int _handle_head_ctrl_msg(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *par);
static int _open_data_channel(SHBThreadPar *par);
static void _req_used_flags	(SHBThreadPar *par, int head, int blkNo, int blkCnt, int line);
static int _do_block_used	(SHBThreadPar *par, SBlockUsedRep *msg);
static int _send_to_board	(SHBThreadPar *par, int head, int blkNo, int blkCnt);
static int _send_image_data	(SBmpSplitInfo *pBmpSplitInfo);
static int _send_image_cmd 	(SBmpSplitInfo *pBmpSplitInfo);
static void _save_to_file	(SBmpSplitInfo *pInfo, int log);
static int _check_image_blocks (SHBThreadPar *par, SBmpSplitInfo *pBmpSplitInfo, int minBlk, int maxBlk);

static void *_stress_test_thread(void *ppar);

//--- hc_start ----------------------------------------------------------------
int hc_start(void)
{
	#ifndef DEBUG
		_Simulation=FALSE;
	#endif
	memset(_HBPar, 0, sizeof(_HBPar));
	memset(_HBUsed, 0, sizeof(_HBUsed));
	_Running = TRUE;
	_Checking = FALSE;
	return REPLY_OK;
}

//--- hc_end ------------------------------------------------------------------
int hc_end(void)
{
	_Running = FALSE;
	_Abort   = TRUE;
	return REPLY_OK;
}

//--- hc_in_simu ------------------------------------------------------------------
int  hc_in_simu(void)
{
	return _Simulation;
}

//--- hc_head_board_cfg ------------------------------------------------------------------
int hc_head_board_cfg(RX_SOCKET socket, SHeadBoardCfg* cfg)
{
	int i;
	UINT32 size;
	TrPrintfL(TRUE, "hc_head_board_cfg[%d]", cfg->no);

	_StressTestRunning = FALSE;
	_ScreeningTime	   = 0;
	
    #ifdef DEBUG
//	if (_Simulation) 
	if (_DeleteFiles)
	{ //--- prepare simulation directory ------------------------------		
		char path[MAX_PATH];
		rx_mkdir(PATH_RIPPED_DATA "trace/");
		for (i=0; i<SIZEOF(RX_Color); i++)
		{
			if (RX_Color[i].lastLine>0)
			{
				sprintf(path, PATH_RIPPED_DATA "trace/%s", RX_ColorNameShort(i));
				rx_mkdir(path);
				rx_remove_old_files(path, 0);
			}
		}
		memset(_SimuNo, 0, sizeof(_SimuNo));
		_DeleteFiles = FALSE;
	}
	#endif

	if (_Simulation) 
	{
		Error(WARN, 0, "Head-Client in Simulation");
		if (_Simulation) cfg->present=dev_simu;
		if (_Simulation==SIMU_READ) rx_mem_clear_caches();
	}
	
	memset(_TestImgNo, 0, sizeof(_TestImgNo));
	memset(_TestBlockSent, 0, sizeof(_TestBlockSent));
	memset(_TestBlockUsedReqTime, 0, sizeof(_TestBlockUsedReqTime));
	if (cfg->no==0) memset(_HBUsed, 0, sizeof(_HBUsed));
	_TestLastBlock=0;
	_BufSize  = 0;
	if (cfg->no>=SIZEOF(_HBPar)) 
	{
		Error(ERR_CONT, 0, "HeadBard No=%d, too large", cfg->no);
		return REPLY_ERROR;
	}

	if (_HBPar[cfg->no]==NULL) 
	{
		_HBPar[cfg->no]=(SHBThreadPar*)malloc(sizeof(SHBThreadPar));
		memset(_HBPar[cfg->no], 0, sizeof(SHBThreadPar));
		_HBPar[cfg->no]->ctrlSocket = INVALID_SOCKET;
		for (i=0; i<MAX_ETH_PORT; i++) _HBPar[cfg->no]->dataSocket[i]=INVALID_SOCKET;
		_HBPar[cfg->no]->mutex = rx_mutex_create();
	}
	else
	{
		//--- close Ethernet Ports if address changed -------------------------------
		if (_HBPar[cfg->no]->cfg.ctrlAddr!=cfg->ctrlAddr) sok_close(&_HBPar[cfg->no]->ctrlSocket);
		for (i=0; i<UDP_PORT_CNT; i++)
		{
				if (_HBPar[cfg->no]->cfg.dataAddr[i]!=cfg->dataAddr[i]) sok_close(&_HBPar[cfg->no]->dataSocket[i]);
		}
	}
	_HBPar[cfg->no]->boardNo     = cfg->no;
	_HBPar[cfg->no]->blockOutIdx = 0;
	_HBPar[cfg->no]->pdCnt		 = 0;
	_HBPar[cfg->no]->blkData[0]  = 0x33;
	if (cfg->dataBlkSize>sizeof(_HBPar[cfg->no]->blkData)-2) Error(ERR_ABORT, 0, "blkData overflow");
	_HBPar[cfg->no]->blkData[cfg->dataBlkSize+1]  = 0x66;
	memset(&_HBPar[cfg->no]->lastId, 0, sizeof(_HBPar[cfg->no]->lastId));
	memcpy(&_HBPar[cfg->no]->cfg, cfg, sizeof(_HBPar[cfg->no]->cfg));
	size =  (cfg->dataBlkCntHead*MAX_HEADS_BOARD)/sizeof(UINT32);
	if (size>_HBPar[cfg->no]->blockUsedSize)
	{
		if (_HBPar[cfg->no]->blockUsed) free(_HBPar[cfg->no]->blockUsed);
		_HBPar[cfg->no]->blockUsed = (UINT32*)rx_malloc(size);
		if (!_HBPar[cfg->no]->blockUsed) return Error(ERR_ABORT, 0, "Could not allocate memory for blockused flags");
		_HBPar[cfg->no]->blockUsedSize = size;
	}
	memset(_HBPar[cfg->no]->blockUsed, 0, _HBPar[cfg->no]->blockUsedSize);
	_HBPar[cfg->no]->errorSent = FALSE;
	_HBUsed[cfg->no]=(cfg->spoolerNo==RX_SpoolerNo);
	hc_check();

	return REPLY_OK;
}

//--- _open_data_channel --------------------------------------------------------
static int _open_data_channel(SHBThreadPar *par)
{
	int errno, i;
	char str[32];
	
	if (par->cfg.present==dev_simu || _Simulation) return REPLY_OK;
//	TrPrintfL(TRUE, "_open_data_channel");

	//--- reset TCP socket if changed -------------------
	if (par->ctrlSocket==INVALID_SOCKET && par->cfg.ctrlAddr)
	{
//		TrPrintfL(TRUE, "Connect CTRL Channel to >>%s<<", sok_addr_str(par->cfg.ctrlAddr, str));		
		errno=sok_open_client(&par->ctrlSocket, sok_addr_str(par->cfg.ctrlAddr, str), par->cfg.ctrlPort, SOCK_STREAM);
		if (errno)
		{
			if (_Checking && !par->errorSent) Error(ERR_ABORT, 0, "Could not connect to Head %d (IP %s:%d)", par->cfg.no+1, str, par->cfg.ctrlPort);
			par->errorSent = TRUE;
			return REPLY_ERROR;
		}
		else
		{
			TrPrintfL(TRUE, "connected, port=>>%s<<", sok_get_socket_name(par->ctrlSocket, str, NULL, NULL));
			rx_thread_start(_head_board_ctrl_thread, par, 0, "_head_board_ctrl_thread");
			sok_send_2(&par->ctrlSocket, CMD_PING, 0, NULL);
		}
	}
	
	//--- open UDP data channel -----
	for (i=0; i<SIZEOF(par->cfg.dataAddr); i++)
	{
		if (par->dataSocket[i]==INVALID_SOCKET && par->cfg.dataAddr[i])
		{
	//		TrPrintfL(TRUE, "Connect Data Channel[%d] to >>%s<<", i, sok_addr_str(par->cfg.dataAddr[i], str));

			{
				errno=sok_open_client(&par->dataSocket[i], sok_addr_str(par->cfg.dataAddr[i], str), par->cfg.dataPort[i], SOCK_DGRAM);			
			}
			if (errno)
			{
				char err[256];
				TrPrintfL(1, "Connecting >>%s:%d<<Socket Error >>%s<<", str, par->cfg.dataPort[i], err_system_error(errno, err, sizeof(err)));
			}
			else TrPrintfL(TRUE, "%s connected to HeadData[%d]", sok_get_peer_name(par->dataSocket[i], str, NULL, NULL), i);
		}
	//	arp_request(par->dataSocket[0], par->cfg.dataAddr[0]);
	}
	return REPLY_OK;
}

//--- hc_start_printing ----------------------------------------------
void hc_start_printing(void)
{
	_Abort = FALSE;
	_Checking = TRUE;
}

//--- hc_abort_printing ----------------------------------------------
void hc_abort_printing(void)
{
	_Abort = TRUE;
	_Checking = FALSE;
	_DeleteFiles = TRUE;
	if (_TestLastBlock)
	{
		int i;
		Error(WARN, 0, "TEST");
		printf("Sent data Blocks=%d\n", _TestLastBlock);
		for (i=0; i<=_TestLastBlock; i++)
		{
			if (i%10==0) printf("Block[%05d]: ", i);
			printf("%02d ", _TestBlockSent[0][i]);
			if (i%10==9) printf("\n");
		}
		printf("\nEND\n");
	}
}

//--- hc_send_next ---------------------------------------------
void hc_send_next()
{
	int i, headCnt;
	int blkNo=0;
	int	time=rx_get_ticks();

	SBmpSplitInfo *pSplitInfo, *pInfo;
	if ((pSplitInfo=data_get_next(&headCnt)))
	{
		//--- tracing --------------------------------------
		memset(_send_image_cmd_flags, '_', sizeof(_send_image_cmd_flags));
		_send_image_cmd_flags[((headCnt+3)/4)*5]=0;
		for (i=4; _send_image_cmd_flags[i]; i+=5) _send_image_cmd_flags[i]=' ';

		//--- ----------------------------------------------
		if (pSplitInfo->pListItem)
		{
			SPageId *pid = &pSplitInfo->pListItem->id;
			TrPrintfL(TRUE, "hc_send_next: (id=%d, page=%d, copy=%d, scan=%d) bitsPerPixel=%d", pid->id, pid->page, pid->copy, pid->scan, pSplitInfo->bitsPerPixel);
		}
		if (pSplitInfo->screening)
		{			
			if (scr_wait(5000)) 
			{
				Error(ERR_STOP, 0, "Screening Timeout");
				return;
			}
		}

		for (i=0, pInfo=pSplitInfo; i<headCnt; i++, pInfo++)
		{
			if (pInfo->used)
			{
				_send_image_cmd_flags[5*pInfo->board+pInfo->head] = '-';

				// TrPrintfL(_Trace, "Head[%d.%d]: send color >>%s<<, present=%d", pInfo->board, pInfo->head, RX_ColorNameLong(pInfo->colorCode), _HBPar[pInfo->board]->cfg.present);
				switch (_HBPar[pInfo->board]->cfg.present)
				{
				case dev__undef:	break;
				case dev_off:		break;
				case dev_simu:		{
										/*
										{
											SPageId *pid = &pInfo->pListItem->id;
											Error(LOG, 0, "File (id=%d, page=%d, copy=%d, scan=%d) headsUsed=%d", pid->id, pid->page, pid->copy, pid->scan, pInfo->pListItem->headsUsed);	
										}
										*/
										if(_Simulation==SIMU_WRITE) _save_to_file(pInfo, FALSE);
										if(data_sent(pInfo,i)) 
										{
											SPrintDoneMsg evt;
											SPageId *pid = &pInfo->pListItem->id;

											evt.hdr.msgId   = EVT_PRINT_DONE;
											evt.hdr.msgLen  = sizeof(evt);
											evt.boardNo		= pInfo->board;
											evt.pd			= _HBPar[pInfo->board]->pdCnt++;
											memcpy(&evt.id, &pInfo->pListItem->id, sizeof(evt.id));
										//	evt.evt			= DATA_PRINT_DONE;
										//	evt.spoolerNo   = RX_SpoolerNo;
										//	evt.bufReady	= data_ready();
											ctrl_send(&evt);
											if(_Simulation==SIMU_WRITE)	
											{
												if (rx_def_is_scanning(RX_Spooler.printerType))
													Error(LOG, 0, "File (id=%d, page=%d, copy=%d, scan=%d) saved to File", pid->id, pid->page, pid->copy, pid->scan);												
												else
													Error(LOG, 0, "File (id=%d, page=%d, copy=%d) saved to File", pid->id, pid->page, pid->copy);												
											}
											else
										{
											static int _time=0;
											int time = rx_get_ticks();
											double lengthPx = pInfo->pListItem->lengthPx/(1200.0*40.0);
											double speed=1000;
												if (time!=_time) speed= lengthPx * 60 * 1000 / (time-_time);
											Error(LOG, 0, "SIMU_READ FILE SENT: (id=%d, page=%d, copy=%d, scan=%d) cycletime=%d ms, speed=%d m/min", pid->id, pid->page, pid->copy, pid->scan, time-_time, (int)speed);
											_time=time;
											}
											hc_send_next();								
										}
									}
									break;

				case dev_on:		if (FALSE && pInfo->colorCode==0)	// see rx_def.c: RX_ColorName
									{						
										_save_to_file(pInfo, FALSE);
										Error(LOG, 0, "File (id=%d, page=%d, copy=%d, scan=%d) blk0=%d, blkCnt=%d saved to File", pInfo->pListItem->id.id, pInfo->pListItem->id.page, pInfo->pListItem->id.copy, pInfo->pListItem->id.scan, pInfo->blk0, pInfo->blkCnt);
									}
									
								//	Error(LOG, 0, "Screening[%d,%d]: (id=%d, p=%d, c=%d, s=%d)", pInfo->board, pInfo->head, id.id, id.page, id.copy, id.scan);

									_send_image_data(pInfo);
									break;

				default:			Error(ERR_ABORT, 0, "Headboard[%d].Present=%d, undefined", pInfo->board, _HBPar[pInfo->board]->cfg.present);
									return;
				}
			}
		}		
	}
}

//--- _save_to_file ---------------------------------
static void _save_to_file(SBmpSplitInfo *pInfo, int log)
{
	int		size=pInfo->blkCnt * RX_Spooler.dataBlkSize;
	int		head, blk;
	char	fname[100];
	BYTE   *buffer;

	buffer = (BYTE*)malloc(size+2);
	if (buffer==NULL)
		return;
	
	head = ((pInfo->board*MAX_HEADS_BOARD)+pInfo->head) % RX_Spooler.headsPerColor;
	
	for (blk=0; blk<pInfo->blkCnt; blk++)
	{
		data_fill_blk(pInfo, blk, &buffer[blk*RX_Spooler.dataBlkSize+1], TRUE);
	}
	
	if (rx_def_is_scanning(RX_Spooler.printerType))
	{
		#ifdef linux
	//	sprintf(fname, "%strace/%s/Scan%02d-img%d-hd%d_%s.bmp", PATH_HOME PATH_RIPPED_DATA_DIR, RX_ColorNameShort(pInfo->inkSupplyNo), ++_SimuNo[pInfo->colorCode], pInfo->pListItem->id.id, pInfo->head, RX_ColorNameShort(pInfo->inkSupplyNo));
		sprintf(fname, "%strace/%s/sent(id=%d, p=%d, c=%d, s=%d, h=%d).bmp",  PATH_HOME PATH_RIPPED_DATA_DIR, RX_ColorNameShort(pInfo->inkSupplyNo), pInfo->pListItem->id.id, pInfo->pListItem->id.page, pInfo->pListItem->id.copy, pInfo->pListItem->id.scan, head);
		#else
		sprintf(fname, PATH_RIPPED_DATA "trace/%s/Scan%02d-img%d-hd%d_%s.bmp", RX_ColorNameShort(pInfo->inkSupplyNo), ++_SimuNo[pInfo->colorCode], pInfo->pListItem->id.id, pInfo->head, RX_ColorNameShort(pInfo->inkSupplyNo));
		#endif
	}
	else
	{
		#ifdef linux
		sprintf(fname, "%strace/%s/(id=%d, p=%d, c=%d)-%s%d.bmp", PATH_HOME PATH_RIPPED_DATA_DIR, RX_ColorNameShort(pInfo->inkSupplyNo), pInfo->pListItem->id.id, pInfo->pListItem->id.page, pInfo->pListItem->id.copy, RX_ColorNameShort(pInfo->inkSupplyNo), head);
		#else
		sprintf(fname, "d:/%strace/%s/(id=%d, p=%d, c=%d)-%s%d.bmp", PATH_RIPPED_DATA_DIR, RX_ColorNameShort(pInfo->inkSupplyNo), pInfo->pListItem->id.id, pInfo->pListItem->id.page, pInfo->pListItem->id.copy, RX_ColorNameShort(pInfo->inkSupplyNo), head);
		#endif
	}
	if (pInfo->bitsPerPixel<8)	
		bmp_write(fname, &buffer[1], pInfo->bitsPerPixel, pInfo->widthPx, pInfo->srcLineCnt, pInfo->dstLineLen, FALSE);
	else
	{
		char dir[MAX_PATH];
		char fname[MAX_PATH];
		sprintf(dir, PATH_RIPPED_DATA "trace/%s", RX_ColorNameShort(pInfo->inkSupplyNo));
		sprintf(fname, "(id=%d, p=%d, c=%d, s=%d)-%s%d", pInfo->pListItem->id.id, pInfo->pListItem->id.page, pInfo->pListItem->id.copy, pInfo->pListItem->id.scan, RX_ColorNameShort(pInfo->inkSupplyNo), head);
		SBmpInfo info;
		memset(&info, 0, sizeof(info));
		info.planes = 1;
		info.srcWidthPx   = pInfo->widthPx;
		info.lengthPx	  = pInfo->srcLineCnt;	
		info.lineLen	  = pInfo->dstLineLen;
		info.bitsPerPixel = pInfo->bitsPerPixel;
		info.resol.x	  = 1200;
		info.resol.y	  = 1200;
		info.colorCode[0] = pInfo->colorCode;
		info.inkSupplyNo[0] = 0;
		info.buffer[0]     = &buffer;
		tif_write(dir, fname, &info);
	}
	free(buffer);
	if (log) Error(LOG, 0, "File saved to >>%s<<", fname);
}

//--- _send_image_data ---------------------------------------------
static int _send_image_data(SBmpSplitInfo *pInfo)
{
	{
		SPageId *pid=&pInfo->pListItem->id;
		int idx=data_printList_idx(pInfo->pListItem);
		int blk0=_HBPar[pInfo->board]->cfg.head[pInfo->head].blkNo0;
		int blkCnt=_HBPar[pInfo->board]->cfg.head[pInfo->head].blkCnt;
		int end=((pInfo->blk0-blk0)+pInfo->blkCnt)%blkCnt + blk0;

		TrPrintfL(TRUE || _Trace, "Head[%d.%d]: _send_image_data pl[%d](id=%d, p=%d, c=%d, s=%d) blocks[%d .. %d] SAME=%d data=%p, blk0=%d, blkCnt=%d", pInfo->board, pInfo->head, idx, pid->id, pid->page, pid->copy, pid->scan, pInfo->blk0, end, pInfo->same, pInfo->data);
		//--- Test ------------------------
		TrPrintfL(_Trace, "Head[%d.%d]: widthPx=%d, bitsPerPixel=%d, widthBt=%d, dstLineLen=%d, srcLineCnt=%d, blkCnt=%d", pInfo->board, pInfo->head, pInfo->widthPx, pInfo->bitsPerPixel, pInfo->widthBt, pInfo->dstLineLen, pInfo->srcLineCnt , pInfo->blkCnt);		
		//---------------------------------
	}

	if(!rx_def_is_scanning(RX_Spooler.printerType) && pInfo->same)
	{
		_HBPar[pInfo->board]->pBmpSplitPar[pInfo->head] = pInfo;
		_send_to_board(_HBPar[pInfo->board], pInfo->head, pInfo->blk0, pInfo->blkCnt);		
	}
	else
	{
        {
		    static int test=0;
		    SPageId *pid=&pInfo->pListItem->id;
		    int idx=data_printList_idx(pInfo->pListItem);
		    TrPrintfL(_Trace, "Head[%d.%d]: _send_image_data pl[%d](id=%d, p=%d, c=%d, s=%d) blocks[%d .. %d] ", pInfo->board, pInfo->head, idx, pid->id, pid->page, pid->copy, pid->scan, pInfo->blk0, pInfo->blk0+pInfo->blkCnt);
        }
		pInfo->sendFromBlk	= 0;

		TrPrintfL(_Trace, "Head[%d.%d]: NEW BMP _req_used_flags blkN0=%d, blkCnt=%d", pInfo->board, pInfo->head, pInfo->blk0, pInfo->blkCnt);
		_HBPar[pInfo->board]->pBmpSplitPar[pInfo->head] = pInfo;
		_HBPar[pInfo->board]->checkBlocksFree = TRUE;
	//	if (RX_Spooler.printerType==printer_LH702) _save_to_file(pInfo, TRUE);
		_req_used_flags(_HBPar[pInfo->board], pInfo->head, pInfo->blk0, pInfo->blkCnt, __LINE__);				
		TrPrintfL(_Trace, "Head[%d.%d]: NEW BMP _req_used_flags blkN0=%d, blkCnt=%d", pInfo->board, pInfo->head, pInfo->blk0, pInfo->blkCnt);
	}
	return REPLY_OK;
}

//--- _send_image_cmd ---------------------------------------------
static int _send_image_cmd(SBmpSplitInfo *pInfo)
{
	SFpgaImageCmd	imageCmd;

	imageCmd.hdr.msgId	= CMD_FPGA_IMAGE;
	imageCmd.hdr.msgLen	= sizeof(imageCmd);
	imageCmd.head		= pInfo->head;

	memcpy(&imageCmd.id, &pInfo->pListItem->id, sizeof(imageCmd.id));
	imageCmd.image.flags			= pInfo->pListItem->flags & (FLAG_MIRROR | FLAG_BIDIR);
	imageCmd.image.bitPerPixel		= pInfo->bitsPerPixel;
	imageCmd.image.blkNo			= pInfo->blk0;
	imageCmd.image.blkCnt			= pInfo->blkCnt;
	imageCmd.image.jetPx0			= pInfo->jetPx0;
	imageCmd.image.lengthPx			= pInfo->srcLineCnt;
	if (rx_def_is_lb(RX_Spooler.printerType) && pInfo->srcLineCnt>1)
		imageCmd.image.lengthPx		= pInfo->srcLineCnt-1;	// Bug in FPGA: (when srcLineCnt==12300, gap=0 it sometimes prints an additional line of old data [instead of blank] between the labels)
	imageCmd.image.widthPx			= pInfo->widthPx;
	imageCmd.image.widthBytes		= pInfo->widthBt;
	imageCmd.image.flipHorizontal	= FALSE;
	imageCmd.image.clearBlockUsed	= pInfo->clearBlockUsed;
	
	if (RX_Color[pInfo->inkSupplyNo].rectoVerso==rv_verso) imageCmd.image.flags ^= FLAG_MIRROR;
	if (RX_Spooler.printerType==printer_cleaf)
	{
		imageCmd.image.flags		  = FLAG_MIRROR;
		imageCmd.image.flipHorizontal = TRUE;
	}

	if (pInfo->data==NULL)
		TrPrintfL(_Trace, "SENT _BlkNo[%d][%d]: idx=%d, blk=%d, cnt=%d, buffer=NULL, SENT", pInfo->board, pInfo->head, _TestImgNo[pInfo->board][pInfo->head], pInfo->blk0, pInfo->blkCnt);
	else
		TrPrintfL(_Trace, "SENT _BlkNo[%d][%d]: idx=%d, blk=%d, cnt=%d, buffer=%03d, clearBlockUsed=%d, SENT", pInfo->board, pInfo->head, _TestImgNo[pInfo->board][pInfo->head], pInfo->blk0, pInfo->blkCnt, ctrl_get_bufferNo(*pInfo->data), pInfo->clearBlockUsed);

	_send_image_cmd_flags[5*pInfo->board+pInfo->head] = '*';
	SPageId *pid = &pInfo->pListItem->id;
	TrPrintfL(_Trace, "_send_image_cmd[%d.%d].img[%d] (id=%d, page=%d, copy=%d, scan=%d) flags=%s", pInfo->board, pInfo->head, ++_TestImgNo[pInfo->board][pInfo->head], pid->id, pid->page, pid->copy, pid->scan, _send_image_cmd_flags);
	sok_send(&_HBPar[pInfo->board]->ctrlSocket, &imageCmd);

	if (FALSE)
	{
		TrPrintfL(TRUE, "Head[%d.%d].Image INFO -------------------------", pInfo->board, pInfo->head);
		
		TrPrintfL(TRUE, "bitPerPixel   =%d", imageCmd.image.bitPerPixel);
		TrPrintfL(TRUE, "blkNo         =%d", imageCmd.image.blkNo	);
		TrPrintfL(TRUE, "blkCnt        =%d", imageCmd.image.blkCnt	);
		TrPrintfL(TRUE, "jetPx0        =%d", imageCmd.image.jetPx0	);
		TrPrintfL(TRUE, "lengthPx      =%d", imageCmd.image.lengthPx);
		TrPrintfL(TRUE, "widthPx       =%d", imageCmd.image.widthPx		);
		TrPrintfL(TRUE, "widthBytes    =%d", imageCmd.image.widthBytes	);
		TrPrintfL(TRUE, "flipHorizontal=%d", imageCmd.image.flipHorizontal);
		TrPrintfL(TRUE, "clearBlockUsed=%d", imageCmd.image.clearBlockUsed);
		TrPrintfL(TRUE, "-------------------------------------");
	}

//	pInfo->blkCnt = -pInfo->blkCnt;

	return REPLY_OK;
}

//--- _head_board_ctrl_thread ---------------------------------------------------------
static void *_head_board_ctrl_thread(void *ppar)
{
	SHBThreadPar *par = (SHBThreadPar*)ppar;
	int			i;
	
	rx_set_process_priority(-10);
	
	TrPrintfL(TRUE, "_head_board_ctrl_thread[%d] started", par->cfg.no);

	sok_receiver(NULL, &par->ctrlSocket, _handle_head_ctrl_msg, par);
	
	TrPrintfL(TRUE, "_head_board_ctrl_thread[%d] ended", par->cfg.no);
	sok_close(&par->ctrlSocket);
		for (i=0; i<SIZEOF(par->dataSocket); i++)	sok_close(&par->dataSocket[i]);
	
	par->errorSent = FALSE;
	return NULL;	
}

//--- _handle_head_ctrl_msg --------------------------------------------------------
static int _handle_head_ctrl_msg(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *ppar)
{
	SHBThreadPar	*par  = (SHBThreadPar*) ppar;
	SMsgHdr			*phdr = (SMsgHdr*)msg;
	char			str[32];

	switch(phdr->msgId)
	{
	case REP_PING:			TrPrintfL(1, "Received REP_PING from %s", sok_get_peer_name(socket, str, NULL, NULL));	break;
	case REP_GET_BLOCK_USED:_do_block_used	(par, (SBlockUsedRep*)msg);									break;
	case EVT_GET_EVT:																					break;
	case REP_FPGA_IMAGE:																				break;
	case CMD_STRESS_TEST:	hc_stress_test(par->boardNo);												break;
	default:				Error(WARN, 0, "Got unknown messageId=0x%08x", phdr->msgId);
	}
	return REPLY_OK;
}

//--- _do_block_used ----------------------------------------------------------------------
static int _do_block_used(SHBThreadPar *par, SBlockUsedRep *msg)
{
	int head;
	UINT32 size, end, len;

	if (_Abort) return REPLY_OK;

	head  = msg->headNo;

	if (!par->reqUsedId[head][msg->id]) 
	{
		TrPrintfL(_Trace, "previous flags", 0);
		return REPLY_OK;
	}

	par->reqUsedId[head][msg->id]=FALSE;
	size  = par->cfg.head[head].blkCnt;
	end   = par->cfg.head[head].blkNo0+size;

	if (head<SIZEOF(_TestBlockUsedReqTime))
		TrPrintfL(_Trace, "Head[%d.%d]: _do_block_used id=%d, blkNo=%d, blkCnt=%d, (Block %d .. %d), blkOutIdx=%d blockused_time=%dms", par->cfg.no, msg->headNo, msg->id, msg->blkNo, msg->blkCnt, msg->blkNo, msg->blkNo+msg->blkCnt, msg->blockOutIdx, rx_get_ticks()-_TestBlockUsedReqTime[head][msg->id]);

	if (msg->blkNo+msg->blkCnt < end)
	{
		memcpy(&par->blockUsed[msg->blkNo/32], msg->used, msg->blkCnt/8);
	}
	else
	{
		BYTE *buf = (BYTE*)msg->used;
		len = end-msg->blkNo;
		memcpy(&par->blockUsed[msg->blkNo/32], buf, len/8);
		buf += len/8;
		memcpy(&par->blockUsed[par->cfg.head[head].blkNo0/32], buf, (msg->blkCnt-len)/8);
	}

	if (_Trace>=2)// && head==0)
	{
		int i, l, n, blk;
		char str[256];
		blk=msg->blkNo;
		l = blk%10;
		memset(str, ' ', l);
		for (i=blk, n=0; n<msg->blkCnt; n++)
		{	
			if (par->blockUsed[i/32] & (1<<(i%32))) str[l++]='*';
			else								    str[l++]='.';
			if (i%10==9) str[l++]=' '; 
			if (i%100==99 || i+1==end*8) 
			{
				str[l]=0;
				l=0;
				TrPrintfL(TRUE, "blk[%03d]: %s", blk, str);
				blk=i+1;
			}
			if (++i==end) 
			{
				str[l]=0;
				l=0;
				TrPrintfL(TRUE, "blk[%03d]: %s END", blk, str);
				blk=i=par->cfg.head[head].blkNo0;
			}
		}
		if  (l)
		{
			str[l]=0;
			TrPrintfL(TRUE, "blk[%03d]: %s", blk, str);
		}
		TrPrintfL(_Trace, "Head[%d.%d]: _do_block_used id=%d, blkNo=%d, blkCnt=%d, (Block %d .. %d), blkOutIdx=%d", par->cfg.no, msg->headNo, msg->id, msg->blkNo, msg->blkCnt, msg->blkNo, msg->blkNo+msg->blkCnt, msg->blockOutIdx);
	}
	
	par->blockOutIdx = msg->blockOutIdx;
	_send_to_board(par, msg->headNo, msg->blkNo, msg->blkCnt);
	return REPLY_OK;
}

//---  _req_used_flags ------------------------------------
static void _req_used_flags(SHBThreadPar *par, int head, int blkNo, int blkCnt, int line)
{
	SBlockUsedCmd	*pcmd = &par->blockUsedCmd;	// save stack
	SHeadCfg		*pHead;

	if (_Abort) return;

	rx_mutex_lock(par->mutex);
	pcmd->hdr.msgLen	= sizeof(SBlockUsedCmd);
	pcmd->hdr.msgId	= CMD_GET_BLOCK_USED;
	
	//---- checks ---
	pHead = &par->cfg.head[head];
	if (blkNo >= (int)(pHead->blkNo0 + pHead->blkCnt)) 
		blkNo = pHead->blkNo0 + ((blkNo-pHead->blkNo0) % pHead->blkCnt);

	//--- change to DWORD ----
	blkCnt  = (blkNo+blkCnt+31)&~31;	// max: rounded up
	blkNo  &= ~31;						// min: rounded down
	blkCnt -= blkNo;
	do 
	{
		pcmd->blkNo = blkNo;
		if (blkCnt>MAX_USED_SIZE*8) pcmd->blkCnt = MAX_USED_SIZE*8;
		else						pcmd->blkCnt = blkCnt;
		pcmd->id		= par->blockUsedId;
		pcmd->headNo	= head;
			
		par->reqUsedId[head][par->blockUsedId] = TRUE;
		par->blockUsedId = (par->blockUsedId+1)%MAX_USED_ID;
		if (head<SIZEOF(_TestBlockUsedReqTime))
			_TestBlockUsedReqTime[head][par->blockUsedId] = rx_get_ticks();
		TrPrintfL(_Trace, "Head[%d.%d]: req block used: id=%d, blkNo=%d, blkCnt=%d (blk %d .. %d) line=%d", par->cfg.no, head, pcmd->id, pcmd->blkNo, pcmd->blkCnt, pcmd->blkNo, pcmd->blkNo+pcmd->blkCnt, line);

		sok_send(&par->ctrlSocket, pcmd);
		blkCnt -= pcmd->blkCnt;
		blkNo  += pcmd->blkCnt;
	} while (blkCnt>0);
	rx_mutex_unlock(par->mutex);
}

//--- _send_to_board ----------------------------------------------------------------
static int _send_to_board(SHBThreadPar *par, int head, int blkNo, int blkCnt)
{
	int i;
	int dstBlk; 
	int headMin, headMax;
	int cnt, sent;
	int endReached=FALSE;
	SBmpSplitInfo		*pinfo;
	SHeadCfg			*phead;

	if (_Abort) return REPLY_OK;

	TrPrintfL(_Trace, "_send_to_board board=%d head=%d, blkNo=%d, blkCnt=%d", par->cfg.no, head, blkNo, blkCnt);

	phead = &par->cfg.head[head];
	headMin = phead->blkNo0;
	headMax = phead->blkNo0+phead->blkCnt;

	par->msg.blkNo = -1;
	
	pinfo = par->pBmpSplitPar[head];

	if (FALSE && par->checkBlocksFree)
	{
		int i, n, blk;
		int end   = par->cfg.head[head].blkNo0+par->cfg.head[head].blkCnt;

		blk=pinfo->blk0;
		par->checkBlocksFree = FALSE;
		TrPrintfL(_Trace>=2, "Head[%d.%d]: checkBlocksFree", par->cfg.no, head);
		for (i=blk, n=0; n<pinfo->blkCnt; n++)
		{	
			if (par->blockUsed[i/32] & (1<<(i%32)))
			{
				Error(ERR_STOP, 0, "Head[%d.%d]: _do_block_used blkNo=%d, blkCnt=%d, (Block %d .. %d): Block[%d] not free", par->cfg.no, head, pinfo->blk0, pinfo->blkCnt, par->cfg.head[head].blkNo0, par->cfg.head[head].blkNo0+pinfo->blkCnt, i);
				break;
			}
			if (++i==end) i=par->cfg.head[head].blkNo0;
		}
	}

	blkCnt = pinfo->blkCnt;
	if (pinfo!=NULL)
	{
		TrPrintfL(_Trace, "Head[%d.%d]: First Block=%d, sendFromBlk=%d", par->cfg.no, head, pinfo->blk0+pinfo->sendFromBlk, pinfo->sendFromBlk);
		if (pinfo->pListItem!=NULL)
			TrPrintfL(_Trace, "Head[%d.%d]: _send_to_board (id=%d, page=%d, copy=%d, scan=%d) (Block %d .. %d), sendFrom=%d", par->cfg.no, head, pinfo->pListItem->id.id, pinfo->pListItem->id.page, pinfo->pListItem->id.copy, pinfo->pListItem->id.scan, pinfo->blk0, pinfo->blk0+pinfo->blkCnt-1, pinfo->blk0+pinfo->sendFromBlk);
		else
			TrPrintfL(_Trace, "Head[%d.%d]: _send_to_board (NULL) (Block %d .. %d), sendFrom=%d", par->cfg.no, head, pinfo->blk0, pinfo->blk0+pinfo->blkCnt-1, pinfo->blk0+pinfo->sendFromBlk);
		TrPrintfL(_Trace, "Head[%d.%d]: Send blocks from %d to %d", par->cfg.no, head, blkNo, blkNo+blkCnt);
		dstBlk = pinfo->blk0 + pinfo->sendFromBlk;
		for(i=pinfo->sendFromBlk, cnt=0; i<pinfo->blkCnt && cnt<BLOCK_BURST_CNT; i++, dstBlk++)
		{	
			if (_Abort) return REPLY_OK;

			if (dstBlk >= headMax) dstBlk -= phead->blkCnt;
			if (dstBlk == par->blockOutIdx) 
			{
				TrPrintfL(_Trace, "BlockOutIdx reached %d: blk0=%d, blkCnt=%d, cnt=%d\n", par->blockOutIdx, pinfo->blk0, pinfo->blkCnt, cnt);
				endReached = TRUE;
				break;
			}

			if (par->blockUsed[dstBlk/32] & (1<<dstBlk%32)) // && pinfo->wasFree)
			{
				if (i==pinfo->sendFromBlk) pinfo->sendFromBlk++;
			}
			else
			{
			//	if (i==0) pinfo->wasFree=TRUE;
				par->msg.blkNo = dstBlk;
				if (RX_Spooler.printerType==printer_LH702)
				{
					if (par->blkData[0]!=0x33) 
						Error(LOG, 0, "Error");
					data_fill_blk(pinfo, i, &par->blkData[1], TRUE);
					if (par->blkData[0]!=0x33) 
						Error(LOG, 0, "Error");
					memcpy(par->msg.blkData, &par->blkData[1], sizeof(par->msg.blkData));
				}
				else data_fill_blk(pinfo, i, par->msg.blkData, FALSE);
				/*
				if (TRUE)
				{
					//--- TEST -----------------------------
					int no=dstBlk-pinfo->blk0;
					if      (no<5) memset(par->msg.blkData, 0x00, RX_Spooler.dataBlkSize);
					else if (no==5) memset(par->msg.blkData, 0xff, RX_Spooler.dataBlkSize);
					else if (no<10) memset(par->msg.blkData, 0x00, RX_Spooler.dataBlkSize);
				}
				*/
				/*
				//--- DDR3-TEST ----------------
				{
					UINT32 *buf = (UINT32*)par->msg.blkData;
					int size;
					for (size=0; size<RX_Spooler.dataBlkSize; size+=4)
					{
						*buf++ = 1000000*head+dstBlk;
					}
				}
				*/
				/*
				if (head==2)
				{	//----------- test ---------------------
					if (dstBlk>SIZEOF(_TestBlockSent[head]))
					{
						printf("Overflow\n");
					}
					_TestBlockSent[head][dstBlk]++;
					if (dstBlk==93119)
					{
						printf("Block %d send %d times\n", dstBlk, _TestBlockSent[head][dstBlk]);
					}
					if (dstBlk>_TestLastBlock) _TestLastBlock=dstBlk;
				}
				*/

					sent=send(par->dataSocket[par->udpNo], (char*)&par->msg, sizeof(par->msg.blkNo)+par->cfg.dataBlkSize, 0);
				if (par->dataSocket[1]!=INVALID_SOCKET) 
					par->udpNo = 1-par->udpNo;

				cnt++;
				if ((dstBlk%100)==0 || cnt==1) TrPrintfL(_Trace>1, "Head[%d.%d]: Send Block %d", par->cfg.no, head, dstBlk);
			}			
		}
		TrPrintfL(_Trace, "Head[%d.%d]: *** sent %d blocks, cnt=%d, from=%d ***", par->cfg.no, head, cnt, pinfo->blkCnt, pinfo->sendFromBlk);

		if (_Abort) return REPLY_OK;
		
//		if (pinfo->sendFromBlk >= pinfo->blkCnt || (RX_Spooler.printerType==printer_LB702_UV && (endReached && (cnt==0 || (pinfo->pListItem->flags&FLAG_SAME)))))
		if (pinfo->sendFromBlk >= pinfo->blkCnt || pinfo->pListItem->flags&FLAG_SAME)
		{
			SPageId *pid   = &pinfo->pListItem->id;
			SPageId *plast = &par->lastId[head];
			if (memcmp(plast, pid, sizeof(SPageId)))
			{
				if (pinfo->printMode!=PM_TEST && pinfo->printMode!=PM_TEST_SINGLE_COLOR && pinfo->printMode!=PM_TEST_JETS) _check_image_blocks(par, pinfo, headMin, headMax);
				_send_image_cmd(pinfo);
				static int scan=0;
				TrPrintfL(_Trace, "All sent: blk0=%d, cnt=%d, scan=%d\n", pinfo->blk0, pinfo->blkCnt, ++scan);
//				Error(LOG, 0, "Head[%d]: FILE SENT (id=%d, page=%d, copy=%d) last (id=%d, page=%d, copy=%d)", head, pid->id, pid->page, pid->copy, plast->id, plast->page, plast->copy);				
				memcpy(plast, pid, sizeof(SPageId));
			}

			if (data_sent(pinfo, head))
			{
				hc_send_next();
			}
			return REPLY_OK;
		}
		if(cnt == 0) rx_sleep(10);
		_req_used_flags(par, head, pinfo->blk0 + pinfo->sendFromBlk, pinfo->blkCnt-pinfo->sendFromBlk+1, __LINE__);
	}
	return REPLY_OK;	
}

//--- _check_image_blocks ----------------------------------------------
static int _check_image_blocks (SHBThreadPar *par, SBmpSplitInfo *pInfo, int minBlk, int maxBlk)
{
	int blk, blkNo;

	if (pInfo->blkCnt<2) return REPLY_OK;
	blkNo = pInfo->blk0;
	for (blk=0; blk<pInfo->blkCnt; blk++)
	{
		if ((par->blockUsed[blkNo/32] & (1<<blkNo%32))==0)
			return Error(WARN, 0, "_check_image_blocks Head[%d.%d], %d .. %d MISS %d, flags=0x%08x", pInfo->board, pInfo->head, pInfo->blk0, pInfo->blk0+pInfo->blkCnt, blkNo, par->blockUsed[blkNo/32]);
		if(++blkNo >= maxBlk) blkNo=minBlk;
	}
//	Error(LOG, 0, "_check_image_blocks Head[%d.%d], %d .. %d OK", pInfo->board, pInfo->head, pInfo->blk0, pInfo->blk0+pInfo->blkCnt);
	return REPLY_OK;
}

//--- hc_check ------------------------------------------------------------
void hc_check(void)
{
	static SUDPDataBlockMsg msg;
	static int sent=0;
	static int time=0;
	int old, len;
	int board, port;
	int addr=0;

	if (_Simulation) return;
	
	old=sent;
	for (board=0; board<SIZEOF(_HBPar); board++)
	{
		if (_HBUsed[board] && _HBPar[board]) 
		{
			_open_data_channel(_HBPar[board]);

			for (port=0; port<SIZEOF(_HBPar[0]->dataSocket); port++)
			{
				msg.blkNo  = 0xffffffff;
				if (_HBPar[board]!=NULL && _HBPar[board]->dataSocket[port]!=INVALID_SOCKET && _HBPar[board]->dataSocket[port]!=0)
				{
					len=send(_HBPar[board]->dataSocket[port], (char*)&msg, sizeof(msg.blkNo)+RX_Spooler.dataBlkSize, MSG_NOSIGNAL);
					
					if (FALSE)
					{
						char str[100];
						sok_get_peer_name(_HBPar[board]->dataSocket[port], str, NULL, NULL);
						TrPrintfL(TRUE, "SpoolerNo=%d Alive Sent to >>%s<< len=%d", RX_SpoolerNo, str, len);
					}
					sent++;
					/*
					if (time%10==0) 
					{
						if (_HBPar[board]->cfg.dataAddr[port]!=addr)
						{
							char str[32];
							TrPrintfL(TRUE, "PING %s", sok_addr_str(_HBPar[board]->cfg.dataAddr[port], str));
							sok_ping(sok_addr_str(_HBPar[board]->cfg.dataAddr[port], str));
							addr = _HBPar[board]->cfg.dataAddr[port];
						}
					}
					*/
				}
			}
		}
		time++;
	}
//	if (sent!=old) TrPrintf(TRUE, "SpoolerNo=%d Alive Sent %d", RX_SpoolerNo, sent);
}

//--- hc_stress_test -------------------------------------
void hc_stress_test(int board)
{
	_StressTestRunning = TRUE;
	rx_thread_start(_stress_test_thread, _HBPar[board], 0, "_stress_test_thread");		
}

static void *_stress_test_thread(void *ppar)
{
	SHBThreadPar	*pboard = (SHBThreadPar*)ppar;
	SUDPDataBlockMsg msg;
	int				 msgLen;

	if (pboard==NULL) return NULL;
	
//	rx_set_process_priority(-10);
	
	memset(&msg, 0x55, sizeof(msg));
	msg.blkNo  = 0xffffffff;
	msgLen = sizeof(msg.blkNo)+RX_Spooler.dataBlkSize;
	
	_open_data_channel(pboard);
	while (_StressTestRunning && pboard->dataSocket[0] && pboard->dataSocket[0]!=INVALID_SOCKET)
	{
		send(pboard->dataSocket[0], (char*)&msg, msgLen, MSG_NOSIGNAL);
		send(pboard->dataSocket[1], (char*)&msg, msgLen, MSG_NOSIGNAL);
	}
	return NULL;							
}
