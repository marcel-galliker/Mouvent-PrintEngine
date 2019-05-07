// ****************************************************************************
//
//	DIGITAL PRINTING - balance.h
//
//	Communication between ARM processor and FPGA
//
// ****************************************************************************
//
//	Copyright 2013 by radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

//--- includes ----------------------------------------------------------------
#include "rx_def.h"
#include "tcp_ip.h"
#include "balance_def.h"

int  scl_init		(void);
int  scl_end		(void);
void scl_cfg_set	(SScalesCalibration *cfg, int len);
void scl_cfg_reload	(void);
void scl_cfg_save	(void);
int  scl_main		(int ticks, int menu, int displayStatus);
void scl_lamp		(int no);

int  scl_weight  (int no);
int  scl_humidity(int no);

void scl_calibrate(SScalesCalibrateCmd *pcmd);


