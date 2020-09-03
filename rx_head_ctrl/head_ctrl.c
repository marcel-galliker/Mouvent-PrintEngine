// ****************************************************************************
//
//	head_ctrl.c
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rx_def.h"
#include "rx_error.h"
#include "rx_file.h"
#include "rx_threads.h"
#include "rx_sok.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "args.h"
#include "tcp_ip.h"
#include "fpga.h"
#include "fpga_simu.h"
#include "nios.h"
#include "rx_head_ctrl.h"
#include "head_ctrl.h"
#include "conditioner.h"

#ifdef linux
	#include <sys/socket.h>
	#define SOCKET int
#endif

#define MAX_CONNECTIONS	5
#define MSG_BUF_SIZE	256
#define MSG_MSG_SIZE	1024

//--- buffer ---------------------------------------------------------
typedef struct
{
	RX_SOCKET socket;
	UCHAR	  msg[MSG_MSG_SIZE];
} SFpgaMsg;

static SFpgaMsg _MsgBuf[MSG_BUF_SIZE];
static int		_MsgBufIn;
static int		_MsgBufOut;
static int		_LastMsgId;
static int		_Printing;
static RX_SOCKET	_SpoolerSocket;

//--- module globals -----------------------------------------------------------------
static HANDLE			_HServer;

// static UINT32			_PgCnt[MAX_HEADS_BOARD];

static int				_StatusReqTime=0;

//--- prototypes ---------------------------------------------------------------------
static int _ctrl_connected  (RX_SOCKET socket, const char *peerName);
static int _ctrl_deconnected(RX_SOCKET socket, const char *peerName);
static int _save_ctrl_msg   (RX_SOCKET socket, void *pmsg, int len, struct sockaddr *sender, void *par);
static int _handle_ctrl_msg (RX_SOCKET socket, void *pmsg);

//--- command Handlers ---------------------------------------------------------------
static int _do_ping				(RX_SOCKET socket);
static int _do_error_reset		(void);
static int _do_block_used		(RX_SOCKET socket, SBlockUsedCmd 	*pmsg);
static int _do_fpga_image		(RX_SOCKET socket, SFpgaImageCmd 	*pmsg);
static int _do_simu_print		(RX_SOCKET socket);
static int _do_simu_encoder		(RX_SOCKET socket, UINT32			*khz);
static int _do_write_image	 	(RX_SOCKET socket, SFpgaWriteBmpCmd *pmsg);
static int _do_head_board_cfg 	(RX_SOCKET socket, SHeadBoardCfg 	*pcfg);
static int _do_head_stat		(RX_SOCKET socket, SFluidStateLight *pmsg);
static int _rep_head_stat		(RX_SOCKET socket);
static int _do_inkdef			(RX_SOCKET socket, SInkDefMsg	    *pmsg);
static int _do_print_abort		(RX_SOCKET socket);
static int _do_set_FluidCtrlMode(RX_SOCKET socket, SFluidCtrlCmd	*pmsg);
static int _do_set_purge_par	(RX_SOCKET socket, SPurgePar		*ppar);
static int _do_disabled_jets	(RX_SOCKET socket, SDisabledJetsMsg  *pmsg);
static int _do_density_values	(RX_SOCKET socket, SDensityValuesMsg *pmsg);
static int _do_rob_pos		    (RX_SOCKET socket, SRobPositionMsg *pmsg);

//--- ctrl_init --------------------------------------------------------------------
int ctrl_init()
{
	_MsgBufIn  = 0;
	_MsgBufOut = 0;
	_Printing  = FALSE;
	RX_MainSocket = INVALID_SOCKET;
	_SpoolerSocket = INVALID_SOCKET;
	sok_start_server(&_HServer, NULL, PORT_CTRL_HEAD, SOCK_STREAM, MAX_CONNECTIONS, _save_ctrl_msg, _ctrl_connected, _ctrl_deconnected);

	err_set_server(_HServer);
	
	return REPLY_OK;
}

//--- ctrl_end --------------------------------------------------------------------
int ctrl_end()
{
	return REPLY_OK;
}

//--- _ctrl_connected ---------------------------------------------------
static int _ctrl_connected (RX_SOCKET socket, const char *peerName)
{
	TrPrintfL(TRUE, "connected from >>%s<<, socket=%d", peerName, socket);
	return REPLY_OK;
}

//--- _ctrl_connected ---------------------------------------------------
static int _ctrl_deconnected (RX_SOCKET socket, const char *peerName)
{
	TrPrintfL(TRUE, "deconnected from >>%s<<, socket=%d", peerName, socket);
	if (socket==_SpoolerSocket) _SpoolerSocket=INVALID_SOCKET;
	if (socket==RX_MainSocket)
	{
		TrPrintfL(TRUE, "deconnected from MAIN");
		RX_MainSocket = INVALID_SOCKET;
	}
	return REPLY_OK;
}

//--- ctrl_connected --------------------------------------
int	ctrl_connected(void)
{
	return RX_MainSocket!=INVALID_SOCKET;	
}

//--- ctrl_printing --------------------------------------
int	ctrl_printing(void)
{
	return _Printing;	
}

//--- _save_ctrl_msg ---------------------------------------------------------
static int _save_ctrl_msg(RX_SOCKET socket, void *pmsg, int len, struct sockaddr *sender, void *par)
{
	SMsgHdr *phdr = (SMsgHdr*)pmsg;
	static int time[MSG_BUF_SIZE];	
	
	// these functions mustn't use any FPGA Register !!!!
	switch (phdr->msgId)
	{
	case 0:				return REPLY_OK;
	case CMD_PING:		_do_ping(socket);
						_SpoolerSocket = socket;
																			break;
	case CMD_HEAD_STAT: _do_head_stat (socket, (SFluidStateLight*) &phdr[1]); break;
	default:		{
						// ALL messages that use FPGA Registers
						int idx = (_MsgBufIn + 1) % MSG_BUF_SIZE;
						if (idx == _MsgBufOut) 
						{			
							//ErrorFlag(ERR_CONT, (UINT32*)&RX_HBStatus[0].err, err_tcpip_overflow, 0, "TCP Message Buffer Overflow");
							ErrorFlag(ERR_CONT, (UINT32*)&RX_HBStatus[0].err, err_tcpip_overflow, 0, "TCP Message Buffer Overflow. _MsgBufIn=%d,  _MsgBufOut=%d, id=0x%08x", _MsgBufIn, _MsgBufOut, _LastMsgId);
							{
								phdr = (SMsgHdr*)&_MsgBuf[idx].msg;
								TrPrintfL(1, "msg[%d] %d: 0x%08x", idx, time[idx], phdr->msgId);
								idx = (idx + 1) % MSG_BUF_SIZE;
							}
						}
						else
						{
							time[_MsgBufIn] = rx_get_ticks();
							_MsgBuf[_MsgBufIn].socket = socket;
							if (len>sizeof(_MsgBuf[0].msg))
								Error(ERR_ABORT, 0, "Message Buffer Overflow, msgsize=%d, bufsize=%d", len, sizeof(_MsgBuf[0].msg));
			
							memcpy(_MsgBuf[_MsgBufIn].msg, pmsg, len);
							_MsgBufIn = idx;
						}
					}
					break;
	}
		
	return REPLY_OK;
}

//--- ctrl_main -------------------
int  ctrl_main(int ticks, int menu)
{
	int cnt=0;
	
	if (_StatusReqTime && rx_get_ticks()>(_StatusReqTime+1500)) 
	{
		if (!RX_HBConfig.debug) Error(LOG, 0, "Status Request Late: time=%d", rx_get_ticks()-_StatusReqTime);
		_StatusReqTime = 0;
	}

	while (_MsgBufOut!=_MsgBufIn)
	{
		cnt++;
		
		_handle_ctrl_msg(_MsgBuf[_MsgBufOut].socket, _MsgBuf[_MsgBufOut].msg);
		_MsgBufOut = (_MsgBufOut+1) % MSG_BUF_SIZE;
		break; // only one per call
	};
	return cnt;
}

//--- _handle_ctrl_msg ---------------------------------------------------------
static int _handle_ctrl_msg(RX_SOCKET socket, void *pmsg)
{
	int reply = REPLY_ERROR;
	int time=rx_get_ticks();

	//--- handle the message --------------
	reply = REPLY_OK;
	SMsgHdr *phdr = (SMsgHdr*)pmsg;
	
	_LastMsgId = phdr->msgId;
	switch (phdr->msgId)
	{
//	case CMD_PING:					_do_ping			(socket);								break;
	case CMD_ERROR_RESET:			_do_error_reset		();										break;
	case CMD_FPGA_IMAGE:			_do_fpga_image		(socket, (SFpgaImageCmd*)	 pmsg);		break;
	case CMD_FPGA_WRITE_BMP:		_do_write_image		(socket, (SFpgaWriteBmpCmd*) pmsg);		break;
	case CMD_FPGA_SIMU_PRINT:		_do_simu_print		(socket);								break;
	case CMD_FPGA_SIMU_ENCODER:		_do_simu_encoder	(socket, (UINT32*)		   &phdr[1]);	break;
	case CMD_GET_BLOCK_USED:		_do_block_used		(socket, (SBlockUsedCmd*)	pmsg);		break;
	case CMD_HEAD_BOARD_CFG:		_do_head_board_cfg	(socket, (SHeadBoardCfg*)  &phdr[1]);	break;
	case CMD_PRINT_ABORT:			_do_print_abort		(socket);								break;		
	case CMD_HEAD_STAT:				_do_head_stat       (socket, (SFluidStateLight*) &phdr[1]); break;
	case SET_GET_INK_DEF:			_do_inkdef			(socket, (SInkDefMsg*)		pmsg);		break;
	case CMD_HEAD_FLUID_CTRL_MODE:	_do_set_FluidCtrlMode(socket, (SFluidCtrlCmd*)  pmsg);		break;
	case CMD_SET_PURGE_PAR:			_do_set_purge_par	(socket, (SPurgePar*)	&phdr[1]);		break;
    case CMD_SET_DISABLED_JETS:		_do_disabled_jets	(socket, (SDisabledJetsMsg*)pmsg);		break;
    case CMD_SET_DENSITY_VAL:		_do_density_values	(socket, (SDensityValuesMsg*)pmsg);		break;
    case CMD_SET_ROB_POS:			_do_rob_pos			(socket, (SRobPositionMsg*)pmsg);			break;
	default:		Error(LOG, 0, "Unknown Command 0x%04x", phdr->msgId);
					reply = REPLY_ERROR;
					break;
	}
	time = rx_get_ticks()-time;
	if (_Printing && phdr->msgId!=CMD_HEAD_BOARD_CFG && time>100) Error(WARN, 0, "_handle_ctrl_msg id=0x%08x, time=%dms", phdr->msgId, time); 
	return reply;
};

//--- ctrl_stress_test ----------------------------------
void ctrl_stress_test(void)
{
	sok_send_2(&_SpoolerSocket, CMD_STRESS_TEST, 0, NULL);
}

//=== command  handlers ===========================================================================

//--- _do_ping -------------------------------------------------------------------------------------
static int _do_ping(RX_SOCKET socket)
{
	static int cnt=0;

	TrPrintf(1, "got CMD_PING");

	SMsgHdr msg;
	int len;
	msg.msgLen = sizeof(msg);
	msg.msgId  = REP_PING;
	len = sok_send(&socket, (char*)&msg);
	TrPrintf(1, "Sent REP_PING, len=%d", len);
	/*
	switch(cnt%3)
	{
	case 0: 	Error(LOG,		100, "do PING cnt=%d, str>>%s<<", cnt, "Log-Entry vom Kopf"); 	break;
	case 1: 	Error(WARN,		100, "do PING cnt=%d, str>>%s<<", cnt, "Warnung Kopf"); 		break;
	case 2: 	Error(ERR_CONT, 100, "do PING cnt=%d, str>>%s<<", cnt, "Error vom Kopf"); 		break;
	}
	*/
	cnt++;
	return REPLY_OK;
}

//--- _do_block_used ----------------------------------------------------------
static int _do_block_used	(RX_SOCKET socket, SBlockUsedCmd *msg)
{
	SBlockUsedRep 	reply;
	UINT32			min, max;

//	TrPrintfL(TRUE, "head[%d]._do_block_used id=%d, blkNo=%d, blkCnt=%d (blk %d .. %d)", msg->headNo, msg->id, msg->blkNo, msg->blkCnt, msg->blkNo, msg->blkNo+msg->blkCnt);
	
	min=RX_HBConfig.head[msg->headNo].blkNo0;
	max=min+RX_HBConfig.head[msg->headNo].blkCnt;
	if (msg->blkNo>=min && msg->blkNo<max)
	{
		int time	= rx_get_ticks();
		
		reply.hdr.msgId		= REP_GET_BLOCK_USED;
		reply.aliveCnt[0]   = fpga_get_aliveCnt(0);
		reply.aliveCnt[1]   = fpga_get_aliveCnt(1);
 		reply.blockOutIdx	= fpga_get_blockOutIdx(msg->headNo);
		reply.headNo		= msg->headNo;
		reply.id			= msg->id;
		reply.blkNo			= msg->blkNo;
		reply.blkCnt		= fpga_get_block_used(reply.headNo, reply.blkNo, msg->blkCnt, reply.used);
		reply.hdr.msgLen = sizeof(reply)-sizeof(reply.used)+reply.blkCnt/8;
		sok_send(&socket, &reply);
		
		time = rx_get_ticks()-time;
		if (time>100) Error(WARN, 0, "_do_block_used time=%d", time);
		
		return REPLY_OK;						
	}
	return Error(ERR_ABORT, 0, "Used Block %d out of range (%d..%d)!", msg->blkNo, min, max);
}

//--- _do_fpga_image -------------------------------------------------------
static int _do_fpga_image		(RX_SOCKET socket, SFpgaImageCmd *msg)
{
	fpga_image(msg);

	sok_send_2(&socket, REP_FPGA_IMAGE, 0, NULL);
	
	return REPLY_OK;
}

//--- _do_write_image -------------------------------------------------------
static int _do_write_image(RX_SOCKET socket, SFpgaWriteBmpCmd *msg)
{
//	simu_write_image(msg->head, msg->imageNo);
	return REPLY_OK;
}

//--- _do_head_board_cfg --------------------------------------------------
static int _do_head_board_cfg 	(RX_SOCKET socket, SHeadBoardCfg *cfg)
{
	int i;
	
	TrPrintfL(TRUE, "_do_head_board_cfg (%d)", cfg->resetCnt);

	RX_MainSocket = socket;
	memcpy(&RX_HBConfig, cfg, sizeof(RX_HBConfig));
	RX_HBStatus[0].warn = 0;
	RX_HBStatus[0].err = 0;

	for (i=0; i<SIZEOF(RX_HBConfig.head); i++)
	{
		if (RX_HBConfig.head[i].blkNo0 % 8) Error(ERR_ABORT, 0, "Head[%d].blockNo0 (%d) must be a multiple of 8", i, RX_HBConfig.head[i].blkNo0);
		if (RX_HBConfig.head[i].blkCnt % 8) Error(ERR_ABORT, 0, "Head[%d].blockCnt (%d) must be a multiple of 8", i, RX_HBConfig.head[i].blkCnt);
	}

	fpga_set_config(RX_MainSocket);
	nios_error_reset();
//	memset(_PgCnt,  0, sizeof(_PgCnt));
	
	_Printing = TRUE;
	
	sok_send_2(&socket, REP_HEAD_BOARD_CFG, sizeof(cfg->resetCnt), &cfg->resetCnt);
	return REPLY_OK;
}

//--- _do_error_reset -------------------------------------
static int _do_error_reset(void)
{
	RX_HBStatus[0].warn = 0;
	RX_HBStatus[0].err  = 0;
	nios_error_reset();

	return REPLY_OK;				
}

//--- _do_head_stat --------------------------------------------------
static int _do_head_stat(RX_SOCKET socket, SFluidStateLight *pmsg)
{	
//	TrPrintfL(TRUE, "*** _do_head_stat(socket=%d) time=%d ***", socket, rx_get_ticks()-_StatusReqTime);
	_StatusReqTime = rx_get_ticks();

	memcpy(RX_FluidStat, pmsg, sizeof(RX_FluidStat));

	
	int head;
	for (head = 0; head < MAX_HEADS_BOARD; head++)
	{
	//	_NiosMem->cfg.cond[head].cylinderPressure    = RX_FluidStat[head].cylinderPressure;
	//	_NiosMem->cfg.cond[head].cylinderPressureSet = RX_FluidStat[head].cylinderPressureSet;
    //	_NiosMem->cfg.cond[head].fluidErr            = RX_FluidStat[head].fluidErr;
        _NiosMem->cfg.cond[head].purge_pos_y		 = RX_FluidStat[head].act_pos_y;
        //Error(LOG, 0, "Head: %d; Act Pos y: %d; PurgeDelayPos: %d", head, _NiosMem->cfg.cond[head].purge_pos_y,
        //      _NiosMem->cfg.cond[head].purgeDelayPos_y);
	}
	
	_rep_head_stat(socket);
	return REPLY_OK;
}

//--- _rep_head_stat ---------------------------------------------------
static int _rep_head_stat(RX_SOCKET socket)
{
	int len;
	len = sok_send_2(&socket,  REP_HEAD_STAT, sizeof(RX_HBStatus[0]), &RX_HBStatus);
	return REPLY_OK;
}

//--- _do_print_abort ---------------------------------------------------
static int _do_print_abort(RX_SOCKET socket)
{
	_Printing=FALSE;
	fpga_abort();
	return REPLY_OK;	
}

//--- _do_inkdef -------------------------------
static int _do_inkdef(RX_SOCKET socket, SInkDefMsg  *pmsg)
{
	nios_setInk(pmsg->headNo, &pmsg->ink, pmsg->dots, pmsg->fpVoltage);
	
	return REPLY_OK;
}

//--- _do_set_FluidCtrlMode -------------------------
static int _do_set_FluidCtrlMode(RX_SOCKET socket, SFluidCtrlCmd *pmsg)
{
	cond_ctrlMode(pmsg->no, pmsg->ctrlMode);
//	rx_sleep(10);
//	_rep_head_stat(socket);	// give feedback now!
	return REPLY_OK;
}

//--- _do_set_purge_par -------------------------------------
static int _do_set_purge_par(RX_SOCKET socket, SPurgePar *ppar)
{
	cond_set_purge_par(ppar->no, ppar->delay_pos_y, ppar->time, ppar->act_pos_y);
	return REPLY_OK;
}

//--- _do_disabled_jets ----------------------------------------------
static int _do_disabled_jets(RX_SOCKET socket, SDisabledJetsMsg *pmsg)
{
	cond_set_disabledJets(pmsg->head, pmsg->disabledJets);
	return REPLY_OK;
}

//--- _do_density_values ----------------------------------------------
static int _do_density_values	(RX_SOCKET socket, SDensityValuesMsg *pmsg)
{
	rx_sleep(100);
	cond_set_densityValues(pmsg->head, pmsg->value);
	cond_set_voltage(pmsg->head, pmsg->voltage);
	rx_sleep(100);
	return REPLY_OK;
}

//--- _do_rob_pos -------------------------------------------------------------
static int _do_rob_pos		    (RX_SOCKET socket, SRobPositionMsg *pmsg)
{
	cond_set_rob_pos(pmsg->head, pmsg->angle, pmsg->dist);
	return REPLY_OK;	
}

//--- _do_simu_print ------------------------------------------------------
static int _do_simu_print		(RX_SOCKET socket)
{
	if (arg_simu_machine) fpga_manual_pg();
	return REPLY_OK;
}

//--- _do_simu_encoder ------------------------------------------
static int _do_simu_encoder		(RX_SOCKET socket, UINT32 *khz)
{
	Error(LOG, 0, "_do_simu_encoder(%d)", *khz);
	fpga_enc_config(*khz);
	if (TRUE)
	{
		if (((int)*khz)>0) 
		{
			rx_sleep(200);
			fpga_manual_pg();
		}
	}
	return REPLY_OK;
}

//--- ctrl_send_file -----------------------------------------
void ctrl_send_file(char *filepath)
{
	FILE *f;
	int	blkSize		=1024;
	int len;
	SFSDirEntry		hdr;
	SDataBlockMsg	*msg;

	f  = fopen(filepath, "rb");
	if (f!=NULL)
	{
		//--- send header ---
		memset(&hdr, 0, sizeof(hdr));
		hdr.hdr.msgLen = sizeof(hdr);
		hdr.hdr.msgId  = CMD_RFS_SAVE_FILE_HDR;
		strcpy(hdr.name, filepath);
		fseek(f, 0, SEEK_END);
		hdr.size	   = ftell(f);
		fseek(f, 0, SEEK_SET);

		sok_send(&RX_MainSocket, &hdr);
		
		//--- send data -------------------
		msg = (SDataBlockMsg*)malloc(sizeof(msg->hdr)+blkSize);
		msg->hdr.msgId = CMD_RFS_SAVE_FILE_BLOCK;
		while(TRUE) 
		{
			len=fread(msg->data, 1, blkSize, f);
			if (len<=0) break;
			msg->hdr.msgLen = sizeof(msg->hdr)+len;
			sok_send(&RX_MainSocket, msg);
		}
		free(msg);
		fclose(f);			
	}
}

