// ****************************************************************************
//
//	rx_pecore_lib.h
//
// ****************************************************************************
//
//	Copyright 2018 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Hubert Zimmermann
//
// ****************************************************************************

#pragma once

#include "rx_def.h"
#include "rx_pecore_msg.h"

//--- Defines, Macros ---------------------------------------------------------

			// Flags for trace
#define TF_RCVMSG				(1<<0)	// Receive message								(main - interface)
#define TF_RCVMSGDATA			(1<<1)	// Receive message from rip						(interface only)
#define TF_RCVPAGE				(1<<4)	// Receive message page (load or data)			(main - interface)
#define TF_PAGESTATE_LISTCNT	(1<<5)	// Count of page list (at each state dialog)	(main - interface)
#define TF_PAGESTATE_INTERF		(1<<6)	// Counter and state of interface				(main only)
#define TF_ITFSTATE_CNT			(1<<7)	// Page Counter (at each state dialog)			(interface only)

#define	TF_API_GETPAGE			(1<<8)	// call Function API GetPage					(main - interface)
#define TF_API_PRINTSTATE		(1<<9)	// call Function API PrintState					(main only)
#define TF_API_PRINTERROR		(1<<10)	// call Function API PrintError					(main - interface)
#define	TF_API_PAGESTACKED		(1<<11)	// call Function API PageStacked				(main only)
#define	TF_API_GETBITMAP		(1<<12)	// call Function API GetBitmap					(interface only)
#define TF_API_PAGESTART		(1<<13)	// call Function API PageStarted				(interface only)
#define TF_API_PAGEPRINT		(1<<14)	// call Function API PagePrinted				(interface only)


//--- Structures, Typedefs ----------------------------------------------------
typedef enum {
	PE_CLOSED=0,
	PE_OPENING,
	PE_LISTEN,
	PE_CONNECTED,
	PE_DISCONNECTED,
	PE_CLOSING,
	PE_FAILED
} SpeState;

typedef enum {
	EJ_NONE = 0,
	EJ_PAGE,
	EJ_FILE,
	EJ_DRAW,
	EJ_LAYOUT
} SpeJobType;

typedef struct speMainPgeJob {
	SpeJobType	jobType;
	char		*info;
	int			jobId;
	int			pageStart;
	int			pageEnd;
	int			pageCopy;
} SpeMainPgeJob;

					/* Print Engine Main */
typedef struct mvt_prt_color SpeMainColor;
typedef struct mvt_prt_bitmap SpeMainBitmap;
typedef struct mvt_prt_interface SpeMainInterface;

typedef struct speMainConfig {
	char	id[32];							/* Printer Id, (example: Mouvent LB701 UV) */
	char	ip[32];							/* ip address of the main server */
	int		port;							/* connection port of the server */
	int		resoHead;						/* resolution (width) (head direction) */
	int		resoPrint;						/* resolution (height) (print direction) */
	int		maxWidth;						/* Maximal size of bitmap in head direction (in pixels) */
	int		maxHeight;						/* Maximal size of bitmap in print direction (in pixels) */
	int		maxSpeed;						/* Maximal paper speed (in m/min) */
	int		duplex;							/* Duplex: 0: no, 1: yes */
	int		dropSizeCount;					/* Max number of drop sizes (3) */
	int		bitPerPixel;					/* Max bit per pixel supported (2) */
	int		alignment;						/* Alignment in bits (8 or multiple of 8) */
	int		speedPassCount;					/* Printing passes nb (min 1) to mutilply the speed*/

	int		colorCount;						/* Number of colors */
	int 	bitmapCount;					/* Number of all bitmaps from all interfaces */
	int		interfCount;					/* Number of interfaces */
	SpeMainColor *colors;					/* Colors definition relative to colorCount */
	SpeMainBitmap *bitmaps;					/* Bitmaps definition relative to bitmapCount*/
	SpeMainInterface *interfaces;			/* Interfaces definition relative to interfaceCount */
} SpeMainConfig;

typedef struct {
	SpeMainConfig					*config;
	UINT32							traceFlags;

				/* callbacks from peMain thread, if not support must be fill with NULL*/
	int(*callbackPrintStart)		(void);												/* (might be 0) */
	int(*callbackPrintStop)			(INT32 page);										/* (might be 0) */
	int(*callbackPageReady)			(INT32 pageIdx, INT32 copyCount);					/* (might be 0) */
	int(*callbackPrintCmd)			(Smvt_prt_cmd *cmd);								/* (might be 0) */
	int(*callbackPrintSynchro)		(Smvt_prt_synchro *cmd);							/* (might be 0) */
	int(*callbackPrintAdjust)		(Smvt_prt_adjust *cmd);								/* (might be 0) */
	int(*callbackLoadFile)			(Smvt_prt_loadfile *cmd, int pageIdx);				/* (might be 0) */
	int(*callbackDrawCmd)			(Smvt_prt_drawcmd *cmd, int pageIdx);				/* (might be 0) */
	int(*callbackLoadlayout)		(Smvt_prt_layoutcmd *cmd, int pageIdx);				/* (might be 0) */
	int(*callbackRecordData)		(Smvt_prt_recorddata *cmd, int pageIdx);			/* (might be 0) */
#ifdef _SIMULATOR
	int(*callbackUpdateConfig)		(SpeMainConfig *config);							/* (might be 0) */
	int(*callbackEngineState)		(Smvt_prt_state *state);							/* (might be 0) */
#endif
} SpeMainPara;

typedef struct SpeMainAPI {
	int(*TraceFlagsUpdate)			(UINT32 flags);
	int(*PrintState)				(INT32 state);
	int(*PrintError)				(INT32 error);
#ifdef _SIMULATOR
	int(*PrintSpeed)				(INT32 speed, INT32 paperSize);
#else
	int(*PrintSpeed)				(INT32 speed);
#endif
	int(*GetPage)					(INT32 pageNum);									/* return 0 Ok, 1 error, 2 page not found */
	int(*PageStacked)				(INT32 pageNum);									/* return 0 Ok, 1 error, 2 page not found */
	int(*GetPgeJob)					(INT32 *count, SpeMainPgeJob **job);				/* return 0 Ok, 1 error, 2 not found */
} SpeMainAPI;

					/* Print Engine Interface */
typedef struct {
	int		interfaceNumber;				/* number of this interface (start with 0) */
	char	ipMain[32];						/* ip address of the peMain server */
	int		portMain;						/* connection port of the peMain server */
	char	ipInterface[32];				/* ip address of this interface */
	int		portData;						/* connnection port of this data server */
} SpeItfConfig;

typedef struct {
	SpeItfConfig					*config;
	UINT32							traceFlags;

	/* callbacks from peItfMain thread, if not support must be fill with NULL*/
	int(*callbackPrintStart)		(void);												/* (might be 0) */
	int(*callbackPrintStop)			(INT32 page);										/* (might be 0) */
	int(*callbackPageReady)			(INT32 page);										/* (might be 0) */
	int(*callbackPrintCmd)			(Smvt_prt_cmd *cmd);								/* (might be 0) */
	int(*callbackPrintSynchro)		(Smvt_prt_synchro *cmd);							/* (might be 0) */
	int(*callbackPrintAdjust)		(Smvt_prt_adjust *cmd);								/* (might be 0) */
	int(*callbackLoadFile)			(Smvt_prt_loadfile *cmd, int pageIdx);				/* (might be 0) */
	int(*callbackDrawCmd)			(Smvt_prt_drawcmd *cmd, int pageIdx);				/* (might be 0) */
	int(*callbackLoadlayout)		(Smvt_prt_layoutcmd *cmd, int pageIdx);				/* (might be 0) */
	int(*callbackRecordData)		(Smvt_prt_recorddata *cmd, int pageIdx);			/* (might be 0) */
#ifdef _SIMULATOR
	int(*callbackUpdateConfig)		(SpeMainConfig *config);							/* (might be 0) */
	int(*callbackRipCount)			(INT32 count);										/* (might be 0) */
	int(*callbackSpeed)				(INT32 speed, INT32 pageSize);						/* (might be 0) */
	int(*callbackState)				(INT32 state);										/* (might be 0) */
#endif
} SpeItfPara;

typedef struct SpeItfAPI {
	int(*TraceFlagsUpdate)			(UINT32 flags);
	int(*PrintError)				(INT32 error);
	int(*GetPage)					(INT32 pageNum, void **page, int *jobId /*=NULL*/, int *pageId /*=NULL*/); /* return 0 Ok, 1 error, 2 page not found */
	int(*GetBitmap)					(INT32 pageNum, void *page, int bitmap, SPlaneInfo *pinfo, int *compression); /* return 0 Ok, 1 error, 2 page not found */
	int(*PrintStarted)				(INT32 pageNum);									/* return 0 Ok, 1 error, 2 page not found */
	int(*PrintPrinted)				(INT32 pageNum);									/* return 0 Ok, 1 error, 2 page not found */
} SpeItfAPI;

typedef int(*WriteTextCallback)		(char *format, ...);


//--- Prototypes --------------------------------------------------------------

#ifdef __cplusplus
extern "C"{
#endif

extern WriteTextCallback WriteText;

void		rx_pewrite_callback		(WriteTextCallback writetext);

							/* Print Engine Main */
void		rx_pemain_start			(SpeMainPara *peMain, SpeMainAPI **peAPI);
void		rx_pemain_stop			(void);
SpeState	rx_pemain_state			(void);

							/* Print Engine Interface */
void		rx_peitf_start			(SpeItfPara *peItf, SpeItfAPI **peAPI);
void		rx_peitf_stop			(void);
SpeState	rx_peitf_state			(void);

#ifdef __cplusplus
}
#endif

// ****************************************************************************
