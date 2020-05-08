// ****************************************************************************
//
//	DIGITAL PRINTING - main
//
// ****************************************************************************
//
//	Copyright 2012 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#include <time.h>
#include "rx_common.h"
#include "rx_def.h"
#include "rx_error.h"
#include "rx_log.h"
#include "rx_setup_file.h"
#include "rx_trace.h"
#ifdef WIN32
	#include <share.h>
	#pragma warning(disable: 4996)
#endif

#define COMMENT				"RX Error Log"
#define MAX_RECORD_CNT		100000

//--- EXTERN VARIABLES -----------------------------------------------------------------
// int m_chapter;

typedef struct SLogHeader
{
	char	comment[32];
	ULONG	max_cnt;
	ULONG	in;
	ULONG	cnt;
} SLogHeader;

typedef struct SLogHandle
{
	FILE		*file;
	SLogHeader	hdr;
} SLogHandle;

//--- defines ---------------------------------------------------------------------------

static void adjust_ctr(FILE *file, SLogHeader *hdr);

//--- log_open -------------------------------------------------------------------------
void log_open(const char *filename, log_Handle *handle, int write)
{
	int	len;
	SLogHandle *log;

	if (*handle == NULL) *handle = malloc(sizeof(SLogHandle));
	log = (SLogHandle*)(*handle);

	memset(&log->hdr, 0, sizeof(log->hdr));
#ifdef WIN32
	if (write)
	{
		log->file = _fsopen(filename, "r+b", _SH_DENYNO);// _SH_DENYWR);
		if (log->file == NULL) log->file = _fsopen(filename, "w+b", _SH_DENYNO);//_SH_DENYWR);
	}
	else log->file=_fsopen(filename, "rb", _SH_DENYNO);
#else
	if (write)
	{
		log->file = fopen(filename, "r+b");
		if (log->file == NULL) log->file = fopen(filename, "w+b");
	}
	else log->file=fopen(filename, "rb");

#endif
	if (log->file != NULL)
	{
		len = (int)fread(&log->hdr, 1, sizeof(log->hdr), log->file);
		if ((len != sizeof(log->hdr) || strcmp(log->hdr.comment, COMMENT)) && write)
		{
			strcpy(log->hdr.comment, COMMENT);
			log->hdr.in = 0;
			log->hdr.cnt=0;
			log->hdr.max_cnt = MAX_RECORD_CNT;
			fseek(log->file, 0, SEEK_SET);
			fwrite(&log->hdr, 1, sizeof(log->hdr), log->file);
			fflush(log->file);
		}
		if (log->hdr.max_cnt > MAX_RECORD_CNT) log->hdr.max_cnt = MAX_RECORD_CNT;
		log->hdr.in = log->hdr.in % log->hdr.max_cnt;
		adjust_ctr(log->file, &log->hdr);
	}
}

//--- log_add_item ---------------------------------------------------------------------
void log_add_item(log_Handle handle, SLogItem *item)
{
	SLogHandle *log = (SLogHandle*)handle;

	if (log!=NULL && log->file != NULL)
	{
		fseek(log->file, sizeof(SLogHeader)+log->hdr.in*sizeof(SLogItem), SEEK_SET);
		fwrite(item, 1, sizeof(SLogItem), log->file);
		log->hdr.in++;
		if (log->hdr.cnt<log->hdr.max_cnt)log->hdr.cnt++; 
		if (log->hdr.in >= log->hdr.max_cnt) log->hdr.in = 0;
		fseek(log->file, 0, SEEK_SET);
		fwrite(&log->hdr, 1, sizeof(log->hdr), log->file);
		fflush(log->file);
	}
}

//--- log_close --------------------------------------------------------------------
void log_close(log_Handle *handle)
{
	SLogHandle *log = (SLogHandle*)(*handle);
	if (log->file) fclose(log->file);
	free(*handle);
	*handle = NULL;
}

//--- adjust_ctr -------------------------------------
static void adjust_ctr(FILE *file, SLogHeader *hdr)
{
	ULONG		pos, i, l;
	SLogItem	log;
	UINT64		time1;

	//--- adjust pointer ----------------
	pos = hdr->in;
	fseek(file, sizeof(*hdr) + pos*sizeof(log), SEEK_SET);
	l = (int)fread(&log, 1, sizeof(log), file);
	time1 = log.time;
	for(i=0; i<20; i++)
	{
		if (pos<hdr->max_cnt)	pos++;
		else					pos=0;
		fseek(file, sizeof(*hdr) + pos*sizeof(log), SEEK_SET);
		l = (int)fread(&log, 1, sizeof(log), file);
		if (l == sizeof(log))
		{
			if (log.time<time1)
			{
				i++;
				hdr->in	= (hdr->in+i) % hdr->max_cnt;
				break;
			}
		}
	}
}


//--- log_get_item_cnt -----------------------------------------------------------
int  log_get_item_cnt(log_Handle handle)
{
	SLogHandle	*log = (SLogHandle*)handle;
	return log->hdr.cnt;
}

//--- log_get_last_item_no ----------------------
int	 log_get_last_item_no(log_Handle handle)
{
	SLogHandle	*log = (SLogHandle*)handle;
	if (log->hdr.cnt<log->hdr.max_cnt) return log->hdr.cnt;
	return (log->hdr.in+log->hdr.max_cnt-1) % log->hdr.max_cnt;			
}

//--- log_eof ----------------------------------------
int log_eof(log_Handle handle, int itemNo)
{
	SLogHandle	*log = (SLogHandle*)handle;
	return (itemNo+1) % log->hdr.max_cnt == log->hdr.in;				
}

//--- log_get_item ----------------------------------------------------------------
int  log_get_item(log_Handle handle, UINT32 itemNo, SLogItem *item)
{
	int pos, l;
	SLogHandle	*log;

	log = (SLogHandle*)handle;

	if (log->file == NULL) return REPLY_ERROR;

	//--- save texts -----------------------------------------
	if (itemNo<=log->hdr.cnt)
	{
		pos = itemNo % log->hdr.max_cnt;
		pos = sizeof(log->hdr) + pos*sizeof(SLogItem);
		fseek(log->file, pos, SEEK_SET);
		l = (int)fread(item, 1, sizeof(SLogItem), log->file);
		if (l != sizeof(SLogItem)) return 0;
		return  REPLY_OK;
	}
	return REPLY_ERROR;
}

//--- log_save_as_xml --------------------------------------
void log_save_as_xml(log_Handle handle, UINT32 itemNo, int no, HANDLE file)
{
	SLogHandle	*log;
	log = (SLogHandle*)handle;
	SLogItem item;
	EN_setup_Action action = WRITE;	
	char str[MAX_PATH];
	
	log_get_item(log, itemNo, &item);
	
	setup_chapter(file, "item", no, action);

	//--- type ------------------------------
	switch(item.logType)
	{
	case LOG_TYPE_WARN:			setup_str	(file, "Type",	action, "WARN",	32,	"");	break;
	case LOG_TYPE_ERROR_CONT:	
	case LOG_TYPE_ERROR_STOP:
	case LOG_TYPE_ERROR_ABORT:	setup_str	(file, "Type",	action, "ERROR",32,	"");	break;
	default:					setup_str	(file, "Type",	action, "LOG",	32,	"");	break;
	}

	//--- time ------------------------
	time_t t = time((time_t*)&item.time);
	struct tm *tmt = localtime(&t);
	sprintf(str, "%d.%s.%d  %d:%02d:%02d", tmt->tm_mday, RX_MonthStr[tmt->tm_mon%12], 1900+tmt->tm_year, tmt->tm_hour, tmt->tm_min, tmt->tm_sec);
	setup_str	(file, "Time",	action, str, sizeof(str),	"");
	
	//--- device --------------------------
	sprintf(str, "%s %d", DeviceStr[item.deviceType%11], item.deviceNo);
	setup_str	(file, "Device",	action, str,	sizeof(str),	"");
	setup_int32	(file, "ErrNo",		action, &item.errNo, 0);	 
 
	//--- message -----------------------
	compose_message(item.deviceType, item.deviceNo, item.errNo, str, SIZEOF(str), item.formatStr, item.arg);

	setup_str	(file, "Message",	action, str, sizeof(str), "");
		
	setup_chapter(file, "..", -1, action);
}
