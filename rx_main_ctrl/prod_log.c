// ****************************************************************************
//
//	DIGITAL PRINTING - pro_log.c
//
// ****************************************************************************
//
//	Copyright 2018 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

//--- Defines -----------------------------------------------------------------

#define PRODLOG
#undef  PRODLOG

#ifdef linux
	#include <time.h>
	#include <sys/time.h>
#endif

#include "libxl.h"
#include "rx_file.h"
#include "rx_threads.h"
#include "bmp.h"
#include "print_queue.h"
#include "prod_log.h"

//--- Statics -----------------------------------------------------------------

static char				_Day[32];
static char				_PreviewPath[MAX_PATH];
static char				_FilePath[MAX_PATH];
static SPrintQueueItem	_Start, _LastJob;
static double			_StartTime;
static BookHandle		_ProdLog;
static SheetHandle		_ActSheet;
static FormatHandle		_TitleFormat;	// 1
static FontHandle		_TitleFont;
static FormatHandle		_DateFormat;	// 2

static void *_prod_log_thread(void *lpParameter);

//--- Prototypes --------------------------------------------------------------
static int _add_picture(char *name, char *path);
static void _pl_stop_tx(SPrintQueueItem *pitem);

//--- pl_init --------------------------------------
void pl_init(void)
{
#ifdef PRODLOG
	memset(&_Start,0, sizeof(_Start));
	memset(&_LastJob,0, sizeof(_LastJob));
	_ProdLog = xlCreateXMLBook();
	xlBookSetKey(_ProdLog, "Mouvent AG", "linux-e5d51b7c91a7a91d0905233d43ncj7m3"); 
	pl_load();
#endif
}

//--- _add_picture ------------------------------
static int _add_picture(char *name, char *path)
{
#ifdef PRODLOG
	int rows =  xlSheetLastRow(_ActSheet);
	int row;
	int idx=-1;
	const char *str;
	FormatHandle format;
	for (row=0; row<rows; row++)
	{
		str = xlSheetReadStr(_ActSheet, row, 1, &format);
		if (format==_TitleFormat)
		{
			idx++;				
			if (str && !strcmp(str, name)) return idx;			
		}		
	}
	return xlBookAddPicture(_ProdLog, path);
#endif
}

//--- pl_load --------------------------------------
void pl_load(void)
{
#ifdef PRODLOG
	FontHandle	font;
	
	rx_remove_old_files(PATH_LOG, 90);
	
	rx_get_system_day_str(_Day, '-');
	sprintf(_FilePath, "%sprod-%s.xlsx", PATH_LOG, _Day);
	 
	if(xlBookLoad(_ProdLog, _FilePath)) _ActSheet = xlBookGetSheet(_ProdLog, 0);						
	else								_ActSheet = xlBookAddSheet(_ProdLog, _Day, 0);

	int formats= xlBookFormatSize(_ProdLog);
	_TitleFormat = xlBookFormat(_ProdLog, 1);
	if (_TitleFormat==0)
	{
		_TitleFormat=xlBookAddFormat(_ProdLog, NULL);
		font = xlFormatFont(_TitleFormat);
		_TitleFont = xlBookAddFont(_ProdLog, font);
		xlFontSetBold(_TitleFont, TRUE);
		xlFormatSetFont(_TitleFormat, _TitleFont);
	}
	_DateFormat = xlBookFormat(_ProdLog, 2);
	if (_DateFormat==0)
	{
		_DateFormat=xlBookAddFormat(_ProdLog, NULL);
		xlFormatSetNumFormat(_DateFormat, NUMFORMAT_CUSTOM_HMMSS);
	}
	xlSheetSetCol(_ActSheet, 0, 0, 50, NULL, FALSE);
#endif
}

//--- pl_start --------------------------------------------
void pl_start(SPrintQueueItem *pitem, char *localpath)
{
#ifdef PRODLOG
	memcpy(&_Start, pitem, sizeof(_Start));
	strcpy(_PreviewPath, localpath);

	int hour, min, sec;
	rx_get_system_hms(&hour, &min, &sec);

	_StartTime =  xlBookDatePack(_ProdLog, 0, 0, 0, hour, min, sec, 0);
#endif
}

//--- pl_stop -----------------------------------
void pl_stop(SPrintQueueItem *pitem)
{
	rx_thread_start(_prod_log_thread, pitem, 0, "_prod_log_thread");
}

static void *_prod_log_thread(void *lpParameter)
{
	SPrintQueueItem *pitem = (SPrintQueueItem*)lpParameter;
#ifdef PRODLOG
	char day[32];

	rx_get_system_day_str(day, '-');
	if (strcmp(day, _Day)) pl_load();			
		
	switch (RX_Config.printer.type)
	{
	case printer_TX801:	_pl_stop_tx(pitem); break; 
	case printer_TX802:	_pl_stop_tx(pitem); break; 
	default: break;
	}
#endif
	return NULL;
}

//--- _pl_stop_tx --------------------------------------------------
static void _pl_stop_tx(SPrintQueueItem *pitem) 
{
#ifdef PRODLOG
	int same;
	int preview;
	int row;
	int total;
	UINT32 width, height, memsize;
	UINT8 bitsPerPixel;
	char *ch=_PreviewPath;
	char *name=NULL;
	char path[MAX_PATH];
	char str[10];
	double rowHeight;
	double scale;
	
	if (RX_Config.printer.type!=printer_TX801 && RX_Config.printer.type!=printer_TX802) return;
	
	pitem = pq_get_item(pitem);
	
	if (!*_Start.filepath || !pitem) return;
	if (pitem->scansPrinted == _Start.scansPrinted) return;
	
	same = !strcmp(_Start.filepath, _LastJob.filepath)
		&& _Start.scanMode    == _LastJob.scanMode
		&& _Start.passes      == _LastJob.passes
		&& _Start.speed       == _LastJob.speed
		&& _Start.wakeup      == _LastJob.wakeup
		&& _Start.pageMargin  == _LastJob.pageMargin
		&& _Start.printGoDist == _LastJob.printGoDist
		&& _Start.lengthUnit  == _LastJob.lengthUnit
		&& _Start.scanLength  == _LastJob.scanLength;
			
	int hour, min, sec;
	rx_get_system_hms(&hour, &min, &sec);

	double time =  xlBookDatePack(_ProdLog, 0, 0, 0, hour, min, sec, 0);

	row = xlSheetLastRow(_ActSheet);
	
	if (!same)
	{
		for(ch=_PreviewPath; *ch; ch++)
		{
			if (*ch=='/') name=ch+1;			
		}

		if (name)
		{
			row++;
			xlSheetWriteStr(_ActSheet, row, 0, name, _TitleFormat); 
			xlSheetSetMerge(_ActSheet, row, row, 0, 3);

			sprintf(path, "%s/%s.bmp", _PreviewPath, name);
		
			preview = _add_picture(name, path);
	
			rowHeight = xlSheetRowHeight(_ActSheet, row);
			rowHeight = 20;
			bmp_get_size(path, &width, &height, &bitsPerPixel, &memsize);
			scale = (10.0*rowHeight)/height;
			xlSheetSetPicture(_ActSheet, row, 4, preview, scale, 0, 0, 0);		
			row++;
		}
		
		xlSheetWriteStr(_ActSheet, row, 0, "Scan Mode",		0);
		switch(_Start.scanMode)
		{
		case PQ_SCAN_STD:   strcpy(str, "--->"); break;
		case PQ_SCAN_RTL:   strcpy(str, "<---"); break;
		case PQ_SCAN_BIDIR: strcpy(str, "<-->"); break;
		default: strcpy(str, "--->");
		}
		xlSheetWriteStr(_ActSheet, row, 1, str,		0);
		row++;
		xlSheetWriteStr(_ActSheet, row, 0, "Passes",		0); xlSheetWriteNum (_ActSheet, row, 1, _Start.passes, 0);
		row++;
		xlSheetWriteStr(_ActSheet, row, 0, "Speed",			0); xlSheetWriteNum (_ActSheet, row, 1, _Start.speed, 0);	xlSheetWriteStr(_ActSheet, row, 2, "m/min",	0);
		row++;
		xlSheetWriteStr(_ActSheet, row, 0, "WakeUp",		0);	
		if(_Start.wakeup)	xlSheetWriteStr(_ActSheet, row, 1, "ON",	0);
		else				xlSheetWriteStr(_ActSheet, row, 1, "OFF",	0);
		row++;
		xlSheetWriteStr(_ActSheet, row, 0, "Image Margin",	0);	xlSheetWriteNum (_ActSheet, row, 1, _Start.pageMargin, 0); 	xlSheetWriteStr(_ActSheet, row, 2, "mm",	0);
		row++;
		xlSheetWriteStr(_ActSheet, row, 0, "Image Gap",		0);	xlSheetWriteNum (_ActSheet, row, 1, _Start.printGoDist, 0); xlSheetWriteStr(_ActSheet, row, 2, "mm",	0);
		row++;
		if(_Start.lengthUnit==PQ_LENGTH_COPIES)
		{
			xlSheetWriteStr(_ActSheet, row, 0, "Volume", 0); xlSheetWriteNum(_ActSheet, row, 1, _Start.copies, 0); xlSheetWriteStr(_ActSheet, row, 2, "copies", 0);
			row++;
		}
		else
		{
			xlSheetWriteStr(_ActSheet, row, 0, "Volume", 0); xlSheetWriteNum(_ActSheet, row, 1, _Start.scanLength/1000, 0); xlSheetWriteStr(_ActSheet, row, 2, "m", 0);
			row++;
		}			
	}	
	if(_Start.lengthUnit==PQ_LENGTH_COPIES)
	{
		xlSheetWriteStr(_ActSheet, row, 0, "Start at", 0); xlSheetWriteNum(_ActSheet, row, 1, _Start.start.copy, 0); 
		xlSheetWriteNum(_ActSheet, row, 3, _StartTime, _DateFormat);
		row++;
		xlSheetWriteStr(_ActSheet, row, 0, "Last Printed", 0); xlSheetWriteNum(_ActSheet, row, 1, pitem->copiesPrinted, 0);
		xlSheetWriteNum(_ActSheet, row, 3, time, _DateFormat);
	}
	else
	{
		xlSheetWriteStr(_ActSheet, row, 0, "Start at", 0); xlSheetWriteNum(_ActSheet, row, 1, _Start.start.scan, 0);	xlSheetWriteStr(_ActSheet, row, 2, "m", 0);
		xlSheetWriteNum(_ActSheet, row, 3, _StartTime, _DateFormat);
		row++;
		total = pitem->scans-pitem->scansStart;
		if (total>0)
		{
			xlSheetWriteStr(_ActSheet, row, 0, "Last Printed", 0); xlSheetWriteNum(_ActSheet, row, 1,_Start.scanLength/1000*(pitem->scansPrinted-pitem->scansStart)/total, 0);	xlSheetWriteStr(_ActSheet, row, 2, "m", 0);
			xlSheetWriteNum(_ActSheet, row, 3, time, _DateFormat);
		}
	}			
	xlSheetSetCol(_ActSheet, 0, 0, 20, NULL, FALSE);
	xlSheetSetCol(_ActSheet, 1, 3, -1, NULL, FALSE);
	memcpy(&_LastJob, &_Start, sizeof(_LastJob));
	memset(&_Start, 0, sizeof(_Start));
	pl_save();	
#endif
}

//--- pl_save --------------------------------------
void pl_save(void)
{
#ifdef PRODLOG
	xlBookSave(_ProdLog, _FilePath);
#endif
}
