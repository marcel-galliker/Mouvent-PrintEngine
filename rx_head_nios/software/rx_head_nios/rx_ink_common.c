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
	case ctrl_shutdown:				return "SHUTDOWN";	//  0x001:
	case ctrl_shutdown_done:		return "SD-done";	//  0x002:
	case ctrl_error:				return "ERROR";		//  0x003:
    case ctrl_wait:					return "WAIT";		//  0x004:
	case ctrl_off:					return "OFF";		//  0x005:
	case ctrl_warmup:				return "WARM ";		//  0x006:
	case ctrl_readyToPrint:			return "READY";		//  0x007:
	case ctrl_print:				return "PRINT";		//  0x008:
    

	case ctrl_check_step0:			return "CHECK/0";	//  0x010:
	case ctrl_check_step1:			return "CHECK/1";	// 	0x011
	case ctrl_check_step2:			return "CHECK/2";	// 	0x012
	case ctrl_check_step3:			return "CHECK/3";	// 	0x013
	case ctrl_check_step4:			return "CHECK/4";	// 	0x014
	case ctrl_check_step5:			return "CHECK/5";	// 	0x015
	case ctrl_check_step6:			return "CHECK/6";	// 	0x016
	case ctrl_check_step7:			return "CHECK/7";	// 	0x017
	case ctrl_check_step8:			return "CHECK/8";	// 	0x018
	case ctrl_check_step9:			return "CHECK/9";	// 	0x019	

	case ctrl_flush_night:			return "FLUSH_N";	//  0x080:
	case ctrl_flush_weekend:		return "FLUSH_WE";	//  0x081:
	case ctrl_flush_week:			return "FLUSH_W";	//  0x082:
	case ctrl_flush_step1:			return "FLUSH/1";	//  0x083:
	case ctrl_flush_step2:			return "FLUSH/2";	//  0x084:
	case ctrl_flush_step3:			return "FLUSH/3";	//  0x084:
	case ctrl_flush_step4:			return "FLUSH/4";	//  0x084:
	case ctrl_flush_done:			return "FLUSH_done";//  0x085:
		
	case ctrl_purge_soft:			return "PRG-S";		//  0x100:
	case ctrl_purge:				return "PRG";		//  0x101:
	case ctrl_purge_hard:			return "PRG-H";		//  0x102:
	case ctrl_purge_hard_wipe:		return "PRG+WPE";	//  0x103:
	case ctrl_purge_hard_vacc:		return "PRG+VAC";	//  0x104:
    case ctrl_purge_hard_wash:		return "PRG+WASH";	//  0x105
	case ctrl_purge_step1:			return "PRG/1";		//  0x111:
	case ctrl_purge_step2:			return "PRG/2";		//  0x112:
	case ctrl_purge_step3:			return "PRG/3";		//  0x113:
	case ctrl_purge_step4:			return "PRG/4";		//  0x114:
	case ctrl_purge_step5:			return "PRG/5";		//  0x115:
	case ctrl_purge_step6:			return "PRG/6";		//  0x116:

	case ctrl_wipe:					return "WIPE";		// 0x120:
	case ctrl_wipe_step1:			return "WIPE/1";	// 0x131:
	case ctrl_wipe_step2:	  		return "WIPE/2";	// 0x132:
	case ctrl_wipe_step3:	  		return "WIPE/3";	// 0x133:
	case ctrl_wipe_step4:	  		return "WIPE/4";	// 0x134:
	case ctrl_wipe_step5:	  		return "WIPE/5";	// 0x135:
	case ctrl_wipe_step6:	  		return "WIPE/6";	// 0x136:

	case ctrl_cap:					return "CAP";		// 0x140:
	case ctrl_cap_step1:			return "CAP/1";		// 0x141:
	case ctrl_cap_step2:			return "CAP/2";		// 0x142:
	case ctrl_cap_step3:			return "CAP/3";		// 0x143:
	case ctrl_cap_step4:			return "CAP/4";		// 0x144:
	case ctrl_cap_step5:			return "CAP/5";		// 0x145:
	
	case ctrl_vacuum:				return "VAC";		// 0x150:
	case ctrl_vacuum_step1:			return "VAC/1";		// 0x151:
	case ctrl_vacuum_step2:	  		return "VAC/2";		// 0x152:
	case ctrl_vacuum_step3:	  		return "VAC/3";		// 0x153:
	case ctrl_vacuum_step4:	  		return "VAC/4";		// 0x154:
	case ctrl_vacuum_step5:	  		return "VAC/5";		// 0x155:
	case ctrl_vacuum_step6:	  		return "VAC/6";		// 0x156:
	case ctrl_vacuum_step7:	  		return "VAC/7";		// 0x157:
	case ctrl_vacuum_step8:	  		return "VAC/8";		// 0x158:
	case ctrl_vacuum_step9:	  		return "VAC/9";		// 0x159:
	case ctrl_vacuum_step10:  		return "VAC/10";	// 0x15a:
	case ctrl_vacuum_step11:  		return "VAC/11";	// 0x15b:
	case ctrl_vacuum_step12:		return "VAC/12";	// 0x15c:
	case ctrl_vacuum_step13:		return "VAC/13";	// 0x15d:
	case ctrl_vacuum_step14:		return "VAC/14";	// 0x15e:
	
	case ctrl_wash:					return "WASH";		// 0x160:
	case ctrl_wash_step1:			return "WASH/1";   	// 0x161:
	case ctrl_wash_step2:			return "WASH/2";  	// 0x162:
	case ctrl_wash_step3:			return "WASH/3";  	// 0x163:
	case ctrl_wash_step4:			return "WASH/4";  	// 0x164:
	case ctrl_wash_step5:			return "WASH/5";  	// 0x165:
	case ctrl_wash_step6:			return "WASH/6";  	// 0x166:
		
	case ctrl_fill:					return "FILL";		//	0x200:
	case ctrl_fill_step1:			return "FILL/1";	//	0x201:
	case ctrl_fill_step2:			return "FILL/2";	//	0x202:
	case ctrl_fill_step3:			return "FILL/3";	//	0x203:
	case ctrl_fill_step4:			return "FILL/4";	//	0x204:	

	case ctrl_empty:				return "EMPTY";		//	0x300:
	case ctrl_empty_step1:			return "EMPTY/1";	//	0x301:
	case ctrl_empty_step2:			return "EMPTY/2";	//	0x302:
	case ctrl_empty_step3:			return "EMPTY/3";	//	0x303:
	case ctrl_empty_step4:			return "EMPTY/4";	//	0x304:
	case ctrl_empty_step5:			return "EMPTY/5";	//	0x305:
	case ctrl_empty_step6:			return "EMPTY/6";	//	0x305:

	case ctrl_cal_start:			return "START CAL";	//  0x00a:
	case ctrl_cal_step1:			return "CAL/1";	//  0x00b:
	case ctrl_cal_step2:			return "CAL/2";	//  0x00c:
	case ctrl_cal_step3:			return "CAL/3";	//  0x00d:
	case ctrl_cal_step4:			return "CAL/4";	//  0x00e:
	case ctrl_cal_done:				return "CAL DONE";	//  0x00f:

	case ctrl_test_watchdog:		return "WATCHDOG";	//0x10000,
	case ctrl_test:					return "TEST";		//0x10001,
    	
//	case ctrl_heater_error:	return "HEAT ERR";  //0x12346,
		
	default:				sprintf(str, "0x%04x", mode); return str;
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

//--- value_str_screw ---------------------------------------------
char *value_str_screw(INT16 val)
{
	static char str[32];
//	if (val<=0)	strcpy(str, "----");
//	else		sprintf(str, "%02d.%d", val/6, val%6);
	if (val==(INT16)INVALID_VALUE) strcpy(str, "-----");
	else					sprintf(str, "%02d.%d", val/6, abs(val)%6);
	return str;
}

//--- str_to_screw ----------------------------------------
int str_to_screw(const char *str)
{
	int val=0;
	int nachkomma=0;
    int sign = 1;
    const char *ch;
    if (str[0] == '-') sign = -1;
    for (ch=str; *ch; ch++)
	{
        if (*ch>='0' && *ch<='9') val=(val*10)+*ch-'0';
		if (*ch=='.' || *ch==',') 
		{
			ch++;
			if (*ch>='0' && *ch<='9') nachkomma=*ch-'0';
			break;
		}
	}
	if (nachkomma>5) nachkomma=5;
	return sign * (6*val+nachkomma);
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
