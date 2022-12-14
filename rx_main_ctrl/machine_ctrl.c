// ****************************************************************************
//
//	machine_ctrl.c
//
// ****************************************************************************
//
//	Copyright 2016 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#include "rx_def.h"
#include "rx_error.h"
#include "rx_trace.h"
#include "tcp_ip.h"
#include "enc_ctrl.h"
#include "plc_ctrl.h"
#include "lh702_ctrl.h"
#include "chiller.h"
#include "step_ctrl.h"
#include "ctrl_svr.h"
#include "setup_assist.h"
#include "step_cleaf.h"
#include "fluid_ctrl.h"
#include "step_tts.h"
#include "drive_ctrl.h"
#include "machine_ctrl.h"


#define CAPPING_TIMEOUT	(20*60*1000)	// ms 
static int		_CappingTimer=0;

typedef enum 
{	
	mi_none,
	mi_plc,
	mi_tt,
	mi_tts,
} EMachineInterface;

static EMachineInterface _MInterface = mi_none;

//--- set_interface ----------------------------------
static void set_interface(void)
{
	switch(RX_Config.printer.type) 
	{
	case printer_test_slide:		_MInterface=mi_none;	break;
	case printer_test_slide_only:	_MInterface=mi_none;	break;
	case printer_test_slide_HB:		_MInterface=mi_none;	break;
    case printer_Dropwatcher:		_MInterface=mi_none;	break;
	case printer_test_table:		_MInterface=mi_tt;		break;
	case printer_test_table_seon:	_MInterface=mi_tts;		break;
	case printer_cleaf:				_MInterface=mi_plc;		break;
	case printer_TX801:				_MInterface=mi_plc;		break;
	case printer_TX802:				_MInterface=mi_plc;		break;
	case printer_TX404:				_MInterface=mi_plc;		break;
	case printer_LB701:				_MInterface=mi_plc;		break;
	case printer_LB702_UV:			_MInterface=mi_plc;		break;
	case printer_LB702_WB:			_MInterface=mi_plc;		break;
	case printer_LH702:				_MInterface=mi_plc;		break;
	case printer_DP803:				_MInterface=mi_plc;		break;
	default:						_MInterface=mi_none;
	}
}

//--- machine_init ------------------------------
int	machine_init(void)
{
	plc_init();
    drive_init();
	lh702_init();
	set_interface();
    return REPLY_OK;
}

//--- machine_end --------------------------------
int	machine_end(void)
{
	plc_end();
	lh702_end();
	return REPLY_OK;
}

//--- machine_tick -------------------------------
int machine_tick(void)
{
	if(rx_def_is_tx(RX_Config.printer.type) || RX_StepperStatus.cln_used)
	{
		if (RX_StepperStatus.info.z_in_cap || !RX_StepperStatus.robinfo.auto_cap) machine_set_capping_timer(FALSE);  // reset timer if already in cap
        else if (plc_get_state() == plc_error || ((RX_StepperStatus.info.z_in_screw || RX_StepperStatus.info.z_in_exchange) && _CappingTimer)) machine_set_capping_timer(TRUE);	// start timer as long as machine is in error state

		if (_CappingTimer>0 && _CappingTimer<rx_get_ticks() && !RX_PrinterStatus.door_open && !RX_StepperStatus.info.moving && !RX_StepperStatus.robinfo.moving && !RX_StepperStatus.screwerinfo.moving)
		{
			_CappingTimer=0;
			Error(LOG, 0, "Setting printhead to capping position. CtrlMode=%d %d", fluid_get_ctrlMode(0),fluid_get_ctrlMode(1));
			step_set_autocapMode(TRUE);
            fluid_send_ctrlMode(-1, ctrl_cap, TRUE);
		}
	}
	return REPLY_OK;
}

//--- machine_reset --------------------------------
void machine_reset(void)
{
	plc_reset();
}

//--- machine_error_reset -------------------------
void machine_error_reset(void)
{
	chiller_error_reset();
	plc_error_reset();
	lh702_error_reset();
	fluid_error_reset();
	ctrl_head_error_reset();
	step_error_reset();
	enc_error_reset();
    tts_error_reset();
    drive_error_reset();
}

//--- machine_set_printpar -----------------------
int	machine_set_printpar(SPrintQueueItem *pItem)
{
	TrPrintfL(TRUE, "machine_set_printpar");
	set_interface();

	ctrl_send_firepulses(pItem->dots, (pItem->srcBitsPerPixel==8));

	if (RX_Config.printer.type==printer_LH702) lh702_set_printpar(pItem);
		
	switch(_MInterface) 
	{
	case mi_none:	return enc_start_printing (pItem, FALSE);
	case mi_tt:		return tt_set_printpar(pItem);
	case mi_tts:	return drive_set_printbar(pItem);
        //return enc_start_printing(pItem, FALSE); // drive_set_printbar(pItem);
	case mi_plc:	return plc_set_printpar(pItem);
	}
	return REPLY_OK;
}

//--- machine_set_scans ----------------------------------
int	machine_set_scans(int scans)
{
	switch(_MInterface) 
	{
	case mi_none:	return REPLY_OK;
	case mi_tt:		return tt_set_scans(scans);
	case mi_tts:	return REPLY_OK;
	case mi_plc:	return REPLY_OK;
	}
	return REPLY_OK;
}


//--- machine_get_scanner_pos ---------------------
UINT32 machine_get_scanner_pos(void)
{
	switch(_MInterface) 
	{
	case mi_none:	return REPLY_OK;
	case mi_tt:		return tt_get_scanner_pos();
	case mi_tts:
			return drive_get_scanner_pos();
	case mi_plc:	return plc_get_scanner_pos();
	}
	return REPLY_OK;
}

//--- machine_start_printing ----------------------
int	machine_start_printing(void)
{
	TrPrintfL(TRUE, "machine_start_printing printState=%d", RX_PrinterStatus.printState);
	machine_set_capping_timer(FALSE);

	switch(_MInterface) 
	{
	case mi_none:	return REPLY_OK;
	case mi_tt:		return tt_start_printing();
	case mi_tts:	return drive_start_printing();
	case mi_plc:	sa_to_print_pos();
					return plc_start_printing();
	}
	return REPLY_OK;
}

//--- machine_pause_printing ----------------------
int	machine_pause_printing(int fromGui)
{
	machine_set_capping_timer(TRUE);
	switch(_MInterface) 
	{
	case mi_none:	return REPLY_OK;
	case mi_tt:		return tt_pause_printing();
	case mi_plc:	return plc_pause_printing(fromGui);
	}
	return REPLY_OK;
}

//--- machine_stop_printing -----------------------
int	machine_stop_printing(void)
{
	machine_set_capping_timer(TRUE);
	switch(_MInterface) 
	{
	case mi_none:	return enc_stop_printing();
	case mi_tt:		return tt_stop_printing();
	case mi_tts:    return enc_stop_printing(); // drive_stop_printing();
	case mi_plc:	return plc_stop_printing();
	}
	return REPLY_OK;
}

//--- machine_abort_printing -----------------------
int	machine_abort_printing(void)
{
	machine_set_capping_timer(TRUE);
	switch(_MInterface) 
	{
	case mi_none:	return enc_abort_printing();
	case mi_tt:		return tt_abort_printing();
	case mi_tts:    return enc_abort_printing();
	case mi_plc:	step_abort_printing();
					return plc_abort_printing();
	}
	return REPLY_OK;
}

//--- machine_clean -------------------------------
int	machine_clean(void)
{
	switch(_MInterface) 
	{
	case mi_none:	return REPLY_OK;
	case mi_tt:		return tt_clean();
	case mi_tts:	return REPLY_OK;
	case mi_plc:	return plc_clean();
	}
	return REPLY_OK;
}

//--- machine_set_capping_timer -----------------------------------
void machine_set_capping_timer(int state)
{
	if (state == TRUE)
		_CappingTimer = rx_get_ticks() + CAPPING_TIMEOUT;
	else
		_CappingTimer = 0;
}