// ****************************************************************************
//
//	DIGITAL PRINTING - balance.c
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
#include "rx_common.h"
#include "rx_error.h"
#include "rx_sok.h"
#include "rx_term.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "tcp_ip.h"
#include "comm.h"
#include "fluid_ctrl.h"
#include "balance.h"

#ifdef linux
	#include <termios.h>
	#include <sys/stat.h> 
	#include <fcntl.h>
#endif

//--- defines --------------------------------------
#define COMM_NAME	"/dev/ttyS1"
#define BUF_SIZE		3

typedef enum
{
	led_off,
	led_on,
	led_flash,
	led_timer,	
} ELedState;

//--- golbals -----------------------------------
static SScalesCalibration _RX_ScalesCalibration[SCALES_CALIBRATION_CNT];

//--- scl_cfg_set ----------------------------------
void scl_cfg_set (SScalesCalibration *cfg, int len)
{
	memset(_RX_ScalesCalibration, 0, sizeof(_RX_ScalesCalibration));
	if (len>sizeof(_RX_ScalesCalibration)) len=sizeof(_RX_ScalesCalibration);
	memcpy(_RX_ScalesCalibration, cfg, len);
}

//--- scl_cfg_save --------------------------------------------------
void scl_cfg_save(void)
{
	RX_SOCKET socket=ctrl_socket();
	sok_send_2(&socket, INADDR_ANY, REP_SCALES_SAVE_CFG, sizeof(SScalesCalibration)*BALANCE_CNT, _RX_ScalesCalibration);			
}

//--- scl_cfg_reload --------------------------------------------------
void scl_cfg_reload(void)
{
	RX_SOCKET socket=ctrl_socket();
	sok_send_2(&socket, INADDR_ANY, CMD_SCALES_LOAD_CFG, 0, NULL);			
}
