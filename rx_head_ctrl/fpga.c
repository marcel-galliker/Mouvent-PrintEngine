// ****************************************************************************
//
//	DIGITAL PRINTING - fpga.c
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
#ifdef linux
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <sys/mman.h>
	#include <unistd.h>
#endif
#include <fcntl.h>
#include <errno.h>
#include "rx_common.h"
#include "rx_csv.h"
#include "rx_error.h"
#include "rx_file.h"
#include "rx_fpga.h"
#include "rx_mac_address.h"
#include "rx_term.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "nios.h"
#include "arp.h"
#include "bmp.h"
#include "firepulse.h"
#include "fpga_def_head.h"
#include "fpga_simu.h"
#include "udp_test.h"
#include "args.h"
#include "conditioner.h"
#include "tse.h"
#include "putty.h"
#include "rx_head_ctrl.h"
#include "fpga.h"

//---- COMPILER OPTIONS ------------------------------------------------------

#ifdef DEBUG
// #define DDR3_TEST
// #define HEAD_DATA_TEST
#endif

//--- defines -----------------------------------------------------------------
#define SET_FLAG(cmd, flag, set) {if(set) cmd|=(flag); else cmd&=~(flag);}
#define PRINTF(n)  for(i=0; i<n; i++) term_printf

//--- structures ----------------------------
typedef struct
{
	INT32				qsys_id;
	INT32				qsys_timestamp;
	SFpgaHeadStat		*stat;
	UINT32				*blockUsed;
	SFpgaPrintList		*print;
	SFpgaDataStat       *data;
	SFpgaHeadError		*error;
	SFpgeEncoderStat	*encoder;
} SFpga;

//--- globals -----------------------------------------------------------------
SFpga				Fpga;
SFpgaHeadBoardCfg	FpgaCfg;

#define	FPGA_FREQ 140000000

#define MAX_PAGES	128
#define MAX_SPEED	100000	// Hz
#define DROP_SPEED	8.000	// m/sec
#define DPI			1200

#define OFFSET_FWD	0
#define OFFSET_BWD	1

#define MAX_FPGA_TEMP	60

//--- module globals ---------------------------------------
static int		_MemId=0;
static BYTE		*_QSYS_mem=NULL;
#define QSYS_MEM_SIZE 0x1000
static BYTE		*_AXI_mem=NULL;
#define AXI_MEM_SIZE 0x1000

static int		_Init=FALSE;
static int		_Reload_FPGA = FALSE;
static int		_Load_Time=0;
static SPageId	_PageId[MAX_PAGES];
static UINT32	_PageEnd[HEAD_CNT][MAX_PAGES];
static UINT32	_BlockOutIdx[HEAD_CNT];
static UINT32	_ImgOutIdx[HEAD_CNT];	// use for tests
static UINT32	_AliveCnt[UDP_PORT_CNT];
static UINT32	_AliveChk_Timeout=0;
static UINT32	_PrintDoneError;
static int		_TempWarn=0;
static int		_TempErr=0;
static UINT32	_ImgInIdx;
static UINT32	_FirstImage;
static UINT32	_PdCnt;
static UINT32	_PgTestFlag;
static UINT32	_PgTestCnt;
static UINT32	_PgSimuIn, _PgSimuOut;
static int		_UdpIsLocal;
static int		_SynthEncoder=FALSE;
static BYTE		*_FpgaBase;
static int		_EncCheckDelay=0;
static UINT32*  _Buffer[HEAD_CNT]={NULL,NULL,NULL,NULL};	// for DDR3-Tests
static UINT32*	_ImgBuf[HEAD_CNT];
static UINT32	_DataCnt[HEAD_CNT];
static UINT32	_LastData[HEAD_CNT];
static int		_TestPgTime[HEAD_CNT];
// static int		_TestFSM;

static FILE		*_DataLog;
static SFpgaImage _Img[HEAD_CNT][MAX_PAGES];	// for debugging

//--- bidirectional: Changing offsets ----------------------- 
static int		_Bidir;
static int		_Direction;
static BYTE		_BidirCnt[HEAD_CNT];
static UINT32	_PgOffset[HEAD_CNT][2];		// foreward/backward

//--- prototypes -------------------------------------------
static void  _read_linux_version(void);
static void  _ethernet_config(void);
static void  _fpga_enc_config(int synth);
static void  _set_mac_addr(UINT64 macAddr, SFpgaEthCfg *ethcfg);
static char* _get_mac_addr(SFpgaEthCfg *ethcfg, char *str);
static void  _set_ip_addr(SFpgaEthCfg *ethcfg, UINT32 ipAddr);
static char* _get_ip_addr(SFpgaEthCfg *ethcfg, char *str);
static void  _write_srd(const char *srcName, int subPulses, int stroke, BYTE *data, int dataSize);

static int   _check_print_done(void);
static void  _handle_pd(int pd);
static void  _check_errors(void);
static void  _count_dots(void);
static void  _check_state_machines(void);

static int _check_block_used_flags(int head, int blkNo, int blkCnt);
static int _check_block_used_flags_clear(int head, int imgNo, int blkNo, int blkCnt);
static int _trace_used_flags(int head, int blkNo, int blkCnt, int blkEnd);
static void _fpga_copy_status(void);
static void _fpga_check_fp_errors(void);
static void _fpga_set_pg_offsets(int bwd);

//*** functions ********************************************


//--- fpga_init ----------------
void fpga_init()
{
	_Init = FALSE;
	_Load_Time = rx_get_ticks()+6000;
	if (rx_fpga_load (PATH_BIN_HEAD FIELNAME_HEAD_RBF)!=REPLY_OK) 
	{
		int i;

		Error(WARN, 0, "TEST");
		
		FpgaCfg.cfg		= (SFpgaMainCfg*)	rx_malloc(sizeof(SFpgaMainCfg));
		for (i=0; i<SIZEOF(FpgaCfg.head); i++) FpgaCfg.head[i] = (SFpgaHeadCfg*)rx_malloc(sizeof(SFpgaHeadCfg));
		for (i=0; i<SIZEOF(FpgaCfg.eth);  i++) FpgaCfg.eth[i]  = (SFpgaEthCfg*) rx_malloc(sizeof(SFpgaEthCfg));
		FpgaCfg.udp		= (SFpgaUdpCfg*)	rx_malloc(sizeof(SFpgaUdpCfg));

		Fpga.stat		= (SFpgaHeadStat*) 	rx_malloc(sizeof(SFpgaHeadStat));
		Fpga.print		= (SFpgaPrintList*) rx_malloc(sizeof(SFpgaPrintList));	//	SFpgaPrintData
		FpgaCfg.encoder = (SFpgaEncoderCfg*)rx_malloc(sizeof(SFpgaEncoderCfg));
		return;
	}

#ifdef WIN32
	RX_HBConfig.dataBlkSize = RX_UdpBlockSize[0];
#endif
#ifdef soc
	{
		int i;
		BYTE   *byte;
		UINT32 *test;

		//--- map the meory ------------------
		_MemId = open("/dev/mem", O_RDWR | O_SYNC);
		if (_MemId==-1) Error(ERR_CONT, 0, "Could not open memory handle.");

		//--- qsys id ---------------------------------------
		{
			_QSYS_mem = (BYTE*)mmap(NULL, QSYS_MEM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, _MemId, 0xff210000);
			UINT32* pQsys = (UINT32*)_QSYS_mem;
			Fpga.qsys_id 		 = pQsys[0];
			Fpga.qsys_timestamp  = pQsys[1];		
		}		
		
		_AXI_mem	  = (BYTE*)mmap(NULL, AXI_MEM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, _MemId, 0xc0010000);
		tse_init(&_AXI_mem[0x800], &_AXI_mem[0x400]);		
		fp_init	(_MemId);

		_FpgaBase   = (BYTE*)													rx_fpga_map_page(_MemId, FPGA_BASE_ADDR,					0x30000,					0x30000);
		FpgaCfg.cfg = (SFpgaMainCfg*)											rx_fpga_map_page(_MemId, FPGA_BASE_ADDR,					sizeof(SFpgaMainCfg),		0x00a0);
		for (i=0; i<SIZEOF(FpgaCfg.head); i++)	FpgaCfg.head[i]=(SFpgaHeadCfg*) rx_fpga_map_page(_MemId, FPGA_BASE_ADDR+0x01000+i*0x1000,	sizeof(SFpgaHeadCfg),		0x0108);
		for (i=0; i<SIZEOF(FpgaCfg.eth); i++)	FpgaCfg.eth[i] =(SFpgaEthCfg*)	rx_fpga_map_page(_MemId, FPGA_BASE_ADDR+0x05000+i*0x1000,   sizeof(SFpgaEthCfg),		0x0640);
		FpgaCfg.udp		= (SFpgaUdpCfg*)										rx_fpga_map_page(_MemId, FPGA_BASE_ADDR+0x07000,			sizeof(SFpgaUdpCfg),		0x0024);
		Fpga.blockUsed	= (UINT32*)												rx_fpga_map_page(_MemId, FPGA_BASE_ADDR+0x08000,			DATA_BLOCK_MEMSIZE,			DATA_BLOCK_MEMSIZE);

		Fpga.print		= (SFpgaPrintList*)										rx_fpga_map_page(_MemId, FPGA_BASE_ADDR+0x20000,			sizeof(SFpgaPrintList),		0x00e0); 
		FpgaCfg.encoder = (SFpgaEncoderCfg*)									rx_fpga_map_page(_MemId, FPGA_BASE_ADDR+0x28000,			sizeof(SFpgaEncoderCfg),	0x0014);

//		Fpga.stat       = (SFpgaHeadStat*)										rx_fpga_map_page(_MemId, FPGA_BASE_ADDR+0x2C000,			sizeof(SFpgaHeadStat),		0x0320);
		Fpga.stat       = (SFpgaHeadStat*)										rx_fpga_map_page(_MemId, FPGA_BASE_ADDR+0x2C000,			sizeof(SFpgaHeadStat),		0x0340);
		Fpga.error		= (SFpgaHeadError*)										rx_fpga_map_page(_MemId, FPGA_BASE_ADDR+0x2D000,			sizeof(SFpgaHeadError),		0x00b4);
		Fpga.data		= (SFpgaDataStat*)										rx_fpga_map_page(_MemId, FPGA_BASE_ADDR+0x2E000,			sizeof(SFpgaDataStat),		0x00a0);
		Fpga.encoder	= (SFpgeEncoderStat*)									rx_fpga_map_page(_MemId, FPGA_BASE_ADDR+0x2F000,			sizeof(SFpgeEncoderStat),	0x0018);

		RX_HBStatus[0].fpgaVersion.major	= Fpga.stat->version.major;
		RX_HBStatus[0].fpgaVersion.minor	= Fpga.stat->version.minor;
		RX_HBStatus[0].fpgaVersion.revision = Fpga.stat->version.revision;
		RX_HBStatus[0].fpgaVersion.build	= Fpga.stat->version.build;

		TrPrintfL(TRUE, "FPGA Version: %d.%d.%d.%d", Fpga.stat->version.major, Fpga.stat->version.minor, Fpga.stat->version.revision, Fpga.stat->version.build);
		_Init = TRUE;
		_Load_Time = 0;
	}

	{
		UINT64 macAddr;
		int head;
	
		char str[32];
		UINT32 addr;
		UCHAR	no;

		sok_get_ip_address_num("eth0", &addr);
		no = addr>>24;
		if (arg_offline)
		{
			sprintf(str, "192.168.200.%d", no+10);
			RX_HBConfig.dataAddr[0] = sok_addr_32(str);
			sprintf(str, "192.168.200.%d", no+20);
			RX_HBConfig.dataAddr[1] = sok_addr_32(str);
		}
		else
		{
			sprintf(str, "192.168.201.%d", no);
			RX_HBConfig.dataAddr[0] = sok_addr_32(str);
			sprintf(str, "192.168.202.%d", no);
			RX_HBConfig.dataAddr[1] = sok_addr_32(str);
		}
	//		RX_HBConfig.dataPort[0] = PORT_UDP_DATA;
	//		RX_HBConfig.dataPort[1] = PORT_UDP_DATA;

		if (arg_offline) RX_HBConfig.dataBlkSize = RX_UdpBlockSize[0];
		else             RX_HBConfig.dataBlkSize = RX_UdpBlockSize[3];
		RX_HBConfig.dataBlkCntHead  = 1024*1024*1024 / RX_HBConfig.dataBlkSize;
		RX_HBConfig.dataBlkCntHead  = RX_HBConfig.dataBlkCntHead & ~127;  // align to 4*32-Bit BlockUsed-Flags
		RX_HBConfig.dataBlkCntHead /= HEAD_CNT;
		RX_HBConfig.dataBlkCntHead  = 0x10000; 

		for (head=0; head<4; head++)
		{
			RX_HBConfig.head[head].enabled = arg_offline ? dev_on: dev_off;
			RX_HBConfig.head[head].blkCnt  = RX_HBConfig.dataBlkCntHead;
			RX_HBConfig.head[head].blkNo0  = RX_HBConfig.head[head].blkCnt*head;
		}
		_read_linux_version();
	}
#endif
}

// --- fpga_end -------------------
void fpga_end()
{
#ifdef linux
	_Init = FALSE;

	tse_end();
	fp_end();
	if (munmap(_QSYS_mem,		QSYS_MEM_SIZE))			Error(ERR_CONT, 0, "Could not UNMAP QSYS-Memory");
	if (munmap(_AXI_mem,		AXI_MEM_SIZE))			Error(ERR_CONT, 0, "Could not UNMAP AXI-Memory");
	if (munmap(FpgaCfg.cfg,		sizeof(*FpgaCfg.cfg)))  Error(ERR_CONT, 0, "Could not UNMAP Fpga.cfg");
	if (munmap(Fpga.stat,		sizeof(*Fpga.stat)))    Error(ERR_CONT, 0, "Could not UNMAP Fpga.stat");

	close (_MemId);
	_MemId = 0;
#endif
}

//--- _ethernet_config -------------------------------
void _ethernet_config()
{
	UINT64	macAddr;
	int		i;
	char	str[64];
	UINT32	ipAddr;

	#ifdef WIN32
		return;		
	#endif

	sok_get_mac_address("eth0", &macAddr);
	sok_get_ip_address_str("eth0", str, sizeof(str));
	ipAddr = sok_addr_32(str) & 0x00ffffff;

	_UdpIsLocal = TRUE;

	RX_HBStatus[0].macAddr=macAddr;
	RX_HBStatus[0].serialNo = swap_uint16((UINT16)(macAddr>>32));
	macAddr &= ~MAC_ID_MASK;
	for (i=0; i<SIZEOF(FpgaCfg.eth); i++)
	{
		_set_ip_addr(FpgaCfg.eth[i], RX_HBConfig.dataAddr[i]);
		_UdpIsLocal &= (RX_HBConfig.dataAddr[i] & 0x00ffffff)==ipAddr;
		FpgaCfg.eth[i]->ipPort = PORT_UDP_DATA;
		macAddr_udp(i, (int)RX_HBStatus[0].serialNo, &macAddr);
		_set_mac_addr(macAddr, FpgaCfg.eth[i]);
		tse_set_mac_addr(i, macAddr);
		_get_mac_addr(FpgaCfg.eth[i], RX_MacAddr[i]);
		_get_ip_addr (FpgaCfg.eth[i], RX_IpAddr[i]);
	}
//	nios_macAddrLoaded();
	
	for (i=0; i<SIZEOF(FpgaCfg.eth); i++)
	{
		TrPrintfL(TRUE, "ETH[%d].macAddr   =%s", i, _get_mac_addr(FpgaCfg.eth[i], str));
		TrPrintfL(TRUE, "ETH[%d].ipAddr    =%s", i, _get_ip_addr(FpgaCfg.eth[i], str));
		TrPrintfL(TRUE, "ETH[%d].port      =%d", i, FpgaCfg.eth[i]->ipPort);
	}
	
	TrPrintfL(TRUE, "Data Block Size  =%d\n", RX_HBConfig.dataBlkSize);
}

//--- _read_linux_version ----------------------------------------------
static void  _read_linux_version(void)
{
	FILE *file;
	RX_VersionLinux = 0;
	file = rx_fopen("/root/version_flash", "rt", _SH_DENYNO);
	if (file!=NULL) 
	{
		fscanf(file, "%d", &RX_VersionLinux);
		fclose(file);
	}
}

//--- fpga_set_config ----------------------------------------------
int  fpga_set_config(RX_SOCKET socket)
{
	int i, n, head;
	char str[64];

	fpga_enable(FALSE);

	_ethernet_config();

	if (!RX_VersionLinux) Error(WARN, 0, "Head unreliable startup of operating system from SD-Card. CHANGE TO FLASH booting!");

	//--- general config -------------------------------------------------------
	FpgaCfg.cfg->head_type = HEAD_TYPE_GEMINI;
	FpgaCfg.cfg->err_reset = 0xffffffff;
	_TempWarn = 0;
	_TempErr  = 0;
	_PrintDoneError = 0;

	//--- head -----------------------------------------------------------------
	nios_set_firepulse_on(FALSE);
	_Bidir     = FALSE;
	for (i=0; i<SIZEOF(FpgaCfg.head); i++)
	{
		FpgaCfg.head[i]->cmd_enable			= FALSE; // need toggle to reset!
		FpgaCfg.head[i]->cmd_enable         = (RX_HBConfig.head[i].enabled == dev_on);
		FpgaCfg.head[i]->encoderNo			= RX_HBConfig.head[i].encoderNo;

		_PgOffset[i][OFFSET_FWD] = (UINT32)((1.0*RX_HBConfig.head[i].dist)/21.166667);
		_PgOffset[i][OFFSET_BWD] = (UINT32)((1.0*RX_HBConfig.head[i].distBack)/21.166667);
		_BidirCnt[i]=0;
		_Direction = OFFSET_FWD;
		FpgaCfg.head[i]->subStroke			= 0;
		FpgaCfg.head[i]->offset_stroke		= _PgOffset[i][OFFSET_FWD];
		FpgaCfg.head[i]->offset_substroke	= 0;
		if (FALSE && RX_HBConfig.head[i].headHeight>0)
		{
			double time = RX_HBConfig.head[i].headHeight/1000000.0/DROP_SPEED;
			FpgaCfg.head[i]->speedComp_MaxSpeed = MAX_SPEED;
			FpgaCfg.head[i]->speedComp_MaxFs	= MAX_SPEED*time;		
		}
		else
		{
			FpgaCfg.head[i]->speedComp_MaxSpeed = 0;
			FpgaCfg.head[i]->speedComp_MaxFs	= 0;							
		}

		FpgaCfg.head[i]->firstJetUsed	= RX_HBConfig.head[i].jetEnabled0;
		FpgaCfg.head[i]->firstJetUnused	= RX_HBConfig.head[i].jetEnabled0 + RX_HBConfig.head[i].jetEnabledCnt;
		
		//--- check waveform ---
		if (FpgaCfg.head[i]->cmd_enable)
		{				
			for (n=0; n<(int)FpgaCfg.head[i]->fp_subPulses; n++)
			{
				if ((n>0 && FpgaCfg.head[i]->fp[n].on<=FpgaCfg.head[i]->fp[n-1].off)
				||          FpgaCfg.head[i]->fp[n].off<=FpgaCfg.head[i]->fp[n].on
				||			FpgaCfg.head[i]->fp[n].off==0)
				{
					Error(ERR_ABORT, 0, "Waveform head[%d] error", i);
					return REPLY_ERROR;
				}
			}
		}
	}

	//--- UDP Interface --------------------------------------------------------
	if (Fpga.blockUsed==NULL) Fpga.blockUsed  =(UINT32*) malloc((RX_HBConfig.dataBlkCntHead+7)/8*HEAD_CNT);

	for (i=0; i<SIZEOF(FpgaCfg.eth); i++)
	{
		TrPrintfL(TRUE, "ETH[%d].macAddr   =%s", i, _get_mac_addr(FpgaCfg.eth[i], str));
		TrPrintfL(TRUE, "ETH[%d].ipAddr    =%s", i, _get_ip_addr(FpgaCfg.eth[i], str));
		TrPrintfL(TRUE, "ETH[%d].port      =%d", i, FpgaCfg.eth[i]->ipPort);
	}

	FpgaCfg.udp->udp_block_size = 0;
	for (i=0; i<SIZEOF(RX_UdpBlockSize); i++)
	{
		if (RX_HBConfig.dataBlkSize==RX_UdpBlockSize[i]) 
		{
			FpgaCfg.udp->udp_block_size = i;
			break;
		}
	}

	for  (i=0; i<MAX_HEADS_BOARD; i++)
	{
		FpgaCfg.udp->block[i].blkNo0    = RX_HBConfig.head[i].blkNo0;
		FpgaCfg.udp->block[i].blkNoEnd  = RX_HBConfig.head[i].blkNo0+RX_HBConfig.head[i].blkCnt-1;
		TrPrintfL(TRUE, "Data_Blocks[%d]  = %d ... %d", i, FpgaCfg.udp->block[i].blkNo0, FpgaCfg.udp->block[i].blkNoEnd);
		Fpga.print->imgInIdx[i] = 0;
	}

//	Error(WARN, 0, "TEST Overheating");
	FpgaCfg.cfg->overheat_treshold = MAX_FPGA_TEMP+128;
	
	memset(_PageId,	   0, sizeof(_PageId));
	memset(_PageEnd,   0, sizeof(_PageEnd));
	memset(_AliveCnt,  0, sizeof(_AliveCnt));
	memset(_ImgOutIdx, 0, sizeof(_ImgOutIdx));
	memset(_TestPgTime,0, sizeof(_TestPgTime));
//	memset(_Buffer,    0, sizeof(_Buffer));

	for (i=0; i<SIZEOF(_PageEnd); i++)
	{
		_BlockOutIdx[i] = FpgaCfg.udp->block[i].blkNoEnd;
	}
	for (i=0; i<SIZEOF(RX_HBStatus[0].head); i++)
	{
		RX_HBStatus[0].head[i].imgInCnt		= 0;
		RX_HBStatus[0].head[i].printGoCnt   = 0;
		RX_HBStatus[0].head[i].printDoneCnt = 0;
		RX_HBStatus[0].head[i].dotCnt       = 0;
	}
	
	//--- clear memory in data path ---
	if (_ImgInIdx)
	{				
		SET_FLAG(FpgaCfg.cfg->cmd, CMD_MASTER_ENABLE, TRUE);
		_fpga_enc_config(40);
		rx_sleep(20);
		SET_FLAG(FpgaCfg.cfg->cmd, CMD_MASTER_ENABLE, FALSE);
	}				

	_PdCnt=0;
	_PgTestCnt=0;
	_PgTestFlag=Fpga.stat->enc_tel[0]&ENC_PG_FLAG;
	_PgSimuIn=0;
	_PgSimuOut=0;
	_ImgInIdx = 0;
	_FirstImage = 0;
	
	//---  set encoder ---------------------------------------------------
	_fpga_enc_config(arg_simu_machine);

	//--- general settings --------------------
	
	//--- DDR3 DATA DEBUG ------------------------------------------------------
	#ifdef DDR3_TEST
	{	
		Error(WARN, 0, "TEST CMD_DDR3_DEBUG");
		SET_FLAG(FpgaCfg.cfg->cmd, CMD_DDR3_DEBUG, TRUE);
		_DataLog = fopen(PATH_TEMP "DataLog.txt", "wt");
		for (i=0; i<SIZEOF(_Buffer); i++)
		{
			if (_Buffer[i]==NULL) _Buffer[i]=(UINT32*)malloc(RX_HBConfig.dataBlkSize * RX_HBConfig.dataBlkCntHead);
			_ImgBuf[i] = _Buffer[i];
			_DataCnt[i]=0;
			_LastData[i] = 0x1234;
		}
	}
	#endif
	
	//--- HEAD DATA DEBUG ------------------------------------------------------
	#ifdef HEAD_DATA_TEST
	{
		Error(WARN, 0, "TEST CMD_HEAD_DATA_DEBUG");
		SET_FLAG(FpgaCfg.cfg->cmd, CMD_HEAD_DATA_DEBUG, TRUE);		
	}
	#endif
		

	//--- MASTER ENABLE ------------------------------------------------------
	TrPrintfL(TRUE, "set CMD_MASTER_ENABLE=TRUE");
	SET_FLAG(FpgaCfg.cfg->cmd, CMD_MASTER_ENABLE, TRUE);
	_AliveChk_Timeout = 5;

//	Error(LOG,  0, "fpga_set_config1: FSM State=0x%04x", Fpga.stat->info);
//	_check_state_machines();
	
	SFpgaHeadInfo* pinfo = &Fpga.stat->info;
	if (pinfo->clearing_udp_flags) 
	{
//		Error(WARN, 0, "Timeout while clearing Used Block Flags");
		rx_sleep(10);
	}
	if (pinfo->clearing_udp_flags) 
	{
		_Reload_FPGA=TRUE;
		return Error(ERR_ABORT, 0, "Timeout while clearing Used Block Flags");
	}
	
	for (head=0; head<SIZEOF(RX_HBStatus[0].head); head++) 
	{
#ifdef USE_HEAD_PRESOUT
		cond_set_config(head, &RX_HBConfig.head[head].cond);
#endif
		_check_block_used_flags_clear(head, 0, RX_HBConfig.head[head].blkNo0, RX_HBConfig.head[head].blkCnt);
	}

	//	Error(LOG,  0, "fpga_set_config2: FSM State=0x%04x", Fpga.stat->info);
//	_TestFSM = 0;
	return REPLY_OK;
}

//--- _fpga_set_pg_offsets ------------------------------------------------
static void _fpga_set_pg_offsets(int bwd)
{
	int i;
	
	if (_Bidir)
	{
		for (i=0; i<HEAD_CNT; i++)
		{
//			TrPrintfL(TRUE, "Head[%d]: ImgOut[0]=%d, ImgOut[1]=%d, pg=%d, pd=%d", i, Fpga.data->imgOutIdx[i][0], Fpga.data->imgOutIdx[i][1], Fpga.stat->pg_ctr[i], Fpga.stat->print_done_ctr[i]);
			
			if ((Fpga.stat->pg_ctr[i] & 0xff) != _BidirCnt[i])
			{
				_BidirCnt[i]++;
				FpgaCfg.head[i]->subStroke			= 0;
				FpgaCfg.head[i]->offset_stroke		= _PgOffset[i][_BidirCnt[i]&0x01];
//				TrPrintfL(TRUE, "Head[%d].offset_stroke=%06d, pg=%d, _BidirCnt=%d", i, FpgaCfg.head[i]->offset_stroke, Fpga.stat->pg_ctr[i], _BidirCnt[i]);
				FpgaCfg.head[i]->offset_substroke	= 0;
			}
		}		
	}
	else
	{
		_Direction = bwd;
		for (i=0; i<HEAD_CNT; i++)
		{
			FpgaCfg.head[i]->subStroke			= 0;
			FpgaCfg.head[i]->offset_stroke		= _PgOffset[i][bwd];
			FpgaCfg.head[i]->offset_substroke	= 0;
		}					
	}
}


//--- fpga_enc_config ---------------------------------------------------
void fpga_enc_config(int khz)
{
	_fpga_enc_config(khz);
}

static void _fpga_enc_config(int khz)
{
	int i;
	
	SET_FLAG(FpgaCfg.encoder->cmd, ENC_ENABLE, FALSE);
	
	FpgaCfg.encoder->dist_pm			= 21166667; // 21.16 µm 
	FpgaCfg.encoder->dist_pm			=  1951000; // 1 µm
	FpgaCfg.encoder->shake_interval		= 0;

	if (khz>1)
	{
		FpgaCfg.encoder->synth.value = 0;
		FpgaCfg.encoder->synth.enable= _SynthEncoder = FALSE;
		nios_set_firepulse_on(TRUE);
		for (i=0; !nios_is_firepulse_on(); i+=10)
		{
			if (i>1000) 
			{
				Error(LOG, 0, "Firepulse On TimeOut");
				return;					
			}
			rx_sleep(10);				
		}
		FpgaCfg.encoder->synth.value = FPGA_FREQ/(khz*1000);
		FpgaCfg.encoder->synth.enable= _SynthEncoder = TRUE;		
	}
	else		
	{
		FpgaCfg.encoder->synth.value = 0;
		FpgaCfg.encoder->synth.enable= _SynthEncoder = FALSE;
		nios_set_firepulse_on(FALSE);
	}

	SET_FLAG(FpgaCfg.encoder->cmd, ENC_SIGNAL_MODE, 0);  // 0: Telegram, 1:no telegram

	SET_FLAG(FpgaCfg.encoder->cmd, ENC_HEAD0_ENABLE, FALSE);
	SET_FLAG(FpgaCfg.encoder->cmd, ENC_HEAD1_ENABLE, FALSE);
	SET_FLAG(FpgaCfg.encoder->cmd, ENC_HEAD2_ENABLE, FALSE);
	SET_FLAG(FpgaCfg.encoder->cmd, ENC_HEAD3_ENABLE, FALSE);

	SET_FLAG(FpgaCfg.encoder->cmd, ENC_HEAD0_ENABLE, FpgaCfg.head[0]->cmd_enable);
	SET_FLAG(FpgaCfg.encoder->cmd, ENC_HEAD1_ENABLE, FpgaCfg.head[1]->cmd_enable);
	SET_FLAG(FpgaCfg.encoder->cmd, ENC_HEAD2_ENABLE, FpgaCfg.head[2]->cmd_enable);
	SET_FLAG(FpgaCfg.encoder->cmd, ENC_HEAD3_ENABLE, FpgaCfg.head[3]->cmd_enable);
	
	SET_FLAG(FpgaCfg.encoder->cmd, ENC_ENABLE, khz<=0);
	 
	RX_FpgaEncCfg.cmd          = FpgaCfg.encoder->cmd;
	RX_FpgaEncCfg.synth.enable = FpgaCfg.encoder->synth.enable;
	RX_FpgaEncCfg.synth.value  = FpgaCfg.encoder->synth.value;

//	memcpy(&RX_FpgaEncCfg, FpgaCfg.encoder, sizeof(RX_FpgaEncCfg));
	_EncCheckDelay = 2;	
}
	
//--- fpga_is_init -----------------------------
int  fpga_is_init(void)
{
	return _Init;
}

//--- fpga_is_ready ------------------
int	  fpga_is_ready(void)
{
	if (!_Init) return FALSE;
	if (_Load_Time && _Load_Time>rx_get_ticks()) return FALSE;
	if  (_Load_Time) Error(LOG, 0, "FPGA Load time over");
	_Load_Time = 0;
	return TRUE;			
}

//--- fpga_udp_block_size ----------------------------------------
int   fpga_udp_block_size(void)
{
	return RX_HBConfig.dataBlkSize;
}

//--- _set_mac_addr -------------------------------------------------------
static void _set_mac_addr(UINT64 macAddr, SFpgaEthCfg *ethcfg)
{
	UCHAR *addr = (UCHAR*)&macAddr;	
	ethcfg->macAddressL = (((UINT32)addr[2])<<24) | (((UINT32)addr[3])<<16) | (((UINT32)addr[4])<<8) | (UINT32)addr[5];
	ethcfg->macAddressH =								                      (((UINT32)addr[0])<<8) | (UINT32)addr[1];
}

//--- _get_mac_addr ----------------------------------
static char* _get_mac_addr(SFpgaEthCfg *ethcfg, char *str)
{
	UCHAR addr[6];
	memcpy(addr, &ethcfg->macAddressL, 4);
	memcpy(addr+4, &ethcfg->macAddressH, 2);
	sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x", addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);
	return str;
}

//--- fpga_get_mac_addr ----------------------------------
void fpga_get_mac_addr(int udpNo, BYTE *addr)
{
	BYTE buf[6];
	int i;

	memset(addr, 0, 6);

	memcpy(buf,   &FpgaCfg.eth[udpNo]->macAddressL, 4);
	memcpy(buf+4, &FpgaCfg.eth[udpNo]->macAddressH, 2);
	for (i=0; i<6; i++) addr[i]=buf[5-i];
}

//--- fpga_get_ip_addr -----------------------------------------
void  fpga_get_ip_addr (int udpNo, UINT32 *addr)
{
	*addr=0;
	*addr = swap_uint32(FpgaCfg.eth[udpNo]->ipAddr);
}

//--- fpga_trace_registers ----------------------------------------
void fpga_trace_registers(char *fname)
{
// #ifdef dasdasda
	FILE *in;
	FILE *out;
	int  line, addr, no;
	UINT32 *pval, val;
	char valstr[32];
	char path[256];
	char str[256];
	char strout[256];

	sprintf(str, PATH_BIN_HEAD "fpga_config_regs_unrolled.csv");
	in  = fopen(str, "rt");

	for (no=0; no<10; no++)
	{
		sprintf(str, PATH_TEMP "%s_%d.csv", fname, no);
		if (!rx_file_exists(str)) break;
	}

	strcpy(path, str);
	out = fopen(str, "wt");
	if (in!=NULL && out!=NULL)
	{
		for (line=1; fgets(str, 256, in); line++)
		{			
			if (line<4) fprintf(out, str);
			else if (line==4)
			{
				csv_put(str, 17, "VALUE", strout);
				fprintf(out, strout);
			}
			else if (line>6)
			{
				addr = csv_get_int(str, 9);
				if (addr>=0)
				{
					pval = (UINT32*)(_FpgaBase+addr);
					val  = *pval;
					sprintf(valstr, "0x%08x", val);
					csv_put(str, 17, valstr, strout);
					fprintf(out, strout);
				}
				else fprintf(out, str);
			}
			else fprintf(out, str);
		}
	}
	if (TRUE)
	{
		int i;
		int head=0;
		for (i=0; i<(int)Fpga.print->imgInIdx[head]; i++)
		{
			fprintf(out, "Image[%d.%d]: blkNo=%d blkCnt=%d\n", head, i, _Img[head][i].blkNo, _Img[head][i].blkCnt);
		}
	}
	if (in!=NULL) fclose(in);
	if (out!=NULL) fclose(out);
	
	ctrl_send_file(path);
//	#endif
}

//--- fpga_get_block_used -----------------------------------------------
UINT32  fpga_get_block_used(int headNo, UINT32 blkNo, UINT32 blkCnt, UINT32 *buffer)
{
	UINT32 min32=RX_HBConfig.head[headNo].blkNo0/32;
	UINT32 max32=(RX_HBConfig.head[headNo].blkNo0+RX_HBConfig.head[headNo].blkCnt+31)/32;
	UINT32 blk32=blkNo/32;
	UINT32 blkCnt32=(blkNo+blkCnt+31)/32-blk32;
	UINT32 cnt;
	for (cnt=0; cnt<blkCnt32; )
	{
		buffer[cnt++] = Fpga.blockUsed[blk32++];
		if (blk32==max32) blk32=min32;
	}
	return 32*cnt;
}

//--- fpga_display_used_flags -------------------------------------------------
void fpga_display_used_flags(void)
{
	int head=0;
	int img =0;
	UINT32 blk, i;
	UINT32 data;

	if (!_Init) return;

	term_printf("imgInIdx:    "); PRINTF(4)("     %03d   ",  Fpga.print->imgInIdx[i]); term_printf("\n");
	term_printf("imgOutIdx:   "); PRINTF(4)(" %03d %03d   ", Fpga.data->imgOutIdx[i][0],		Fpga.data->imgOutIdx[i][1]); term_printf("\n");

	for (img=Fpga.print->imgInIdx[head]; img<Fpga.data->imgOutIdx[head][0]; img++)
	{
	//	printf("Head[%d].UsedFlags (%d .. %d)", head, RX_HBConfig.head[head].blkNo0, RX_HBConfig.head[head].blkNo0+RX_HBConfig.head[head].blkCnt);
		TrPrintfL(TRUE, "Head[%d].img[%d].UsedFlags (%d .. %d)\n", head, img, _Img[head][img].blkNo, _Img[head][img].blkNo+_Img[head][img].blkCnt);
	//	for (blk=RX_HBConfig.head[head].blkNo0, i=0; i<RX_HBConfig.head[head].blkCnt; i++, blk++)
		for (blk=_Img[head][img].blkNo, i=0; i<_Img[head][img].blkCnt; i++, blk++)
		{
			if (blk%100==0) TrPrintfL(TRUE, "\nblk[%03d]:", blk);
			if (blk%10==0)	TrPrintfL(TRUE," ", NULL);
			if (blk%32==0)  data =  Fpga.blockUsed[blk/32];
			if (data & (1<<(blk%32))) TrPrintfL(TRUE,"*");
			else					  TrPrintfL(TRUE,".");
		}
		TrPrintfL(TRUE,"\n");
	}
	/*	
	{
		SFpgaImage	*pimg;
		int idx = Fpga.data->imgOutIdx[head][0];
		UINT32 end;
		pimg = &_Img[0][idx];
		end = pimg->blkNo+pimg->blkCnt;
		if (end>RX_HBConfig.head[head].blkNo0+RX_HBConfig.head[head].blkCnt) end -= RX_HBConfig.head[head].blkCnt;
		printf("Image[%d]: blocks %d .. %d\n", idx, pimg->blkNo, end);
	}
	*/

	TrPrintfL(TRUE, "Press <ENTER> to continue.");
	getchar();
}

//--- _check_block_used_flags --------------------------------------
static int _check_block_used_flags(int head, int blkNo, int blkCnt)
{
	int blk, bit;
	int min = FpgaCfg.udp->block[head].blkNo0;
	int max = FpgaCfg.udp->block[head].blkNoEnd;
	int err=FALSE;
	
	UINT32 flags;
	flags = Fpga.blockUsed[blkNo/32];
	for (blk=blkNo; blkCnt>0; blkCnt--, blk++)
	{
		if (blk>max) blk=min;
		bit = blk%32;
		if (!bit) flags = Fpga.blockUsed[blk/32];
		if (!(flags & (1<<bit)) && !err) err=Error(ERR_CONT, 0, "DataBlock[%d] missing", blk);
	}
}

//--- _check_block_used_flags_clear --------------------------------------
static int _check_block_used_flags_clear(int head, int imgNo, int blkNo, int blkCnt)
{
	int blk, bit, n;
	int min = FpgaCfg.udp->block[head].blkNo0;
	int max = FpgaCfg.udp->block[head].blkNoEnd;
	int reply=REPLY_OK;
	
//	TrPrintfL(TRUE, "CHECK_BLOCK_USED_FLAGS_CLEAR(head=%d, imgNo=%d, blkNo=%d, blkCnt=%d", head, imgNo, blkNo, blkCnt);
	UINT32 flags;
	flags = Fpga.blockUsed[blkNo/32];
//	for (blk=blkNo; blkCnt>0; blkCnt--, blk++)
	for (blk=blkNo, n=0; n<blkCnt; n++, blk++)
	{
		if (blk>max)blk=min;			
		bit = blk%32;
		if (!bit) flags = Fpga.blockUsed[blk/32];
		if ((flags & (1<<bit)) && reply==REPLY_OK)
		{
			reply=Error(ERR_CONT, 0, "Head[%d]: image=%d BlockUsedFlag[%d] not cleared (from=%d, cnt=%d, blk=%d)", head, imgNo, blk, blkNo, blkCnt, n, Fpga.data->blockCnt[head]);		
			fpga_trace_registers("BlockUsedFlag-not-cleared");
		}
	}
	return reply;
}

//--- _trace_used_flags ----------------------------------------------
static int _trace_used_flags(int head, int blkNo, int blkCnt, int blkEnd)
{
	int blk, bit;
	int min = FpgaCfg.udp->block[head].blkNo0;
	int max = FpgaCfg.udp->block[head].blkNoEnd;
	int err=FALSE;
	char str[256];
	int len;
	
	UINT32 flags;
	flags = Fpga.blockUsed[blkNo/32];
	TrPrintfL(TRUE, "Blk used Flags: %d .. %d, cnt=%d", blkNo, blkEnd, blkCnt);
	len=0;
	if (blkNo%100!=0) len=sprintf(str, "blk[% 5d]:", blkNo);
	for (blk=blkNo; blkCnt>0; blkCnt--, blk++)
	{
		if (blk>max) blk=min;
		bit = blk%32;
		if (blk%100==0)
		{
			str[len]=0;
			if (len) TrPrintfL(TRUE, str);
			len=sprintf(str, "blk[% 5d]:", blk);
		}
		if (blk%10==0) str[len++]= ' ';
		if (!bit) flags = Fpga.blockUsed[blk/32];
		if (flags & (1<<bit)) str[len++]= '*';
		else                  str[len++]= '.';
	}
	str[len]=0;
	TrPrintfL(TRUE, str);	
}

//--- fpga_get_blockOutIdx ---------------------------------
UINT32 fpga_get_blockOutIdx(int headNo)
{
	return _BlockOutIdx[headNo];
}

//--- fpga_get_aliveCnt ----------------
UINT32 fpga_get_aliveCnt(int udpNo)
{
	return _AliveCnt[udpNo];
}

//--- _set_ip_addr --------------------------------------------
static void _set_ip_addr(SFpgaEthCfg *ethcfg, UINT32 ipAddr)
{
	ethcfg->ipAddr = swap_uint32(ipAddr);
}

//--- _get_ip_addr ----------------------------------
static char* _get_ip_addr(SFpgaEthCfg *ethcfg, char *str)
{
	UCHAR addr[4];
	memcpy(addr, &ethcfg->ipAddr, 4);
	sprintf(str, "%d.%d.%d.%d", addr[3], addr[2], addr[1], addr[0]);
	return str;
}

//--- fpga_image ------------------------------------------------
int  fpga_image	(SFpgaImageCmd *msg)
{
	UINT8 idx;
	int head = msg->head;
	int trace = FALSE; // !arg_test_fire;

	TrPrintfL(trace, "size=0x%x", sizeof(Fpga.print->image));
	TrPrintfL(trace, "pos.image=0x%x", (BYTE*)&Fpga.print->image-(BYTE*)Fpga.print);
//	TrPrintfL(trace, "pos.printGoInIdx=0x%x", (BYTE*)&Fpga.print->printGoInIdx-(BYTE*)Fpga.print);

	TrPrintfL(trace, "head[%d].fpga_image(id=%d, page=%d, copy=%)", head, msg->id.id, msg->id.page, msg->id.copy);

	if (_Init)
	{
//		if (!_TestFSM) Error(LOG,  0, "fpga_image: FSM State=0x%04x", Fpga.stat->info);
//		_TestFSM = 1;

		if (head<0 || head>MAX_HEADS_BOARD) return Error(ERR_CONT, 0, "Head number %d out of range", head);

//		_check_block_used_flags(head, msg->image.blkNo, msg->image.blkCnt);

		TrPrintfL(trace, "imageListInIdx  = %d, %d, %d, %d", Fpga.print->imgInIdx[0], Fpga.print->imgInIdx[1], Fpga.print->imgInIdx[2], Fpga.print->imgInIdx[3]);
		TrPrintfL(trace, "imageListOutIdx = %d, %d, %d, %d", Fpga.data->imgOutIdx[0][0], Fpga.data->imgOutIdx[1][0], Fpga.data->imgOutIdx[2][0], Fpga.data->imgOutIdx[3][0]);

		idx = Fpga.print->imgInIdx[head];
		memcpy(&_PageId[idx], &msg->id, sizeof(SPageId));
		memcpy(&_Img[head][idx], &msg->image, sizeof(SFpgaImage));
		
		if (msg->image.backward & IMAGE_BIDIR) _Bidir = TRUE;				
		else
		{
			_Bidir=FALSE;
		//	TrPrintf(TRUE, "fpga_image.fpga_set_offset(%d)", msg->image.backward);
			_fpga_set_pg_offsets(msg->image.backward);			
		}
				
		_PageEnd[head][idx] = RX_HBConfig.head[head].blkNo0 + (msg->image.blkNo-RX_HBConfig.head[head].blkNo0+msg->image.blkCnt-1) % RX_HBConfig.head[head].blkCnt;

		TrPrintf(trace, "Image[hd=%d][%d] loaded: blocks %05d ... %05d, clearBlockUsed=%d", head, idx, msg->image.blkNo, _PageEnd[head][idx], msg->image.clearBlockUsed);

//		if (head==0) 
		if (FALSE)
		{
			TrPrintfL(TRUE, "\n");
			TrPrintfL(TRUE, "Image[head=%d][img=%d] loaded: blocks %05d ... %05d, clearBlockUsed=%d", head, idx, msg->image.blkNo, _PageEnd[head][idx], msg->image.clearBlockUsed);
		//	_trace_used_flags(head, msg->image.blkNo, msg->image.blkCnt, _PageEnd[head][idx]);
		}
		
		idx = (idx+1) % IMAGE_LIST_SIZE;
		if (idx==Fpga.data->imgOutIdx[head][1]) 
		{
			return Error(ERR_ABORT, 0, "fpga_image: Image Buffer Overflow. imgInIdx[%d]=%d imgOutIdx[%d]=%d\n", head, Fpga.print->imgInIdx[head], head, Fpga.data->imgOutIdx[head][1]);			
		}
		else
		{
			Fpga.print->image[head].bitPerPixel		= msg->image.bitPerPixel-1;
			Fpga.print->image[head].blkNo			= msg->image.blkNo;	
			Fpga.print->image[head].widthPx			= msg->image.widthPx;		
			Fpga.print->image[head].widthBytes		= msg->image.widthBytes;			
			Fpga.print->image[head].lengthPx		= msg->image.lengthPx;
			Fpga.print->image[head].jetPx0			= msg->image.jetPx0;
			Fpga.print->image[head].backward		= !(msg->image.backward & 1);
			Fpga.print->image[head].flipHorizontal	= msg->image.flipHorizontal;
			Fpga.print->image[head].clearBlockUsed	= msg->image.clearBlockUsed;

	//		if (head==0) Error(LOG, 0, "fpga_image[%d] id=%d, page=%d, copy=%d, clearBlockUsed=%d", Fpga.print->imgInIdx[head], msg->id.id, msg->id.page, msg->id.copy, msg->image.clearBlockUsed);

		//	TrPrintfL(TRUE, "fpga_image[imgInIdx=%d]: Udp ptr= %08d  %08d  %08d  %08d\n", Fpga.print->imgInIdx[head], Fpga.stat->blockOutNo[0], Fpga.stat->blockOutNo[1], Fpga.stat->blockOutNo[2], Fpga.stat->blockOutNo[3]);
		//	TrPrintfL(TRUE, "Image.blkNo = %08d\n", Fpga.print->image.blkNo);
			Fpga.print->imgInIdx[head] = idx;
			RX_HBStatus[0].head[head].imgInCnt++;
			if (head==0) _ImgInIdx++;

			if (RX_HBStatus[0].head[head].imgInCnt==1) 
			{
//				if (++_FirstImage==4) fpga_trace_registers("IMAGE-loaded");
			}
		}
//		_check_state_machines();
	}	
	nios_set_firepulse_on(TRUE);
	return REPLY_OK;
}

//--- _fpga_check_fp_errors -------------------------------------------
static void _fpga_check_fp_errors(void)
{
	int i;
	for (i=0; i<HEAD_CNT; i++) 
	{
		RX_FpgaData.wf_busy_warn[i] = Fpga.data->wf_busy_warn[i];
		memcpy(&RX_FpgaError.enc_fp[i], &Fpga.error->encoder[i], sizeof(UINT32));
		
		{
		//	if (Fpga.error->head[i].write_img_line)   {                    ErrorFlag (ERR_ABORT, (UINT32*)&RX_HBStatus[0].err,  err_printgo_but_no_data_0<<i,  0, "Head[%d]: Got Print Go but had no Data cnt=%d", i, Fpga.error->head[i].write_img_line);}
		//	if (Fpga.error->head[i].write_img_line)   {                    ErrorFlag (WARN,      (UINT32*)&RX_HBStatus[0].err,  err_printgo_but_no_data_0<<i,  0, "Head[%d]: Got Print Go but had no Data cnt=%d", i, Fpga.error->head[i].write_img_line);}
			if (Fpga.error->head[i].write_img_line || Fpga.error->img_line_err[0][i] || Fpga.error->img_line_err[1][i] || Fpga.error->img_line_err[2][i] || Fpga.error->img_line_err[3][i])
			{                    
				
				if (*((UINT32*)&RX_HBStatus[0].err) & (err_printgo_but_no_data_0<<i))
				{
					*((UINT32*)&RX_HBStatus[0].err) |= (err_printgo_but_no_data_0<<i);
					
					fpga_trace_registers("write_img_line");
					_trace_used_flags(i, _Img[0][0].blkNo, _Img[0][0].blkCnt, _PageEnd[0][0]);
					
					if (Fpga.error->head[i].write_img_line)   { Error(ERR_ABORT, 0, "Head[%d]: write-img-line.4: cnt=%d", i, Fpga.error->head[i].write_img_line);}
					if (Fpga.error->img_line_err[0][i])		  { Error(ERR_ABORT, 0, "Head[%d]: write-img-line.0: cnt=%d", i, Fpga.error->img_line_err[0][i]);}
					if (Fpga.error->img_line_err[1][i])		  { Error(ERR_ABORT, 0, "Head[%d]: write-img-line.1: cnt=%d", i, Fpga.error->img_line_err[1][i]);}
					if (Fpga.error->img_line_err[2][i])		  { Error(ERR_ABORT, 0, "Head[%d]: write-img-line.2: cnt=%d", i, Fpga.error->img_line_err[2][i]);}
					if (Fpga.error->img_line_err[3][i])		  { Error(ERR_ABORT, 0, "Head[%d]: write-img-line.3: cnt=%d", i, Fpga.error->img_line_err[3][i]);}
				}
			}			
		}
		if (Fpga.error->head[i].enc_fp_missed>1)   { if (ErrorFlag (WARN,	  (UINT32*)&RX_HBStatus[0].err,  err_firepulse_missed_0   <<i,  0, "Head[%d]: Firepulse missed", i)) fpga_trace_registers("FP-Missed");}
		if (RX_FpgaError.enc_fp[i].waveform_busy>1){ if (ErrorFlag (ERR_CONT, (UINT32*)&RX_HBStatus[0].err,  err_overspeed               ,  0, "Firepulse overspeed"))			 fpga_trace_registers("FP-Overspeed");}
	}	
}

//--- _fpga_copy_status -----------------------------------------------
static void _fpga_copy_status(void)
{
	if (_Init)
	{
		//--- simulation -------------
		static UINT32 data[4];
		static UINT32 cnt[4]={0,0,0,0};
					
		int head, change=FALSE;
		int blkCnt;
		char str[32], str1[32];
		char line[90];
		int i,l;
		int no[MAX_HEADS_BOARD];
		{
			UINT32 *src=(UINT32*)Fpga.stat;
			UINT32 *dst=(UINT32*)&RX_FpgaStat;
			for (i=0; i<sizeof(RX_FpgaStat); i+=4) *dst++ = *src++;
		}

		{
			UINT32 *src=(UINT32*)Fpga.error;
			UINT32 *dst=(UINT32*)&RX_FpgaError;
			for (i=0; i<sizeof(RX_FpgaError); i+=4) 
			{
				if (src<(UINT32*)&Fpga.error->enc_fp[0] || src>=(UINT32*)&Fpga.error->enc_fp[4])
					*dst++ = *src++;
			}
		}
		
		{
			UINT32 *src=(UINT32*)Fpga.data;
			UINT32 *dst=(UINT32*)&RX_FpgaData;
//			for (i=0; i<sizeof(RX_FpgaData); i+=4) *dst++ = *src++;
			for (i=0; i<sizeof(RX_FpgaData)-sizeof(Fpga.data->wf_busy_warn); i+=4) *dst++ = *src++;
		}
		
		{
			UINT32 *src=(UINT32*)Fpga.print;
			UINT32 *dst=(UINT32*)&RX_FpgaPrint;
			for (i=0; i<sizeof(RX_FpgaPrint); i+=4) *dst++ = *src++;
		}
		
		RX_FpgaCmd = FpgaCfg.cfg->cmd;
		
		for (i=0; i<SIZEOF(RX_BlockUsed); i++) RX_BlockUsed[i] = Fpga.blockUsed[RX_HBConfig.head[i].blkNo0/32];
		
		#ifdef DDR3_TEST
		if (_ImgInIdx>7)
		{
			//--- DDR3 test -----------------------------------------
			for (head=0; head<HEAD_CNT; head++)
			{
				UINT32 size, linelen;
				UINT32 data;
				int inIdx=Fpga.print->imgInIdx[head];

				if(Fpga.print->imgInIdx[head]<=_ImgOutIdx[head]) continue;
				int outIdx = _ImgOutIdx[head]; 
				while (Fpga.stat->ddr3[head].valid)
				{
					change = TRUE;
					linelen = (_Img[head][outIdx].widthBytes+31) & ~31; // multiple of 256 bits
					size = (linelen*_Img[head][outIdx].lengthPx);
					
					data = Fpga.stat->ddr3[head].data;
					data = swap_uint32(data);
				
					if (FALSE) // check DATA=ADDRESS
					{
						if (head==2 && data!=_LastData[head])
						{
							fprintf(_DataLog, "DATA[%d.%d] = %07d   UdpPtr=%08d\n", _DataCnt[head]/RX_HBConfig.dataBlkSize, _DataCnt[head]%RX_HBConfig.dataBlkSize, data, Fpga.stat->blockOutNo[head]);
							fflush(_DataLog);
							_LastData[head] = data;
						}
						_DataCnt[head]+=sizeof(data);
					}

				//	if (head==0)
					{
						*_ImgBuf[head]++ = data;
						cnt[head] += sizeof(data);
						if (cnt[head]>=size)
						{
							/*
							if (_DataLog) 
							{
								fprintf(_DataLog, "Image [%d.%d] widthPx=%d, lengthPx=%d, c=%d, cnt=%d\n", outIdx, head, _Img[head][outIdx].widthPx, _Img[head][outIdx].lengthPx, c, cnt[head]);
								fflush(_DataLog);
							}
							*/
							// if (FALSE && head==0)
							{
								term_printf("Head[%d]: END OF IMAGE %d\n", head, outIdx);
								//--- write the bitmap --------------------------------------
								char filename[MAX_PATH];
								SFpgaImage	*info = &_Img[head][outIdx];
								if (_DataLog) 
								{
									fprintf(_DataLog, "Image [%d.%d] widthPx=%d, widthBytes=%d, lengthPx=%d, jetPx0=%d, size=%d\n", outIdx, head, info->widthPx, info->widthBytes, info->lengthPx, info->jetPx0, cnt[head]);
									fflush(_DataLog);
								}
								sprintf(filename, "%s/print_%d_%d.bmp", PATH_TEMP, outIdx, head);
								bmp_write(filename, (UCHAR*)_Buffer[head], info->bitPerPixel, info->widthPx, info->lengthPx, linelen, FALSE);									
								ctrl_send_file(filename);
								remove(filename);
							}
							_BlockOutIdx[head] = _PageEnd[head][_ImgOutIdx[head]%MAX_PAGES];	
							_ImgOutIdx[head] ++;							
							_ImgBuf[head]=_Buffer[head];
							cnt[head] = 0;
							break;
						}
					}
				}
			}

			if (change)
			{
			//	term_printf("\nPress <ENTER> to continue.\n");
				term_flush();
			//	getchar();
			}
		}
		#endif // DDR3_TEST
		
		#ifdef HEAD_DATA_TEST
		//--- Head_data[0]Test ---------------------------------------------------
		{
			static int done=FALSE;
			static int stroke=0;
			int head=1;
			UINT32 n;
			int idx = Fpga.data->imgOutIdx[head][0]; 
			UINT32 d;
			UINT32 data[64];
			if (!done)
			{
				n=64;
				while (Fpga.stat->head_data[head].valid && stroke<(int)((_Img[0][idx].lengthPx+610)*FpgaCfg.head[0]->fp_subPulses))
				{
					d = Fpga.stat->head_data[head].data;
					data[--n] = d;
					if (n==0) 
					{
						_write_srd(udp_test_fname(), FpgaCfg.head[0]->fp_subPulses, stroke++, (BYTE*)data, sizeof(data));
						n=64;
					}
				}
				if (stroke) 
				{
					_write_srd(udp_test_fname(), FpgaCfg.head[0]->fp_subPulses, 0, NULL, 0);
					done=TRUE;
				}
			}
		}
		#endif	// HEAD_DATA_TEST
	}
}

//--- write_srd -------------------------------------------------------------
void _write_srd(const char *srcName, int subPulses, int stroke, BYTE *data, int dataSize)
{
	char	path[256];
	char	buf[256];
	int		x, xx;
	UINT	fp;
	int		dotCnt;
	int		lineLen=2048;
	static FILE *hex_file=NULL;
	static int done=FALSE;

	if (done) return;

	if (hex_file && data==NULL)
	{
		fclose(hex_file);
		hex_file = NULL;
		done = TRUE;
		if (arg_debug)
		{
			term_printf("\nHead-Data File >>%s<< created.\n", path);
			term_printf("Press <ENTER> to continue.\n");
			term_flush();
			getchar();
		}
		return;
	}

	if (hex_file==NULL)
	{
		char *voreback="fb";
		char *flip="lr";
		SFpgaImage	*pimg;

		pimg = &_Img[0][Fpga.data->imgOutIdx[0][0]];

		if (pimg->jetPx0>0)		sprintf(path, "%s%s+%d.%c%c.hex", PATH_TEMP, srcName, pimg->jetPx0, voreback[pimg->backward], flip[pimg->flipHorizontal]);
		else if (pimg->jetPx0)	sprintf(path, "%s%s%d.%c%c.hex",  PATH_TEMP, srcName, pimg->jetPx0, voreback[pimg->backward], flip[pimg->flipHorizontal]);
		else				sprintf(path, "%s%s.%c%c.hex",    PATH_TEMP, srcName, voreback[pimg->backward], flip[pimg->flipHorizontal]);
		hex_file = rx_fopen(path, "wb", _SH_DENYNO);
		if (hex_file == NULL) 
		{
			term_printf("\n cound not craete file >>%s<<\n", path);
			term_printf("Press <ENTER> to continue.\n");
			term_flush();
			getchar();
			return;
		}

		fputs("Dump of: head0 shift register data\n", hex_file);
		fputs("==================================\n", hex_file);
		sprintf(buf, "#fp per fp-seq : %d\n", subPulses);	fputs(buf, hex_file);
		sprintf(buf, "greyscale-array: 01\n", FpgaCfg.head[0]->gl_2_pulse[1]);	fputs(buf, hex_file);
		sprintf(buf, "image source   : %s\n", srcName);		fputs(buf, hex_file);
		fputs("\n", hex_file);
		fputs("                       |    64    |   63    |   62    |   61    |   60    |   59    |   58    |   57    |   56    |   55    |   54    |   53    |   52    |   51    |   50    |   49    |   48    |   47    |   46    |   45    |   44    |   43    |   42    |   41    |   40    |   39    |   38    |   37    |   36    |   35    |   34    |   33    |   32    |   31    |   30    |   29    |   28    |   27    |   26    |   25    |   24    |   23    |   22    |   21    |   20    |   19    |   18    |   17    |   16    |   15    |   14    |   13    |   12    |   11    |   10    |   09    |   08    |   07    |   06    |   05    |   04    |   03    |   02    |   01    |\n", hex_file);
		fputs("                       |  B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  |\n", hex_file);
		fputs(" y-pos |   #fp | #jets | 15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|\n", hex_file);
		fputs("-------+-------+-------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n", hex_file);
	}

	fp = stroke % subPulses;
	{
		if (fp==0) sprintf(buf, "% 6d  % 6d", stroke/subPulses, fp+1);
		else       sprintf(buf, "        % 6d", fp+1);
		fputs(buf, hex_file);
			
		dotCnt = 0;
		for (x=0; x<lineLen; x++)
		{
			if (data[x/8] & 1<<(x%8)) 
				dotCnt++;
		}
			
		if (dotCnt) sprintf(buf, "  % 6d   ", dotCnt);
		else        sprintf(buf, "       -   ");
		fputs(buf, hex_file);

		for (x=0; x<lineLen/8; x++)
		{
			switch (x & 0x03)
			{
			case 0x00:	xx = (x & ~0x03)+3; break;
			case 0x01:	xx = (x & ~0x03)+2; break;
			case 0x02:	xx = (x & ~0x03)+1; break;
			case 0x03:	xx = (x & ~0x03)+0; break;
			}

			sprintf(buf, "%02X", data[xx]);
		//	for (i=0; i<4; i++) if (buf[i]=='0') buf[i]='.';
			fputs(buf, hex_file);
			if (x%2==1) fputs(" ", hex_file);
		}

		fputs("\n", hex_file);
	}
	term_printf("stroke %d done\n", stroke);
}

//--- fpga_enable -----------------------------------
int  fpga_enable(int enable)
{
	if(_Init)
	{
		if(!enable)
		{
			/*
			int i;
			for (i=0; i<SIZEOF(Fpga.stat->pg_ctr); i++)
			{
				if (Fpga.stat->pg_ctr[i]!=Fpga.stat->print_done_ctr[i]) _Reload_FPGA=TRUE;		
			}
			*/
			{
				int i;
				int warn=FALSE;
				for(i=0; i<SIZEOF(Fpga.error->enc_fp); i++)
				{
					if(RX_FpgaData.wf_busy_warn[i] && !warn)				
					{
						Error(WARN,
							0,
							"Overspeed Counters: %d  %d  %d  %d", 
							RX_FpgaData.wf_busy_warn[0],
							RX_FpgaData.wf_busy_warn[1],
							RX_FpgaData.wf_busy_warn[2],
							RX_FpgaData.wf_busy_warn[3]);
						warn=TRUE;
					}
					
					if(RX_FpgaError.enc_fp[i].waveform_busy)				
					{
						Error(ERR_CONT,
							0,
							"Overspeed Counters: %d  %d  %d  %d", 
							RX_FpgaError.enc_fp[i].waveform_busy,
							RX_FpgaError.enc_fp[i].waveform_busy,
							RX_FpgaError.enc_fp[i].waveform_busy,
							RX_FpgaError.enc_fp[i].waveform_busy);
						break;
					}
				}				
			}
						
			nios_set_firepulse_on(FALSE);
			if(FpgaCfg.encoder->synth.enable)
			{
				FpgaCfg.encoder->cmd = 0;
				FpgaCfg.encoder->synth.value = 0;
				FpgaCfg.encoder->synth.enable = _SynthEncoder = FALSE;
				RX_FpgaEncCfg.cmd = FpgaCfg.encoder->cmd;
	//			rx_sleep(5);
			}
			if(FpgaCfg.cfg->cmd & CMD_MASTER_ENABLE)
			{	
//				Error(LOG,0,"fpga_disable: FSM State=0x%04x",Fpga.stat->info);
				//--- TEST PRINT-DONE --------------
				{
					int i,err;
					for(i=0,err=FALSE; i<HEAD_CNT; i++)
					{
						if(FpgaCfg.head[i]->cmd_enable && Fpga.stat->pg_ctr[i] > Fpga.stat->print_done_ctr[i]+1)
						{
							Error(WARN,0,"Head[%d]: Print-Done missing, #PrintGo=%d #Print-Done=%d",i,Fpga.stat->pg_ctr[i],Fpga.stat->print_done_ctr[i]); 
							err=TRUE;
						}
					}
					if(err) fpga_trace_registers("Print-Done-missed");	
				//	else     fpga_trace_registers("Print-Done-OK");	
				}
				//---
				TrPrintf(TRUE,"fpga_enable(false) CMD_MASTER_ENABLE=FALSE: blockCnt[0]=%d, imgInIdx[0]=%d, blockUsed=0x%08x",Fpga.data->blockCnt[0],Fpga.print->imgInIdx[0],Fpga.blockUsed[0]);

				_fpga_copy_status();
				term_save(PATH_TEMP "status.txt");
				term_flush();

				TrPrintfL(TRUE,"set CMD_MASTER_ENABLE=FALSE");				
				SET_FLAG(FpgaCfg.cfg->cmd,CMD_MASTER_ENABLE,FALSE);
			//	rx_sleep(5);
			//	fpga_set_config(INVALID_SOCKET);
			}

			if(_Reload_FPGA)
			{
				Error(WARN,0,"RELOAD-FPGA");
				putty_end();
				nios_end();
				fpga_end();
				fpga_init();
				nios_init();
				cond_init();
				putty_init();	
				_Reload_FPGA = FALSE;
			}

		}			
	}
	return REPLY_OK;
}

//--- fpga_manual_pg -----------------------------
void fpga_manual_pg(void)
{
	if (_Init)
	{
		if (arg_simu_machine)
		{
			_PgSimuIn++;
		}
		else
		{
			FpgaCfg.encoder->manual_printGo = 1;
			FpgaCfg.encoder->manual_printGo = 0;
			if (arg_test_fire)
				udp_test_send_block(4, 10000); // load while printing!
		}
	}
}



//--- fpga_nios_reset ------------------------------------
int   fpga_nios_reset(int reset)
{
	if (!_Init) 
		return REPLY_ERROR; //Error(ERR_CONT, 0, "fpga_nios_reset(%d): RBF not loaded", reset);
	SET_FLAG(FpgaCfg.cfg->cmd, CMD_NIOS_RESET, reset);
	rx_sleep(100);
	_PrintDoneError = 0;
	/*
	int tio;
	tio = 100;
	while(Fpga.stat->info.nios_reset != reset)
	{
		if (tio<=0) return Error(ERR_CONT, 0, "fpga_nios_reset(%d) timeout", reset);
		tio -= 10;
		rx_sleep(10);
	}
	*/

	return REPLY_OK;
}

//--- fpga_main ------------------------------------------
void  fpga_main(int ticks, int menu)
{
	static int led=0;
	int pd=0;

	if (!_Init) return;

	int time=rx_get_ticks();
	
	_check_errors();
	if (menu)
	{
		//--- LED -------------------------------
		// FpgaCfg.head[0]->cmd_led = led;
		FpgaCfg.cfg->led[2]=led;
		led = !led;

		
	//	_check_errors();
		_count_dots();
		
		//--- checks ------------------------------
		if (Fpga.stat->eth_ctr[0].frames_all==0xffff || Fpga.stat->eth_ctr[1].frames_all==0xffff) 
			FpgaCfg.cfg->err_reset = ERR_ETH;
			
		//--- user interface ------------------------------				
		_fpga_copy_status();

		//----- activate TCP/IP --------------
		if (_UdpIsLocal && RX_HBConfig.dataBlkSize==DATA_BLOCK_SIZE_STD && led)
		{
			SUDPDataBlockMsg msg;
			memset(&msg, 0, sizeof(msg));
			msg.blkNo  = 0xffffffff;
			udp_test_send(&msg, fpga_udp_block_size()+4);
		}
		if (_AliveChk_Timeout) _AliveChk_Timeout--;
	}
	
	int time1=rx_get_ticks()-time;
	
	_AliveCnt[0] = Fpga.stat->udp_alive[0];
	_AliveCnt[1] = Fpga.stat->udp_alive[1];

	_fpga_set_pg_offsets(_Direction);

	int time2=rx_get_ticks()-time;

	pd = _check_print_done();

	int time3=rx_get_ticks()-time;

	_handle_pd(pd);
	
	int time4=rx_get_ticks()-time;

	if (time4>200) Error(WARN, 0, "fpga_main(%d) t1=%d, t2=%d, t3=%d, t4=%d", menu, time1, time2, time3, time4);
}

//--- _check_print_done -----------------------------------------------------------------
static int _check_print_done(void)
{
	int		head;
	UINT32	pd;	// number of image printed by all heads
	int		time=rx_get_ticks();
		
	pd = (UINT32)-1;
	for (head=0; head<MAX_HEADS_BOARD; head++)
	{
		if (FpgaCfg.head[head]->cmd_enable)
		{			
			int time1=rx_get_ticks()-time;
			if (head==0 && RX_HBStatus[0].head[head].printGoCnt != Fpga.stat->pg_ctr[head])
			{
				int t=rx_get_ticks();
				TrPrintfL(TRUE, "Head[%d].PrintGo=%d time=%d", head, Fpga.stat->pg_ctr[head], t-_TestPgTime[head]);
				_TestPgTime[head]=t;
				/*
				if (Fpga.stat->pg_ctr[head] > Fpga.stat->print_done_ctr[head]+2)
				{
					Error(ERR_ABORT,0,"Head[%d]: Print-Done missing, #PrintGo=%d #Print-Done=%d",head,Fpga.stat->pg_ctr[head],Fpga.stat->print_done_ctr[head]); 
				}
				*/
			}
			int time2=rx_get_ticks()-time;
			
			RX_HBStatus[0].head[head].printGoCnt = Fpga.stat->pg_ctr[head];
			if(Fpga.stat->print_done_ctr[head]>RX_HBStatus[0].head[head].printDoneCnt) 
			{
				SFpgaImage	*img = &_Img[head][RX_HBStatus[0].head[head].printDoneCnt%MAX_PAGES];
				_BlockOutIdx[head] = _PageEnd[head][RX_HBStatus[0].head[head].printDoneCnt%MAX_PAGES];	
				RX_HBStatus[0].head[head].printDoneCnt++;
			//	if (img->clearBlockUsed) _check_block_used_flags_clear(head, RX_HBStatus[0].head[head].printDoneCnt, img->blkNo, img->blkCnt);
				_fpga_check_fp_errors();
				// TrPrintfL(TRUE, "Head[%d].PrintDone=%d", head, RX_HBStatus[0].head[head].printDoneCnt);
			}
			if (RX_HBStatus[0].head[head].printDoneCnt<pd) pd=RX_HBStatus[0].head[head].printDoneCnt;
			int time3=rx_get_ticks()-time;
						
			//--- test print-go/print-done for scanning machines 
//			if (FpgaCfg.head[2]->encoderNo!=0) // then it a textile machine
			{
				if (RX_HBStatus[0].head[head].printGoCnt > RX_HBStatus[0].head[head].printDoneCnt+2 && !(_PrintDoneError&(1<<head)))
				{
					_PrintDoneError |= (1<<head);
//					Error(ERR_ABORT, 0, "Head[%d]: Print-Done missing, #PrintGo=%d #Print-Done=%d", head, 
					if (ErrorFlag(ERR_ABORT, (UINT32*)&RX_HBStatus[0].err, err_fifo_full_0, 0, "Head[%d]: Print-Done missing, #PrintGo=%d #Print-Done=%d", head, 
						RX_HBStatus[0].head[head].printGoCnt, 
						RX_HBStatus[0].head[head].printDoneCnt))
					{
						_Reload_FPGA = TRUE;
						fpga_trace_registers("Print-Done-missed");															
					}
				}
			}
			int time4=rx_get_ticks()-time;
			if (time4>100) Error(WARN, 0, "_check_print_done[%d], t1=%d, t2=%d, t3=%d, t4=%d", head, time1, time2, time3, time4);
		}			
	}
	return pd;
}

//--- _handle_pd --------------------------------------------------------
static void _handle_pd(int pd)
{
	SPrintFileMsg	msg;

	while (pd!=(UINT32)-1 && pd>(int)_PdCnt)
	{
		//--- print done for all heads --------------
		msg.hdr.msgLen  = sizeof(msg);
		msg.hdr.msgId	= EVT_PRINT_FILE;

		memcpy(&msg.id, &_PageId[_PdCnt%MAX_PAGES], sizeof(msg.id));
		msg.evt			= DATA_PRINT_DONE;
		msg.bufReady	= 0;
		sok_send(&RX_MainSocket, &msg);
//		TrPrintfL(TRUE, "PRINT DONE:  id=%d, page=%d, copy=%d, scan=%d", msg.id.id, msg.id.page, msg.id.copy, msg.id.scan);
				
		if (_SynthEncoder)
		{
			if (_ImgInIdx > RX_HBStatus[0].head[0].printDoneCnt)
			{
				FpgaCfg.encoder->manual_printGo = 1;
				FpgaCfg.encoder->manual_printGo = 0;
			}
		}

		_PdCnt++;
	}
	
	if (FALSE)
	{
		//---  TEST Encoder-PG ----------------------------
		UINT32 flag = Fpga.stat->enc_tel[0]&ENC_PG_FLAG;
		if (flag != _PgTestFlag) 
		{
			int i=0;
			_PgTestFlag = flag;
			_PgTestCnt++;
		
			if (_PgTestCnt > Fpga.stat->pg_ctr[i]+1)
			{
				Error(ERR_CONT, 0, "Head[%d]: Encoder PrintGO=%d > Head PrintGO=%d", i, _PgTestCnt, Fpga.stat->pg_ctr[i]); 
			}
		}		
	}
}

//--- _check_errors ---------------------------------------------------------------------
static void _check_errors(void)
{
	int i, n, err;
	static INT32	_enc_tel_cnt=12345;
	static BYTE		_enc_crc[SIZEOF(Fpga.error->encoder)];
	static UINT16	_udp_too_long[2];
	
	RX_FpgaError.enc_tel_cnt = Fpga.error->enc_tel_cnt;
		
	for (i=0; i<2; i++)
	{
		if (Fpga.error->eth_fifo_full[i]) { _Reload_FPGA=TRUE; if(ErrorFlag (ERR_ABORT, (UINT32*)&RX_HBStatus[0].err,  err_fifo_full_0<<i,  0, "ETH[%d]: FIFO Full", i)) fpga_trace_registers("ETH-FIFO_FULL");}
		if (Fpga.error->udp_fifo_full[i]) { _Reload_FPGA=TRUE; if(ErrorFlag (ERR_ABORT, (UINT32*)&RX_HBStatus[0].err,  err_fifo_full_0<<i,  0, "UDP[%d]: FIFO Full", i)) fpga_trace_registers("UDP-FIFO_FULL");}
		if (RX_FpgaError.dup_error[i].too_long!=_udp_too_long[i])
		{
			if (RX_FpgaError.dup_error[i].too_long)
			{
				Error(WARN, 0, "UDP[%d] Too Long", i);
				fpga_trace_registers("UDP-TOO_LONG");				
			}
			_udp_too_long[i] = RX_FpgaError.dup_error[i].too_long;
		}
	}
	if (Fpga.error->udp_flush_fifo==0xffff) { _Reload_FPGA = TRUE; if(ErrorFlag(ERR_ABORT, (UINT32*)&RX_HBStatus[0].err, err_fifo_full_0, 0, "UDP[%d]: FIFO FLUSH Error", i)) fpga_trace_registers("UDP-FIFO_FLUSH"); }
	if (!fpga_is_ready()) return;	

	/*
	if (Fpga.stat->temp==0)
	{
		if (_TempErr<10 && ++_TempErr==10) ErrorFlag (ERR_CONT, (UINT32*)&RX_HBStatus[0].err,  err_overheated,  0, "Fpga Temperatur Sensor ERROR");
	}
	else
	*/
	{
		RX_HBStatus[0].tempFpga  = Fpga.stat->temp-128;
		if (RX_HBStatus[0].tempFpga>MAX_FPGA_TEMP)    
		{
			if (++_TempErr==100)
			{
				TrPrintfL(TRUE, "Head FPGA overheated temp=%d row=%d", RX_HBStatus[0].tempFpga, Fpga.stat->temp);
				if (ErrorFlag(ERR_ABORT, (UINT32*)&RX_HBStatus[0].err,  err_firepulse_missed_0, 0, "Head FPGA overheated (%d °C)", RX_HBStatus[0].tempFpga))
					nios_shutdown();				
			}
		}
		else 
		{
			_TempErr = 0;
			if(RX_HBStatus[0].tempFpga > (MAX_FPGA_TEMP - 5)) 
			{
				if(++_TempWarn == 100)	ErrorFlag(WARN, (UINT32*)&RX_HBStatus[0].err,  err_firepulse_missed_0, 0, "Head FPGA is getting too hot (%d °C)", RX_HBStatus[0].tempFpga / 1000);
			}
			else _TempWarn=0;
		}
	}
		
	/*
	for (i=0; i<1; i++)
	{
		set_err (udp_test_sent_alive(i)-Fpga.stat->udp_alive[i]>10, &RX_HBStatus[0].err, err_udp_alive_0+i, "UDP Alive missing");
	}
	*/
	for (i=0; i<MAX_HEADS_BOARD; i++)
	{		
		if (Fpga.error->head[i].write_img_line || Fpga.error->img_line_err[0][i] || Fpga.error->img_line_err[1][i] || Fpga.error->img_line_err[2][i] || Fpga.error->img_line_err[3][i])
		{                    				
			if (*((UINT32*)&RX_HBStatus[0].err) & (err_printgo_but_no_data_0<<i))
			{
				*((UINT32*)&RX_HBStatus[0].err) |= (err_printgo_but_no_data_0<<i);
					
				fpga_trace_registers("write_img_line");
				_trace_used_flags(i, _Img[0][0].blkNo, _Img[0][0].blkCnt, _PageEnd[0][0]);
					
				if (Fpga.error->head[i].write_img_line)   { Error(ERR_ABORT, 0, "Head[%d]: write-img-line.4: cnt=%d", i, Fpga.error->head[i].write_img_line);}
				if (Fpga.error->img_line_err[0][i])		  { Error(ERR_ABORT, 0, "Head[%d]: write-img-line.0: cnt=%d", i, Fpga.error->img_line_err[0][i]);}
				if (Fpga.error->img_line_err[1][i])		  { Error(ERR_ABORT, 0, "Head[%d]: write-img-line.1: cnt=%d", i, Fpga.error->img_line_err[1][i]);}
				if (Fpga.error->img_line_err[2][i])		  { Error(ERR_ABORT, 0, "Head[%d]: write-img-line.2: cnt=%d", i, Fpga.error->img_line_err[2][i]);}
				if (Fpga.error->img_line_err[3][i])		  { Error(ERR_ABORT, 0, "Head[%d]: write-img-line.3: cnt=%d", i, Fpga.error->img_line_err[3][i]);}
			}
		}
	}

	if (FpgaCfg.encoder->cmd & ENC_ENABLE)
	{
		if (_EncCheckDelay>0) _EncCheckDelay--;
		else
		{
			#ifdef DEBUG
				if (Fpga.error->enc_tel_cnt==_enc_tel_cnt) ErrorFlag (WARN,      (UINT32*)&RX_HBStatus[0].err,  err_encoder_not_conected,  0, "Encoder not connected");
			#else
				if (Fpga.error->enc_tel_cnt==_enc_tel_cnt) ErrorFlag (ERR_ABORT, (UINT32*)&RX_HBStatus[0].err,  err_encoder_not_conected,  0, "Encoder not connected");
			#endif
			for (i=0; i<SIZEOF(Fpga.error->encoder); i++)
			{
				if (Fpga.error->encoder[i].crc != _enc_crc[i])
				{
					if (Fpga.error->encoder[i].crc) ErrorFlag (WARN, (UINT32*)&RX_HBStatus[0].err,  err_encoder_not_conected,  0, "Encoder CRC ");
					_enc_crc[i] = Fpga.error->encoder[i].crc;				
				}
			}
		}
		_enc_tel_cnt = Fpga.error->enc_tel_cnt;		
	}
	else 
	{
		for (i=0; i<SIZEOF(Fpga.error->encoder); i++)_enc_crc[i] = Fpga.error->encoder[i].crc;
		_enc_tel_cnt++;
	}
}

//--- _check_state_machines --------------------
static void _check_state_machines(void)
{
	if ((Fpga.stat->info & 0x0fff) != 0x0fff) 
	{
		ErrorFlag (ERR_CONT,  (UINT32*)&RX_HBStatus[0].err,  err_udp1_alive,  0, "FPGA not all State Mashines running");
		_Reload_FPGA = TRUE;
	}
}

//--- _count_dots ------------------------------------------------------------------------
static void _count_dots(void)
{
	int i=0;
	static int _time=0;
	int			time, diff;
	UINT32		droplets;
	double		dropvolume = 2.4/1000000000.0;	// 2.4 pl = 1 droplet
	
	time = rx_get_ticks();
	diff = time-_time;
	for (i=0; i<MAX_HEADS_BOARD; i++) 
	{		
		droplets = Fpga.stat->head_dot_cnt[i];
		RX_HBStatus[0].head[i].dotCnt += droplets;
		cond_volume_printed(i, (int)(droplets*dropvolume*1000.0*1000.0/diff)); // [ul/s]
	}
	_time = time;
}

int fpga_enc_position(int no)	{return Fpga.stat->enc_position[no];}
int fpga_temp()					{return Fpga.stat->temp - 128;}
int fpga_qsys_id(void)			{return Fpga.qsys_id;}
int fpga_qsys_timestamp(void)	{return Fpga.qsys_timestamp;}
int fpga_fp_clock_mhz(void)		{return FPGA_FREQ/1000000;};
