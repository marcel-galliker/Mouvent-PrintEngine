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
	"dev_8",			// 08
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

//--- RX_Color_InkSupplyPerSide -----------------------
static char _ColorShortName[INK_SUPPLY_CNT][8];
void RX_ColorNameInit (int inkSupplyNo, ERectoVerso	rectoVerso, int code)
{	
	if (rectoVerso == rv_recto)			sprintf(_ColorShortName[inkSupplyNo], "R%s", RX_ColorName[code].shortName);
	else if (rectoVerso == rv_verso)	sprintf(_ColorShortName[inkSupplyNo], "V%s", RX_ColorName[code].shortName);
	else								sprintf(_ColorShortName[inkSupplyNo], "%s",  RX_ColorName[code].shortName);
}

//--- RX_ColorNameLong ---------------------
const char* RX_ColorNameLong(int code)
{
	return RX_ColorName[code].name;
}

//--- RX_ColorShortName ------------------------------------------
const char* RX_ColorNameShort(int inkSupplyNo)
{
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
	default: return FALSE;
	}
}

//--- rx_def_is_web ---------------------------------------
int rx_def_is_web(EPrinterType printerType)
{
	switch(printerType)	
	{
	case printer_LB701:		return TRUE;
	case printer_LB702:		return TRUE;
	case printer_DP803:		return TRUE;
	case printer_cleaf:		return TRUE;
	default: return FALSE;
	}
}

//--- rx_def_is_singlepage ---------------------------------------
int rx_def_is_singlepage(EPrinterType printerType)
{
	switch(printerType)	
	{
	case printer_test_table:		return TRUE;
	case printer_test_slide:		return TRUE;
	case printer_test_slide_only:	return TRUE;
	default: return FALSE;
	}
}
