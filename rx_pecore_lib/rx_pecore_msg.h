// ****************************************************************************
//
//	rx_pecore_msg.h
//
// ****************************************************************************
//
//	Copyright 2018 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Hubert Zimmermann
//
// ****************************************************************************

#pragma once

//--- Includes ----------------------------------------------------------------
#include "rx_common.h"

//--- Defines -----------------------------------------------------------------

#define	MVT_PRT_REQ_PING	0			// CMD & STATUS, DATA
#define	MVT_PRT_ANS_PING	0

#define	MVT_PRT_REQ_CONF	1			// CMD & STATUS only
#define	MVT_PRT_ANS_CONF	2

#define	MVT_PRT_REQ_OPEN	3			// CMD & STATUS, DATA
#define	MVT_PRT_ANS_OPEN	4

#define	MVT_PRT_REQ_CLOSE	5			// CMD & STATUS, DATA
#define	MVT_PRT_ANS_CLOSE	6

#define MVT_PRT_REQ_START	7			// CMD & STATUS only
#define	MVT_PRT_ANS_START	8

#define	MVT_PRT_REQ_STOP	9			// CMD & STATUS only
#define	MVT_PRT_ANS_STOP	10

#define	MVT_PRT_REQ_CMD		11			// CMD & STATUS only
#define	MVT_PRT_ANS_CMD		12

#define	MVT_PRT_REQ_SYNC	13			// CMD & STATUS only
#define	MVT_PRT_ANS_SYNC	14

#define	MVT_PRT_REQ_ADJ		15			// CMD & STATUS only
#define	MVT_PRT_ANS_ADJ		16

#define	MVT_PRT_REQ_CMDPG	17			// CMD & STATUS only
#define	MVT_PRT_ANS_CMDPG	18

#define	MVT_PRT_REQ_DATAPG	19			// DATA only
#define	MVT_PRT_ANS_DATAPG	20

#define	MVT_PRT_REQ_STATE	21			// CMD & STATUS only
#define	MVT_PRT_ANS_STATE	22

#define	MVT_PRT_REQ_FILECMD	23			// CMD & STATUS only
#define	MVT_PRT_ANS_FILECMD	24

#define	MVT_PRT_REQ_DRAWCMD	25			// CMD & STATUS only
#define	MVT_PRT_ANS_DRAWCMD	26

#define	MVT_PRT_REQ_LAYOUTCMD	27		// CMD & STATUS only
#define	MVT_PRT_ANS_LAYOUTCMD	28

#define	MVT_PRT_REQ_RECORDDATA	29		// CMD & STATUS only
#define	MVT_PRT_ANS_RECORDDATA	30


/*
#define	MVT_PRT_REQ_
#define	MVT_PRT_ANS_
*/

//--- Structures, Typedefs ----------------------------------------------------

//--- HEADER
typedef struct mvt_prt_header {
	INT32	length;							/* Length of body (can be 0) */
	UINT32	id;								/* Cmd Id */
} Smvt_prt_header;
// Same format as SMsgHdr file tcp_ip.h

//--- PING
#define	PING_VER		1

struct mvt_prt_ping_req_v1 {
	Smvt_prt_header hdr;
	INT32	ver;							/* Version of this Cmd Id (starts from 1)*/
};

struct mvt_prt_ping_ans_v1 {
	Smvt_prt_header hdr;
	INT32	ver;							/* Version of this Cmd Id (starts from 1)*/
};

//--- CONFIGURATION
#define CONFIG_VER		1

struct mvt_prt_config_req_v1 {
	Smvt_prt_header hdr;
	INT32	ver;							/* Version of this Cmd Id (starts from 1)*/
};

struct mvt_prt_config_ans_v1 {
	Smvt_prt_header hdr;
	INT32	ver;							/* Version of this Cmd Id (starts from 1)*/
	char	id[32];							/* Printer Id (example: Mouvent LB701 UV) */
	INT32	resoHead;						/* Resolution in head direction (width) (dpi) (1200) */
	INT32	resoPrint;						/* Resolution in print direction (height) (dpi) (1200) */
	INT32	maxWidth;						/* Maximal size of bitmap in head direction (in pixels) */
	INT32	maxHeight;						/* Maximal size of bitmap in print direction (in pixels) */
	INT32	maxSpeed;						/* Maximal paper speed (in m/min) */
	INT32	duplex;							/* Duplex: 0: no, 1: yes */
	INT32	dropSizeCount;					/* Max number of drop sizes (3) */
	INT32	bitPerPixel;					/* Max bit per pixel supported (2) */
	INT32	alignment;						/* Alignment in bits (8 or multiple of 8) */
	INT32	speedPassCount;					/* Printing passes nb (min 1) to mutilply the speed*/
	INT32	colorCount;						/* Number of colors */
	INT32 	bitmapCount;					/* Number of all bitmaps from all interfaces */
	INT32	interfCount;					/* Number of interfaces */
};
/* Followed by  mvt_prt_colors[colorCount]			Description of colors,
				mvt_prt_bitmap[bitmapCount]			Description of bitmaps and
				mvt_prt_interface[interfCount]		Description of interfaces */

struct mvt_prt_color {
	INT32	index;							/* Color index (from 0 to colorCount-1) */
	INT32	colorCode;
};

struct mvt_prt_bitmap {
	INT32	index;							/* Bitmap index (starts on 0) */
	INT32	interf;							/* Interface index */
	INT32	side;							/* Side of paper (0=front, 1=back) */
	INT32	offset;							/* Pixel offset (head direction) */
	INT32	width;							/* Bitmap width (head direction) */
	INT32	speedPass;						/* Speed print pass index (starts on 0) */
	INT32	color;							/* Color index */
};

struct mvt_prt_interface {
	INT32	index;							/* Interface index (from 0 to interfaceCount-1) */
	INT32	connected;						/* state 0: not connected, 1 connected */
	char	ip[32];							/* Ip address (example: 192.168.200.201) */
	INT32	port;							/* Port number (example: 9001) */
};

//--- OPEN SESSION
#define OPEN_VER		1

struct mvt_prt_session_req_v1 {
	Smvt_prt_header hdr;
	INT32	ver;							/* Version of this Cmd Id (starts from 1)*/
	INT32	id;								/* unique id: must be different than 0 */
	};

struct mvt_prt_session_ans_v1 {
	Smvt_prt_header hdr;
	INT32	ver;							/* Version of this Cmd Id (starts from 1)*/
	INT32	id;								/* session is open, if not 0 */
	};

//--- CLOSE SESSION
#define	CLOSE_VER		1

struct mvt_prt_close_req_v1 {
	Smvt_prt_header hdr;
	INT32	ver;							/* Version of this Cmd Id (starts from 1)*/
};
struct mvt_prt_close_ans_v1	{
	Smvt_prt_header hdr;
	INT32	ver;							/* Version of this Cmd Id (starts from 1)*/
};

//--- PRINT START
#define PRINTSTART_VER	1

struct mvt_prt_start_req_v1 {
	Smvt_prt_header hdr;
	INT32	ver;							/* Version of this Cmd Id (starts from 1)*/
};

struct mvt_prt_start_ans_v1 {
	Smvt_prt_header hdr;
	INT32	ver;							/* Version of this Cmd Id (starts from 1)*/
	INT32	cmd_status;						/* 0= error, 1= acknowledged */
};

//--- PRINT STOP
#define PRINTSTOP_VER	1

struct mvt_prt_stop_req_v1 {
	Smvt_prt_header hdr;
	INT32	ver;							/* Version of this Cmd Id (starts from 1)*/
	INT32	page_idx;
};

struct mvt_prt_stop_ans_v1 {
	Smvt_prt_header hdr;
	INT32	ver;							/* Version of this Cmd Id (starts from 1)*/
	INT32	cmd_status;						/* 0= error, 1= acknowledged */
};

//--- PRINT COMMAND
#define	PRINTCMD_VER	1
#define	PRINTCMD_PAPERSTART	0
#define	PRINTCMD_PAPERSTOP	1
#define	PRINTCMD_PAPERFEED	2

typedef struct mvt_prt_cmd_req_v1 {
	Smvt_prt_header hdr;
	INT32	ver;							/* Version of this Cmd Id (starts from 1)*/
	INT32	id;								/* Prt Cmd Id */
	INT32	length;							/* Length of body (can be 0) */
} Smvt_prt_cmd_req_v1;
		/* Followed by the command */

struct mvt_prt_cmd_ans_v1 {
	Smvt_prt_header hdr;
	INT32	ver;							/* Version of this Cmd Id (starts from 1)*/
	INT32	cmd_status;						/* 0= error, 1= acknowledged */
};

#define Smvt_prt_cmd	Smvt_prt_cmd_req_v1

//--- PRINT SYNCHRO COMMAND
#define PRINTSYNC_VER	1

struct mvt_prt_sync {
	UINT32 printGoMode;				   		/* Printgo mode */
	UINT32 printGoDistance;					/* distance between 2 printgo in µm */
	UINT32 printGoGap;						/* Gap after page, in µm */
};

typedef struct mvt_prt_jobsync_req_v1 {
	Smvt_prt_header hdr;
	INT32	ver;							/* Version of this Cmd Id (starts from 1)*/
	INT32	Width;							/* Max width in µm*/
	struct mvt_prt_sync rectoSync;			/* Description recto sync */
	struct mvt_prt_sync versoSync;			/* Description recto sync */
} Smvt_prt_job_req_v1;

struct mvt_prt_jobsync_ans_v1 {
	Smvt_prt_header hdr;
	INT32	ver;							/* Version of this Cmd Id (starts from 1)*/
	INT32	cmd_status;						/* 0= error, 1= acknowledged */
};

#define	Smvt_prt_synchro Smvt_prt_job_req_v1	

//--- PRINT ADJUST
#define	PRINTADJ_VER	1

typedef struct mvt_prt_adjust_req_v1 {
	Smvt_prt_header hdr;
	INT32	ver;							/* Version of this Cmd Id (starts from 1)*/
	INT32	RectoX;							/* X adjustment of Recto page (in µm) */
	INT32	RectoY;							/* Y adjustment of Recto page (in µm) */
	INT32	VersoX;							/* X adjustment of Verso page (in µm) */
	INT32	VersoY;							/* Y adjustment of Verso page (in µm) */
} Smvt_prt_adjust_req_v1;

struct mvt_prt_adjust_ans_v1 {
	Smvt_prt_header hdr;
	INT32	ver;							/* Version of this Cmd Id (starts from 1)*/
	INT32	cmd_status;						/* 0= error, 1= acknowledged */
};

#define	Smvt_prt_adjust Smvt_prt_adjust_req_v1 

//--- LOAD PAGE CMD
#define	CMDPG_VER		1

typedef struct mvt_prt_loadpagecmd_req_v1 {
	Smvt_prt_header hdr;
	INT32	ver;							/* Version of this Cmd Id (starts from 1)*/
	INT32	job_id;							/* id of the job known by the sender */
	INT32	page_id;						/* id of the page known by the sender */
	INT32	copyCount;						/* Number copy to print of this page (Default: 1) */
	INT32	Width;							/* Width of the page in pixel*/
	INT32	Height;							/* Height of the page in pixel */
	INT32	lineLen;						/* line length (in bytes) to support alignment */
	INT32	bitPerPixel;					/* bits per pixel 1 or 2 */
	INT32	alignment;						/* Alignment in bits (8 or multiple of 8) */
	INT32	bitmapCount;					/* Number of all bitmaps */
} Smvt_prt_loadpagecmd_req_v1;

struct mvt_prt_loadpagecmd_ans_v1 {
	Smvt_prt_header hdr;
	INT32	ver;							/* Version of this Cmd Id (starts from 1)*/
	INT32	page_idx;						/* 0= error, else index of the 1st copy of this page */
	INT32	copyCount;					 	/* copy count of this page (Default: 1) */
};
#define Smvt_prt_loadpagecmd Smvt_prt_loadpagecmd_req_v1

//--- LOAD PAGE DATA
#define	DATAPG_VER		1

#define	MVT_COMP_NONE	0
#define	MVT_COMP_LZW	1
#define	MVT_COMP_TIFF	2

typedef struct mvt_prt_loadpagedata_req_v1 {
	Smvt_prt_header hdr;
	INT32	ver;							/* Version of this Cmd Id (starts from 1)*/
	INT32	page_idx;						/* index of the page */
	INT32	job_id;							/* id of the job known by the sender */
	INT32	page_id;						/* id of the page known by the sender */
	INT32	copyCount;						/* Number copy to print of this page (Default: 1) */
	INT32	Width;							/* Width of the page */
	INT32 	lineLen;						/* line length (in bytes) to support alignment */
	INT32	Height;							/* Height of the page */
	INT32	bitPerPixel;					/* bits per pixel 1 or 2 */
	INT32	alignment;						/* Alignment in bits (8 or multiple of 8) */
	INT32	bitmapCount;					/* Number of bitmaps of this data link */
} Smvt_prt_loadpagedata_req_v1;
	/* Followed by mvt_prt_bmpdata[bitmapCount]	Description of bitmaps */

struct mvt_prt_bmpdata {
	INT32	index;		/* Bitmap index (0 to cumulated bitmapCount-1) */
	INT32 	compression;	/* 0: none, 1: FastLZ, 2: Tiff */
	INT32 	dataSize;	/* size of the buffer */
};
	/* Followed by the bitmap(compressed or not compressed) */

struct mvt_prt_loadpagedata_ans_v1 {
	Smvt_prt_header hdr;
	INT32	ver;							/* Version of this Cmd Id (starts from 1)*/
	INT32	cmd_status;						/* 0= error, 1= acknowledged */
};
#define Smvt_prt_loadpagedata Smvt_prt_loadpagedata_req_v1

//--- PRINTER STATE
#define	STATE_VER		1

struct mvt_prt_state_req_v1 {
	Smvt_prt_header hdr;
	INT32	ver;							/* Version of this Cmd Id (starts from 1)*/
	UINT32	watchTime;
};

typedef struct mvt_prt_page {
	INT32	idx;							/* index of the page */
	INT32	job_id;							/* id of the job known by the sender */
	INT32	page_id;						/* id of the page known by the sender */
	INT32	copy;							/* copy number (could be >1 if page is duplicated) */
} Smvt_prt_page;

typedef struct	mvt_prt_state_ans_v1 {
	Smvt_prt_header hdr;
	INT32	ver;							/* Version of this Cmd Id (starts from 1)*/
	INT32	watchtime;						/* 0: no watch time, else in MS Printer State occurrence */
	INT32	session_id;						/* session id if open, if not 0 */
	INT32	state;							/* 0, stopped, 1 printing, 2 feeding, …*/
	INT32	error;							/* 0 no error, 1 minor, 2 restart, 3 fatal, 4 failure. */
	INT32	speed;							/* measured speed of printer (in mm/min) */
	Smvt_prt_page rcv_pg;					/* last fully received page */
	Smvt_prt_page sch_pg;					/* last scheduled page */
	Smvt_prt_page cmt_pg;					/* last committed page */
	Smvt_prt_page prt_pg;					/* last printed page */
	Smvt_prt_page stk_pg;					/* last stacked page */
	INT32	interfCount;					/* Number of interfaces */
} Smvt_prt_state_ans_v1;
	/* Followed by mvt_prt_interfstate[interfCount]		Interface status */

struct mvt_prt_interfstate {
	INT32	connected;						/* interface state: 0 not connected, 1: connected*/
	INT32	sessionCount;					/* session open count */
};

#define Smvt_prt_state Smvt_prt_state_ans_v1

//--- LOAD FILE COMMAND
#define LOADFILE_VER		1

typedef struct mvt_prt_loadfilecmd_req_v1 {
	Smvt_prt_header hdr;
	INT32	ver;							/* Version of this Cmd Id (starts from 1)*/
	INT32	job_id;							/* id of the job known by the sender */
	char	filepath[256];					/* relative file path from ripped_data printer dir */
	INT32	pageCount;						/* Number of all pages */
	INT32	pageStart;						/* page number for start (default: 1) */
	INT32	pageEnd;						/* page number for end (default: pageCount) */
	INT32	copyCount;						/* Number of copies to print (default: 1) */
} Smvt_prt_loadfilecmd_req_v1;

struct mvt_prt_loadfilecmd_ans_v1 {
	Smvt_prt_header hdr;
	INT32	ver;							/* Version of this Cmd Id (starts from 1)*/
	INT32	page_idx;						/* 0= error, else index of the 1st copy of this page */
	INT32	copyCount;					 	/*  (1 + pageEnd – pageStart) * copyCount */
};

#define Smvt_prt_loadfile Smvt_prt_loadfilecmd_req_v1 

//--- DRAW FILE COMMAND
#define	DRAWCMD_VER		1

typedef struct mvt_prt_drawcmd_req_v1 {
	Smvt_prt_header hdr;
	INT32	ver;							/* Version of this Cmd Id (starts from 1)*/
	INT32	job_id;							/* id of the job known by the sender */
	INT32	page_id;						/* id of the page known by the sender */
	char	filepath[256];					/* relative file path from ripped_data printer dir */
	INT32	ovlPage;						/* page number to use as overlay */
	INT32	copyCount;						/* Number of copies to print (default: 1) */
	INT32	orderCount;						/* Number of drawing orders  */
} Smvt_prt_drawcmd_req_v1;
	/* Followed by mvt_prt_drawOrder[orderCount]	Description of drawing orders */

struct mvt_prt_drawOrder {
	INT32	index;							/* Order index (0 to cumulated orderCount-1) */
	INT32	image_id;						/* image_id to use as pattern */
	INT32	side;							/* side: 0: front, 1: back */
	INT32	Xpos;							/* anchor point X value (in pixels) */
	INT32	Ypos;							/* anchor point Y value (in pixels) */
	INT32	width;							/* width  (in pixels) */
	INT32	height;							/* height (in pixels) */
	INT32	fill;							/* 0 no, 1 : yes, pattern is repeated to fill*/
	INT32 	compression;					/* compression: 0: none, 1: FastLZ , 2: Tiff*/
	INT32 	dataSize;						/* size of the bitmapmask */
};
	/* Followed by the bitmapmask, 1 bit per pixel, alignment = 8 (compressed or not compressed) */

struct mvt_prt_drawcmd_ans_v1 {
	Smvt_prt_header hdr;
	INT32	ver;							/* Version of this Cmd Id (starts from 1)*/
	INT32	page_idx;						/* 0= error, else index of the page */
};

#define Smvt_prt_drawcmd Smvt_prt_drawcmd_req_v1 

//--- LOAD LAYOUT COMMAND
#define LAYOUTCMD_VER	1

typedef struct mvt_prt_loadlayoutcmd_req_v1 {
	Smvt_prt_header hdr;
	INT32	ver;							/* Version of this Cmd Id (starts from 1)*/
	INT32	job_id;							/* id of the job known by the sender */
	char	filepath[256];					/* relative file path from ripped_data printer dir */
	INT32	recordCount;					/* Number of all records */
	INT32	recordStart;					/* Record number for start (default: 1) */
	INT32	recordEnd;						/* Record number for end (default: recordCount) */
	INT32	copyCount;						/* Number of copies to print (default: 1) */
} Smvt_prt_loadlayoutcmd_req_v1;

struct mvt_prt_loadlayoutcmd_ans_v1 {
	Smvt_prt_header hdr;
	INT32	ver;							/* Version of this Cmd Id (starts from 1)*/
	INT32	page_idx;						/* 0= error, else index of the 1 st page */
	INT32	copyCount;					 	/*  (1 + recordEnd – recordStart) * copyCount */
};

#define Smvt_prt_layoutcmd Smvt_prt_loadlayoutcmd_req_v1 

//--- LOAD RECORD DATA
#define	RECORDDATA_VER	1

typedef struct mvt_prt_loadrecorddata_req_v1 {
	Smvt_prt_header hdr;
	INT32	ver;							/* Version of this Cmd Id (starts from 1)*/
	INT32	job_id;							/* id of the job known by the sender */
	INT32	record_id;						/* id of the record known by the sender */
	INT32	copyCount;						/* Number of copies to print of this record (Default: 1) */
	INT32 	recordSize;						/* size of the record */
} Smvt_prt_loadrecorddata_req_v1;
		/* Followed by the record */

struct mvt_prt_loadrecorddata_ans_v1{
	Smvt_prt_header hdr;
	INT32	ver;							/* Version of this Cmd Id (starts from 1)*/
	INT32	page_idx;						/* 0= error, else index of the 1 st page */
};

#define Smvt_prt_recorddata Smvt_prt_loadrecorddata_req_v1 

// ****************************************************************************
