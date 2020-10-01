// ****************************************************************************
//
//	print_queue.c	
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

//--- includes ----------------------------------------------------------------

#include "tcp_ip.h"
#include "rx_error.h"
#include "rx_file.h"
#include "rx_trace.h"
#include "gui_svr.h"
#include "rx_setup_file.h"
#include "args.h"
#include "print_ctrl.h"
#include "machine_ctrl.h"
#include "plc_ctrl.h"
#include "ctrl_svr.h"
#include "es_rip.h"
#include "enc_ctrl.h"
#include "ctr.h"
#include "spool_svr.h"
#include "print_queue.h"

//--- statics -----------------------------------------------------------------

#define LIST_SIZE		100
#define SPOOL_CNT		10

static SPrintQueueItem	_List[LIST_SIZE];
static SPrintQueueItem	_PrintedItem;

static UINT32			_Loading[LIST_SIZE];
static char				_ListText[LIST_SIZE][SPOOL_CNT][32];
static int				_Size=0;
static INT32			_ID=0;
static int				_Item;
static int				_TestDataSent;
static int				_TimeCompleted;
static int				_Speed=0;
static int				_BufState;
static int				_PrintDoneCnt;
static int				_ActiveState;
static char				_ActiveRipState[MAX_PATH];

//--- prototypes -------------------------------------------------------------
static int _find_item(int id, int *idx);
static char* _filename(char *path);
static void _trace_queue(void);

//--- pq_init ----------------------------------------------------------------
int	pq_init(void)
{
	memset(_List, 0, sizeof(_List));
	_Item = 0;
	_TimeCompleted    = 0;
	_BufState		  = 0;
	
	return REPLY_OK;
}

//--- pq_end ------------------------------------------------------------------
int pq_end(void)
{
	return REPLY_OK;
}

//--- setup_item -------------------------------------------------------------
static int setup_item(HANDLE file, int idx, SPrintQueueItem *item, EN_setup_Action action)
{
	if (setup_chapter(file, "item", idx,	action)==REPLY_OK)
	{
		setup_uint32	(file, "id",			action, &item->id.id, 0);
		setup_str	    (file, "filepath",		action, item->filepath, sizeof(item->filepath), "");
//		setup_str	    (file, "preview",		action, item->preview,  sizeof(item->preview), "");
		setup_str	    (file, "printEnv",		action, item->printEnv, sizeof(item->printEnv), "");
		setup_str	    (file, "material",		action, item->material, sizeof(item->material), "");
		setup_uint32	(file, "srcPages",		action, &item->srcPages,		1);
		setup_uint32	(file, "srcWidth",		action, &item->srcWidth,		0);
		setup_uint32	(file, "srcHeight",		action, &item->srcHeight,		0);
		setup_uint32	(file, "firstPage",		action, &item->firstPage,		1);
		setup_uint32	(file, "lastPage",		action, &item->lastPage,		1);
		setup_uchar		(file, "singlePage",	action, &item->singlePage,		0);
		setup_uint32	(file, "startPage",		action, &item->start.page,		item->firstPage);
		setup_uint32	(file, "startCopy",		action, &item->start.copy,		1);
		setup_uint32	(file, "startScan",		action, &item->start.scan,		0);
		setup_uint32	(file, "copies",		action, &item->copies,			1);
		setup_uchar		(file, "collate",		action, &item->collate,			1);
		setup_uchar		(file, "variable",		action, &item->variable,		0);
		setup_uchar 	(file, "wakeup",		action, &item->wakeup,			0);
		setup_uchar		(file, "lengthUnit",	action, &item->lengthUnit,		PQ_LENGTH_MM);
		setup_uchar 	(file, "orientation",	action, &item->orientation,		0);
		setup_uint32	(file, "pageWidth",		action, &item->pageWidth,		0);
		setup_uint32	(file, "pageHeight",	action, &item->pageHeight,		0);
		setup_int32		(file, "pageMargin",	action, &item->pageMargin,		0);
		setup_uint32	(file, "printGoMode",	action, &item->printGoMode,		0);
		setup_uint32	(file, "printGoDist",	action, &item->printGoDist,		0);
		setup_uint32	(file, "scanLength",	action, &item->scanLength,		100);
		setup_uchar		(file, "passes",		action, &item->passes,			1);
		setup_uchar		(file, "virtualPasses",	action, &item->virtualPasses,	0);
		setup_uchar		(file, "penetrationPasses",	action, &item->penetrationPasses,	0);
		setup_uchar		(file, "curingPasses",	action, &item->curingPasses,	1);
		setup_uchar 	(file, "bidirectional",	action, &item->scanMode,		0);
		setup_uint32	(file, "speed",			action, &item->speed,			4);
		setup_uint32	(file, "checks",		action, &item->checks,			0);
		setup_str	    (file, "Dots",		    action, item->dots, sizeof(item->dots), "");

//		setup_uint32	(file, "actPage",		action, &item->actPage,			1);
//		setup_uint32	(file, "actCopy",		action, &item->actCopy,			1);
//		setup_uint32	(file, "actScan",		action, &item->id.scan,			1);
//		setup_uint32	(file, "copiesTotal",	action, &item->copiesTotal,		1);
		setup_uint32	(file, "copiesPrinted",	action, &item->copiesPrinted,	0);
		setup_uint32	(file, "scansPrinted",	action, &item->scansPrinted,	0);
		setup_uint32	(file, "scansStart",	action, &item->scansStart,		0);

		item->id.copy = 0;		
		item->copiesTotal   = item->copies*(item->lastPage-item->firstPage+1);

		if (setup_chapter(file, "PageNumber", -1,	action)==REPLY_OK)
		{
			setup_uint32	(file, "enabled",		action, &item->pageNumber.enabled,		TRUE);
			setup_uint32	(file, "number",		action, &item->pageNumber.number,		1);
			setup_uint32	(file, "fontSize",		action, &item->pageNumber.fontSize,		20);
			setup_uint32	(file, "imgDist",		action, &item->pageNumber.imgDist,		2000);

			setup_chapter	(file, "..", idx,		action);
		}

		setup_chapter	(file, "..", idx,		action);

//		char datapath[MAX_PATH];
//		esrip_file_ripped(item, datapath, sizeof(datapath));
		
		return REPLY_OK;
	}
	else return REPLY_ERROR;
}

//--- pq_trace_item -------------------------------------------------------------
void pq_trace_item(SPrintQueueItem *pitem)
{
	HANDLE file = setup_create();
	setup_item(file, 1, pitem, WRITE);
	char* str=setup_to_str(file);
	TrPrintf(TRUE, str);
	free(str);
}

//--- pq_load ------------------------------------------------------------------
int pq_load(const char *filepath)
{
	HANDLE file = setup_create();

	setup_load(file, filepath);
	setup_chapter(file, "PrintQueue", -1, READ);
	_ID = 0;
	for (_Size = 0; _Size < SIZEOF(_List); _Size++)
	{
		if (setup_item(file, _Size, &_List[_Size], READ)==REPLY_ERROR) break;
		/*
		if (!rx_file_exists(_List[_Size].preview))
		{
			_List[_Size].preview[0] = 0;
		}
		*/
		_List[_Size].state = PQ_STATE_QUEUED;
		if (_List[_Size].id.id > _ID) _ID=_List[_Size].id.id;
	}
	setup_destroy(file);
	return REPLY_OK;
}

//--- pq_save ------------------------------------------------------------------
int pq_save(const char *filepath)
{
	int i;
	HANDLE file=setup_create();

	setup_chapter(file, "PrintQueue", -1, WRITE);
	for (i = 0; i < _Size; i++)
	{
		if (_List[i].state!=PQ_STATE_PRINTED)
			setup_item(file, i, &_List[i], WRITE);
	}
	setup_save(file, filepath);
	setup_destroy(file);
	return REPLY_OK;
}

//--- pq_start ----------------------------------------------------------------
int	pq_start(void)
{
	_Item=0;
	_TestDataSent	  = 0;
	_TimeCompleted	  = 0;
	_BufState		  = 0;
	_PrintDoneCnt	  = 0;	
	_ActiveState	  = 0;
	memset(_ActiveRipState, 0, sizeof(_ActiveRipState));
	memset(&_PrintedItem, 0, sizeof(_PrintedItem));

	return REPLY_OK;
}

//--- _send_PrintedItem -----------------------------------
static void _send_PrintedItem(void)
{			
	if (*_PrintedItem.filepath)
	{
		gui_send_printer_status(&RX_PrinterStatus);
		gui_send_print_queue(EVT_GET_PRINT_QUEUE, &_PrintedItem);
		memset(&_PrintedItem, 0, sizeof(_PrintedItem));		
	}
}

//--- pq_tick ---------------------------------------------
void pq_tick(void)
{
	_send_PrintedItem();

	if (_TimeCompleted && _TimeCompleted>rx_get_ticks())
	{
		Error(WARN, 0, "_TimeCompleted: Job stopped by timer");
		machine_stop_printing();
		_TimeCompleted = 0;
	}
}

//--- pq_stop ---------------------------------------------------------------
int pq_stop(void)
{
	int i;
	int done=FALSE;

//	TrPrintfL(TRUE, "pq_stop");
//	_trace_queue();
	_TimeCompleted = 0;
	_send_PrintedItem();
	
//	ctr_save();

	/*
	if (RX_Config.printer.type==printer_LH702)
	{	
		gui_send_cmd(REP_GET_PRINT_QUEUE);
		for (i=0; i<_Size; i++)
		{
			if (_List[i].state == PQ_STATE_QUEUED)
			{
				if (i) memcpy(&_List[0], &_List[i], sizeof(_List[0]));
				_Size=1;
				gui_send_print_queue(EVT_ADD_PRINT_QUEUE, &_List[0]);
				return REPLY_OK;
			}
		}
		
		for (i=0; i<_Size; i++)
		{
			if (_List[i].state != PQ_STATE_QUEUED && _List[i].state<PQ_STATE_PRINTED)
			{
				if (i) memcpy(&_List[0], &_List[i], sizeof(_List[0]));
				_List[0].state=PQ_STATE_QUEUED;
				_Size=1;					
				gui_send_print_queue(EVT_ADD_PRINT_QUEUE, &_List[0]);
				return REPLY_OK;
			}
		}
		if (_Size>1) memcpy(&_List[0], &_List[_Size-1], sizeof(_List[0]));
		_List[0].state=PQ_STATE_QUEUED;		
		_Size=1;
		gui_send_print_queue(EVT_ADD_PRINT_QUEUE, &_List[0]);
		
		return REPLY_OK;	
	}
	*/

	for (i=_Size-1; i>=0;  i--)
	{
		if (_List[i].state==PQ_STATE_STOPPING) 
		{
			_List[i].state = PQ_STATE_STOPPED;
			gui_send_print_queue(EVT_GET_PRINT_QUEUE, &_List[i]);
		}
		if (_List[i].state!=PQ_STATE_QUEUED && _List[i].state!=PQ_STATE_PRINTED)
		{
			_List[i].state = PQ_STATE_QUEUED;
			gui_send_print_queue(EVT_ADD_PRINT_QUEUE, &_List[i]);
		}
	}

//	TrPrintfL(TRUE, "pq_stop END");
//	_trace_queue();
	pq_save(PATH_USER FILENAME_PQ);

	return REPLY_ERROR;	
}

//--- pq_abort ----------------------------------------
int pq_abort(void)
{
	TrPrintfL(TRUE, "pq_abort");
	int i;
		
	for (i=_Size-1; i>=0;  i--)
	{
		if ((_List[i].state>=PQ_STATE_PRINTING) && (_List[i].state<=PQ_STATE_STOPPED))
		{
			if(rx_def_is_scanning(RX_Config.printer.type) && _List[i].scans && _List[i].id.scan<_List[i].scans)
			{				
				double s = (double)_List[i].scanLength/1000.0 * (double)(_List[i].scansPrinted-_List[i].scansStart) / (double)(_List[i].scans-_List[i].scansStart);
				_List[i].start.scan = (INT32)s;
			}
		}
	}
	pq_stop();
	return REPLY_OK;
}

//--- _find_item -----------------------------------------------------
static int _find_item(int id, int *pidx)
{
	int i;
	if (!rx_def_use_pq(RX_Config.printer.type))
	{
		*pidx=0;
		return REPLY_OK;
	}
	for (i=0; i<_Size;  i++)
	{
		if (_List[i].id.id == id)
		{
			*pidx=i;
			return REPLY_OK;
		}
	}
	*pidx=0;
	return REPLY_ERROR;
}

//--- pq_get_item_n -------------------------------------------------------------
SPrintQueueItem *pq_get_item_n(int i)
{
	if (i<_Size) return &_List[i];
	return NULL;
}

//--- pq_get_next_item ---------------------------------------------------------
SPrintQueueItem *pq_get_next_item(void)
{	
	int slide=!rx_def_use_pq(RX_Config.printer.type);
	if (slide && _Item>0) return NULL;

	for(int item=0; item<_Size; item++)
	{
//		if (_List[item].state==PQ_STATE_QUEUED && _List[item].scanLength>0)
		if ((slide || _List[item].state==PQ_STATE_QUEUED) && (_List[item].copies>0 || _List[item].scanLength>0))
		{
			if (slide) _List[item].start.copy=0;
			_Item = item+1;
			_ActiveState = _List[item].state;
			return &_List[item];
		}
	}		
	return NULL;
}

/*
//--- _load_variable_info ------------------------------------------------------
static void _load_variable_info(SPrintQueueItem *pitem)
{
	Error(ERR_CONT, 0, "To BE IMPLEMENTED");				
}
*/

//--- pq_add_item --------------------------------------------------------------
SPrintQueueItem *pq_add_item(SPrintQueueItem *pitem)
{	
	SPrintQueueItem *ret;
	
	//--- remove latest printed item ---
	if (_Size>=SIZEOF(_List))
	{
		if (_List[0].state > PQ_STATE_PRINTING)
		{
			memcpy(&_List[0], &_List[1], (SIZEOF(_List)-1)*sizeof(SPrintQueueItem));
			_Size--;			
		}
	}
	
	if (_Size<SIZEOF(_List))
	{
//		if (pitem->variable) _load_variable_info(pitem);
		if (pitem->firstPage<1) pitem->firstPage=1;
		if (pitem->lastPage<pitem->firstPage) pitem->lastPage=pitem->firstPage;
		if (!rx_def_use_pq(RX_Config.printer.type) && pitem->copies<1) pitem->copies=1;
		pitem->id.id = ++_ID;
		pitem->state = PQ_STATE_QUEUED;
		TrPrintf(TRUE, "pq_add_item id=%d, >>%s<< pages: %d..%d copies=%d", pitem->id, pitem->filepath, pitem->firstPage, pitem->lastPage, pitem->copies);
		ret = &_List[_Size];
		memcpy(&_List[_Size], pitem, sizeof(_List[_Size]));
		_Size++;
//		char datapath[MAX_PATH];
//		esrip_file_ripped(ret, datapath, sizeof(datapath));
		pc_print_next();
		return ret;
	}
	else return NULL;
}

//--- pq_get_item --------------------------------------------------------------
SPrintQueueItem *pq_get_item(SPrintQueueItem *pitem)
{
	int i;
	if (_find_item(pitem->id.id, &i)==REPLY_OK)
		return &_List[i];
	else
		return NULL;
}

//--- pq_set_item --------------------------------------------------------------
SPrintQueueItem *pq_set_item(SPrintQueueItem *pitem)
{
	int i;
	if (_find_item(pitem->id.id, &i)==REPLY_OK)
	{
		pitem->state = _List[i].state;
//		pitem->scansPrinted = _List[i].scansPrinted;
		memcpy(&_List[i], pitem, sizeof(_List[i]));		
		if (_List[i].state<=PQ_STATE_RIPPING)
		{
			memset(&_ListText[i], 0, sizeof(_ListText[i]));
		}
		return pitem;
	}
	return NULL;
}

//--- pq_del_item -------------------------------------------------
int pq_del_item(SPrintQueueItem *pitem)
{
	int i;
	if (_find_item(pitem->id.id, &i)==REPLY_OK)
	{
		_Size--;
		memmove(&_List[i], &_List[i+1], (_Size-i)*sizeof(_List[0]));		
		return REPLY_OK;
	}
	return REPLY_ERROR;
}

//--- _trace_queue ----------------------------
static void _trace_queue(void)
{
	int i;
		
	for (i=0; i<_Size; i++)
	{
		TrPrintfL(TRUE, "PQ[%d].id=%d", i, _List[i].id.id);			
	}
}

//--- pq_move_item --------------------------------------------------
int	pq_move_item(SPrintQueueItem *pitem, int d)
{
	int i;
	if (_find_item(pitem->id.id, &i)==REPLY_OK)
	{
		if (i+d>=0 && i+d<_Size)
		{
			SPrintQueueItem x;
			memcpy(&x,			&_List[i+d], sizeof(x));
			memcpy(&_List[i+d],	&_List[i],	 sizeof(x));
			memcpy(&_List[i],   &x,			 sizeof(x));
		}
		return REPLY_OK;
	}
	return REPLY_ERROR;
}

//--- pq_preflight --------------------------------------------------------------
int pq_preflight(SPageId *pid)
{
	int i;
	if (_find_item(pid->id, &i)==REPLY_OK)
	{
		if (_List[i].state<PQ_STATE_PREFLIGHT)
		{
			_List[i].state=PQ_STATE_PREFLIGHT;
			memset(_List[i].ripState, 0, sizeof(_List[i].ripState));
			gui_send_print_queue(EVT_GET_PRINT_QUEUE, &_List[i]);
		}
		return REPLY_OK;
	}
	return REPLY_ERROR;
}

//--- pq_ripping ---------------------------------------------------------------
int pq_ripping(SPrintQueueItem *pitem)
{
	int i;
	if (_find_item(pitem->id.id, &i)==REPLY_OK)
	{
		if (_List[i].state<=PQ_STATE_RIPPING)
		{
			_List[i].state=PQ_STATE_RIPPING;
			_Loading[i] = 0;
			memset(&_ListText[i], 0, sizeof(_ListText[i]));
			strcpy(_List[i].ripState, pitem->ripState);
			gui_send_print_queue(EVT_GET_PRINT_QUEUE, &_List[i]);
		}
		return REPLY_OK;
	}
	return REPLY_ERROR;
}

//--- pq_loading --------------------------------------------------------
int pq_loading(int spoolerNo, SPageId *pid, char *txt)
{
	int i;
	int n, l, len;
//	if (RX_PrinterStatus.printState!=ps_printing) return REPLY_OK;
	if (_find_item(pid->id, &i)==REPLY_OK)
	{
		if (_List[i].state<=PQ_STATE_LOADING || *txt)
		{			
//			if (_List[i].state<=PQ_STATE_LOADING)
			if (_List[i].state!=PQ_STATE_LOADING)
			{
				_Loading[i] |= 1<<spoolerNo;
				_List[i].state=PQ_STATE_LOADING;
				// _List[i].scansSent = 0;
			}
			strncpy(_ListText[i][spoolerNo], txt, sizeof(_ListText[i][spoolerNo])-1);
			_ListText[i][spoolerNo][sizeof(_ListText[i][spoolerNo])] = 0;
			for (n=0, len=0; n<SPOOL_CNT; n++)
			{
				l = (int)strlen(_ListText[i][n]);
				if (len+l>=sizeof(_List[i].ripState)) 
					break;
				len += sprintf(&_List[i].ripState[len], "%8s", _ListText[i][n]);
			}
			if (_ActiveState<=_List[i].state && strcmp(_ActiveRipState, _List[i].ripState))
			{
				strcpy(_ActiveRipState, _List[i].ripState); 
				_ActiveState=_List[i].state;
				gui_send_print_queue(EVT_GET_PRINT_QUEUE, &_List[i]);								
			}
		}
		return REPLY_OK;
	}
	return REPLY_ERROR;
}

//--- pq_sending ---------------------------------------------------------------
int pq_sending(int spoolerNo, SPageId *pid)
{
	int i;
	int send=FALSE;
	if (RX_PrinterStatus.printState!=ps_printing) return REPLY_OK;
	if (_find_item(pid->id, &i)==REPLY_OK)
	{		
		if (*_List[i].ripState)
		{
			_List[i].ripState[0]=0;
			send=TRUE;
		}
		if (_List[i].state<PQ_STATE_TRANSFER)
		{
			_Loading[i] &= ~(1<<spoolerNo);	
			if (!_Loading[i])
			{
				_List[i].ripState[0]=0;
				_List[i].state=PQ_STATE_TRANSFER;
				send=TRUE;
			}
		}
		if (send && _ActiveState<_List[i].state) 
		{
			_ActiveState=_List[i].state;
			gui_send_print_queue(EVT_GET_PRINT_QUEUE, &_List[i]);			
		}
		return REPLY_OK;
	}
	return REPLY_ERROR;
}

//--- pq_stopping ---------------------------------------------------------------
int pq_stopping(SPrintQueueItem *pitem)
{
	int i;
	SPrintQueueItem item;
//	Error(LOG, 0, "pq_stopping 1 (id=%d, page=%d, copy=%d, scan=%d)", pitem->id.id, pitem->id.page, pitem->id.copy, pitem->id.scan);
	/*
	if (RX_Config.printer.type==printer_LH702)
	{
		for (int i=0; i<_Size; i++)
		{
			if (_List[i].state>PQ_STATE_PREFLIGHT)
			{
				_List[i].state=PQ_STATE_STOPPING;
		//		Error(LOG, 0, "STOPPING PrintQueue[%d].file=>>%s<<, State=%d", i, _List[i].filepath, _List[i].state);
				gui_send_print_queue(EVT_GET_PRINT_QUEUE, &_List[i]);
			}
		}
		return REPLY_OK;
	}
	*/

	if (_find_item(pitem->id.id, &i)==REPLY_OK)
	{
		if (_List[i].state<PQ_STATE_STOPPING)
		{
			_List[i].state=PQ_STATE_STOPPING;
			memcpy(&item, &_List[i], sizeof(item));
	//		Error(LOG, 0, "pq_stopping 2 (id=%d, page=%d, copy=%d, scan=%d)", item.id.id, item.id.page, item.id.copy, item.id.scan);
			pq_next_page(&_List[i], &item.id);
			gui_send_print_queue(EVT_GET_PRINT_QUEUE, &_List[i]);
		}
		return REPLY_OK;
	}
	return REPLY_ERROR;
}

//--- pq_stopped ----------------------------
void pq_stopped(SPrintQueueItem *pitem)
{
	int i;
	pitem->state  = PQ_STATE_STOPPED;
	if (_find_item(pitem->id.id, &i)==REPLY_OK) _List[i].state= PQ_STATE_STOPPED;
}

//--- _filename ------------------------------------
static char* _filename(char *path)
{
	char ripped_data[32];
	int pos, len;
	strcpy(ripped_data, PATH_RIPPED_DATA_DIR);
	len = (int)strlen(ripped_data);
	ripped_data[--len]=0;
	for (pos=0; path[pos]; pos++)
	{
		if (!strncmp(&path[pos], ripped_data, len))
		{
			pos+=len;
			while(path[pos]=='\\' || path[pos]=='/' || path[pos]==':') pos++;
			return &path[pos];			
		}
	}
	return path;
}

//--- pq_next_page ----------------------------------------
void pq_next_page(SPrintQueueItem *pitem, SPageId *pid)
{
	if (RX_PrinterStatus.testMode) return;
	pid->id = pitem->id.id;

	if(rx_def_is_scanning(RX_Config.printer.type))
	{
		if(pitem->srcPages>1)
		{
			if( pitem->id.scan < pitem->scans)
			{
				pid->copy = 1;
				pid->page = pitem->id.page;
				pid->scan = pitem->id.scan+1;	
			}
			else
			{
				pid->copy = 1;
				pid->page = pitem->id.page+1;
				pid->scan = 0;				
			}
		}
		else
		{
			if(pitem->id.scan < pitem->scans)
			{
				pid->copy = pitem->id.copy;
				if(pitem->lengthUnit == PQ_LENGTH_COPIES && pitem->scans != pitem->scansStart && pitem->copies)
				{
					double copy = (double)(pitem->id.scan - pitem->scansStart) / ((double)(pitem->scans - pitem->scansStart) / (double)pitem->copies);
					if((int)copy >= pid->copy) pid->copy = (int)copy + 1;
				}
				pid->page = pitem->id.page;
				pid->scan = pitem->id.scan+1;	
			}
			else
			{	
				if(pitem->lengthUnit == PQ_LENGTH_COPIES && pitem->scans != pitem->scansStart && pitem->copies)
				{
					double copy = (double)(pitem->id.scan - pitem->scansStart) / ((double)(pitem->scans - pitem->scansStart) / (double)pitem->copies);
					if((int)copy >= pid->copy) pid->copy = (int)copy + 1;
				}
				else pid->copy = pitem->id.copy+1;
				pid->page = pitem->id.page;
				pid->scan = 1;								
			}			
		}
	}
	else // WEB
	{
		pid->scan = 1;
		if (pitem->collate)
		{
			//--- copies of whole document ----------------------
			if (pitem->id.page+1<=pitem->lastPage) 
			{
				pid->copy = pitem->id.copy;
				pid->page = pitem->id.page+1;
			}
			else
			{
				pid->copy = pitem->id.copy+1;
				pid->page = pitem->firstPage;
			}
		}
		else if (pitem->copies)
		{
			//--- copies of each page ---------------------------
			if (pitem->id.copy < pitem->copies)
			{
				pid->copy = pitem->id.copy+1;
				pid->page = pitem->id.page;
			}
			else
			{
				pid->copy  = 1;
				pid->page  = pitem->id.page+1;
			}
		}			
	}
}

//--- pq_printed ---------------------------------------------------------------
int pq_printed(int headNo, SPageId *pid, int *pageDone, int *jobDone, SPrintQueueItem **pnextItem)
{
	int idx=0;
	int printed;
	SPrintQueueItem *pitem;

	*pageDone  = FALSE;
	*jobDone   = FALSE;
	*pnextItem = NULL;
	
	if(RX_PrinterStatus.testMode || _find_item(pid->id, &idx) == REPLY_OK)
	{
		if (RX_PrinterStatus.testMode) pitem = &RX_TestImage;
		else 
		{
			pitem = &_List[idx];
		    pitem->id.page = pid->page;
		    pitem->id.copy = pid->copy;
		}
		_Speed = pitem->speed;
		
		pq_next_page(pitem, &pitem->start);
		
		printed = pitem->copiesPrinted;
		if(rx_def_is_scanning(RX_Config.printer.type))
		{
			pitem->scansPrinted++;
			*pageDone = TRUE;
			if (pitem->srcPages>1 && (pid->scan==pitem->scans || (pitem->copiesPrinted && pid->scan==pitem->scans-pitem->scansStart)))
			{	
				*pageDone = TRUE;
				pitem->copiesPrinted = pitem->id.page;
			}
			else if (pitem->scans<=pitem->scansStart || pid->scan<pitem->scansStart || !pitem->copies) 
			{
			}
			else
			{
				double p;
				p = (double)(pid->scan-pitem->scansStart) / ((double)(pitem->scans-pitem->scansStart) / (double)pitem->copies);
				if((int)p>printed)
				{
					*pageDone = TRUE;
					if (pitem->scansPrinted==pitem->scansTotal)	pitem->copiesPrinted = pitem->copies;
					else										pitem->copiesPrinted = pitem->id.copy;
				}
			}
		}
		else
		{
			*pageDone = TRUE;
			pitem->copiesPrinted++;
			if (RX_Config.printer.type==printer_DP803 && (pitem->lastPage>pitem->firstPage))
				pitem->scansPrinted = (pitem->id.copy-1) * (pitem->lastPage - pitem->firstPage + 1) + pitem->id.page;
		}
		
		TrPrintfL(TRUE, "pq_printed id=%d, scan=%d, scanTotal=%d, scansPrinted=%d, copiesPrinted=%d, copies=%d, copiesTotal=%d, state=%d, counterAct=%.3f", pid->id, pid->scan, pitem->scans, pitem->scansPrinted, pitem->copiesPrinted, pitem->copies, pitem->copiesTotal, pitem->state, RX_PrinterStatus.counterAct);
				
		//--- log ----------------------------------------
		if(rx_def_is_scanning(RX_Config.printer.type))
		{
			static int _printed = 0;
			static int _time;

			if(pitem->copiesPrinted != printed) 
				pitem->start.page = pid->page + 1;			
			if(pitem->scansPrinted < 2)
			{
				_printed = 0;
				_time = rx_get_ticks();
			}
			double step=plc_get_step_dist_mm();
            if (step==0)
                step=plc_get_step_dist_mm();
			ctr_add((int)step);
			
			//--- LOG ---
			{	
				int printed = (int)(step*pitem->scansPrinted / 1000);
				int time    = rx_get_ticks();
				if(printed > _printed)
				{
					_printed = printed;			
					Error(LOG, 0, "%s: printed %d m time=%d.%03d", _filename(pitem->filepath), _printed, (time - _time) / 1000, (time - _time) % 1000);
					_time = time;
				}				
			}
		}
		else 
		{
			{ //---------- test --------------------------------
				static int _id=0;
				static int _srcHeight=0;
				if (_PrintDoneCnt==0)
				{
					_id=pitem->id.id;
					_srcHeight = 1;
				}
				if (pitem->id.id!=_id)
				{
					_id = pitem->id.id;
					if (pitem->srcHeight==0) Error(ERR_CONT, 0, "Counter: new jobId=%d, srcHeight=%d, file >>%s<<", pitem->id.id, pitem->srcHeight,  _filename(pitem->filepath));
				}
				if (pitem->srcHeight==0 && _srcHeight!=0) Error(ERR_CONT, 0, "Counter: new jobId=%d, copy=%d, srcHeight=%d, file >>%s<<", pitem->id.id, pitem->id.copy, pitem->srcHeight,  _filename(pitem->filepath));
				_srcHeight = pitem->srcHeight;
			}

			ctr_add(pitem->srcHeight / 1000);		
		}
	
		if(pitem->state <= PQ_STATE_STOPPING)
		{		
			if (pid->copy==pitem->copies) spool_file_printed(pid);
			if (pitem->scansStop && pitem->scansPrinted>=pitem->scansStop)
			{
				*jobDone = TRUE;
			}
			else if(RX_Config.printer.type==printer_DP803 && pitem->copiesTotal && pitem->scansPrinted >= pitem->copiesTotal) 
			{
				*jobDone = TRUE;
			}
			else
			{
				if((pitem->copiesTotal && pitem->copiesPrinted >= pitem->copiesTotal) 
				|| (!pitem->copiesTotal && pitem->scans && pitem->scansPrinted  >= pitem->scans))
					*jobDone = TRUE;
				else if (pitem->scansTotal && pitem->scansPrinted >= pitem->scansTotal) 
					*jobDone = TRUE;
			}
			if (*jobDone)
			{
				if (!RX_PrinterStatus.testMode && idx + 1 < _Size) *pnextItem = &_List[idx + 1];
				TrPrintfL(TRUE, "PQ.size=%d, idx=%d", _Size, idx);
				if(pitem->state != PQ_STATE_PRINTED)
				{
	//				TrPrintfL(TRUE, "%s: complete", _filename(pitem->filepath));
					if (pitem->scansStop && pitem->scansPrinted>=pitem->scansStop)
						Error(ERR_CONT, 0, "%d: %s: stopped", pitem->id .id, _filename(pitem->filepath));
					else
						Error(LOG, 0, "%d: %s: complete", pitem->id .id, _filename(pitem->filepath));
//					*jobDone = TRUE;
					pitem->state = PQ_STATE_PRINTED;
					gui_send_print_queue(EVT_GET_PRINT_QUEUE, pitem);
					if (rx_def_is_scanning(RX_Config.printer.type))
					{
						enc_enable_printing(FALSE);
						if(*pnextItem==NULL || RX_PrinterStatus.printState==ps_stopping) pc_abort_printing();
					}
					if (RX_Config.printer.type==printer_LH702 && *pnextItem)
					{
						enc_change();
					}
				}
			}
			else pitem->state   = PQ_STATE_PRINTING;
		}
		else 
		{
			TrPrintfL(TRUE, "PQ_STATE_STOPPING (id=%d, page=%d, copy=%d, scan=%d)", pid->id, pid->page, pid->copy, pid->scan);
		}
	
		_PrintDoneCnt++;
		if (RX_PrinterStatus.printState==ps_stopping)
		{
		//	Error(LOG, 0, "PrintDone=%d, stop-at=%d", _PrintDoneCnt, enc_pg_stop_cnt());
			int cnt=enc_pg_stop_cnt();
			if (cnt && _PrintDoneCnt==cnt ) pc_abort_printing();				
		}
		
		memcpy(&_PrintedItem, pitem, sizeof(_PrintedItem));
		return *pageDone || *jobDone;
	}

	_find_item(pid->id, &idx);
	return FALSE;
}

//--- pq_sent ----------------------------------------------
SPrintQueueItem *pq_sent(SPageId *pid)
{
	int i;
	if (RX_PrinterStatus.testMode)
	{
		_TestDataSent++;
	}
	else if (_find_item(pid->id, &i)==REPLY_OK)
	{
		_List[i].scansSent++;
		return &_List[i];
	}
	return NULL;
}

//--- pq_got_printGo ----------------------------------------------
SPrintQueueItem *pq_got_printGo(SPageId *pid)
{
	int i;
	if (_find_item(pid->id, &i)==REPLY_OK)
	{
		if (i < _Size) return &_List[i+1];
	}
	return NULL;
}

//--- pq_is_ready2print -----------------------------------
int pq_is_ready2print(SPrintQueueItem *pitem)
{
	int i;
	if (RX_PrinterStatus.testMode)
	{
	//	Error(LOG,0, "pq_is_ready2print test: sent=%d, copies=%d", _TestDataSent, RX_TestImage.copies);
		return _TestDataSent>5 || _TestDataSent>=(int)RX_TestImage.copies; // TEST PRINT
	}
	if (pitem->id.id==0) return TRUE;
	if (_find_item(pitem->id.id, &i)==REPLY_OK)
	{
		TrPrintf(TRUE, "pq_is_ready2print: state=%d, scansSent=%d, scans=%d", _List[i].state, _List[i].scansSent, _List[i].scans);
		if (_List[i].state>PQ_STATE_TRANSFER 
		|| (_List[i].scans>0 && _List[i].scansSent>=min(5, _List[i].scans)) 
	    || (_List[i].scans==0 && _List[i].scansSent>=min(5, _List[i].copiesTotal-_List[i].copiesPrinted)))  
			return TRUE;
	}
	return FALSE;
}

//--- pq_sent_document -----------------------------
void pq_sent_document(int pages)
{
	_BufState = 4;
}

//--- pq_is_ready ----------------------------------
int pq_is_ready(void)
{
	if (TRUE)
	{
		static UINT32 sent=0;
		static UINT32 transferred=0;
		static UINT32 printGoCnt=0;

		if (RX_PrinterStatus.sentCnt!=sent || RX_PrinterStatus.transferredCnt!=transferred || RX_PrinterStatus.printGoCnt!=printGoCnt)
		{
			// if (RX_PrinterStatus.sentCnt) Error(LOG, 0, "Buffered pages=%d", RX_PrinterStatus.sentCnt-RX_PrinterStatus.printedCnt);
			TrPrintfL(TRUE, "pq_is_ready: Buffer: sent=%d, transferred=%d, printGoCnt=%d, buffered=%d", RX_PrinterStatus.sentCnt, RX_PrinterStatus.transferredCnt, RX_PrinterStatus.printGoCnt, RX_PrinterStatus.transferredCnt-RX_PrinterStatus.printGoCnt);
			sent=RX_PrinterStatus.sentCnt;
			transferred=RX_PrinterStatus.transferredCnt;
			printGoCnt=RX_PrinterStatus.printGoCnt;

			{
			//	int bufsize = RX_PrinterStatus.transferredCnt-RX_PrinterStatus.printedCnt;
				int bufsize = RX_PrinterStatus.transferredCnt-RX_PrinterStatus.printGoCnt;
				
				if ((RX_Config.printer.type==printer_DP803) && RX_PrinterStatus.transferredCnt && RX_PrinterStatus.transferredCnt%100==0) Error(LOG, 0, "Buffer size (transferred=%d, bufsize=%d, speed=%d)", RX_PrinterStatus.transferredCnt, bufsize, _Speed);
	
				switch(_BufState)
				{
				case 0: // filling
						if (bufsize>15) _BufState = 1; 
						break;
			
				case 1: // buffer full
						if(bufsize < 10)	
						{
							Error(WARN, 0, "Buffer low (transferred=%d, bufsize=%d, speed=%d)", RX_PrinterStatus.transferredCnt, bufsize, _Speed);
							_BufState = 2;
						}
						break;
			
				case 2:	// buffer low
						if (bufsize>15)
						{
							Error(LOG, 0, "Buffer recovered (transferred=%d, bufsize=%d)", RX_PrinterStatus.transferredCnt, bufsize);
							_BufState = 1;							
						}
						else if (bufsize<3)	
						{
							Error(ERR_STOP, 0, "Buffer underflow (transferred=%d, bufsize=%d, speed=%d)", RX_PrinterStatus.transferredCnt, bufsize, _Speed);
							_BufState=3;
						}
						break;
			
				case 3: // underflow
						break;
				
				case 4:	// ENDING Document: check OFF 
						break;
				default: break;
				}					
			}
		}		
	}
	
	if(RX_Config.printer.type == printer_LH702)				
	{
		TrPrintfL(TRUE, "pq_is_ready: sentCnt=%d, printGoCnt=%d", RX_PrinterStatus.sentCnt, RX_PrinterStatus.printGoCnt);
		if (RX_PrinterStatus.testMode) return (RX_PrinterStatus.sentCnt-RX_PrinterStatus.printGoCnt) < 64;
		else						   return (RX_PrinterStatus.sentCnt-RX_PrinterStatus.printGoCnt) < 5;	// minimize buffer, independent on format!
	}
	else if(RX_Config.printer.type == printer_cleaf)	return (RX_PrinterStatus.sentCnt-RX_PrinterStatus.printedCnt) < 16;
	else if (rx_def_is_tx(RX_Config.printer.type))		return (RX_PrinterStatus.sentCnt-RX_PrinterStatus.printedCnt) < 20;
	else												return (RX_PrinterStatus.sentCnt-RX_PrinterStatus.printedCnt) < 64;
}
