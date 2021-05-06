// ****************************************************************************
//
//	ctrl_svr.cpp
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
#include "rx_setup_file.h"
#include "rx_sok.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "tcp_ip.h"
#include "print_queue.h"
#include "network.h"
#include "setup.h"
#include "args.h"
#include "ctr.h"
#include "enc_ctrl.h"
#include "chiller.h"
#include "fluid_ctrl.h"
#include "gui_svr.h"
#include "step_ctrl.h"
#include "spool_svr.h"
#include "ctrl_msg.h"
#include "ctrl_svr.h"

//--- Defines -----------------------------------------------------------------
	

//--- Externals ---------------------------------------------------------------
extern SInkSupplyStat   _FluidStatus[INK_SUPPLY_CNT];

//--- Modlue Globals -----------------------------------------------------------------
typedef struct
{
	//--- config ---------------
	SHeadBoardCfg	*cfg;

	//--- thread parameters ----
	int			running;
	HANDLE		threadHdl;
	INT32		threadId;

	//--- socket -----------------
	EDevice		device;
	int			no;			// head strl number
	RX_SOCKET	socket;
	int			aliveTime;
	
	msg_handler	handle_msg;
} SHeadCtrlPar;

HANDLE	_HeadCtrlSvr;

SHeadCtrlPar _HeadCtrl[HEAD_BOARD_CNT];
static int	_HeadResetCnt;
static int	 _SingleHead;
static int	 _AwaitHeadCfg;


//--- Prototypes ------------------------------------------------------
static void* _head_ctrl_thread(void* lpParameter);
static int  _prepare_config();

static void _headboard_config(int colorCnt, int headsPerColor, int ethPortCnt);	// SAMBA head, one color per headboard

static void _ping_test(int cnt);
static void _send_head_cfg(int headNo);
static void _send_ink_def(int headNo, char *drops, int screenOnPrinter);

//static void Data_Test_TCP();
//static void Data_Test_UDP();

//--- ctrl_start ----------------------------------------------------------------
int	ctrl_start(void)
{
	int i;
	_AwaitHeadCfg = 0;
	_HeadResetCnt = 0;
	memset(_HeadCtrl, 0, sizeof(_HeadCtrl));
	for (i=0; i<SIZEOF(_HeadCtrl); i++) _HeadCtrl[i].socket = INVALID_SOCKET;
	
	ctrl_update_hostname();
	_prepare_config();
//	_ping_test(3);
//	Data_Test_TCP();
//	Data_Test_UDP();
	
//	CTRL_reset();
//	ctrl_set_config();
	step_set_config();
	fluid_set_config();
	enc_set_config(FALSE);
	
	return REPLY_OK;
}

//--- ctrl_update_hostname --------------------------
void ctrl_update_hostname(void)
{
	memset(RX_Hostname, 0, sizeof(RX_Hostname));
#ifdef linux
	{
		int i;
		FILE *file;
		file = fopen("/etc/hostname", "rt");
		if (file!=NULL)
		{
			fgets(RX_Hostname, sizeof(RX_Hostname), file);
			for(i=strlen(RX_Hostname)-1; i>=0 && RX_Hostname[i]<' '; i--)  
				RX_Hostname[i]=0x00;
			fclose(file);
		}
	}
#else
	gethostname(RX_Hostname, sizeof(RX_Hostname));
#endif
}

//--- ctrl_is_connected --------------------------------------------------------------
int  ctrl_is_connected(EDevice device, int no)
{
	if (device==dev_head && no>=0 && no<SIZEOF(_HeadCtrl)) return _HeadCtrl[no].socket!=INVALID_SOCKET;
	return FALSE;
}

//--- _prepare_config -----------------------------------------------------------------
static int _prepare_config()
{
	int color;
	int n;
	int overlap;
	int jets;
	int head;
	int ethPortCnt;
	int isNo;
	int maxTemp;
	int chillerTemp;
	SHeadBoardCfg	*pBoard;
	
	if (RX_Config.simulation) Error(WARN, 0, "Config Simulation");

	memset(&RX_Spooler, 0, sizeof(RX_Spooler));
	RX_Spooler.printerType = RX_Config.printer.type;
	RX_Spooler.overlap	   = rx_def_is_scanning(RX_Config.printer.type) && RX_Config.printer.overlap;
	
	//--- ethernet ports on additional interface board -------------------
#ifdef linux
	if (RX_Config.printer.type==printer_cleaf)			 ethPortCnt=4;
	else if (RX_Config.printer.type==printer_test_table) ethPortCnt=0;
	else if (arg_hamster)								 ethPortCnt=0;
	else 
	{
		ethPortCnt=sok_get_ifcnt("^p[0-9]+p[0-9]+$");
		if (ethPortCnt==0) Error(WARN, 0, "No PxPx Ports"); 
	}

#else
	ethPortCnt=0;
#endif
	
	if(ethPortCnt) RX_Spooler.dataBlkSize = DATA_BLOCK_SIZE_JUMBO;
	else           RX_Spooler.dataBlkSize = DATA_BLOCK_SIZE_STD;		

	if (arg_simuPLC)     Error(WARN, 0, "PLC in Simulation");
	if (arg_simuEncoder) Error(WARN, 0, "Encoder in Simulation");
	
	RX_Spooler.dataBlkCntHead	= (1024*1024*1024/RX_Spooler.dataBlkSize) & ~0x7f; // limit for easy testing UDP-BlockFlags
	RX_Spooler.dataBlkCntHead	/= MAX_HEADS_BOARD;
	if (RX_Spooler.dataBlkSize == DATA_BLOCK_SIZE_STD)   RX_Spooler.dataBlkCntHead = 0x20000;
	
	RX_Spooler.headWidthPx		= HEAD_WIDTH_SAMBA;
	RX_Spooler.headOverlapPx	= HEAD_OVERLAP_SAMBA;
	RX_Spooler.barStartPx		= HEAD_OVERLAP_SAMBA;
	RX_Spooler.headsPerBoard	= MAX_HEADS_BOARD;
	RX_Spooler.barEndPx			= RX_Config.headsPerColor*RX_Spooler.headWidthPx;
	RX_Spooler.colorCnt			= RX_Config.colorCnt;
	RX_Spooler.headsPerColor	= RX_Config.headsPerColor;
	RX_Spooler.barWidthPx		= RX_Spooler.headWidthPx*RX_Spooler.headsPerColor - RX_Spooler.headOverlapPx;

	if (RX_Config.headsPerColor==0)
	{
		Error(ERR_ABORT, 0, "Configuration: Heads Per Color=0");
		return REPLY_ERROR;
	}
	if (RX_Config.headsPerColor>MAX_HEADS_COLOR)
	{
		Error(ERR_ABORT, 0, "Configuration: Heads Per Color=%d (max=%d)", RX_Config.headsPerColor, MAX_HEADS_COLOR);
		return REPLY_ERROR;
	}
	
	//--- alignment printheads and boards to colors --------------
	_headboard_config(RX_Config.colorCnt, RX_Config.headsPerColor, ethPortCnt);

	memset(RX_Color, 0, sizeof(RX_Color));
	maxTemp		= 0;
	chillerTemp = 1000;
	for (n=0; n<SIZEOF(RX_Config.headBoard); n++) RX_Config.headBoard[n].present=dev__undef;
	for (color=0, head=0; color<RX_Config.colorCnt; color++)
	{
		if(RX_Config.inkSupply[color].ink.fileName[0] == 0)
		{
			memset(&RX_Color[color], 0, sizeof(RX_Color[color]));
			RX_ColorNameInit(color, RX_Color[color].rectoVerso, RX_Color[color].color.fileName, RX_Color[color].color.colorCode);
			for (n=0; n<RX_Config.headsPerColor; n++, head++)
			{
				pBoard = &RX_Config.headBoard[head / MAX_HEADS_BOARD];
				pBoard->head[head % MAX_HEADS_BOARD].enabled   = dev_off;
				pBoard->head[head % MAX_HEADS_BOARD].inkSupply = color;
				pBoard->head[head % MAX_HEADS_BOARD].encoderNo = 0;
			}
		}
		else
		{
			if (RX_Config.inkSupply[color].ink.temp > maxTemp) maxTemp = RX_Config.inkSupply[color].ink.temp;
			if (RX_Config.inkSupply[color].ink.tempChiller && RX_Config.inkSupply[color].ink.tempChiller < chillerTemp) chillerTemp = RX_Config.inkSupply[color].ink.tempChiller;
			RX_Color[color].no = color;
			RX_Color[color].inkSupplyNo = color;
			memcpy(&RX_Color[color].color, &RX_Config.inkSupply[color].ink, sizeof(RX_Color[color].color));
			RX_Color[color].firstLine = 0;
			RX_Color[color].lastLine  = 1000000;
			if (RX_Config.printer.type == printer_DP803)
				if (!strcmp(RX_Hostname, "TEST-0001"))
				{
					Error(WARN, 0, "TEST: all on first spooler");
					RX_Color[color].spoolerNo = 0;			
				}
				else RX_Color[color].spoolerNo = color;	// one pc per color	
			else
				RX_Color[color].spoolerNo = 0;	
			{
				//--- split one bitmap over 4 heads ----

				//---  geometry Fuji printhead -------
				//		bmp(px) nozzle
				//				0___________
				//		0_______128_________|\
				//							| |
				//							| |
				//				2048________| |
 				//		2048____2048+128_____\|	
				//					
				overlap = 128;
				jets    = 2048;
				for (n = 0; n < RX_Config.headsPerColor; n++, head++)
				{
					pBoard = &RX_Config.headBoard[head / MAX_HEADS_BOARD];
					pBoard->present										= dev_on;
					pBoard->printerType									= RX_Config.printer.type;
					pBoard->reverseHeadOrder							= rx_def_is_tx(RX_Config.printer.type);
					pBoard->head[head % MAX_HEADS_BOARD].enabled   		= dev_on;
					pBoard->head[head % MAX_HEADS_BOARD].inkSupply 		= isNo = (color*RX_Config.inkCylindersPerColor)+(n*RX_Config.inkCylindersPerColor)/RX_Config.headsPerColor;
					pBoard->head[head % MAX_HEADS_BOARD].encoderNo 		= 0;
					pBoard->spoolerNo									= RX_Color[color].spoolerNo;		
					if(rx_def_is_scanning(RX_Config.printer.type))
					{
						pBoard->head[head % MAX_HEADS_BOARD].jetEnabled0   	= 0;
						pBoard->head[head % MAX_HEADS_BOARD].jetEnabledCnt 	= jets;							
						if (n==0 && !RX_Config.printer.overlap) pBoard->head[head % MAX_HEADS_BOARD].jetEnabled0 = overlap;
						if(n + 1 < RX_Config.headsPerColor || RX_Config.printer.overlap) 
							pBoard->head[head % MAX_HEADS_BOARD].jetEnabledCnt += overlap;
					}
					else
					{
						if (n==0) pBoard->head[head % MAX_HEADS_BOARD].jetEnabled0 = overlap;
						if(n + 1 < RX_Config.headsPerColor) pBoard->head[head % MAX_HEADS_BOARD].jetEnabledCnt = jets + overlap;
						else                                pBoard->head[head % MAX_HEADS_BOARD].jetEnabledCnt = jets;
					}
										
					if (RX_Config.printer.type == printer_DP803) RX_Color[color].rectoVerso = RX_Config.inkSupply[color].rectoVerso;
					else  RX_Color[color].rectoVerso=rv_undef;
					RX_ColorNameInit(isNo, RX_Color[color].rectoVerso, RX_Color[color].color.fileName, RX_Color[color].color.colorCode);
					if(RX_Config.headsPerColor > 1)	
					{
						if(pBoard->reverseHeadOrder)
							sprintf(pBoard->head[head % MAX_HEADS_BOARD].name, "%s-%d", RX_ColorNameShort(isNo), n + 1);
						else
							sprintf(pBoard->head[head % MAX_HEADS_BOARD].name, "%s-%d", RX_ColorNameShort(isNo), n + 1);
					}
					else sprintf(pBoard->head[head % MAX_HEADS_BOARD].name, "%s",    RX_ColorNameShort(isNo));
					
					if(RX_Config.printer.type == printer_TX801)
					{
						if(!RX_Config.printer.overlap)
						{
							pBoard->head[head % MAX_HEADS_BOARD].jetEnabled0   = overlap;
							pBoard->head[head % MAX_HEADS_BOARD].jetEnabledCnt = jets - overlap;							
						}								
					}
					
					if (RX_PrinterStatus.testMode)
					{
						pBoard->head[head % MAX_HEADS_BOARD].jetEnabled0   = 0;
						pBoard->head[head % MAX_HEADS_BOARD].jetEnabledCnt = jets+overlap;								
					}
					
					if(RX_Config.printer.type==printer_cleaf)	RX_Spooler.headNo[color][RX_Config.headsPerColor-1-n] = 1 + head;		// reverse order			
					else										RX_Spooler.headNo[color][n] = 1 + head;
					RX_Color[color].split[n].firstPx  = n*jets;
					RX_Color[color].split[n].widthPx  = jets;
					
					if (rx_def_is_scanning(RX_Config.printer.type))  RX_Color[color].offsetPx =                         (RX_Config.colorOffset[color] * 1200) / 25400;
					else                                             RX_Color[color].offsetPx = RX_Spooler.barWidthPx - (RX_Config.colorOffset[color] * 1200) / 25400;
					if (RX_Color[color].offsetPx > RX_Spooler.maxOffsetPx)
						RX_Spooler.maxOffsetPx = RX_Color[color].offsetPx;

//					RX_Color[color].split[n].headNo	  = head+1;
//					RX_Color[color].split[n].block0	  = (head%MAX_HEADS_BOARD)*RX_Spooler.dataBlkCnt;
//					RX_Color[color].split[n].blockCnt = RX_Spooler.dataBlkCnt;
					if (n > 0)									RX_Color[color].split[n].firstPx -= overlap;
					if (n > 0 && n < RX_Config.headsPerColor)	RX_Color[color].split[n].widthPx += overlap;
				} 
			}
		}
	}
	
	//--- Virtual encoders for textile printers ----------------------------------------------
	// TX801: CORR_LINEAR: Board[0].Head[0] nearest to Encoder[1]
	if (!arg_simuPLC)
	{
		if(rx_def_is_tx(RX_Config.printer.type) && RX_Config.printer.type!=printer_TX404 && RX_Config.headsPerColor)
		{
			for (head=0; head<RX_Config.colorCnt * RX_Config.headsPerColor; head++)
			{
				RX_Config.headBoard[head/MAX_HEADS_BOARD].head[head%MAX_HEADS_BOARD].encoderNo = 7-(head/RX_Config.headsPerColor);
			}
		}
	}
	
	chiller_set_temp(chillerTemp, maxTemp);

	ctrl_set_max_speed();
	
	//--- start head client processes -------------------------------------------
	int hb, no;
	char name[64];
	for (hb=0; hb < SIZEOF(RX_Config.headBoard); hb++)
	{
		if (RX_Config.headBoard[hb].ctrlAddr)
		{
			no = RX_Config.headBoard[hb].no;
			if (!_HeadCtrl[no].running) 
			{
				_HeadCtrl[no].cfg		= &RX_Config.headBoard[hb];
				_HeadCtrl[no].running	= TRUE;
				_HeadCtrl[no].device	= dev_head;
				_HeadCtrl[no].no		= no;
				_HeadCtrl[no].handle_msg= handle_headCtrl_msg;
				sprintf(name, "_head_ctrl_thread[%d]", no);
				_HeadCtrl[no].threadHdl	= rx_thread_start(_head_ctrl_thread, &_HeadCtrl[no], 0, name);
			}
		}
	}

	return REPLY_OK;
}

//--- ctrl_set_max_speed -----------------------------------------
void ctrl_set_max_speed(void)
{
	//--- calculate max speed --------------------------------------
	int color, n;
	INT32 inkSpeed[MAX_DROP_SIZES];
	int maxSpeed[4];

	for (n=0; n<MAX_DROP_SIZES; n++) inkSpeed[n] = 1000;
	for (color=0; color<RX_Config.colorCnt; color++)
	{
		if (*RX_Config.inkSupply[color*RX_Config.inkCylindersPerColor].ink.name)
		{
			char str[MAX_PATH];
			int len=0;
			for (n=0; n<MAX_DROP_SIZES; n++)
			{
				len += sprintf(&str[len], " %d", RX_Config.inkSupply[color].ink.maxSpeed[n]);
				if (RX_Config.inkSupply[color].ink.maxSpeed[n] && RX_Config.inkSupply[color].ink.maxSpeed[n] < inkSpeed[n]) inkSpeed[n] = RX_Config.inkSupply[color].ink.maxSpeed[n];
			}
			TrPrintfL(TRUE, "Waveform[%s].maxSpeed:%s m/s", RX_Config.inkSupply[color].ink.name, str);
		}
	}

	switch(RX_Config.printer.type)
	{
		case printer_TX801:		
		case printer_TX802:		
		case printer_TX404:		if (enc_is_analog())
								{
									maxSpeed[0]= 100; maxSpeed[1]= 100; maxSpeed[2]= 100; maxSpeed[3]= 100;				
								}
								else 
								{
									maxSpeed[0]=  60; maxSpeed[1]=  60; maxSpeed[2]=  60; maxSpeed[3]=  40;
									if (enc_is_connected()) Error(WARN, 0, "No Communication to analog encoder board! Limited speed!");
								}
								break;
		case printer_LB701:		maxSpeed[0]= 100; maxSpeed[1]= 100; maxSpeed[2]= 100; maxSpeed[3]= 100; break;
		case printer_LB702_UV:	maxSpeed[0]= 120; maxSpeed[1]= 120; maxSpeed[2]= 120; maxSpeed[3]= 120; break;
		case printer_LB702_WB:	maxSpeed[0]= 100; maxSpeed[1]= 100; maxSpeed[2]= 100; maxSpeed[3]= 100; break;
		case printer_LH702:		maxSpeed[0]= 100; maxSpeed[1]= 100; maxSpeed[2]= 100; maxSpeed[3]= 100; break;
		case printer_DP803:		maxSpeed[0]= 120; maxSpeed[1]= 120; maxSpeed[2]= 120; maxSpeed[3]= 120; break;
		default:				maxSpeed[0]= 100; maxSpeed[1]= 100; maxSpeed[2]= 100; maxSpeed[3]= 100; break; 	
	}

	if (!strcmp(RX_Hostname, "DropWatcher"))
	{
		maxSpeed[0]= 120; maxSpeed[1]= 120; maxSpeed[2]= 120; maxSpeed[3]= 120;		
	}
	
	for (n=0; n<MAX_DROP_SIZES; n++)
	{
		if(inkSpeed[n]) RX_PrinterStatus.maxSpeed[n] = (int) (0.95*inkSpeed[n]); // for safety
		else			RX_PrinterStatus.maxSpeed[n] = 30;
		if ((int)RX_PrinterStatus.maxSpeed[n]>maxSpeed[n]) RX_PrinterStatus.maxSpeed[n]=maxSpeed[n];				
	}
	
	TrPrintfL(TRUE, "ctrl_set_max_speed speed=%d %d %d %d, hostname=>>%s<<, analogenc=%d", 
		RX_PrinterStatus.maxSpeed[0], 
		RX_PrinterStatus.maxSpeed[1], 
		RX_PrinterStatus.maxSpeed[2],
		RX_PrinterStatus.maxSpeed[3], 
		RX_Hostname, enc_is_analog());
	
	gui_send_printer_status(&RX_PrinterStatus);
}

//--- _headboard_config -----------------------------------------------
static void _headboard_config(int colorCnt, int headsPerColor, int ethPortCnt)
{
	// ethcnt = number of ethernet connestions on spooler
	//--- scanning: All Heads on one PrintBar ----------------------
	int i;
	int board, head, port;
	int udpPortCnt;
	char ipAddr[32];
	SHeadBoardCfg	*pBoard;

	if (headsPerColor==0) headsPerColor=1;
	
	//--- config the boards TCP/IP Interfaces ------------
	pBoard=RX_Config.headBoard;
	RX_Config.headDistMax=0;
	RX_Config.headDistBackMax=0;
	for (board=0, head=0; head<colorCnt*headsPerColor; board++, pBoard++, head+=MAX_HEADS_BOARD)
	{
		if (head>=SIZEOF(RX_Config.headBoard)*MAX_HEADS_BOARD) Error(ERR_ABORT, 0, "Memory Overflow");
//		pBoard->present		= dev_on; // only if one of the heads is active 
		pBoard->no			= board;
#ifdef DEBUG
		pBoard->debug		= TRUE;
#else
		pBoard->debug		= FALSE;
#endif
			pBoard->simuPlc		= arg_simuPLC;
		net_device_to_ipaddr(dev_head, pBoard->no, ipAddr, sizeof(ipAddr));
	//	TrPrintfL(TRUE, "ctrl interface >>%s<<", ipAddr);
		pBoard->ctrlAddr	 = sok_addr_32(ipAddr);
		pBoard->ctrlPort     = PORT_CTRL_HEAD;			
		if (head%headsPerColor==0) port=200;
		if (rx_def_is_tx(RX_Config.printer.type) || ethPortCnt<2)udpPortCnt=1;
		else                                     udpPortCnt=2;
		for (i=0; i<SIZEOF(pBoard->dataAddr); i++)
		{
			port++;
			pBoard->dataPort[i]  = PORT_UDP_DATA;
			if(RX_Config.printer.type == printer_DP803)
			{
				char str[32];
				UCHAR no=(pBoard->ctrlAddr>>24)&0xff;
		//		if (i==0) Error(WARN, 0, "Check IP-Address assignment");
				if (ethPortCnt) sprintf(str, "%d.%d.%d.%d", 192, 168, port, no);
				pBoard->dataAddr[i]  = sok_addr_32(str);
			}
			else 
			{
				pBoard->dataAddr[i]  = net_head_data_addr(pBoard->no, i, ethPortCnt, udpPortCnt);
			}
		}

		if(RX_Config.printer.type != printer_DP803)
		{
			//--- allow missing heads for "unused" color or in test mode
			{
				int n;
				int used=FALSE;
				for (n=0; n<MAX_HEADS_BOARD; n++) used |= (pBoard->head[n].enabled!=dev_off);
				if (!used) pBoard->ctrlAddr = 0;
			}
			if (pBoard->ctrlAddr && RX_PrinterStatus.testMode && !net_port_listening(dev_head, pBoard->no, PORT_CTRL_HEAD))
			{
				ErrorEx(dev_head, head/4, WARN, 0, "Head not available for test print!");
				pBoard->ctrlAddr = 0;	
			}
			if (pBoard->ctrlAddr==0)
			{							
				pBoard->dataAddr[0]  = 0;
				pBoard->dataAddr[1]  = 0;			
			}			
		}		
		
		pBoard->dataBlkSize		= RX_Spooler.dataBlkSize;
		pBoard->dataBlkCntHead  = RX_Spooler.dataBlkCntHead;

		for (i=0; i<MAX_HEADS_BOARD; i++)
		{
			pBoard->head[i].blkNo0    = i*RX_Spooler.dataBlkCntHead;
			pBoard->head[i].blkCnt    = RX_Spooler.dataBlkCntHead;
			pBoard->head[i].dist	  = RX_Config.headDist[board*MAX_HEADS_BOARD+i];			
			if (rx_def_is_lb(RX_Config.printer.type))		pBoard->head[i].distBack = pBoard->head[i].dist;
			else if (rx_def_is_tx(RX_Config.printer.type))	pBoard->head[i].distBack = 5000+RX_Config.headDistBack[board*MAX_HEADS_BOARD+i]-pBoard->head[i].dist;
			else pBoard->head[i].distBack = RX_Config.headDistBack[board*MAX_HEADS_BOARD+i];
			
			TrPrintfL(TRUE, "Head[%d]: dist=%06d  distback=%06d", board*MAX_HEADS_BOARD+i, pBoard->head[i].dist, pBoard->head[i].distBack);
			
			pBoard->head[i].headHeight= RX_Config.stepper.print_height;
			memcpy(&pBoard->head[i].cond, &RX_Config.cond[board*MAX_HEADS_BOARD+i], sizeof(SConditionerCfg));
			pBoard->head[i].cond.headsPerColor = RX_Config.headsPerColor;
			if (RX_PrinterStatus.testMode)
			{
				int offset;
				int inkSupply = pBoard->head[i].inkSupply;
				if (RX_Config.printer.type==printer_DP803) inkSupply=inkSupply%(RX_Config.inkSupplyCnt/2);	// recto/verso
				if (rx_def_is_lb(RX_Config.printer.type))
				{
                    offset = 0;
					if (RX_TestImage.testImage==PQ_TEST_ENCODER)			offset = 265000*inkSupply;
					if (RX_TestImage.testImage==PQ_TEST_ANGLE_SEPARATED)	offset =  50000*inkSupply;
					pBoard->head[i].dist	 += offset;	// recto
					pBoard->head[i].distBack += offset;	// verso				
				}
				else if (rx_def_is_tx(RX_Config.printer.type) && RX_TestImage.testImage==PQ_TEST_DENSITY && RX_TestImage.copies>1)
				{
					pBoard->head[i].dist	 += 150000*(inkSupply%2);	
					pBoard->head[i].distBack += 150000*(inkSupply%2);				
				}
				else if (RX_TestImage.testImage==PQ_TEST_ANGLE_SEPARATED)
				{
					pBoard->head[i].dist	 += 50000*inkSupply;
					pBoard->head[i].distBack -= 50000*inkSupply;
				}
			}
			if (pBoard->head[i].dist>RX_Config.headDistMax)			RX_Config.headDistMax = pBoard->head[i].dist;
			if (pBoard->head[i].distBack>RX_Config.headDistBackMax) RX_Config.headDistBackMax = pBoard->head[i].distBack;
		}
	}
}

//--- ctrl_tick -----------------------------------------------
void ctrl_tick(void)
{
	int i, head, inkSupply;
	int time = rx_get_ticks();
	SFluidStateLight stat[MAX_HEADS_BOARD];

#define TIMEOUT 3000
	for (i=0; i<SIZEOF(_HeadCtrl); i++)
	{
		if (_HeadCtrl[i].running)
		{
			if (_HeadCtrl[i].aliveTime && _HeadCtrl[i].aliveTime + TIMEOUT < time && _HeadCtrl[i].socket != INVALID_SOCKET)	
			{
				#ifdef DEBUG
					// Error(WARN, 0, "Head[%d]: Status Response TIMEOUT: time=%d", i, time - _HeadCtrl[i].aliveTime);
				#else
					ErrorEx(dev_head, i, ERR_ABORT, 0, "Status Response TIMEOUT: time=%d", time - _HeadCtrl[i].aliveTime);
					sok_close(&_HeadCtrl[i].socket);
				#endif
			}
			else
			{
				if (_HeadCtrl[i].aliveTime && _HeadCtrl[i].aliveTime+TIMEOUT-1000<time && _HeadCtrl[i].socket!=INVALID_SOCKET)
				{
					ErrorEx(dev_head, i, WARN, 0, "Status Response Late: time=%d", time-_HeadCtrl[i].aliveTime);					
				}		
		
				for (head = 0; head < MAX_HEADS_BOARD; head++)
				{
					inkSupply = RX_Config.headBoard[i].head[head].inkSupply;
					stat[head].cylinderPressure    = fluid_get_cylinderPres(inkSupply);
					stat[head].cylinderPressureSet = fluid_get_cylinderPresSet(inkSupply);
					stat[head].fluidErr            = fluid_get_error(inkSupply);
					stat[head].machineMeters	   = (UINT32)RX_PrinterStatus.counterTotal;
				}
				sok_send_2(&_HeadCtrl[i].socket, CMD_HEAD_STAT, sizeof(stat), stat);
			} //if (_HeadCtrl[i].aliveTime && ..
		} // if (_HeadCtrl[i].running)
	} // for
}

//--- _send_ink_def ------------------------------------------------
static void _send_ink_def(int headNo, char *dots, int screenOnPrinter)
{
	int n, no;
	int inksupply=-1;
	SInkDefMsg	msg;
	
	if (_HeadCtrl[headNo].running)
	{
		for (n=0; n<SIZEOF(_HeadCtrl[headNo].cfg->head); n++)
		{
			if (_HeadCtrl[headNo].cfg->head[n].enabled==dev_on)
			{
				inksupply = _HeadCtrl[headNo].cfg->head[n].inkSupply;
				msg.hdr.msgId = SET_GET_INK_DEF;
				msg.hdr.msgLen = sizeof(msg);
				msg.headNo = n;
			//	msg.maxDropSize = max;
				memcpy(msg.dots, dots, sizeof(msg.dots));
				
				no = headNo*HEAD_CNT+n;
				if (screenOnPrinter && RX_HBStatus[headNo].head[n].eeprom_density.voltage)	msg.fpVoltage = RX_HBStatus[headNo].head[n].eeprom_density.voltage;
				else if (RX_Config.headFpVoltage[no]) msg.fpVoltage = RX_Config.headFpVoltage[no];
				else msg.fpVoltage = RX_HBStatus[headNo].head[n].eeprom.voltage;
				
				/*
                Error(LOG, 0, "Head %s: FirepulseVoltage=%d%% (mvt=%d, fuji=%d)",
					RX_Config.headBoard[headNo].head[n].name,
					msg.fpVoltage,
					screenOnPrinter,
					RX_Config.headFpVoltage[no],
					RX_HBStatus[headNo].head[n].eeprom_density.voltage, 
					RX_HBStatus[headNo].head[n].eeprom.voltage);
				*/

				if(_HeadCtrl[headNo].cfg->reverseHeadOrder) no = RX_Config.colorCnt*RX_Config.headsPerColor-1-no;
				memcpy(&msg.ink, &RX_Config.inkSupply[inksupply].ink, sizeof(msg.ink));
				TrPrintfL(TRUE, "Board[%d].head[%d].ink = >>%s<<", headNo, n, RX_Config.inkSupply[inksupply].ink.description);
				sok_check_addr_32(&_HeadCtrl[headNo].socket, _HeadCtrl[headNo].cfg->ctrlAddr, __FILE__, __LINE__);
				sok_send(&_HeadCtrl[headNo].socket, &msg);
			}
		}
	}
}

//--- _send_head_cfg ----------------------------------------
static void _send_head_cfg(int headNo)
{
	int n;
	int inksupply=-1;
	
//	if (arg_simuHeads) spool_start_sending(_HeadResetCnt);

	if (_HeadCtrl[headNo].running && _HeadCtrl[headNo].socket!=INVALID_SOCKET)
	{			
		_HeadCtrl[headNo].cfg->resetCnt = _HeadResetCnt;
		sok_send_2(&_HeadCtrl[headNo].socket, CMD_HEAD_BOARD_CFG, sizeof(SHeadBoardCfg), _HeadCtrl[headNo].cfg);
		_AwaitHeadCfg |= (1<<headNo);

		//--- send head control mode ----------------------------
		if (RX_Config.printer.type!=printer_test_slide_only)
		{
			for (n=0; n<SIZEOF(_HeadCtrl[headNo].cfg->head); n++)
			{
				if (_HeadCtrl[headNo].cfg->head[n].enabled==dev_on && RX_HBStatus[headNo].head[n].ctrlMode==ctrl_undef)
				{
					inksupply = _HeadCtrl[headNo].cfg->head[n].inkSupply;
					ctrl_send_head_fluidCtrlMode(headNo*MAX_HEADS_BOARD+n, fluid_get_ctrlMode(inksupply), FALSE, FALSE);
				}
			}
		}
	}
}

//--- ctrl_head_cfg_done ------------------------------------
int ctrl_head_cfg_done(int headNo, UINT32 resetCnt)
{
	if (_AwaitHeadCfg && resetCnt==_HeadResetCnt)
	{
		_AwaitHeadCfg &= ~(1<<headNo);
		if (!_AwaitHeadCfg) spool_start_sending(_HeadResetCnt);
	}
	return REPLY_OK;
}

//--- ctrl_headResetCnt --------------------------------
UINT32 ctrl_headResetCnt(void)
{
	return _HeadResetCnt;				
}


//--- ctrl_singleHead -------------------------------
int  ctrl_singleHead(void)
{
	return _SingleHead;
}

//--- ctrl_send_head_fluidCtrlMode --------------------------------------------------------------
void ctrl_send_head_fluidCtrlMode(int headNo, EnFluidCtrlMode ctrlMode, int sendToFluid, int fromGui)
{
	int mode=RX_HBStatus[headNo/HEAD_CNT].head[headNo%HEAD_CNT].ctrlMode;
	if (mode==INVALID_VALUE || (mode==ctrl_off && ctrlMode==ctrl_off)) return;
	if (fromGui) 
	{
		if (ctrlMode>=ctrl_purge_soft && ctrlMode<ctrl_purge_step1)
			fluid_send_ctrlMode(RX_Config.headBoard[headNo/HEAD_CNT].head[headNo%HEAD_CNT].inkSupply, ctrl_off, TRUE);
		_SingleHead = headNo;
        if (ctrlMode == ctrl_off) fluid_purgeCluster(headNo / 4, FALSE);
    }
	if (ctrlMode<=ctrl_print) _SingleHead=-1;
	if (_HeadCtrl[headNo/HEAD_CNT].socket!=INVALID_SOCKET)
	{
		SFluidCtrlCmd cmd;
		cmd.hdr.msgId	= CMD_HEAD_FLUID_CTRL_MODE;
		cmd.hdr.msgLen	= sizeof(cmd);
		cmd.no			= headNo%HEAD_CNT;
		cmd.ctrlMode	= ctrlMode;
		sok_send(&_HeadCtrl[headNo/HEAD_CNT].socket, &cmd);

		if (sendToFluid) fluid_send_ctrlMode(RX_Config.headBoard[headNo/HEAD_CNT].head[headNo%HEAD_CNT].inkSupply, ctrlMode, FALSE);
	}
}

//--- ctrl_send_all_heads_fluidCtrlMode ------------------------------------
void ctrl_send_all_heads_fluidCtrlMode(int fluidNo, EnFluidCtrlMode ctrlMode)
{
	int head;
	SHeadCfg *pcfg;
	for (head=0; head<SIZEOF(RX_Config.headBoard)*MAX_HEADS_BOARD; head++)
	{
		pcfg = &RX_Config.headBoard[head/MAX_HEADS_BOARD].head[head%MAX_HEADS_BOARD];
		if (pcfg->enabled && pcfg->inkSupply==fluidNo)
		{
			ctrl_send_head_fluidCtrlMode(head, ctrlMode, FALSE, FALSE);
		}
	}
}

//--- ctrl_send_purge_par ----------------------------------------------
int ctrl_send_purge_par(int fluidNo, int time)
{
#define HEAD_WIDTH	43000
	int head;
	int delay;
	int timeTotal;
	SPurgePar par;
	SHeadCfg *pcfg;
	
	if (RX_Config.stepper.wipe_speed) delay =  HEAD_WIDTH / RX_Config.stepper.wipe_speed;
	else delay=5000;

	if (time==0 || _SingleHead != -1) delay=0;	

	timeTotal = 0;
	par.delay = 0;
	par.time  = time;
	for (head=0; head<SIZEOF(RX_Config.headBoard)*MAX_HEADS_BOARD; head++)
	{
		pcfg = &RX_Config.headBoard[head/MAX_HEADS_BOARD].head[head%MAX_HEADS_BOARD];
		if (pcfg->enabled && pcfg->inkSupply==fluidNo)
		{
			par.no = head%HEAD_CNT;
			sok_send_2(&_HeadCtrl[head/HEAD_CNT].socket, CMD_SET_PURGE_PAR, sizeof(par), &par);
			if (par.time+par.delay>timeTotal) timeTotal = par.time+par.delay;
			par.delay+=delay;
		}
	}
	return timeTotal;
}

//--- ctrl_check_all_heads_in_fluidCtrlMode ------------
int  ctrl_check_all_heads_in_fluidCtrlMode(int fluidNo, EnFluidCtrlMode ctrlMode)
{
	int head, mode;
	SHeadCfg *pcfg;
	for (head=0; head<SIZEOF(RX_Config.headBoard)*MAX_HEADS_BOARD; head++)
	{
		pcfg = &RX_Config.headBoard[head/MAX_HEADS_BOARD].head[head%MAX_HEADS_BOARD];
		if (pcfg->enabled && pcfg->inkSupply==fluidNo)
		{
			mode = RX_HBStatus[head/MAX_HEADS_BOARD].head[head%MAX_HEADS_BOARD].ctrlMode;
			if ((_SingleHead<0)  && (mode!=ctrlMode)) return FALSE;
			if ((_SingleHead>=0) && (mode==ctrlMode)) return TRUE;
		}
	}
	return TRUE;
}

//--- ctrl_set_config -------------------------------------------------------------------
int ctrl_set_config(void)
{
	int cnt;

	TrPrintf(TRUE, "ctrl_set_config");

	_prepare_config();

	TrPrintf(TRUE, "Send config to Heads");
//	Error(LOG, 0, "Send config to Heads");
	
	ctrl_send_head_cfg();
	cnt=spool_set_config(INVALID_SOCKET, _HeadResetCnt);
	TrPrintfL(TRUE, "Send Config to %d Spoolers",  cnt);
	if (cnt==0) 
	{
		if (TRUE || RX_Config.simulation) Error(WARN, 0, "No Spooler running");
		else                              Error(ERR_CONT, 0, "No Spooler running");
	}

	fluid_set_config();
	step_set_config();
		
	return REPLY_OK;
}

//--- ctrl_head_cal_done ---------------------
void ctrl_head_cal_done(int inkSupply)
{
/*		
	int head;
	SHeadCfg *pcfg;

	for (head=0; head<SIZEOF(RX_Config.headBoard)*MAX_HEADS_BOARD; head++)
	{
		pcfg = &RX_Config.headBoard[head/MAX_HEADS_BOARD].head[head%MAX_HEADS_BOARD];
		if (pcfg->enabled && pcfg->inkSupply==inkSupply)
		{
			TrPrintfL(TRUE, "pid_offset[%d]=%d", head, RX_HBStatus[head/MAX_HEADS_BOARD].head[head%MAX_HEADS_BOARD].pid_offset);
			RX_Config.cond[head].pid_offset = RX_HBStatus[head/MAX_HEADS_BOARD].head[head%MAX_HEADS_BOARD].pid_offset;	
		}
	}*/		
}

//--- ctrl_send_firepulses -----------------------------------------
void ctrl_send_firepulses(char *dots, int screenOnPrinter)
{
	int i;
//	Error(LOG, 0, "ctrl_send_firepulses >>%s<<", dots);
	for (i=0; i<SIZEOF(_HeadCtrl); i++) _send_ink_def(i, dots, screenOnPrinter);
}

//--- ctrl_send_head_cfg -------------------------
void ctrl_send_head_cfg(void)
{
	int i;
	_HeadResetCnt++;
    ctr_reset_jobLen();
//	Error(LOG, 0, "ctrl_send_head_cfg(%d)", _HeadResetCnt);
	for (i=0; i<SIZEOF(_HeadCtrl); i++) _send_head_cfg(i);
}

//--- ctrl_set_density_values ------------------------------------------
void ctrl_set_density(SDensityMsg *pmsg)
{
	Error(LOG, 0, "ctrl_set_density head=%d", pmsg->head);
	sok_send(&_HeadCtrl[pmsg->head/MAX_HEADS_BOARD].socket, pmsg);
}

//--- ctrl_abort_printing ------------------------------------------------------
int	 ctrl_abort_printing(void)
{
	int i;
	for (i=0; i<SIZEOF(_HeadCtrl); i++)
	{
		if (_HeadCtrl[i].running) sok_send_2(&_HeadCtrl[i].socket, CMD_PRINT_ABORT, 0, NULL);
	}
	spool_abort_printing();
	return REPLY_OK;
}

//--- ctrl_get_device --------------------------------------------------------------
int  ctrl_get_device(RX_SOCKET socket, EDevice *device, int *no)
{
	int i;
	for (i=0; i < SIZEOF(_HeadCtrl); i++)
	{
		if (socket == _HeadCtrl[i].socket)
		{
			*device = _HeadCtrl[i].device;
			*no		= _HeadCtrl[i].no;
			return REPLY_OK;
		}
	}
	*device = dev_undef;
	*no = 0;
	return REPLY_ERROR;
}

//--- _head_ctrl_thread ------------------------------------------------------------
static void* _head_ctrl_thread(void* lpParameter)
{
	SHeadCtrlPar *ppar = (SHeadCtrlPar*)lpParameter;

	// SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);

	//	if (Trace) Error(LOG, 0, L"Server started at >>%s:%d<<", ipAddr, TCP_IP_PORT);

	TrPrintf(1, "_head_ctrl_thread START");
	char str[32];
	char myaddr[32];
	int ret=0;
	while (ppar->running)
	{
		if (net_port_listening(dev_head, ppar->cfg->no, ppar->cfg->ctrlPort))
		{
			sok_addr_str(ppar->cfg->ctrlAddr, str);
			ret=sok_open_client(&ppar->socket, str, ppar->cfg->ctrlPort, SOCK_STREAM);
			if(ret == REPLY_OK)
			{
				TrPrintfL(1,"_HeadCtrl[%d]: Connected: myport=>>%s<<",ppar->no,sok_get_socket_name(ppar->socket,myaddr,NULL,NULL));
				ErrorEx(dev_head,ppar->no,LOG,0,"Connected");
				ppar->aliveTime = 0;
				net_send_item(dev_head,ppar->no);
				_send_head_cfg(ppar->no);
				_send_ink_def(ppar->no,"SML", FALSE);
				sok_receiver(NULL,&ppar->socket,handle_headCtrl_msg,&ppar->no);
				net_send_item(dev_head,ppar->no);
				ErrorEx(dev_head,ppar->no,ERR_ABORT,0,"TCP/IP Connection lost");
				TrPrintfL(1,"_HeadCtrl[%d]: Disconnected: myport=>>%s<<",ppar->no,myaddr);
			}
			/*
			else 
			{
				char errstr[256];
				err_system_error(ret, errstr, sizeof(errstr));
				printf(">>%s<< %s\n", str, errstr);
			}
			*/
			
		}
		rx_sleep(1000);
	}

	TrPrintf(1, "_head_ctrl_thread END");
	return 0;
}

//--- ctrl_head_alive ----------------------------------------------
void ctrl_head_alive(int headNo)
{
	_HeadCtrl[headNo].aliveTime = rx_get_ticks();
}

//--- ctrl_ping_head -------------------------------------------------
int ctrl_ping_head(int no)
{
	if (no < 0)
	{
		for (no = 0; no < SIZEOF(_HeadCtrl); no++)
			ctrl_ping_head(no);
	}
	else
	{
		if (_HeadCtrl[no].running)
		{
			sok_send_2(&_HeadCtrl[no].socket, CMD_PING, 0, NULL);
			TrPrintf(1, "PING Head[%d]", no);
		}
	}
	return REPLY_OK;
}

//--- ctrl_head_error_reset -------------------------------------------------
int ctrl_head_error_reset(void)
{
	int i;
	for (i=0; i<SIZEOF(_HeadCtrl); i++)
	{
		if (_HeadCtrl[i].running)	sok_send_2(&_HeadCtrl[i].socket, CMD_ERROR_RESET, 0, NULL);
	}
	return REPLY_OK;
}

//--- ctrl_print_page --------------------------------------------------
int ctrl_print_page(SPageId *id)
{
	return REPLY_OK;

	int i;
	for (i=0; i<SIZEOF(_HeadCtrl); i++)
	{
		if (_HeadCtrl[i].running)
		{
			Error(WARN, 0, "CMD_FPGA_SIMU_PRINT");
			sok_send_2(&_HeadCtrl[i].socket, CMD_FPGA_SIMU_PRINT, 0, NULL);
		}
	}
	return REPLY_OK;
}

//--- ctrl_simu_encoder --------------------------------------------------
int ctrl_simu_encoder(int khz)
{
	if (enc_simu_encoder(khz)!=REPLY_OK)	// first try simulation using the encoder board
	{
		int i;
		for (i=0; i<SIZEOF(_HeadCtrl); i++)
		{
			if (_HeadCtrl[i].running)
			{
				Error(WARN, 0, "Head[%d]: CMD_FPGA_SIMU_ENCODER, %d KHz", i, khz);
				if (sok_send_2(&_HeadCtrl[i].socket, CMD_FPGA_SIMU_ENCODER, sizeof(khz), &khz))
					Error(ERR_CONT, 0, "Could not send CMD_FPGA_SIMU_ENCODER");
			}
		}		
	}
	return REPLY_OK;
}

//--- ctrl_reply_stat ---------------------------------------------
void ctrl_reply_stat(RX_SOCKET socket)
{
	int i;
	for (i=0; i<HEAD_BOARD_CNT; i++)
	{
		RX_HBStatus[i].boardNo=i;
		RX_HBStatus[i].info.connected = (_HeadCtrl[i].socket!=INVALID_SOCKET);
	//	RX_HBStatus[i].info.flushed   = (_HeadsFlushed & (0x01LL<<i)) != 0;

		if (_HeadCtrl[i].running) 
			gui_send_msg_2(socket, REP_HEAD_STAT, sizeof(SHeadBoardStat), &RX_HBStatus[i]);
	}
}

//--- ctrl_set_cluster_no ----------------------------------------------
void ctrl_set_cluster_no(SValue* pdata)
{
    if (_HeadCtrl[pdata->no - 1].socket != INVALID_SOCKET)
		sok_send_2(&_HeadCtrl[pdata->no - 1].socket, CMD_CHANGE_CLUSTER_NO, sizeof(&pdata), pdata);
}

//--- ctrl_reset_cond ------------------------------
void ctrl_reset_cond(void)
{
    for (int i = 0; i < SIZEOF(_HeadCtrl); i++)
    {
        if (_HeadCtrl[i].socket != INVALID_SOCKET)
            sok_send_2(&_HeadCtrl[i].socket, CMD_RESET_COND, 0, NULL);
    }
    
}