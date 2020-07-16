// ****************************************************************************
//
//	print_ctrl.cpp	
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

//--- includes ----------------------------------------------------------------
#include "rx_def.h"
#include "rx_error.h"
#include "rx_file.h"
#include "rx_flz.h"
#include "rx_threads.h"
#include "rx_tif.h"
#include "rx_trace.h"
#include "tcp_ip.h"
#include "gui_svr.h"
#include "args.h"
#include "spool_svr.h"
#include "ctrl_svr.h"
#include "print_queue.h"
#include "es_rip.h"
#include "rip_clnt.h"
#include "enc_ctrl.h"
#include "plc_ctrl.h"
#include "print_ctrl.h"
#include "machine_ctrl.h"
#include "fluid_ctrl.h"
#include "lh702_ctrl.h"
#include "step_ctrl.h"
#include "step_cleaf.h"
#include "step_std.h"
#include "chiller.h"
#include "prod_log.h"
#include "cleaf_orders.h"
#include "bmp.h"
#include "label.h"

//--- prototypes -----------------------
static void _on_error(ELogItemType type, char *deviceStr, int no, char *msg);
static void _set_src_size(SPrintQueueItem *pItem, const char *path);

static int _print_next(void);

//--- processes ----------------------
static void *_print_thread(void *lpParameter);
static int   _PrintThreadRunning;
static HANDLE _PrintSem;

//--- statics -----------------------------------------------------------------

#define MAX_PAGES	128

static SPrintQueueItem	_Item;
static INT32			_PageMargin_Next;
static int				_ChangeJob;
static int				_StopJob;
static char				_FilePathLocal[MAX_PATH];
static int				_Scanning;
static char				_DataPath[MAX_PATH];
static int				_ScanOffset;
static int				_ScanLengthPx;
static INT32			_PreloadCnt=0;
static int				_SetPrintPar = TRUE;
static int				_PrintGo;
static int				_PrintDone[MAX_PAGES];
static int				_PrintDoneFlags;
static int				_PrintDoneNo;
static int				_PrintGoNo;
static int				ERR_z_in_print;

//--- pc_init ----------------------------------------------------------------
int	pc_init(void)
{
	err_SetHandler(_on_error);

	memset(&RX_PrinterStatus, 0, sizeof(RX_PrinterStatus));
	memset(&_Item, 0, sizeof(_Item));
	memset(_FilePathLocal, 0, sizeof(_FilePathLocal));
	RX_PrinterStatus.printState = ps_off;
	if (chiller_is_running()) RX_PrinterStatus.printState = ps_ready_power;
	_PrintThreadRunning = TRUE;
	_PrintSem = rx_sem_create();
	rx_thread_start(_print_thread, NULL, 0, "_print_thread");
	
	return REPLY_OK;
}

//--- pc_end ------------------------------------------------------------------
int pc_end(void)
{
	_PrintThreadRunning = FALSE;
	return REPLY_OK;
}

//--- _on_error -----------------------------------------------------------------
static void _on_error(ELogItemType type, char *deviceStr, int no, char *msg)
{
	if (type==LOG_TYPE_UNDEF)
	{
		RX_PrinterStatus.error = LOG_TYPE_LOG;
		gui_send_printer_status(&RX_PrinterStatus);
	}
	else if (type > RX_PrinterStatus.error)
	{
		RX_PrinterStatus.error = type;

		switch (type)
		{
			case LOG_TYPE_LOG:			break;
			case LOG_TYPE_WARN:			break;
			case LOG_TYPE_ERROR_CONT:	break;
			case LOG_TYPE_ERROR_STOP:	if (!arg_simuPLC) 
										{
											Error(LOG, 0, "STOP Printing after LOG_TYPE_ERROR_STOP");
											pc_stop_printing(FALSE); 
										}
										break;
			case LOG_TYPE_ERROR_ABORT:	if (!arg_simuPLC) 
										{
											Error(LOG, 0, "STOP Printing after LOG_TYPE_ERROR_ABORT");
											pc_abort_printing(); 
										} break;
		default: break;
		}
		lh702_on_error(type, deviceStr, no, msg);
		gui_send_printer_status(&RX_PrinterStatus);
	}
}
	
//--- pc_start_printing -------------------------------------------------------
int pc_start_printing(void)
{	
	TrPrintfL(TRUE, "pc_start_printing printState=%d, (%d)", RX_PrinterStatus.printState, ps_ready_power);
	ERR_z_in_print = FALSE;
	if (RX_PrinterStatus.printState==ps_ready_power || RX_PrinterStatus.printState==ps_webin)
	{
		step_set_config();
		if (!RX_StepperStatus.info.z_in_print && 
			(RX_Config.printer.type==printer_TX801 
		  || RX_Config.printer.type==printer_TX802
		  || (RX_Config.printer.type==printer_LH702 && plc_in_simu())
			))
		{
			step_handle_gui_msg(INVALID_SOCKET, CMD_LIFT_PRINT_POS, NULL, 0);				
		}
		
		TrPrintfL(TRUE, "pc_start_printing: ref_done=%d", RX_StepperStatus.info.ref_done);		
		
		if (rx_def_is_lb(RX_Config.printer.type))
		{
			if(!RX_StepperStatus.info.ref_done)
			{
				TrPrintfL(TRUE, "pc_start_printing: CMD_LIFT_REFERENCE");		
				step_handle_gui_msg(INVALID_SOCKET, CMD_LIFT_REFERENCE, NULL, 0);					
			}
			else if (RX_StepperStatus.info.z_in_cap)
			{
				TrPrintfL(TRUE, "pc_start_printing: CMD_LIFT_UP_POS");		
				step_handle_gui_msg(INVALID_SOCKET, CMD_LIFT_UP_POS, NULL, 0);										
			}
		}
		_Scanning = rx_def_is_scanning(RX_Config.printer.type);
		_PreloadCnt = 5;
		memset(&_Item, 0, sizeof(_Item));
		err_clear_all();
		gui_send_cmd(REP_EVT_CONFIRM);
		ctrl_set_config();
		enc_set_config(FALSE);
		machine_reset();
		RX_PrinterStatus.dataReady = FALSE;
		RX_PrinterStatus.sentCnt=0;
		RX_PrinterStatus.printGoCnt=0;
		RX_PrinterStatus.printedCnt=0;
		RX_PrinterStatus.printState=ps_printing;
		_PrintGo = 0;
		_ChangeJob = FALSE;
		_StopJob   = FALSE;
		memset(_PrintDone, 0, sizeof(_PrintDone));
		_PrintDoneNo = 0;
		_PrintGoNo	 = 0;
		_PrintDoneFlags = spool_head_board_used_flags();
		_SetPrintPar   = TRUE;
//		fluid_start_printing();
		spool_start_printing();
		pq_start();
		pc_print_next();
		gui_send_printer_status(&RX_PrinterStatus);
	}
	else if (RX_PrinterStatus.printState==ps_pause || RX_PrinterStatus.printState==ps_printing)
	{
		RX_PrinterStatus.dataReady=TRUE;
		machine_start_printing();
		RX_PrinterStatus.printState=ps_printing;
		gui_send_printer_status(&RX_PrinterStatus);
	}
	return REPLY_OK;
}

//--- pc_stop_printing --------------------------------------------------------
int pc_stop_printing(int userStop)
{
	TrPrintfL(TRUE, "pc_stop_printing");
	
	if (RX_PrinterStatus.printState==ps_stopping 
	||  RX_PrinterStatus.printState==ps_goto_pause
	||  RX_PrinterStatus.printState==ps_pause
	||  RX_PrinterStatus.printState==ps_webin
	||  RX_PrinterStatus.printState==ps_ready_power
	|| (RX_PrinterStatus.printState==ps_printing && RX_PrinterStatus.printedCnt >= RX_PrinterStatus.sentCnt))
	{
		pc_abort_printing();
	}
	else if (RX_PrinterStatus.printState==ps_printing)
	{
		if (userStop) Error(ERR_CONT, 0, "STOPPED by Operator");
		RX_PrinterStatus.printState=ps_stopping;
		pq_stopping(&_Item);
		if (rx_def_is_tx(RX_Config.printer.type))
			_StopJob = TRUE;
		else
			enc_stop_pg("pc_stop_printing");
		gui_send_printer_status(&RX_PrinterStatus);
	}
	return REPLY_OK;
}

//--- pc_change_job ---------------------------------------------
int  pc_change_job(void)
{
	_ChangeJob = 1;
	return REPLY_OK;
}

//--- pc_off ------------------------------------------------------------
int pc_off(void)
{
	if (RX_PrinterStatus.printState==ps_stopping)
	{
		RX_PrinterStatus.printState=ps_off;
//		if (RX_Config.printer.type==printer_cleaf) step_handle_gui_msg(INVALID_SOCKET, CMD_LIFT_CAPPING_POS, NULL, 0);
		if (chiller_is_running()) RX_PrinterStatus.printState = ps_ready_power;
		RX_PrinterStatus.testMode   = FALSE;
		gui_send_printer_status(&RX_PrinterStatus);
	}
	return REPLY_OK;
}

//--- pc_abort_printing -----------------------------------------------------------
int pc_abort_printing(void)
{
	TrPrintfL(TRUE, "pc_abort_printing");
	ctrl_abort_printing();
	enc_abort_printing();
	RX_PrinterStatus.printState = ps_off;
	if (chiller_is_running()) RX_PrinterStatus.printState = ps_ready_power;
	RX_PrinterStatus.testMode   = FALSE;
	gui_send_printer_status(&RX_PrinterStatus);
	machine_abort_printing();
	pl_stop(&_Item);
	pq_abort();
	co_stop_printing();
	return REPLY_OK;	
}

//--- pc_pause_printing -----------------------------------------------------------
int pc_pause_printing(int fromGui)
{
	TrPrintfL(TRUE, "pc_pause_printing");
	RX_PrinterStatus.printState=ps_goto_pause;
	if (RX_PrinterStatus.printState==ps_printing) Error(LOG, 0, "PAUSE called by user");
//	if (RX_Config.printer.type == printer_LB702_UV) RX_PrinterStatus.printState = ps_pause;
	if (rx_def_is_lb(RX_Config.printer.type)) RX_PrinterStatus.printState = ps_pause;
	gui_send_printer_status(&RX_PrinterStatus);
	enc_stop_pg("pc_pause_printing");
	machine_pause_printing(fromGui);
	return REPLY_OK;	
}

//--- pc_print_next --------------------------------------------------------------
int pc_print_next()
{
	rx_sem_post(_PrintSem);
	return REPLY_OK;
}

//--- _send_head_info -----------------------------------------------------------
static void _send_head_info(void)
{
	int color, n, headNo, len, bad;
	char str[MAX_TEST_DATA_SIZE];
	char time[64];
	SHeadEEpromInfo *pinfo;
	SHeadStat		*pstat;
	FILE		*file=NULL;

	if (_SetPrintPar) file = rx_fopen(PATH_TEMP "printheads.txt", "wb", _SH_DENYNO);
	
	rx_get_system_time_str(time, '-');

	for (color=0, headNo=0; color<RX_Config.colorCnt; color++)
	{
		for (n=0; n<RX_Config.headsPerColor; n++, headNo++)
		{
			pstat = &RX_HBStatus[headNo/MAX_HEADS_BOARD].head[headNo%MAX_HEADS_BOARD];
			pinfo = &pstat->eeprom;
			len = 0;
			len += sprintf(&str[len], "%s\n", RX_TestImage.testMessage);
			len += sprintf(&str[len], "%s-%d                     %s\n", RX_ColorNameShort(color), n+1, time);
			
			double ml=(double)RX_HBStatus[headNo/MAX_HEADS_BOARD].head[headNo%MAX_HEADS_BOARD].printedDroplets;
			ml *= 1000000000;
			ml *= RX_HBStatus[headNo/MAX_HEADS_BOARD].head[headNo%MAX_HEADS_BOARD].dropVolume;

			len += sprintf(&str[len], "cl# %06d  printed %12s l\n", RX_HBStatus[headNo/MAX_HEADS_BOARD].clusterNo, value_str3((int)(1000.0*ml)));
			len += sprintf(&str[len], "s# %d-%02d\n", pinfo->serialNo/100, pinfo->serialNo%100);
			len += sprintf(&str[len], "volt %d / straight %d / uniform %d\n", pinfo->voltage, pinfo->straightness, pinfo->uniformity);
			len += sprintf(&str[len], "bad");
			for (bad=0; bad<SIZEOF(pinfo->badJets) && pinfo->badJets[bad]; bad++)
			{
				len += sprintf(&str[len], "  %d", pinfo->badJets[bad]); 
			}
			len += sprintf(&str[len], "\n");
			len += sprintf(&str[len], "Dots=%s / Men=%d.%d / Pump=%d.%d\n", RX_TestImage.dots, pstat->meniscus/10, abs(pstat->meniscus)%10, pstat->pumpFeedback/10, pstat->pumpFeedback%10);
			if (RX_TestImage.testImage==PQ_TEST_DENSITY) 
			{
				len += sprintf(&str[len], "Density Correction: volt=%d%%\n", pstat->eeprom_mvt.voltage);
				for (int i=0; i<MAX_DENSITY_VALUES; i++)
				{
					len += sprintf(&str[len], "%d  ", pstat->eeprom_mvt.densityValue[i]);			
				}
			len += sprintf(&str[len], "\n");
			}
//			printf("TestData[%d]: >>%s<<\n", n, str);
			spool_send_test_data(headNo, str);
			if (file) fprintf(file, "%s\n", str);
		}
	}
	if (file) fclose(file);
}

//--- _load_test -----------------------------------------------------------------
static void _load_test(void)
{
	UINT32 width, height, memsize;
	UINT8  bitsPerPixel;
	int ret;

	// see also gui_msg.c::_do_test_start
	if (((int)RX_PrinterStatus.sentCnt) < (int)(RX_TestImage.copies*RX_TestImage.scans))
	{
		if (_Scanning)	RX_TestImage.id.scan++;
		else			RX_TestImage.id.copy++;
		if (RX_TestImage.id.copy==0) RX_TestImage.id.copy=1;
		ret=bmp_get_size(RX_TestImage.filepath, &width, &height, &bitsPerPixel, &memsize);
		if (ret) ret=tif_get_size(RX_TestImage.filepath, 0, 0, &width, &height, &bitsPerPixel);
		if (ret) ret=flz_get_size(RX_TestImage.filepath, 0, 0, &width, &height, &bitsPerPixel);
		RX_TestImage.pageHeight = RX_TestImage.srcHeight = (UINT32)(height/1200.0*25400.0);
		RX_TestImage.pageWidth  = RX_TestImage.srcWidth  = (UINT32)(width/1200.0*25400.0);
		RX_TestImage.printGoMode = PG_MODE_LENGTH;
		RX_TestImage.printGoDist = (UINT32)(height/1200.0*25400.0);
		if (RX_TestImage.testImage==PQ_TEST_ANGLE_SEPARATED)
		{
			if (rx_def_is_tx(RX_Config.printer.type)) RX_TestImage.printGoDist=50000;
			else									  RX_TestImage.printGoDist=50000*RX_Config.inkSupplyCnt;
		}
		else if (!rx_def_is_scanning(RX_Config.printer.type) && !rx_def_is_lb(RX_Config.printer.type) && RX_TestImage.testImage!=PQ_TEST_GRID && RX_TestImage.testImage!=PQ_TEST_ANGLE_OVERLAP) 
			RX_TestImage.printGoDist *= (RX_Config.colorCnt+1);
		if (bitsPerPixel>1) strcpy(RX_TestImage.dots, "SML");
		_send_head_info();
	//	spool_print_file(&RX_TestImage.id, RX_TestImage.filepath, 0, height, PG_MODE_GAP, 0, PQ_LENGTH_COPIES, 0, RX_TestImage.scanMode, 1, TRUE);
		{
			SPrintQueueItem item;
			int clearBlockUsed;
			memset(&item, 0, sizeof(item));
			item.printGoMode = PG_MODE_GAP;
			item.lengthUnit  = PQ_LENGTH_COPIES;
			item.scanMode    = RX_TestImage.scanMode;
			item.srcPages    = 1;
			clearBlockUsed	 = (RX_TestImage.testImage==PQ_TEST_SCANNING) || ((int)RX_PrinterStatus.sentCnt+1) >= (int)(RX_TestImage.copies*RX_TestImage.scans);
			spool_print_file(&RX_TestImage.id, RX_TestImage.filepath, 0, height, &item, clearBlockUsed);	
		}
	}
}

//--- _get_image_size --------------------------------------------------------
static int _get_image_size(UINT32 gap)
{
	UINT32 length;

	if (flz_get_size(_DataPath, 0, gap, &length, NULL, NULL)==REPLY_OK) return length;
	if (tif_get_size(_DataPath, 0, gap, &length, NULL, NULL)==REPLY_OK) return length;

	UINT32 height, memSize;
	UINT8  bitsPerPixel;
	char path[MAX_PATH];
	bmp_color_path(_DataPath, RX_ColorNameShort(0), path);
	if (bmp_get_size(path, (UINT32*) &length, &height, &bitsPerPixel, &memSize)==REPLY_OK) return length+gap;
	return 0;
}

//--- _set_src_size ---------------------------------------------------------------
static void _set_src_size(SPrintQueueItem *pItem, const char *path)
{
	UINT32 width, height;
	int ret;

	ret = flz_get_size(path, 0, 0, &width, &height, NULL);
	if (ret) ret = tif_get_size(path, 0, 0, &width, &height, NULL);
	if (ret) 
	{
		UINT32 height, memSize;
		UINT8  bitsPerPixel;
		char p[MAX_PATH];
		bmp_color_path(path, RX_ColorNameShort(0), p);
		ret = bmp_get_size(p, (UINT32*) &width, &height, &bitsPerPixel, &memSize);
	}
	if (ret==REPLY_OK)
	{
		pItem->srcWidth  = width *25400/1200;
		pItem->srcHeight = height*25400/1200;
	}
}

//--- _local_path  -----------------------------------------------------
static void _local_path(const char *global, char *local)
{
	int pos;
	char *ch;
	char sharename[MAX_PATH], str[MAX_PATH];
	
	if ((pos=str_start(global, PATH_RIPPED_DATA_DRIVE)))
		sprintf(local, "//%s/%s%s", RX_CTRL_MAIN, PATH_RIPPED_DATA_DIR,  &global[pos]);
	else strcpy(local, global);
	for (ch=local; *ch; ch++) if(*ch=='\\') *ch='/';
	
	sprintf(sharename, "//%s/%s", RX_CTRL_MAIN, PATH_RIPPED_DATA_DIR);
	if ((pos=str_start(local, sharename)))
	{	
		strcpy(str, &local[pos]);
		sprintf(local, "%s%s", PATH_RIPPED_DATA, str);
	}
}

//--- pc_del_file -------------------------------------
void pc_del_file(char *path)
{
	char localPath[MAX_PATH];
	_local_path(path, localPath);
	rx_remove_old_files(localPath, 0);
	int ret=rx_rmdir(localPath);
}

//--- pc_set_pageMargin ------------------------------------
void pc_set_pageMargin(INT32 pageMargin)
{
	_PageMargin_Next = 	pageMargin;
}

//--- _filename ------------------------------------
static char* _filename(char *path)
{
	char ripped_data[32];
	int pos, len;
	strcpy(ripped_data, PATH_RIPPED_DATA_DIR);
	len = strlen(ripped_data);
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

//--- _print_next --------------------------------------------------------------
static int _print_next(void)
{
	static int _first;
	static int _ScansNext;
	static int _CopiesStart;
	TrPrintfL(TRUE, "_print_next printState=%d, spooler_ready=%d, pq_ready=%d", RX_PrinterStatus.printState, spool_is_ready(), pq_is_ready());
	while ((RX_PrinterStatus.printState==ps_printing || RX_PrinterStatus.printState==ps_pause || (_Scanning&&RX_PrinterStatus.printState==ps_stopping)) && spool_is_ready() && pq_is_ready())
	{	
		if (RX_PrinterStatus.testMode)
		{
            if (rx_def_is_lb(RX_Config.printer.type))
            {
                RX_TestImage.copies *= RX_TestImage.scans;
                RX_TestImage.scans = 1;
			}
            
			switch (RX_TestImage.testImage)
			{
				case PQ_TEST_ANGLE_OVERLAP:  strcpy(RX_TestImage.filepath, PATH_BIN_SPOOLER "angle.tif");
											 RX_TestImage.scansTotal = RX_TestImage.copies;
											 break;
				case PQ_TEST_ANGLE_SEPARATED:strcpy(RX_TestImage.filepath, PATH_BIN_SPOOLER "angle.tif");	
											 RX_TestImage.scansTotal = RX_TestImage.copies;
											 break;
				case PQ_TEST_JETS:			 strcpy(RX_TestImage.filepath, PATH_BIN_SPOOLER "fuji.tif");
											 if (RX_Config.printer.type==printer_TX801 
											 ||  RX_Config.printer.type==printer_TX802 
											 ||  RX_Config.printer.type==printer_test_table) 
												 RX_TestImage.scansTotal = RX_TestImage.copies;
											 break;
				case PQ_TEST_JET_NUMBERS:	 strcpy(RX_TestImage.filepath, PATH_BIN_SPOOLER "jet_numbers.tif");
											 if (RX_Config.printer.type==printer_TX801 
											 ||  RX_Config.printer.type==printer_TX802 
											 ||  RX_Config.printer.type==printer_test_table) 
												 RX_TestImage.scansTotal = RX_TestImage.copies;	
											 break;
				case PQ_TEST_GRID:			 strcpy(RX_TestImage.filepath, PATH_BIN_SPOOLER "grid.tif");	
											 RX_TestImage.scansTotal = RX_TestImage.copies;
											 break;
				case PQ_TEST_ENCODER:		 strcpy(RX_TestImage.filepath, PATH_BIN_SPOOLER "encoder.tif"); 
											 RX_TestImage.scansTotal = RX_TestImage.copies;
											 break;
				case PQ_TEST_SCANNING:		 sprintf(RX_TestImage.filepath, PATH_BIN_SPOOLER "scanning_%d.tif", RX_TestImage.id.scan+1);
											 RX_TestImage.copies=1;	
											 RX_TestImage.scans=RX_TestImage.scansTotal=8;
											 break;
				case PQ_TEST_FULL_ALIGNMENT: strcpy(RX_TestImage.filepath, PATH_BIN_SPOOLER "FullAlignement.tif");
											 RX_TestImage.scansTotal = RX_TestImage.copies;
											 break;
				case PQ_TEST_DENSITY:		 // preparing the bitmap:
											 // 1. Draw bitmap as 24-Bit BMP in paint
											 // 2. Open it in IrfanView
											 // 3. Menu Image/Information change to 1200x1200 DPI
											 // 4. Save it as TIF
											 // 5. Rip it in Mouvent DFE
											 // 6. Copy the black image to the binary
											 strcpy(RX_TestImage.filepath, PATH_BIN_SPOOLER "density.flz");
											 RX_TestImage.scansTotal = RX_TestImage.copies;
											 break;
				default:					 strcpy(RX_TestImage.filepath, PATH_BIN_SPOOLER "fuji.tif");	break;
			}
			RX_TestImage.collate     = FALSE;
			RX_TestImage.copiesTotal = RX_TestImage.copies;
			_first=(RX_TestImage.id.copy==0) && (RX_TestImage.id.scan==0);
			_load_test();
			if (_first)
			{
				if (_PreloadCnt>RX_TestImage.scans) _PreloadCnt = RX_TestImage.scans;
				machine_set_printpar(&RX_TestImage);
				enc_sent_document(RX_TestImage.copiesTotal, &RX_TestImage.id);
				pq_sent_document(RX_TestImage.copiesTotal);
			}
			return REPLY_OK;
		}
		
		//---  load next ---
		if (*_Item.filepath==0) 
		{
			memset(&_Item, 0, sizeof(_Item));
			*_DataPath	  = 0;
			_ScanLengthPx = 0;
			_CopiesStart  = 0;
			SPrintQueueItem *item = pq_get_next_item();
			if (item) 
			{
				pq_trace_item(item);

				memcpy(&_Item, item, sizeof(_Item));
				_first		  = TRUE;
				_Item.scansStop = 0;

				_local_path(_Item.filepath, _FilePathLocal);
				
				_Item.id.copy = _Item.start.copy;
				if (_Item.copies==0) _Item.copies=1; 
		//		if (_Item.id.copy<1) _Item.id.copy=1;
				if (_Item.start.page < _Item.firstPage) _Item.id.page = _Item.firstPage;
				else _Item.id.page = _Item.start.page;
				_Item.id.scan=1;

				if(!_Scanning)
				{
					if(!_Item.variable && _Item.lengthUnit==PQ_LENGTH_MM && _Item.pageHeight) 
					{
						_Item.copies  = _Item.copiesTotal = (int)((_Item.scanLength*1000.0+_Item.pageHeight-1) / _Item.pageHeight);
						_Item.id.copy = _Item.copiesPrinted+1;						
						if (_Item.id.copy==0) _Item.id.copy=1;
					}
					else 
					{
						TrPrintfL(TRUE,"PRINT-NEXT: ID=%d >>%s<<, copies=%d, printed=%d, pages=(%d..%d)", _Item.id.id, _Item.filepath,_Item.copies,_Item.copiesPrinted,_Item.firstPage,_Item.lastPage);
						if (_Item.start.copy>0) _Item.copiesPrinted = _CopiesStart = _Item.start.copy-1;
						_Item.copiesTotal = _Item.copies * (_Item.lastPage - _Item.firstPage + 1);
					}
				//	Error(LOG, 0, "Printig1: copy=%d, copies=%d", _Item.id.copy, _Item.copies);
					co_start_printing(_Item.pageHeight);
				}

				if(_Item.lengthUnit == PQ_LENGTH_COPIES)
				{
					if (_Item.id.copy<1) _Item.id.copy=1;
					if(rx_def_use_pq(RX_Config.printer.type) && (_Item.id.copy > _Item.copies))
					{
                        int id=_Item.id.id;
						memset(&_Item, 0, sizeof(_Item));
						return Error(ERR_ABORT, 0, "ID=%d: Invalid copy settings", id);
					}
					_Item.copiesTotal = _Item.copies * (_Item.lastPage - _Item.firstPage + 1);
				}
							
				if(_Item.copiesTotal < 1)_Item.copiesTotal = 1;

				if (_Item.variable)
				{
					label_load(&_Item, _FilePathLocal);
				}

//				Error(LOG, 0, "Printig2: copies=%d, copy=%d, copiesTotal=%d", _Item.copies, _Item.id.copy, _Item.copiesTotal);
				if (rx_def_is_lb(RX_Config.printer.type)) Error(LOG, 0, "%d: %s: Started, %d copies", _Item.id.id, _filename(_Item.filepath), _Item.copies);
				TrPrintfL(TRUE, "PRINT-NEXT: >>%s<<, copiesTotal=%d", _Item.filepath, _Item.copiesTotal);
	//			Error(LOG, 0, "copiesTotal=%d", _Item.copiesTotal);
				_Item.scans=0; 
				_Item.scansSent=0;
				_PageMargin_Next = _Item.pageMargin;
				_CopiesStart = _Item.copiesPrinted;
				if (RX_Config.printer.type==printer_DP803 && _Item.lastPage!=_Item.firstPage)
					_CopiesStart = (_Item.copiesPrinted* (_Item.lastPage - _Item.firstPage + 1) + _Item.start.page)-1;

				_set_src_size(&_Item, _FilePathLocal);

				pq_set_item(&_Item);
				pl_start(&_Item, _FilePathLocal);
				if (_SetPrintPar)
				{
					_SetPrintPar = FALSE;
					machine_set_printpar(&_Item);
				}
				
				//--- check print-go Mode ----
				if (_Item.printGoMode==PG_MODE_LENGTH)
				{
					if (_Scanning)
					{
						if (_Item.printGoDist < (_Item.srcWidth*90)/100)
						{
							Error(ERR_ABORT, 0, "Print to Print Distance (%d mm) too short (min=%d mm)", _Item.printGoDist/1000, (_Item.srcWidth*90)/100000);						
							memset(&_Item, 0, sizeof(_Item));
							pc_abort_printing();
						}
						else if (_Item.printGoDist < (_Item.srcWidth*95)/100) 
							Error(WARN, 0, "Print to Print Distance (%d mm) short", _Item.printGoDist/1000);							
					}
					else
					{							
						if (_Item.printGoDist < (_Item.srcHeight*90)/100)
						{
							Error(ERR_ABORT, 0, "Print to Print Distance (%d mm) too short (min=%d mm)", _Item.printGoDist/1000, (_Item.srcHeight*90)/100000);						
							memset(&_Item, 0, sizeof(_Item));
							pc_abort_printing();
						}
						else if (_Item.printGoDist < (_Item.srcHeight*95)/100) 
							Error(WARN, 0, "Print to Print Distance (%d mm) short", _Item.printGoDist/1000);	
					}
				}

				if (RX_Config.printer.type==printer_DP803) Error(LOG, 0, "Start Printing: >>%s<<, copiesTotal=%d, speed=%d m/min", _Item.filepath, _Item.copiesTotal, _Item.speed);
			}
		}
		if (*_Item.filepath) 
		{
			if (RX_Config.printer.type==printer_test_slide || RX_Config.printer.type==printer_test_slide_only)
			{
				_Item.scansPrinted	= 0;
				_Item.start.copy	= 0;
				_Item.copiesPrinted	= 0;
				_Item.id.scan		= 0;
				_Item.id.copy++;
			}
			else
			{
				if (_first) TrPrintfL(TRUE, "NEW FILE id=%d, page=%d, copy=%d, scan=%d", _Item.id.id, _Item.id.page, _Item.id.copy, _Item.id.scan);
				if (!_first) pq_next_page(&_Item, &_Item.id);
				TrPrintfL(TRUE, "pq_next_page id=%d, page=%d, copy=%d, scan=%d", _Item.id.id, _Item.id.page, _Item.id.copy, _Item.id.scan);
				if (_Item.id.copy>_Item.copies || _Item.id.page>_Item.lastPage || _ChangeJob==2)
				{
					Error(LOG, 0, "Document sent >>%s<<, copiesTotal=%d, _CopiesStart=%d, _ChangeJob=%d", _Item.filepath, _Item.copiesTotal, _CopiesStart, _ChangeJob);
					_ChangeJob = 0;
				//	Error(LOG, 0, "enc_sent_document, _Item.copiesTotal=%d, _CopiesStart=%d, _TotalPgCnt=%d", _Item.copiesTotal, _CopiesStart, _Item.copiesTotal-_CopiesStart);
					if (_Scanning && arg_simuEncoder)	
					{
						enc_sent_document(_Item.scans, &_Item.id);
						pq_sent_document(_Item.scans);
					}
					else if (RX_Config.printer.type==printer_LH702)
					{
						enc_sent_document(_Item.id.copy-_CopiesStart, &_Item.id);
						pq_sent_document(_Item.id.copy-_CopiesStart);							
					}
					else
					{
						enc_sent_document(_Item.copiesTotal-_CopiesStart, &_Item.id);
						pq_sent_document(_Item.copiesTotal-_CopiesStart);
					}
					TrPrintf(TRUE, "Document sent >>%s<<, copiesTotal=%d, _CopiesStart=%d", _Item.filepath, _Item.copiesTotal, _CopiesStart);
					memset(&_Item, 0, sizeof(_Item));
					return REPLY_OK;				
				}					
			}
			_first = FALSE;

			//--- check all printed ------
			/*
			{
				int n=_PrintGo % SIZEOF(_PrintDone);
				if (_PrintDone[n]) 
				{
					Error(WARN, 0, "PRINTGO[%d]: PrintDone[%d] still %d missing!", _PrintGo, _PrintGo-SIZEOF(_PrintDone), _PrintDone[n]);
					pc_abort_printing();
					return REPLY_ERROR;
				}
				_PrintGo++;
			}
			*/
			//----
				
			TrPrintfL(TRUE, "scan=%d, scans=%d, collate=%d, copies=%d", _Item.id.scan, _Item.scans, _Item.collate, _Item.copies);
			if (_Scanning && *_DataPath) // && _Item.id.scan<_Item.scans)
			{				
				if (_Item.passes)
				{
					if(_Item.id.scan==0)
					{
						if(RX_Config.printer.overlap) _ScanOffset = -RX_Spooler.headOverlapPx;
						else						  _ScanOffset = 0;
						_Item.scans	= _ScansNext;
					}
					else
					{
						if (RX_Config.printer.overlap)  _ScanOffset += (RX_Spooler.barWidthPx+RX_Spooler.headOverlapPx) / _Item.passes;
						else                            _ScanOffset += RX_Spooler.barWidthPx / _Item.passes;							
					}
				}
				if (_StopJob)
				{
					_StopJob = FALSE;
					_ScanLengthPx = _ScanOffset;
					SPrintQueueItem *pitem =pq_get_item(&_Item);
					int barwidth=RX_Spooler.barWidthPx;
					if (RX_Config.printer.overlap) barwidth += RX_Spooler.headOverlapPx;
					pitem->scansStop = (INT32)((_ScanLengthPx+RX_Spooler.maxOffsetPx+barwidth-1) / (barwidth/_Item.passes));
					Error(LOG, 0, "_print_next:_StopJob");
					pq_set_item(pitem);
				}
				spool_print_file(&_Item.id, _DataPath, _ScanOffset, _ScanLengthPx, &_Item, TRUE);
				return REPLY_OK;
			}
			{
				strcpy(_DataPath, _FilePathLocal);

				TrPrintfL(TRUE, "send >>%s<<, page=%d, copy=%d", _FilePathLocal, _Item.id.page, _Item.id.copy);
				if (_Scanning) 
				{
					if (_Item.scans==0)
					{				
						double length;
						
						// _ScanLengthPx = length to print
						if (_Item.lengthUnit==PQ_LENGTH_COPIES)
						{
							int copies    = _Item.scanLength/1000;
							if (_Item.start.copy<1) _Item.start.copy=1;
							_Item.copiesPrinted = _CopiesStart = _Item.start.copy-1;									
							length        = _get_image_size((UINT32)(_Item.printGoDist * 1.200 / 25.4))*copies;
							_ScanLengthPx = _get_image_size((UINT32)(_Item.printGoDist * 1.200 / 25.4))*(copies-_Item.copiesPrinted);									
							_Item.id.copy = _Item.start.copy;
						}
						else 
						{
							double l		  = _Item.scanLength;
							length        = (UINT32)(l * 1200 / 25.4+0.5);
							l			  = l - 1000*_Item.start.scan;
							_ScanLengthPx = (UINT32)(l * 1200 / 25.4+0.5);
							_Item.id.copy = 1;
							if (_Item.copies!=1) Error(WARN, 0, "Here was the bug: _Item.copies=%d", _Item.copies);
							_Item.copies  = 1;
						}

						int barwidth;
						if(RX_Config.printer.overlap) 
						{
						//	if (_Item.srcPages>1) return Error(ERR_ABORT, 0, "Multi Page Files only allowed in NO OVERLAP mode");

							barwidth	= RX_Spooler.barWidthPx + RX_Spooler.headOverlapPx;
							_ScanOffset	= -RX_Spooler.headOverlapPx;
						}
						else
						{
							barwidth	= RX_Spooler.barWidthPx;
							_ScanOffset = 0;								
						}
						if (_Item.passes<1) _Item.passes=1;
						if (_Item.passes>1) _ScanOffset	-= barwidth*(_Item.passes-1) / _Item.passes;

						_Item.scansStart	= (RX_Spooler.maxOffsetPx+barwidth-1) / (barwidth/_Item.passes);
						_Item.scans			= (INT32)((length+RX_Spooler.maxOffsetPx+barwidth-1) / (barwidth/_Item.passes));								
					//	_ScansNext			= (length+RX_Spooler.maxOffsetPx%barwidth+barwidth-1) / (barwidth/_Item.passes);								
						_ScansNext			= _Item.scans-_Item.scansStart;
						_Item.scansPrinted	= (INT32)(((length-_ScanLengthPx)+(barwidth/_Item.passes)-1) / (barwidth/_Item.passes));
						if (_Item.lengthUnit==PQ_LENGTH_COPIES)
							_Item.scansTotal    = _Item.scans;
						else
							_Item.scansTotal    = _Item.scansStart + _Item.copiesTotal*(_ScansNext+1);
						if(_Item.srcPages > 1)		
						{
							if(_ScansNext < 0) return Error(ERR_ABORT, 0, "File too short for Multi Page printing");
							_Item.scansTotal  = _Item.scansStart + (_Item.lastPage-_Item.start.page+1)*(_ScansNext+1)-1;
						}
						_Item.id.scan		= _Item.scansPrinted+1;								
												
						machine_set_scans(_Item.scans);
						pq_set_item(&_Item);
						pl_start(&_Item, _FilePathLocal);
					}
					spool_print_file(&_Item.id, _DataPath, _ScanOffset, _ScanLengthPx, &_Item, TRUE);
				}
				else
				{
					if (RX_Config.printer.type==printer_cleaf)
					{
					//	spool_print_file(&_Item.id, _DataPath, _Item.pageMargin, 0, _Item.printGoMode, _Item.printGoDist, PQ_LENGTH_UNDEF, _Item.variable, _Item.scanMode, _Item.srcPages, FALSE);
						SPrintQueueItem item;
						memcpy(&item, &_Item, sizeof(item));
						item.lengthUnit = PQ_LENGTH_UNDEF;
						spool_print_file(&_Item.id, _DataPath, _Item.pageMargin, 0, &item, FALSE);
					}
					else
					{
						int img_offset=_Item.pageMargin + _Item.pageWidth;
						int bar_width=RX_Spooler.barWidthPx*25400/1200;
						int clearBlockUsed=(_Item.id.copy >= _Item.copies) || (_Item.firstPage!=_Item.lastPage) || (_Item.pageMargin!=_PageMargin_Next);
						if (img_offset>bar_width && RX_Config.printer.type!=printer_LH702) img_offset = bar_width;
						if (_ChangeJob==1)
						{	
							Error(LOG, 0, "PrintCtrl: New Job: copy=%d, scansSent=%d", _Item.id.copy, _Item.scansSent);
							clearBlockUsed = TRUE;
							_ChangeJob = 2;
							SPrintQueueItem *pitem =pq_get_item(&_Item);
							pitem->copiesTotal = _Item.id.copy;
							pq_set_item(pitem);									
						}

						SPrintQueueItem item;
						memcpy(&item, &_Item, sizeof(item));
						item.lengthUnit = PQ_LENGTH_UNDEF;
						if (_Item.variable) label_send_data(&_Item.id);
						spool_print_file(&_Item.id, _DataPath, img_offset, 0, &item, clearBlockUsed);
						_Item.pageMargin=_PageMargin_Next;
					}
				}
				return REPLY_OK;
			}		
		}
		else 
		{	//--- nothing to print --------------------
			if (_PreloadCnt && RX_PrinterStatus.sentCnt)
			{
				RX_PrinterStatus.dataReady = TRUE;
				TrPrintfL(TRUE, "_print_next");
				machine_start_printing();
				gui_send_printer_status(&RX_PrinterStatus);
				_PreloadCnt = 0;
			}
			RX_PrinterStatus.testMode = FALSE;
			return REPLY_OK;	
		}
	}
	return REPLY_OK;
}

//--- _print_thread ---------------------------------------------
static void *_print_thread(void *lpParameter)
{
	while (_PrintThreadRunning)
	{
		if (rx_sem_wait(_PrintSem, 1000)==REPLY_OK)
		{
			if (!RX_PrinterStatus.externalData) _print_next();
		}
	}
	return NULL;
}

//--- pc_sent --------------------------------------------
int pc_sent(SPageId *id)
{
//	Error(LOG, 0, "Data Sent id=%d, page=%d, scan=%d, copy=%d", id->id, id->page, id->scan, id->copy);
	TrPrintfL(TRUE, "**** SENT id=%d, page=%d, scan=%d, copy=%d ****  _PreloadCnt=%d", id->id, id->page, id->scan, id->copy, _PreloadCnt);
	
	if  (_PreloadCnt && !(--_PreloadCnt))
	{
		RX_PrinterStatus.dataReady = TRUE;
		TrPrintfL(TRUE, "pc_sent start machine");
		machine_start_printing();
		gui_send_printer_status(&RX_PrinterStatus);
	}
	return REPLY_OK;
}

//--- pc_print_done ------------------------------
int pc_print_done(int headNo, SPrintDoneMsg *pmsg)
{	
	int n=pmsg->pd%SIZEOF(_PrintDone);
	_PrintDone[n] |= (1<<headNo);
	
	TrPrintfL(TRUE, "Head[%d] PRINT-DONE: PD=%d: id=%d, page=%d, scan=%d, copy=%d **** (0x%08x/0x%08x)", headNo, pmsg->pd, pmsg->id.id, pmsg->id.page, pmsg->id.scan, pmsg->id.copy, _PrintDone[n], _PrintDoneFlags);	
	
	if (RX_Config.printer.type==printer_cleaf)
	{
		co_printed();
		if (!RX_StepperStatus.info.z_in_print) 
		{
			if (!ERR_z_in_print) Error(WARN, 0, "Printing while head not in print position");
			ERR_z_in_print = TRUE;
		}
	}
	
	if (_PrintDone[n] == _PrintDoneFlags)
	{
		SPrintQueueItem *pnext;
		int pageDone, jobDone;
        static int _LastTime;
		int ticks=rx_get_ticks();
        int time;
        
        if (_PrintDoneNo==0) time=0;
        else time=ticks-_LastTime;
        _LastTime = ticks;
        
		TrPrintf(TRUE, "*** PRINT-DONE #%d *** (id=%d, page=%d, scan=%d, copy=%d) sent=%d, printed=%d, stopping=%d, time=%d", ++_PrintDoneNo, pmsg->id.id, pmsg->id.page, pmsg->id.scan, pmsg->id.copy, RX_PrinterStatus.sentCnt, RX_PrinterStatus.printedCnt, RX_PrinterStatus.printState==ps_stopping, time);

		_PrintDone[n] = 0;
 
		pq_printed(headNo, &pmsg->id, &pageDone, &jobDone, &pnext);
		TrPrintf(TRUE, "PRINTED pq_printed=%d, pageDone=%d, jobDone=%d, pnext=%d", RX_PrinterStatus.printedCnt, pageDone, jobDone, pnext);
		if (pageDone || jobDone)
		{
			RX_PrinterStatus.printedCnt++;

			if (RX_Config.printer.type==printer_test_slide || RX_Config.printer.type==printer_test_slide_only)
			{
				if (RX_PrinterStatus.sentCnt==RX_PrinterStatus.printedCnt) pc_abort_printing();
				return REPLY_OK;
			}
			else if (RX_Config.printer.type==printer_test_table)
			{
			//	if (arg_simuPLC) pc_abort_printing();

				if (RX_PrinterStatus.sentCnt==RX_PrinterStatus.printedCnt) 
					pc_abort_printing(); // curing!
				return REPLY_OK;
			}
			else
			{
				if (pnext) 
				{
//					if (!arg_simuEncoder && _Scanning) machine_set_printpar(pnext);
					if (_Scanning) machine_set_printpar(pnext);
					if(pnext->state < PQ_STATE_TRANSFER)
					{
						if (rx_def_is_tx(RX_Config.printer.type))
						{
							Error(LOG, 0, "file >>%s<< not loaded completely: PAUSE printing", _filename(pnext->filepath));
							machine_pause_printing(FALSE);
							_PreloadCnt = 5;								
						}
						else Error(WARN, 0, "file >>%s<< not loaded completely: HERE WAS THE BUG", _filename(pnext->filepath));
					}
				}
			}
			/*
			TrPrintfL(TRUE, "pc_print_done=TRUE, stopping=%d, printed=%d, sent=%d", RX_PrinterStatus.printState==ps_stopping, RX_PrinterStatus.printedCnt, RX_PrinterStatus.sentCnt);
			if (RX_PrinterStatus.printState==ps_stopping && RX_PrinterStatus.printedCnt>=RX_PrinterStatus.sentCnt)
			{
				Error(LOG, 0, "pc_print_done: sent=%d, printed=%d, scan=%d, scans=%d: STOP", RX_PrinterStatus.sentCnt, RX_PrinterStatus.printedCnt, _Item.id.scan, _Item.scans);
				enc_stop_printing();
				pc_stop_printing(FALSE);
			}
			*/
			if (jobDone && pnext==NULL)
			{
				TrPrintfL(TRUE, "pc_print_done: sent=%d, printed=%d, scan=%d, scans=%d: STOP", RX_PrinterStatus.sentCnt, RX_PrinterStatus.printedCnt, _Item.id.scan, _Item.scans);
			//	Error(LOG, 0, "pc_print_done: sent=%d, printed=%d, scan=%d, scans=%d: STOP", RX_PrinterStatus.sentCnt, RX_PrinterStatus.printedCnt, _Item.id.scan, _Item.scans);
				enc_stop_pg("pc_print_done");
				enc_stop_printing();
				pc_stop_printing(FALSE);

				/*
				SPrintQueueItem item;
				memcpy(&item.id, &pmsg->id, sizeof(item.id));
				pl_stop(&item);
				*/
			}
		}		
	}
	
//	TrPrintfL(TRUE, "pc_print_done: pc_print_next");
	pc_print_next();
	return REPLY_OK;
}

//--- pc_print_go -----------------------------------
void pc_print_go(void)
{
	if (_Scanning) 
	{
		SPageId *pid  = spool_get_id(RX_PrinterStatus.printGoCnt);
		SPageId *next = spool_get_id(RX_PrinterStatus.printGoCnt+1);
		
        plc_print_go(RX_PrinterStatus.printGoCnt);
	//	Error(LOG, 0, "PrintGo[%d] (id=%d, page=%d, scan=%d, copy=%d)", RX_PrinterStatus.printGoCnt, pid->id, pid->page, pid->scan, pid->copy);
	//	Error(LOG, 0, "NEXT   [%d] (id=%d, page=%d, scan=%d, copy=%d)", RX_PrinterStatus.printGoCnt, next->id, next->page, next->scan, next->copy);
		if (!RX_PrinterStatus.testMode && next->id != pid->id) 
		{
			if (!arg_simuEncoder) machine_pause_printing(FALSE);			
		}
	}
	else
	{
		SPageId *pid  = spool_get_id(RX_PrinterStatus.printGoCnt);
		TrPrintf(TRUE, "*** PRINT-GO #%d *** (id=%d, page=%d, scan=%d, copy=%d)", ++_PrintGoNo, pid->id, pid->page, pid->scan, pid->copy);
	}
}
 