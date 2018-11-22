// ****************************************************************************
//
//	DIGITAL PRINTING - fpga.c
//
//	Communication between ARM processor and FPGA
//
// ****************************************************************************
//
//	Copyright 2015 by radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************
	
// WARNING: No access to the ADDR_FPGA_BASE address room during runtime! 

//--- includes ----------------------------------------------------------------
#ifdef linux
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <sys/mman.h>
	#include <unistd.h>
#endif
#include <fcntl.h>
#include <errno.h>
#include "rx_common.h"
#include "rx_def.h"
#include "rx_fpga.h"
#include "rx_error.h"
#include "rx_file.h"
#include "rx_mac_address.h"
#include "rx_term.h"
#include "rx_trace.h"
#include "fpga_def_fluid.h"
#include "fpga_fluid.h"

//---- COMPILER OPTIONS ------------------------------------------------------

//--- defines -----------------------------------------------------------------

//--- structures ----------------------------


//--- globals -----------------------------------------------------------------

//--- module globals ---------------------------------------
static int				_Init=FALSE;
static SFpgaQSys		*_Qsys;
static SFluidFpgaStatus	*_Stat;
static int				_MemId=0;

//--- prototypes -------------------------------------------
static void  _fpga_display_error(void);
static void  _fpga_display_status(void);
static void  _check_errors(void);

//*** functions ********************************************

//--- fpga_init ----------------
void fpga_init()
{
	int reply, fpga_running;	
	_Init = FALSE;
	if (rx_fpga_load (PATH_BIN_FLUID FIELNAME_FLUID_RBF)!=REPLY_OK) return;	

	//--- map the meory ------------------
	_MemId = open("/dev/mem", O_RDWR | O_SYNC);
	if (_MemId==-1) Error(ERR_CONT, 0, "Could not open memory handle.");
	
	_Qsys = (SFpgaQSys*)rx_fpga_map_page(_MemId, ADDR_FPGA_QSYS, sizeof(SFpgaQSys),	0x20020);
	_Stat = (SFluidFpgaStatus*)rx_fpga_map_page(_MemId, ADDR_FPGA_STAT, sizeof(SFluidFpgaStatus),	0x005c);
	
	TrPrintfL(TRUE, "Version: %d.%d.%d.%d", _Stat->version.major, _Stat->version.minor, _Stat->version.revision, _Stat->version.build);

	_Init = TRUE;
}

// --- fpga_end -------------------
void fpga_end()
{
	_Init = FALSE;
}

//--- fpga_is_init -----------------------------
int  fpga_is_init(void)
{
	return _Init;
}

//--- fpga_qsys_id ---
int	  fpga_qsys_id(void)
{
	return 	_Qsys->qsys_id;		
}

//--- fpga_qsys_timestamp ---------------------
int	  fpga_qsys_timestamp(void)
{
	return _Qsys->qsys_timestamp;			
}

//--- _fpga_display_status -----------------------------------------------
static void _fpga_display_status(void)
{
	if (_Init)
	{
	}
	else
	{
		term_printf("\n");
		term_printf("--- FPGA NOT LOADED -----------------------------------\n");
		term_printf("\n");			
	}
}

//--- _fpga_display_error --------------------------------------------------
static void _fpga_display_error(void)
{
	if (_Init)
	{
	}
}

//--- _check_errors ---------------------------------------------------------------------
static void  _check_errors(void)
{
	if (_Init)
	{
		
	}
	else 
	{
		ErrorFlag (ERR_ABORT, &RX_FluidBoardStatus.err,  err_fpga_not_loaded,  0, "FPGA NOT LOADED");	
	}
}

//--- fpga_main ------------------------------------------
void  fpga_main(int ticks, int menu)
{
	if (menu)
	{
		_fpga_display_status();
		_check_errors();
		_fpga_display_error();				
	}
}
