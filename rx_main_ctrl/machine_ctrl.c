// ****************************************************************************
//
//	machine_ctrl.h	
//
// ****************************************************************************
//
//	Copyright 2016 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#include "rx_def.h"
#include "tcp_ip.h"
#include "enc_ctrl.h"
#include "plc_ctrl.h"
#include "chiller.h"
#include "step_ctrl.h"
#include "ctrl_svr.h"
#include "step_cleaf.h"
#include "fluid_ctrl.h"

typedef enum 
{	
	mi_none,
	mi_plc,
	mi_tt,
} EMachineInterface;

static EMachineInterface _MInterface = mi_none;

//--- set_interface ----------------------------------
static void set_interface(void)
{
	switch(RX_Config.printer.type) 
	{
	case printer_test_slide:		_MInterface=mi_none;	break;
	case printer_test_slide_only:	_MInterface=mi_none;	break;
	case printer_test_table:		_MInterface=mi_tt;		break;
	case printer_cleaf:				_MInterface=mi_plc;		break;
	case printer_TX801:				_MInterface=mi_plc;		break;
	case printer_TX802:				_MInterface=mi_plc;		break;
	case printer_LB701:				_MInterface=mi_plc;		break;
	case printer_LB702_UV:			_MInterface=mi_plc;		break;
	case printer_LB702_WB:			_MInterface=mi_plc;		break;
	case printer_LH702:				_MInterface=mi_plc;		break;
	case printer_DP803:				_MInterface=mi_plc;		break;
	default:						_MInterface=mi_none;
	}
}

//--- machine_init ------------------------------
int		machine_init(void)
{
	plc_init();
	set_interface();
	return REPLY_OK;
}

//--- machine_end --------------------------------
int		machine_end(void)
{
	plc_end();
	return REPLY_OK;
}

//--- machine_tick -------------------------------
int		machine_tick(void)
{
	return REPLY_OK;
}


//--- machine_reset --------------------------------
void machine_reset(void)
{
	plc_reset();
}

//--- machine_error_reset -------------------------
void	machine_error_reset(void)
{
	chiller_error_reset();
	plc_error_reset();
	fluid_error_reset();
	ctrl_head_error_reset();
	step_error_reset();
	enc_error_reset();
}

//--- machine_set_printpar -----------------------
int		machine_set_printpar(SPrintQueueItem *pItem)
{
	set_interface();
	
	ctrl_send_firepulses(pItem->dots);

	switch(_MInterface) 
	{
	case mi_none:	return enc_start_printing (pItem, FALSE);
	case mi_tt:		return tt_set_printpar(pItem);
	case mi_plc:	return plc_set_printpar(pItem);
	}
	return REPLY_OK;
}

//--- machine_set_scans ----------------------------------
int		machine_set_scans(int scans)
{
	switch(_MInterface) 
	{
	case mi_none:	return REPLY_OK;
	case mi_tt:		return tt_set_scans(scans);
	case mi_plc:	return REPLY_OK;
	}
	return REPLY_OK;
}


//--- machine_get_scanner_pos ---------------------
UINT32	machine_get_scanner_pos(void)
{
	switch(_MInterface) 
	{
	case mi_none:	return REPLY_OK;
	case mi_tt:		return tt_get_scanner_pos();
	case mi_plc:	return plc_get_scanner_pos();
	}
	return REPLY_OK;
}

//--- machine_start_printing ----------------------
int		machine_start_printing(void)
{
	switch(_MInterface) 
	{
	case mi_none:	return REPLY_OK;
	case mi_tt:		return tt_start_printing();
	case mi_plc:	return plc_start_printing();
	}
	return REPLY_OK;
}

//--- machine_pause_printing ----------------------
int		machine_pause_printing(void)
{
	switch(_MInterface) 
	{
	case mi_none:	return REPLY_OK;
	case mi_tt:		return tt_pause_printing();
	case mi_plc:	return plc_pause_printing();
	}
	return REPLY_OK;
}

//--- machine_stop_printing -----------------------
int		machine_stop_printing(void)
{
	switch(_MInterface) 
	{
	case mi_none:	return enc_stop_printing();
	case mi_tt:		return tt_stop_printing();
	case mi_plc:	return plc_stop_printing();
	}
	return REPLY_OK;
}

//--- machine_abort_printing -----------------------
int		machine_abort_printing(void)
{
	switch(_MInterface) 
	{
	case mi_none:	return enc_abort_printing();
	case mi_tt:		return tt_abort_printing();
	case mi_plc:	// if (!rx_def_is_scanning(RX_Config.printer.type)) step_handle_gui_msg(INVALID_SOCKET, CMD_CAP_UP_POS, NULL, 0);
					step_abort_printing();
					return plc_abort_printing();
	}
	return REPLY_OK;
}

//--- machine_clean -------------------------------
int		machine_clean(void)
{
	switch(_MInterface) 
	{
	case mi_none:	return REPLY_OK;
	case mi_tt:		return tt_clean();
	case mi_plc:	return plc_clean();
	}
	return REPLY_OK;
}
