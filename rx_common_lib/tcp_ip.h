// ****************************************************************************
//
//	TCP_IP.h
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

#include "rx_common.h"
#include "rx_def.h"
#include "fpga_def_head.h"
#include "rx_ink_common.h"

#pragma pack(1)

//--- Globals ---------------------------------------------------
#define MAX_MESSAGE_SIZE	9000 // (8*1024) // jumbo blocks
#define TCPIP_HEADER_SIZE	  66

#ifndef RX_CTRL_SUBNET // allow to redefine local subnet during compilation
#define RX_CTRL_SUBNET		"192.168.200."
#endif // !RX_CTRL_SUBNET

#define RX_CTRL_MAIN		RX_CTRL_SUBNET "1"
#define RX_CTRL_PLC			RX_CTRL_SUBNET "3"
#define RX_CTRL_ENC_0		4				// 6..8
#define RX_CTRL_FLUID_0		6				// 6..8
#define RX_CTRL_STEPPER_0	9				// 9..10
#define RX_CTRL_STEPPER_HD	RX_CTRL_SUBNET "9"
#define RX_CTRL_STEPPER_CLN	RX_CTRL_SUBNET "10"
#define RX_CTRL_IQ500		RX_CTRL_SUBNET "240"
#define RX_CTRL_POWERSUPPLY	RX_CTRL_SUBNET "241"
#define RX_CTRL_SWITCH_MAIN RX_CTRL_SUBNET "250"
#define RX_CTRL_SWITCH_BAR0 RX_CTRL_SUBNET "251"
#define RX_CTRL_HEAD_0		11
// #define RX_CTRL_DATA		RX_CTRL_SUBNET "200"
#define RX_CTRL_SPOOL		RX_CTRL_SUBNET "200" //+no: 201..

#define RX_CTRL_BROADCAST	RX_CTRL_SUBNET "255"

//--- TCP ports -------------------------------------------------
#define PORT_GUI				7000
#define PORT_CTRL_MAIN			7001
#define PORT_CTRL_HEAD			7011
#define PORT_UDP_DATA			7012	// on FPGA
#define PORT_CTRL_ENCODER		7013
#define PORT_CTRL_FLUID			7014
#define PORT_CTRL_STEPPER		7015
#define PORT_CTRL_PLC			7016

#define PORT_UDP_BOOT_SVR		7004
#define PORT_UDP_BOOT_CLNT		7005
#define PORT_REMOTE_FILE_SERVER	7006
#define PORT_DATA_SERVER		7007
#define PORT_RIP_SERVER			7008

#define PORT_PE_MAIN			7100
#define PORT_PE_SPOOL			7101
#define PORT_RE_MAIN			7200

#define PORT_TERM				777

//--- message IDs -----------------------------------------------
#define CMD_X					0x01000000		// commands
#define REP_X					0x02000000		// replies to commands
#define EVT_X					0x03000000		// events

#define CMD_PING				0x01000001
#define REP_PING				0x02000001

#define CMD_STATUS				0x01000002
#define REP_STATUS				0x02000002

#define CMD_ERROR_RESET			0x01000003

#define CMD_RESTART_MAIN		0x01000004

#define CMD_GET_EVT				0x01000011
#define REP_GET_EVT				0x02000011
#define EVT_GET_EVT				0x03000011

#define CMD_EVT_CONFIRM			0x01000012
#define REP_EVT_CONFIRM			0x02000012	

#define CMD_REQ_LOG				0x01000013
#define REP_REQ_LOG				0x02000013
#define EVT_GET_LOG				0x03000013

#define CMD_EXPORT_LOG			0x01000014

#define CMD_GET_NETWORK			0x01000031	// reqeuests all network items
#define REP_GET_NETWORK			0x02000031	// reply
#define EVT_NETWORK_ITEM		0x03000031	// send one network item

#define CMD_SET_NETWORK			0x01000032	// set/change one network item
#define REP_SET_NETWORK			0x02000032	// reply
#define CMD_NETWORK_SAVE		0x01000033
#define CMD_NETWORK_RELOAD		0x01000034
#define REP_NETWORK_RELOAD		0x02000034
#define CMD_NETWORK_DELETE		0x01000035

#define CMD_NETWORK_SETTINGS	0x01000036
#define REP_NETWORK_SETTINGS	0x02000036

#define CMD_GET_PRINT_QUEUE		0x01000041
#define REP_GET_PRINT_QUEUE		0x02000041
#define EVT_GET_PRINT_QUEUE		0x03000041		
#define CMD_GET_PRINT_QUEUE_ITM	0x04000041

#define CMD_ADD_PRINT_QUEUE		0x01000042
#define REP_ADD_PRINT_QUEUE		0x02000042
#define EVT_ADD_PRINT_QUEUE		0x03000042

#define CMD_SET_PRINT_QUEUE		0x01000043
#define REP_SET_PRINT_QUEUE		0x02000043
#define EVT_SET_PRINT_QUEUE		0x03000043
#define CMD_CHG_PRINT_QUEUE		0x04000043
#define REP_CHG_PRINT_QUEUE		0x05000043

#define CMD_DEL_PRINT_QUEUE		0x01000044
#define REP_DEL_PRINT_QUEUE		0x02000044
#define EVT_DEL_PRINT_QUEUE		0x03000044		

#define CMD_UP_PRINT_QUEUE		0x01000045
#define REP_UP_PRINT_QUEUE		0x02000045
#define EVT_UP_PRINT_QUEUE		0x03000045		

#define CMD_DN_PRINT_QUEUE		0x01000046
#define REP_DN_PRINT_QUEUE		0x02000046
#define EVT_DN_PRINT_QUEUE		0x03000046		

#define CMD_GET_PRINT_ENV		0x01000047
#define REP_GET_PRINT_ENV		0x02000047
#define BEG_GET_PRINT_ENV		0x04000047		// begin of list
#define ITM_GET_PRINT_ENV		0x05000047		// list item
#define END_GET_PRINT_ENV		0x06000047		// end of list

#define CMD_GET_INK_DEF			0x01000048
#define REP_GET_INK_DEF			0x02000048
#define SET_GET_INK_DEF			0x03000048		// -> sent ink-def to head 
#define BEG_GET_INK_DEF			0x04000048		// begin of list
#define ITM_GET_INK_DEF			0x05000048		// list item
#define END_GET_INK_DEF			0x06000048		// end of list

#define CMD_GET_PRINTER_CFG		0x01000049
#define REP_GET_PRINTER_CFG		0x02000049

#define CMD_SET_PRINTER_CFG		0x0100004A
#define REP_SET_PRINTER_CFG		0x0200004A

#define CMD_DEL_FILE			0x0100004B

#define CMD_RIP_START			0x0100004E
#define REP_RIP_START			0x0200004E
#define EVT_RIP_START			0x0300004E

#define CMD_HEAD_BOARD_CFG		0x01000101
#define REP_HEAD_BOARD_CFG		0x02000101

#define CMD_COLOR_CFG			0x01000102
#define REP_COLOR_CFG			0x02000102

#define CMD_HEAD_STAT			0x01000103
#define REP_HEAD_STAT			0x02000103

#define CMD_HEAD_FLUID_CTRL_MODE 0x01000104
#define REP_HEAD_FLUID_CTRL_MODE 0x02000104

#define CMD_GET_DENSITY			0x01000107
#define REP_GET_DENSITY			0x02000107
#define CMD_SET_DENSITY			0x01000108
#define CMD_SET_ROB_POS			0x01000109
#define CMD_HEAD_ADJUST			0x0100010a
#define CMD_PURGE_CLUSTER		0x0100010b
#define CMD_HEAD_OUT_OF_RANGE	0x0100010c		

#define CMD_ENCODER_CFG			0x01000111
#define REP_ENCODER_CFG			0x02000111

#define CMD_ENCODER_STAT		0x01000112
#define REP_ENCODER_STAT		0x02000112

#define CMD_ENCODER_UV_ON		0x01000113
#define CMD_ENCODER_UV_OFF		0x01000114

#define CMD_ENCODER_PG_INIT		0x01000115
#define CMD_ENCODER_PG_DIST		0x01000116
#define CMD_ENCODER_PG_STOP		0x01000117
#define CMD_ENCODER_PG_RESTART	0x01000118

#define CMD_ENCODER_DISABLE		0x01000119	// temporary disable the encoder input (CLEAF Splice)
#define CMD_ENCODER_ENABLE		0x0100011a	// enable after temorary disable

#define CMD_ENCODER_SAVE_PAR	0x0100011c	// save correction parameters	
#define CMD_ENCODER_SAVE_PAR_1	0x0100011d	// save correction parameters	

#define CMD_FLUID_CFG			0x01000121
#define REP_FLUID_CFG			0x02000121

#define CMD_FLUID_IS_CFG		0x01000122

#define CMD_FLUID_STAT			0x01000123
#define REP_FLUID_STAT			0x02000123

#define CMD_SET_PURGE_PAR		0x01000124
#define REP_SET_PURGE_PAR		0x02000124

#define CMD_FLUID_CTRL_MODE		0x01000125
#define REP_FLUID_CTRL_MODE		0x02000125

#define CMD_FLUID_PRESSURE		0x01000126

#define CMD_FLUID_DEGASSER		0x02000127

#define CMD_FLUID_FLUSH			0x01000129

#define CMD_GET_STEPPER_CFG		0x01000131
#define REP_GET_STEPPER_CFG		0x02000131
#define CMD_SET_STEPPER_CFG		0x01000132
#define REP_SET_STEPPER_CFG		0x02000132

#define CMD_SCALES_SET_CFG		0x01000141
#define CMD_SCALES_GET_CFG		0x01000142
#define REP_SCALES_GET_CFG		0x02000142
#define CMD_SCALES_TARA			0x01000143
#define CMD_SCALES_CALIBRATE	0x01000144
#define CMD_SCALES_STAT			0x01000145
#define REP_SCALES_STAT			0x02000145

#define CMD_BCSCANNER_RESET		0x01000146
#define CMD_BCSCANNER_IDENTIFY	0x01000147
#define CMD_BCSCANNER_TRIGGER	0x01000148

#define REP_CHILLER_STAT		0x02000152

#define CMD_CHANGE_CLUSTER_NO	0x01000161
#define CMD_RESET_COND			0x01000162

#define CMD_SETUP_ASSIST_STAT	0x01000170
#define REP_SETUP_ASSIST_STAT	0x02000170
#define CMD_SA_REFERENCE		0x01000171
#define CMD_SA_MOVE				0x01000172
#define CMD_SA_STOP				0x01000173
#define CMD_SA_OUT_TRIGGER		0x01000174
#define CMD_SA_WEB_MOVE			0x01000175
#define CMD_SA_WEB_STOP			0x01000176

#define CMD_START_PRINTING		0x01000201
#define REP_START_PRINTING		0x02000201

#define CMD_PAUSE_PRINTING		0x01000202
#define REP_PAUSE_PRINTING		0x02000202

#define CMD_STOP_PRINTING		0x01000203
#define REP_STOP_PRINTING		0x02000203
	
#define CMD_ABORT_PRINTING		0x01000204
#define REP_ABORT_PRINTING		0x02000204

#define CMD_EXTERNAL_DATA_ON	0x01000205
#define CMD_EXTERNAL_DATA_OFF	0x01000206

#define CMD_RESET_CTR			0x01000207

#define CMD_CLEAN_START			0x01000210
#define REP_CLEAN_START			0x02000210

#define CMD_TEST_START			0x01000211
#define REP_TEST_START			0x02000211

#define CMD_GET_PRN_STAT		0x01000220
#define EVT_PRINTER_STAT		0x03000220

//--- commands for PLC ---------------------
#define CMD_PLC_0				0x01000300

#define CMD_PLC_GET_INFO		0x01000350
#define REP_PLC_GET_INFO		0x02000350
#define CMD_PLC_RESET_ERROR		0x01000351
#define CMD_PLC_REBOOT			0x01000352
#define CMD_PLC_GET_LOG			0x01000353
#define REP_PLC_GET_LOG			0x02000353
// #define END_PLC_GET_LOG			0x06000353

#define CMD_PLC_SAVE_PAR		0x01000360
#define CMD_PLC_LOAD_PAR		0x01000361
#define REP_PLC_LOAD_PAR		0x02000361
#define CMD_PLC_GET_VAR			0x01000362	
#define REP_PLC_GET_VAR			0x02000362
#define CMD_PLC_SET_VAR			0x01000363	
#define CMD_PLC_SET_CMD			0x01000364

#define CMD_PLC_REQ_MATERIAL	0x01000365
#define CMD_PLC_RES_MATERIAL    0x02000365
#define CMD_PLC_ITM_MATERIAL    0x03000365
#define CMD_PLC_SAVE_MATERIAL   0x01000366
#define CMD_PLC_DEL_MATERIAL    0x01000367

//--- Siemens messages -------------------------
#define CMD_PLC_CFG				0x010003a1

#define CMD_PLC_STAT			0x010003a2
#define REP_PLC_STAT			0x020003a2

#define CMD_PLC_PAR				0x010003a3
#define REP_PLC_PAR				0x020003a3
#define EVT_PLC_PAR				0x030003a3


#define CMD_PLC_END				0x010003FF

//--- Steppper Board ---------------------------------------------
#define CMD_STEPPER_CFG			0x01000401
#define CMD_STEPPER_STAT		0x01000402
#define REP_STEPPER_STAT		0x02000402
#define CMD_STEPPER_TEST		0x01000403

//--- Test Table Commands ------------------------------------------
#define CMD_TT_0				0x01000500
#define CMD_TT_STOP				0x01000501
#define CMD_TT_START_REF		0x01000502
#define CMD_TT_MOVE_TABLE		0x01000503
#define CMD_TT_SCAN_RIGHT		0x01000504
#define CMD_TT_SCAN_LEFT		0x01000505
#define CMD_TT_VACUUM			0x01000506
#define CMD_TT_SCAN				0x01000507
#define CMD_TT_MOVE_LOAD		0x01000508
#define CMD_TT_MOVE_CAP 		0x01000509
#define CMD_TT_MOVE_PURGE		0x0100050a
#define CMD_TT_MOVE_ADJUST		0x0100050b
#define CMD_TT_ABORT			0x0100050c
#define CMD_TT_SCAN_TRAY		0x0100050d

#define CMD_TTS_WASTE_VALVE		0x01000510
#define CMD_TTS_PUMP_JET_TRAY	0x01000511
#define CMD_TTS_PUMP_PURGE		0x01000512
#define CMD_TTS_JOG_FWD			0x01000513
#define CMD_TTS_JOG_BWD			0x01000514
#define CMD_TTS_JOG_STOP		0x01000515
#define CMD_FLUID_TTS			0x01000516
#define REP_FLUID_TTS			0x02000516

#define CMD_TT_END				0x010005ff

//--- Capping Commands ---------------------------------------------------
#define CMD_LIFT_0				0x01000600
#define CMD_LIFT_STOP			0x01000601
#define CMD_LIFT_REFERENCE		0x01000602
#define CMD_LIFT_PRINT_POS		0x01000603
#define CMD_LIFT_CAPPING_POS	0x01000604
#define CMD_LIFT_UP_POS			0x01000605
#define CMD_LIFT_CALIBRATE		0x01000606
#define REP_LIFT_CALIBRATE		0x02000606
#define CMD_LIFT_TEST			0x01000607
#define CMD_LIFT_JET_TRAY		0x01000608
#define CMD_LIFT_CLUSTER_CHANGE	0x01000609
#define CMD_LIFT_SCREW			0x0100060a
#define CMD_LIFT_VENT			0x0100060f

//---- CAPPING ----
#define CMD_LIFT_FILL			0x01000611
#define CMD_LIFT_EMPTY			0x01000612

//---- WIPING -----
#define CMD_LIFT_WIPE_POS		0x01000614
#define CMD_LIFT_VACUUM_POS		0x01000615
#define CMD_LIFT_WASH_POS		0x01000616
#define CMD_LIFT_VACUUM_HIGH_POS	0x01000617

//---- END OF CAPPING ----
#define CMD_LIFT_END				0x010006ff

//--- Cleaning Commands ---------------------------------------------------
#define CMD_ROB_0				0x01000700
#define CMD_ROB_STOP			0x01000701
#define CMD_ROB_REFERENCE		0x01000702
#define CMD_ROB_MOVE_POS		0x01000703
#define CMD_SEARCH_ALL_SCREWS	0x01000704
#define CMD_CFG_SCREW_POS		0x01000705
#define CMD_RESET_ALL_SCREWS	0x01000706

#define CMD_ROB_DRIP_PANS				0x01000721
// #define CMD_ROB_DRIP_PANS_EN			0x01000722		// Main ask board 0 for positions of drip pans
#define CMD_LIFT_ALLOW_MOVE_DOWN		0x01000723
#define CMD_LIFT_NOT_ALLOW_MOVE_DOWN	0x01000724
#define CMD_ROB_DRIP_PANS_CAP			0x01000725
#define CMD_ROB_DRIP_PANS_REF			0x01000726

#define CMD_ROB_ROT_REF			0x01000730
#define CMD_ROB_SHIFT_REF		0x01000731
#define CMD_ROB_SHIFT_MOV		0x01000732
#define CMD_ROB_SHIFT_LEFT		0x01000733
#define CMD_ROB_ROT_REF2		0x01000734
#define CMD_ROB_FILL_CAP		0x01000735
#define CMD_ROB_EMPTY_WASTE		0x01000736
#define CMD_ROB_TILT_CAP		0x01000737
#define CMD_ROB_VACUUM			0x01000738
#define CMD_ROB_WAIT			0x01000739
#define CMD_ROB_WIPE			0x0100073a



#define CMD_ROBI_REFERENCE			0x01000740
#define CMD_ROBI_STOP				0x01000741
#define CMD_ROBI_MOVE_X				0x01000742
#define CMD_ROBI_MOVE_Y				0x01000743
#define CMD_ROBI_MOVE_Z_UP			0x01000744
#define CMD_ROBI_MOVE_Z_DOWN		0x01000745
#define CMD_ROBI_SCREW_STEPS		0x01000746
#define CMD_ROBI_SCREW_LEFT			0x01000747
#define CMD_ROBI_SCREW_RIGHT		0x01000748
#define CMD_ROBI_MOVE_TO_X			0x01000749
#define CMD_ROBI_MOVE_TO_Y			0x0100075a
#define CMD_ROBI_MOVE_TO_GARAGE		0x0100075b
#define CMD_ROBI_WIPE_LEFT			0x0100075c
#define CMD_ROBI_WIPE_RIGHT			0x0100075d

#define CMD_ROB_END				0x010007ff


//--- Data Spooler -------------------------------------------------
#define CMD_SET_SPOOL_CFG		0x01003001
#define REP_SET_SPOOL_CFG		0x02003001
#define CMD_REQ_SPOOL_CFG		0x04003001

#define CMD_LOAD_FILE			0x01003002

#define CMD_PRINT_FILE			0x01003003
#define REP_PRINT_FILE			0x02003003
#define EVT_PRINT_FILE			0x03003003

#define EVT_PRINT_DONE			0x03003004

#define CMD_PRINT_ABORT			0x01003005
#define REP_PRINT_ABORT			0x02003005

#define CMD_FONTS_UPDATED		0x01003006

#define CMD_STRESS_TEST			0x01003007

#define BEG_SET_FILEDEF			0x04003010
#define ITM_SET_FILEDEF			0x05003010	// header + data
#define END_SET_FILEDEF			0x06003010

#define BEG_SET_LAYOUT			0x04003011		
#define ITM_SET_LAYOUT			0x05003011	// header + data	
#define END_SET_LAYOUT			0x06003011		

#define CMD_SET_CTRDEF			0x01003012

#define CMD_PRINT_DATA			0x01003030
#define CMD_PRINT_TEST_DATA		0x01003031

#define CMD_GET_BLOCK_USED		0x01001010
#define REP_GET_BLOCK_USED		0x02001010

#define CMD_FPGA_IMAGE			0x01001011
#define REP_FPGA_IMAGE			0x02001011

#define CMD_FPGA_WRITE_BMP		0x01001012
#define REP_FPGA_WRITE_BMP		0x02001012

#define CMD_FPGA_SIMU_PRINT		0x01001013
#define CMD_FPGA_SIMU_ENCODER	0x01001014

#define EVT_TRACE				0x03000100	

/*
//--- Data Server ---------------------------------------------------------
#define CMD_DS_OPEN				0x01002000
#define REP_DS_OPEN				0x02002000
#define CMD_DS_READ				0x01002001
#define REP_DS_READ				0x02002001
#define CMD_DS_CLOSE			0x01002002
#define REP_DS_CLOSE			0x02002002
*/

//--- CLEAF Orders ----------------------------------------------------
#define CMD_CO_SET_ORDER		0x01003000
#define CMD_CO_GET_ORDER		0x01003001
#define REP_CO_GET_ORDER		0x02003001
#define CMD_CO_SET_OPERATOR		0x01003002
#define REP_CO_SET_OPERATOR		0x02003002
#define CMD_CO_GET_PRODUCTION	0x02003003
#define REP_CO_GET_PRODUCTION	0x02003003
#define CMD_CO_GET_ROLLS		0x01003004
#define REP_CO_GET_ROLLS		0x02003004
#define CMD_CO_SET_ROLL			0x01003005
#define REP_CO_SET_ROLL			0x02003005

//--- UDP Booting commands ----------------------------------------
#define CMD_BOOT_INFO_REQ		0x11000001
#define REP_BOOT_INFO			0x12000001

#define CMD_BOOT_ADDR_SET		0x11000002
#define CMD_BOOT_FLASH_ON		0x11000003
#define CMD_BOOT_PING			0x11000004
#define REP_BOOT_PING			0x12000004

//--- Remote File Server commands --------------------------------------
#define CMD_RFS_MAKE_DIR		0x21000001
#define REP_RFS_MAKE_DIR		0x22000001
#define CMD_RFS_DEL_DIR			0x21000002
#define REP_RFS_DEL_DIR			0x22000002
#define CMD_RFS_GET_DIR			0x21000003
#define REP_RFS_GET_DIR			0x22000003
#define EVT_RFS_DIR_ENTRY		0x23000003
#define CMD_RFS_DEL_FILE		0x21000004
#define REP_RFS_DEL_FILE		0x22000004

#define CMD_RFS_SAVE_FILE_HDR	0x21000005
#define REP_RFS_SAVE_FILE_HDR	0x22000005
#define CMD_RFS_SAVE_FILE_BLOCK	0x21000006
#define REP_RFS_SAVE_FILE_BLOCK	0x22000006

#define CMD_RFS_KILL_PROCESS	0x21000007
#define REP_RFS_KILL_PROCESS	0x22000007
#define CMD_RFS_START_PROCESS	0x21000008
#define REP_RFS_START_PROCESS	0x21000008

//--- defines ----------------------------------------------
#define IP_ADDR_SIZE	32

//--- message header --------------------------------------------
typedef struct SMsgHdr
{
	INT32  msgLen;
	UINT32 msgId;
} SMsgHdr;

typedef struct SReply
{
	SMsgHdr		hdr;
	UINT32		reply;
} SReply;

//--- Head Configuration -----------------------------------------
typedef enum EJetAlignment
{
	align_1200,
	align_600
} EJetAlignment;

//--- message REP_STATUS / EVT_STATUS -------------------------------------------------
typedef struct SStatusMsg
{
	SMsgHdr		hdr;
	//---  to be defined ----
} SStatusMsg;

//--- message EVT_TRACE -------------------------------------------------
typedef struct STraceMsg
{
	SMsgHdr		hdr;
	INT64		time;
	char		message[256];
} STraceMsg;

//--- message GET_LOG -------------------------------------------------
typedef struct SLogReqMsg
{
	SMsgHdr		hdr;
	char		filepath[256];
	char		find[256];
	INT32		first;
	INT32		count;	
} SLogReqMsg;

//--- message EVT_LOG -------------------------------------------------
typedef struct SLogMsg
{
	SMsgHdr		hdr;
	SLogItem	log;
} SLogMsg;

//-- SGetNetworkCmd -----------------------------------------------------
typedef struct SGetNetworkCmd
{
	SMsgHdr		hdr;
} SGetNetworkCmd;

//--- SNetworkMsg ----------------------------------------------------
typedef struct SNetworkMsg
{
	SMsgHdr			hdr;
	SNetworkItem	item;
	UINT8			flash;
} SNetworkMsg;

//--- SSetNetworkCmd ----------------------------------------------------
typedef struct SSetNetworkCmd
{
	SMsgHdr			hdr;
	SNetworkItem	item;
	UINT8			flash;
} SSetNetworkCmd;


//--- Print Queue -----------------------------------------------------

//--- SPrintQueueMsg -----------------------
typedef struct SPrintQueueMsg
{
	SMsgHdr			hdr;
	SPrintQueueItem	item;
} SPrintQueueEvt;

//--- SPrinterStatusMsg -------------------
typedef struct SPrinterStatusMsg
{
	SMsgHdr			hdr;
	SPrinterStatus	stat;
} SPrinterStatusEvt;

//--- SInkSupplyStatMsg ------------------------
typedef struct SInkSupplyStatMsg
{
	SMsgHdr			hdr;
	INT32			no;
	SInkSupplyStat	stat;	
} SInkSupplyStatMsg;

//--- SPrintEnvMsg ---------------------------------
typedef struct SPrintEnvEvt
{
	SMsgHdr			hdr;
	char			printEnv[128];
} SPrintEnvEvt;

//--- SRipMsg --------------------------------------
typedef struct SRipCmd
{
	SMsgHdr			hdr;
	char			jobName[128];
	char			filePath[256];
	char			printEnv[128];
	char			sourceIpAddr[128];
	INT16			orientation;
	INT16			columns;
	INT16			colDist;
} SRipCmd;

//--- SRipEvt ------------------------
typedef struct SRipEvt
{
	SMsgHdr			hdr;
	UINT32			jobId;
	char			jobName[128];
	char			state[128];
} SRipEvt;

//--- SInkDefMsg ---------------------------------
typedef struct SInkDefMsg
{
	SMsgHdr			hdr;
	SInkDefinition	ink;
	UINT32			headNo;
	char			dots[4];
	UINT32			fpVoltage;
} SInkDefMsg;

//--- SPrinterCfgMsg ---
typedef struct
{
	SMsgHdr			hdr;
	char			hostName[64];
	EPrinterType	type;
	UINT32			overlap;
	SOffsetCfg		offset;
	INT32			externalData;
	char			inkFileNames[MAX_COLORS*64];
//	ERectoVerso		rectoVerso[MAX_COLORS];
	ERectoVerso		rectoVerso[INK_SUPPLY_CNT];
	UINT32			colorCnt;
	UINT32			headsPerColor;			
	UINT32			inkSupplyCnt;
	UINT32			inkCylindersPerColor;
	INT32			headFpVoltage[MAX_HEAD_DIST];
	INT32			headDist[MAX_HEAD_DIST];
	INT32			headDistBack[MAX_HEAD_DIST];
	INT32			colorOffset[MAX_COLORS];

} SPrinterCfgMsg;

typedef struct SDensityMsg
{
	SMsgHdr	hdr;
	int		head;
	SHeadEEpromDensity data;
} SDensityMsg;

typedef struct SRobPositionMsg
{
	SMsgHdr	hdr;
	int		printBar;
	int		head;
	INT16	angle;
	INT16	stitch;
} SRobPositionMsg;


typedef struct
{
    SMsgHdr hdr;
    INT32 printbarNo;
#define LEFT 0
#define RIGHT 1
    INT32 headNo;
    INT32 axis;
#define AXE_ANGLE 0
#define AXE_STITCH 1
    INT32 steps; // in steps
} SHeadAdjustmentMsg;

//--- message CMD_SEND_DATA -------------------------------------------

//--- message DATA ----------------------------------------------------
typedef struct SDataBlockMsg
{
	SMsgHdr		hdr;
	BYTE		data[1];
} SDataBlockMsg;

//--- CMD_PRINT_DATA ----------------------------------------------------
typedef struct SPrintDataMsg
{
	SMsgHdr		hdr;
	SPageId		id;
	BYTE		data[1];
} SPrintDataMsg;

//--- CMD_PRINT_TEST_DATA ----------------------------------------------------
typedef struct SPrintTestDataMsg
{
	SMsgHdr		hdr;
	UINT32		headNo;
	char		data[MAX_TEST_DATA_SIZE];
} SPrintTestDataMsg;

//--- CMD_LOAD_FILE -------------------------
typedef struct SLoadFileCmd
{
	SMsgHdr		hdr;
	SPageId		id;
	char		filepath[256];
} SLoadFileCmd;

//--- CMD_PRINT_FILE -------------------------
typedef struct SPrintFileCmd
{
	SMsgHdr		hdr;
	SPageId		id;
	UINT32		blkNo;
	UINT32		offsetWidth;
	UINT32		lengthPx;
	UINT32		gapPx;
	UINT32		flags;		// for scanning
		#define FLAG_MIRROR			0x0001
		#define FLAG_BIDIR			0x0002
		#define FLAG_SMP_FIRST_PAGE 0x0004
		#define FLAG_SMP_LAST_PAGE	0x0008
		#define FLAG_SAME			0x0010

	UINT8		virtualPasses;
	UINT8		virtualPass;
	UINT8		penetrationPasses;
	
	UINT8		printMode;	// see rx_def.h::SBmpInfo
		#define PM_UNDEF				0
		#define PM_SCANNING				1
		#define PM_SINGLE_PASS			2
		#define PM_TEST					3
		#define PM_TEST_SINGLE_COLOR	4
		#define PM_TEST_JETS			5
		#define PM_SCAN_MULTI_PAGE		6
	
	UINT16		smp_bufSize;		// PM_SCAN_MULTI_PAGE: buffer size
	UINT8		variable;
	UINT8		lengthUnit;	// see SPrintQueueItem.LengthUnit
	UINT8		clearBlockUsed;
	UINT8		wakeup;
	char		filename[256];
	char		dots[4];
} SPrintFileCmd;

typedef struct SPrintFileRep
{
	SMsgHdr		hdr;
	SPageId		id;
	INT32		offsetWidth;
	UINT32		msgNo;
	UINT32		blkCnt;
	UINT8		bufReady;
	UINT8		same;
	UINT8		clearBlockUsed;
} SPrintFileRep;

typedef struct SPrintFileMsg
{
	SMsgHdr		hdr;
	SPageId		id;
	UINT8		spoolerNo;
	UINT8		evt;
			#define	DATA_RIPPING	1
			#define	DATA_SCREENING	2
			#define DATA_LOADING	3	// bufReady = progress
			#define DATA_SENDING	4
			#define DATA_SENT		5
	//		#define DATA_PRINT_DONE	6
	UINT8		bufReady;
	char		txt[64];
} SPrintFileMsg;

typedef struct SPrintDoneMsg
{
	SMsgHdr		hdr;
	UINT32		pd;
	SPageId		id;
	UINT32		boardNo; // simulation from spooler
} SPrintDoneMsg;

//--- UDP message --------------------------------------------
// #define MAX_UDP_MSG_SIZE	1472
// #define DATA_BLOCK_SIZE		(5*256)
// #define DATA_BLOCK_CNT		(5000*8)

typedef struct SUDPDataBlockMsg
{
	UINT32	blkNo;					// blockNo==0xffffffff: reset all used flags
	UINT8	blkData[DATA_BLOCK_SIZE_JUMBO];
} SUDPDataBlockMsg;

//--- BLOCK_USED --------------------
typedef struct SBlockUsedCmd
{
	SMsgHdr		hdr;
	UINT8		headNo;
	UINT8		id;
	UINT32		blkNo;	// number of first block
	UINT16		blkCnt;	// number of blocks
} SBlockUsedCmd;

// #define MAX_USED_SIZE	1400	std frames
#define MAX_USED_SIZE 8000	// jumbo frames
typedef struct SBlockUsedRep
{ 
	SMsgHdr		hdr;
	UINT32		aliveCnt[2];
	UINT32		blockOutIdx;	// number of the last block used
	UINT32		blkNo;			// BYTE-index of first used bits
	UINT8		headNo;
	UINT8		id;
	UINT16		blkCnt;				// number of bytes
	UINT32		used[MAX_USED_SIZE/4];// maximum size
} SBlockUsedRep;

//--- CMD_FPGA_IMAGE -----------------------------------------
typedef struct SFpgaImageCmd
{
	SMsgHdr		hdr;
	UINT16		head;
	SFpgaImage	image;
	SPageId		id;
} SFpgaImageCmd;

//--- CMD_FPGA_WRITE_BMP --------------------------------------
typedef struct SFpgaWriteBmpCmd
{
	SMsgHdr		hdr;
	UINT16		head;
	UINT16		imageNo;	
} SFpgaWriteBmpCmd;

//--- CMD_HEAD_FLUID_CTRL_MODE ------------------------
typedef struct SFluidCtrlCmd
{
	SMsgHdr			hdr;
	int				no;
	EnFluidCtrlMode	ctrlMode;
} SFluidCtrlCmd;

//--- SFluidTestTable ----------------------------------
typedef struct SFluidTestTable
{
	SMsgHdr			hdr;
	int				index;

} SFluidTestTable;

//--- SScalesCfgMsg ----------------------
typedef struct SScalesCfgMsg
{
	SMsgHdr			hdr;
	INT32			tara[MAX_SCALES];
	INT32			calib[MAX_SCALES];
} SScalesCfgMsg;

//--- SScalesStatMsg ----------------------
typedef struct SScalesStatMsg
{
	SMsgHdr			hdr;
	INT32			val[MAX_SCALES];
} SScalesStatMsg;

//--- data server -------------------
typedef struct SDsOpenCmd
{
	SMsgHdr		hdr;
	char		filepath[MAX_PATH];
} SDsOpenCmd;

typedef struct SDsOpenRep
{
	SMsgHdr		hdr;
	UINT32		file_id;
	UINT64		filetime;	
	UINT64		size;
	char		filepath[MAX_PATH];
} SDsOpenRep;

typedef struct SDsReadCmd
{
	SMsgHdr		hdr;
	UINT32		file_id;
	UINT32		block;
	UINT32		block_size;
	UINT32		cnt;
} SDsReadCmd;

typedef struct SDsReadRep
{
	SMsgHdr		hdr;
	UINT32		file_id;
	UINT32		block;
	BYTE		data[1];
} SDsReadRep;

typedef struct SDsCloseCmd
{
	SMsgHdr		hdr;
	UINT32		file_id;
} SDsCloseCmd;

typedef struct SDsCloseRep
{
	SMsgHdr		hdr;
} SDsCloseRep;

//--- Booting ------------------------------------------------
typedef struct SBootInfoReqCmd
{
	UINT32	cmd;
} SBootInfoReqCmd;

typedef struct SBootInfoMsg
{
	UINT32			id;
	SNetworkItem	item;
} SBootInfoMsg;

typedef struct SBootAddrSetCmd
{
	UINT32			id;
	UINT64			macAddr;
	char			ipAddr[IP_ADDR_SIZE];
} SBootAddrSetCmd;

//=== File System ===============================================
//--- CMD_MAKE_DIR, CMD_DEL_DIR, CMD_FS_GET_DIR,  CMD_DEL_FILE ---
typedef struct SFSDirCmd
{
	SMsgHdr		hdr;
	char		name[256];
} SFSDirCmd;

typedef struct SFSDirEntry
{
	SMsgHdr		hdr;
	char		name[256];
	UINT32		flags;
	UINT32		size;
	UINT64		timeModified;
} SFSDirEntry;

//=== Data Server =============================================

#pragma pack()
