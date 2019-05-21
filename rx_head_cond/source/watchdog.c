/******************************************************************************/
/** \file watchdog.c
 ** 
 ** software watchdog
 **
 **	Copyright 2016 by radex AG, Switzerland. All rights reserved.
 **	Written by Stefan Weber
 **
 ******************************************************************************/

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "watchdog.h" 
  
boolean_t bSwwdtFlag = FALSE;

/**
******************************************************************************
** \brief   Initialize the Software Watchdog Timer
******************************************************************************/
void InitSwWatchdog(void)
{
	FM4_CRG->SWC_PSR= 3u;					   // Software Watchdog pre scalar (PCLK0 / 8)
	FM4_SWWDT->WDOGLOCK = 0x1ACCE551ul;        // Unlock Software Watchdog Timer registers
	FM4_SWWDT->WDOGCONTROL = 0x02u;            // Window watchdog mode disabled, reload is enabled, watchdog reset is enabled
	FM4_SWWDT->WDOGLOAD = ((160000000ul / 2u / 8u) * SWWDT_DELAY_S);    // Timer reload value, delay -> 2.5s ((200000000 / 2 / 8) * 2.5)
	FM4_SWWDT->WDOGCONTROL_f.INTEN = 1u;       // Start Software Watchdog Timer
	FM4_SWWDT->WDOGLOCK = 0xFFFFFFFFul;        // Lock Software Watchdog Timer registers
}

/**
******************************************************************************
** \brief   Reload the Software Watchdog Timer
**          Reloads the set value from the WdogLoad register to the watchdog 
**          timer counter.
******************************************************************************/
void ClearSwWatchdog(void)
{
	FM4_SWWDT->WDOGLOCK = 0x1ACCE551ul;        // Unlock Software Watchdog Timer registers
	FM4_SWWDT->WDOGINTCLR = 0xFFFFFFFFul;      // Reload watchdog timer
	bSwwdtFlag = FALSE;
	FM4_SWWDT->WDOGLOCK = 0xFFFFFFFFul;        // Lock Software Watchdog Timer registers
}

//--- watchdog_toggle24V -------------------------------------
void	watchdog_toggle24V(void)
{
	static int _watchdog = 0;
	Gpio1pin_Put(GPIO1PIN_P34, (++_watchdog)&0x01);
}
