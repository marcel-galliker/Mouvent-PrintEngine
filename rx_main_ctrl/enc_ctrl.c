// ****************************************************************************
//
//	enc_ctrl.c
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#include <stdio.h>
#include "rx_common.h"
#include "rx_def.h"
#include "rx_error.h"
#include "rx_sok.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "args.h"
#include "tcp_ip.h"
#include "plc_ctrl.h"
#include "print_queue.h"
#include "print_ctrl.h"
#include "machine_ctrl.h"
#include "setup.h"
#include "gui_svr.h"
#include "ctrl_msg.h"
#include "ctrl_svr.h"
#include "enc_ctrl.h"
#include "network.h"
#include "label.h"
#include "spool_svr.h"

//--- Defines -----------------------------------------------------------------
	

//--- Externals ---------------------------------------------------------------
SEncoderStat	_EncoderStatus[ENC_CNT];
static SPageId	_ID;

//--- Statics -----------------------------------------------------------------

typedef struct 
{
	int		  used;
	char	  ipAddrStr[32];
	RX_SOCKET socket;
	int		  timeout;
	int		  ready;		
	int		  printGoCnt;
	int		  webOffset_mm;
} _SEncoder;

static _SEncoder _Encoder[ENC_CNT];
static RX_SOCKET _TestSocket[ENC_CNT] = {0,0};

static int		 _EncoderThreadRunning=FALSE;

static int		_IncPerMeter;
static int		_WakeupLen;
static int		_PrintGo_Dist;
static int		_PrintMark_Mode;
static int		_Scanning=FALSE;
static int		_DistTelCnt=0;
static int		_FirstPG;
static UINT32	_PrintingId; 

static int		_TotalPgCnt;
static int		_StopPG;
static int		_Printing=FALSE;
static int		_Khz=0;
static UINT32	_WarnMarkReaderPos;
static INT32	_LH702_printGoMode=0;

typedef struct
{
	SPageId			id;
	int				pages;
} SBufItem;


#define PRINT_BUF_SIZE	128
static SBufItem	_PrintBuf[PRINT_BUF_SIZE];
static int		_PrintBufIn;
static int		_PrintBufOut;

#define CHANGE_FIFO_SIZE 32
static SPrintQueueItem*	_ChangeFifo[CHANGE_FIFO_SIZE];
static int				_ChangeFifoIdx;

//--- Prototypes --------------------------------------------------------------

static void *_enc_thread(void *lpParameter);
static int   _enc_closed(RX_SOCKET socket, const char *peerName);

static int  _handle_enc_msg		(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *par);
static void _handle_status		(int no, SEncoderStat* pstat);
static void _handle_config_reply(int no, SReply* preply);
static void _handle_event		(int no, SLogMsg *msg);
static void _enc_start_printing (int no, SPrintQueueItem *pitem, int restart);

//--- enc_init -------------------------------------------------
void enc_init(void)
{
	int i, n;
	memset(_Encoder, 0, sizeof(_Encoder));
	memset(_EncoderStatus, 0, sizeof(_EncoderStatus));
	memset(&_ID, 0, sizeof(_ID));
	_ChangeFifoIdx = 0;
	for (i=0; i<ENC_CNT; i++)
	{
		net_device_to_ipaddr(dev_enc, i, _Encoder[i].ipAddrStr, SIZEOF(_Encoder[i].ipAddrStr));
		_Encoder[i].socket=INVALID_SOCKET;
		_Encoder[i].used = (i==0);
		
		_EncoderStatus[i].ampl_new	 = INVALID_VALUE;
		_EncoderStatus[i].ampl_old	 = INVALID_VALUE;
		_EncoderStatus[i].percentage = INVALID_VALUE; 
		for (n=0; n<SIZEOF(_EncoderStatus[i].corrRotPar); n++) _EncoderStatus[i].corrRotPar[n]=INVALID_VALUE;
	}
	if(!_EncoderThreadRunning)
	{
		_EncoderThreadRunning = TRUE;
		rx_thread_start(_enc_thread, NULL, 0, "_enc_thread");
	}
}

//--- enc_end -------------------------------------
void enc_end(void)
{
	_EncoderThreadRunning = FALSE;		
}

//--- enc_error_reset -----------------------
void enc_error_reset(void)
{
	int no;
	for (no=0; no<SIZEOF(_Encoder); no++)
	{
		if (_Encoder[no].used && _Encoder[no].socket!=INVALID_SOCKET)	
			sok_send_2(&_Encoder[no].socket, CMD_ERROR_RESET, 0, NULL);
	}	
}

//--- _enc_thread ------------------------------------------------------------
static void *_enc_thread(void *lpParameter)
{
	int errNo;
	int no=0;

	while (_EncoderThreadRunning)
	{
		_Encoder[1].used = (RX_Config.printer.type == printer_DP803) && strcmp(RX_Hostname, "LB701-0001");
		for (no=0; no<ENC_CNT; no++)
		{
			if (_Encoder[no].used && _Encoder[no].socket==INVALID_SOCKET && (net_port_listening(dev_enc, no, PORT_CTRL_ENCODER) || (!_Encoder[1].used && net_port_listening(dev_enc, -1, PORT_CTRL_ENCODER))))
			{			
				errNo=sok_open_client_2(&_Encoder[no].socket, _Encoder[no].ipAddrStr, PORT_CTRL_ENCODER, SOCK_STREAM, _handle_enc_msg, _enc_closed);
				if (errNo)
				{
				//	char str[256];
				//	Error(ERR_CONT, 0, "Socket Error >>%s<<", err_system_error(errNo, str,  sizeof(str)));
				}
				else
				{
					ErrorEx(dev_enc, no, LOG, 0, "Connected");
				}
			}				
		}
		rx_sleep(1000);
	}
	return NULL;
}

//--- _enc_closed --------------------------------------------
static int   _enc_closed(RX_SOCKET socket, const char *peerName)
{
	int i;
	for (i=0; i<SIZEOF(_Encoder); i++)
	{
		if (socket==_Encoder[i].socket)	
		{
			_EncoderStatus[i].info.connected = FALSE;
			sok_close(&_Encoder[i].socket);
			TrPrintf(TRUE, "Encoder[%d].closed", i);
		}
	}

//	Error(WARN, 0, "_enc_closed socket=%d >>%s<<", socket, peerName);
	return REPLY_OK;
}

//--- enc_set_config -----------------------------------
int	 enc_set_config(int restart)
{	
	int no;
	
	if (RX_Config.printer.type==printer_LH702) Error(LOG, 0, "enc_set_config(restart=%d)", restart);
	
	_Scanning = rx_def_is_scanning(RX_Config.printer.type);
		
	if (arg_simuEncoder) Error(WARN, 0, "Encoder in Simulation");

//	_Encoder[1].used = RX_Config.printer.type==printer_DP803;
	switch (RX_Config.printer.type)
	{
	case printer_TX801:			_Encoder[0].webOffset_mm=WEB_OFFSET; break;	// 20
	case printer_TX802:			_Encoder[0].webOffset_mm=WEB_OFFSET; break;	// 20
	case printer_TX404:			_Encoder[0].webOffset_mm=WEB_OFFSET; break; // 20
	case printer_DP803:			_Encoder[0].webOffset_mm=WEB_OFFSET; 
							//	_Encoder[1].webOffset_mm=WEB_OFFSET+WEB_OFFSET_VERSO+RX_Config.printer.offset.versoDist;
								_Encoder[1].webOffset_mm=WEB_OFFSET;
								break;
	case printer_test_slide:	
	case printer_test_slide_only:
								_Encoder[0].webOffset_mm=150; break;
//	case printer_test_table:	if (RX_Config.inkSupplyCnt<=4) _Encoder[0].webOffset_mm=200;	// CLEAF
//								else						   _Encoder[0].webOffset_mm=110;	// Bobst
//								break; 
	case printer_test_table:	_Encoder[0].webOffset_mm=110; break;
	case printer_LB701:
	case printer_LB702_UV:
	case printer_LB703_UV:
	case printer_LH702:
	case printer_cleaf:			_Encoder[0].webOffset_mm=10;  break;
	case printer_LB702_WB:		_Encoder[0].webOffset_mm=10+RX_Config.printer.offset.versoDist;
								break;
	default:					_Encoder[0].webOffset_mm=5;   break;
	}
	_PrintGo_Dist	= 10000;
	_PrintMark_Mode	= FALSE;
	_DistTelCnt		= 0;
	_FirstPG		= TRUE;
	_TotalPgCnt		= 0;
	_WarnMarkReaderPos = 9;
	
	memset(_PrintBuf, 0, sizeof(_PrintBuf));
	_PrintBufIn = _PrintBufOut = 0;
	
	for (no=0; no<ENC_CNT; no++) 
	{
		if (!restart) _Encoder[no].printGoCnt= -1;
		if (_Encoder[no].used) sok_send_2(&_Encoder[no].socket, CMD_ENCODER_PG_INIT, sizeof(restart), &restart);
	}
//	Error(LOG, 0, "CMD_ENCODER_PG_INIT");
	return REPLY_OK;		
}

//--- enc_tick --------------------------------------------
void  enc_tick(void)
{
	int no;
	for (no=0; no<ENC_CNT; no++)
	{
		if (_Encoder[no].used && _Encoder[no].socket!=INVALID_SOCKET) sok_send_2(&_Encoder[no].socket, CMD_ENCODER_STAT, 0, NULL);
		if (_Encoder[no].timeout>0 && !(--_Encoder[no].timeout)) Error(ERR_ABORT, 0, "Encoder Communication Timeout");			
	}
}

//--- enc_start_printing ---------------------------------------------------------------
int  enc_start_printing(SPrintQueueItem *pitem, int restart)
{
	int no;
//	Error(LOG, 0, "enc_start_printing");
	for (no=0; no<ENC_CNT; no++)
	{
		if (_Encoder[no].used) 	_enc_start_printing(no, pitem, restart);		
	}
	_Printing = TRUE;
	_StopPG   = FALSE;
	
	if (arg_simuEncoder)
	{
		for(no=0; no<ENC_CNT; no++) 
		{
			sok_send_2(&_Encoder[no].socket, CMD_FPGA_SIMU_ENCODER, sizeof(_Khz), &_Khz);						
		}
	}
	return REPLY_OK;
}

//--- _enc_start_printing ---------------------------------------------------------------
static void _enc_start_printing(int no, SPrintQueueItem *pitem, int restart)
{
	SEncoderCfg msg;
	double comp, comp2;
	memset(&msg, 0, sizeof(msg));

	if (RX_Config.printer.type==printer_LH702) Error(LOG, 0, "_enc_start_printing");
		
	msg.restart		= restart;
	msg.simulation  = arg_simuEncoder;
	msg.printerType = RX_Config.printer.type;
	msg.encoderType = RX_Config.printer.encoderType;
	// if (arg_testMachine) msg.printerType=printer_LB701;

	if (pitem->printGoMode==PG_MODE_MARK) msg.printGoMode = PG_MODE_MARK_FILTER;
	else                                  msg.printGoMode = pitem->printGoMode;
	msg.printGoDist = pitem->printGoDist;
	msg.correction  = CORR_OFF; 
	msg.ftc			= RX_Config.printer.offset.manualFlightTimeComp;
	
	_WakeupLen = 0;
	switch (msg.printerType)
	{
	case printer_test_table:	msg.orientation = TRUE;	msg.scanning=TRUE;  msg.incPerMeter=1000000; msg.pos_actual = machine_get_scanner_pos(); 
								break;
			
	case printer_test_slide: 
	case printer_test_slide_only:
								msg.orientation = FALSE;	msg.scanning=TRUE;  msg.incPerMeter=1000000; msg.pos_actual = 0;
								break;
			
	case printer_TX801:
	case printer_TX802:			
	case printer_TX404:			msg.orientation = FALSE;	msg.scanning=TRUE;  msg.incPerMeter=1000000; msg.pos_actual = machine_get_scanner_pos(); 
								if (!pitem->testImage) _WakeupLen=WAKEUP_BAR_LEN*(RX_Config.inkSupplyCnt+1);
								if (!arg_simuPLC) 
								{
									if (TRUE) msg.correction=CORR_LINEAR;
									else Error(WARN, 0, "Encoder compensation OFF");
								}
								break;
		
	case printer_LB701:			msg.orientation = FALSE;	msg.scanning=FALSE; msg.incPerMeter=1000000; msg.pos_actual = 0; 
								msg.diameter[0]=79; msg.diameter[1]=74;
								msg.correction=CORR_ROTATIVE;								
								if (FALSE)
								{
									msg.correction=CORR_OFF;
									Error(WARN, 0, "Encoder Correction OFF for tests");
								}
								break;
		
    case printer_LB702_WB:
    case printer_LH702:
    case printer_LB702_UV:
        msg.orientation = FALSE;
        msg.scanning = FALSE;
        msg.incPerMeter = 971253;
        msg.pos_actual = 0;
        msg.correction = CORR_ROTATIVE;
        msg.diameter[0] = 78;
        msg.diameter[1] = 74;
        break;


	// due to fpga the diameter should be between 70 and 85 (real diameter of LB730 are 130 and 125), so we transform 130/125 to 78/75
    case printer_LB703_UV:
        msg.orientation = FALSE;
        msg.scanning = FALSE;
        msg.incPerMeter = 606072;
        msg.pos_actual = 0;
        msg.correction = CORR_ROTATIVE;
        msg.diameter[0] = 75;
        msg.diameter[1] = 78;
        break;

	case printer_DP803:			msg.orientation = FALSE;	msg.scanning=FALSE; msg.incPerMeter=1000000; msg.pos_actual = 0; msg.correction=CORR_ROTATIVE;
								msg.diameter[0] = 74; 
								msg.diameter[1] = 76;
								if (no == 0)
								{
									msg.printGoOutDist = 1000*(WEB_OFFSET_VERSO + RX_Config.printer.offset.versoDist);
								}
								else
								{
									msg.diameter[0] = 78; 
									msg.printGoMode = PG_MODE_MARK;
									msg.printGoDist = 0;
								}
								break;

	case printer_cleaf:			msg.orientation = TRUE;		msg.scanning=FALSE; msg.incPerMeter= 180200; msg.pos_actual = 0; break;
	default:					msg.orientation = TRUE;		msg.scanning=TRUE;  msg.incPerMeter=1000000; msg.pos_actual = 0; break;
	}
		
	if (arg_simuEncoder && msg.correction==CORR_ROTATIVE) msg.correction=CORR_OFF;
	
//	_WakeupLen = 0;
//	if (wakeupLen) Error(LOG, 0, "Set WakeupLen for Lazy jets: %d strokes", wakeupLen);
	
		
	msg.speed_mmin  = pitem->speed;
	_IncPerMeter    = msg.incPerMeter;

	memcpy(msg.corrRotPar, RX_Config.encoder[no].corrRotPar, sizeof(msg.corrRotPar));
	msg.incPerMeter = _IncPerMeter+RX_Config.printer.offset.incPerMeter[no];
//	Error(LOG, 0, "EncoderOffset=%d", RX_Config.printer.offset.incPerMeter[0]);

	if (_Scanning)
	{
		msg.pos_pg_fwd  = _Encoder[no].webOffset_mm*1000 + pitem->pageMargin - (_WakeupLen*25400/1200);
		if (rx_def_is_tx(RX_Config.printer.type))
			msg.pos_pg_bwd  = _Encoder[no].webOffset_mm*1000 + pitem->pageMargin + pitem->srcHeight + 13350 + 5000 + (_WakeupLen*25400/1200);	// add extra 5000 for nagative distance
		else // test table
			msg.pos_pg_bwd  = _Encoder[no].webOffset_mm*1000 + pitem->pageMargin + pitem->srcHeight + 13350 + RX_Config.headDistBackMax;
	}
	else
	{
		msg.pos_pg_fwd  = _Encoder[no].webOffset_mm*1000;
		msg.pos_pg_bwd  = 10000000;			
	}
			
	if (_Scanning && arg_simuEncoder)
	{
		msg.scanning=FALSE;
		msg.pos_actual = 0;			
	}

	switch(pitem->scanMode)
	{
	case PQ_SCAN_RTL:		msg.pos_pg_fwd = 	10000000; 
							break;
			
	case PQ_SCAN_BIDIR:		if (pitem->speed)
							{
								if(RX_Config.printer.offset.manualFlightTimeComp)
								{
									comp=RX_Config.printer.offset.manualFlightTimeComp;
								//	Error(LOG, 0, "Manual Flightime Comp: %dµm", (int)comp);											
								}
								else 
								{
									comp =  1000.0 * 0.0090 * (pitem->speed-60);								// ref done at 60m/min
								//	Error(LOG, 0, "FlightTimeComp: speed=%d (ref 60m/min): comp=%dµm", pitem->speed, (int)comp);
                                    comp2 = 0.0027 * (RX_Config.stepper.print_height-2000) * pitem->speed;		// ref done at 2.0 mm (height above material, don't add material_thickness) 
								//	Error(LOG, 0, "FlightTimeComp: height=%d.%03dmm (ref 2.0mm): comp=%dµm", RX_Config.stepper.print_height/1000, RX_Config.stepper.print_height%1000, (int)comp2);
                                    comp += comp2;
								//	Error(LOG, 0, "FlightTimeComp: comp=%dµm", (int)comp);
								}
								msg.pos_pg_bwd += (int)comp;  
							}
							break;
			
	default:				msg.pos_pg_bwd = 	10000000; break;			
	}
	
	if (arg_simuEncoder)  msg.correction=CORR_OFF;
	_Encoder[no].ready = FALSE;
	if (_Encoder[no].socket!=INVALID_SOCKET) // || enc_connect()==REPLY_OK)	
	{
		_Encoder[no].timeout = 2;
		sok_send_2(&_Encoder[no].socket, CMD_ENCODER_CFG, sizeof(msg), &msg);
	}
//	Error(LOG, 0, "CMD_ENCODER_CFG");
}

//--- enc_sent_document ------------------------
void enc_sent_document(int pages, SPageId *pId)
{
	//--- put to fifo
	memcpy(&_PrintBuf[_PrintBufIn].id, pId, sizeof(SPageId));
	_PrintBuf[_PrintBufIn].pages = pages;
	_PrintBufIn = (_PrintBufIn+1)%PRINT_BUF_SIZE;
	/*
	_TotalPgCnt += pages;
	_FirstPG = TRUE;
	*/
}

//--- enc_change ---------------------------------------------------
int enc_change(void)
{
//	if (RX_Config.printer.type==printer_LH702) Error(LOG, 0, "enc_change idx=%d", _ChangeFifoIdx);
	if (_ChangeFifoIdx>0)
	{
		if (_ChangeFifoIdx<1) return Error(ERR_ABORT, 0, "ChangeFIFO empty");
		enc_set_config(TRUE);
		enc_start_printing(_ChangeFifo[0], FALSE);
		memcpy(&_ID, &_ChangeFifo[0]->id, sizeof(_ID));
		for (int i=0; i<_ChangeFifoIdx; i++)
		{
			enc_set_pg(_ChangeFifo[i], &_ChangeFifo[i]->id);
		}
	}
	_ChangeFifoIdx=0;
	return REPLY_OK;
}

//--- enc_set_pg ----------------------------------------
int	 enc_set_pg(SPrintQueueItem *pitem, SPageId *pId)
{
	int no;
	
	if (!memcmp(pId, &_PrintBuf[_PrintBufOut].id, sizeof(SPageId)))
	{
		_TotalPgCnt += _PrintBuf[_PrintBufOut].pages;
		_FirstPG = TRUE;
		_PrintBufOut = (_PrintBufOut+1)%PRINT_BUF_SIZE;
		TrPrintf(TRUE, "enc_sent_document(%d) _TotalPgCnt=%d", _PrintBuf[_PrintBufOut].pages, _TotalPgCnt);
	}
	
	if (RX_Config.printer.type==printer_LH702)
	{	
		if (_ID.id && pId->id!=_ID.id && pitem->printGoMode!=_LH702_printGoMode)
		{
			if (_ChangeFifoIdx>=CHANGE_FIFO_SIZE) return Error(ERR_ABORT, 0, "Fifo Overflow");
			_ChangeFifo[_ChangeFifoIdx++] = pitem;
		//	Error(LOG, 0, "enc_set_pg (id=%d, page=%d, copy=%d, scan=%d) to ChangeFIFO", pId->id, pId->page, pId->copy, pId->scan);
			return REPLY_OK;
		}
		_LH702_printGoMode = pitem->printGoMode;
	}

	if (pId->scan==0xffffffff) return REPLY_OK; // flush

//	if (RX_Config.printer.type==printer_LH702) Error(LOG, 0, "enc_set_pg (id=%d, page=%d, copy=%d, scan=%d)", pId->id, pId->page, pId->copy, pId->scan);
	
	/*
	//--- put to fifo
	memcpy(&_PrintBuf[_PrintBufIn].id, pId, sizeof(SPageId));
	_PrintBuf[_PrintBufIn].scans = pitem->scans;
	_PrintBufIn = (_PrintBufIn+1)%PRINT_BUF_SIZE;
	*/
	
	if (_Encoder[0].socket==INVALID_SOCKET) return REPLY_OK;
	
	TrPrintfL(TRUE, "enc_set_pg id=%d, page=%d, copy=%d, scan=%d, printGoMode=%d", pId->id, pId->page, pId->copy, pId->scan, pitem->printGoMode);						
	
	if (!_Scanning)
	{
		SEncoderPgDist dist;
		memset(&dist, 0, sizeof(dist));
		if (pId!=NULL) memcpy(&dist.id, pId, sizeof(dist.id));
		dist.cnt	= 1;
		dist.dist	= _PrintGo_Dist;
		dist.printGoMode = pitem->printGoMode;
		
	//	Error(LOG, 0, "enc_set_pg id=%d, page=%d, copy=%d, scan=%d, dist=%d", pId->id, pId->page, pId->copy, pId->scan, _PrintGo_Dist);						

		_PrintMark_Mode = (pitem->printGoMode!=PG_MODE_LENGTH) && (pitem->printGoMode!=PG_MODE_GAP);
		
		if (!_PrintMark_Mode) sok_send_2(&_Encoder[0].socket, CMD_ENCODER_PG_DIST, sizeof(dist), &dist);

		_DistTelCnt++;
		switch(pitem->printGoMode)
		{
		case PG_MODE_LENGTH: _PrintGo_Dist = pitem->printGoDist; break;
		case PG_MODE_GAP:	 _PrintGo_Dist = pitem->pageHeight+pitem->printGoDist;
				 			 // Bug in FPGA: (when srcLineCnt==12300, gap=0 it sometimes prints an additional line of old data [instead of blank] between the labels)
							 if (pitem->printGoDist<22)
							 {
								_PrintGo_Dist = pitem->pageHeight+22;
							 }

							 /*
 							 {
								if (_DistTelCnt%100==1) Error(WARN, 0, "TEST: Cut image");
								_PrintGo_Dist -= (_DistTelCnt%10)*5; 										
							 }
							 */
							 break;
			
		case PG_MODE_MARK:	 
		case PG_MODE_MARK_INV:	 
		case PG_MODE_MARK_VERSO:
		case PG_MODE_MARK_VERSO_INV:	 
		case PG_MODE_MARK_VRT:		
							 dist.dist		   = (pitem->printGoDist>0) ? pitem->printGoDist:0;
							 dist.printGoMode  = PG_MODE_MARK_FILTER;
							 if (!_FirstPG)
							 {
								dist.ignore   = pitem->pageHeight*8/10;
								if (RX_Config.printer.type==printer_LH702 && str_start(RX_Hostname, "LH702"))
									dist.window   = pitem->pageHeight/4;	// check only on real machine!
							 }
							 sok_send_2(&_Encoder[0].socket, CMD_ENCODER_PG_DIST, sizeof(dist), &dist);
							 _FirstPG = FALSE;
							 break;
			
		default:			if (pId->id != _ID.id) Error(WARN, 0, "PrintGo-Mode not defined");	
							_PrintGo_Dist = pitem->pageHeight; 
							break;				
		}
		TrPrintfL(TRUE, "enc_set_pg (no=%d, dist=%d, ignore=%d, window=%d)", _DistTelCnt, dist.dist, dist.ignore, dist.window);						
		//-- DP803: encoder[1]: Always in Mark Reading Mode -----
		if (_Encoder[1].used)
		{
			dist.printGoMode = PG_MODE_MARK;
			dist.dist		 = 0;
			if (_DistTelCnt>1) dist.ignore   = pitem->pageHeight*9/10;
			if (_DistTelCnt>1) dist.window   = pitem->pageHeight/4;			
			sok_send_2(&_Encoder[1].socket, CMD_ENCODER_PG_DIST, sizeof(dist), &dist);
		}
	}
	else if (_Scanning && arg_simuEncoder)
	{ 
		SEncoderPgDist dist;
		memset(&dist, 0, sizeof(dist));
		dist.cnt	= 1;
		dist.dist	= pitem->pageHeight+2*(_WakeupLen*25400/1200)+100000; // simulate time for direction change!
		for(no=0; no<ENC_CNT; no++) 
		{
			sok_send_2(&_Encoder[no].socket, CMD_ENCODER_PG_DIST, sizeof(dist), &dist);
		}

		/*
		//--- bug in simulation? ---
		if (_TotalPgCnt && pId->scan==_TotalPgCnt)
		{
			for(no=0; no<ENC_CNT; no++) 
			{
				sok_send_2(&_Encoder[no].socket, CMD_ENCODER_PG_DIST, sizeof(dist), &dist);
			}			
		}
		*/
		
//		Error(LOG, 0, "CMD_ENCODER_PG_DIST: scan=%d", pId->scan);
	}
	memcpy(&_ID, pId, sizeof(_ID));
	return REPLY_OK;					
}

//--- enc_change_printGoDist --------------------------------------
int	enc_change_printGoDist(int printGoDist)
{
	Error(LOG, 0, "enc_change_printGoDist(%d)", printGoDist);
	sok_send_2(&_Encoder[0].socket, CMD_ENCODER_PG_STOP, 0, NULL);
	return REPLY_OK;
}

//--- enc_stop_pg ------------------------------
int  enc_stop_pg(char *reason)
{
	if (!_Scanning)
	{
	//	Error(LOG, 0, "enc_stop_pg(%s)", reason);
		_StopPG = TRUE;
		sok_send_2(&_Encoder[0].socket, CMD_ENCODER_PG_STOP, 0, NULL);			
	}
	return REPLY_OK;
}

//--- enc_restart_pg ------------------------------
int  enc_restart_pg(void)
{
	TrPrintfL(TRUE, "enc_restart_pg");
	sok_send_2(&_Encoder[0].socket, CMD_ENCODER_PG_RESTART, 0, NULL);
	return REPLY_OK;	
}

//--- enc_pg_stop_cnt ----------------------------------
int	 enc_pg_stop_cnt(void)
{
	return _EncoderStatus[0].PG_stop;
}

//--- enc_stop_printing ------------------------------
int  enc_stop_printing(void)
{
	int no;
	_Printing = FALSE;
	for(no=0; no<ENC_CNT; no++) sok_send_2(&_Encoder[no].socket, CMD_STOP_PRINTING, 0, NULL);
	return REPLY_OK;
}

//--- enc_abort_printing ------------------------------
int  enc_abort_printing(void)
{
	int no;
	_Printing = FALSE;
	for(no=0; no<ENC_CNT; no++) sok_send_2(&_Encoder[no].socket, CMD_ABORT_PRINTING, 0, NULL);
	memset(&_ID, 0, sizeof(_ID));
	_LH702_printGoMode = 0;
	return REPLY_OK;
}

//--- enc_enable_printing ----------------------------
int	 enc_enable_printing(int enable)
{
	static int _enable=FALSE;
//	if (enable!=_enable) 
//	Error(LOG, 0, "enc_enable_printing(%d)", enable);
	_enable = enable;

	int no;
	for(no=0; no<ENC_CNT; no++)
	{
		if (enable) sok_send_2(&_Encoder[no].socket, CMD_ENCODER_ENABLE, 0, NULL);
		else        sok_send_2(&_Encoder[no].socket, CMD_ENCODER_DISABLE, 0, NULL);		
	}
	return REPLY_OK;				
}

//--- enc_uv_on --------------------------------------
int  enc_uv_on(void)
{
	int no;
	for(no=0; no<ENC_CNT; no++) sok_send_2(&_Encoder[no].socket, CMD_ENCODER_UV_ON, 0, NULL);
	return REPLY_OK;
}

//--- enc_uv_off ------------------------------------
int  enc_uv_off(void)
{
	int no;
	for(no=0; no<ENC_CNT; no++) sok_send_2(&_Encoder[no].socket, CMD_ENCODER_UV_OFF, 0, NULL);
	return REPLY_OK;
}

int  enc_is_uv_on(void)    { return _EncoderStatus[0].info.uv_on; }
int  enc_is_uv_ready(void) { return _EncoderStatus[0].info.uv_ready;}
int  enc_is_analog(void)   { return _EncoderStatus[0].info.analog_encoder;}	
int  enc_is_connected(void)   { return _EncoderStatus[0].info.connected;}	

//--- _handle_enc_msg ------------------------------------------------------------------
static int _handle_enc_msg(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *par)
{
	SMsgHdr	*phdr = (SMsgHdr*)msg;
	int no;
	if (len)
	{
		for (no=0; no<ENC_CNT; no++)
		{
			if (socket==_Encoder[no].socket)
			{
				net_register_by_device(dev_enc, no);

			//	TrPrintfL(TRUE, "received Encoder[%d].MsgId=0x%08x", no, phdr->msgId);

				switch(phdr->msgId)
				{
				case REP_ENCODER_CFG:	_handle_config_reply(no, (SReply*)msg);				break;	
				case REP_ENCODER_STAT:  _handle_status		(no, (SEncoderStat*)&phdr[1]);	break;
				case EVT_GET_EVT:		_handle_event		(no, (SLogMsg*)	 msg);			break;
				default: Error(WARN, 0, "Encoder[%d]:Got unknown MessageId=0x%08x", no, phdr->msgId);
				}
				break;				
			}
		}
	}
	return REPLY_OK;
}

//--- _handle_status ------------------------------------
static void _handle_status(int no, SEncoderStat* pstat)
{
	SEncoderInfo info=_EncoderStatus[no].info;


	//--- test ------------------
	if (!_Scanning && !arg_simuEncoder && !_StopPG)
	{
		if (_DistTelCnt && _TotalPgCnt && pstat->PG_cnt!=_TotalPgCnt)
		{
		//	if (RX_Config.printer.type == printer_DP803) Error(LOG, 0, "_DistTelCnt=%d, PG_cnt=%d, _TotalPgCnt=%d", _DistTelCnt, pstat->PG_cnt, _TotalPgCnt);
			if (pstat->fifoEmpty_PG >_EncoderStatus[no].fifoEmpty_PG)  ErrorEx(dev_enc, no, ERR_STOP, 0, "PrintGo.dist FIFO empty (PG=%d, err=%d) (Images: transferred=%d, printGo=%d, buffer=%d)", pstat->PG_cnt, pstat->fifoEmpty_PG, RX_PrinterStatus.transferredCnt, RX_PrinterStatus.printGoCnt, RX_PrinterStatus.transferredCnt-RX_PrinterStatus.printGoCnt);
			if (pstat->fifoEmpty_IGN>_EncoderStatus[no].fifoEmpty_IGN) ErrorEx(dev_enc, no, ERR_STOP, 0, "PrintGo.ignore FIFO empty (PG=%d, err==%d)", pstat->PG_cnt, pstat->fifoEmpty_IGN);
			if (pstat->fifoEmpty_WND>_EncoderStatus[no].fifoEmpty_WND) ErrorEx(dev_enc, no, ERR_STOP, 0, "PrintGo.window FIFO empty (PG=%d, err==%d)", pstat->PG_cnt, pstat->fifoEmpty_WND);		
		}
	//	if (pstat->PG_cnt>_EncoderStatus[no].PG_cnt) ErrorEx(dev_enc, no, LOG, 0, "PrintGo %d/%d", pstat->PG_cnt, _TotalPgCnt);			
	}
	
	if (!_EncoderStatus[no].info.connected) 
	{
		ctrl_set_max_speed();
	}
	memcpy(&_EncoderStatus[no], pstat, sizeof(_EncoderStatus[no]));
	_EncoderStatus[no].info.connected = TRUE;
	if (no==0 &&_Encoder[no].printGoCnt==-1)
	{
		TrPrintfL(TRUE, "*** _Encoder.printGoCnt=-1: PG_cnt=%d ***", _EncoderStatus[no].PG_cnt);
	}
	if (_Encoder[no].printGoCnt==-1 && _EncoderStatus[no].PG_cnt==0) 
	{
		TrPrintfL(TRUE, "Reset PrintGoCnt");
		_Encoder[no].printGoCnt = 0;
	}
	if (_Encoder[no].printGoCnt>=0  && _EncoderStatus[no].PG_cnt != _Encoder[no].printGoCnt)	
	{
		_Encoder[no].printGoCnt = _EncoderStatus[no].PG_cnt;

		TrPrintfL(TRUE, "");

		while ((UINT32)_Encoder[no].printGoCnt > RX_PrinterStatus.printGoCnt)
		{
			pc_print_go();
			RX_PrinterStatus.printGoCnt++;
		}
	}
	if (_PrintMark_Mode && _Encoder[no].printGoCnt==0 && _EncoderStatus[no].meters>_WarnMarkReaderPos)
	{
		Error(WARN, 0, "PrintGo triggered by PrintMark but NO MARK DETECTED after %d meters", _EncoderStatus[no].meters);
		_WarnMarkReaderPos = _EncoderStatus[no].meters;
	}
	if (no==0 && _EncoderStatus[no].info.analog_encoder != info.analog_encoder) ctrl_set_max_speed();
}

//--- enc_reply_stat ---------------------------------------------------------------
void enc_reply_stat(RX_SOCKET socket)
{
	gui_send_msg_2(socket, REP_ENCODER_STAT, sizeof(_EncoderStatus), _EncoderStatus);
}

//--- enc_save_par ---------------------------------------------------
void enc_save_par(int no)
{
	memcpy(RX_Config.encoder[no].corrRotPar, _EncoderStatus[no].corrRotPar, sizeof(RX_Config.encoder[no].corrRotPar));
	memcpy(RX_Config.encoder[no].corrRotPar, _EncoderStatus[no].corrRotPar, sizeof(RX_Config.encoder[no].corrRotPar));
	setup_save_config();
}

//--- _handle_config_reply --------------------------------
static void _handle_config_reply(int no, SReply* preply)
{
	if (preply->reply) 
		Error(WARN, 0, "Encoder Configuration msgLen=%d reply=%d", preply->hdr.msgLen, preply->reply);
	_Encoder[no].ready = TRUE;
	_Encoder[no].timeout = 0;
}

//--- _handle_event -----------------------------
static void _handle_event(int no, SLogMsg *msg)
{
	SlaveError(dev_enc, no, &msg->log);
	if (msg->log.logType==LOG_TYPE_ERROR_ABORT) pc_abort_printing();
}

//--- enc_ready ----------------------------------------
int  enc_ready(void)
{
	int no;
	for (no=0; no<ENC_CNT; no++) if (_Encoder[no].used && (!_Encoder[no].ready || _Encoder[no].socket==INVALID_SOCKET)) return FALSE;
	return TRUE;
}

//--- enc_speed ----------------------------------
int  enc_speed(void)
{
	return _EncoderStatus[0].speed;
}

//--- enc_simu_encoder ----------------------
#define LH702_SPEED 20	

int	 enc_simu_encoder(int mmin)
{	
	int no;
	int khz_head;
	
	if (mmin && RX_Config.printer.type==printer_LH702) mmin=LH702_SPEED;
	
	khz_head = (int)(mmin/60.0/25.4*1200.0);
//	if (_Printing || !_Khz)
	if (_Khz != khz_head)
	{
		Error(WARN, 0, "CMD_FPGA_SIMU_ENCODER %d m/min %d kHz", mmin, khz_head);
		for(no=0; no<ENC_CNT; no++) sok_send_2(&_Encoder[no].socket, CMD_FPGA_SIMU_ENCODER, sizeof(khz_head), &khz_head);						
	}
	_Khz = khz_head;
	return REPLY_OK;
}
