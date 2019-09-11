/******************************************************************************/
/** power.c
 ** 
 ** uart functions to communicate between fpga and conditioner board
 **
 **	Copyright 2017 by radex AG, Switzerland. All rights reserved.
 **	Written by Marcel Galliker
 **
 ******************************************************************************/

/*****************************************************************************/
/* Include files                                                             */
/*****************************************************************************/
#include "system.h"
#include "io.h"
#include "altera_avalon_pio_regs.h"
#include "AMC7891.h"
#include "trprintf.h"
#include "nios_def_head.h"
#include "watchdog.h"
#include "timer.h"
#include "power.h"

/*****************************************************************************/
/* Defines                                                                   */
/*****************************************************************************/

static int _amplifier_on = TRUE;
static int _v3_3_on		 = TRUE;
static int _error_delay	 = 0;
static int _power_on_timer;

#define ERROR_DELAY			20 		// *100 ms
#define POWER_ON_TIMEOUT	10		// *100 ms
#define POWER_OFF_TIMEOUT	100		// *100 ms

/*****************************************************************************/
/* Function prototype                                                        */
/*****************************************************************************/

static int  _power_test_v3_3(void);
static void _power_amplifier(int on);
static void _power_v3_3(int on);
static void _stop_fpga(void);

static int  _power_all_off_test(void);
static int  _power_amp_on_test(void);
static int  _power_all_on_test(void);

//--- power_init -----------------------------------------
void power_init(void)
{
	pRX_Config->cmd.shutdown = TRUE;
	pRX_Status->powerState = power_off;
	_power_amplifier(FALSE);
	_power_v3_3(FALSE);
	_error_delay  = POWER_OFF_TIMEOUT;
	pRX_Status->powerState = power_wait_all_off;
}

//--- _stop_fpga -------------------------------------
static void _stop_fpga(void)
{
	IOWR_ALTERA_AVALON_PIO_DATA(PIO_FPGA_SHUTDOWN_BASE,1);
}

//--- power_tick_100ms --------------------------------
void power_tick_100ms(void)
{
	volatile UINT16 val=0;
	int				dummy;
	int				error;
	int				critical_error;

	if (IORD_ALTERA_AVALON_PIO_DATA(PIO_OVERHEAT_BASE))
		pRX_Status->error.fpga_overheated=TRUE;

	error = pRX_Config->cmd.shutdown
			|| pRX_Status->error.fpga_overheated
			|| pRX_Status->error.cooler_overheated
			|| pRX_Status->error.power_all_off_timeout
			|| pRX_Status->error.power_amp_on_timeout
			|| pRX_Status->error.power_all_on_timeout
			|| pRX_Status->error.u_plus_2v5
			|| pRX_Status->error.arm_timeout
			;

	// shutdown request which is NOT from ARM
	critical_error =  pRX_Status->error.fpga_overheated
			|| pRX_Status->error.cooler_overheated
			|| pRX_Status->error.power_all_off_timeout
			|| pRX_Status->error.power_amp_on_timeout
			|| pRX_Status->error.power_all_on_timeout
			|| pRX_Status->error.u_plus_2v5
			|| pRX_Status->error.arm_timeout
			;

	if ((pRX_Status->powerState < power_sd) && (error || !pRX_Config->cmd.firepulse_on))
	{
		if (critical_error && !pRX_Config->cmd.debug)
			pRX_Status->powerState = power_sd_fpga;
		else
			pRX_Status->powerState = power_sd;
	}

	IOWR_ALTERA_AVALON_PIO_DATA(PIO_ALL_ON_EN_BASE,pRX_Status->powerState  == power_all_on
							 	 	 	 	 	 || pRX_Status->powerState == power_pre_all_on
												 || pRX_Status->powerState == power_sd_fpga);

	if (_error_delay > 0) _error_delay--;

	// start converting -36v
	dummy = IORD_16DIRECT(AMC7891_0_BASE,AMC7891_ADC0_DATA) & 0x3ff;
	dummy++;	// to ignore warning

	// result=36V / start converting -5V
	val = IORD_16DIRECT(AMC7891_0_BASE,AMC7891_ADC1_DATA) & 0x3ff;
	pRX_Status->u_firepulse = 109*(val-978);

	// result=-5V / start converting +5V
	val = IORD_16DIRECT(AMC7891_0_BASE,AMC7891_ADC2_DATA) & 0x3ff;
	pRX_Status->u_minus_5v		= (146*val-100000)/10;

	// result=+5V / start converting -2.5V
	val = IORD_16DIRECT(AMC7891_0_BASE,AMC7891_ADC3_DATA) & 0x3ff;
	pRX_Status->u_plus_5v			= val*5000/1024;

	val = IORD_16DIRECT(AMC7891_0_BASE,AMC7891_ADC4_DATA) & 0x3ff;
	pRX_Status->u_plus_2v5		= val*5000/1024;

	if(pRX_Status->u_plus_2v5 <=2000   || pRX_Status->u_plus_2v5 >=  3000) if (!_error_delay)pRX_Status->error.u_plus_2v5 =TRUE;

	switch(pRX_Status->powerState)
	{
	case power_off:				_error_delay  = ERROR_DELAY;
								break;

	case power_wait_all_off:	if (!_power_all_off_test() && pRX_Config->cmd.firepulse_on)
								{
									_power_amplifier(TRUE);
									_error_delay  = ERROR_DELAY;
									pRX_Status->powerState = power_wait_amp_on;
								}
								break;

	case power_wait_amp_on:		watchdog_toggle(WATCHDOG_TIME_MS, WATCHDOG_CHECK_FP);
								if (_power_amp_on_test())
								{
									_power_v3_3(TRUE);
									_error_delay  = ERROR_DELAY;
									_power_on_timer = POWER_ON_TIMEOUT;
									pRX_Status->powerState = power_wait_all_on;
								}
								break;

	case power_wait_all_on:		watchdog_toggle(WATCHDOG_TIME_MS, WATCHDOG_CHECK_FP);
								if(_power_all_on_test())
								{
									_error_delay  = 2;	//200ms 11.02.19 Stefan
									pRX_Status->powerState = power_pre_all_on;
								}
								break;

	case power_pre_all_on:		watchdog_toggle(WATCHDOG_TIME_MS, WATCHDOG_CHECK_FP);		//new state for power up, 11.02.19 Stefan
								if(_error_delay==0)
								{
									if(_power_all_on_test())
									{
										pRX_Status->powerState = power_all_on;
									}
								}
								break;

	case power_all_on:			_power_all_on_test();
								watchdog_toggle(WATCHDOG_TIME_MS, WATCHDOG_CHECK_FP);
								break;

    // --- SHUT-DOWN ------------------------------------------------------
	case power_sd_fpga:			_stop_fpga();	// does not allow debugging with breakpoints!!!
								pRX_Status->powerState = power_sd;
								break;

	case power_sd:				pRX_Status->powerState = power_sd_3v3;
								pRX_Config->cmd.shutdown = 0;
								watchdog_toggle(WATCHDOG_TIME_MS, WATCHDOG_CHECK_FP);
								break;

	case power_sd_3v3:			_power_v3_3(FALSE);
								_error_delay  = 2;	//200ms 11.02.19 Stefan
								watchdog_toggle(WATCHDOG_TIME_MS, WATCHDOG_CHECK_FP);
								pRX_Status->powerState = power_sd_amp;
								break;

	case power_sd_amp:			if(_power_test_v3_3() == 0x00)	// all 3V3float off
								{
									_power_amplifier(FALSE);
									pRX_Status->info.is_shutdown = 1;
									pRX_Status->powerState = power_down;
								}
								break;

	case power_down:			if (!error && pRX_Config->cmd.firepulse_on)
								{
									pRX_Status->powerState = power_wait_all_off;
								}
								break;
	}
}

//--- _power_v3_3 ---------------------------------------------
static void _power_v3_3(int on)
{
	if (on!=_v3_3_on)
	{
		if (on)
			IOWR_16DIRECT(AMC7891_0_BASE,AMC7891_GPIO_OUT,0x00FF);
		else
			IOWR_16DIRECT(AMC7891_0_BASE,AMC7891_GPIO_OUT,0x0000);

		_v3_3_on 	= on;
	}
}

//--- _power_amplifier --------------------------------
static void _power_amplifier(int on)
{
	if (on!=_amplifier_on)
	{
		if (on)
			watchdog_toggle(WATCHDOG_TIME_MS, WATCHDOG_CHECK_FP); //watchdog_start_debug_mode();
		else
			watchdog_init();

		_amplifier_on = on;
	}
}

//--- _power_test_v3_3 -----------------------------------
static int _power_test_v3_3(void)
{
	return (IORD_ALTERA_AVALON_PIO_DATA(HEAD_CON_ERR_IN_BASE));
}

//--- _power_all_off_test ------------------------------------------------
static int _power_all_off_test(void)
{
	int on = FALSE;

	if(_power_test_v3_3() != 0x00)      {on=TRUE; if (!_error_delay) pRX_Status->error.u_plus_3v3  = TRUE;}
	if(pRX_Status->u_firepulse < -5000) {on=TRUE; if (!_error_delay) pRX_Status->error.u_firepulse = TRUE;}
	if(pRX_Status->u_minus_5v  < -4000) {on=TRUE; if (!_error_delay) pRX_Status->error.u_minus_5v  = TRUE;}
	if(pRX_Status->u_plus_5v   >  4000) {on=TRUE; if (!_error_delay) pRX_Status->error.u_plus_5v   = TRUE;}

	if(!_error_delay && on == TRUE) pRX_Status->error.power_all_off_timeout=TRUE;

	return on;
}

//--- _power_amp_on_test ------------------------------------------------
static int _power_amp_on_test(void)
{
	int on = TRUE;

	if(_power_test_v3_3() != 0x00)                                         {on=FALSE; if (!_error_delay)pRX_Status->error.u_plus_3v3  = TRUE;}
	if(pRX_Status->u_firepulse>=(-36000+5000) || pRX_Status->u_firepulse<=(-48000-5000))
	{
		on=FALSE;
		pRX_Status->info.u_firepulse_48V = (pRX_Status->u_firepulse<(-48000+8000));
		if (!_error_delay)pRX_Status->error.u_firepulse = TRUE;
	}
	if(pRX_Status->u_minus_5v >= -4000 || pRX_Status->u_minus_5v <= -6000) {on=FALSE; if (!_error_delay)pRX_Status->error.u_minus_5v  = TRUE;}
	if(pRX_Status->u_plus_5v  <=  4000 || pRX_Status->u_plus_5v  >=  6000) {on=FALSE; if (!_error_delay)pRX_Status->error.u_plus_5v   = TRUE;}

	if(!_error_delay && on == FALSE) pRX_Status->error.power_amp_on_timeout=TRUE;

	return on;
}

//--- _power_all_on_test ------------------------------------------------
static int _power_all_on_test(void)
{
	int on = TRUE;
	if (_power_on_timer) --_power_on_timer;

	if(_power_test_v3_3() != 0x0f)                                         {on=FALSE; if (!_power_on_timer) pRX_Status->error.u_plus_3v3  = TRUE;}
	if(pRX_Status->u_firepulse>=-30000 || pRX_Status->u_firepulse<=-54000) {on=FALSE; if (!_power_on_timer) pRX_Status->error.u_firepulse = TRUE;}
	if(pRX_Status->u_minus_5v >= -4000 || pRX_Status->u_minus_5v <= -6000) {on=FALSE; if (!_power_on_timer) pRX_Status->error.u_minus_5v  = TRUE;}
	if(pRX_Status->u_plus_5v  <=  4000 || pRX_Status->u_plus_5v  >=  6000) {on=FALSE; if (!_power_on_timer) pRX_Status->error.u_plus_5v   = TRUE;}

	if (on) _power_on_timer = POWER_ON_TIMEOUT;

	if(!_power_on_timer && on == FALSE) pRX_Status->error.power_all_on_timeout=TRUE;

	return on;
}
