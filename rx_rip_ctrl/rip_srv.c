// ****************************************************************************
//
//	rip_srv.c
//
// ****************************************************************************
//
//	Copyright 2015 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

//--- includes ----------------------------------------------------------------

#include "rx_common.h"
#include "rx_def.h"
#include "rx_error.h"
#include "rx_file.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "rx_sok.h"
#include "tcp_ip.h"
#include "es_rip.h"
#include "emb_rip.h"

#include "rip_srv.h"

//--- Defines -----------------------------------------------------------------
#define MAX_CONNECTIONS 16

typedef struct
{
	RX_SOCKET	socket;
	char		addr[64];
	char		filename[128];
	FILE		*file;
	UINT64		filesize;
	UINT64		pos;
	int			time0;
	int			time;
} SConnection;

//--- Externals ---------------------------------------------------------------


//--- Module Globals -----------------------------------------------------------------
HANDLE  sRipSrv=NULL;

int		RipEngine;
#define RT_ESKO			0
#define RT_EMBEDDED		1

SColorSplitCfg	RX_Color[MAX_COLORS];


SConnection _Connection[MAX_CONNECTIONS]={0};

//--- Prototypes ------------------------------------------------------
static int _handle_rip_msg(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *par);
static int _handle_rip_client_connected(RX_SOCKET socket, const char *peerName);
static int _handle_rip_client_closed(RX_SOCKET socket, const char *peerName);

static void _handle_open	(RX_SOCKET socket, SDsOpenCmd *pmsg);
static void _handle_read	(RX_SOCKET socket, SDsReadCmd *pmsg);
static void _handle_close	(RX_SOCKET socket, SDsCloseCmd *pmsg);

static void _do_rip_start(RX_SOCKET socket, SRipCmd *pcmd);
SConnection* _connection(RX_SOCKET socket);
static void _update_status(void);
static int _do_color_cfg(RX_SOCKET socket, SColorSplitCfg* cfg);

//--- commands --------

//--- rip_init ------------------------------------------
void rip_init(void)
{
	/*	do not start Ergosoft!
	if (esrip_init() == REPLY_OK)
		RipEngine = RT_ESKO;
	else 
	*/
	{
		embrip_init();
		RipEngine = RT_EMBEDDED;
	}

	if (sRipSrv == NULL)
	{
		sok_start_server(&sRipSrv, NULL, PORT_RIP_SERVER, SOCK_STREAM, MAX_CONNECTIONS, _handle_rip_msg, _handle_rip_client_connected, _handle_rip_client_closed);
	}
}

//--- rip_end --------------------------------
void rip_end(void)
{
	if (RipEngine == RT_ESKO)
		esrip_end();
	else
		embrip_end();

	if (sRipSrv != NULL) sok_close(sRipSrv);
}

//--- _connection --------------------------------------
static SConnection* _connection(RX_SOCKET socket)
{
	int i;
	for (i=0; i<SIZEOF(_Connection); i++)
	{
		if (_Connection[i].socket==socket) return &_Connection[i];
		if (_Connection[i].socket==0)      return &_Connection[i];
	}
	return NULL;
}

//--- _handle_rip_client_connected ------------------------------------------------
static int _handle_rip_client_connected(RX_SOCKET socket, const char *peerName)
{
	SConnection* pconnection = _connection(socket);

	if (pconnection)
	{
		pconnection->socket = socket;
		strcpy(pconnection->addr, peerName);
		_update_status();
	}
	return REPLY_OK;
}

//--- _handle_rip_client_closed --------------------------------------------------
static int _handle_rip_client_closed(RX_SOCKET socket, const char *peerName)
{
	SConnection* pconnection = _connection(socket);
	if (pconnection) 
	{
		memset(pconnection, 0, sizeof(*pconnection));
		_update_status();
	}
	return REPLY_OK;
}

//--- _handle_rip_msg ----------------------------------------------------------
static int _handle_rip_msg(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *par)
{
	SMsgHdr *phdr = (SMsgHdr*)msg;
//	TrPrintfL(FALSE, "Data:  Received cmd=0x%08x", phdr->msgId);
	switch (phdr->msgId)
	{
	case CMD_GET_PRINT_ENV:	if (RipEngine == RT_ESKO) esrip_send_print_env(socket);
							else embrip_send_print_env(socket); 
							break;
	case CMD_COLOR_CFG:		_do_color_cfg(socket, (SColorSplitCfg*)&phdr[1]);	break;
	case CMD_RIP_START:		_do_rip_start(socket, (SRipCmd*)msg);	break;

	case CMD_DS_OPEN:		_handle_open (socket, (SDsOpenCmd*)msg);	break;
	case CMD_DS_READ:		_handle_read (socket, (SDsReadCmd*)msg);	break;
	case CMD_DS_CLOSE:		_handle_close(socket, (SDsCloseCmd*)msg);	break;

	default: Error(WARN, 0, "Received unknown msgId 0x%08x", phdr->msgId);
	}
	return REPLY_OK;
}

//--- do_rip_start ---------------------------------
void _do_rip_start(RX_SOCKET socket, SRipCmd *pcmd)
{
	char  path[MAX_PATH];
	char  *ip=NULL, *p=NULL;

	sprintf(path, "net use \\\\%s\\%s /USER:%s %s", RX_CTRL_MAIN, "PrintData", "root", "radex");
	if (rx_process_execute(path, NULL, 1000))
	{
		printf("could not connect to PrintData\n");
	}

	if (str_start(pcmd->filePath, "\\\\")) strcpy(path, pcmd->filePath);
	else if (pcmd->filePath[1]==':')
	{
		// replace drivename by ip address
		snprintf(path, sizeof(path), "\\\\%s\\%s", pcmd->sourceIpAddr, &pcmd->filePath[2]);
	}
	if (RipEngine == RT_ESKO) 
		esrip_rip_file(socket, pcmd->jobName, pcmd->filePath, pcmd->printEnv, pcmd->orientation, pcmd->columns, pcmd->colDist);
	else
		embrip_rip_file(socket, pcmd->jobName, pcmd->filePath, pcmd->printEnv, pcmd->orientation, pcmd->columns, pcmd->colDist);

}

//--- _handle_open -------------------------------------------------
void _handle_open	(RX_SOCKET socket, SDsOpenCmd *pmsg)
{
	SDsOpenRep msg;
	SConnection* pconnection = _connection(socket);

	msg.hdr.msgLen = sizeof(msg);
	msg.hdr.msgId  = REP_DS_OPEN;
	msg.file_id    = 1;
	msg.filetime   = rx_file_get_mtime(pmsg->filepath);
	msg.size	   = pconnection->filesize = rx_file_get_size(pmsg->filepath);
	strncpy(msg.filepath, pmsg->filepath, sizeof(msg.filepath));
	pconnection->pos=0;
	strcpy(pconnection->filename, pmsg->filepath);
	pconnection->file = rx_fopen(pmsg->filepath, "rb", _SH_DENYNO);
	sok_send(&socket, &msg);
}

//--- _handle_read ----------------------------------------------------
static void _handle_read	(RX_SOCKET socket, SDsReadCmd *pmsg)
{
	UINT i;
	int size;
	SDsReadRep *msg = malloc(sizeof(SDsReadRep)-1+pmsg->block_size);
	SConnection* pconnection = _connection(socket);

	if (pconnection->time0==0) pconnection->time0 = rx_get_ticks();
	for (i=0; i<pmsg->cnt; i++)
	{
	//	rx_file_seek(_file, pmsg->block*pmsg->block_size, SEEK_SET);
		pconnection->pos= pmsg->block*pmsg->block_size;
		msg->hdr.msgId  = REP_DS_READ;
		msg->file_id    = pmsg->file_id;
		msg->block		= pmsg->block+i;
		size			= (int)fread(msg->data, 1, pmsg->block_size, pconnection->file);
		msg->hdr.msgLen = sizeof(SDsReadRep)-1+ size;
		sok_send(&socket, msg);
	}
	pconnection->time = rx_get_ticks();
	free(msg);
}

//--- _handle_close -----------------------------------------------------
void _handle_close	(RX_SOCKET socket, SDsCloseCmd *pmsg)
{
	SConnection* pconnection = _connection(socket);
	if (pconnection && pconnection->file)
	{
		pconnection->pos=pconnection->filesize;
		fclose(pconnection->file);
	}
}

//--- _update_status ----------------------------------------------------
static void _update_status(void)
{
	/*
	int i, t;

	goto_xy(0,7);
	printf(" # | Sok  | Addr                 | File                                                          | %%  | ms     | MB/s\n");
	printf("---|------|----------------------|---------------------------------------------------------------|----|--------|-----\n");

	for (i=0; i<SIZEOF(_Connection); i++)
	{
		t = _Connection[i].time-_Connection[i].time0;
		if (!t) t=1;
		if (_Connection[i].filesize)
			printf("%2d: %5d  %-22s %-64s %-3d  %3d'%03d   %-3d\n", i+1, _Connection[i].socket, _Connection[i].addr, _Connection[i].filename, (int)(100*_Connection[i].pos/_Connection[i].filesize), t/1000, t%1000, (int)(1000.0*_Connection[i].pos/t/1024/1024));
		else
			printf("%2d: %5d  %-22s %-64s                   \n", i+1, _Connection[i].socket, _Connection[i].addr, _Connection[i].filename);
	}
	*/
}


//--- _do_color_cfg ---------------------------------------------------------------------
static int _do_color_cfg(RX_SOCKET socket, SColorSplitCfg* cfg)
{
	TrPrintfL(TRUE, "_do_color_cfg[%d], >>%s<<, firstLine=%d, lastLine=%d", cfg->no, cfg->color.name, cfg->firstLine, cfg->lastLine);

	if (cfg->no<SIZEOF(RX_Color))
	{
		memcpy(&RX_Color[cfg->no], cfg, sizeof(RX_Color[cfg->no]));
	}
	else Error(ERR_CONT, 0, "CMD_COLOR_CFG no=%d out of range", cfg->no);
	return REPLY_OK;
}