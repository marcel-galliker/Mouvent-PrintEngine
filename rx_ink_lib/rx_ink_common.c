/******************************************************************************/
/** rx_ink_common.h
 ** 
 ** Communication between NIOS and Conditioner
 **
 **	Copyright 2016 by radex AG, Switzerland. All rights reserved.
 **	Written by Marcel Galliker
 **
 ******************************************************************************/
#include <string.h>
#include <stdio.h>
#include "rx_ink_common.h"

#ifdef NIOS
	int abs(int);
#endif

#ifdef COND
	int abs(int);
#endif

//--- FluidCtrlModeStr ---------------------------------------------------------
char *FluidCtrlModeStr(EnFluidCtrlMode mode)
{
	static char str[10];
	switch (mode)
	{		
	case ctrl_undef:	    		return "---";		//  0x000:
//	case ctrl_shutdown:				return "SHUTDOWN";	//  0x001:
//	case ctrl_shutdown_done:		return "SD-done";	//  0x002:
	case ctrl_off:					return "OFF";		//  0x003:
	case ctrl_warmup:				return "WARM ";		//  0x004:
	case ctrl_readyToPrint:			return "READY";		//  0x005:

	case ctrl_print_step0:			return "PRINT /0";	//  0x010:
	case ctrl_print_step1:			return "PRINT /1";	// 	0x011
	case ctrl_print_step2:			return "PRINT /2";	// 	0x012
	case ctrl_print_step3:			return "PRINT /3";	// 	0x013
	case ctrl_print_step4:			return "PRINT /4";	// 	0x014
	case ctrl_print_step5:			return "PRINT /5";	// 	0x015
	case ctrl_print_step6:			return "PRINT /6";	// 	0x016
	case ctrl_print_step7:			return "PRINT /7";	// 	0x017
	case ctrl_print_step8:			return "PRINT /8";	// 	0x018
	case ctrl_print_step9:			return "PRINT /9";	// 	0x019	
	case ctrl_print_run:			return "PRINT";		//  0x01a:

	case ctrl_flush_night:			return "FLUSH_N";	//  0x080:
	case ctrl_flush_weekend:		return "FLUSH_WE";	//  0x081:
	case ctrl_flush_week:			return "FLUSH_W";	//  0x082:
	case ctrl_flush_step1:			return "FLUSH /1";  //  0x083:
	case ctrl_flush_step2:			return "FLUSH /2";  //  0x084:
	case ctrl_flush_step3:			return "FLUSH /3";  //  0x084:
	case ctrl_flush_step4:			return "FLUSH /4";  //  0x084:
	case ctrl_flush_done:			return "FLUSH_done";//  0x085:
		
	case ctrl_purge_soft:			return "PRG-S";		//  0x100:
	case ctrl_purge:				return "PRG";		//  0x101:
	case ctrl_purge_hard:			return "PRG-H";		//  0x102:
	case ctrl_purge_micro:			return "PRG-M";		//  0x103:
	case ctrl_purge_step1:			return "PRG /1";	//  0x111:
	case ctrl_purge_step2:			return "PRG /2";	//  0x112:
	case ctrl_purge_step3:			return "PRG /3";	//  0x113:
	case ctrl_purge_step4:			return "PRG /4";	//  0x114:
		
	case ctrl_fill:					return "FILL";		//	0x200:
	case ctrl_fill_step1:			return "FILL /1";	//	0x201:
	case ctrl_fill_step2:			return "FILL /2";	//	0x202:
	case ctrl_fill_step3:			return "FILL /3";	//	0x203:
	case ctrl_fill_step4:			return "FILL /4";	//	0x204:	

	case ctrl_empty:				return "EMPTY";		//	0x300:
	case ctrl_empty_step1:			return "EMPTY /1";	//	0x301:
	case ctrl_empty_step2:			return "EMPTY /2";	//	0x302:
	case ctrl_empty_step3:			return "EMPTY /3";	//	0x303:
	case ctrl_empty_step4:			return "EMPTY /4";	//	0x304:
	case ctrl_empty_step5:			return "EMPTY /5";	//	0x305:

	case ctrl_cal_start:			return "START CAL";	//  0x00a:
	case ctrl_cal_step1:			return "CAL /1";	//  0x00b:
	case ctrl_cal_step2:			return "CAL /2";	//  0x00c:
	case ctrl_cal_step3:			return "CAL /3";	//  0x00d:
	case ctrl_cal_step4:			return "CAL /4";	//  0x00e:
	case ctrl_cal_done:				return "CAL DONE";	//  0x00f:

	case ctrl_test_watchdog:		return "WATCHDOG";	//0x10000,
	case ctrl_test:					return "TEST";		//0x10001,
    	
	case ctrl_offset_cal: 			return "CAL OFFSET";	// 0x10003
	case ctrl_offset_cal_done: 		return "CAL DONE";		// 0x10004

//	case ctrl_heater_error:	return "HEAT ERR";  //0x12346,
		
	default:				sprintf(str, "%d", mode); return str;
	}
}

//--- DownloadStateStr -----------------------------------
char *DownloadStateStr(EnDownloadState state)
{
	switch (state)
	{
	case dnl_start:				return "start";
	case dnl_prepare:			return "prepare";
	case dnl_reset:				return "reset";
	case dnl_send_ram_kernel:	return "send ram Kernel";
	case dnl_start_ram_kernel:	return "start ram Kernel";
	case dnl_read_flash:		return "read flash";
	case dnl_chip_erase:		return "erase flash";
	case dnl_blank_check:		return "blankcheck";
	case dnl_write_binary:		return "write flash";
	case dnl_mcu_reset2:		return "restart";
	case dnl_done:				return "done";
	default:					return "undef";	
	}
}

//--- valid --------------------------------------------
int valid(int val)
{
	if (val==INVALID_VALUE)      return FALSE;
	else if (val==VAL_UNDERFLOW) return FALSE;
	else if (val==VAL_OVERFLOW)  return FALSE;
	return TRUE;
}

//--- value_str ---------------------------------------------
char *value_str(int val)
{
	static char str[32];
	if (val==INVALID_VALUE)      strcpy(str, "-----");
	else if (val==VAL_UNDERFLOW) strcpy(str, "vvvv");	
	else if (val==VAL_OVERFLOW)  strcpy(str, "^^^^");	
	else						 sprintf(str, "%d", val);
	return str;
}

//--- value_str1 ---------------------------------------------
char *value_str1(int val)
{
	static char str[32];
	if (val == INVALID_VALUE)	  strcpy(str, "-----");
	else if (val==VAL_UNDERFLOW)  strcpy(str, "vvvv");	
	else if (val==VAL_OVERFLOW)   strcpy(str, "^^^^");	
	else					      sprintf(str, "%d.%d", val/10, abs(val)%10);
	return str;
}

//--- value_str_temp ---------------------------------------------
char *value_str_temp(int val)
{
	static char str[32];
	if (val==INVALID_VALUE)      strcpy(str, "-----");
	else if (val==VAL_UNDERFLOW) strcpy(str, "vvvv");	
	else if (val==VAL_OVERFLOW)  strcpy(str, "^^^^");	
	else					     sprintf(str, "%d.%d", val/1000, (abs(val)%1000)/100);
	return str;
}

//--- value_str_u ---------------------------------------------
char *value_str_u(int val)
{
	static char str[32];
	if (val==INVALID_VALUE) strcpy(str, "-----");
	else					sprintf(str, "%d.%d", val/1000, (abs(val)%1000)/10);
	return str;
}

//--- value_str3 ---------------------------------------------
char *value_str3(int val)
{
	static char str[32];
	if (val==INVALID_VALUE) strcpy(str, "-----");
	else					sprintf(str, "%d.%03d", val/1000, abs(val)%1000);
	return str;
}

//--- convert_16_to_32 ------------------
/*
INT32 convert_i16_to_i32(INT16 val)
{
	if (val==INVALID_VALUE_16) return INVALID_VALUE;
	return (INT32)val;
}
*/
