/******************************************************************************/
/** log.c
 **
 **
 **	Copyright 2016 by radex AG, Switzerland. All rights reserved.
 **	Written by Marcel Galliker
 **
 ******************************************************************************/

#include <stdio.h>
#include "nios_def_fluid.h"
#include "log.h"

static int _cnt=0;

//--- log_init ----------------------------
void log_init(void)
{
	pRX_Status->logInIdx  = 0;
	pRX_Status->logOutIdx = 0;
}


static int valstr(char *str, INT32 val)
{
	if (val==INVALID_VALUE) return sprintf(str, "\t");
	else return sprintf(str, "%d\t", (int)val);
}

//--- log_tick_10ms ------------------------
void log_tick_10ms(void)
{
	char *str;
	int len, i;
	if (pRX_Config->cmd.logging)
	{
		if (_cnt==0)
		{
			str = pRX_Status->logStr[pRX_Status->logInIdx];
			len  = sprintf(str, "#:\t");
			for (i=0; i<NIOS_INK_SUPPLY_CNT; i++)
			{
				len += sprintf(&str[len], "cond%d:\t", i);
				len += sprintf(&str[len], "pset%d:\t", i);
				len += sprintf(&str[len], "pres%d:\t", i);
				len += sprintf(&str[len], "pumpset%d:\t", i);
//				len += sprintf(&str[len], "pump%d:\t", i);
				len += sprintf(&str[len], "bleed%d:\t", i);
				len += sprintf(&str[len], "air%d:\t", i);
			}
			sprintf(&str[len], "\n");
			pRX_Status->logInIdx = (pRX_Status->logInIdx+1) % LOG_STR_CNT;
		}

		str = pRX_Status->logStr[pRX_Status->logInIdx];
		len = sprintf(str, "%d\t", _cnt);
		for (i=0; i<NIOS_INK_SUPPLY_CNT; i++)
		{
			len += valstr(&str[len], pRX_Config->ink_supply[i].condPresIn);
			len += valstr(&str[len], pRX_Status->ink_supply[i].cylinderPresSet);
			len += valstr(&str[len], pRX_Status->ink_supply[i].IS_Pressure_Actual);
			len += valstr(&str[len], pRX_Status->ink_supply[i].inkPumpSpeed_set+110);
//			len += valstr(&str[len], pRX_Status->ink_supply[i].inkPumpSpeed_measured);
			len += sprintf(&str[len], "%d\t", pRX_Status->ink_supply[i].bleedValve);
			len += sprintf(&str[len], "%d\t", pRX_Status->ink_supply[i].airValve*20+110);
		}
		sprintf(&str[len], "\n");

		pRX_Status->logInIdx = (pRX_Status->logInIdx+1) % LOG_STR_CNT;
		_cnt++;
	}
	else _cnt=FALSE;
}



