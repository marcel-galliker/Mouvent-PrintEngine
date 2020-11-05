// ****************************************************************************
//
//	rx_def.h
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

#include "rx_ink_common.h"
#include "nios_def_fluid.h"
#include "cond_def_head.h"

#ifdef __cplusplus
extern "C"{
#endif

#pragma pack(1)

//--- initialisation ---------------------------------

void rx_def_init();
	
//--- File locatoions --------------------------------

#define PATH_ROOT_LX		"/opt/radex/"
#define PATH_BIN_LX			PATH_ROOT_LX "bin/"

#define PATH_ROOT_WIN		"D:/Radex/"
// #define PATH_BIN_WIN		PATH_ROOT_WIN "bin/"
#define PATH_SOURCE_DATA_DIR	"source-data/"
#define PATH_RIPPED_DATA_DIR	"ripped-data/"
#define PATH_RIPPED_DATA_DRIVE	"ripped-data:"
#define	PATH_EMBRIP_PRENV_DIR	"PrEnv/"

#ifdef WIN32
	#define PATH_ROOT			PATH_ROOT_WIN
	#define PATH_TRACE			PATH_ROOT "Trace/"
	#define PATH_TEMP			"D:/Temp/"
	#define PATH_HOME			"D:/radex/"
	#define PATH_SOURCE_DATA	"D:/" PATH_SOURCE_DATA_DIR
	#define PATH_RIPPED_DATA	"D:/" PATH_RIPPED_DATA_DIR
	#define PATH_EMBRIP_PRENV	"D:/" PATH_EMBRIP_PRENV_DIR
	#define PATH_FPGA_REGS		PATH_ROOT "fpga-regs"								
	#define PATH_USER			PATH_ROOT "user/"
	#define PATH_LOG			PATH_ROOT "log/"
#else
	#define PATH_ROOT			PATH_ROOT_LX
	#define PATH_HOME			"/home/radex/"
	#define PATH_TRACE			"/tmp/"
	#define PATH_TEMP			"/tmp/"
	#define PATH_USER			PATH_HOME "digiprint/"
	#define PATH_SOURCE_DATA	PATH_HOME PATH_SOURCE_DATA_DIR
	#define PATH_RIPPED_DATA	PATH_HOME PATH_RIPPED_DATA_DIR
	#define PATH_EMBRIP_PRENV	PATH_HOME PATH_EMBRIP_PRENV_DIR
	#define PATH_FPGA_REGS		PATH_HOME "fpga-regs/"				
	#define PATH_LOG			PATH_USER "log/"
#endif

#define PATH_BIN			PATH_ROOT "bin/"
#define PATH_BIN_MAIN		PATH_BIN  "main/"
#define PATH_BIN_SPOOLER	PATH_BIN  "spooler/"
#define PATH_BIN_HEAD		PATH_BIN  "head/"
#define PATH_BIN_FLUID		PATH_BIN  "fluid/"
#define PATH_BIN_ENCODER	PATH_BIN  "encoder/"
#define PATH_BIN_REXROTH	PATH_BIN  "rexroth/"
#define PATH_BIN_STEPPER	PATH_BIN  "stepper/"
#define PATH_BIN_WIN		PATH_BIN  "win/"

#define PATH_WAVE_FORM		PATH_ROOT "waveform/"

#define PATH_FONTS			PATH_TEMP "/Fonts/"

#define FIELNAME_HEAD_RBF		"rx_head_rbf"
#define FIELNAME_HEADTEMP_RBF	"rx_headtemp_rbf"
#define FIELNAME_HEAD_NIOS		"rx_head_nios"
#define FILENAME_HEAD_COND		"rx_head_cond"
#define FIELNAME_FLUID_RBF		"rx_fluid_rbf"
#define FIELNAME_FLUID_NIOS		"rx_fluid_nios"
#define FIELNAME_ENCODER_RBF	"rx_encoder_rbf"
#define FIELNAME_STEPPER_RBF	"rx_stepper_rbf"
#define FIELNAME_STEPPER_NIOS	"rx_stepper_nios"
#define FILENAME_SPOOLER_CTRL	"rx_spooler_ctrl"
#define FILENAME_LOG			"log.log"
#define FILENAME_NETWORK		"network.cfg"
#define FILENAME_PQ				"print_queue.xml"
#define FILENAME_CFG			"config.cfg"
#define FILENAME_FLUID_STATE	"fluid.xml"
#define FILENAME_HEADS_FLUSHED	"heads_flushed.xml"
#define FILENAME_PLC_CFG		"plc.cfg"
#define FILENAME_PLC_PAR		"plc.par"
#define FILENAME_MATERIAL		"material.xml"
#define FILENAME_MATERIAL_LIST	"materialList.xml"
#define FILENAME_SPLICE_PAR		"splicepar.xml"
#define FILENAME_HEAD_PRESOUT	"head_presout.xml"
#define FILENAME_COUNTERS		".counters.xml"

//--- defines ---------------------------------

#define INK_SUPPLY_CNT		24
#define MAX_HEADS_INKCYLINDER 12
#define ENC_CNT				2
#define FLUID_BOARD_CNT		4
#define FLUID_SCALE_SENSORS 3
#define INK_PER_BOARD		4
#define HEAD_CNT			4	// heads per head board
#define UDP_PORT_CNT		2
	
#define	MAX_HEADS_BOARD		4
#define MAX_GREY_LEVELS		8
#define MAX_DROP_SIZES		4
#define MAX_COLORS			16
#define MAX_SCALES			(MAX_COLORS+2)
#define MAX_HEADS_COLOR		48
#define MAX_HEAD_DIST		(INK_SUPPLY_CNT*MAX_HEADS_INKCYLINDER) // 288
#define MAX_DISABLED_JETS	32
#define MAX_DENSITY_VALUES	(2+10)
#define MAX_DENSITY_FACTORS	(2048+128)
	
#define HEAD_BOARD_CNT		(MAX_HEAD_DIST/MAX_HEADS_BOARD) // head boards per print bar

#define MAX_STEPPERS		4
#define MAX_STEPPER_MOTORS	5

#define IP_ADDR_SIZE		32

#define DPI_X				1200
#define DPI_Y				1200

#define MAX_DATA_SIZE		2048
#define MAX_TEST_DATA_SIZE	512

#define WAKEUP_BAR_LEN		128	// dots to wakeup lazy jets

//--- simple value ----------------------------------------------	
typedef struct SValue
{
	UINT32		no;
	INT32		value;
} SValue;

//--- device configuration --------------------
//--- devices -----------------------------------------------------------------
typedef enum EDevice
{
	dev_undef,	// 00
	dev_main,	// 01
	dev_gui,	// 02
	dev_plc,	// 03
	dev_enc,	// 04	
	dev_fluid,	// 05
	dev_6,		// 06
	dev_enc32,	// 07
	dev_8,		// 08
	dev_stepper,// 09
	dev_head,	// 10
	dev_spooler,// 11
	dev_end		// 13
} EDevice;
extern char *DeviceStr[];

typedef enum
{
	dev__undef,	// 00
	dev_off,	// 01
	dev_on,		// 02
	dev_simu	// 03	
} EPresent;

typedef enum ELogItemType
{
	LOG_TYPE_UNDEF,
	LOG_TYPE_LOG,
	LOG_TYPE_WARN,
	LOG_TYPE_ERROR_CONT,
	LOG_TYPE_ERROR_STOP,
	LOG_TYPE_ERROR_ABORT
} ELogItemType;
extern char *LogItemTypeStr[];

//--- ERectoVerso ------------------------------
typedef enum ERectoVerso
{
	rv_undef,	// 00
	rv_recto,	// 01
	rv_verso,	// 02
} ERectoVerso;

typedef struct SLogItem
{
	UINT8			deviceType;
	UINT8			deviceNo;
	UINT8			logType;	// ELogItemType
	UINT8			res1;
	UINT64			time;
	UINT32			errNo;
	UINT32			line;
	char			file[32];
	char			formatStr[256];
	BYTE			arg[256];
} SLogItem;

/*
typedef struct SVersion
{
	UINT32 major;		// 
	UINT32 minor;		// 
	UINT32 revision;	// SVN-Revision number 
	UINT32 build;		// incremented by each compilation
} SVersion;
*/
	
typedef struct SNetworkItem
{
	char	deviceTypeStr[32];
	char	serialNo[32];
	UINT64	macAddr;
	UCHAR	deviceType;
	INT8	deviceNo;
	char	ipAddr[32];
	UINT8	connected;
	UINT8	platform;	// EPlatform
	UINT32	rfsPort;
	UINT32	ports[8];
} SNetworkItem;

typedef struct SRxNetwork
{
	SNetworkItem item[64];
} SRxNetwork;

//--- SPageId -------------------------------
typedef struct SPageId
{
	INT32 id;
	INT32 page;
	INT32 copy;
	INT32 scan;
} SPageId;

//--- SPageNumber --------------------------------
typedef struct SPageNumber
{
	UINT32	enabled;
	UINT32	number;
	UINT32	fontSize;
	UINT32	imgDist;	// [µm]
} SPageNumber;

//--- print queue ----------------------------
typedef struct SPrintQueueItem
{
	SPageId	id;
	SPageId	start;

	char	filepath[256];
	char	preview [256];
	char	ripState[128];
	char	printEnv[64];
	char	material[64];
	char	testMessage[64];
	INT32	srcPages;
	INT32	srcWidth;	// µm
	INT32	srcHeight;	// µm
	UINT8	srcBitsPerPixel;
	INT32	firstPage;
	INT32	lastPage;
	INT8	singlePage;
	INT32	copies;
	INT8	collate;
	INT8	variable;	// variable data job
	UINT8	state;
			#define PQ_STATE_UNDEF		0
			#define PQ_STATE_QUEUED		1
			#define PQ_STATE_PREFLIGHT	2
			#define PQ_STATE_RIPPING	3
			#define PQ_STATE_SCREENING	4
			#define PQ_STATE_LOADING	5
			#define PQ_STATE_TRANSFER	6
			#define PQ_STATE_BUFFERED	7
			#define PQ_STATE_PRINTING	8
			#define PQ_STATE_PRINTED	9
			#define PQ_STATE_STOPPING	10	
			#define PQ_STATE_STOPPED	11

	UINT8	lengthUnit;
			#define PQ_LENGTH_UNDEF		0
			#define PQ_LENGTH_MM		1
			#define PQ_LENGTH_COPIES	2
			#define PQ_LENGTH_PIXELS	3
			
	UINT8	scanMode;
			#define PQ_SCAN_STD		0
			#define PQ_SCAN_RTL		1
			#define PQ_SCAN_BIDIR	2
	
	UINT8	orientation;
	UINT8	testImage;
			#define PQ_TEST_UNDEF			0
			#define PQ_TEST_ANGLE_OVERLAP	1
			#define PQ_TEST_ANGLE_SEPARATED	2
			#define PQ_TEST_JETS			3
			#define PQ_TEST_JET_NUMBERS		4
			#define PQ_TEST_GRID			5
			#define PQ_TEST_ENCODER			6
			#define PQ_TEST_SCANNING		7
			#define PQ_TEST_FULL_ALIGNMENT	8
			#define PQ_TEST_DENSITY			9

	INT32	pageWidth;	// µm
	INT32	pageHeight;	// µm
	INT32	pageMargin;	// µm
	INT32	printGoMode;
			#define PG_MODE_MARK		1	// print mark to PG (IN[0])
			#define PG_MODE_LENGTH		2	// distance between two GPs
			#define PG_MODE_GAP			3	// gap getween images
			#define PG_MODE_MARK_FILTER	4	// print mark to PG (IN[0])
			#define PG_MODE_MARK_VRT	5	// print mark to PG (IN[1])
			#define PG_MODE_MARK_INV	6	// print mark to PG (IN[0]/INVERSE)
	
	INT32	printGoDist;
	INT32	scanLength; // mm
	INT8	passes;
	INT8	virtualPasses;
	INT8	penetrationPasses;
	INT8	curingPasses;
	INT32	scans;
	INT32	speed;
	INT32	copiesTotal;
	INT32	copiesPrinted;
	INT32	scansSent;
	INT32	scansPrinted;
	INT32	scansStart;
	INT32	scansTotal;
	INT32	scansStop;
	INT32	progress;
	SPageNumber pageNumber;
	INT32	checks;

	char    dots[4];
	INT8	wakeup;

//	UINT16	previewOrientation;
} SPrintQueueItem;

// --- SColorConfig --------------------------------------------
typedef struct
{
	int			colorCode; // 0 to sizeof RX_ColorName
	char		name[64]; // colorname used in pdf i.e.: Cyan, Magenta, Yellow, Black, Blue, Orange, Green, Spot1, Micr ...
	char		shortName[8];	// file extension
	UINT8		rgbR;	// 0 to 0xFF used to simulated color in rgb for display
	UINT8		rgbG;	// "
	UINT8		rgbB;	// "
	int			technical; // color is technical type: 0: no, 1: yes
	int			penetrationFluid; // color is penetrationFluid type: 0: no, 1: yes
	int			inMaxInk; // color to be compute to calculate TotalMaxInk
	int			preserveBlack; // color for preserveBlack for APPE
	int			dieCutType; // color for dieCut for APPE
	int			mergeInBlack; // color to merge in black (plane 3)
	int			outRectoIdx; // outputpath for recto color plane idx. Default: 0
	int			outVersoIdx; // outputpath for recto color plane idx. Default: same as Recto
	int			DropSize[3];	// size of drop 1, 2 and 3 in femtoliter (10 exp-15)
} SColorConfig;

typedef struct SPoint
{
	INT32	x;
	INT32	y;
} SPoint;

#define MAX_OUTPATH 8
// --- SColorSpaceConfig --------------------------------------------
typedef struct
{
	char		name[64];
	int			duplex;		// 0:no, 1:yes
	SPoint		resol;
	int			count;
	INT64		maxWidth;
	SColorConfig color[MAX_COLORS];
	int			defDropSize[3];	// default size of drop 1, 2 and 3 in femtoliter (10 exp-15)
	int			outPathInit;
	int			countPath;
	char		outPath[MAX_OUTPATH][2 * MAX_PATH];
} SColorSpaceConfig;

//--- Colornames ------------------------------
typedef struct 
{
	int code;
	char name[32];
	char shortName[8];
}  SColorName;
	
//--- ColorName --------------------------------
extern SColorName RX_ColorName[55];
void RX_ColorNameInit (int inkSupplyNo, ERectoVerso	rectoVerso, char *fileName, int colorCode);
const char* RX_ColorNameLong (int code); 
const char* RX_ColorNameShort(int inkSupplyNo);

//--- SInkDefinition ------------------------------
typedef struct
{
	UINT16	pos;
	UINT16	volt;
} SWfPoint;
#define MAX_WF_POINTS	32
	
	// first pulse can start at this position
#define WF_FIRST_PULSE_POS	144 // 1.80 µs@160MHz
								// 2.06 µs@140MHz minimum 2.0 (140)
								
#define WF_FILLER	68			// delay of Waveform DAC OLD_OFF_OFFSET - OLD_WF_OFFSET

typedef struct SInkDefinition
{
	char	family[64];
	char	fileName[64];
	char	name[64];
	char	description[128];
	UINT32	colorRGB;
	UINT32	colorCode;
	INT32	temp;
	INT32	tempMax;
	INT32	tempChiller;
	INT32	dropletVolume;
	INT32	meniscus;
	INT32	viscosity;
	INT32	dropSpeed;
	INT32	printingSpeed[8];
	INT32	flushTime[3];
	INT32	maxSpeed[MAX_DROP_SIZES];
	UINT8	greyLevel[MAX_DROP_SIZES];
	SWfPoint wf[MAX_WF_POINTS];
} SInkDefinition;

typedef struct SInkLicense
{
	UINT16	machineID;
	UINT16	inkID;
	UINT16	licNo;
	UINT16	code_1;
	UINT32	volume;
	UINT16	code_2;
	UINT16	crc16;
} SInkLicense;
	
typedef struct SOffsetCfg
{
	INT32	angle;			// to adjust angle fault
	INT32	step;			// to adjust step fault
	INT32	incPerMeter[2];	// increments per meter
	INT32	versoDist;
	INT32	manualFlightTimeComp;
} SOffsetCfg;

int rx_def_is_scanning(EPrinterType printerType);
int rx_def_is_tx(EPrinterType printerType);
int rx_def_is_lb(EPrinterType printerType);
int rx_def_is_tts(EPrinterType printerType);
int rx_def_is_test(EPrinterType printerType);
int rx_def_use_pq(EPrinterType printerType);

int rx_printMode_is_test(int printMode);

typedef struct SPrinterCfg
{
	EPrinterType	type;

	UINT16	inkSupplyCnt;

	UINT32  jetEnabled0;
	UINT32	jetEnabledCnt;

	INT32	bmpOffsetUM;	// [µm]
	INT32	bmpHeightUM;	// [µm]
	INT32	bmpLengthUM;	// [µm]
	UINT8	forwardBack;
	UINT8	upwardDown;
	
	UINT32	overlap;
	
	SOffsetCfg offset;
} SPrinterCfg;

typedef enum
{
	ps_undef,		// 00
	ps_off,			// 01
	ps_ready_power, // 02
	ps_printing,	// 03
	ps_stopping,	// 04
    ps_goto_pause,  // 05
    ps_pause,       // 06
    ps_cleaning,    // 07
	ps_webin,		// 08
	ps_setup,		// 09
} EPrintState;

extern char *PrintStateStr[];

typedef struct SPrinterStatus
{
	EPrintState		printState;
	UINT8			error;

	//--- flags --------------------------
	union
	{
		struct
		{
		UINT32			cleaning:1;			// 0x0001
		UINT32			dataReady:1;		// 0x0002
		UINT32			splicing:1;			// 0x0004
		UINT32			inkSupilesOff:1;	// 0x0008
		UINT32			inkSupilesOn:1;		// 0x0010
		UINT32			testMode:1;			// 0x0020
		UINT32			externalData:1;		// 0x0040
		UINT32			txRobot:1;			// 0x0080
		UINT32			tempReady:1;		// 0x0100
		UINT32			lbRobot : 1;		// 0x0200
        UINT32			NeedDegasser : 1;   // 0x0400
        UINT32			door_open : 1;		// 0x0800
        };
		UINT32 flags;		
	};
	//--- flags end -------------------------
	
	UINT32			sentCnt;
	UINT32			transferredCnt;
	UINT32			printGoCnt;
	UINT32			printedCnt;
	UINT32			maxSpeed[MAX_DROP_SIZES];	// [m/min]
	UINT32			actSpeed;
	INT64			counterAct;	// [mm]
    INT64			counterTotal;	// [mm]
}  SPrinterStatus;

//--- SSpoolerCfg ----------------------------------
typedef struct SSpoolerCfg
{
	//--- data blocks per head board 
	INT32	dataBlkSize;
	INT32	dataBlkCntHead;

	INT32	printerType;	// EPrinterType
	INT32	overlap;

	//--- nozzles per printhead ---
	INT32	headWidthPx;
	INT32	headOverlapPx;
	INT32	barWidthPx;
	INT32	maxOffsetPx;

	//--- nozzles per color ---
	INT32   barStartPx;		// printbar first pixel		
	INT32   barEndPx;		// printbar last pixel

	INT32	colorCnt;
	INT32	headsPerColor;
	INT32	headsPerBoard;
	UINT32	resetCnt;

	//--- network -----------------------------
	char	dataRoot[MAX_PATH];	// network path

	//--- first printhead per color ---
	// INT32	firstHead[MAX_COLORS];
	INT16	headNo	   [MAX_COLORS][MAX_HEADS_COLOR];
//	INT8	board	   [MAX_COLORS][MAX_HEADS_COLOR];
//	INT8	headOnBoard[MAX_COLORS][MAX_HEADS_COLOR];
//	INT32	blockNo0   [MAX_HEADS_BOARD];
} SSpoolerCfg;

typedef struct SConditionerCfg
{
	INT32	meniscus_setpoint;
	INT32	headsPerColor;
} SConditionerCfg;
	
//--- print head ---------------------------- 
typedef struct SHeadCfg
{
	UINT8	inkSupply;
	UINT8	_headNo_unused;	// within color
	UINT8	enabled;
	UINT8	encoderNo;
	INT32	dist;	// [µm] distance correction inside print bar 
	INT32	distBack;	// [µm] distance correction inside print bar
	INT32	headHeight;	// [µm] height of printhead above substrate
	UINT16	jetEnabled0;
	UINT16  jetEnabledCnt;
//	UINT32	bmpJetOffset;
//	UINT32  bmpJetCnt;
	
	UINT32	blkNo0;
	UINT32	blkCnt;
	
	SConditionerCfg cond;

	char	name[8];
} SHeadCfg;

typedef struct SHeadInfo
{
	UINT32 connected : 1;
	UINT32 flushed   : 1;
	UINT32 meniscus : 1;
    UINT32 info_3 : 1;
    UINT32 info_4 : 1;
    UINT32 info_5 : 1;
	UINT32 info_6 : 1;
	UINT32 info_7 : 1;
	UINT32 info_8 : 1;
	UINT32 info_9 : 1;
	UINT32 info10 : 1;
	UINT32 info11 : 1;
	UINT32 info12 : 1;
	UINT32 info13 : 1;
	UINT32 info14 : 1;
	UINT32 info15 : 1;
	UINT32 info16 : 1;
	UINT32 info17 : 1;
	UINT32 info18 : 1;
	UINT32 info19 : 1;
	UINT32 info20 : 1;
	UINT32 info21 : 1;
	UINT32 info22 : 1;
	UINT32 info23 : 1;
	UINT32 info24 : 1;
	UINT32 info25 : 1;
	UINT32 info26 : 1;
	UINT32 info27 : 1;
	UINT32 info28 : 1;
	UINT32 info29 : 1;
	UINT32 info30 : 1;
	UINT32 info31 : 1;
} SHeadInfo;

typedef enum
{
	info_is_shutdown			= 0x00000001,
	info_wf_ready				= 0x00000002,
	info_nios_ready				= 0x00000004,
	info_watchdog_dbg_mode      = 0x00000008,
	info_watchdog_catch_fp_err	= 0x00000010,
	info_cooler_pcb_present		= 0x00000020,
	info_6           = 0x00000040,
	info_7           = 0x00000080,
	info_8           = 0x00000100,
	info_9           = 0x00000200,
	info10           = 0x00000400,
	info11           = 0x00000800,
	info12           = 0x00001000,
	info13           = 0x00002000,
	info14           = 0x00004000,
	info15           = 0x00008000,
	info16           = 0x00010000,
	info17           = 0x00020000,
	info18           = 0x00040000,
	info19           = 0x00080000,
	info20           = 0x00100000,
	info21           = 0x00200000,
	info22           = 0x00400000,
	info23           = 0x00800000,
	info24           = 0x01000000,
	info25           = 0x02000000,
	info26           = 0x04000000,
	info27           = 0x08000000,
	info28           = 0x10000000,
	info29           = 0x20000000,
	info30           = 0x40000000,
	info31           = 0x80000000,
} EHeadinfo;
	
typedef enum
{
	warn_fpga_overheating = 0x00000001,	// head is overheating
	warn_flow_factor	  = 0x00000002,
	warn_2			 = 0x00000004,
	warn_3			 = 0x00000008,
	warn_4			 = 0x00000010,
	warn_5			 = 0x00000020,
	warn_6			 = 0x00000040,
	warn_7			 = 0x00000080,
	warn_8			 = 0x00000100,
	warn_9			 = 0x00000200,
	warn10			 = 0x00000400,
	warn11			 = 0x00000800,
	warn12			 = 0x00001000,
	warn13			 = 0x00002000,
	warn14			 = 0x00004000,
	warn15			 = 0x00008000,
	warn16			 = 0x00010000,
	warn17			 = 0x00020000,
	warn18			 = 0x00040000,
	warn19			 = 0x00080000,
	warn20			 = 0x00100000,
	warn21			 = 0x00200000,
	warn22			 = 0x00400000,
	warn23			 = 0x00800000,
	warn24			 = 0x01000000,
	warn25			 = 0x02000000,
	warn26			 = 0x04000000,
	warn27			 = 0x08000000,
	warn28			 = 0x10000000,
	warn29			 = 0x20000000,
	warn30			 = 0x40000000,
	warn31			 = 0x80000000,
} EHeadWarn;

typedef enum EHeadNiosErr
{
	err_nios_incompatible		= 0x00000001,
	err_fpga_overheated			= 0x00000002,
	err_cooler_overheated		= 0x00000004,
	err_cooler_overpressure		= 0x00000008,
	err_encoder_not_conected	= 0x00000010,
	err_fpga_state_machines		= 0x00000020,
	err_not_in_print			= 0x00000040,
	err_PG_0					= 0x00000080,
	err_PG_1					= 0x00000100,
	err_PG_2					= 0x00000200,
	err_PG_3					= 0x00000400,
	err_36volt					= 0x00000800,
	err_firepulse_missed_0		= 0x00001000,
	err_firepulse_missed_1		= 0x00002000,
	err_firepulse_missed_2		= 0x00004000,
	err_firepulse_missed_3		= 0x00008000,
	err_2_5volt					= 0x00010000,
	err_17						= 0x00020000,
	err_fifo_full_0				= 0x00040000,
	err_fifo_full_1				= 0x00080000,
	err_tcpip_overflow			= 0x00100000,
	err_therm_cooler		    = 0x00200000,
	err_22						= 0x00400000,
	err_23						= 0x00800000,
	err_amc7891					= 0x01000000,
	err_3_3_volt				= 0x02000000,
	err_5volt					= 0x04000000,
	err_min_5volt				= 0x08000000,
	err_pwr_all_off				= 0x10000000,
	err_amp_all_on				= 0x20000000,
	err_pwr_all_on				= 0x40000000,
	err_31						= 0x80000000,
} EHeadNiosErr;

	
typedef struct
{
	//--- fuji eeprom ---------------------------------
	UINT32			serialNo;
	UINT8			week;
	UINT8			year;
	UINT8			flowResistance;
	UINT8			straightness;
	INT16			uniformity;
	UINT16			voltage;
	UINT16			dropSize[3];
	UINT16			badJets[8];
} SHeadEEpromInfo;

typedef struct
{
	UINT16	clusterNo;			//	0x00
	UINT16	flowResistance;		//	0x02
	UINT8	flowResistanceCRC;	//	0x04
	UINT32	dropletsPrinted_old;	//  0x05..0x08
	UINT8	dropletsPrintedCRC_old;	//	0x09
	INT16	disabledJets[MAX_DISABLED_JETS];	// 0x0a..0x4b
	UINT8	disabledJetsCRC;					//	0x4a
	INT16	densityValue[MAX_DENSITY_VALUES];	// 0x4b..0x62
	UINT8	densityValueCRC;					// 0x63
	UINT8	voltage;							// 0x64: Firepulse voltage
	UINT8	voltageCRC;							// 0x65
	UINT64	dropletsPrinted;					// 0x66..0x6d
	UINT8	dropletsPrintedCRC;					// 0x6e
	INT16	rob_angle;							// 0x6f..0x70	// 0=undef
	INT16	rob_dist;							// 0x71..0x72	// 0=undef
	UINT8	rob_CRC;							// 0x73
	UINT8	res_74[0x80-0x74];	
} SHeadEEpromMvt;	// size must be 0x80!!
	
typedef struct SHeadStat
{	
	SCondInfo		info;
	EHeadWarn		warn;
	UINT32			err;	// conditioner --> cond_def_head.h

	//-- job info ------------------------------------
	UINT64	dotCnt;	// printed drops since last reset
	UINT32	imgInCnt;
	UINT32	imgBuf;
	UINT32	encPos;
	UINT32	encPgCnt;
	UINT32	printGoCnt;
	UINT32	printDoneCnt;

	//--- ink system ---------------------------------
	UINT32			tempHead;
	UINT32			tempCond;
	UINT32			tempSetpoint;
	INT32			presIn;
	INT32			presIn_max;
	INT32			presIn_diff;
	INT32			flowFactor;
	INT32			presOut;
	INT32			presOut_diff;
	INT32			meniscus;
	INT32			meniscus_diff;
	INT32			meniscus_Setpoint;
	INT32			pid_offset;

	FLOAT			dropVolume;	// in pl
	UINT32			pumpSpeed;
	UINT32			pumpFeedback;
	UINT32			printingSeconds;
	UINT64			printedDroplets;
	UINT64			printed_ml;

	INT32			presIn_0out;
	EnFluidCtrlMode	ctrlMode;
	
	SHeadEEpromInfo	eeprom;
	SHeadEEpromMvt	eeprom_mvt;
} SHeadStat;

typedef struct SHeadBoardCfg
{
	UINT16		no;
	EPresent	present;
	EPrinterType printerType;

	// tcp/ip adresses
	UINT32		ctrlAddr;					// TCP (on ARM)
	UINT16		ctrlPort;					// TCP (on ARM)
	UINT32		dataAddr[UDP_PORT_CNT];		// UDP (on FPGA)
	UINT16		dataPort[UDP_PORT_CNT];		// UDP (on FPGA)
	INT16		dataBlkSize;
	INT32		dataBlkCntHead;
	SHeadCfg	head[HEAD_CNT];
	UINT16		reverseHeadOrder;
	UINT16		spoolerNo;
	UINT32		machineMeters;
	UINT32		simuPlc;
	UINT32		resetCnt;
	UINT32		debug;
} SHeadBoardCfg;

typedef struct SHeadBoardStat
{
	UINT32		boardNo;

	//--- rom values, stored in head board ----
	UINT64		macAddr;
	UINT64		serialNo;
	SVersion	hwVersion;
	SVersion	swVersion;
	SVersion	fpgaVersion;
	SVersion	niosVersion;

	//--- values stored in head-board (no reset) ---------
	UINT32		clusterNo;
	UINT32		clusterTime;
	INT32		tempFpga;
	UINT32		flow;
    UINT32		cooler_temp;

    //--- warnings/errors ----------------
	SHeadInfo		info;
	EHeadWarn		warn;
	EHeadNiosErr	err;

	INT16			headCnt;
	SHeadStat		head[HEAD_CNT];

	//--- job info -------------------------
	UINT32			encoderPos;
} SHeadBoardStat;

//--- SEncoderCfg --------------------------------
typedef struct SEncoderCfg
{
	INT32	simulation;
	INT32	restart;
	INT32	pos_actual;
	INT32	pos_pg_fwd;
	INT32	pos_pg_bwd;
	INT32	incPerMeter;
	INT32	diameter[2];
	INT32	orientation;
	INT32	scanning;
	INT32	correction;
			#define CORR_OFF		0x00
			#define CORR_ENCODER	0x01	// 32 values
			#define CORR_ROTATIVE	0x02	// 
			#define CORR_LINEAR		0x04	// using two encoders (scanning)
	INT32	printGoMode;
	INT32	printGoDist;					// in microns
	INT32	printGoOutDist;					// in microns
	INT32	speed_mmin;
	INT32	corrRotPar[4];					// parameters for CORR_ROTATIVE
	INT32	ftc;							// Flight Time Compensation
	EPrinterType	printerType;
} SEncoderCfg;
	
typedef struct SEncoderPgDist
{
	SPageId	id;
	
	// all distances in microns
	// all values relative to last Print-Go
	INT32	printGoMode;
	INT32	cnt;		// number of print-Gos
	INT32	ignore;	// length from PG until the sensor window is opened  (0=unused)
	INT32	window;	// length of the sensor window (0=unused)
	INT32	dist;	// distace to the next Print-Go (0=unused)
} SEncoderPgDist;
	
typedef struct SEncoderInfo
{
	UINT32 connected :		1;	// 0x00000001
	UINT32 uv_on	 :		1;	// 0x00000002
	UINT32 uv_ready  :		1;	// 0x00000004
	UINT32 analog_encoder : 1;	// 0x00000008
	UINT32 can_start :		1;	// 0x00000010
	UINT32 backwards : 1;		// 0x00000020
	UINT32 info_6 : 1;			// 0x00000040
	UINT32 info_7 : 1;			// 0x00000080
	UINT32 info_8 : 1;
	UINT32 info_9 : 1;
	UINT32 info10 : 1;
	UINT32 info11 : 1;
	UINT32 info12 : 1;
	UINT32 info13 : 1;
	UINT32 info14 : 1;
	UINT32 info15 : 1;
	UINT32 info16 : 1;
	UINT32 info17 : 1;
	UINT32 info18 : 1;
	UINT32 info19 : 1;
	UINT32 info20 : 1;
	UINT32 info21 : 1;
	UINT32 info22 : 1;
	UINT32 info23 : 1;
	UINT32 info24 : 1;
	UINT32 info25 : 1;
	UINT32 info26 : 1;
	UINT32 info27 : 1;
	UINT32 info28 : 1;
	UINT32 info29 : 1;
	UINT32 info30 : 1;
	UINT32 info31 : 1;
} SEncoderInfo;

typedef struct SEncoderWarn
{
	UINT32 warn_0 : 1;
	UINT32 warn_1 : 1;
	UINT32 warn_2 : 1;
	UINT32 warn_3 : 1;
	UINT32 warn_4 : 1;
	UINT32 warn_5 : 1;
	UINT32 warn_6 : 1;
	UINT32 warn_7 : 1;
	UINT32 warn_8 : 1;
	UINT32 warn_9 : 1;
	UINT32 warn10 : 1;
	UINT32 warn11 : 1;
	UINT32 warn12 : 1;
	UINT32 warn13 : 1;
	UINT32 warn14 : 1;
	UINT32 warn15 : 1;
	UINT32 warn16 : 1;
	UINT32 warn17 : 1;
	UINT32 warn18 : 1;
	UINT32 warn19 : 1;
	UINT32 warn20 : 1;
	UINT32 warn21 : 1;
	UINT32 warn22 : 1;
	UINT32 warn23 : 1;
	UINT32 warn24 : 1;
	UINT32 warn25 : 1;
	UINT32 warn26 : 1;
	UINT32 warn27 : 1;
	UINT32 warn28 : 1;
	UINT32 warn29 : 1;
	UINT32 warn30 : 1;
	UINT32 warn31 : 1;
} SEncoderWarn;

typedef struct SEncoderErr
{
	UINT32 err_0 : 1;
	UINT32 err_1 : 1;
	UINT32 err_2 : 1;
	UINT32 err_3 : 1;
	UINT32 err_4 : 1;
	UINT32 err_5 : 1;
	UINT32 err_6 : 1;
	UINT32 err_7 : 1;
	UINT32 err_8 : 1;
	UINT32 err_9 : 1;
	UINT32 err10 : 1;
	UINT32 err11 : 1;
	UINT32 err12 : 1;
	UINT32 err13 : 1;
	UINT32 err14 : 1;
	UINT32 err15 : 1;
	UINT32 err16 : 1;
	UINT32 err17 : 1;
	UINT32 err18 : 1;
	UINT32 err19 : 1;
	UINT32 err20 : 1;
	UINT32 err21 : 1;
	UINT32 err22 : 1;
	UINT32 err23 : 1;
	UINT32 err24 : 1;
	UINT32 err25 : 1;
	UINT32 err26 : 1;
	UINT32 err27 : 1;
	UINT32 err28 : 1;
	UINT32 err29 : 1;
	UINT32 err30 : 1;
	UINT32 err31 : 1;
} SEncoderErr;

//--- SEncoderStat --------------------------------
typedef struct SEncoderStat
{
	SEncoderInfo	info;
	SEncoderWarn	warn;
	SEncoderErr		err;
	UINT32			distTelCnt;
	UINT32			PG_cnt;
	UINT32			PG_stop;
	UINT32			fifoEmpty_PG;
	UINT32			fifoEmpty_IGN;
	UINT32			fifoEmpty_WND;
	INT32			corrRotPar[4];	// parameters for CORR_LINEAR
	INT32			ampl_old;
	INT32			ampl_new;
	INT32			percentage;
	UINT32			meters;
	INT32			speed;	// [m/min]
} SEncoderStat;

//---  ink-supply Configuration ----------------------------
typedef struct SInkSupplyCfg
{
    int				no;
	char			inkFileName[64];
	SInkDefinition	ink;
	INT32			cylinderPresSet;
	INT32			meniscusSet;
	ERectoVerso		rectoVerso;
	char			scannerSN[16];
} SInkSupplyCfg;
	
typedef struct SFluidBoardCfg
{		
	EPrinterType	printerType;
	UINT32			lung_enabled;
	UINT32			headsPerColor;
} SFluidBoardCfg;
	
typedef struct SHeadStateLight
{
	INT32			temp;
	INT32			condPumpSpeed;			
	INT32			condPumpFeedback;
	INT32			condPresIn;
	INT32			condPresOut;
	INT32			condMeniscus;
	INT32			condMeniscusDiff;
	INT32			condTempReady;
	INT32			condFlowfactor_ok;
	INT32			canisterEmpty;
} SHeadStateLight;
			
typedef struct SFluidStateLight
{
	INT32			cylinderPressure;
	INT32			cylinderPressureSet;
	INT32			inkPump;
	INT32			inkPumpFeedback;
	INT32			amcTemp;
	INT32			fluidErr;
	UINT32			machineMeters;
} SFluidStateLight;
	
typedef struct SInkSupplyInfo
{
	union
	{
		UINT32 val;
		struct
		{
			UINT32 connected : 1;		// 0x00000001
			UINT32 bleedValve : 1;		// 0x00000002
			UINT32 cusionValve : 1;		// 0x00000004
			UINT32 flushed : 1;			// 0x00000008
			UINT32 condTempReady : 1;	// 0x00000010
			UINT32 heaterTempReady : 1;	// 0x00000020
			UINT32 cond_flowFactor_ok : 1;// 0x00000040
			UINT32 info_7 : 1;			// 0x00000080
			UINT32 info_8 : 1;			// 0x00000100
			UINT32 info_9 : 1;			// 0x00000200
			UINT32 info10 : 1;			// 0x00000400
			UINT32 info11 : 1;			// 0x00000800
			UINT32 info12 : 1;			// 0x00001000
			UINT32 info13 : 1;			// 0x00002000
			UINT32 info14 : 1;			// 0x00004000
			UINT32 info15 : 1;			// 0x00008000
			UINT32 info16 : 1;			// 0x00010000
			UINT32 info17 : 1;			// 0x00020000
			UINT32 info18 : 1;			// 0x00040000
			UINT32 info19 : 1;			// 0x00080000
			UINT32 info20 : 1;			// 0x00100000
			UINT32 info21 : 1;			// 0x00200000
			UINT32 info22 : 1;			// 0x00400000
			UINT32 info23 : 1;			// 0x00800000
			UINT32 info24 : 1;			// 0x01000000
			UINT32 info25 : 1;			// 0x02000000
			UINT32 info26 : 1;			// 0x04000000
			UINT32 info27 : 1;			// 0x08000000
			UINT32 info28 : 1;			// 0x10000000
			UINT32 info29 : 1;			// 0x20000000
			UINT32 info30 : 1;			// 0x40000000
			UINT32 info31 : 1;			// 0x80000000
		};
	};
} SInkSupplyInfo;

typedef struct SInkSupplyWarn
{
	union
	{
		UINT32 val;
		struct
		{
			UINT32 inkLow : 1;
			UINT32 licLow : 1;	// lisence is running out
			UINT32 warn_2 : 1;
			UINT32 warn_3 : 1;
			UINT32 warn_4 : 1;
			UINT32 warn_5 : 1;
			UINT32 warn_6 : 1;
			UINT32 warn_7 : 1;
			UINT32 warn_8 : 1;
			UINT32 warn_9 : 1;
			UINT32 warn10 : 1;
			UINT32 warn11 : 1;
			UINT32 warn12 : 1;
			UINT32 warn13 : 1;
			UINT32 warn14 : 1;
			UINT32 warn15 : 1;
			UINT32 warn16 : 1;
			UINT32 warn17 : 1;
			UINT32 warn18 : 1;
			UINT32 warn19 : 1;
			UINT32 warn20 : 1;
			UINT32 warn21 : 1;
			UINT32 warn22 : 1;
			UINT32 warn23 : 1;
			UINT32 warn24 : 1;
			UINT32 warn25 : 1;
			UINT32 warn26 : 1;
			UINT32 warn27 : 1;
			UINT32 warn28 : 1;
			UINT32 warn29 : 1;
			UINT32 warn30 : 1;
			UINT32 warn31 : 1;
		};
	};
} SInkSupplyWarn;

typedef struct SInkSupplyStat
{
	//--- warnings/errors ----------------
	SInkSupplyInfo	info;
	SInkSupplyWarn	warn;
	UINT32			err;		// ENiosFluidErr
	
	INT32	cylinderPresSet;	//  Pressure intermediate Tank Set
	INT32	cylinderPres;		//  Pressure intermediate Tank
	INT32	cylinderSetpoint;		//  Pressure intermediate Tank
	INT32	airPressureTime;
	INT32	flushTime;
	INT32	purge_putty_ON;
	INT32   presLung;			//  Lung pressure
	INT32	condPresOut;	
	INT32	condPresIn;  
	INT32   condPumpSpeed;
	INT32	condTemp;
	INT32	temp;				//	Temperature
	INT32	pumpSpeedSet;		//	Consumption pump speed
	INT32	pumpSpeed;			//	Consumption pump speed measured
	INT32	canisterLevel;
	INT32	canisterErr;
    INT32	flush_pump_val;
    char	scannerSN[16];
	char	barcode[128];
	EnFluidCtrlMode	ctrlMode;	//	EnFluidCtrlMode
} SInkSupplyStat;

typedef struct SFluidBoardStat
{	
	UINT32  err;	// SNiosFluidErr
	SInkSupplyStat stat[INK_PER_BOARD];
} SFluidBoardStat;

//--- Chiller ---------------------------
	
typedef struct SChillerStat
{
	INT32	enabled;
	INT32	running;
	INT32	tempSet;
	INT32	temp;
	INT32	pressure;
	INT32	resistivity;
	UINT32	status;
} SChillerStat; 
	
typedef struct SDriveStat
{
	INT32	enabled;
	INT32	speed;
	INT32	targetposition;
    INT32	acceleration;
    INT32	decelerlation;
    INT32	jerk;
    INT32	actualpoint;
} SDriveStat;
	
typedef struct SRobotOffsets
{
	INT32			ref_height;
	INT32			head_align;
	INT32			ref_height_back;
	INT32			ref_height_front;
	INT32			cap_height;
} SRobotOffsets;
	
typedef enum ERobotFunctions
{
	rob_fct_cap,			//  0: Capping
	rob_fct_wash,			//  1: Wash
	rob_fct_vacuum,			//  2: Vacuum
	rob_fct_wipe,			//  3: Wiping
	rob_fct_vacuum_change,	//  4: Vaccum change
	rob_fct_tilt,			//  5: Tilt for Capping
	rob_fct_vacuum_all,		//  6: Vacuum all heads
	rob_fct_purge_all,		//  7: Purge all heads
	rob_fct_purge_head0,	//  8: Purge head 0
	rob_fct_purge_head1,	//  9: Purge head 1
	rob_fct_purge_head2,	// 10: Purge head 2
	rob_fct_purge_head3,	// 11: Purge head 3
	rob_fct_purge_head4,	// 12: Purge head 4
	rob_fct_purge_head5,	// 13: Purge head 5
	rob_fct_purge_head6,	// 14: Purge head 6
	rob_fct_purge_head7,	// 15: Purge head 7

} ERobotFunctions;
	
typedef enum ERobotVaccumState
{
	rob_vacuum_1_to_4,
	rob_vacuum_5_to_8,
	rob_vacuum_all,
} ERobotVacuumState;
	
//--- Stepper Board --------------------
typedef struct SStepperCfg
{
	//	---> REFERENCE --------------------
	//       printing   
	//		 "Belt"		== 0
	//		 wiping
	//		 capping	
	
	EPrinterType	printerType;
	INT32			boardNo;
			#define		step_lift	0
			#define		step_clean	1
	INT32			ref_height;			// distance "Belt" to reference position in µm: "0" is at "Belt" Level
	INT32			print_height;		// in µm
	INT32			wipe_height;		// in µm
	INT32			wipe_delay;			// in ms
	INT32			wipe_speed;			// in mm/s
	INT32			cap_height;			// in µm
	INT32			cap_pos;
	INT32			adjust_pos;
	INT32			use_printhead_en;	// if true use PRINTHEAD_EN to allow head going down
	INT32			material_thickness;
	
	SRobotOffsets	robot[4];
} SStepperCfg;
	
typedef struct SStepperMotorTest
{
	INT32	boardNo;
	INT32	motorNo;
	INT32	microns;
} SStepperMotorTest;

//--- STestTableScanPar ---------------------------------
typedef struct
{
	INT32		speed;
	INT32		scanCnt;
	INT32		scanMode;
	INT32		yStep;
	INT32		offsetAngle;
	INT32		curingPasses;
} STestTableScanPar;

typedef struct 
{
    INT32	inkSupplyNo;
    INT32   headNo;
    INT32   angle;	// in µm
    INT32   stitch; // in µm
} SHeadAdjustment;

typedef struct SScrewAdjustment
{
	INT32	nr;
	INT32   rot;	// in rotations
	INT32   bar;
} SScrewAdjustment;
	

	//--- check also GUI: RX_DigiPrint.Models.TestTableStatus.Update
typedef struct ETestTableInfo
{
	UINT32 ref_done			: 1;	//	0x00000001
	UINT32 moving			: 1;	//	0x00000002
	UINT32 uv_on			: 1;	//	0x00000004
	UINT32 uv_ready			: 1;	//	0x00000008
	UINT32 z_in_ref			: 1;	//	0x00000010
	UINT32 z_in_print		: 1;	//	0x00000020
	UINT32 z_in_cap			: 1;	//	0x00000040
	UINT32 z_in_up			: 1;	//	0x00000080
	UINT32 x_in_cap			: 1;	//	0x00000100
	UINT32 x_in_ref			: 1;	//	0x00000200
	UINT32 printing			: 1;	//	0x00000400
	UINT32 curing			: 1;	//	0x00000800
	UINT32 z_in_jet			: 1;	//	0x00001000
	UINT32 info_13			: 1;	//	0x00002000
	UINT32 info_14			: 1;	//	0x00004000
	UINT32 info_15			: 1;	//	0x00008000
	UINT32 info_16			: 1;	//	0x00010000
	UINT32 info_17			: 1;	//	0x00020000
	UINT32 headUpInput_0	: 1;	//	0x00040000
	UINT32 headUpInput_1	: 1;	//	0x00080000
	UINT32 headUpInput_2	: 1;	//	0x00100000
	UINT32 headUpInput_3	: 1;	//	0x00200000
	UINT32 info_22			: 1;	//	0x00400000
	UINT32 info_23			: 1;	//	0x00800000
	UINT32 scannerEnable	: 1;	//	0x01000000
	UINT32 info_25			: 1;	//	0x02000000
	UINT32 printhead_en		: 1;    //  0x04000000
	UINT32 splicing			: 1;	//  0x08000000
	UINT32 DripPans_InfeedUP			: 1;	//  0x10000000
	UINT32 DripPans_InfeedDOWN			: 1;	//	0x20000000
	UINT32 DripPans_OutfeedUP			: 1;	//	0x40000000
	UINT32 DripPans_OutfeedDOWN			: 1;	//	0x80000000
} ETestTableInfo;
	
typedef struct EInkPumpInfo
{
    UINT32 waste_valve_0 : 1;		 //	0x00000001
    UINT32 waste_valve_1 : 1;        //	0x00000002
    UINT32 waste_valve_2 : 1;        //	0x00000004
    UINT32 waste_valve_3 : 1;		 //	0x00000008
    UINT32 ipa_valve_0 : 1;			 //	0x00000010
    UINT32 ipa_valve_1 : 1;			 //	0x00000020
    UINT32 ipa_valve_2 : 1;          //	0x00000040
    UINT32 ipa_valve_3 : 1;          //	0x00000080
    UINT32 xl_valve_0 : 1;           //	0x00000100
    UINT32 xl_valve_1 : 1;           //	0x00000200
    UINT32 xl_valve_2 : 1;           //	0x00000400
    UINT32 xl_valve_3 : 1;           //	0x00000800
    UINT32 flush_valve_0 : 1;        //	0x00001000
    UINT32 flush_valve_1 : 1;        //	0x00002000
    UINT32 flush_valve_2 : 1;        //	0x00004000
    UINT32 flush_valve_3 : 1;        //	0x00008000
    UINT32 info_16 : 1;              //	0x00010000
    UINT32 info_17 : 1;              //	0x00020000
    UINT32 info_18 : 1;				 //	0x00040000
    UINT32 info_19 : 1;				 //	0x00080000
    UINT32 info_20 : 1;				 //	0x00100000
    UINT32 info_21 : 1;				 //	0x00200000
    UINT32 info_22 : 1;              //	0x00400000
    UINT32 info_23 : 1;              //	0x00800000
    UINT32 info_24 : 1;				 //	0x01000000
    UINT32 info_25 : 1;              //	0x02000000
    UINT32 info_26 : 1;				 // 0x04000000
    UINT32 info_27 : 1;              // 0x08000000
    UINT32 info_28 : 1;				 // 0x10000000
    UINT32 info_29 : 1;				 //	0x20000000
    UINT32 info_30 : 1;				 //	0x40000000
    UINT32 info_31 : 1;				 //	0x80000000
} EInkPumpInfo;
	
typedef struct ERobotInfo
{
	UINT32 ref_done			: 1;	//	0x00000001
	UINT32 moving			: 1;	//	0x00000002
	UINT32 x_in_ref			: 1;	//	0x00000004
	UINT32 z_in_ref			: 1;	//	0x00000008
	UINT32 z_in_print		: 1;	//	0x00000010
	UINT32 z_in_cap			: 1;	//	0x00000020
	UINT32 z_in_wipe		: 1;	//	0x00000040
	UINT32 z_in_vacuum		: 1;	//	0x00000080
	UINT32 z_in_wash		: 1;	//	0x00000100
	UINT32 move_ok			: 1;	//	0x00000200
	UINT32 cap_ready		: 1;	//	0x00000400
	UINT32 wipe_ready		: 1;	//	0x00000800
	UINT32 vacuum_ready		: 1;	//	0x00001000
	UINT32 wash_ready		: 1;	//	0x00002000
	UINT32 purge_ready		: 1;	//	0x00004000	
	UINT32 wipe_done		: 1;	//	0x00008000
	UINT32 vacuum_done		: 1;	//	0x00010000
	UINT32 wash_done		: 1;	//	0x00020000
	UINT32 purge_done		: 1;	//	0x00040000
	UINT32 vacuum_in_change	: 1;	//	0x00080000
	UINT32 r_info_20		: 1;	//	0x00100000
	UINT32 rob_in_wipe		: 1;	//	0x00200000
	UINT32 rob_in_vac		: 1;	//	0x00400000
	UINT32 rob_in_wash		: 1;	//	0x00800000
	UINT32 rob_in_cap		: 1;	//	0x01000000
	UINT32 moving_wd		: 1;	//	0x02000000
	UINT32 ref_done_wd		: 1;    //  0x04000000
	UINT32 wd_front_up		: 1;	//  0x08000000
	UINT32 wd_back_up		: 1;	//  0x10000000
	UINT32 wrinkle_detected	: 1;	//	0x20000000
	UINT32 wd_in_up			: 1;	//	0x40000000
	UINT32 r_info_31		: 1;	//	0x80000000
} ERobotInfo;

typedef struct ETestTableWarn
{
	UINT32 warn_0 : 1;
	UINT32 warn_1 : 1;
	UINT32 warn_2 : 1;
	UINT32 warn_3 : 1;
	UINT32 warn_4 : 1;
	UINT32 warn_5 : 1;
	UINT32 warn_6 : 1;
	UINT32 warn_7 : 1;
	UINT32 warn_8 : 1;
	UINT32 warn_9 : 1;
	UINT32 warn10 : 1;
	UINT32 warn11 : 1;
	UINT32 warn12 : 1;
	UINT32 warn13 : 1;
	UINT32 warn14 : 1;
	UINT32 warn15 : 1;
	UINT32 warn16 : 1;
	UINT32 warn17 : 1;
	UINT32 warn18 : 1;
	UINT32 warn19 : 1;
	UINT32 warn20 : 1;
	UINT32 warn21 : 1;
	UINT32 warn22 : 1;
	UINT32 warn23 : 1;
	UINT32 warn24 : 1;
	UINT32 warn25 : 1;
	UINT32 warn26 : 1;
	UINT32 warn27 : 1;
	UINT32 warn28 : 1;
	UINT32 warn29 : 1;
	UINT32 warn30 : 1;
	UINT32 warn31 : 1;
} ETestTableWarn;

//--- SStepperMotor -------------------------------------- 
typedef struct SStepperMotor
{
	UINT32	state;
		#define MOTOR_STATE_UNDEF		0
		#define MOTOR_STATE_IDLE		1	
		#define MOTOR_STATE_MOVING_FWD	2
		#define MOTOR_STATE_MOVING_BWD	3
		#define MOTOR_STATE_BLOCKED		4		
	INT32	motor_pos;
	INT32	encoder_pos;	
} SStepperMotor;

//--- SStepperStat -------------------------------------- 
typedef struct SStepperStat
{
	UINT32		no;
	//--- rom values, stored in head board ----
	UINT64		macAddr;
	UINT64		serialNo;
	SVersion	swVersion;
	SVersion	fpgaVersion;

	INT32		robot_used;
	
	INT32		cmdRunning;

	//--- warnings/errors ----------------
	ETestTableInfo	info;		// UINT32
	ERobotInfo		robinfo;	// UINT32
    EInkPumpInfo	inkinfo;	// UINT32		

	UINT32		warn;
	
	UINT32		err;
		#define	TT_ERR_FPGA_NOT_LOADED	0x00000001
		#define	TT_ERR_NIOS_NOT_LOADED	0x00000002
		#define TT_ERR_STEPPER_Y_LEFT	0x00000004
		#define TT_ERR_STEPPER_Y_RIGHT	0x00000008
		#define TT_ERR_STEPPER_Z		0x00000010
		#define TT_ERR_SLIDE			0x00000020
		#define TT_ERR_COVER_OPEN		0x00000040

	UINT32		tts_fluidvalves;

	INT32		posX;
	INT32		posY;
	INT32		posZ;
	
	INT32		adjustmentProgress;
	UINT32		alive[2];

	INT32			inputs;
	SStepperMotor	motor[MAX_STEPPER_MOTORS];
//	INT32			unused_set_io_cnt;
} SStepperStat;

	
//... robot ---------------------------------

typedef struct SRobotIO
{
	INT8	board;
	INT8	io;
} SRobotIO;
	
typedef struct SRoboMotorCfg
{
	INT8	board;			// number of stepper board
	INT8	motor;			// number of motor on board
	INT8	referenceIn;	// number of input at reference point
	INT16	currentHold; 
	INT16	currentWork;
} SRoboMotorCfg;
	
typedef struct SRoboPumpCfg
{
	INT8		board;
	INT8		pwmOut[2];
	INT8		analogIn[2];
	INT8		in[2];		
	SRobotIO	flushOut[8];
} SRoboPumpCfg;

#define LEFT	0
#define RIGHT	1
typedef struct SRobotCfg
{
	SRoboMotorCfg slidMotor[2];
	SRoboMotorCfg screwMotor[2]; 
	
	//--- outputs ---
	SRobotIO		out0;	
	SRobotIO		out1;	
	SRobotIO		out2;	
	SRobotIO		out3;
	SRobotIO		out4;	
	SRobotIO		out5;	
	SRobotIO		out6;	
	SRobotIO		out7;	
	//--- inputs ---
	SRobotIO		bitUp_In[2];
	SRobotIO		detach_In[2];	
} SRobotCfg;		

	
//=== Images ================================================================
typedef struct SSplitCfg
{
	INT32	firstPx;	// number of first pixel
	INT32	widthPx;	// number of pixels

//	UINT32	headNo;		// number of print head board
	UINT32	block0;		// first memory block to use
	UINT32	blockCnt;	// number of memory blocks on head board
} SSplitCfg;

typedef struct SColorSplitCfg
{
	INT32			no;
	INT32			inkSupplyNo;
	SInkDefinition	color;
	ERectoVerso		rectoVerso;
	INT32			spoolerNo;
	INT32			firstLine;
	INT32			lastLine;
	INT32			offsetPx;
	SSplitCfg		split[MAX_HEADS_COLOR];
} SColorSplitCfg;

//--- print system -------------------------------

typedef struct SRxConfig
{
	UINT8			simulation;
	UINT8			inkSupplyCnt;
	UINT8			inkCylindersPerColor;
	UINT8			colorCnt;
	UINT8			headsPerColor;
	INT32			headDist[MAX_HEAD_DIST];
	INT32			headDistBack[MAX_HEAD_DIST];
	INT32			headDistMax;
	INT32			headDistBackMax;
	INT32			colorOffset[INK_SUPPLY_CNT];
	char			material[64];
	SPrinterCfg		printer;
	SEncoderCfg		encoder[ENC_CNT];
	SStepperCfg		stepper;
	SInkSupplyCfg	inkSupply[INK_SUPPLY_CNT];
	SHeadBoardCfg	headBoard[HEAD_BOARD_CNT];	
	INT32			externalData;
	SConditionerCfg	cond[MAX_HEAD_DIST];
	INT32			headFpVoltage[MAX_HEAD_DIST];
	struct
	{
		INT32			tara[MAX_SCALES];
		INT32			calib[MAX_SCALES];			
	} scales;
//	INT16			headDisabledJets[MAX_HEAD_DIST][MAX_DISABLED_JETS];
} SRxConfig;

/*
typedef struct SRxStatus
{
	SEncStat		encoder;
//	SCleanStat		clean;
} SRxStatus;
*/

#pragma pack()

//--- SPlaneInfo ---------------------------------------------
typedef struct
{
	UINT32 widthPx;
	UINT32 lengthPx;
	UINT32 bitsPerPixel;
	UINT32 lineLen;		// in bytes
	UINT32 aligment;	// 8, 16, 32 ,....
	UINT64 dataSize;
	SPoint resol;
	INT32 planenumber;		// plane number
	INT32 colorCode;
	PBYTE buffer;
	UINT64 DropCount[3];	// Drop count for drop size 1, 2, and 3
} SPlaneInfo;


//--- SBmpInfo ---------------------------------------------
typedef struct
{
	INT32 srcWidthPx;
	INT32 memWidthPx;
	INT32 lengthPx;
	INT32 bitsPerPixel;
	INT32 lineLen;		// in bytes
	INT32 aligment;	// 8, 16, 32 ,....
	UINT64 dataSize;
	INT32 inverseColor;	// value 1=white
	INT32 topFirst;		// first line is top line
	SPoint resol;
	INT32 printMode;
		#define PM_UNDEF				0
		#define PM_SCANNING				1
		#define PM_SINGLE_PASS			2
		#define PM_TEST					3
		#define PM_TEST_SINGLE_COLOR	4
		#define PM_TEST_JETS			5
		#define PM_SCAN_MULTI_PAGE		6

	INT32 scanCopies;
	INT32 planes;		// number of planes
	INT32 colorCode[MAX_COLORS];
	INT32 inkSupplyNo[MAX_COLORS];
	PBYTE *buffer[MAX_COLORS];
	UINT64 DropCount[MAX_COLORS][3]; //Drop count for drop size 1, 2, and 3 per plane
	INT8 multiCopy;
	INT8 colorCnt;
	INT8 screening;
} SBmpInfo;

//--- global variables -----------------------------
extern SRxConfig		RX_Config;
//extern SRxStatus		RX_Status;
extern SRxNetwork		RX_Network;
extern int				RX_SpoolerNo;
extern SSpoolerCfg		RX_Spooler;
extern SColorSplitCfg	RX_Color[MAX_COLORS];
extern SHeadBoardCfg	RX_HBConfig;
extern SHeadBoardStat	RX_HBStatus[];
extern SPrinterStatus	RX_PrinterStatus;
extern SEncoderStat		RX_EncoderStatus;
extern SEncoderCfg		RX_EncoderCfg;
extern SFluidBoardStat	RX_FluidBoardStatus;
extern SStepperStat	RX_StepperStatus;
extern SStepperStat	RX_ClnStatus;
extern SPrintQueueItem  RX_TestImage;
extern char				RX_TestData[MAX_COLORS*MAX_HEADS_COLOR][MAX_TEST_DATA_SIZE];
extern int				RX_HeadIsVerso[MAX_COLORS*MAX_HEADS_COLOR];
	
extern SStepperCfg		RX_StepperCfg;
extern char				RX_Hostname[64];

//--- prototype -----------------------------------
char *mode(EnFluidCtrlMode mode);
	
#ifdef __cplusplus
}
#endif
