// ****************************************************************************
//
//	TCP_IP_OEM.h
//
//-----------------------------------------------------------------------------
//	Definition of TCP/IP messages.
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************


#ifndef TCP_IP_OEM_H
#define TCP_IP_OEM_H

#ifdef linux
	#include "TECITStd/TECDataTypes.h"

	//--- source: wingdi.h ------------------------
	#define TA_LEFT                     0
	#define TA_RIGHT                    2
	#define TA_CENTER                   6

	#define DEFAULT_CHARSET				1
	#define OUT_DEFAULT_PRECIS          0
	#define CLIP_DEFAULT_PRECIS			0
	#define DEFAULT_QUALITY				0
	#define DEFAULT_PITCH				0
	#define FF_DONTCARE					(0<<4)  /* Don't care or don't know. */

	#define FW_NORMAL           400

#endif

#define TCP_IP_VERSION 		0x00000108

#define TCP_IP_PORT			6000

//--- TELEGRAM IDs ------------------------------------------------------------

#define CMD_START_PRINTING		0x0030	// selects file and starts printing
#define REP_START_PRINTING		0x3000
#define CMD_STOP_PRINTING		0x0031	// stops printing
#define REP_STOP_PRINTING		0x3100
#define CMD_CONFIG				0x0032
#define REP_CONFIG				0x3200
#define CMD_PRINT_POS			0x0033	
#define REP_PRINT_POS			0x3300
#define CMD_PRINT_PAGE			0x0034	// load one page of the selected file
#define REP_PRINT_PAGE			0x3400
#define CMD_CLEAR_BUFFERS		0x0035
#define REP_CLEAR_BUFFERS		0x3500
#define CMD_SET_LAYOUT			0x0036
#define REP_SET_LAYOUT			0x3600
#define CMD_PRINT_DATA			0x0037
#define REP_PRINT_DATA			0x3700
#define CMD_PRINT_PAGE_EX		0x0038	// load one rectangle (x,y,l,h) of the selected file
#define REP_PRINT_PAGE_EX		0x3800

#define CMD_STATUS				0x0040
#define REP_STATUS				0x4000

#define	MSG_PRINT_DONE			0x4100

#define MSG_ERROR				0x4200

#define MSG_CLEAR_ERRORS		0x0043
#define REP_CLEAR_ERRORS		0x4300
#define MSG_STATUS				0x4400
#define MSG_PRINT_PAGE			0x4500

//--- Cleaning station -----------------
#define CMD_CLN_REFERENCE		0x0050	// move the printhead to reference positions
#define REP_CLN_REFERENCE		0x5000	
#define MSG_CLN_REFERENCE_DONE	0x5001

#define CMD_CLN_GOTO_PARK		0x0051	// move printhead to park position
#define REP_CLN_GOTO_PARK		0x5100
#define MSG_CLN_GOTO_PARK_DONE	0x5101

#define CMD_CLN_GOTO_PRINT		0x0052	// move printhead to print position
#define REP_CLN_GOTO_PRINT		0x5200
#define MSG_CLN_GOTO_PRINT_DONE	0x5201

#define CMD_CLN_DO_CLEAN		0x0053	// clean the printhead
#define REP_CLN_DO_CLEAN		0x5300
#define MSG_CLN_CLEAN_DONE		0x5301

#define CMD_CLN_STOP			0x0054	// clean the printhead
#define REP_CLN_STOP			0x5400
#define MSG_CLN_STOP_DONE		0x5401

#define CMD_CLN_WASH			0x0055
#define REP_CLN_WASH			0x5500
#define MSG_CLN_WASH_DONE		0x5501

#define CMD_CLN_DOWN			0x0056
#define REP_CLN_DOWN			0x5600
#define MSG_CLN_DOWN_DONE		0x5601

#define CMD_CLN_EMPTY			0x0057
#define REP_CLN_EMPTY			0x5700
#define MSG_CLN_EMPTY_DONE		0x5701

#define CMD_CLN_FILL			0x0058
#define REP_CLN_FILL			0x5800
#define MSG_CLN_FILL_DONE		0x5801

#define CMD_CLN_SKIP_CLEAN		0x0059
#define REP_CLN_SKIP_CLEAN		0x5900
#define MSG_CLN_SKIP_CLEAN_DONE	0x5901

#define CMD_CLN_CHANGE_PRINT	0x005a	// change the print position parameter

#define CMD_SET_PURGE_PAR		0x005b
#define REP_SET_PURGE_PAR		0x5b00

#define CMD_PRN_SHUT_DOWN		0x0060
#define REP_PRN_SHUT_DOWN		0x6000

#define REPLY_OK			0
#define REPLY_ERROR			1

//--- structures -------------------------------------------------------------
#pragma pack(1)

//--- command CMD_CONFIG -------------------------------------------
typedef struct
{
	ULONG	cmdLen; 		// == sizeof(SConfig)
	ULONG	cmd;			// == CMD_CONFIG

	ULONG  pm_distFirstWindow;	// Print-Mark check: distance from the "Start"-Signal to first "PrintMark", in mm
	ULONG  pm_sizeWindow;		// Print-Mark check: Size of the active window in whitch the print mark is expected, in mm
	ULONG  pm_distWindow;		// Print-Mark check: Distance between 2 print marks, in µm

	ULONG  paperHeight; 		// Paper height in µm 
	ULONG  paperWidth; 			// Paper width in µm 
	
	USHORT watchDogTimer;		// Watch dog timer in second (for compatibilty only, not used)
	USHORT sendReplies; 		// send command replies: default = 1
	USHORT sendPrintDone; 		// send print done after each print: default = 1
	USHORT sendErrorMsg; 		// send Error Message: default = 1
	USHORT disableEventMsg; 	// disables asynchronous sending of the status message (default=0) 
	
	USHORT dpiX; 				// encoder resolution in DPI
	UCHAR  monoDropSize; 		// sets grey level for one bit images, 0=option not used
	UCHAR  pg_mode; 			// reserved
		#define PG_EXTERNAL			0	// print-gos before firepulse is on = ERROR
		#define PG_INTERNAL			1
		#define PG_ABS_FORWARDS		2
		#define PG_ABS_BACKWARDS	3
		#define PG_EXTERNAL_INHIBIT	4	// ignores print-gos before firepulses are on
		#define PG_BIDIR			5
//		#define PG_SCANNING			6
	
	UCHAR scanningPasses;
	UCHAR Param11; 				// reserved
	USHORT Param12; 			// reserved
} SCmdConfig;

typedef struct
{
	ULONG	cmdLen;	// == sizeof(SRepConfig)
	ULONG	cmd;

	short	reply;
} SRepConfig;

//--- command CMD_PRINT_POS -----------------------------------
typedef struct
{
	ULONG	cmdLen;	// == sizeof(SCmdPrintPos)
	ULONG	cmd;	//	== CMD_PRINT_POS
	
	long	posx;	// distance in encoder direction [µm]
	long	posy;	// distance in slide direction [µm], use CMD_CLN_POSITION for larger distances
} SCmdPrintPos;

typedef struct
{
	ULONG	cmdLen;	// == sizeof(SRepPrintPos)
	ULONG	cmd;	// == REP_PRINT_POS

	short	reply;
} SRepPrintPos;

//--- command CMD_START_PRINTING ---------------------
typedef struct
{
	ULONG	cmdLen;	// == sizeof(SCmdStart)
	ULONG	cmd;	// == CMD_START_PRINTING
} SCmdStart;

typedef struct
{
	ULONG	cmdLen;	// == sizeof(SRepStart)
	ULONG	cmd;	// == REP_START_PRINTING

	short	reply;
} SRepStart;

//--- command CMD_STOP_PRINTING ---------------------------
typedef struct
{
	ULONG	cmdLen;	// == sizeof(SCmdStop)
	ULONG	cmd;	// == CMD_STOP_PRINTING
} SCmdStop;

typedef struct
{
	ULONG	cmdLen;	// == sizeof(SRepStop)
	ULONG	cmd;	// == REP_STOP_PRINTING

	short	reply;
} SRepStop;

//--- command CMD_CLEAR_BUFFERS ---------------------------
typedef struct
{
	ULONG	cmdLen;	// == sizeof(SCmdClearBuffers)
	ULONG	cmd;	// == CMD_CLEAR_BUFFERS
} SCmdClearBuffers;

typedef struct
{
	ULONG	cmdLen;	// == sizeof(SRepClearBuffers)
	ULONG	cmd;	// == REP_STOP

	short	reply;
} SRepClearBuffers;

//--- command CMD_SHUT_DOWN ---------------------------
typedef struct
{
	ULONG	cmdLen;	// == sizeof(SCmdShutDown)
	ULONG	cmd;	// == CMD_SHUT_DOWN
} SCmdShutDown;

typedef struct
{
	ULONG	cmdLen;	// == sizeof(SRepShutDown)
	ULONG	cmd;	// == REP_SHUT_DOWN

	short	reply;
} SRepShutDown;

//--- command CMD_CLEAR_ERRORS ---------------------------
typedef struct
{
	ULONG	cmdLen;	// == sizeof(SCmdClearErrors)
	ULONG	cmd;	// == CMD_CLEAR_ERRORS
} SCmdClearErrors;

typedef struct
{
	ULONG	cmdLen;	// == sizeof(SRepClearErrors)
	ULONG	cmd;	// == REP_ERRORS

	short	reply;
} SRepClearErrors;

//--- command CMD_PRINT_PAGE -----------------------------------------------
typedef struct
{
	ULONG	cmdLen;		// == sizeof(SCmdPrintPage)
	ULONG	cmd;		// = CMD_PRINT_PAGE
	
	ULONG	printId;	// is returned by MSG_PRINT_DONE message
	char	filepath[128];// local file path: in multi-color systems the file end is adapted by the system  
	ULONG	firstPage;	// for multi-page documents: first page to print (starts with 1)  
	ULONG	lastPage;	// for multi-page documents: last page to print (0=only one page)
	long	copies;		// number of copies of this document (negative=meters)
} SCmdPrintPage;

//--- command CMD_PRINT_PAGE_EX -----------------------------------------------
typedef struct
{
	ULONG	cmdLen;		// == sizeof(SCmdPrintPage)
	ULONG	cmd;		// = CMD_PRINT_PAGE_EX
	
	ULONG	printId;		// is returned by MSG_PRINT_DONE message
	char	filepath[128];	// local file path: in multi-color systems the file end is adapted by the system  
	ULONG	firstPage;		// for multi-page documents: first page to print (starts with 1)  
	ULONG	lastPage;		// for multi-page documents: last page to print (0=only one page)
	long	copies;		// number of copies of this document

	ULONG	x_pos;			// [µm]
	ULONG	y_pos;			// [µm]
	ULONG	length;			// [µm]
	ULONG	height;			// [µm]
} SCmdPrintPageEx;

typedef struct
{
	ULONG	cmdLen;	// == sizeof(SRepPrintPage)
	ULONG	cmd;	// == REP_PRINT_PAGE

	short	reply;
} SRepPrintPage;

//--- command CMD_STATUS ---------------------------------------
typedef struct
{
	ULONG	cmdLen;	// == sizeof(SCmdStatus)
	ULONG	cmd;	// == CMD_STATUS
} SCmdStatus;

typedef struct
{
	ULONG	cmdLen;			// == sizeof(SRepStatus)
	ULONG	cmd;			// == REP_STATUS
 
    ULONG	version; 			// for compatibility only (not used)
    char	szFabId[64]; 		// for compatibility only (not used)
    ULONG	enginePos; 			// for compatibility only (not used)
    ULONG	statusBits; 		// Main Status bits word

		#define	SI_ST_LOCAL		 0x00000001 // 0 Local: not used
		#define	SI_ST_REMOTE 	 0x00000002 // 1 Remote: not used
		#define SI_ST_IDLE 		 0x00000004 // 2 Idle: print engine is off or in error condition
		#define SI_ST_READY 	 0x00000008 // 3 Ready: print engine is ready to print (encoder enabled, waiting for print-go)
		#define SI_ST_STARTING 	 0x00000010 // 4 Starting: print engine is staring
		#define SI_ST_EMPTY 	 0x00000020 // 5 Data Buffer Empty:
		#define SI_ST_PRINTING 	 0x00000040 // 6 Printing: not used
		#define SI_ST_STOPPING 	 0x00000080 // 7 Stopping: printing, restricted checks (print go but no data)
		#define SI_ST_WARMING_UP 0x00000100 // 8 Warming up: 
		#define SI_ST_STANDBY	 0x00000200 // 9
		#define SI_ST_RES2		 0x00000400	// 10
		#define SI_ST_RES3		 0x00000800 // 11
		#define SI_ST_E_STOP	 0x00001000 // 12 Emergency Stop
		#define SI_ST_RES4		 0x00002000 // 13 
		#define SI_ST_RES5		 0x00004000 // 14
		#define SI_ST_RES6		 0x00008000 // 15
		#define SI_ST_RES7		 0x00010000 // 16 
		#define SI_ST_NEW_MSG	 0x00020000 // 17 new message
		#define SI_ST_NEW_ERR1	 0x00040000 // 18 new error1
		#define SI_ST_NEW_WARN	 0x00080000 // 19 new warning
		#define SI_ST_NEW_ERR2	 0x00100000 // 20 new error 2

    ULONG	printedPage;		// Last printed page number
    ULONG	messageBits; 		// Message bits word; see ErrMsg.h
    ULONG	warningBits; 		// Warning bits word; see ErrMsg.h
    ULONG	error1Bits; 		// Error bits Word 1; see ErrMsg.h
    ULONG	error2Bits; 		// Error bits Word 2; see ErrMsg.h
    ULONG	Resv1; 				// Reserve;
    ULONG	Resv2; 				// Reserve;
    ULONG	Resv3; 				// Reserve;
    ULONG	Resv4; 				// Reserve;
} SRepStatus;

//--- message MSG_PRINT_DONE MSG_PRINT_PAGE ------------------------------
typedef struct
{
	ULONG	cmdLen;	// == sizeof(SMsgPrintDone)
	ULONG	cmd;	// == MSG_PRINT_DONE

	ULONG	printId;
	ULONG	page;
} SMsgPrintDone;

//--- message MSG_ERROR -----------------------------------
typedef struct
{
	ULONG	cmdLen;	// == sizeof(SMsgError)
	ULONG	cmd;	// == MSG_ERROR
	UCHAR	deviceType;
		#define DEV_UNDEF	0x00
		#define DEV_PC01	0x01
		#define DEV_MAIN	0x02
		#define DEV_CLEAN	0x03
		#define DEV_HUB		0x04
		#define DEV_PNM		0x05
		#define DEV_PNM_EXT	0x06
		#define DEV_HEAD	0x07
		#define DEV_SLAVE	0x08

	UCHAR	deviceNo;
	short	errNo;

	char	errStr[256];

	UCHAR	errType;
		#define ERRTYPE_LOG		0
		#define ERRTYPE_WARN	1
		#define ERRTYPE_FATAL	2
} SMsgError;

//--- Printhead movement ----------------------------------
typedef struct
{
	ULONG	cmdLen;	// == sizeof(SCmdClnMove)
	ULONG	cmd;	// == CMD_CLN_REFERENCE / CMD_CLN_GOTO_PARK / CMD_CLN_GOTO_PRINT / CMD_CLN_DO_CLEAN
} SCmdClnMove;

typedef struct
{
	ULONG	cmdLen;	// == sizeof(SCmdClnMove)
	ULONG	cmd;	// == CMD_CLN_CHANGE_PRINT
	ULONG	pos;
} SCmdClnChange;

typedef struct
{
	ULONG	cmdLen;	// == sizeof(SCmdClnMove)
	ULONG	cmd;	// == CMD_SET_PURGE_PAR
	ULONG	level;
		#define PURGE_LEVEL_SHORT	0
		#define PURGE_LEVEL_REGULAR	1
		#define PURGE_LEVEL_STRONG	2
		#define PURGE_LEVEL_HARD	3
} SCmdClnPurgePar;

typedef struct
{
	ULONG	cmdLen;	// == sizeof(SCmdClnMove)
	ULONG	cmd;	// == REP_CLN_REFERENCE / REP_CLN_GOTO_PARK / REP_CLN_GOTO_PRINT / REP_CLN_DO_CLEAN
	short	reply;		
} SRepClnMove;

typedef struct
{
	ULONG	cmdLen;	// == sizeof(SCmdClnMove)
	ULONG	cmd;	// == MSG_CLN_REFERENCE_DONE / MSG_CLN_PARK_DONE / REP_CLN_PRINT_DONE / REP_CLN_CLEAN_DONE
	short	reply;	// 1=abort , 2=error, 4=ok
		#define DONE_ABORT	0x01
		#define DONE_ERROR	0x02
		#define DONE_OK		0x04
} SMsgClnMove;


//--------- layout -------------------------------------

#define LAYOUT_CNT	1
#define BOX_TYPE_TEXT		0
#define BOX_TYPE_BARCODE	1
#define BOX_TYPE_BITMAP		2

#define BOX_MIRROR_OFF		0
#define BOX_MIRROR_HORIZ	1
#define BOX_MIRROR_VERT		2

typedef struct
{
	SHORT		boxType;
	RECT		rect;
	USHORT		mirror;
} SBoxHeader;

#define TA_NEW_LINE		32

typedef struct
{
	SBoxHeader	hdr;
	POINT		pos;
	float		dx;			// linespace -X
	float		dy;			// linespace -Y
	USHORT		alignment;	// TA_LEFT | TA_RIGHT | TA_CENTER | TA_NEW_LINE
	USHORT		extraSpace;
	LOGFONTA	font;
} STextBox;

typedef struct
{
	SBoxHeader	hdr;
	//--- general -----
	SHORT		bcType;			// e_BarCType
	SHORT		orientation;	// e_Degree	
	USHORT		bar[8];			// bar width
	USHORT		space[8];		// space width
	USHORT		stretch;		// module for Matrix codes
	USHORT		filter;
	short		codePage;

	//--- Paramaters for TEC-It Library ---------------------------------------------------------------------------------
	//-------------------------	DM			QR			MicroQR			CodeBlock	MicroPDF	PDF			Aztec
	USHORT		check;		//				e_QRECLevel	
	USHORT		size;		//	e_DMSizes	e_QRVersion								e_MPDFVers
	USHORT		format;		//	e_DMFormat	e_QRFormat	e_MQRVersion	e_CBFFormat	e_MPDFMode	e_PDFMode	e_AztecFormat
	USHORT		mask;		//	Encoding Mode	x
	USHORT		rows;		//																	x
	USHORT		cols;		//																	x

	USHORT		ccType;
	USHORT		segPerRow;	// RSS
	USHORT		wchar;		// data as wchar_t string

	//--- human readable ------------------
	LOGFONTA	font;
	short		above;
	short		dist;
} SBarcodeBox; 

typedef struct
{
	SBoxHeader	hdr;
} SBitmapBox;

typedef struct
{
	ULONG	cmdLen;	// == sizeof(SCmdClnMove)
	ULONG	cmd;	// == MSG_CLN_REFERENCE_DONE / MSG_CLN_PARK_DONE / REP_CLN_PRINT_DONE / REP_CLN_CLEAN_DONE
	USHORT	layoutNo;
	USHORT	boxCnt;
	BYTE	data[1];	// generic length
	//		(<STextBox> | <SBarcodeBox>)[boxCnt]
	//	
} SMsgLayout;

typedef struct
{
	ULONG	cmdLen;	// == sizeof(SCmdClnMove)
	ULONG	cmd;	// == MSG_CLN_REFERENCE_DONE / MSG_CLN_PARK_DONE / REP_CLN_PRINT_DONE / REP_CLN_CLEAN_DONE
	short	reply;
} SRepLayout;

typedef struct
{
	ULONG	cmdLen;	// == sizeof(SCmdClnMove)
	ULONG	cmd;	// == CMD_PRINT_DATA
	ULONG	dataId;

	//--- background file ------------------
	char	filepath[128];	// local path of a GTJ-File
	ULONG	page;			// page number of the GTJ-File

	//--- variable data ----------------------
	USHORT	layoutNo;
	USHORT	boxCnt;
	wchar_t	data[1]; // generic length	
	// data  = BOX[boxCnt]
	//	BOX	 = <(USHORT)lineCnt> LINE[lineCnt]
	//	LINE = <(USHORT)lineLen> (BYTE)byte[lineLen]
} SMsgPrintData;

#pragma pack()

#endif