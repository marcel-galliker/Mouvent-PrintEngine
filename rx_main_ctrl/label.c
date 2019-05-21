// ****************************************************************************
//
//	label.cpp
//
// ****************************************************************************
//
//	Copyright 2015 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

//--- includes ----------------------------------------------------------------
#include "rx_common.h"
#include "rx_error.h"
#include "rx_file.h"
#include "rx_threads.h"
#include "rx_xml.h"
#include "rx_data.h"
#include "rx_counter.h"
#include "rx_rip_lib.h"
#include "print_queue.h"
#include "es_rip.h"
#include "rip_clnt.h"	
#include "spool_svr.h"
#include "tcp_ip.h"
#include "label.h"

//--- global variables ---------------------------------------------------
// static char *_TempPath="D:\\MainTemp\\";

static char				_ActFilePath[MAX_PATH]="";
static char				_ActDataPath[MAX_PATH]="";
static SFileDef			_FileDef;
static SLayoutDef		_Layout;
static SCounterDef		_CtrDef={.size=sizeof(SCounterDef)};
static int				_Send=FALSE;
static int				_MaxDataSize;
static SPrintDataMsg	*_DataMsg=NULL;

static HANDLE			_FontSem=NULL;
static int				_ReplyCnt, _ReplyExp;
static int				_UpdateFonts;

//--- label_reset -------------------------------------------------------
void label_reset()
{
	_Send = TRUE;
	if (_DataMsg==NULL) _DataMsg = (SPrintDataMsg*)malloc(MAX_MESSAGE_SIZE);
	_MaxDataSize = MAX_MESSAGE_SIZE - sizeof(SPrintDataMsg)-1;
	_MaxDataSize /=2;
}

//--- font_callback -----------------------------------------
static void _font_callback(const char *fontname)
{
	printf("Font >>%s%s<<\n", PATH_TEMP, fontname);
	
	SFSDirEntry		hdr;
	SDataBlockMsg	*msg;
	FILE			*file;
	int				blkSize=2000;
	int				len;
	char			path[MAX_PATH];

	sprintf(path, "%s%s", PATH_TEMP, fontname);

	file = rx_fopen(path, "rb", _SH_DENYNO);
	if (file)
	{
		//--- send header ---
		memset(&hdr, 0, sizeof(hdr));
		hdr.hdr.msgLen = sizeof(hdr);
		hdr.hdr.msgId  = CMD_RFS_SAVE_FILE_HDR;
		strcpy(hdr.name, fontname);
		_ReplyCnt = 0;
		_ReplyExp = spool_send_msg(&hdr);
		
		//--- send blocks ---
		msg = (SDataBlockMsg*)malloc(sizeof(msg)+blkSize);
		msg->hdr.msgId = CMD_RFS_SAVE_FILE_BLOCK;
		do
		{
			len = (int)fread(msg->data, 1, blkSize, file);
			msg->hdr.msgLen = sizeof(msg->hdr)+len;
	
			if (rx_sem_wait(_FontSem, 1000)) return;
	
			_ReplyCnt = 0;
			_ReplyExp = spool_send_msg(msg);
			// len=0 signals "end of file"
		} while (len);
		fclose(file);
	}
}

//--- label_rep_file_hdr -------------------------------
void label_rep_file_hdr()
{
	_ReplyCnt++;
	if (_ReplyCnt==_ReplyExp) rx_sem_post(_FontSem);
}

//--- label_rep_file_block ------------------------------
void label_rep_file_block()
{
	_ReplyCnt++;
	if (_ReplyCnt==_ReplyExp) rx_sem_post(_FontSem);
}


//--- label_load -------------------------------------------------------
int label_load(SPrintQueueItem *pitem, char *datapath, int size)
{
	if (strcmp(pitem->filepath, _ActFilePath))
	{
		void *doc=NULL;
		char name[MAX_PATH];
		char path[MAX_PATH];;
		char ext[16];
		char labelName[MAX_PATH];
		char colorName[MAX_PATH];
		char str[MAX_PATH];
		char ripState1[MAX_PATH]="";
		char ripState2[MAX_PATH]="";
		char *start;

		memset(_ActFilePath, 0, sizeof(_ActFilePath));
		split_path(pitem->filepath, path, name, ext);
		start = strstr(path, "ripped-data");
		if (start) sprintf(_ActDataPath, PATH_RIPPED_DATA "%s/%s", &start[strlen("ripped-data")], name);
		else Error(ERR_ABORT, 0, "INVALID PATH >>%s<<", path);
#ifdef linux
		{	char *ch;
			for (ch = _ActDataPath; *ch; ch++) if (*ch == '\\') *ch = '/'; 
		}
#endif
		
		//--- load definition -------------------------------------------
		if (_FontSem==NULL) _FontSem=rx_sem_create();
//		doc = rip_open_xml_local(pitem->filepath, _TempPath);
		sprintf(path, "%s/%s.rxd", _ActDataPath, name);
		doc = rip_open_xml_local(path, _ActDataPath);
		/*
		if (FALSE)
		{
			//--- check local copy -----------------------
			split_path(pitem->filepath, NULL, name, ext);
			sprintf(localPath, "%s%s%s", _ActDataPath, name,  ext);
			if (!rx_file_del_if_older(localPath, pitem->filepath))
				doc = rx_xml_load(localPath);
			if (doc) localPath[0]=0;
			else doc = rx_xml_load (pitem->filepath);
		}
		*/

		if (doc==0) return Error(ERR_CONT, 0, "Cound not load label >>%s<<", pitem->filepath);
		_FileDef.size = sizeof(_FileDef);
		_Layout.size  = sizeof(_Layout);
		dat_load_file_def(doc, _ActDataPath, &_FileDef);
		ctr_load_def     (doc, &_CtrDef);
		rip_load_layout  (doc, _ActDataPath, &_Layout);
	//	rip_load_files   (doc, _ActDataPath, NULL);
		_FileDef.id = _Layout.id = pitem->id.id;
		if (*_Layout.label)		 sprintf(labelName, "%s", name); else *labelName=0;
		if (*_Layout.colorLayer) sprintf(colorName, "%s-Color", name); else *colorName=0;
		
		if (*_Layout.colorLayer){ strcpy(str, _Layout.colorLayer);	sprintf(_Layout.colorLayer, "%s/%s", _ActDataPath, str); }
		if (*_FileDef.dataFile) { strcpy(str, _FileDef.dataFile);	sprintf(_FileDef.dataFile,	"%s/%s", _ActDataPath, str); }
			
		//--- send fonts to spoolers ---
		if (FALSE)
		{
			rip_enum_fonts(doc, _font_callback);
			/*
			SMsgHdr hdr;
			hdr.msgId = CMD_FONTS_UPDATED;
			hdr.msgLen = sizeof(hdr);
			spool_send_msg(&hdr);
			spool_send_msg_2(CMD_FONTS_UPDATED, strlen(_ActDataPath)+1, _ActDataPath);
			*/
		}
		
		/*
		//--- rip the data -----------------------------------------------
		if (rip_file_ripped(labelName, pitem->printEnv, name, ripState1))
		{
			strcpy(pitem->ripState, "Waiting for RIP");
			pq_ripping(pitem); 
			if (rip_file(labelName, _Layout.label, pitem->printEnv, 0, _Layout.columns, _Layout.columnDist)) return Error(ERR_CONT, 0, "RIP Error ripping >>%s<<-Label", pitem->filepath);
		}

		if (rip_file_ripped(colorName, pitem->printEnv, name, ripState2))
		{
			strcpy(pitem->ripState, "Waiting for RIP");
			pq_ripping(pitem);
			if (rip_file(colorName, _Layout.colorLayer,  pitem->printEnv, 0, _Layout.columns, _Layout.columnDist)) return Error(ERR_CONT, 0, "RIP Error ripping >>%s<<-Color", pitem->filepath);;
		}

		while (rip_file_ripped(labelName, pitem->printEnv, _Layout.label, ripState1) || rip_file_ripped(colorName, pitem->printEnv, _Layout.colorLayer, ripState2))
		{
			if (*ripState1)
			{
				strcpy(pitem->ripState, ripState1); 
				pq_ripping(pitem);			
			}
			else if (*ripState2)
			{
				strcpy(pitem->ripState, ripState2); 
				pq_ripping(pitem);
			} 

			rx_sleep(100);
		}
		strcpy(pitem->ripState, "Ripping done");		
		*/
		pq_ripping(pitem);
		//--- open data file -----------------------------------------------------
		dat_set_file_def(&_FileDef);

		strcpy(_ActFilePath, pitem->filepath);

		if (*_Layout.label)		strcpy(_Layout.label, _ActDataPath);
		_Send = TRUE;
	}
//	strncpy(_ActDataPath, pitem->filepath, size);

	if (_Send)
	{
		spool_set_filedef(&_FileDef);
		spool_set_counter(&_CtrDef);
		spool_set_layout(&_Layout, _ActDataPath);
		_Send = FALSE;
	}

	pitem->pageWidth = pitem->srcWidth;
	if (_Layout.columns > 1) pitem->pageWidth = _Layout.columnDist*(_Layout.columns-1)+pitem->srcWidth;
	strcpy(datapath, _ActDataPath);
	return REPLY_NEW_JOB;	
}

//--- label_send_data --------------------------------------------
int  label_send_data(SPageId *pid)
{
	int len, i, cnt, idlen;

	dat_seek((pid->page-1)*_Layout.columns);
	idlen = sizeof(UINT32);
	for (i=0; i<_Layout.columns; i++)
	{
		len = dat_read_next_record();
//		if (len>0)
		{
			if (len>_MaxDataSize)
			{
				Error(WARN, 0, "Data Record[%d] len=%d too long (max=%d)", pid->id, len, _MaxDataSize);
				len = _MaxDataSize;
			}
			_DataMsg->hdr.msgId  = CMD_PRINT_DATA;
			_DataMsg->hdr.msgLen = sizeof(SPrintDataMsg)-1+2*len;
			memcpy(&_DataMsg->id, pid, sizeof(SPageId));
			dat_get_buffer(0, len, _DataMsg->data);
			cnt = spool_send_msg(_DataMsg);
			if (cnt==0) return Error(ERR_CONT, 0, "No Spoolers connected");
		}
	}

	return REPLY_OK;
}
