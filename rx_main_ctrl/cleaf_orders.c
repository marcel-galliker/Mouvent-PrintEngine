// ****************************************************************************
//
//	cleaf_orders.c		
//
// ****************************************************************************
//
//	Copyright 2018 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

//--- includes ----------------------------------------------------------------

#include "gui_svr.h"
#include "tcp_ip.h"
#include "rx_error.h"
#include "rx_file.h"
#include "rx_log.h"
#include "rx_setup_file.h"
#include "cleaf_orders.h"

static void _update_counters(void);
static void _save_status(const char *time);

//--- global vairables --------------------------------------
static SCleafOrder			_Order;
static SCleafProduction		_Prod;
static SCleafRoll			_Roll[256];
static SCleafRoll			*_ActRoll;
static int					_RollCnt;
static double				_PageLength;
static char					_Operator[64]="";
static char					_StatusTime[64]="";
static int					_LogNo=0;



//--- co_init -------------------------------------------
void co_init()
{
	static log_Handle			log=NULL;
	log_open(PATH_LOG FILENAME_LOG, &log, FALSE);
	_LogNo = log_get_last_item_no(log);
	log_close(&log);
	/*
	strcpy(_Operator, "");
	co_read_order("180018015", &_Order);
	co_setup_production(_Order.id, READ,  &_Prod, _Roll, SIZEOF(_Roll), &_RollCnt);
//	co_setup_production(_Order.id, WRITE, &_Prod, _Roll, SIZEOF(_Roll), &_RollCnt);
	*/
}

//--- co_tick -----------------------------
void co_tick(void)
{
	if (RX_Config.printer.type==printer_cleaf)
	{
		char time[64];
		rx_get_system_time_str(time, '-');
		if (strcmp(time, _StatusTime))
		{
			strcpy(_StatusTime, time);
			_save_status(_StatusTime);
		}
	}
}

//--- _save_status ----------------------------------
static void _save_status(const char *time)
{
	EN_setup_Action action = WRITE;
	char path[MAX_PATH], *ch;
	HANDLE file = setup_create();
	
	rx_remove_old_files(PATH_CLEAF_ORDERS "state/", 1);
	
	sprintf(path, "%s%s/%s.xml", PATH_CLEAF_ORDERS, "state", time);
	for (ch=path; *ch; ch++) if (*ch==':') *ch='.';
	
	if (setup_chapter(file, "Status", -1, action)==REPLY_OK)
	{
		setup_str	(file, "Operator",	action, _Operator,	sizeof(_Operator),	"");
		setup_str	(file, "Order",	    action, _Order.id,	sizeof(_Order.id),	"");
		setup_int32	(file, "Length",	action,	&_Order.length,		0);	
		setup_int32	(file, "Produced",	action,	&_Prod.produced,	0);	
		if (_ActRoll!=NULL)
			setup_int32	(file, "Roll",		action,	&_ActRoll->no,		0);	
		
		if (setup_chapter(file, "Log", -1, action)==REPLY_OK)
		{
			int					no=0;
			static log_Handle	log=NULL;

			log_open(PATH_LOG FILENAME_LOG, &log, FALSE);
			while (!log_eof(log, _LogNo))
			{
				log_save_as_xml(log, _LogNo++, ++no, file);
			}
			log_close(&log);
			setup_chapter(file, "..", -1, action);
		}	
		
		setup_chapter(file, "..", -1, action);
	}
	setup_save(file, path);
	setup_destroy(file);
}


//--- _init_roll ----------------------------------------------
static void _init_roll(int no)
{
	memset(&_Roll[no], 0, sizeof(_Roll[no]));
	_Roll[no].no = no+1;
	_RollCnt = 0;
	_ActRoll = &_Roll[0];
}

//--- co_read_order ------------------------------------------------
int co_read_order(char *id, SCleafOrder *porder)
{
	int i;
	char path[MAX_PATH];
	EN_setup_Action action	= READ;
	HANDLE file = setup_create();

	sprintf(path, "%s%s/%s.xml", PATH_CLEAF_ORDERS, id, id);

	if (action==READ) memset(porder, 0, sizeof(*porder));

	setup_load(file, path);

	if (setup_chapter(file, "Order", -1, action)==REPLY_OK)
	{
		setup_str	(file, "Id",			action, porder->id,			sizeof(porder->id),			"");
	
		if (setup_chapter(file, "Settings", -1, action)==REPLY_OK)
		{
			setup_str	(file, "Code",			action, porder->code,		sizeof(porder->code),		"");
			setup_str	(file, "AbsCode",		action, porder->absCode,	sizeof(porder->absCode),	"");
			setup_str	(file, "Material",		action, porder->material,	sizeof(porder->material),	"");
			setup_int32	(file, "Speed",			action,	&porder->speed, 0);	
			setup_int32	(file, "Length",		action,	&porder->length, 0);	
			setup_double(file, "PrintHeight",	action,	&porder->printHeight, 0);	
			setup_str	(file, "PrintFile",		action, porder->printFile,	sizeof(porder->printFile),	"");
			setup_int32	(file, "Gloss",			action,	&porder->gloss, 0);	
			setup_str	(file, "Note",			action, porder->note,		sizeof(porder->note),		"");
	
			for (i=0; i<SIZEOF(porder->flexo); i++)
			{
				if (setup_chapter(file, "Flexo", i+1, action)==REPLY_OK) 
				{
					setup_str	(file, "LacCode",			action, porder->flexo[i].lacCode, sizeof(porder->flexo[i].lacCode), "");
					setup_str	(file, "Anilox",			action, porder->flexo[i].anilox, sizeof(porder->flexo[i].anilox), "");
					setup_str	(file, "RubberRoll",		action, porder->flexo[i].robberRoll, sizeof(porder->flexo[i].robberRoll), "");
					setup_chapter(file, "..", -1, action);
				}			
			}
			setup_chapter(file, "..", -1, action);						
		}
		setup_chapter(file, "..", -1, action);
	}
	setup_destroy(file);
	return REPLY_OK;
}

//--- co_setup_production ----------------------------------------
int co_setup_production	(char *id, EN_setup_Action action, SCleafProduction *pprod, SCleafRoll *roll, int size, int *pcnt)
{
	int i;
	char path[MAX_PATH];
	HANDLE file = setup_create();
	
	sprintf(path, "%s%s/%s-Rolls.xml", PATH_CLEAF_ORDERS, id, id);

	setup_load(file, path);
	if (action==READ)
	{
		memset(pprod, 0, sizeof(*pprod));
	}
	
	_update_counters();
	if (setup_chapter(file, "Order", -1, action)==REPLY_OK)
	{
		if (action==WRITE) setup_str(file, "Id", action, id, MAX_PATH, "");
		
		if (setup_chapter(file, "Production", -1, action)==REPLY_OK)
		{
			setup_int32	(file, "Produced",			action,	&pprod->produced,	0);	
			setup_int32	(file, "Waste",				action,	&pprod->waste,		0);	
			setup_int32	(file, "InkUsed_Cyan",		action,	&pprod->inkUsed[0], 0);	
			setup_int32	(file, "InkUsed_Magenta",	action,	&pprod->inkUsed[1], 0);	
			setup_int32	(file, "InkUsed_Yellow",	action,	&pprod->inkUsed[2], 0);	
			setup_int32	(file, "InkUsed_Black",		action,	&pprod->inkUsed[3], 0);	
			
			setup_chapter(file, "..", -1, action);
		}

		if (action==WRITE) size=*pcnt;
		
		for (i=0; i<size; i++)
		{			
			if (setup_chapter(file, "Roll", i+1, action)==REPLY_OK) 
			{
				if (action==READ) *pcnt=i+1;			
				setup_double(file, "Length",		action,	&roll[i].length, 0);
				setup_enum	(file, "Quality",		action,	&roll[i].quality, "undef|waste|ok|");	
				setup_str	(file, "Operator",		action, roll[i].operator, sizeof(roll[i].operator), "");
				setup_chapter(file, "..", -1, action);
			}			
		}
		
		setup_chapter(file, "..", -1, action);
	}
	
	_update_counters();
	if (action==WRITE)	setup_save(file, path);

	setup_destroy(file);

	return REPLY_OK;
}

//--- _save_order ----------------------------------------------
static void _save_order(void)
{
	_update_counters();
	co_setup_production(_Order.id, WRITE,  &_Prod, _Roll, SIZEOF(_Roll), &_RollCnt);
	gui_send_msg_2(INVALID_SOCKET, REP_CO_GET_PRODUCTION, sizeof(_Prod), &_Prod);
	gui_send_msg_2(INVALID_SOCKET, REP_CO_SET_ROLL, sizeof(SCleafRoll), _ActRoll);
}

//--- co_send_order -----------------------------------------------
void co_send_order(RX_SOCKET socket)
{
	int i;
	gui_send_msg_2(socket, REP_CO_GET_ORDER,		sizeof(_Order), &_Order);
	gui_send_msg_2(socket, REP_CO_GET_PRODUCTION,	sizeof(_Prod),	&_Prod);
	gui_send_msg_2(socket, REP_CO_GET_ROLLS, 0, NULL);
	for (i=0; i<_RollCnt; i++)
	{
		_Roll[i].no = i+1;
		gui_send_msg_2(socket, REP_CO_SET_ROLL, sizeof(_Roll[i]), &_Roll[i]);			
	}	
}

//--- _update_counters --------------------------------------
static void _update_counters(void)
{
	_Prod.waste		= 0;
	_Prod.produced	= 0;

	for (int i=0; i<_RollCnt; i++)
	{
		switch(_Roll[i].quality)
		{
		case CO_QUALITY_WASTE:	_Prod.waste		+=	(int)_Roll[i].length; break;
		case CO_QUALITY_OK:		_Prod.produced	+=	(int)_Roll[i].length; break;
		default: break;	
		}
	}	
}

//--- co_set_order ---------------------------
void co_set_order(char *id)
{
	co_read_order(id, &_Order);
	co_setup_production(_Order.id, READ,  &_Prod, _Roll, SIZEOF(_Roll), &_RollCnt);
	if(_RollCnt==0)
	{
		_init_roll(_RollCnt++);
	}
	_ActRoll = &_Roll[_RollCnt-1];
	co_send_order(INVALID_SOCKET);
}

//--- co_set_roll -----------------------
void co_set_roll(SValue* pvalue)
{
	int no = pvalue->no-1;
	if (no<_RollCnt)
	{
		_Roll[no].quality = pvalue->value;
		_save_order();
	}
}

//--- co_set_operator ------------------------------
void co_set_operator(char *name)
{		
	strncpy(_Operator, name, sizeof(_Operator));
	SCleafRoll roll;
	strcpy(roll.operator, _Operator);
	gui_send_msg_2(INVALID_SOCKET, REP_CO_SET_OPERATOR, sizeof(roll), &roll);
}

//--- co_start_printing ----------------------------------
void co_start_printing(int pageLength)
{
	_PageLength = pageLength/1000000.0;
}

//--- co_stop_printing ------------------------------------
void co_stop_printing(void)
{
	if (_RollCnt) _save_order();
}

//--- co_printed -----------------------------------------
void co_printed(void)
{
	if (_RollCnt)
	{
		int cnt=(int)(_ActRoll->length/10);
		_ActRoll->quality = CO_QUALITY_UNDEF;
		_ActRoll->length += _PageLength;
		if ((int)_ActRoll->length/10>cnt) _save_order();
	}
}

//--- co_roll_end ------------------------------------
void co_roll_end(void)
{
	if (_RollCnt)
	{
		strcpy(_ActRoll->operator, _Operator);
		Error(LOG, 0, "ROLL[%d] done, length=%d m ", _ActRoll->no, (int)_ActRoll->length);
		_init_roll(_RollCnt);
		_ActRoll = &_Roll[_RollCnt++];
		_save_order();		
	}
};