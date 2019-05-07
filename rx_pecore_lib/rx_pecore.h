// ****************************************************************************
//
//	rx_pecore.h
//
// ****************************************************************************
//
//	Copyright 2018 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Hubert Zimmermann
//
// ****************************************************************************

#include "rx_def.h"
#include "rx_sok.h"
#include "rx_threads.h"
#include "rx_error.h"
#include "rx_trace.h"
#include "rx_fifo.h"
#include "rx_list.h"
#include "rx_pecore_lib.h"
#include "rx_pecore_msg.h"

//--- Defines, Macros ---------------------------------------------------------
#define	MVT_CORE_INTERNAL	(1<<31)			/* Internal Pe Comm (Main <-> Interface) */

#define	MVT_CORE_REQ_CONF		101
#define	MVT_CORE_ANS_CONF		102

#define	MVT_CORE_REQ_STATE		103
#define	MVT_CORE_ANS_STATE		104

#define MVT_CORE_REQ_CMDPG		105
#define MVT_CORE_ANS_CMDPG		106

#define MVT_CORE_REQ_FILECMD	107
#define MVT_CORE_ANS_FILECMD	108

#define MVT_CORE_REQ_DRAWCMD	109
#define MVT_CORE_ANS_DRAWCMD	110

#define MVT_CORE_REQ_LAYOUTCMD	111
#define MVT_CORE_ANS_LAYOUTCMD	112

#define MVT_CORE_REQ_RECORDDATA	113
#define MVT_CORE_ANS_RECORDDATA	114

#define	MVT_CORE_REQ_MAINSTATE	115
#define MVT_CORE_ANS_MAINSTATE	116

#ifdef _SIMULATOR
#define MVT_CORE_REQ_MAINSPEED	117
#define MVT_CORE_ANS_MAINSPEED	118
#endif

#define	MAX_RIPCOUNT			32

//--- Strutures, Typedefs -----------------------------------------------------

//--- INTERFACE CONFIG
#define		CORE_CONFIG_VER		1
struct mvt_core_conf_req_v1 {
	Smvt_prt_header hdr;
	INT32	ver;							/* Version of this Cmd Id (starts from 1) */
	INT32	interfNum;						/* Interface number (starts from 0) */
	char	ip[32];							/* Ip address of this interface */
	INT32	port;							/* Data port of this interface */
};

struct mvt_core_conf_ans_v1 {
	Smvt_prt_header hdr;
	INT32	ver;
	INT32	cmd_status;
};

//--- INTERFACE STATE
#define	CORE_STATE_VER		1

typedef struct	mvt_core_state_req_v1 {
	Smvt_prt_header hdr;
	INT32	ver;							/* Version of this Cmd Id (starts from 1)*/
	INT32	watchtime;						/* 0: no watch time, else in MS interface State occurrence */
	INT32	session_id;						/* session id if open, if not 0 */
	INT32	state;							/* 0, stopped, 1 printing, 2 feeding, …*/
	INT32	sessionCount;					/* opened session count */
	INT32	connectedRipCount;				/* Connected rip count */
	Smvt_prt_page rcv_pg;					/* last fully received page */
	Smvt_prt_page sch_pg;					/* last scheduled page */
	Smvt_prt_page cmt_pg;					/* last committed page */
	Smvt_prt_page prt_pg;					/* last printed page */
} Smvt_core_state_req_v1;

struct mvt_core_state_ans_v1 {
	Smvt_prt_header hdr;
	INT32	ver;							/* Version of this Cmd Id (starts from 1)*/
	UINT32	watchTime;						/* same as request or new value if need */
};

#define	Smvt_core_state	Smvt_core_state_req_v1

//--- PAGE CMD
#define	CORE_CMDPG_VER	1

struct mvt_core_loadpagecmd_req_v1 {
	struct mvt_prt_loadpagecmd_req_v1 cmd;
	INT32	pageIdx;
};

//--- FILE CMD
#define	CORE_CMDFILE_VER	1

struct mvt_core_loadfilecmd_req_v1 {
	struct mvt_prt_loadfilecmd_req_v1 cmd;
	INT32	pageIdx;
};

//--- DRAW CMD
#define	CORE_CMDDRAW_VER	1

struct mvt_core_drawcmd_req_v1 {
	struct mvt_prt_drawcmd_req_v1 cmd;
	INT32	pageIdx;
};

//--- LAYOUT CMD
#define	CORE_CMDLAYOUT_VER	1

struct mvt_core_loadlayoutcmd_req_v1 {
	struct mvt_prt_loadlayoutcmd_req_v1 cmd;
	INT32	pageIdx;
};

//--- RECORDDATA CMD
#define	CORE_CMDRECORDDATA_VER	1

struct mvt_core_loadrecorddata_req_v1 {
	struct mvt_prt_loadrecorddata_req_v1 cmd;
	INT32	pageIdx;
};

//--- MAIN STATE CHANGE
#define		CORE_MAINSTATE_VER		1
struct mvt_core_mainstate_req_v1 {
	Smvt_prt_header hdr;
	INT32	ver;							/* Version of this Cmd Id (starts from 1) */
	INT32	state;							/* Main state */
};

#ifdef _SIMULATOR
//--- SIMULATOR MAIN SPEED PAGE SIZE CHANGE
#define		CORE_MAINSPEED_VER		1
struct mvt_core_mainspeed_req_v1 {
	Smvt_prt_header hdr;
	INT32	ver;							/* Version of this Cmd Id (starts from 1) */
	INT32	speed;							/* Main speed in tic */
	INT32	size;							/* Main paper size in mm */
};
#endif

typedef struct SCounter {
	int			pageNum;
	int			copyNum;
} SCounter;


//--- Macros ------------------------------------------------------------------

#define pe_printf(level,...) {TrPrintfL(level, __VA_ARGS__); if (level&&WriteText) WriteText(__VA_ARGS__);}

//--- Variables ---------------------------------------------------------------

//--- Prototypes --------------------------------------------------------------

#ifdef __cplusplus
extern "C"{
#endif

	int	pe_send_msg(RX_SOCKET *socket, void *msg);

#ifdef __cplusplus
}
#endif

// ****************************************************************************
