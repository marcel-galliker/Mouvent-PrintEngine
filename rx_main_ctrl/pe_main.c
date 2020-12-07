// ****************************************************************************
//
//	pe_main.c		
//
// ****************************************************************************
//
//	Copyright 2018 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

//--- includes ----------------------------------------------------------------

#include "rx_common.h"
#include "rx_error.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "rx_pecore_lib.h"
#include "tcp_ip.h"
#include "network.h"
#include "enc_ctrl.h"
#include "print_ctrl.h"
#include "machine_ctrl.h"
#include "pe_main.h"

//--- prototypes --------------------------------------------------------------
static int _printsynchro(Smvt_prt_synchro *cmd);
static int _printstart(void);
static int _printstop(INT32 pageIdx);
static int _pageready(INT32 pageIdx, INT32 copies);
static int _printcmd(Smvt_prt_cmd *cmd);
static int _loadfile(Smvt_prt_loadfile *cmd, int pageIdx);
static int _printadjust(Smvt_prt_adjust *cmd);
static int _init_pq_item(SPrintQueueItem *pitem, struct mvt_prt_sync *settings);

//--- global variables --------------------------------------------------------
static SpeMainAPI			*_Api=NULL;
static SpeMainConfig		_ActCfg;
static SpeMainPara			_ActPar;
static int					_Printing;

static SPrintQueueItem _Item;


static Smvt_prt_synchro	_Synchro;

//--- _init_pq_item ---------------------------------------------------------
static int _init_pq_item(SPrintQueueItem *pitem, struct mvt_prt_sync *settings)
{
	pitem->printGoMode = settings->printGoMode;
	pitem->printGoDist = (settings->printGoMode==PG_MODE_GAP) ? settings->printGoGap : settings->printGoDistance;
	pitem->speed       = 10;
//	pitem->pageWidth   = 100000;
	pitem->pageHeight  = 500000;

	if (RX_Config.externalData)
	{
	//	Error(WARN, 0, "Need to know SPEED");
	//	Error(WARN, 0, "Need to pageWidth");
	//	Error(WARN, 0, "Need to pageHeight");			
	}
	
	return REPLY_OK;
}

//--- pem_init ----------------------------------------------------------------
int pem_init(void)
{	 
	memset(&_ActCfg, 0, sizeof(_ActCfg));
	memset(&_ActPar, 0, sizeof(_ActPar));
	memset(&_Synchro, 0, sizeof(_Synchro));
	memset(&_Item, 0, sizeof(_Item));
	_Synchro.rectoSync.printGoMode = PG_MODE_GAP;
	_Synchro.rectoSync.printGoGap  = 10000;
	_init_pq_item(&_Item, &_Synchro.rectoSync);
	return REPLY_OK;
}

//--- pem_set_config --------------------------------------
int pem_set_config(void)
{	
	SpeMainConfig cfg;
	SpeMainPara par;

	memcpy (&par, &_ActPar, sizeof(par));
	memcpy (&cfg, &_ActCfg,	sizeof(cfg));

	{
		SIfConfig config;
		sok_get_ifconfig("em1", &config); 		
		strncpy(cfg.id, config.hostname, sizeof(cfg.id));	
	}

	net_device_to_ipaddr(dev_main, 0, cfg.ip, sizeof(cfg.ip));
	
	cfg.port			= PORT_PE_MAIN;
	cfg.resoHead		= DPI_X;
	cfg.resoPrint		= DPI_X;
	cfg.maxWidth		= RX_Spooler.barWidthPx;
	cfg.maxHeight		= 10000;
	cfg.maxSpeed		= 1000;
	cfg.duplex			= FALSE;
	cfg.dropSizeCount	= 3;
	cfg.bitPerPixel		= 2;
	cfg.alignment		= 0; // pConfig->alignment;
	cfg.speedPassCount	= 1;

	cfg.colorCount		= RX_Spooler.colorCnt;
	cfg.bitmapCount		= RX_Spooler.colorCnt;
	cfg.interfCount		= 1; // RX_Spooler.colorCnt;

	{
		int size = cfg.colorCount*sizeof(SpeMainColor);
		SpeMainColor *colors = (SpeMainColor*)rx_malloc(size);
		for (int i = 0; i < cfg.colorCount; i++) {
			colors[i].index = i;
			colors[i].colorCode = RX_Color[i].color.colorCode;
		}
		if(_ActCfg.colorCount != cfg.colorCount || memcmp(_ActCfg.colors, colors, size))
		{
			ReleasePtr(_ActCfg.colors);
			cfg.colors = colors;
		}
		else ReleasePtr(colors);
	}
	
	{
		int size=cfg.bitmapCount*sizeof(SpeMainBitmap);
		SpeMainBitmap* bitmaps = (SpeMainBitmap*)rx_malloc(size);
		for (int i = 0; i < cfg.bitmapCount; i++) {
			bitmaps[i].index		= i;
			bitmaps[i].interf		= RX_Color[i].spoolerNo;
			bitmaps[i].color		= i;
			bitmaps[i].side		= 0;
			bitmaps[i].offset		= RX_Color[i].firstLine;
			bitmaps[i].width		= RX_Color[i].lastLine-RX_Color[i].firstLine+1;
			bitmaps[i].speedPass	= 0;
		}
		if(_ActCfg.bitmapCount != cfg.bitmapCount || memcmp(_ActCfg.bitmaps, bitmaps, size))
		{
			ReleasePtr(_ActCfg.bitmaps);
			cfg.bitmaps = bitmaps;
		}
		else ReleasePtr(bitmaps);		
	}
	
	{
		int size=cfg.interfCount*sizeof(SpeMainInterface);
		SpeMainInterface* interfaces = (SpeMainInterface*)rx_malloc(size);
		for (int i = 0; i < cfg.interfCount; i++)
		{
			interfaces[i].index		= i;
			interfaces[i].connected	= TRUE;
			net_device_to_ipaddr(dev_spooler, i, interfaces[i].ip, sizeof(interfaces[i].ip));
			interfaces[i].port		= PORT_PE_SPOOL;
		}
		if(_ActCfg.interfCount != cfg.interfCount || memcmp(_ActCfg.interfaces, interfaces, size))
		{
			ReleasePtr(_ActCfg.interfaces);
			cfg.interfaces = interfaces;
		}
		else ReleasePtr(interfaces);		
	}

	// Init CallBacks for master
	par.callbackPrintStart   = _printstart;
	par.callbackPrintStop    = _printstop;
	par.callbackPageReady	 = _pageready;
	par.callbackPrintCmd	 = _printcmd;
	par.callbackPrintSynchro = _printsynchro;
	par.callbackPrintAdjust	 = _printadjust;
	par.callbackLoadFile	 = _loadfile;
//	par.callbackDrawCmd		 = null;
//	par.callbackLoadlayout	 = null;
//	par.callbackRecordData	 = null;

	par.config				 = &_ActCfg;
	
	if (memcmp(&par, &_ActPar, sizeof(_ActPar)) || memcmp(&cfg, &_ActCfg, sizeof(_ActCfg)) )
	{
		rx_pemain_stop();
		while (rx_pemain_state () != PE_CLOSED) rx_sleep (100);
				
		memcpy(&_ActCfg, &cfg, sizeof(_ActCfg));
		memcpy(&_ActPar, &par, sizeof(_ActPar));

		TrPrintfL(TRUE, "PEM: rx_pemain_start");
		rx_pemain_start(&_ActPar, &_Api);

		pem_send_printer_status();
	}
	return REPLY_OK;
}

//--- pem_end -----------------------------------------------------------------
int pem_end(void)
{
	TrPrintfL(TRUE, "PEM: rx_pemain_stop");
	rx_pemain_stop();
	ReleasePtr(_ActCfg.colors);
	ReleasePtr(_ActCfg.bitmaps);
	ReleasePtr(_ActCfg.interfaces);
	memset(&_ActCfg, 0, sizeof(_ActCfg));
	memset(&_ActPar, 0, sizeof(_ActPar));
	
	return REPLY_OK;
}

//--- pem_send_printer_status -------------------------
int pem_send_printer_status(void)
{
	if (_Api) 
	{
		_Printing	=	RX_PrinterStatus.printState==ps_printing 
					||  RX_PrinterStatus.printState==ps_stopping 
					||	RX_PrinterStatus.printState==ps_pause;
		_Api->PrintState(_Printing);			
	}
	return REPLY_OK;
}

//--- _printsynchro -----------------------------------------------------------
static int _printsynchro(Smvt_prt_synchro *cmd)
{
	memcpy(&_Synchro, cmd, sizeof(_Synchro));

	TrPrintfL(TRUE, "PEM: _printsynchro Recto: PrintGo Mode=%d, dist=%d, gap=%d", cmd->rectoSync.printGoMode, cmd->rectoSync.printGoDistance, cmd->rectoSync.printGoGap);
	TrPrintfL(TRUE, "PEM: _printsynchro Verso: PrintGo Mode=%d, dist=%d, gap=%d", cmd->versoSync.printGoMode, cmd->versoSync.printGoDistance, cmd->versoSync.printGoGap);

	_init_pq_item(&_Item, &_Synchro.rectoSync);
	return REPLY_OK;
}

//--- _printadjust ----------------------------------------
static int _printadjust(Smvt_prt_adjust *cmd)
{
	TrPrintfL(TRUE, "Print Adjust: recto:(%d, %d) verso:(%d, %d)", cmd->RectoX, cmd->RectoY, cmd->VersoX, cmd->VersoY);
	return REPLY_OK;
}

//--- _printstart -------------------------------------------------------------
static int _printstart(void)
{
	Error(LOG, 0, "PEM: _printstart");
	if (_Item.pageWidth==0) _Item.pageWidth=1; // trigger to start encoder
	machine_set_printpar(&_Item);
	pc_start_printing();
	return REPLY_OK;
}

//--- _printstop --------------------------------------------------------------
static int _printstop(INT32 pageIdx)
{
	Error(LOG, 0, "PEM: _printstop (page=%d)", pageIdx);
	if (pageIdx==0) pc_abort_printing();
	else pc_stop_printing(FALSE);
	return REPLY_OK;
}

//--- _pageready --------------------------------------------------------------
static int _pageready(INT32 pageIdx, INT32 copies)
{
	int count;
	SpeMainPgeJob *job;
	if (!_Printing) return Error(ERR_CONT, 0, "Received MVT_PRT_REQ_CMDPG while printstate=OFF");
	Error(LOG, 0, "PEM: Page %d is ready, %d copies", pageIdx, copies);
	Error(WARN, 0, "Need to pageHeight");
	_Api->GetPage(pageIdx);
	_Api->GetPgeJob(&count, &job);
	_Item.id.id		= 0;
	_Item.id.page	= pageIdx;
	_Item.id.copy	= 1;
	enc_set_pg(&_Item, &_Item.id);
	return REPLY_OK;
}

//--- _printcmd ------------------------------------------------
static int _printcmd(Smvt_prt_cmd *cmd)
{
	if (!_Printing) return Error(ERR_CONT, 0, "Received MVT_PRT_REQ_CMD while printstate=OFF");

	switch (cmd->id) {
	case	PRINTCMD_PAPERSTART:	Error(LOG, 0, "PEM: CMD PAPER-START");
									machine_start_printing();
									break;
		
	case	PRINTCMD_PAPERSTOP:		Error(LOG, 0, "PEM: CMD PAPER-STOP");
									machine_pause_printing(TRUE);
									break;
		
	case	PRINTCMD_PAPERFEED:		Error(LOG, 0, "PEM: CMD PAPER-FEED");  break;
	default: return REPLY_ERROR;
	}
	return REPLY_OK;
}

//--- _loadfile ---------------------------------------------------------------
static int _loadfile(Smvt_prt_loadfile *cmd, int pageIdx)
{
	if (!_Printing) return Error(ERR_CONT, 0, "Received MVT_PRT_REQ_FILECMD while printstate=OFF");

	Error(LOG, 0, "PEM: _loadfile >>%s<< pages: %d..%d, copies=%d", cmd->filepath, cmd->pageStart, cmd->pageEnd, cmd->copyCount);
	return REPLY_OK;
}

//--- pem_print_file_evt ------------------------------------------------------
int pem_print_file_evt	(RX_SOCKET socket, int headNo, SPrintFileMsg	*msg)
{
	TrPrintfL(TRUE, "Documment ID=%d  page=%d , copy=%d, scan=%d: EVENT %d",	msg->id.id, msg->id.page, msg->id.copy, msg->id.scan, msg->evt);
	
	memcpy(&_Item.id, &msg->id, sizeof(_Item.id));

	switch (msg->evt)
	{
	case DATA_RIPPING:	TrPrintf(TRUE, "SPOOLER %d: Documment ID=%d  page=%d: RIPPING,   copy=%d, scan=%d, bufReady=%d",	msg->spoolerNo, msg->id.id, msg->id.page, msg->id.copy, msg->id.scan, msg->bufReady); break;
	case DATA_SCREENING:TrPrintf(TRUE, "SPOOLER %d: Documment ID=%d  page=%d: SCREENING, copy=%d, scan=%d, bufReady=%d",	msg->spoolerNo, msg->id.id, msg->id.page, msg->id.copy, msg->id.scan, msg->bufReady); break;
	case DATA_LOADING:	TrPrintfL(TRUE, "SPOOLER %d: Documment ID=%d  page=%d: LOADING,   copy=%d, scan=%d, bufReady=%d >>%s<<",	msg->spoolerNo, msg->id.id, msg->id.page, msg->id.copy, msg->id.scan, msg->bufReady, msg->txt);
					//	Error(LOG, 0, "Loading (id=%d, page=%d, copy=%d) >>%s<<", _Item.id.id, _Item.id.page, _Item.id.copy, msg->txt);
						// pq_loading(msg->spoolerNo, &msg->id, msg->txt);
						break;

	case DATA_SENDING:	TrPrintf(TRUE, "SPOOLER %d: Documment ID=%d  page=%d: SENDING,   copy=%d, scan=%d, bufReady=%d",	msg->spoolerNo, msg->id.id, msg->id.page, msg->id.copy, msg->id.scan, msg->bufReady); 
						Error(LOG, 0, "PEM: Sending (id=%d, page=%d, copy=%d)", _Item.id.id, _Item.id.page, _Item.id.copy);
						// pq_sending(msg->spoolerNo, &msg->id); 
						break;

	case DATA_SENT:		TrPrintf(TRUE, "PEM: SPOOLER %d: Documment ID=%d  page=%d: SENT,      copy=%d, scan=%d, bufReady=%d",		msg->spoolerNo, msg->id.id, msg->id.page, msg->id.copy, msg->id.scan, msg->bufReady);
						Error(LOG, 0, "Sent (id=%d, page=%d, copy=%d)", _Item.id.id, _Item.id.page, _Item.id.copy);
						break;

	/*		
	case fv:
						Error(LOG, 0, "Head[%d]: PrintDone (id=%d, page=%d, copy=%d)", headNo, _Item.id.id, _Item.id.page, _Item.id.copy);
						break;	
	*/
		
	default:			TrPrintf(TRUE, "Documment ID=%d  page=%d , copy=%d, scan=%d: UNKNOWN EVENT %d",	msg->spoolerNo, msg->id.id, msg->id.page, msg->id.copy, msg->id.scan, msg->evt); break;
	}
	return REPLY_OK;
}
