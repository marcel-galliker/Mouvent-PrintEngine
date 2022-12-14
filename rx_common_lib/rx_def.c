// ****************************************************************************
//
//	RX_def.c
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#include "rx_def.h"
#include "tcp_ip.h"
#include <stdio.h>

//--- rx_def_init -----------------------------------------------------------------
void rx_def_init(void)
{
}

//--- Global variables -----------------------------------------------------

char *DeviceStr[] =
{
	"undef",			// 00
	"Main",				// 01
	"GUI",				// 02
	"Plc",				// 03
	"Encoder",			// 04
	"Fluid",			// 05
	"dev_6",			// 06
	"dev_7",			// 07
	"Robot",			// 08
	"Stepper",			// 09
	"Head",				// 10
	"Spooler",			// 11
	0
};

//--- Log Item Types ---------------------------------------------------------
char *LogItemTypeStr[] =
{
	"undef",
	"LOG",
	"WARN",
	"ERROR-CONT",
	"ERROR-STOP",
	"ERROR-ABORT",
	0
};
	
//--- ColorName --------------------------------
SColorName RX_ColorName[] =
{
	{ 0,   "Black",						"K"   },
	{ 1,   "Cyan",						"C"   },
	{ 2,   "Magenta",					"M"   },
	{ 3,   "Yellow",					"Y"   },
	{ 4,   "Red/Orange",				"R"   },
	{ 5,   "Green",						"G"   },
	{ 6,   "Blue",						"B"   },
	{ 7,   "Light Black",				"LK"  },
	{ 8,   "Light Cyan",				"LC"  },
	{ 9,   "Light Magenta",				"LM"  },
	{10,   "Light Black 2",				"LLK" },
	{11,   "Light Cyan 2",				"LLC" },
	{12,   "Light Magenta 2",			"LLM" },
	{13,   "Light Yellow",				"LY"  },
	{14,   "Light Red",					"LR"  },
	{15,   "Light Green",				"LG"  },
	{16,   "Light Blue",				"LB"  },
	{17,   "Spot Color 6",				"S6"  },
	{18,   "Spot Color 5",				"S5"  },
	{19,   "Spot Color 4",				"S4"  },
	{20,   "Spot Color 3",				"S3"  },
	{21,   "Spot Color 2",				"S2"  },
	{22,   "Spot Color 1",				"S1"  },
	{23,   "Light Yellow 2",			"LLY" },
	{24,   "Midlayer",					"MID" },
	{25,   "Primer 1",					"PRI1" },
	{26,   "Primer 2",					"PRI2" },
	{27,   "Varnish / Finish 4",		"V4"  },
	{28,   "Varnish / Finish 3",		"V3"  },
	{29,   "Varnish / Finish 2",		"V2"  },
	{30,   "Varnish / Finish 1",		"V1"  },
	{31,   "White 4",					"W4"  },
	{32,   "White 3",					"W3"  },
	{33,   "White 2",					"W2"  },
	{34,   "White 1",					"W1"  },
	{35,   "Profile Channel 8",			"P8"  },
	{36,   "Profile Channel 9",			"P9" },
	{37,   "Profile Channel 10",		"P10" },
	{38,   "Profile Channel 11",		"P11" },
	{39,   "Profile Channel 12",		"P12" },
	{40,   "Light Black 3",				"3LK" },
	{41,   "Light Black 4",				"4LK" },
	{42,   "Light Black 5",				"5LK" },
	{43,   "Light Black 6",				"6LK" },
	{44,   "Light Profile Channel 8",	"LP8" },
	{45,   "Light Profile Channel 9",	"LP9" },
	{46,   "Light Profile Channel 10",	"LP10"},
	{47,   "Light Profile Channel 11",	"LP11"},
	{48,   "Light Profile Channel 12",	"LP12"},
	{49,   "Light Spot Color 6",		"LS6" },
	{50,   "Light Spot Color 5",		"LS5" },
	{51,   "Light Spot Color 4",		"LS4" },
	{52,   "Light Spot Color 3",		"LS3" },
	{53,   "Light Spot Color 2",		"LS2" },
	{54,   "Light Spot Color 1",		"LS1" }
};

char *PrintStateStr[] =
{
	"ps_undef",			// 00
	"ps_off",			// 01
	"ps_ready_power",	// 02
	"ps_printing",		// 03
	"ps_stopping",		// 04
    "ps_goto_pause",	// 05
    "ps_pause",			// 06
    "ps_cleaning",		// 07
	"ps_webin",			// 08
	"ps_setup",			// 09
};

//--- RobFunctionStr ----------------
char* RobFunctionStr[]=
{   
	"rob_fct_undef",		//  0: undef	
	"rob_fct_cap",			//  1: Capping
	"rob_fct_wash",			//  2: Wash
	"rob_fct_vacuum",		//  3: Vacuum
	"rob_fct_wipe",			//  4: Wiping
	"rob_fct_cap_empty",	//  5: Capping without Flush in Capping station
	"rob_fct_vacuum_change",//  6: Vaccum change
	"rob_fct_tilt",			//  7: Tilt for Capping
	"rob_fct_vacuum_all",	//  8: Vacuum all heads
    "rob_fct_move_purge",	//  9: Move to Purge position
    "rob_fct_move_purge_end",//10: Move to Purge position
    "rob_fct_purge4ever",	// 11: Purge Capping
    "rob_fct_move",			// 12: Wash heads
	"rob_fct_move_to_pos",	// 13: Move to absomute position
	"rob_fct_maintenance",	// 14: Maintenance Pos
};

//--- MsgIdStr ----------------------------------
const char *MsgIdStr(UINT32 msgId)
{
	switch(msgId)
	{
    case CMD_ERROR_RESET:			return "CMD_ERROR_RESET";

    case CMD_LIFT_STOP:			    return "CMD_LIFT_STOP";
    case CMD_LIFT_REFERENCE:	    return "CMD_LIFT_REFERENCE";
    case CMD_LIFT_UP_POS:			return "CMD_LIFT_UP_POS";			
    case CMD_LIFT_SCREW:		    return "CMD_LIFT_SCREW";
    case CMD_LIFT_PRINT_POS:		return "CMD_LIFT_PRINT_POS";
    case CMD_LIFT_CAPPING_POS:		return "CMD_LIFT_CAPPING_POS";
    case CMD_LIFT_WASH_POS:		    return "CMD_LIFT_WASH_POS";
    case CMD_LIFT_CLUSTER_CHANGE:	return "CMD_LIFT_CLUSTER_CHANGE";

    case CMD_ROB_REFERENCE:			return "CMD_ROB_REFERENCE";
    case CMD_ROB_MOVE_POS:			return "CMD_ROB_MOVE_POS";
    case CMD_ROB_WASH:				return "CMD_ROB_WASH";
    case CMD_ROB_VACUUM:			return "CMD_ROB_VACUUM";
    case CMD_ROB_SERVICE:			return "CMD_ROB_SERVICE";
    case CMD_ROB_EMPTY_WASTE:		return "CMD_ROB_EMPTY_WASTE";
    case CMD_ROB_FILL_CAP:			return "CMD_ROB_FILL_CAP";
    case CMD_ROB_SET_FLUSH_VALVE:	return "CMD_ROB_SET_FLUSH_VALVE";

    default: {
				static char str[32];
				sprintf(str, "0x%08x", msgId);
				return str;
			 }
	}
}

//--- RX_Color_InkSupplyPerSide -----------------------
static char _ColorShortName[INK_SUPPLY_CNT][8];
void RX_ColorNameInit (int inkSupplyNo, ERectoVerso	rectoVerso, char *fileName, int colorCode)
{	
	if(*fileName == 0) strcpy(_ColorShortName[inkSupplyNo], "-");
	else
	{
		if(rectoVerso == rv_recto)		sprintf(_ColorShortName[inkSupplyNo], "R%s", RX_ColorName[colorCode].shortName);
		else if(rectoVerso == rv_verso)	sprintf(_ColorShortName[inkSupplyNo], "V%s", RX_ColorName[colorCode].shortName);
		else							sprintf(_ColorShortName[inkSupplyNo], "%s", RX_ColorName[colorCode].shortName);
	}
}

//--- RX_ColorNameLong ---------------------
const char* RX_ColorNameLong(int code)
{
	if (code >= SIZEOF(RX_ColorName)) return "INVALID ADDRESS";
	return RX_ColorName[code].name;
}

//--- RX_ColorShortName ------------------------------------------
const char* RX_ColorNameShort(int inkSupplyNo)
{
    if (inkSupplyNo >= SIZEOF(_ColorShortName)) return "INVALID ADDRESS";
	return _ColorShortName[inkSupplyNo];
}

//--- rx_def_is_scanning ---------------------------------------
int rx_def_is_scanning(EPrinterType printerType)
{
	switch(printerType)	
	{
	case printer_test_table:		return TRUE;
	case printer_test_slide:		return TRUE;
	case printer_test_slide_only:	return TRUE;
	case printer_TX801:				return TRUE;
	case printer_TX802:				return TRUE;
	case printer_TX404:				return TRUE;
    case printer_test_table_seon:	return TRUE;
	default: return FALSE;
	}
}

//--- rx_def_is_tx ---------------------------------------
int rx_def_is_tx(EPrinterType printerType)
{
	switch(printerType)	
	{
	case printer_TX801:				return TRUE;
	case printer_TX802:				return TRUE;
	case printer_TX404:				return TRUE;
	default: return FALSE;
	}
}

//--- rx_def_is_lb ---------------------------------------
int rx_def_is_lb(EPrinterType printerType)
{
	switch(printerType)	
	{
	case printer_LB701:		return TRUE;
	case printer_LB702_UV:	return TRUE;
	case printer_LB702_WB:	return TRUE;
	case printer_LH702:		return TRUE;
	case printer_DP803:		return TRUE;
	case printer_cleaf:		return TRUE;
	default: return FALSE;
	}
}

//--- rx_def_is_production_test ---------------------------------------
int rx_def_is_tts(EPrinterType printerType)
{
    switch (printerType)
    {
    case printer_test_table_seon:
        return TRUE;
    default:
        return FALSE;
    }
}

//--- rx_def_is_test ---------------------------------------
int rx_def_is_test(EPrinterType printerType)
{
	switch(printerType)	
	{
	case printer_test_table:		return TRUE;
	case printer_test_slide:		return TRUE;
	case printer_test_slide_only:	return TRUE;
	case printer_test_table_seon:	return TRUE;
	case printer_test_CTC:			return TRUE;
	default: return FALSE;
	}
}

//--- rx_def_use_pq ---------------------------------------
int rx_def_use_pq(EPrinterType printerType)
{
	switch(printerType)	
	{
	case printer_test_table:		return FALSE;
	case printer_test_slide:		return FALSE;
	case printer_test_slide_only:	return FALSE;
	default: return TRUE;
	}
}

//--- rx_printMode_is_test -------------------
int rx_printMode_is_test(int printMode)
{
	return printMode==PM_TEST || printMode==PM_TEST_SINGLE_COLOR || printMode==PM_TEST_JETS;
}
