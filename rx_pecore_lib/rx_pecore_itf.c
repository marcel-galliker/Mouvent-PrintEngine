// ****************************************************************************
//
//	rx_pecore_itf.c
//
// ****************************************************************************
//
//	Copyright 2018 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Hubert Zimmermann
//
// ****************************************************************************

//--- Includes ----------------------------------------------------------------

#include "rx_pecore.h"

//--- Defines, Macros ---------------------------------------------------------
#define	TIMEOUT_FIFO			1000			/* 1 sec. */

#define	SURVEY_TIME				5000			/* 5 sec. */
#define	SHUTDOWN_SURVEY_TIME	200				/* 0.2 sec. */
#define	TIME_1SEC				1000			/* 1 sec. */
#define	RESTART_AFTER_LOST_CONNECTION	10		/* 10 * TIME_1SEC */

			// Flag for trace control, trace if true
#define TRAPI_PRINTERROR		(_pPeItfParameter.traceFlags & TF_API_PRINTERROR)		// call Function API PrintError
#define	TRAPI_GETPAGE			(_pPeItfParameter.traceFlags & TF_API_GETPAGE)			// call Function API GetPage
#define	TRAPI_GETBITMAP			(_pPeItfParameter.traceFlags & TF_API_GETBITMAP)		// call Function API GetBitmap
#define TRAPI_PAGESTART			(_pPeItfParameter.traceFlags & TF_API_PAGESTART)		// call Function API PageStarted
#define TRAPI_PAGEPRINT			(_pPeItfParameter.traceFlags & TF_API_PAGEPRINT)		// call Function API PagePrinted

#define TR_RCVMSG				(_pPeItfParameter.traceFlags & TF_RCVMSG)				// Receive message from main
#define TR_RCVMSGDATA			(_pPeItfParameter.traceFlags & TF_RCVMSGDATA)			// Receive message from rip

#define TR_RCVPAGE				(_pPeItfParameter.traceFlags & TF_RCVPAGE)				// Receive message page (load and data)

#define TR_PAGESTATE_LISTCNT	(_pPeItfParameter.traceFlags & TF_PAGESTATE_LISTCNT)	// Count of page list (at each state dialog)
#define TR_ITFSTATE_CNT			(_pPeItfParameter.traceFlags & TF_ITFSTATE_CNT)			// Page Counter (at each state dialog)

//--- Structures, Typedefs ----------------------------------------------------
typedef struct STarget {
	HANDLE		id;
	char		peerName[32];					/* Name or address ip (rip)*/
	RX_SOCKET	socket;
	int			connected;						/* FALSE: disconnect, TRUE: connected */
	int			currentSession;					/* id of the session # 0 */
} STarget;

typedef struct SBitmap {
	int			compression;					/* 0: none, 1: FastLZ */
	char		*buffer;
	int			length;
} SBitmap;

typedef struct SPage {
	int			job_id;
	int			page_id;
	int			bitmapCount;
	int			copyCount;
	int			height;
	int			width;
	int			lineLen;
	int			bitPerPixel;
	int			alignment;
	int			pageIdx;

	SBitmap		*bitmap;
	SCounter	received;
	SCounter	scheduled;
	SCounter	committed;
	SCounter	printed;
} SPage;

//--- Prototypes --------------------------------------------------------------

/* API - Interface */
static int		i_peTraceFlagChange			(UINT32 flags);
static int		i_pePrintError				(int value);
static int		i_peGetPage					(INT32 pageNum, void **PAGE, int *jobId /*=NULL*/, int *pageId /*=NULL*/);
static int		i_peGetBitmap				(INT32 pageNum, void *PAGE, int bitmap, SPlaneInfo *planeInfo, int *compression);
static int		i_pePageStarted				(INT32 pageNum);
static int		i_pePagePrinted				(INT32 pageNum);

/* Threads */
static void		*i_peDataThread				(void *param);
static void		*i_peStateThread			(void *param);

/* Helpers */
static STarget	*i_peTargetSource			(RX_SOCKET socket);
static void		i_peUpdatePageState			(void);
static void		i_peClearPageState			(void);
static void		i_peClearPageList			(void);

/* Main Client */
static int		i_peCommMain_msg			(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *par);
static int		i_peCommMain_close			(RX_SOCKET socket, const char *peerName);

/* Data server */
static int		i_peCommData_msg			(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *par);
static int		i_peCommData_open			(RX_SOCKET socket, const char *peerName);
static int		i_peCommData_close			(RX_SOCKET socket, const char *peerName);

/* Messages */
static int		i_peComm_Ping				(STarget *target, RX_SOCKET socket, void *msg, int len);
static int		i_peComm_Config				(STarget *target, RX_SOCKET socket, void *msg, int len);
static int		i_peComm_Open				(STarget *target, RX_SOCKET socket, void *msg, int len);
static int		i_peComm_Close				(STarget *target, RX_SOCKET socket, void *msg, int len);
static int		i_peComm_StartCmd			(STarget *target, RX_SOCKET socket, void *msg, int len);
static int		i_peComm_StopCmd			(STarget *target, RX_SOCKET socket, void *msg, int len);
static int		i_peComm_Command			(STarget *target, RX_SOCKET socket, void *msg, int len);
static int		i_peComm_Synchro			(STarget *target, RX_SOCKET socket, void *msg, int len);
static int		i_peComm_Adjust				(STarget *target, RX_SOCKET socket, void *msg, int len);
static int		i_peComm_Page				(STarget *target, RX_SOCKET socket, void *msg, int len);
static int		i_peComm_DataPage			(STarget *target, RX_SOCKET socket, void *msg, int len);
static int		i_peComm_State				(STarget *target, RX_SOCKET socket, void *msg, int len);
static int		i_peComm_FileCmd			(STarget *target, RX_SOCKET socket, void *msg, int len);
static int		i_peComm_DrawCmd			(STarget *target, RX_SOCKET socket, void *msg, int len);
static int		i_peComm_LayoutCmd			(STarget *target, RX_SOCKET socket, void *msg, int len);
static int		i_peComm_RecordData			(STarget *target, RX_SOCKET socket, void *msg, int len);

static int		i_peCommCore_RcvConfig		(STarget *target, RX_SOCKET socket, void *msg, int len);
static int		i_peCommCore_InterfConfig	(STarget *target, RX_SOCKET socket, void *msg, int len);
static int		i_peCommCore_InterfState	(STarget *target, RX_SOCKET socket, void *msg, int len);
static int		i_peCommCore_MainState		(STarget *target, RX_SOCKET socket, void *msg, int len);

#ifdef _SIMULATOR
static int		i_peComm_SimuSpeed			(STarget *target, RX_SOCKET socket, void *msg, int len);
#endif

//--- Locals ------------------------------------------------------------------
static void				*_threadPeData = NULL;
static void				*_threadPeState = NULL;
static int				_bShutDown = FALSE;

static RX_SOCKET		_hPeMainClient = 0;
static int				_bCommMainAlive = FALSE;

static SpeState			_statePeCommData = PE_CLOSED;

static SpeMainConfig	_peMainConfig;
static SpeItfPara		_pPeItfParameter;

static int				_mainCurrentSession = 0;
static int				_mainWatchTime = -1;

static Smvt_core_state	*_printStateToSend = NULL;

static HANDLE			_hListTarget = NULL;
static HANDLE			_hMutexTarget = NULL;
static HANDLE			_hListPage = NULL;
static HANDLE			_hMutexPage = NULL;

static struct {
	STarget								*target;
	struct mvt_prt_loadpagedata_req_v1	*msg;
	int									length;
}	_DataPagePending = { NULL, NULL, 0 };

// ****************************************************************************

// ****************************************************************************
// *			P U B L I C S
// ****************************************************************************

//--- rx_peitf_start ----------------------------------------------------------
void rx_peitf_start(SpeItfPara *peItf, SpeItfAPI **peAPI)
{
	if (!peItf || !peAPI)
		return;

	*peAPI = NULL;
	if (_threadPeData) {
		pe_printf(TRUE, "_threadPeData Already exists, must be stop before re-start");
		return;
	}

	_bShutDown = FALSE;

	if (_hListPage) rx_list_close(_hListPage); _hListPage = NULL;
	rx_list_create(&_hListPage);
	if (_hMutexPage) rx_mutex_destroy(&_hMutexPage);
	_hMutexPage = rx_mutex_create();
	if (_DataPagePending.msg) {
		free(_DataPagePending.msg);
		_DataPagePending.msg = NULL;
		_DataPagePending.length = 0;
	}

	_statePeCommData = PE_OPENING;

	memcpy(&_pPeItfParameter, peItf, sizeof(_pPeItfParameter));
	_pPeItfParameter.config = (SpeItfConfig*)calloc(1, sizeof(SpeItfConfig));
	memcpy(_pPeItfParameter.config, peItf->config, sizeof(SpeItfConfig));

	_threadPeData = rx_thread_start(i_peDataThread, NULL, 0, "_peData_thread");

	_threadPeState = rx_thread_start(i_peStateThread, NULL, 0, "_peState_thread");

	SpeItfAPI *pAPI = (SpeItfAPI*)calloc(1, sizeof(SpeItfAPI));
	pAPI->TraceFlagsUpdate = i_peTraceFlagChange;
	pAPI->PrintError = i_pePrintError;
	pAPI->GetPage = i_peGetPage;
	pAPI->GetBitmap = i_peGetBitmap;
	pAPI->PrintStarted = i_pePageStarted;
	pAPI->PrintPrinted = i_pePagePrinted;
	*peAPI = pAPI;
}

//--- rx_peitf_stop -----------------------------------------------------------
void   rx_peitf_stop(void)
{
	if ((_statePeCommData != PE_CLOSED) && (_statePeCommData != PE_CLOSING)) {
		_bShutDown = TRUE;
		_statePeCommData = PE_CLOSING;
	}
}

//--- rx_peitf_state ----------------------------------------------------------
SpeState rx_peitf_state(void)
{
	return _statePeCommData;
}


// ****************************************************************************

// ****************************************************************************
// *			P R I V A T E S
// ****************************************************************************

// ****************************************************************************
// *			I N T E R N A L S
// ****************************************************************************

// ****************************************************************************
// *			A P I - I N T E R F A C E
// ****************************************************************************


//--- i_peTraceFlagChange -----------------------------------------------------
int i_peTraceFlagChange(UINT32 flags)
{
	_pPeItfParameter.traceFlags = flags;
	return REPLY_OK;
}


//--- i_pePrintError ----------------------------------------------------------
int i_pePrintError(int value)
{
	if (value) {
		// Clear job, pages, ...
		i_peClearPageList();
	}
	pe_printf(TRAPI_PRINTERROR, "i_pePrintError %d", value);
	return REPLY_OK;
}

//--- i_peGetPage -------------------------------------------------------------
int i_peGetPage(int pageNum, void **PAGE, int *jobId, int *pageId)
{
	SPage *page = NULL;

	if (!PAGE || !pageNum)
		return REPLY_ERROR;
	*PAGE = NULL;
	if (jobId) *jobId = 0;
	if (pageId) *pageId = 0;
	rx_mutex_lock(_hMutexPage);
	rx_list_begin(_hListPage, (void**)&page);
	while (page) {
		if ((pageNum < (page->pageIdx + page->copyCount)) && (pageNum >= page->pageIdx)) {
			if (!page->received.pageNum || (pageNum < page->received.pageNum))
				page = NULL;
			break;
			}
		rx_list_next(_hListPage, (void**)&page);
	}
	if (page) {
		*PAGE = page;
		if (jobId) *jobId = page->job_id;
		if (pageId) *pageId = page->page_id;
		page->scheduled.pageNum = pageNum;
		page->scheduled.copyNum++;
	}
	rx_mutex_unlock(_hMutexPage);

	pe_printf(TRAPI_GETPAGE, "i_peGetPage %d / %p", pageNum, page);

	return (page) ? REPLY_OK : REPLY_NOT_FOUND;
}

//--- i_peGetBitmap -----------------------------------------------------------
int i_peGetBitmap(int pageNum, void *PAGE, int bitmap, SPlaneInfo *planeInfo, int *compression)
{
	SPage *page = (SPage*)PAGE;
	
	pe_printf(TRAPI_GETBITMAP, "i_peGetBitmap page %d / %p, bitmap %d", pageNum, PAGE, bitmap);

	if (!page || !pageNum || !planeInfo || !compression)
		return REPLY_ERROR;

	if ((pageNum >= (page->pageIdx + page->copyCount)) || (pageNum < page->pageIdx))
		return REPLY_ERROR;

	memset(planeInfo, 0, sizeof(SPlaneInfo));
	rx_mutex_lock(_hMutexPage);
	planeInfo->widthPx = page->width;
	planeInfo->lengthPx = page->height;
	planeInfo->aligment = page->alignment;
	planeInfo->lineLen = page->lineLen;
	planeInfo->bitsPerPixel = page->bitPerPixel;
	planeInfo->colorCode = _peMainConfig.colors[_peMainConfig.bitmaps[bitmap].color].colorCode;
	planeInfo->buffer = (PBYTE)page->bitmap[bitmap].buffer;
	planeInfo->dataSize = page->bitmap[bitmap].length;
	planeInfo->planenumber = bitmap;
	planeInfo->resol.x = _peMainConfig.resoHead;
	planeInfo->resol.y = _peMainConfig.resoPrint;

	*compression = page->bitmap[bitmap].compression;
	if (page->committed.pageNum != pageNum) {
		page->committed.pageNum = pageNum;
		page->committed.copyNum++;
	}
	rx_mutex_unlock(_hMutexPage);

	return (page) ? REPLY_OK : REPLY_NOT_FOUND;
}

//--- i_pePageStarted ---------------------------------------------------------
int i_pePageStarted(int pageNum)
{
	SPage *page = NULL;

	if (!pageNum)
		return REPLY_ERROR;

	rx_mutex_lock(_hMutexPage);
	rx_list_begin(_hListPage, (void**)&page);
	while (page) {
		if ((pageNum < (page->pageIdx + page->copyCount)) && (pageNum >= page->pageIdx))
			break;
		rx_list_next(_hListPage, (void**)&page);
	}
	rx_mutex_unlock(_hMutexPage);

	pe_printf(TRAPI_PAGESTART, "i_pePageStarted page %d / %p", pageNum, page);
	return (page) ? REPLY_OK : REPLY_NOT_FOUND;
}

//--- i_pePagePrinted ---------------------------------------------------------
int i_pePagePrinted(int pageNum)
{
	SPage *page = NULL;

	if (!pageNum)
		return REPLY_ERROR;

	rx_mutex_lock(_hMutexPage);
	rx_list_begin(_hListPage, (void**)&page);
	while (page) {
		if ((pageNum < (page->pageIdx + page->copyCount)) && (pageNum >= page->pageIdx))
			break;
		rx_list_next(_hListPage, (void**)&page);
	}
	if (page && (page->printed.pageNum != pageNum)) {
		page->printed.pageNum = pageNum;
		page->printed.copyNum++;
	}
	rx_mutex_unlock(_hMutexPage);

	pe_printf(TRAPI_PAGEPRINT, "i_pePagePrinted page %d / %p", pageNum, page);
	return (page) ? REPLY_OK:REPLY_NOT_FOUND;
}

// ****************************************************************************
// *			T H R E A D S
// ****************************************************************************

//--- i_peDataThread ----------------------------------------------------------
void *i_peDataThread(void *param)
{
	HANDLE hPeDataServer = NULL;
	int count = 0;

	if (_hMutexTarget) rx_mutex_destroy(&_hMutexTarget);
	_hMutexTarget = rx_mutex_create();
	rx_list_create(&_hListTarget);

	if (!_printStateToSend) {
		int size = sizeof (Smvt_core_state);
		_printStateToSend = (Smvt_core_state*)calloc(1, size);
		_printStateToSend->hdr.length = size;
		_printStateToSend->hdr.id = MVT_CORE_REQ_STATE | MVT_CORE_INTERNAL;
		_printStateToSend->ver = CORE_STATE_VER;
	}

	while (!_bShutDown) {
		if (!_bCommMainAlive) {
			if (hPeDataServer) {
				/* Close the remainded rip targets */
				if (_hListTarget) {
					STarget *target = NULL;
					rx_mutex_lock(_hMutexTarget);
					rx_list_begin(_hListTarget, (void**)&target);
					while (target) {
						if (target->connected) {
							target->connected = FALSE;
							sok_close(&target->socket);
						}
						rx_list_next(_hListTarget, (void**)&target);
					}
					rx_mutex_unlock(_hMutexTarget);
				}
				/* Wait end of connections */
				if (!(_hListTarget && rx_list_count(_hListTarget))) {
					sok_stop_server(&hPeDataServer);
					hPeDataServer = NULL;
				}
			}
			else {
				if (count)
					count--;
				else {
					_statePeCommData = PE_OPENING;
					if (!_bCommMainAlive) {
						if (!sok_open_client_2(&_hPeMainClient, _pPeItfParameter.config->ipMain, _pPeItfParameter.config->portMain, SOCK_STREAM, i_peCommMain_msg, i_peCommMain_close)) {
							_bCommMainAlive = TRUE;
							i_peCommCore_InterfConfig(NULL, _hPeMainClient, NULL, 0);
						}
					}
					pe_printf(TRUE, "Comm slave: Open main client");
				}
			}
			rx_sleep(TIME_1SEC);
		}
		else if (!hPeDataServer) {
			count = RESTART_AFTER_LOST_CONNECTION;					// time before restart if lost connection
			_statePeCommData = PE_LISTEN;
			if (sok_start_server(&hPeDataServer, NULL, _pPeItfParameter.config->portData, SOCK_STREAM, MAX_RIPCOUNT, i_peCommData_msg, i_peCommData_open, i_peCommData_close))
				_statePeCommData = PE_FAILED;

			pe_printf(TRUE, "Comm slave: Data server started");

			rx_sleep(TIME_1SEC);
		}
		else {
			rx_sleep(SURVEY_TIME);
		}
	}

	/* Close the remainded rip targets */
	if (_hListTarget) {
		STarget *target = NULL;
		rx_mutex_lock(_hMutexTarget);
		rx_list_begin(_hListTarget, (void**)&target);
		while (target) {
			if (target->connected) {
				target->connected = FALSE;
				sok_close(&target->socket);
			}
			rx_list_next(_hListTarget, (void**)&target);
		}
		rx_mutex_unlock(_hMutexTarget);
	}

	/* Wait end of state thread */
	while (_threadPeState) rx_sleep(SHUTDOWN_SURVEY_TIME);

	/* Wait end of connections */
	while (_hListTarget && rx_list_count(_hListTarget)) rx_sleep(SHUTDOWN_SURVEY_TIME);
	sok_stop_server(&hPeDataServer);
	hPeDataServer = NULL;

	/* Close main connection */
	sok_close(&_hPeMainClient);
	_hPeMainClient = 0;

	_statePeCommData = PE_CLOSED;

	/* Clear page list */
	i_peClearPageList();
	if (_hListPage) { rx_list_close(_hListPage); _hListPage = NULL; }
	if (_hMutexPage) { rx_mutex_destroy(&_hMutexPage);}
	if (_DataPagePending.msg) {
		free(_DataPagePending.msg);
		_DataPagePending.msg = NULL;
		_DataPagePending.length = 0;
	}

	/* Clear status */
	if (_printStateToSend) { free(_printStateToSend); _printStateToSend = NULL; }

	/* Clear target */
	if (_hListTarget) { rx_list_close(_hListTarget); _hListTarget = NULL; }
	if (_hMutexTarget)
	{
		rx_mutex_destroy(&_hMutexTarget);
		Error(LOG, 0, "rx_mutex_destroy(&_hMutexTarget)");
	}

	/* Clear config */
	if (_peMainConfig.colors) { free(_peMainConfig.colors); _peMainConfig.colors = NULL; }
	if (_peMainConfig.bitmaps) { free(_peMainConfig.bitmaps); _peMainConfig.bitmaps = NULL; }
	if (_peMainConfig.interfaces) {	free(_peMainConfig.interfaces); _peMainConfig.interfaces = NULL; }
	memset(&_peMainConfig, 0, sizeof(_peMainConfig));

	if (_pPeItfParameter.config) free(_pPeItfParameter.config);
	_pPeItfParameter.config = NULL;

	_threadPeData = NULL;
	return NULL;
}

//--- i_peStateThread ---------------------------------------------------------
void *i_peStateThread(void *param)
{
	while (!_bShutDown) {
		if (_bCommMainAlive && _hPeMainClient) {
			if (_mainWatchTime>0)
				i_peCommCore_InterfState(NULL, _hPeMainClient, NULL, 0);
			else
				i_peComm_Ping(NULL, _hPeMainClient, NULL, 0);
		}
		rx_sleep((_mainWatchTime <= 0) ? (TIME_1SEC) : _mainWatchTime);
	}
	_threadPeState = NULL;
	return NULL;
}

// ****************************************************************************
// *			H E L P E R S
// ****************************************************************************

//--- i_peTargetSource --------------------------------------------------------
STarget *i_peTargetSource(RX_SOCKET socket)
{
	STarget *target = NULL;

	if (_hListTarget) {
		rx_mutex_lock(_hMutexTarget);
		rx_list_begin(_hListTarget, (void**)&target);
		while (target && (target->socket != socket))
			rx_list_next(_hListTarget, (void**)&target);
		rx_mutex_unlock(_hMutexTarget);
	}
	return target;
}

//--- i_peUpdatePagetate ------------------------------------------------------
void i_peUpdatePageState(void)
{
	SPage *page = NULL;

	if (!_printStateToSend)
		return;

	if (!_hMutexPage) return;
	// Page Counter
	int mask = 0;
	rx_mutex_lock(_hMutexPage);
	rx_list_end(_hListPage, (void**)&page);
	while (page) {
		if (!(mask & 1) && (page->received.pageNum)) {
			mask |= 1;
			_printStateToSend->rcv_pg.job_id = page->job_id;
			_printStateToSend->rcv_pg.page_id = page->page_id;
			_printStateToSend->rcv_pg.idx = page->received.pageNum;
			_printStateToSend->rcv_pg.copy = page->received.copyNum;
		}
		if (!(mask & (1 << 1)) && (page->scheduled.pageNum)) {
			mask |= (1 << 1);
			_printStateToSend->sch_pg.job_id = page->job_id;
			_printStateToSend->sch_pg.page_id = page->page_id;
			_printStateToSend->sch_pg.idx = page->scheduled.pageNum;
			_printStateToSend->sch_pg.copy = page->scheduled.copyNum;
		}
		if (!(mask & (1 << 2)) && (page->committed.pageNum)) {
			mask |= (1 << 2);
			_printStateToSend->cmt_pg.job_id = page->job_id;
			_printStateToSend->cmt_pg.page_id = page->page_id;
			_printStateToSend->cmt_pg.idx = page->committed.pageNum;
			_printStateToSend->cmt_pg.copy = page->committed.copyNum;
		}
		if (page->printed.pageNum) {
			if (!(mask & (1 << 3))) {
				mask |= (1 << 3);
				_printStateToSend->prt_pg.job_id = page->job_id;
				_printStateToSend->prt_pg.page_id = page->page_id;
				_printStateToSend->prt_pg.idx = page->printed.pageNum;
				_printStateToSend->prt_pg.copy = page->printed.copyNum;
			}
			if ((_printStateToSend->prt_pg.job_id != page->job_id) || (_printStateToSend->prt_pg.page_id != page->page_id) ||
				(page->copyCount == 1) || (page->copyCount == page->printed.copyNum) ) {
				SPage *delpage = NULL;
				rx_list_begin(_hListPage, (void**)&delpage);
				while (delpage && (delpage != page)) {
					if (delpage->bitmap) {
						for (int k = 0; k < delpage->bitmapCount; k++) {
							if (delpage->bitmap[k].buffer)
								free(delpage->bitmap[k].buffer);
						}
						free(delpage->bitmap);
					}
					free(delpage);
					rx_list_remove(_hListPage, NULL);
					rx_list_begin(_hListPage, (void**)&delpage);
				}
				if (page->bitmap) {
					for (int k = 0; k < page->bitmapCount; k++) {
						if (page->bitmap[k].buffer)
							free(page->bitmap[k].buffer);
					}
					free(page->bitmap);
				}
				free(page);
				rx_list_remove(_hListPage, page);
				break;
			}
		}
		rx_list_prev(_hListPage, (void**)&page);
	}
	rx_mutex_unlock(_hMutexPage);

	pe_printf(TR_PAGESTATE_LISTCNT, "i_peUpdatePageState ListPage Count: %d", rx_list_count(_hListPage));
}

//--- i_peClearPageState ------------------------------------------------------
void i_peClearPageState(void)
{
	if (!_printStateToSend)
		return;

	memset(&_printStateToSend->rcv_pg, 0, sizeof(Smvt_prt_page));
	memset(&_printStateToSend->sch_pg, 0, sizeof(Smvt_prt_page));
	memset(&_printStateToSend->cmt_pg, 0, sizeof(Smvt_prt_page));
	memset(&_printStateToSend->prt_pg, 0, sizeof(Smvt_prt_page));
}

//--- i_peClearPageList -------------------------------------------------------
void i_peClearPageList(void)
{
	if (!_hMutexPage) return;
	rx_mutex_lock(_hMutexPage);
	if (_hListPage) {
		while (rx_list_count(_hListPage) > 0) {
			SPage *page = NULL;
			rx_list_begin(_hListPage, (void**)&page);
			if (page) {
				if (page->bitmap) {
					for (int k = 0; k < page->bitmapCount; k++) {
						if (page->bitmap[k].buffer)
							free(page->bitmap[k].buffer);
					}
					free(page->bitmap);
				}
				free(page);
			}
			rx_list_remove(_hListPage, NULL);
		}
	}
	rx_mutex_unlock(_hMutexPage);
	if (_DataPagePending.msg) {
		free(_DataPagePending.msg);
		_DataPagePending.msg = NULL;
		_DataPagePending.length = 0;
	}

}


// ****************************************************************************
// *		C L I E N T		M A I N		( I N T E R F A C E )
// ****************************************************************************

//--- i_peCommMain_close ------------------------------------------------------
int i_peCommMain_close(RX_SOCKET socket, const char *peerName)
{
	i_peClearPageList();

	pe_printf(TRUE, "peCommMain client closed: >>%s<<", peerName);

	_bCommMainAlive = FALSE;

	sok_close(&_hPeMainClient);
	_hPeMainClient = 0;
	return REPLY_OK;
}

//--- i_peCommMain_msg --------------------------------------------------------
int i_peCommMain_msg(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *par)
{
	int reply = REPLY_ERROR;
	static int _err = FALSE;
	Smvt_prt_header *phdr = (Smvt_prt_header*)msg;

	pe_printf(TR_RCVMSG, "PES main Recv msg 0x%X (%d) - len %d", phdr->id, phdr->id & ~MVT_CORE_INTERNAL, phdr->length);

	switch (phdr->id) {
	case MVT_PRT_REQ_PING:								reply = i_peComm_Ping(NULL, socket, msg, len);  break;
	case MVT_PRT_REQ_CONF:								reply = i_peComm_Config(NULL, socket, msg, len); break;
	case MVT_PRT_REQ_OPEN:								reply = i_peComm_Open(NULL, socket, msg, len); break;
	case MVT_PRT_REQ_CLOSE:								reply = i_peComm_Close(NULL, socket, msg, len); break;
	case MVT_PRT_REQ_START:								reply = i_peComm_StartCmd(NULL, socket, msg, len); break;
	case MVT_PRT_REQ_STOP:								reply = i_peComm_StopCmd(NULL, socket, msg, len); break;
	case MVT_PRT_REQ_CMD:								reply = i_peComm_Command(NULL, socket, msg, len); break;
	case MVT_PRT_REQ_SYNC:								reply = i_peComm_Synchro(NULL, socket, msg, len); break;
	case MVT_PRT_REQ_ADJ:								reply = i_peComm_Adjust(NULL, socket, msg, len); break;
	case MVT_CORE_REQ_CMDPG | MVT_CORE_INTERNAL:		reply = i_peComm_Page(NULL, socket, msg, len); break;
	case MVT_PRT_REQ_DATAPG:
		ErrorEx(0, 0, ERR_CONT, 0, "Pe ItfMain: Data Command not supported (0x%8x)", phdr->id);
		break;
	case MVT_PRT_REQ_STATE:								reply = i_peComm_State(NULL, socket, msg, len); break;
	case MVT_CORE_REQ_FILECMD | MVT_CORE_INTERNAL:		reply = i_peComm_FileCmd(NULL, socket, msg, len); break;
	case MVT_CORE_REQ_DRAWCMD | MVT_CORE_INTERNAL:		reply = i_peComm_DrawCmd(NULL, socket, msg, len); break;
	case MVT_CORE_REQ_LAYOUTCMD | MVT_CORE_INTERNAL:	reply = i_peComm_LayoutCmd(NULL, socket, msg, len); break;
	case MVT_CORE_REQ_RECORDDATA | MVT_CORE_INTERNAL:	reply = i_peComm_RecordData(NULL, socket, msg, len); break;

	case MVT_CORE_REQ_MAINSTATE | MVT_CORE_INTERNAL:	reply = i_peCommCore_MainState(NULL, socket, msg, len); break;

#ifdef _SIMULATOR
	case MVT_CORE_REQ_MAINSPEED | MVT_CORE_INTERNAL:	reply = i_peComm_SimuSpeed(NULL, socket, msg, len); break;
#endif

	case MVT_CORE_ANS_CONF | MVT_CORE_INTERNAL:
	case MVT_CORE_ANS_STATE | MVT_CORE_INTERNAL:		reply = REPLY_OK;  break;

	case MVT_PRT_ANS_CONF | MVT_CORE_INTERNAL:			reply = i_peCommCore_RcvConfig(NULL, socket, msg, len); break;

	default:
		if (!_err) {
			_err = TRUE;
			ErrorEx(0, 0, ERR_CONT, 0, "Pe ItfMain: Unknown dialog 0x%8x", phdr->id);
		}
		pe_printf(TRUE, "Pe ItfMain: Unknown dialog 0x%X (%d)", phdr->id, phdr->id & ~MVT_CORE_INTERNAL);
		break;
	}

	return reply;
}

// ****************************************************************************
// *			D A T A		R I P		S E R V E R
// ****************************************************************************

//--- i_peCommData_open -------------------------------------------------------
int i_peCommData_open(RX_SOCKET socket, const char *peerName)
{
	STarget *target = NULL;

	if (!_hListTarget) goto error;

	rx_mutex_lock(_hMutexTarget);
	rx_list_begin(_hListTarget, (void**)&target);
	while (target) {
		if (!strcmp(target->peerName, peerName)) {
			rx_mutex_unlock(_hMutexTarget);
			goto error;
		}
		rx_list_next(_hListTarget, (void**)&target);
	}
	rx_mutex_unlock(_hMutexTarget);
	target = (STarget*)calloc(1, sizeof(STarget));
	strncpy(target->peerName, peerName, sizeof(target->peerName));
	target->socket = socket;
	target->connected = TRUE;
	rx_mutex_lock(_hMutexTarget);
	rx_list_add(_hListTarget, target);
	rx_mutex_unlock(_hMutexTarget);

	pe_printf(TRUE, "peCommData (Rip) Server opened: >>%s<<", peerName);
#ifdef _SIMULATOR
	if (_pPeItfParameter.callbackRipCount)
		_pPeItfParameter.callbackRipCount(rx_list_count(_hListTarget));
#endif
	_statePeCommData = PE_CONNECTED;
	return REPLY_OK;

error:
	return REPLY_ERROR;
}

//--- i_peCommData_close ------------------------------------------------------
int i_peCommData_close(RX_SOCKET socket, const char *peerName)
{
	STarget *target = NULL;

	if (!_hListTarget) goto error;

	rx_mutex_lock(_hMutexTarget);
	rx_list_begin(_hListTarget, (void**)&target);
	while (target && strcmp(target->peerName, peerName))
		rx_list_next(_hListTarget, (void**)&target);
	rx_mutex_unlock(_hMutexTarget);
	if (!target) goto error;
	rx_mutex_lock(_hMutexTarget);
	target->connected = FALSE;
	rx_list_remove(_hListTarget, target);
	rx_mutex_unlock(_hMutexTarget);
	free(target);

	pe_printf(TRUE, "peCommData (Rip) Server closed: >>%s<<", peerName);

#ifdef _SIMULATOR
	if (_pPeItfParameter.callbackRipCount)
		_pPeItfParameter.callbackRipCount(rx_list_count(_hListTarget));
#endif
	_statePeCommData = rx_list_count(_hListTarget) ? PE_CONNECTED : PE_LISTEN;
	return REPLY_OK;

error:
	return REPLY_ERROR;
}

//--- i_peCommData_msg --------------------------------------------------------
int i_peCommData_msg(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *par)
{
	int reply = REPLY_ERROR;
	static int _err = FALSE;
	STarget *target;
	Smvt_prt_header *phdr = (Smvt_prt_header*)msg;
	
	target = i_peTargetSource(socket);
	if (!target)
		return reply;

	pe_printf(TR_RCVMSGDATA, "PES rip Recv msg 0x%X (%d) - len %d", phdr->id, (phdr->id & ~MVT_CORE_INTERNAL), phdr->length);

	switch (phdr->id) {
	case MVT_PRT_REQ_PING | MVT_CORE_INTERNAL:
	case MVT_PRT_REQ_PING:							reply = i_peComm_Ping(target, socket, msg, len);  break;
	case MVT_PRT_REQ_OPEN:							reply = i_peComm_Open(target, socket, msg, len); break;
	case MVT_PRT_REQ_CLOSE:							reply = i_peComm_Close(target, socket, msg, len); break;
	case MVT_PRT_REQ_DATAPG:						reply = i_peComm_DataPage(target, socket, msg, len); break;

	default:
		if (!_err) {
			_err = TRUE;
			ErrorEx(0, 0, ERR_CONT, 0, "Pe Data: Unknown dialog 0x%8x", phdr->id);
		}
		pe_printf(TRUE, "Pe Data: Unknown dialog 0x%X (%d)", phdr->id, (phdr->id & ~MVT_CORE_INTERNAL));
		break;
	}

	return reply;
}

// ****************************************************************************
// *			M E S S A G E S
// ****************************************************************************

//--- i_peComm_Ping -----------------------------------------------------------
int i_peComm_Ping(STarget *target, RX_SOCKET socket, void *msg, int len)
{
	struct mvt_prt_ping_ans_v1 answer;

	answer.hdr.id = ((msg) ? MVT_PRT_ANS_PING : MVT_PRT_REQ_PING) | (!target) ? MVT_CORE_INTERNAL:0;
	answer.hdr.length = sizeof(answer);
	answer.ver = PING_VER;
	pe_send_msg(&socket, &answer);

	return REPLY_OK;
}

//--- i_peComm_Config ---------------------------------------------------------
int i_peComm_Config(STarget *target, RX_SOCKET socket, void *msg, int len)
{
	return REPLY_OK;
}

//--- i_peComm_Open -----------------------------------------------------------
int i_peComm_Open(STarget *target, RX_SOCKET socket, void *msg, int len)
{
	struct mvt_prt_session_req_v1 *request = (struct mvt_prt_session_req_v1*) msg;
	struct mvt_prt_session_ans_v1 answer;

	if (target)
		answer.id = target->currentSession = (request->id == _mainCurrentSession) ? request->id : 0;
	else
		answer.id = _mainCurrentSession = request->id;
	answer.hdr.id = MVT_PRT_ANS_OPEN | (!target) ? MVT_CORE_INTERNAL : 0;
	answer.hdr.length = sizeof(answer);
	answer.ver = OPEN_VER;
	pe_send_msg(&socket, &answer);

	return REPLY_OK;
}

//--- i_peComm_Close ----------------------------------------------------------
int i_peComm_Close(STarget *target, RX_SOCKET socket, void *msg, int len)
{
	struct mvt_prt_close_ans_v1 answer;

	answer.hdr.id = MVT_PRT_ANS_CLOSE | (!target) ? MVT_CORE_INTERNAL : 0;
	answer.hdr.length = sizeof(answer);
	answer.ver = CLOSE_VER;
	pe_send_msg(&socket, &answer);
	if (target)
		target->currentSession = 0;
	else
		_mainCurrentSession = 0;
	i_peClearPageList();
	i_peClearPageState();
	return REPLY_OK;
}

//--- i_peComm_StartCmd -------------------------------------------------------
int i_peComm_StartCmd(STarget *target, RX_SOCKET socket, void *msg, int len)
{
	if (target)
		return REPLY_OK;

	if (_pPeItfParameter.callbackPrintStart)
		_pPeItfParameter.callbackPrintStart();

	return REPLY_OK;
}

//--- i_peComm_StopCmd --------------------------------------------------------
int i_peComm_StopCmd(STarget *target, RX_SOCKET socket, void *msg, int len)
{
	struct mvt_prt_stop_req_v1 *request = (struct mvt_prt_stop_req_v1*) msg;

	if (target)
		return REPLY_OK;

	if (_pPeItfParameter.callbackPrintStop)
		_pPeItfParameter.callbackPrintStop(request->page_idx);

	return REPLY_OK;
}

//--- i_peComm_Command --------------------------------------------------------
int i_peComm_Command(STarget *target, RX_SOCKET socket, void *msg, int len)
{
	struct mvt_prt_cmd_req_v1 *request = (struct mvt_prt_cmd_req_v1*) msg;

	if (target)
		return REPLY_OK;

	if (_pPeItfParameter.callbackPrintCmd)
		_pPeItfParameter.callbackPrintCmd(request);

	return REPLY_OK;
}

//--- i_peComm_Synchro --------------------------------------------------------
int i_peComm_Synchro(STarget *target, RX_SOCKET socket, void *msg, int len)
{
	struct mvt_prt_jobsync_req_v1 *request = (struct mvt_prt_jobsync_req_v1*) msg;

	if (target)
		return REPLY_OK;

	if (_pPeItfParameter.callbackPrintSynchro)
		_pPeItfParameter.callbackPrintSynchro(request);

	return REPLY_OK;
}

//--- i_peComm_Adjust ---------------------------------------------------------
int i_peComm_Adjust(STarget *target, RX_SOCKET socket, void *msg, int len)
{
	struct mvt_prt_adjust_req_v1 *request = (struct mvt_prt_adjust_req_v1*) msg;

	if (target)
		return REPLY_OK;

	if (_pPeItfParameter.callbackPrintAdjust)
		_pPeItfParameter.callbackPrintAdjust(request);

	return REPLY_OK;
}

//--- i_peComm_Page -----------------------------------------------------------
int i_peComm_Page(STarget *target, RX_SOCKET socket, void *msg, int len)
{
	struct mvt_core_loadpagecmd_req_v1 *request = (struct mvt_core_loadpagecmd_req_v1*) msg;
	struct mvt_prt_loadpagecmd_ans_v1 answer;
	SPage *page;

	if (target)
		return REPLY_OK;
	if (!_hMutexPage) return REPLY_ERROR;
	
	if (request->pageIdx == 1) i_peClearPageState();

	page = (SPage*)calloc(1, sizeof(SPage));
	page->job_id = request->cmd.job_id;
	page->page_id = request->cmd.page_id;
	page->copyCount = request->cmd.copyCount;
	page->height = request->cmd.Height;
	page->width = request->cmd.Width;
	page->lineLen = request->cmd.lineLen;
	page->bitPerPixel = request->cmd.bitPerPixel;
	page->alignment = request->cmd.alignment;

	page->bitmapCount = 0;
	page->pageIdx = request->pageIdx;
	page->bitmap = (SBitmap*)calloc(_peMainConfig.bitmapCount, sizeof(SBitmap));

	rx_mutex_lock(_hMutexPage);

	rx_list_add(_hListPage, page);
	rx_mutex_unlock(_hMutexPage);

	answer.hdr.id = MVT_PRT_ANS_CMDPG | (!target) ? MVT_CORE_INTERNAL : 0;
	answer.hdr.length = sizeof(answer);
	answer.ver = CMDPG_VER;
	answer.page_idx = page->pageIdx;
	answer.copyCount = request->cmd.copyCount;
	pe_send_msg(&socket, &answer);

	pe_printf(TR_RCVPAGE, "Pe ItfMain: Cmd Page %d (job_id %d - page_id %d - Copy count - %d)", page->pageIdx, page->job_id, page->page_id, page->copyCount);

	if (_DataPagePending.msg && _DataPagePending.length) {
		i_peComm_DataPage(_DataPagePending.target, _DataPagePending.target->socket, _DataPagePending.msg, _DataPagePending.length);
	}

	return REPLY_OK;
}

//--- i_peComm_DataPage -------------------------------------------------------
int i_peComm_DataPage(STarget *target, RX_SOCKET socket, void *msg, int len)
{
	struct mvt_prt_loadpagedata_req_v1 *request = (struct mvt_prt_loadpagedata_req_v1*) msg;
	struct mvt_prt_loadpagedata_ans_v1 answer;
	struct mvt_prt_bmpdata *bitmap;
	SPage *page=NULL;
	int found = FALSE;

	if (!target)
		return REPLY_OK;

	answer.hdr.id = MVT_PRT_ANS_DATAPG | (!target) ? MVT_CORE_INTERNAL : 0;
	answer.hdr.length = sizeof(answer);
	answer.ver = DATAPG_VER;
	answer.cmd_status = 1;

	pe_printf(TR_RCVPAGE, "Pe Data: Page data %d (job_id %d - page_id %d)", request->page_idx, request->job_id, request->page_id);

	rx_mutex_lock(_hMutexPage);
	rx_list_end(_hListPage, (void**)&page);
	while (page) {
		if (_bShutDown) {
			rx_mutex_unlock(_hMutexPage);
			return REPLY_OK;
		}

		if ((page->job_id == request->job_id) && (page->page_id == request->page_id) && (page->pageIdx == request->page_idx))
			break;
		rx_list_prev(_hListPage, (void**)&page);
	}

	if (!page) {
		if (!_DataPagePending.msg && !_DataPagePending.length) {
			_DataPagePending.target = target;
			_DataPagePending.msg = (struct mvt_prt_loadpagedata_req_v1*) malloc(len);
			_DataPagePending.length = len;
			memcpy(_DataPagePending.msg, msg, len);
			rx_mutex_unlock(_hMutexPage);
			pe_printf(TR_RCVPAGE, "Pe Data: Page data %d - pending", _DataPagePending.msg->page_idx);
			return REPLY_OK;
		}
	}

	rx_mutex_unlock(_hMutexPage);

	if (page) {
		if (!page->height) page->height = request->Height;
		if (!page->width) page->width = request->Width;
		if (!page->lineLen) page->lineLen = request->lineLen;
		if (!page->bitPerPixel) page->bitPerPixel = request->bitPerPixel;
		if (!page->alignment) page->alignment = request->alignment;
	}

	if (!page ||
		(page->height != request->Height) || (page->width != request->Width) ||
		(page->lineLen != request->lineLen) || (page->bitPerPixel != request->bitPerPixel) ||
		(page->alignment != request->alignment)) {
		answer.cmd_status = 0;
	}
	else {
		const char *Compress[4] = { "No Comp", "LZW", "TIFF", "unknow" };

		bitmap = (struct mvt_prt_bmpdata *) &request[1];
		for (int i = 0; i < request->bitmapCount; i++) {
			if (bitmap->index < _peMainConfig.bitmapCount) {
				pe_printf(TR_RCVPAGE, "Pe Data: Page data %d (bitmap %d - %s(%d))", page->pageIdx, bitmap->index, (bitmap->compression <= MVT_COMP_TIFF) ? Compress[bitmap->compression] : Compress[3], bitmap->compression);
				rx_mutex_lock(_hMutexPage);
				page->bitmapCount++;
				page->bitmap[bitmap->index].compression = bitmap->compression;
				page->bitmap[bitmap->index].length = bitmap->dataSize;
				page->bitmap[bitmap->index].buffer = (char*)malloc(bitmap->dataSize);
				char *buffer = (char*)&bitmap[1];
				memcpy(page->bitmap[bitmap->index].buffer, buffer, bitmap->dataSize);
				bitmap = (struct mvt_prt_bmpdata *) &buffer[bitmap->dataSize];
				rx_mutex_unlock(_hMutexPage);
			}
			else {
				answer.cmd_status = 0;
				break;
			}
		}
		if (answer.cmd_status) {
			rx_mutex_lock(_hMutexPage);
			int i;
			for (i = 0; i < _peMainConfig.bitmapCount; i++) {
				if ((_peMainConfig.bitmaps[i].interf == _pPeItfParameter.config->interfaceNumber) && (page->bitmap[i].buffer == NULL)) {
					break;
				}
			}
			if (i == _peMainConfig.bitmapCount) {
				page->copyCount = request->copyCount;
				page->received.pageNum = page->pageIdx;
				page->received.copyNum = page->copyCount;
			}
			rx_mutex_unlock(_hMutexPage);

			if ((i == _peMainConfig.bitmapCount) && _pPeItfParameter.callbackPageReady) _pPeItfParameter.callbackPageReady(page->pageIdx);

			pe_printf(TR_RCVPAGE, "Pe Data: Page data %d - %s", page->pageIdx, (i == _peMainConfig.bitmapCount) ? "Full" : "Partial");
		}
	}
	pe_send_msg(&socket, &answer);

	pe_printf(TR_RCVPAGE, "Pe Data: Page data %d - %s", request->page_idx, (answer.cmd_status) ? "Ok" : "Fail");

	if ((msg == _DataPagePending.msg) && (len == _DataPagePending.length) && answer.cmd_status) {
		free(_DataPagePending.msg);
		_DataPagePending.msg = NULL;
		_DataPagePending.length = 0;
	}

	return REPLY_OK;
}

//--- i_peComm_State ----------------------------------------------------------
int i_peComm_State(STarget *target, RX_SOCKET socket, void *msg, int len)
{
	struct mvt_prt_state_req_v1 *request = (struct mvt_prt_state_req_v1*) msg;
	if (target || !_printStateToSend)
		return REPLY_OK;

	if (msg && (request->watchTime >= 0))
		_mainWatchTime = request->watchTime / 2;

	i_peCommCore_InterfState(NULL, socket, NULL, 0);
	return REPLY_OK;
}

//--- i_peComm_FileCmd --------------------------------------------------------
int i_peComm_FileCmd(STarget *target, RX_SOCKET socket, void *msg, int len)
{
	struct mvt_core_loadfilecmd_req_v1 *request = (struct mvt_core_loadfilecmd_req_v1*) msg;

	if (target)
		return REPLY_OK;

	if (request->pageIdx == 1) i_peClearPageState();

	int nIx = request->pageIdx;
	int cpyInc = (request->cmd.pageEnd - request->cmd.pageStart) ? 1 : request->cmd.copyCount;
	for (int nCp = 0; nCp < request->cmd.copyCount; nCp += cpyInc) {
		for (int nId = request->cmd.pageStart; nId <= request->cmd.pageEnd; nId++) {
			SPage *page;
			page = (SPage*)calloc(1, sizeof(SPage));
			page->job_id = request->cmd.job_id;
			page->page_id = nId;
			page->copyCount = cpyInc;
			page->bitmapCount = _peMainConfig.bitmapCount;
			page->pageIdx = nIx;
			page->bitmap = (SBitmap*)calloc(_peMainConfig.bitmapCount, sizeof(SBitmap));
			page->received.pageNum = nIx;
			page->received.copyNum = nCp + cpyInc;

			rx_mutex_lock(_hMutexPage);
			rx_list_add(_hListPage, page);
			rx_mutex_unlock(_hMutexPage);
			nIx += cpyInc;
		}
	}

	if (_pPeItfParameter.callbackLoadFile)
		_pPeItfParameter.callbackLoadFile(&request->cmd, request->pageIdx);

	return REPLY_OK;
}

//--- i_peComm_DrawCmd --------------------------------------------------------
int i_peComm_DrawCmd(STarget *target, RX_SOCKET socket, void *msg, int len)
{
	struct mvt_core_drawcmd_req_v1 *request = (struct mvt_core_drawcmd_req_v1*) msg;
	SPage *page = NULL;

	if (target)
		return REPLY_OK;

	if (request->pageIdx == 1) i_peClearPageState();

	page = (SPage*)calloc(1, sizeof(SPage));
	page->job_id = request->cmd.job_id;
	page->page_id = request->pageIdx;
	page->copyCount = request->cmd.copyCount;
	page->bitmapCount = _peMainConfig.bitmapCount;
	page->pageIdx = request->pageIdx;
	page->bitmap = (SBitmap*)calloc(_peMainConfig.bitmapCount, sizeof(SBitmap));
	page->received.pageNum = request->pageIdx;
	page->received.copyNum = 0;

	rx_mutex_lock(_hMutexPage);
	rx_list_add(_hListPage, page);
	rx_mutex_unlock(_hMutexPage);

	if (_pPeItfParameter.callbackDrawCmd)
		_pPeItfParameter.callbackDrawCmd(&request->cmd, request->pageIdx);

	return REPLY_OK;
}

//--- i_peComm_LayoutCmd ------------------------------------------------------
int i_peComm_LayoutCmd(STarget *target, RX_SOCKET socket, void *msg, int len)
{
	struct mvt_core_loadlayoutcmd_req_v1 *request = (struct mvt_core_loadlayoutcmd_req_v1*) msg;

	if (target)
		return REPLY_OK;

	if (request->pageIdx == 1) i_peClearPageState();

	int nIx = request->pageIdx;
	int cpyInc = (request->cmd.recordEnd - request->cmd.recordStart) ? 1 : request->cmd.copyCount;
	for (int nCp = 0; nCp < request->cmd.copyCount; nCp += cpyInc) {
		for (int nId = request->cmd.recordStart; nId <= request->cmd.recordEnd; nId++) {
			SPage *page;
			page = (SPage*)calloc(1, sizeof(SPage));
			page->job_id = request->cmd.job_id;
			page->page_id = nId;
			page->copyCount = cpyInc;
			page->bitmapCount = _peMainConfig.bitmapCount;
			page->pageIdx = nIx;
			page->bitmap = (SBitmap*)calloc(_peMainConfig.bitmapCount, sizeof(SBitmap));
			page->received.pageNum = nIx;
			page->received.copyNum = nCp + cpyInc;

			rx_mutex_lock(_hMutexPage);
			rx_list_add(_hListPage, page);
			rx_mutex_unlock(_hMutexPage);
			nIx += cpyInc;
		}
	}

	if (_pPeItfParameter.callbackLoadlayout)
		_pPeItfParameter.callbackLoadlayout(&request->cmd, request->pageIdx);

	return REPLY_OK;
}

//--- i_peComm_RecordData -----------------------------------------------------
int i_peComm_RecordData(STarget *target, RX_SOCKET socket, void *msg, int len)
{
	struct mvt_core_loadrecorddata_req_v1 *request = (struct mvt_core_loadrecorddata_req_v1*) msg;

	if (target)
		return REPLY_OK;

	if (_pPeItfParameter.callbackRecordData)
		_pPeItfParameter.callbackRecordData(&request->cmd, request->pageIdx);

	return REPLY_OK;
}

// ****************************************************************************
// *			M E S S A G E S			I N T E R F A C E S
// ****************************************************************************

//--- i_peCommCore_RcvConfig --------------------------------------------------
int i_peCommCore_RcvConfig(STarget *target, RX_SOCKET socket, void *msg, int len)
{
	struct mvt_prt_config_ans_v1 *request = (struct mvt_prt_config_ans_v1 *) msg;
	
	if (target || !msg)
		return REPLY_OK;

	if (_peMainConfig.colors) {	free(_peMainConfig.colors); _peMainConfig.colors = NULL; }
	if (_peMainConfig.bitmaps) { free(_peMainConfig.bitmaps); _peMainConfig.bitmaps = NULL; }
	if (_peMainConfig.interfaces) {	free(_peMainConfig.interfaces); _peMainConfig.interfaces = NULL; }

	strncpy(_peMainConfig.id, request->id, sizeof(_peMainConfig.id));
	strncpy(_peMainConfig.ip, _pPeItfParameter.config->ipMain, sizeof(_peMainConfig.ip));
	_peMainConfig.resoHead = request->resoHead;
	_peMainConfig.resoPrint = request->resoPrint;
	_peMainConfig.maxWidth = request->maxWidth;
	_peMainConfig.maxHeight = request->maxHeight;
	_peMainConfig.maxSpeed = request->maxSpeed;
	_peMainConfig.duplex = request->duplex;
	_peMainConfig.dropSizeCount = request->dropSizeCount;
	_peMainConfig.bitPerPixel = request->bitPerPixel;
	_peMainConfig.alignment = request->alignment;
	_peMainConfig.speedPassCount = request->speedPassCount;

	_peMainConfig.colorCount = request->colorCount;
	_peMainConfig.colors = (SpeMainColor*)calloc(_peMainConfig.colorCount, sizeof(SpeMainColor));
	struct mvt_prt_color *color = (struct mvt_prt_color *) &request[1];
	for (int i = 0; i < request->colorCount; i++, color++) {
		_peMainConfig.colors[i].index = color->index;
		_peMainConfig.colors[i].colorCode = color->colorCode;
	}

	_peMainConfig.bitmapCount = request->bitmapCount;
	_peMainConfig.bitmaps = (SpeMainBitmap*)calloc(_peMainConfig.bitmapCount, sizeof(SpeMainBitmap));
	struct mvt_prt_bitmap *bitmap = (struct mvt_prt_bitmap *) color;
	for (int i = 0; i < request->bitmapCount; i++, bitmap++) {
		_peMainConfig.bitmaps[i].index = bitmap->index;
		_peMainConfig.bitmaps[i].interf = bitmap->interf;
		_peMainConfig.bitmaps[i].color = bitmap->color;
		_peMainConfig.bitmaps[i].side = bitmap->side;
		_peMainConfig.bitmaps[i].offset = bitmap->offset;
		_peMainConfig.bitmaps[i].width = bitmap->width;
		_peMainConfig.bitmaps[i].speedPass = bitmap->speedPass;
	}

	_peMainConfig.interfCount = request->interfCount;
	_peMainConfig.interfaces = (SpeMainInterface*)calloc(_peMainConfig.interfCount, sizeof(SpeMainInterface));
	struct mvt_prt_interface *interf = (struct mvt_prt_interface *) bitmap;
	for (int i = 0; i < request->interfCount; i++, interf++) {
		_peMainConfig.interfaces[i].index = interf->index;
		_peMainConfig.interfaces[i].connected = interf->connected;
		strncpy(_peMainConfig.interfaces[i].ip, interf->ip, sizeof(_peMainConfig.interfaces[i].ip));
		_peMainConfig.interfaces[i].port = interf->port;
	}
#ifdef _SIMULATOR
	if (_pPeItfParameter.callbackUpdateConfig)
		_pPeItfParameter.callbackUpdateConfig(&_peMainConfig);
#endif
	return REPLY_OK;
}

//--- i_peCommCore_InterfConfig -----------------------------------------------
int i_peCommCore_InterfConfig(STarget *target, RX_SOCKET socket, void *msg, int len)
{
	struct mvt_core_conf_req_v1 request;

	if (target || msg)
		return REPLY_OK;

	request.hdr.length = sizeof(struct mvt_core_conf_req_v1);
	request.hdr.id = MVT_CORE_REQ_CONF | MVT_CORE_INTERNAL;
	request.ver = CORE_CONFIG_VER;
	request.interfNum = _pPeItfParameter.config->interfaceNumber;
	strncpy(request.ip, _pPeItfParameter.config->ipInterface, sizeof(request.ip));
	request.port = _pPeItfParameter.config->portData;

	pe_send_msg(&socket, &request);

	return REPLY_OK;
}

//--- i_peCommCore_InterfState ------------------------------------------------
int i_peCommCore_InterfState(STarget *target, RX_SOCKET socket, void *msg, int len)
{
	if (target || msg)
		return REPLY_OK;

	if (!_printStateToSend)
		return REPLY_ERROR;

	_printStateToSend->watchtime = _mainWatchTime;
	_printStateToSend->session_id = _mainCurrentSession;
	_printStateToSend->connectedRipCount = (_hListTarget) ? rx_list_count(_hListTarget) : 0;
	_printStateToSend->sessionCount = 0;
	if (_mainCurrentSession) {
		rx_mutex_lock(_hMutexTarget);
		STarget *t = NULL;
		rx_list_begin(_hListTarget, (void**)&t);
		while (t) {
			if (t->currentSession == _mainCurrentSession)
				_printStateToSend->sessionCount++;
			rx_list_next(_hListTarget, (void**)&t);
		}
		rx_mutex_unlock(_hMutexTarget);
	}
	i_peUpdatePageState();

	pe_send_msg(&socket, _printStateToSend);

	if (TR_ITFSTATE_CNT) {
		static Smvt_core_state sv_printStateToSend;
		if (memcmp(&_printStateToSend->rcv_pg, &sv_printStateToSend.rcv_pg, sizeof(Smvt_prt_page)))
			pe_printf(TR_ITFSTATE_CNT, "i_peUpdatePageState received: %d - %d-%d-%d",
				_printStateToSend->rcv_pg.idx,
				_printStateToSend->rcv_pg.job_id,
				_printStateToSend->rcv_pg.page_id,
				_printStateToSend->rcv_pg.copy);
		if (memcmp(&_printStateToSend->sch_pg, &sv_printStateToSend.sch_pg, sizeof(Smvt_prt_page)))
			pe_printf(TR_ITFSTATE_CNT, "i_peUpdatePageState scheduled: %d - %d-%d-%d",
				_printStateToSend->sch_pg.idx,
				_printStateToSend->sch_pg.job_id,
				_printStateToSend->sch_pg.page_id,
				_printStateToSend->sch_pg.copy);
		if (memcmp(&_printStateToSend->cmt_pg, &sv_printStateToSend.cmt_pg, sizeof(Smvt_prt_page)))
			pe_printf(TR_ITFSTATE_CNT, "i_peUpdatePageState committed: %d - %d-%d-%d",
				_printStateToSend->cmt_pg.idx,
				_printStateToSend->cmt_pg.job_id,
				_printStateToSend->cmt_pg.page_id,
				_printStateToSend->cmt_pg.copy);
		if (memcmp(&_printStateToSend->prt_pg, &sv_printStateToSend.prt_pg, sizeof(Smvt_prt_page)))
			pe_printf(TR_ITFSTATE_CNT, "i_peUpdatePageState printed: %d - %d-%d-%d",
				_printStateToSend->prt_pg.idx,
				_printStateToSend->prt_pg.job_id,
				_printStateToSend->prt_pg.page_id,
				_printStateToSend->prt_pg.copy);
		memcpy(&sv_printStateToSend, _printStateToSend, sizeof(Smvt_core_state));
	}

	return REPLY_OK;
}

//--- i_peCommCore_MainState --------------------------------------------------
int i_peCommCore_MainState(STarget *target, RX_SOCKET socket, void *msg, int len)
{
	struct mvt_core_mainstate_req_v1 *request = (struct mvt_core_mainstate_req_v1*) msg;

	if (target)
		return REPLY_OK;

	if (!_printStateToSend)
		return REPLY_OK;

	if (_printStateToSend->state != request->state) {
		_printStateToSend->state = request->state;
		if (!request->state) {
			// Clear job, pages, ...
			i_peClearPageList();
		}
	}
#ifdef _SIMULATOR
	if (_pPeItfParameter.callbackState)
		_pPeItfParameter.callbackState(request->state);
#endif
	return REPLY_OK;
}


#ifdef _SIMULATOR
// ****************************************************************************
// *			M E S S A G E S			S I M U L A T O R
// ****************************************************************************

//--- i_peComm_SimuSpeed -----------------------------------------------------
int i_peComm_SimuSpeed(STarget *target, RX_SOCKET socket, void *msg, int len)
{
	struct mvt_core_mainspeed_req_v1 *request = (struct mvt_core_mainspeed_req_v1*) msg;

	if (target)
		return REPLY_OK;

	if (_pPeItfParameter.callbackSpeed)
		_pPeItfParameter.callbackSpeed(request->speed, request->size);

	return REPLY_OK;
}
#endif

// ****************************************************************************
