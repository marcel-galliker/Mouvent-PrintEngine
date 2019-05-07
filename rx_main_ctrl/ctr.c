// ****************************************************************************
//
//	DIGITAL PRINTING - ctr.c
//
// ****************************************************************************
//
//	Copyright 2018 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#include "rx_def.h"
#include "rx_setup_file.h"
#include "rx_hash.h"
#include "rx_sok.h"
#include "ctr.h"

static int		_Time;

//--- ctr_init --------------------------------------
void ctr_init(void)
{	
	RX_PrinterStatus.counterAct = 0;
	RX_PrinterStatus.counterTotal = 0;
	_Time   = 0;

	HANDLE file = setup_create();

	setup_load(file, PATH_USER FILENAME_COUNTERS);
	
	if (setup_chapter(file, "Counters", -1, READ)==REPLY_OK)
	{
		setup_double(file, "actual", READ, &RX_PrinterStatus.counterAct,   0);
		setup_double(file, "total",  READ, &RX_PrinterStatus.counterTotal, 0);
	}
	setup_destroy(file);
}

//--- ctr_tick -----------------------------
void ctr_tick(void)
{
	if (++_Time>60)
	{
		ctr_save();
		_Time=0;			
	}
}

//--- ctr_add -------------------------------------------
void ctr_add(double m)
{
	RX_PrinterStatus.counterTotal	+= m;
	RX_PrinterStatus.counterAct		+= m;
}

//--- ctr_reset ---------------------------------------------
void ctr_reset(void)
{
	RX_PrinterStatus.counterAct = 0;
	ctr_save();
}

//--- _ctr_save --------------------------------------------------
void ctr_save(void)
{
	HANDLE file = setup_create();
	
	if (setup_chapter(file, "Counters", -1, WRITE)==REPLY_OK)
	{
		setup_str	(file, "machine", WRITE, RX_Hostname, sizeof(RX_Hostname), "");
		setup_double(file, "actual", WRITE, &RX_PrinterStatus.counterAct, 0);
		setup_double(file, "total",  WRITE, &RX_PrinterStatus.counterTotal, 0);
	}
	setup_save(file, PATH_USER FILENAME_COUNTERS);
	setup_destroy(file);		
}
