// ****************************************************************************
//
//	rx_pecore_main.c
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
#define	MAX_FORWARD				25		/* Size of the forward message fifo */
#define	TIMEOUT_FIFO			1000	/* 1 sec. */

#define	SURVEY_TIME				5000	/* 5 sec. */
#define	SHUTDOWN_SURVEY_TIME	200		/* 0.2 sec. */

			// Flag for trace control (trace if true)
#define TRAPI_PRINTSTATE		(_pPeMainParameter.traceFlags & TF_API_PRINTSTATE)		// call Function API PrintState
#define TRAPI_PRINTERROR		(_pPeMainParameter.traceFlags & TF_API_PRINTERROR)		// call Function API PrintError
#define	TRAPI_GETPAGE			(_pPeMainParameter.traceFlags & TF_API_GETPAGE)			// call Function API GetPage
#define	TRAPI_PAGESTACKED		(_pPeMainParameter.traceFlags & TF_API_PAGESTACKED)		// call Function API PageStacked

#define TR_RCVMSG				(_pPeMainParameter.traceFlags & TF_RCVMSG)				// Receive message from main
#define TR_RCVPAGE				(_pPeMainParameter.traceFlags & TF_RCVPAGE)				// Receive message page (load)

#define TR_PAGESTATE_LISTCNT	(_pPeMainParameter.traceFlags & TF_PAGESTATE_LISTCNT)	// Count of page list (at each state dialog)
#define TR_PAGESTATE_INTERF		(_pPeMainParameter.traceFlags & TF_PAGESTATE_INTERF)	// Counter and state of interface

//--- Structures, Typedefs ----------------------------------------------------
typedef enum EConnect {
	EC_NONE = 0,		// Not connected and not initialized
	EC_CONNECTED,		// Connected but not initialized
	EC_INITIALIZED		// Initialized
} EConnect;

typedef enum ELink {
	EL_NONE = 0,		// No link
	EL_RIPMAIN,			// Link with Rip
	EL_INTERF			// Link with Interface
} ELink;

typedef enum EPage {
	EP_NOTHING = 0,
	EP_RECEIVING,
	EP_RECEIVED,
	EP_SCHEDULED,
	EP_COMMITTED,
	EP_PRINTING,
	EP_PRINTED,
	EP_STACKED
} EPage;

typedef struct STarget {
	HANDLE		id;
	char		peerName[32];	/* Name or address ip */
	RX_SOCKET	socket;
	EConnect	connect;
	ELink		type;
	int			watchTime;		/* state watchtime in ms, if # 0 */
	int			currentSession;	/* id of the session # 0 */
	int			pageIdx;		/* Starts at 1 and is incremented at each page or copy it is reset after a stop (immediate or scheduled) */
	int			pageRecordIdx;	/* page index for record data cmd */

					/* Interface only */
	int			interfNumber;	/* -1: not defined, number of this interface start 0 */
	char		ip[32];			/* address ip of this interface */
	int			port;			/* data port */
	int			ripCount;		/* count of Rip server connected to the data port*/
	int			sessionCount;	/* count of opened session */
} STarget;

typedef struct SInterf {
	SCounter	received;
	SCounter	scheduled;
	SCounter	committed;
	SCounter	printed;
} SItfCounter;

typedef struct SPage {
	int			job_id;
	int			page_id;
	int			bitmapCount;
	int			copyCount;
	int			pageIdx;
	int			copyCurrent;
	SCounter	stacked;
	SItfCounter *itfCounter;
	SpeJobType	jobType;
	char		*info;
} SPage;

//--- Prototypes --------------------------------------------------------------

					/* API - Interface */
static int		i_peTraceFlagChange			(UINT32 flags);
static int		i_pePrintState				(int value);
static int		i_pePrintError				(int value);
#ifdef _SIMULATOR
static int		i_pePrintSpeed				(int value, int size);
#else
static int		i_pePrintSpeed				(int value);
#endif
static int		i_peGetPage					(INT32 pageNum);
static int		i_pePageStacked				(INT32 pageNum);
static int		i_peGetPgeJob				(INT32 *count, SpeMainPgeJob **job);

					/* Threads */
static void		*i_peMainThread				(void *param);
static void		*i_peMainForwardThread		(void *param);
static void		*i_peMainStateThread		(void *param);

					/* Helpers */
static STarget	*i_peTargetSource			(RX_SOCKET socket);
static void		i_peUpdatePrinterState		(void);
static void		i_peClearPageState			(void);
static void		i_peClearPageList			(void);
static void		i_peUpdateCommMainState		(void);

					/* Main server */
static void		i_peCommMain_init			(void);
static int		i_peCommMain_msg			(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *par);
static int		i_peCommMain_open			(RX_SOCKET socket, const char *peerName);
static int		i_peCommMain_close			(RX_SOCKET socket, const char *peerName);

					/* Messages */
static int		i_peCommMain_Ping			(STarget *target, RX_SOCKET socket, void *msg, int len);
static int		i_peCommMain_Config			(STarget *target, RX_SOCKET socket, void *msg, int len);
static int		i_peCommMain_Open			(STarget *target, RX_SOCKET socket, void *msg, int len);
static int		i_peCommMain_Close			(STarget *target, RX_SOCKET socket, void *msg, int len);
static int		i_peCommMain_StartCmd		(STarget *target, RX_SOCKET socket, void *msg, int len);
static int		i_peCommMain_StopCmd		(STarget *target, RX_SOCKET socket, void *msg, int len);
static int		i_peCommMain_Command		(STarget *target, RX_SOCKET socket, void *msg, int len);
static int		i_peCommMain_Synchro		(STarget *target, RX_SOCKET socket, void *msg, int len);
static int		i_peCommMain_Adjust			(STarget *target, RX_SOCKET socket, void *msg, int len);
static int		i_peCommMain_Page			(STarget *target, RX_SOCKET socket, void *msg, int len);
static int		i_peCommMain_State			(STarget *target, RX_SOCKET socket, void *msg, int len);
static int		i_peCommMain_FileCmd		(STarget *target, RX_SOCKET socket, void *msg, int len);
static int		i_peCommMain_DrawCmd		(STarget *target, RX_SOCKET socket, void *msg, int len);
static int		i_peCommMain_LayoutCmd		(STarget *target, RX_SOCKET socket, void *msg, int len);
static int		i_peCommMain_RecordData		(STarget *target, RX_SOCKET socket, void *msg, int len);

static int		i_peCommCore_InterfConfig	(STarget *target, RX_SOCKET socket, void *msg, int len);
static int		i_peCommCore_InterfState	(STarget *target, RX_SOCKET socket, void *msg, int len);
static void		i_pePageUpdate				(int interf, Smvt_prt_page *pge, EPage state);

//--- Locals ------------------------------------------------------------------
static void				*_threadPeMain = NULL;
static void				*_threadPeMainForward = NULL;
static void				*_threadPeMainState = NULL;
static int				_bShutDown = FALSE;

static HANDLE			_hPeMainServer = NULL;
static SpeState			_statePeCommMain = PE_CLOSED;
static SpeMainPara		_pPeMainParameter;

Smvt_prt_state			*_printStateAnswer = NULL;

static HANDLE			_hFifoMsgForward = NULL;

static HANDLE			_hListTarget = NULL;
static HANDLE			_hMutexTarget = NULL;

static HANDLE			_hListPage = NULL;
static HANDLE			_hMutexPage = NULL;


// ****************************************************************************

// ****************************************************************************
// *			P U B L I C S
// ****************************************************************************

//--- rx_pemain_start ---------------------------------------------------------
void rx_pemain_start(SpeMainPara *peMain, SpeMainAPI **peAPI)
{
	SpeMainAPI *pAPI = NULL;
	
	if (!peMain || !peAPI)
		return;
	
	*peAPI = NULL;
	if (_threadPeMain) {
		pe_printf(TRUE, "_threadPeMain Already exists, must be stop before re-start. _statePeCommMain=%d", _statePeCommMain);
		return;
	}
	_bShutDown = FALSE;

	if (_hFifoMsgForward) { rx_fifo_close(_hFifoMsgForward); _hFifoMsgForward = NULL; }
	rx_fifo_create(&_hFifoMsgForward, MAX_FORWARD);

	if (_hListPage) { rx_list_close(_hListPage); _hListPage = NULL; }
	rx_list_create(&_hListPage);

	if (_hMutexPage) rx_mutex_destroy(&_hMutexPage);
	_hMutexPage = rx_mutex_create();

	_statePeCommMain = PE_OPENING;

	memcpy(&_pPeMainParameter, peMain, sizeof(SpeMainPara));
	_pPeMainParameter.config = (SpeMainConfig*)calloc(1, sizeof(SpeMainConfig));
	if (!_pPeMainParameter.config) goto error;
	memcpy(_pPeMainParameter.config, peMain->config, sizeof(SpeMainConfig));
	_pPeMainParameter.config->colors = NULL;
	_pPeMainParameter.config->bitmaps = NULL;
	_pPeMainParameter.config->interfaces = NULL;

	int size = _pPeMainParameter.config->colorCount*sizeof(SpeMainColor);
	_pPeMainParameter.config->colors = (SpeMainColor*)malloc(size);
	if (!_pPeMainParameter.config->colors) goto error;
	memcpy(_pPeMainParameter.config->colors, peMain->config->colors, size);
	size = _pPeMainParameter.config->bitmapCount*sizeof(SpeMainBitmap);
	_pPeMainParameter.config->bitmaps = (SpeMainBitmap*)malloc(size);
	if (!_pPeMainParameter.config->bitmaps) goto error;
	memcpy(_pPeMainParameter.config->bitmaps, peMain->config->bitmaps, size);
	size = _pPeMainParameter.config->interfCount*sizeof(SpeMainInterface);
	_pPeMainParameter.config->interfaces = (SpeMainInterface*)malloc(size);
	if (!_pPeMainParameter.config->interfaces) goto error;
	memcpy(_pPeMainParameter.config->interfaces, peMain->config->interfaces, size);

	pAPI = (SpeMainAPI*)calloc(1, sizeof(SpeMainAPI));
	if (!pAPI) goto error;

	pAPI->TraceFlagsUpdate = i_peTraceFlagChange;
	pAPI->PrintError = i_pePrintError;
	pAPI->PrintSpeed = i_pePrintSpeed;
	pAPI->PrintState = i_pePrintState;
	pAPI->GetPage = i_peGetPage;
	pAPI->PageStacked = i_pePageStacked;
	pAPI->GetPgeJob = i_peGetPgeJob;

	_threadPeMain = rx_thread_start(i_peMainThread, NULL, 0, "_peMain_thread");
	
	_threadPeMainForward = rx_thread_start(i_peMainForwardThread, NULL, 0, "_peMainForward_thread");

	*peAPI = pAPI;
	return;

error:
	_statePeCommMain = PE_FAILED;
	
	if (pAPI) free(pAPI);
	if (_pPeMainParameter.config) {
		if (_pPeMainParameter.config->interfaces)	free(_pPeMainParameter.config->interfaces);
		if (_pPeMainParameter.config->bitmaps)		free(_pPeMainParameter.config->bitmaps);
		if (_pPeMainParameter.config->colors)		free(_pPeMainParameter.config->colors);
		free(_pPeMainParameter.config);
		_pPeMainParameter.config = NULL;
	}
}

//--- rx_pemain_stop ----------------------------------------------------------
void   rx_pemain_stop(void)
{
	if ((_statePeCommMain != PE_CLOSED) && (_statePeCommMain != PE_CLOSING)) {
		_bShutDown = TRUE;
		_statePeCommMain = PE_CLOSING;

#ifdef _SIMULATOR
		if (_pPeMainParameter.callbackEngineState)
			_pPeMainParameter.callbackEngineState(_printStateAnswer);
#endif
	}
	else if (_statePeCommMain != PE_CLOSED)
	{
		pe_printf(TRUE, "rx_pemain_stop, _statePeCommMain=%d", _statePeCommMain);
	}
}

//--- rx_pemain_state ---------------------------------------------------------
SpeState rx_pemain_state(void)
{
	return _statePeCommMain;
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
	_pPeMainParameter.traceFlags = flags;
	return REPLY_OK;
}

//--- i_pePrintState ----------------------------------------------------------
int i_pePrintState(int value)
{
	if (_printStateAnswer) {
		if (_printStateAnswer->state != value) {
			_printStateAnswer->state = value;
			if (!value) {
				// Clear job, pages, ...
				i_peClearPageList();
			}
		}
	}

	// SendTo All slave
	int len = sizeof(struct mvt_core_mainstate_req_v1);
	struct mvt_core_mainstate_req_v1 *buffer = malloc(len);
	if (buffer) {
		buffer->hdr.length = len;
		buffer->hdr.id = MVT_CORE_REQ_MAINSTATE | MVT_CORE_INTERNAL;
		buffer->state = value;
		rx_fifo_put(_hFifoMsgForward, buffer, len);

		pe_printf(TRAPI_PRINTSTATE, "i_pePrintState %d", value);
	}
	return REPLY_OK;
}

//--- i_pePrintError ----------------------------------------------------------
int i_pePrintError(int value)
{
	if (_printStateAnswer)
		_printStateAnswer->error = value;
	if (value) {
		// Clear job, pages, ...
		i_peClearPageList();
	}
	pe_printf(TRAPI_PRINTERROR, "i_pePrintError %d", value);
	return REPLY_OK;
}

//--- i_pePrintSpeed ----------------------------------------------------------
#ifdef _SIMULATOR
int i_pePrintSpeed(int value, int size)
{
	if (_printStateAnswer)
		_printStateAnswer->speed = value;
	// SendTo All slave
	int len = sizeof(struct mvt_core_mainspeed_req_v1);
	struct mvt_core_mainspeed_req_v1 *buffer = malloc(len);
	if (buffer) {
		buffer->hdr.length = len;
		buffer->hdr.id = MVT_CORE_REQ_MAINSPEED | MVT_CORE_INTERNAL;
		buffer->speed = value;
		buffer->size = size;
		rx_fifo_put(_hFifoMsgForward, buffer, len);

		pe_printf(FALSE, "i_pePrintSpeed %d, pagesize %d", value, size);
	}
	return REPLY_OK;
}
#else
int i_pePrintSpeed(int value)
{
	if (_printStateAnswer)
		_printStateAnswer->speed = value;
	return REPLY_OK;
}
#endif

//--- i_peGetPage -------------------------------------------------------------
int i_peGetPage(int pageNum)
{
	SPage *page = NULL;

	if (!pageNum)
		return REPLY_ERROR;

	rx_mutex_lock(_hMutexPage);
	rx_list_begin(_hListPage, (void**)&page);
	while (page) {
		if ((pageNum < (page->pageIdx + page->copyCount)) && (pageNum >= page->pageIdx)) {
			for (int i = 1; page && (i < _pPeMainParameter.config->interfCount); i++) {
				if (!page->itfCounter[i].received.pageNum || (pageNum < page->itfCounter[i].received.pageNum))
					page = NULL;
			}
			break;
		}
		rx_list_next(_hListPage, (void**)&page);
	}
	rx_mutex_unlock(_hMutexPage);

	pe_printf(TRAPI_GETPAGE, "i_peGetPage %d / %p", pageNum, page);

	return (page) ? REPLY_OK : REPLY_NOT_FOUND;
}

//--- i_pePageStacked ---------------------------------------------------------
int i_pePageStacked(int pageNum)
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
	if (page && (page->stacked.pageNum != pageNum)) {
		page->stacked.pageNum = pageNum;
		page->stacked.copyNum = ++page->copyCurrent;
	}
	rx_mutex_unlock(_hMutexPage);

	pe_printf(TRAPI_PAGESTACKED, "i_pePageStacked %d / %p", pageNum, page);

	return (page) ? REPLY_OK : REPLY_NOT_FOUND;
}

//--- i_peGetPgeJob -----------------------------------------------------------
int i_peGetPgeJob(int *count, SpeMainPgeJob **JOB)
{
	SpeMainPgeJob *job;
	SPage *page;
	int i;

	if (!count || !JOB)
		return REPLY_ERROR;
	*count = rx_list_count(_hListPage);
	*JOB = NULL;

	if (!*count)
		return REPLY_NOT_FOUND;

	rx_mutex_lock(_hMutexPage);
	job = (SpeMainPgeJob*)calloc(rx_list_count(_hListPage), sizeof(SpeMainPgeJob));
	if (!job) {
		rx_mutex_unlock(_hMutexPage);
		return REPLY_ERROR;
	}
	i = 0;
	page = 0;
	rx_list_begin(_hListPage, (void**)&page);
	while (page) {
		if (job[i].jobId != page->job_id) {
			if (job[i].jobId) i++;
			job[i].jobId = page->job_id;
			job[i].pageStart = page->page_id;
			job[i].pageEnd = page->page_id;
			job[i].pageCopy = page->copyCount;
			job[i].jobType = page->jobType;
			if (page->info)
				job[i].info = strdup(page->info);
		}
		else if (job[i].pageStart > page->page_id)
			job[i].pageStart = page->page_id;
		else if (job[i].pageEnd < page->page_id)
			job[i].pageEnd = page->page_id;
		else if (job[i].pageStart == page->page_id)
			job[i].pageCopy++;
		rx_list_next(_hListPage, (void**)&page);
	}
	rx_mutex_unlock(_hMutexPage);
	*count = i + 1;
	*JOB = job;
	return REPLY_OK;
}

// ****************************************************************************
// *			T H R E A D S
// ****************************************************************************

//--- i_peMainThread ----------------------------------------------------------
void *i_peMainThread(void *param)
{
	if (_hMutexTarget) rx_mutex_destroy(&_hMutexTarget);
	_hMutexTarget = rx_mutex_create();
	rx_list_create(&_hListTarget);

	if (_printStateAnswer) { free(_printStateAnswer); _printStateAnswer = NULL; }
	if (!_printStateAnswer) {
		struct mvt_prt_state_ans_v1 *answer = NULL;
		struct mvt_prt_interfstate *interf = NULL;
		int size;

		size = sizeof (struct mvt_prt_state_ans_v1) + (_pPeMainParameter.config->interfCount*sizeof(struct mvt_prt_interfstate));
		answer = (struct mvt_prt_state_ans_v1 *) calloc(1, size);
		if (!answer) {
			_statePeCommMain = PE_FAILED;
			goto error;
		}
		answer->hdr.id = MVT_PRT_ANS_STATE;
		answer->hdr.length = size;
		answer->ver = STATE_VER;
		answer->interfCount = _pPeMainParameter.config->interfCount;
		_printStateAnswer = answer;
	}

	i_peCommMain_init();

	while (!_bShutDown)
		rx_sleep(SURVEY_TIME);
	
	/* Close the remainded targets */
	if (_hListTarget) {
		STarget *target = NULL;
		rx_mutex_lock(_hMutexTarget);
		rx_list_begin(_hListTarget, (void**)&target);
		while (target) {
			if (target->connect)
				sok_close(&target->socket);
			rx_list_next(_hListTarget, (void**)&target);
		}
		rx_mutex_unlock(_hMutexTarget);
	}

	/* Wait end of threads state and forward */
	while (_threadPeMainState || _threadPeMainForward) rx_sleep(SHUTDOWN_SURVEY_TIME);

	/* Wait end of connections and close server */
	while (_hListTarget && rx_list_count(_hListTarget)) rx_sleep(SHUTDOWN_SURVEY_TIME);
	sok_stop_server(&_hPeMainServer);
	_hPeMainServer = NULL;

	_statePeCommMain = PE_CLOSED;

#ifdef _SIMULATOR
	if (_pPeMainParameter.callbackEngineState)
		_pPeMainParameter.callbackEngineState(NULL);
#endif

error:
	/* Close fifo */
	if (_hFifoMsgForward) {
		while (rx_fifo_count(_hFifoMsgForward) > 0) {
			void *msg = NULL;
			int ln;
			rx_fifo_get(_hFifoMsgForward, &msg, &ln, TIMEOUT_FIFO);
			if (msg) free(msg);
		}
		rx_fifo_close(_hFifoMsgForward);
		_hFifoMsgForward = NULL;
	}

	/* Clear page list */
	i_peClearPageList();
	if (_hListPage) { rx_list_close(_hListPage); _hListPage = NULL; }
	if (_hMutexPage) rx_mutex_destroy(&_hMutexPage);

	/* Clear status */
	if (_printStateAnswer) free(_printStateAnswer);
	_printStateAnswer = NULL;

	/* Free config. */
	if (_pPeMainParameter.config->colors) free(_pPeMainParameter.config->colors);
	if (_pPeMainParameter.config->bitmaps) free(_pPeMainParameter.config->bitmaps);
	if (_pPeMainParameter.config->interfaces) free(_pPeMainParameter.config->interfaces);
	if (_pPeMainParameter.config) free(_pPeMainParameter.config);
	_pPeMainParameter.config = NULL;

	/* Clear target list */
	if (_hListTarget) { rx_list_close(_hListTarget); _hListTarget = NULL; }
	if (_hMutexTarget) { rx_mutex_destroy(&_hMutexTarget); _hMutexTarget = NULL; }

	/* Clear main thread */
	_threadPeMain = NULL;
	_bShutDown = FALSE;
	return NULL;
}

//--- i_peMainForwardThread ---------------------------------------------------
void *i_peMainForwardThread(void *param)
{
	void *msg = NULL;
	int ln;

	while (_hFifoMsgForward && !_bShutDown) {
		if (rx_fifo_get(_hFifoMsgForward, &msg, &ln, TIMEOUT_FIFO)) {
			pe_printf(TRUE, "_threadPeMainForward access fifo failed");
		}
		if (_bShutDown)
			break;
		if (msg && ln) {
			STarget *target = NULL;
			rx_mutex_lock(_hMutexTarget);
			rx_list_begin(_hListTarget, (void**)&target);
			while (target) {
				if (target->type == EL_INTERF)
					pe_send_msg(&target->socket, msg);
				rx_list_next(_hListTarget, (void**)&target);
			}
			rx_mutex_unlock(_hMutexTarget);
		}
		if (msg) free(msg);
		msg = NULL;
	}
	if (msg) free(msg);

	/* Close fifo */
	if (_hFifoMsgForward) {
		while (rx_fifo_count(_hFifoMsgForward) > 0) {
			rx_fifo_get(_hFifoMsgForward, &msg, &ln, TIMEOUT_FIFO);
			if (msg) free(msg);
		}
		rx_fifo_close(_hFifoMsgForward);
		_hFifoMsgForward = NULL;
	}
	_threadPeMainForward = NULL;
	return NULL;
}

//--- i_peMainStateThread -----------------------------------------------------
void *i_peMainStateThread(void *param)
{
	while (!_bShutDown) {
		STarget *target = NULL;
		rx_mutex_lock(_hMutexTarget);
		rx_list_begin(_hListTarget, (void**)&target);
		while (target && ((target->type != EL_RIPMAIN) || (target->connect != EC_INITIALIZED)))
			rx_list_next(_hListTarget, (void**)&target);
		rx_mutex_unlock(_hMutexTarget);
		if (_bShutDown || !target || !target->watchTime)
			break;
		i_peCommMain_State(target, target->socket, NULL, 0);
		rx_sleep(target->watchTime);
	}
	_threadPeMainState = NULL;
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

//--- i_peUpdatePrinterState --------------------------------------------------
void i_peUpdatePrinterState(void)
{
	SPage *page = NULL;
	STarget *target = NULL;
	struct mvt_prt_interfstate *interfState = NULL;

	if (!_printStateAnswer)
		return;

	// Interface
	interfState = (struct mvt_prt_interfstate *)&_printStateAnswer[1];
	memset(interfState, 0, _printStateAnswer->interfCount * sizeof(struct mvt_prt_interfstate));
	rx_mutex_lock(_hMutexTarget);
	rx_list_begin(_hListTarget, (void**)&target);
	while (target) {
		if (target->type == EL_INTERF) {
			interfState[target->interfNumber].connected = (target->connect == EC_INITIALIZED) ? 1 : 0;
			interfState[target->interfNumber].sessionCount = target->sessionCount;
//			interfState[target->interfNumber].ripCount = target->ripCount;
		}
		rx_list_next(_hListTarget, (void**)&target);
	}
	rx_mutex_unlock(_hMutexTarget);

	// Page Counter
	int mask = 0;
	rx_mutex_lock(_hMutexPage);
	rx_list_end(_hListPage, (void**)&page);
	while (page) {
		if (page->itfCounter[0].received.pageNum && !(mask & (1 << 0))) {
			mask |= (1 << 0);
			_printStateAnswer->rcv_pg.job_id = page->job_id;
			_printStateAnswer->rcv_pg.page_id = page->page_id;
			_printStateAnswer->rcv_pg.idx = page->itfCounter[0].received.pageNum;
			_printStateAnswer->rcv_pg.copy = page->itfCounter[0].received.copyNum;
			for (int i = 1; i < _pPeMainParameter.config->interfCount; i++) {
				if (page->itfCounter[i].received.pageNum < _printStateAnswer->rcv_pg.idx) {
					_printStateAnswer->rcv_pg.idx = page->itfCounter[i].received.pageNum;
					_printStateAnswer->rcv_pg.copy = page->itfCounter[i].received.copyNum;
				}
			}
		}
		if (page->itfCounter[0].scheduled.pageNum && !(mask & (1 << 1))) {
			mask |= (1 << 1);
			_printStateAnswer->sch_pg.job_id = page->job_id;
			_printStateAnswer->sch_pg.page_id = page->page_id;
			_printStateAnswer->sch_pg.idx = page->itfCounter[0].scheduled.pageNum;
			_printStateAnswer->sch_pg.copy = page->itfCounter[0].scheduled.copyNum;
			for (int i = 1; i < _pPeMainParameter.config->interfCount; i++) {
				if (page->itfCounter[i].scheduled.pageNum > _printStateAnswer->sch_pg.idx) {
					_printStateAnswer->sch_pg.idx = page->itfCounter[i].scheduled.pageNum;
					_printStateAnswer->sch_pg.copy = page->itfCounter[i].scheduled.copyNum;
				}
			}
		}
		if (page->itfCounter[0].committed.pageNum && !(mask & (1 << 2))) {
			mask |= (1 << 2);
			_printStateAnswer->cmt_pg.job_id = page->job_id;
			_printStateAnswer->cmt_pg.page_id = page->page_id;
			_printStateAnswer->cmt_pg.idx = page->itfCounter[0].committed.pageNum;
			_printStateAnswer->cmt_pg.copy = page->itfCounter[0].committed.copyNum;
			for (int i = 1; i < _pPeMainParameter.config->interfCount; i++) {
				if (page->itfCounter[i].committed.pageNum > _printStateAnswer->cmt_pg.idx) {
					_printStateAnswer->cmt_pg.idx = page->itfCounter[i].committed.pageNum;
					_printStateAnswer->cmt_pg.copy = page->itfCounter[i].committed.copyNum;
				}
			}
		}
		if (page->itfCounter[0].printed.pageNum && !(mask & (1 << 3))) {
			mask |= (1 << 3);
			_printStateAnswer->prt_pg.job_id = page->job_id;
			_printStateAnswer->prt_pg.page_id = page->page_id;
			_printStateAnswer->prt_pg.idx = page->itfCounter[0].printed.pageNum;
			_printStateAnswer->prt_pg.copy = page->itfCounter[0].printed.copyNum;
			for (int i = 1; i < _pPeMainParameter.config->interfCount; i++) {
				if (page->itfCounter[i].printed.pageNum < _printStateAnswer->prt_pg.idx) {
					_printStateAnswer->prt_pg.idx = page->itfCounter[i].printed.pageNum;
					_printStateAnswer->prt_pg.copy = page->itfCounter[i].printed.copyNum;
				}
			}
		}
		if (page->stacked.pageNum) {
			if (!(mask & (1 << 4))) {
				mask |= (1 << 4);
				_printStateAnswer->stk_pg.job_id = page->job_id;
				_printStateAnswer->stk_pg.page_id = page->page_id;
				_printStateAnswer->stk_pg.idx = page->stacked.pageNum;
				_printStateAnswer->stk_pg.copy = page->stacked.copyNum;
			}

			if (((_printStateAnswer->stk_pg.job_id != page->job_id) || (_printStateAnswer->stk_pg.page_id != page->page_id)) ||
				(page->copyCount == 1) || (page->copyCount == page->stacked.copyNum)) {
				SPage *delpage = NULL;
				rx_list_begin(_hListPage, (void**)&delpage);
				while (delpage && (delpage != page)) {
					if (delpage->itfCounter) free(delpage->itfCounter);
					if (delpage->info) free(delpage->info);
					free(delpage);
					rx_list_remove(_hListPage, NULL);
					rx_list_begin(_hListPage, (void**)&delpage);
				}
				if (page->itfCounter) free(page->itfCounter);
				if (page->info) free(page->info);
				free(page);
				rx_list_remove(_hListPage, page);
				break;
			}
		}
		rx_list_prev(_hListPage, (void**)&page);
	}
	rx_mutex_unlock(_hMutexPage);

	pe_printf(TR_PAGESTATE_LISTCNT, "i_peUpdatePrinterState ListPage Count: %d", rx_list_count(_hListPage));
}

//--- i_peClearPageState ------------------------------------------------------
void i_peClearPageState(void)
{
	if (!_printStateAnswer)
		return;
	memset(&_printStateAnswer->rcv_pg, 0, sizeof(Smvt_prt_page));
	memset(&_printStateAnswer->sch_pg, 0, sizeof(Smvt_prt_page));
	memset(&_printStateAnswer->cmt_pg, 0, sizeof(Smvt_prt_page));
	memset(&_printStateAnswer->prt_pg, 0, sizeof(Smvt_prt_page));
	memset(&_printStateAnswer->stk_pg, 0, sizeof(Smvt_prt_page));
}

//--- i_peClearPageList -------------------------------------------------------
void i_peClearPageList(void)
{
	rx_mutex_lock(_hMutexPage);
	if (_hListPage) {
		while (rx_list_count(_hListPage) > 0) {
			SPage *page = NULL;
			rx_list_begin(_hListPage, (void**)&page);
			if (page) {
				if (page->itfCounter)	free(page->itfCounter);
				if (page->info)			free(page->info);
				free(page);
			}
			rx_list_remove(_hListPage, NULL);
		}
	}
	rx_mutex_unlock(_hMutexPage);

	if (_hListTarget) {
		STarget *target = NULL;
		rx_mutex_lock(_hMutexTarget);
		rx_list_begin(_hListTarget, (void**)&target);
		while (target && (target->type != EL_RIPMAIN))
			rx_list_next(_hListTarget, (void**)&target);
		if (target)
			target->pageIdx = 1;
		rx_mutex_unlock(_hMutexTarget);
	}
}

//--- i_peUpdateCommMainState -------------------------------------------------
void i_peUpdateCommMainState(void)
{
#ifdef _SIMULATOR
	SpeState OldState = _statePeCommMain;
#endif

	int i = 0;
	STarget *t = NULL;
	rx_mutex_lock(_hMutexTarget);
	rx_list_begin(_hListTarget, (void**)&t);
	while (t) {
		if ((t->type == EL_INTERF) && (t->connect == EC_INITIALIZED)) i++;
		rx_list_next(_hListTarget, (void**)&t);
	}
	rx_mutex_unlock(_hMutexTarget);

	_statePeCommMain = (!i) ? PE_DISCONNECTED : (i != _pPeMainParameter.config->interfCount) ? PE_LISTEN : PE_CONNECTED;

#ifdef _SIMULATOR
	if (_pPeMainParameter.callbackEngineState && (OldState != _statePeCommMain))
		_pPeMainParameter.callbackEngineState(_printStateAnswer);
#endif
}

// ****************************************************************************
// *			M A I N		S E R V E R
// ****************************************************************************

//--- i_peCommMain_init -------------------------------------------------------
void i_peCommMain_init(void)
{
	_statePeCommMain = PE_DISCONNECTED;

	if (sok_start_server(&_hPeMainServer, NULL, _pPeMainParameter.config->port, SOCK_STREAM, 32, i_peCommMain_msg, i_peCommMain_open, i_peCommMain_close))
		_statePeCommMain = PE_FAILED;
}

//--- i_peCommMain_open -------------------------------------------------------
int i_peCommMain_open(RX_SOCKET socket, const char *peerName)
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
	target->connect = EC_CONNECTED;
	target->type = EL_NONE;
	rx_mutex_lock(_hMutexTarget);
	rx_list_add(_hListTarget, target);
	rx_mutex_unlock(_hMutexTarget);

	pe_printf(TRUE, "peCommMain Server opened: >>%s<<", peerName);

	i_peUpdateCommMainState();
	return REPLY_OK;

error:
	return REPLY_ERROR;
}

//--- i_peCommMain_close ------------------------------------------------------
int i_peCommMain_close(RX_SOCKET socket, const char *peerName)
{
	STarget *target = NULL;

	if (!_hListTarget) goto error;

	rx_mutex_lock(_hMutexTarget);
	rx_list_begin(_hListTarget, (void**)&target);
	while (target && strcmp(target->peerName, peerName))
		rx_list_next(_hListTarget, (void**)&target);
	rx_mutex_unlock(_hMutexTarget);
	if (!target) goto error;
	if (target->type == EL_INTERF)
		_pPeMainParameter.config->interfaces[target->interfNumber].connected = 0;
	rx_mutex_lock(_hMutexTarget);
	rx_list_remove(_hListTarget, target);
	rx_mutex_unlock(_hMutexTarget);
	free(target);

	pe_printf(TRUE, "peCommMain Server closed: >>%s<<", peerName);

	if (_pPeMainParameter.callbackPrintStop)
		_pPeMainParameter.callbackPrintStop(0);

	i_peClearPageList();
	i_peUpdatePrinterState();
	i_peUpdateCommMainState();

#ifdef _SIMULATOR
	if (_pPeMainParameter.callbackUpdateConfig)
		_pPeMainParameter.callbackUpdateConfig(_pPeMainParameter.config);
#endif
	return REPLY_OK;

error:
	return REPLY_ERROR;
}

//--- i_peCommMain_msg --------------------------------------------------------
int i_peCommMain_msg(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *par)
{
	int reply = REPLY_ERROR;
	static int _err = FALSE;
	STarget *target;
	Smvt_prt_header *phdr = (Smvt_prt_header*)msg;

	target = i_peTargetSource(socket);
	if (!target)
		return reply;

	if (!target->type) {
		rx_mutex_lock(_hMutexTarget);
		target->type = (phdr->id & MVT_CORE_INTERNAL) ? EL_INTERF : EL_RIPMAIN;
		rx_mutex_unlock(_hMutexTarget);
	}
	else if (((target->type == EL_INTERF) && !(phdr->id & MVT_CORE_INTERNAL)) || ((target->type == EL_RIPMAIN) && (phdr->id & MVT_CORE_INTERNAL)))
		return reply;

	if (target->type == EL_RIPMAIN) {
		int lenalloc;

		if (target->connect == EC_CONNECTED) {
			rx_mutex_lock(_hMutexTarget);
			target->connect = EC_INITIALIZED;
			rx_mutex_unlock(_hMutexTarget);
		}

		if (_statePeCommMain != PE_CONNECTED) {
			lenalloc = ((phdr->id == MVT_PRT_REQ_PING) || (phdr->id == MVT_PRT_REQ_CONF) || (phdr->id == MVT_PRT_REQ_STATE)) ? len : 0;
		}
		else {
			lenalloc = 0;
			switch (phdr->id) {
			case MVT_PRT_REQ_CMDPG:
				lenalloc = sizeof(struct mvt_core_loadpagecmd_req_v1);
				break;
			case MVT_PRT_REQ_FILECMD:
				if (_pPeMainParameter.callbackLoadFile)
					lenalloc = sizeof(struct mvt_core_loadfilecmd_req_v1);
				break;
			case MVT_PRT_REQ_DRAWCMD:
				if (_pPeMainParameter.callbackDrawCmd)
					lenalloc = sizeof(struct mvt_core_drawcmd_req_v1);
				break;
			case MVT_PRT_REQ_LAYOUTCMD:
				if (_pPeMainParameter.callbackLoadlayout)
					lenalloc = sizeof(struct mvt_core_loadlayoutcmd_req_v1);
				break;
			case MVT_PRT_REQ_RECORDDATA:
				if (_pPeMainParameter.callbackRecordData)
					lenalloc = sizeof(struct mvt_core_loadrecorddata_req_v1);
				break;
			default:
				lenalloc = len;
				break;
			}
		}

		if (lenalloc) {
			void *buffer = malloc(lenalloc);
			memcpy(buffer, msg, len);
			switch (phdr->id) {
			case MVT_PRT_REQ_CMDPG:
				{
					struct mvt_core_loadpagecmd_req_v1 *pbuf = (struct mvt_core_loadpagecmd_req_v1 *) buffer;
					pbuf->cmd.hdr.id = MVT_CORE_REQ_CMDPG | MVT_CORE_INTERNAL;
					pbuf->cmd.hdr.length = lenalloc;
					pbuf->pageIdx = target->pageIdx;
				}
				break;
			case MVT_PRT_REQ_FILECMD:
				{
					struct mvt_core_loadfilecmd_req_v1 *pbuf = (struct mvt_core_loadfilecmd_req_v1 *) buffer;
					pbuf->cmd.hdr.id = MVT_CORE_REQ_FILECMD | MVT_CORE_INTERNAL;
					pbuf->cmd.hdr.length = lenalloc;
					pbuf->pageIdx = target->pageIdx;
				}
				break;
			case MVT_PRT_REQ_DRAWCMD:
				{
					struct mvt_core_drawcmd_req_v1 *pbuf = (struct mvt_core_drawcmd_req_v1 *) buffer;
					pbuf->cmd.hdr.id = MVT_CORE_REQ_DRAWCMD | MVT_CORE_INTERNAL;
					pbuf->cmd.hdr.length = lenalloc;
					pbuf->pageIdx = target->pageIdx;
				}
				break;
			case MVT_PRT_REQ_LAYOUTCMD:
				{
					struct mvt_core_loadlayoutcmd_req_v1 *pbuf = (struct mvt_core_loadlayoutcmd_req_v1 *) buffer;
					pbuf->cmd.hdr.id = MVT_CORE_REQ_LAYOUTCMD | MVT_CORE_INTERNAL;
					pbuf->cmd.hdr.length = lenalloc;
					pbuf->pageIdx = target->pageIdx;
				}
				break;
			case MVT_PRT_REQ_RECORDDATA:
				{
					struct mvt_core_loadrecorddata_req_v1 *pbuf = (struct mvt_core_loadrecorddata_req_v1 *) buffer;
					pbuf->cmd.hdr.id = MVT_CORE_REQ_RECORDDATA | MVT_CORE_INTERNAL;
					pbuf->cmd.hdr.length = lenalloc;
					pbuf->pageIdx = target->pageRecordIdx;
				}
				break;
			}
			rx_fifo_put(_hFifoMsgForward, buffer, lenalloc);
		}
	}

	pe_printf(TR_RCVMSG, "PEM Recv msg 0x%X (%d) - len %d", phdr->id, (phdr->id & ~MVT_CORE_INTERNAL), phdr->length);

	switch (phdr->id) {
	case MVT_PRT_REQ_PING:
	case MVT_PRT_REQ_PING | MVT_CORE_INTERNAL:		reply = i_peCommMain_Ping(target, socket, msg, len);  break;
	case MVT_PRT_REQ_CONF:							reply = i_peCommMain_Config(target, socket, msg, len); break;
	case MVT_PRT_REQ_OPEN:							reply = i_peCommMain_Open(target, socket, msg, len); break;
	case MVT_PRT_REQ_CLOSE:							reply = i_peCommMain_Close(target, socket, msg, len); break;
	case MVT_PRT_REQ_START:							reply = i_peCommMain_StartCmd(target, socket, msg, len); break;
	case MVT_PRT_REQ_STOP:							reply = i_peCommMain_StopCmd(target, socket, msg, len); break;
	case MVT_PRT_REQ_CMD:							reply = i_peCommMain_Command(target, socket, msg, len); break;
	case MVT_PRT_REQ_SYNC:							reply = i_peCommMain_Synchro(target, socket, msg, len); break;
	case MVT_PRT_REQ_ADJ:							reply = i_peCommMain_Adjust(target, socket, msg, len); break;
	case MVT_PRT_REQ_CMDPG:							reply = i_peCommMain_Page(target, socket, msg, len); break;
	case MVT_PRT_REQ_DATAPG:
		ErrorEx(0, 0, ERR_CONT, 0, "Pe Main: Data Command not supported (0x%8x)", phdr->id);
		break;
	case MVT_PRT_REQ_STATE:							reply = i_peCommMain_State(target, socket, msg, len); break;
	case MVT_PRT_REQ_FILECMD:						reply = i_peCommMain_FileCmd(target, socket, msg, len); break;
	case MVT_PRT_REQ_DRAWCMD:						reply = i_peCommMain_DrawCmd(target, socket, msg, len); break;
	case MVT_PRT_REQ_LAYOUTCMD:						reply = i_peCommMain_LayoutCmd(target, socket, msg, len); break;
	case MVT_PRT_REQ_RECORDDATA:					reply = i_peCommMain_RecordData(target, socket, msg, len); break;

	case MVT_CORE_REQ_CONF | MVT_CORE_INTERNAL:
		{
			reply = i_peCommCore_InterfConfig(target, socket, msg, len);
			i_peUpdateCommMainState();
		}
		break;
	case MVT_CORE_REQ_STATE | MVT_CORE_INTERNAL:	reply = i_peCommCore_InterfState(target, socket, msg, len); break;

	case MVT_PRT_ANS_OPEN | MVT_CORE_INTERNAL:
	case MVT_PRT_ANS_CLOSE | MVT_CORE_INTERNAL:
	case MVT_PRT_ANS_CMDPG | MVT_CORE_INTERNAL:		reply = REPLY_OK; break;

	default:
		if (!_err) {
			_err = TRUE;
			ErrorEx(0, 0, ERR_CONT, 0, "Pe Main: Unknown dialog 0x%8x", phdr->id);
		}

		pe_printf(TRUE, "Pe Main: Unknown dialog 0x%X (%d)", phdr->id, (phdr->id & ~MVT_CORE_INTERNAL));
		break;
	}

	return reply;
}

// ****************************************************************************
// *			M E S S A G E S
// ****************************************************************************

//--- i_peCommMain_Ping -------------------------------------------------------
int i_peCommMain_Ping(STarget *target, RX_SOCKET socket, void *msg, int len)
{
	struct mvt_prt_ping_ans_v1 answer;
	
	if (target->type == EL_INTERF)
		return REPLY_OK;

	answer.hdr.id = MVT_PRT_ANS_PING;
	answer.hdr.length = sizeof(answer);
	answer.ver = PING_VER;

	pe_send_msg(&socket, &answer);
	return REPLY_OK;
}

//--- i_peCommMain_Config -----------------------------------------------------
int i_peCommMain_Config(STarget *target, RX_SOCKET socket, void *msg, int len)
{
	pe_printf(TR_RCVMSG, "i_peCommMain_Config");
	// Answer
	struct mvt_prt_config_ans_v1 *answer;
	int size = sizeof(struct mvt_prt_config_ans_v1) +
		(_pPeMainParameter.config->colorCount * sizeof(struct mvt_prt_color)) +
		(_pPeMainParameter.config->bitmapCount * sizeof(struct mvt_prt_bitmap)) +
		(_pPeMainParameter.config->interfCount * sizeof(struct mvt_prt_interface));

	answer = (struct mvt_prt_config_ans_v1*) calloc(1, size);
	answer->hdr.id = MVT_PRT_ANS_CONF;
	if (target->type == EL_INTERF)
		answer->hdr.id |= MVT_CORE_INTERNAL;

	answer->hdr.length = size;
	answer->ver = CONFIG_VER;
	strncpy(answer->id, _pPeMainParameter.config->id, sizeof(answer->id));

	answer->resoHead = _pPeMainParameter.config->resoHead;
	answer->resoPrint = _pPeMainParameter.config->resoPrint;
	answer->maxWidth = _pPeMainParameter.config->maxWidth;
	answer->maxHeight = _pPeMainParameter.config->maxHeight;
	answer->maxSpeed = _pPeMainParameter.config->maxSpeed;
	answer->duplex = _pPeMainParameter.config->duplex;
	answer->dropSizeCount = _pPeMainParameter.config->dropSizeCount;
	answer->bitPerPixel = _pPeMainParameter.config->bitPerPixel;
	answer->alignment = _pPeMainParameter.config->alignment;
	answer->speedPassCount = _pPeMainParameter.config->speedPassCount;

	answer->colorCount = _pPeMainParameter.config->colorCount;
	struct mvt_prt_color *color = (struct mvt_prt_color *) &answer[1];
	for (int i = 0; i < answer->colorCount; i++, color++) {
		color->index = _pPeMainParameter.config->colors[i].index;
		color->colorCode = _pPeMainParameter.config->colors[i].colorCode;
	}

	answer->bitmapCount = _pPeMainParameter.config->bitmapCount;
	struct mvt_prt_bitmap *bitmap = (struct mvt_prt_bitmap *) color;
	for (int i = 0; i < answer->bitmapCount; i++, bitmap++) {
		bitmap->index = _pPeMainParameter.config->bitmaps[i].index;
		bitmap->interf = _pPeMainParameter.config->bitmaps[i].interf;
		bitmap->color = _pPeMainParameter.config->bitmaps[i].color;
		bitmap->side = _pPeMainParameter.config->bitmaps[i].side;
		bitmap->offset = _pPeMainParameter.config->bitmaps[i].offset;
		bitmap->width = _pPeMainParameter.config->bitmaps[i].width;
		bitmap->speedPass = _pPeMainParameter.config->bitmaps[i].speedPass;
	}

	answer->interfCount = _pPeMainParameter.config->interfCount;
	struct mvt_prt_interface *interf = (struct mvt_prt_interface *) bitmap;
	for (int i = 0; i < answer->interfCount; i++, interf++) {
		interf->index = _pPeMainParameter.config->interfaces[i].index;
		strncpy(interf->ip, _pPeMainParameter.config->interfaces[i].ip, sizeof(interf->ip));
		interf->port = _pPeMainParameter.config->interfaces[i].port;
		interf->connected = _pPeMainParameter.config->interfaces[i].connected;
	}
	pe_send_msg(&socket, answer);

	if (target->type == EL_RIPMAIN) {
		struct mvt_prt_config_ans_v1 *buffer = malloc(size);
		memcpy(buffer, answer, size);
		buffer->hdr.id |= MVT_CORE_INTERNAL;
		rx_fifo_put(_hFifoMsgForward, buffer, size);
	}
	free(answer);

#ifdef _SIMULATOR
	if (_pPeMainParameter.callbackUpdateConfig)
		_pPeMainParameter.callbackUpdateConfig(_pPeMainParameter.config);
#endif

	return REPLY_OK;
}

//--- i_peCommMain_Open -------------------------------------------------------
int i_peCommMain_Open(STarget *target, RX_SOCKET socket, void *msg, int len)
{
	struct mvt_prt_session_req_v1 *request = (struct mvt_prt_session_req_v1*) msg;
	struct mvt_prt_session_ans_v1 answer;
	
	pe_printf(TR_RCVMSG, "i_peCommMain_Open");

	target->currentSession = request->id;
	
	answer.hdr.id = MVT_PRT_ANS_OPEN;
	answer.hdr.length = sizeof(answer);
	answer.ver = OPEN_VER;
	answer.id = (_statePeCommMain == PE_CONNECTED) ? target->currentSession :0;
	pe_send_msg(&socket, &answer);
	
	target->pageIdx = 1;

	return REPLY_OK;
}

//--- i_peCommMain_Close ------------------------------------------------------
int i_peCommMain_Close(STarget *target, RX_SOCKET socket, void *msg, int len)
{
	struct mvt_prt_close_ans_v1 answer;

	pe_printf(TR_RCVMSG, "i_peCommMain_Close");

	answer.hdr.id = MVT_PRT_ANS_CLOSE;
	answer.hdr.length = sizeof(answer);
	answer.ver = CLOSE_VER;
	pe_send_msg(&socket, &answer);
	target->currentSession = 0;
	i_peClearPageList();
	return REPLY_OK;
}

//--- i_peCommMain_StartCmd ---------------------------------------------------
int i_peCommMain_StartCmd(STarget *target, RX_SOCKET socket, void *msg, int len)
{
	struct mvt_prt_start_ans_v1 answer;

	pe_printf(TR_RCVMSG, "i_peCommMain_StartCmd");

	answer.hdr.id = MVT_PRT_ANS_START;
	answer.hdr.length = sizeof(answer);
	answer.ver = PRINTSTART_VER;
	answer.cmd_status = (_pPeMainParameter.callbackPrintStart && (_statePeCommMain == PE_CONNECTED)) ? 1 : 0;

	if (answer.cmd_status)
		answer.cmd_status = _pPeMainParameter.callbackPrintStart() ? 0:1;

	pe_send_msg(&socket, &answer);

	return REPLY_OK;
}

//--- i_peCommMain_StopCmd ----------------------------------------------------
int i_peCommMain_StopCmd(STarget *target, RX_SOCKET socket, void *msg, int len)
{
	struct mvt_prt_stop_req_v1 *request = (struct mvt_prt_stop_req_v1*) msg;
	struct mvt_prt_stop_ans_v1 answer;

	pe_printf(TR_RCVMSG, "i_peCommMain_StopCmd");

	answer.hdr.id = MVT_PRT_ANS_STOP;
	answer.hdr.length = sizeof(answer);
	answer.ver = PRINTSTOP_VER;
	answer.cmd_status = (_pPeMainParameter.callbackPrintStop && (_statePeCommMain == PE_CONNECTED)) ? 1 : 0;

	if (answer.cmd_status) {
		answer.cmd_status = _pPeMainParameter.callbackPrintStop(request->page_idx) ? 0 : 1;
		if (!request->page_idx)
			i_peClearPageList();
	}
	pe_send_msg(&socket, &answer);

	return REPLY_OK;
}

//--- i_peCommMain_Command ----------------------------------------------------
int i_peCommMain_Command(STarget *target, RX_SOCKET socket, void *msg, int len)
{
	struct mvt_prt_cmd_req_v1 *request = (struct mvt_prt_cmd_req_v1*) msg;
	struct mvt_prt_cmd_ans_v1 answer;

	pe_printf(TR_RCVMSG, "i_peCommMain_Command");

	answer.hdr.id = MVT_PRT_ANS_CMD;
	answer.hdr.length = sizeof(answer);
	answer.ver = PRINTCMD_VER;
	answer.cmd_status = (_pPeMainParameter.callbackPrintCmd && (_statePeCommMain == PE_CONNECTED)) ? 1 : 0;

	if (answer.cmd_status)
		answer.cmd_status = _pPeMainParameter.callbackPrintCmd(request) ? 0 : 1;

	pe_send_msg(&socket, &answer);

	return REPLY_OK;
}

//--- i_peCommMain_Synchro ----------------------------------------------------
int i_peCommMain_Synchro(STarget *target, RX_SOCKET socket, void *msg, int len)
{
	struct mvt_prt_jobsync_req_v1 *request = (struct mvt_prt_jobsync_req_v1*) msg;
	struct mvt_prt_jobsync_ans_v1 answer;

	pe_printf(TR_RCVMSG, "i_peCommMain_Synchro");

	answer.hdr.id = MVT_PRT_ANS_SYNC;
	answer.hdr.length = sizeof(answer);
	answer.ver = PRINTSYNC_VER;
	answer.cmd_status = (_pPeMainParameter.callbackPrintSynchro && (_statePeCommMain == PE_CONNECTED)) ? 1 : 0;

	if (answer.cmd_status)
		answer.cmd_status = _pPeMainParameter.callbackPrintSynchro(request) ? 0 : 1;

	pe_send_msg(&socket, &answer);

	return REPLY_OK;
}

//--- i_peCommMain_Adjust -----------------------------------------------------
int i_peCommMain_Adjust(STarget *target, RX_SOCKET socket, void *msg, int len)
{
	struct mvt_prt_adjust_req_v1 *request = (struct mvt_prt_adjust_req_v1*) msg;
	struct mvt_prt_adjust_ans_v1 answer;

	pe_printf(TR_RCVMSG, "i_peCommMain_Adjust");

	answer.hdr.id = MVT_PRT_ANS_ADJ;
	answer.hdr.length = sizeof(answer);
	answer.ver = PRINTADJ_VER;
	answer.cmd_status = (_pPeMainParameter.callbackPrintAdjust && (_statePeCommMain == PE_CONNECTED)) ? 1 : 0;

	if (answer.cmd_status)
		answer.cmd_status = _pPeMainParameter.callbackPrintAdjust(request) ? 0 : 1;

	pe_send_msg(&socket, &answer);

	return REPLY_OK;
}

//--- i_peCommMain_Page -------------------------------------------------------
int i_peCommMain_Page(STarget *target, RX_SOCKET socket, void *msg, int len)
{
	struct mvt_prt_loadpagecmd_req_v1 *request = (struct mvt_prt_loadpagecmd_req_v1*) msg;
	struct mvt_prt_loadpagecmd_ans_v1 answer;
	SPage *page;

	pe_printf(TR_RCVMSG, "i_peCommMain_Page");

	answer.hdr.id = MVT_PRT_ANS_CMDPG;
	answer.hdr.length = sizeof(answer);
	answer.ver = CMDPG_VER;
	answer.page_idx = (_statePeCommMain == PE_CONNECTED) ? target->pageIdx : 0;
	answer.copyCount = request->copyCount;

	if (answer.page_idx) {
		if (target->pageIdx == 1) i_peClearPageState();
		page = (SPage*)calloc(1, sizeof(SPage));
		page->job_id = request->job_id;
		page->page_id = request->page_id;
		page->copyCount = request->copyCount;
		page->bitmapCount = request->bitmapCount;
		page->pageIdx = target->pageIdx;
		page->copyCurrent = 0;
		page->itfCounter = (SItfCounter*)calloc(_pPeMainParameter.config->interfCount, sizeof(SItfCounter));
		page->jobType = EJ_PAGE;

		rx_mutex_lock(_hMutexPage);
		rx_list_add(_hListPage, page);
		rx_mutex_unlock(_hMutexPage);
		target->pageIdx += request->copyCount;

		if (_pPeMainParameter.callbackPageReady) _pPeMainParameter.callbackPageReady(page->pageIdx, page->copyCount);

		pe_printf(TR_RCVPAGE, "Pe Main: Cmd Page %d (job_id %d - page_id %d - Copy count - %d)", page->pageIdx, page->job_id, page->page_id, page->copyCount);
	}

	pe_send_msg(&socket, &answer);

	return REPLY_OK;
}

//--- i_peCommMain_State ------------------------------------------------------
int i_peCommMain_State(STarget *target, RX_SOCKET socket, void *msg, int len)
{
	struct mvt_prt_state_req_v1 *request = (struct mvt_prt_state_req_v1*) msg;

	if (msg && (request->watchTime>=0))
		target->watchTime = request->watchTime;

	if (target->type == EL_RIPMAIN) {
		_printStateAnswer->watchtime = target->watchTime;
		_printStateAnswer->session_id = target->currentSession;				// Check if all targets have the same Session ?
		i_peUpdatePrinterState();
	}
	pe_send_msg(&socket, _printStateAnswer);

	if (msg && (target->type == EL_RIPMAIN) && target->watchTime && !_threadPeMainState) {
		_threadPeMainState = rx_thread_start(i_peMainStateThread, NULL, 0, "_peMainState_thread");
	}

#ifdef _SIMULATOR
	if (_pPeMainParameter.callbackEngineState)
		_pPeMainParameter.callbackEngineState(_printStateAnswer);
#endif

	return REPLY_OK;
}

//--- i_peCommMain_FileCmd ----------------------------------------------------
int i_peCommMain_FileCmd(STarget *target, RX_SOCKET socket, void *msg, int len)
{
	struct mvt_prt_loadfilecmd_req_v1 *request = (struct mvt_prt_loadfilecmd_req_v1*) msg;
	struct mvt_prt_loadfilecmd_ans_v1 answer;

	pe_printf(TR_RCVMSG, "i_peCommMain_FileCmd");

	answer.hdr.id = MVT_PRT_ANS_FILECMD;
	answer.hdr.length = sizeof(answer);
	answer.ver = LOADFILE_VER;
	answer.page_idx = (_pPeMainParameter.callbackLoadFile && (_statePeCommMain == PE_CONNECTED)) ? target->pageIdx : 0;
	answer.copyCount = (1 + request->pageEnd - request->pageStart) * request->copyCount;

	if (answer.page_idx && _pPeMainParameter.callbackLoadFile(request, target->pageIdx))
		answer.page_idx = 0;

	if (answer.page_idx) {
		int nIx = target->pageIdx;
		int cpyInc = (request->pageEnd - request->pageStart) ? 1 : request->copyCount;

		if (target->pageIdx == 1) i_peClearPageState();

		for (int nCp = 0; nCp < request->copyCount; nCp += cpyInc) {
			for (int nId = request->pageStart; nId <= request->pageEnd; nId++) {
				SPage *page;
				page = (SPage*)calloc(1, sizeof(SPage));
				page->job_id = request->job_id;
				page->page_id = nId;
				page->copyCount = cpyInc;
				page->bitmapCount = _pPeMainParameter.config->bitmapCount;
				page->pageIdx = nIx;
				page->copyCurrent = nCp;
				page->itfCounter = (SItfCounter*)calloc(_pPeMainParameter.config->interfCount, sizeof(SItfCounter));
				page->jobType = EJ_FILE;
				page->info = strdup(request->filepath);

				rx_mutex_lock(_hMutexPage);
				rx_list_add(_hListPage, page);
				rx_mutex_unlock(_hMutexPage);
				nIx += cpyInc;
			}
		}
		target->pageIdx = nIx;
	}
	pe_send_msg(&socket, &answer);

	return REPLY_OK;
}

//--- i_peCommMain_DrawCmd ----------------------------------------------------
int i_peCommMain_DrawCmd(STarget *target, RX_SOCKET socket, void *msg, int len)
{
	struct mvt_prt_drawcmd_req_v1 *request = (struct mvt_prt_drawcmd_req_v1*) msg;
	struct mvt_prt_drawcmd_ans_v1 answer;

	pe_printf(TR_RCVMSG, "i_peCommMain_DrawCmd");

	answer.hdr.id = MVT_PRT_ANS_DRAWCMD;
	answer.hdr.length = sizeof(answer);
	answer.ver = DRAWCMD_VER;
	answer.page_idx = (_pPeMainParameter.callbackDrawCmd && (_statePeCommMain == PE_CONNECTED)) ? target->pageIdx : 0;

	if (answer.page_idx && _pPeMainParameter.callbackDrawCmd(request, target->pageIdx))
		answer.page_idx = 0;

	if (answer.page_idx) {
		SPage *page;

		if (target->pageIdx == 1) i_peClearPageState();

		page = (SPage*)calloc(1, sizeof(SPage));
		page->job_id = request->job_id;
		page->page_id = request->page_id;
		page->copyCount = request->copyCount;
		page->bitmapCount = _pPeMainParameter.config->bitmapCount;
		page->pageIdx = target->pageIdx;
		page->copyCurrent = 0;
		page->itfCounter = (SItfCounter*)calloc(_pPeMainParameter.config->interfCount, sizeof(SItfCounter));
		page->jobType = EJ_DRAW;
		page->info = strdup(request->filepath);

		rx_mutex_lock(_hMutexPage);
		rx_list_add(_hListPage, page);
		rx_mutex_unlock(_hMutexPage);
		
		target->pageIdx += request->copyCount;
	}

	pe_send_msg(&socket, &answer);

	return REPLY_OK;
}

//--- i_peCommMain_LayoutCmd --------------------------------------------------
int i_peCommMain_LayoutCmd(STarget *target, RX_SOCKET socket, void *msg, int len)
{
	struct mvt_prt_loadlayoutcmd_req_v1 *request = (struct mvt_prt_loadlayoutcmd_req_v1*) msg;
	struct mvt_prt_loadlayoutcmd_ans_v1 answer;

	pe_printf(TR_RCVMSG, "i_peCommMain_LayoutCmd");

	answer.hdr.id = MVT_PRT_ANS_LAYOUTCMD;
	answer.hdr.length = sizeof(answer);
	answer.ver = LAYOUTCMD_VER;
	answer.page_idx = (_pPeMainParameter.callbackLoadlayout && (_statePeCommMain == PE_CONNECTED)) ? target->pageIdx : 0;
	answer.copyCount = (1 + request->recordEnd - request->recordStart) * request->copyCount;

	if (answer.page_idx && _pPeMainParameter.callbackLoadlayout(request, target->pageIdx))
		answer.page_idx = 0;

	if (answer.page_idx) {
		int nIx = target->pageIdx;
		int cpyInc = (request->recordEnd - request->recordStart) ? 1 : request->copyCount;
		target->pageRecordIdx = target->pageIdx;

		if (target->pageIdx == 1) i_peClearPageState();

		for (int nCp = 0; nCp < request->copyCount; nCp += cpyInc) {
			for (int nId = request->recordStart; nId <= request->recordEnd; nId++) {
				SPage *page;
				page = (SPage*)calloc(1, sizeof(SPage));
				page->job_id = request->job_id;
				page->page_id = nId;
				page->copyCount = cpyInc;
				page->bitmapCount = _pPeMainParameter.config->bitmapCount;
				page->pageIdx = nIx;
				page->copyCurrent = nCp;
				page->itfCounter = (SItfCounter*)calloc(_pPeMainParameter.config->interfCount, sizeof(SItfCounter));
				page->jobType = EJ_LAYOUT;
				page->info = strdup(request->filepath);

				rx_mutex_lock(_hMutexPage);
				rx_list_add(_hListPage, page);
				rx_mutex_unlock(_hMutexPage);
				nIx += cpyInc;
			}
		}
		target->pageIdx = nIx;
	}

	pe_send_msg(&socket, &answer);

	return REPLY_OK;
}

//--- i_peCommMain_RecordData -------------------------------------------------
int i_peCommMain_RecordData(STarget *target, RX_SOCKET socket, void *msg, int len)
{
	struct mvt_prt_loadrecorddata_req_v1 *request = (struct mvt_prt_loadrecorddata_req_v1*) msg;
	struct mvt_prt_loadrecorddata_ans_v1 answer;

	pe_printf(TR_RCVMSG, "i_peCommMain_RecordData");

	answer.hdr.id = MVT_PRT_ANS_RECORDDATA;
	answer.hdr.length = sizeof(answer);
	answer.ver = RECORDDATA_VER;
	answer.page_idx = (_pPeMainParameter.callbackRecordData && (_statePeCommMain == PE_CONNECTED)) ? target->pageRecordIdx : 0;

	if (answer.page_idx = 0) {
		if (_pPeMainParameter.callbackRecordData(request, target->pageRecordIdx))
			answer.page_idx = 0;
		target->pageRecordIdx += request->copyCount;
	}

	pe_send_msg(&socket, &answer);

	return REPLY_OK;
}

// ****************************************************************************
// *			M E S S A G E S			I N T E R F A C E S
// ****************************************************************************

//--- i_peCommCore_InterfConfig -----------------------------------------------
int i_peCommCore_InterfConfig(STarget *target, RX_SOCKET socket, void *msg, int len)
{
	struct mvt_core_conf_req_v1 *request = (struct mvt_core_conf_req_v1*) msg;
	struct mvt_core_conf_ans_v1 answer;

	pe_printf(TR_RCVMSG, "i_peCommCore_InterfConfig");

	answer.hdr.id = MVT_CORE_ANS_CONF | MVT_CORE_INTERNAL;
	answer.hdr.length = sizeof(answer);
	answer.ver = CORE_CONFIG_VER;
	answer.cmd_status = ((target->type != EL_INTERF) || (request->interfNum >= _pPeMainParameter.config->interfCount)) ? 0 : 1;
	if (answer.cmd_status) {
		STarget *t = NULL;
		rx_mutex_lock(_hMutexTarget);
		rx_list_begin(_hListTarget, (void**)&t);
		while (t) {
			if ((t->type == EL_INTERF) && (t->interfNumber == request->interfNum) && (t->connect == EC_INITIALIZED))
				answer.cmd_status = 0;		// Already connected -> error
			rx_list_next(_hListTarget, (void**)&t);
		}
		rx_mutex_unlock(_hMutexTarget);
	}
	pe_send_msg(&socket, &answer);
	
	if (answer.cmd_status) {
		rx_mutex_lock(_hMutexTarget);
		target->connect = EC_INITIALIZED;
		target->interfNumber = request->interfNum;
		strncpy(target->ip, request->ip, sizeof(target->ip));
		target->port = request->port;
		rx_mutex_unlock(_hMutexTarget);

		strncpy(_pPeMainParameter.config->interfaces[target->interfNumber].ip, request->ip, sizeof(_pPeMainParameter.config->interfaces[target->interfNumber].ip));
		_pPeMainParameter.config->interfaces[target->interfNumber].port = request->port;
		_pPeMainParameter.config->interfaces[target->interfNumber].connected = 1;

#ifdef _SIMULATOR
		i_peUpdatePrinterState();
		if (_pPeMainParameter.callbackEngineState)
			_pPeMainParameter.callbackEngineState(_printStateAnswer);
#endif

		return i_peCommMain_Config(target, socket, NULL, 0);
	}

	sok_close(&socket);			// Bad interface, close connection
	return REPLY_OK;
}

//--- i_peCommCore_InterfState ------------------------------------------------
int i_peCommCore_InterfState(STarget *target, RX_SOCKET socket, void *msg, int len)
{
	struct mvt_core_state_req_v1 *request = (struct mvt_core_state_req_v1*) msg;
	struct mvt_core_state_ans_v1 answer;

	if (msg && (request->watchtime >= 0))
		target->watchTime = request->watchtime;

	answer.hdr.id = MVT_CORE_ANS_STATE | MVT_CORE_INTERNAL;
	answer.hdr.length = sizeof(answer);
	answer.ver = CORE_STATE_VER;
	answer.watchTime = target->watchTime;
	pe_send_msg(&socket, &answer);

	if (msg) {
		pe_printf(TR_PAGESTATE_INTERF, "IFSTATE[%d]: session=%d, rips=%d, sessions=%d", target->interfNumber, request->session_id, request->connectedRipCount, request->sessionCount);
		target->currentSession = request->session_id;
		target->ripCount = request->connectedRipCount;
		target->sessionCount = request->sessionCount;
		i_pePageUpdate(target->interfNumber, &request->rcv_pg, EP_RECEIVED);
		i_pePageUpdate(target->interfNumber, &request->sch_pg, EP_SCHEDULED);
		i_pePageUpdate(target->interfNumber, &request->cmt_pg, EP_COMMITTED);
		i_pePageUpdate(target->interfNumber, &request->prt_pg, EP_PRINTED);
	}
	return REPLY_OK;
}

//--- i_pePageUpdate ----------------------------------------------------------
void i_pePageUpdate(int interf, Smvt_prt_page *pge, EPage state)
{
	SPage *page = NULL;
	char  *stateStr[] = { "NOTHING",
		"RECEIVING",
		"RECEIVED ",
		"SCHEDULED",
		"COMMITTED",
		"PRINTING ",
		"PRINTED  ",
		"STACKED  "
	};

	if (!pge->idx || !pge->copy || !pge->page_id || !pge->job_id)
		return;

	pe_printf(TR_PAGESTATE_INTERF, "IFSTATE[%d]: %s (idx=%d, job=%d, page=%d, cpy=%d)", interf, stateStr[state], pge->idx, pge->job_id, pge->page_id, pge->copy);

	rx_mutex_lock(_hMutexPage);
	rx_list_begin(_hListPage, (void**)&page);
	while (page) {
		if ((pge->idx < (page->pageIdx + page->copyCount)) && (pge->idx >= page->pageIdx))
			break;
		rx_list_next(_hListPage, (void**)&page);
	}
	if (page) {
		SCounter *p = NULL;
		switch (state) {
		case EP_RECEIVED:	p = &page->itfCounter[interf].received; break;
		case EP_SCHEDULED:	p = &page->itfCounter[interf].scheduled; break;
		case EP_COMMITTED:	p = &page->itfCounter[interf].committed; break;
		case EP_PRINTED:	p = &page->itfCounter[interf].printed; break;
		}
		if (p) {
			p->pageNum = pge->idx;
			p->copyNum = pge->copy;
		}
	}
	rx_mutex_unlock(_hMutexPage);
}

// ****************************************************************************
