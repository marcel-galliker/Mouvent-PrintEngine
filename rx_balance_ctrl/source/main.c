/******************************************************************************/
/** \file main.c
 ** 
 ** Main Module
 **
 **	Copyright 2016 by radex AG, Switzerland. All rights reserved.
 **	Written by Stefan Weber
 **
 ******************************************************************************/

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "mcu.h"
#include <string.h>
#include "iomux.h" 
#include "clk/clk.h"
#include <stdio.h>
#include "gpio/gpio.h"
#include "watchdog.h"
#include "version.h"
#include "balance.h"
#include "balance_def.h"
#include "comm.h"
#include "uart.h"
#include "fifo.h"
#include "rx_ink_common.h"
#include "sensor.h"

//defines --------------------------------------

#define TIMER_INTERVAL	10 // ms

// Global variabels

SBalanceCfg		RX_BalanceCfg [BALANCE_CNT];
SBalanceStat	RX_BalanceStat[BALANCE_CNT];

volatile UINT32 weight[3];

/*****************************************************************************/
/* Function prototype                                                        */
/*****************************************************************************/
static void _tick___50ms(void);
static void _tick__300ms(void);
static void _tick_500ms(void);
int _sensor_measure(int command);
int _sensor_read(int command);
static void _send_status(void);
static void _handle_cfg(void);
void RxMessage_Handler_Cfg(void);
void RxMessage_Handler_Stat(void);
char        check_revision(void);

#define SWDG_RESET        0x0010u	
		
/**
 ******************************************************************************
 ** \brief  Main function of project for MB9B160L series.
 **
 ** \return uint32_t return value, if needed
 ******************************************************************************/

int32_t main(void) 
{
	uint32_t returnCode;		                // for systick init
	int i;
	UCHAR data;
		
	InitSwWatchdog();                          // Init Software Watchdog Timer  
	
	// Disable not used Peripheral
	Clk_PeripheralClockDisable(ClkGateExtif);	
	Clk_PeripheralClockDisable(ClkGateUsb0);
	Clk_PeripheralClockDisable(ClkGateUsb1);
	Clk_PeripheralClockDisable(ClkGateDma);
	Clk_PeripheralClockDisable(ClkGateQprc0);
	Clk_PeripheralClockDisable(ClkGateQprc1);
	Clk_PeripheralClockDisable(ClkGateQprc2);
	Clk_PeripheralClockDisable(ClkGateQprc3);
	Clk_PeripheralClockDisable(ClkGateBt4);
	Clk_PeripheralClockDisable(ClkGateBt8);
	Clk_PeripheralClockDisable(ClkGateBt12);	
	Clk_PeripheralClockDisable(ClkGateSdIf);
	Clk_PeripheralClockDisable(ClkGateCan0);
	Clk_PeripheralClockDisable(ClkGateCan1);
	Clk_PeripheralClockDisable(ClkGateMfs1);
	Clk_PeripheralClockDisable(ClkGateMfs4);
	Clk_PeripheralClockDisable(ClkGateMfs5);
	Clk_PeripheralClockDisable(ClkGateMfs6);
	Clk_PeripheralClockDisable(ClkGateMfs7);
	Clk_PeripheralClockDisable(ClkGateMfs8);	
	Clk_PeripheralClockDisable(ClkGateMfs9);	
	Clk_PeripheralClockDisable(ClkGateMfs10);	
	Clk_PeripheralClockDisable(ClkGateMfs11);		
	Clk_PeripheralClockDisable(ClkGateMfs12);	
	Clk_PeripheralClockDisable(ClkGateMfs13);	
	Clk_PeripheralClockDisable(ClkGateMfs14);	
	Clk_PeripheralClockDisable(ClkGateMfs15);	
	Clk_PeripheralClockDisable(ClkGateMft0);	
	Clk_PeripheralClockDisable(ClkGateMft1);	
	Clk_PeripheralClockDisable(ClkGateMft2);		
	Clk_PeripheralClockDisable(ClkGateMft3);		
	Clk_PeripheralClockDisable(ClkGateAdc1);	
	Clk_PeripheralClockDisable(ClkGateAdc2);	
	Clk_PeripheralClockDisable(ClkGateAdc3);		
		
	memset(&RX_BalanceCfg, 0, sizeof(RX_BalanceCfg));	
	memset(&RX_BalanceStat, 0, sizeof(RX_BalanceStat));	
		
	// Init Systick
	returnCode = SysTick_Config(SystemCoreClock / (10*TIMER_INTERVAL));     // Configure SysTick to generate an interrupt every 1ms 
	if (returnCode != 0)  
	{                                   									// Check return code for errors 
		while(1);	    // Error Handling 
	}
	NVIC_SetPriority(SysTick_IRQn, 10u);	
	
	Iomux_Init(); 		// Init used Peripherals
		
	// Init Board specific Peripherals
	prev_led_0(0);		// prev_led_0	OFF
	prev_led_1(0); 		// prev_led_1 OFF
	lamp_switch(0); 	// lamp_switch OFF
	pump_pwm_0(0); 		// pump_pwm_0 OFF
	pump_pwm_1(0); 		// pump_pwm_1 OFF
	pump_rtry(0); 		// pump_rtry OFF
	next_led_0(0); 		// next_led_0 OFF
	next_led_1(0); 		// next_led_1 OFF
		
    RX_BalanceStat->board_revision=check_revision();
    
	for(i=0;i<SENSOR_CNT;i++) // 3 Sensors 
    {
		adc_init(i);
	}
	uart_init(9600);
	
	FM4_BT0_RT->TMCR_f.STRG=1;	// Trigger BT0

	while(1)
	{
		if(uart_read_prev(&data))
		{	
			if (comm_received(0, data))
			{
				RxMessage_Handler_Cfg();
			}
		}
		if(uart_read_next(&data))
		{
			if (comm_received(1, data))
			{
				RxMessage_Handler_Stat();	
			}
		}
	}
}

//--- SysTick_Handler --------------------------
void SysTick_Handler(void)
{ 
	static int time_ms=0;

	time_ms+=TIMER_INTERVAL;
	
	if ((time_ms%  50)==0) _tick___50ms();
	if ((time_ms% 300)==0) _tick__300ms();
	if ((time_ms%500)==0) _tick_500ms();
	
	ClearSwWatchdog();                      // Reload the watchdog timer
}

//--- _tick___50ms -------------------------------
static void _tick___50ms(void)
{
	FM4_ADC0->SCCR_f.SSTR = 1u;     		// Trigger AD Conversion
}

//--- _tick__300ms ---------------------
static void _tick__300ms(void)
{	
	sensor_state_machine();
}

//--- _tick_500ms -----------------------------------------
static void _tick_500ms(void)
{	
	uint32_t pump_flt=0;
	pump_flt=Gpio1pin_Get(GPIO1PIN_P35);	//pump_flt
	RX_BalanceStat[0].err.pump=!pump_flt;
    
    RX_BalanceStat[0].balance[0]=weight[0];
    RX_BalanceStat[0].balance[1]=weight[1];
    RX_BalanceStat[0].balance[2]=weight[2];
	
	//--- Send Status ---------------------------------
	RX_BalanceStat[0].alive++;

	_send_status();
}

//--- RxMessage_Handler_Cfg -----------------------------
void RxMessage_Handler_Cfg(void)
{	
	static BYTE _CfgBuf[sizeof(RX_BalanceCfg)];
	
	int length;
	if (comm_get_data(0, _CfgBuf, sizeof(RX_BalanceCfg), &length))
	{
		int size = sizeof(SBalanceCfg);		
		int cnt = length/size;
		if (length%size==0 && cnt<=BALANCE_CNT)
		{
			memcpy(RX_BalanceCfg, _CfgBuf, length);
			
			RX_BalanceStat[0].prev_msg_received++;
			if (cnt>1)
			{	
				static	UCHAR buf[COMM_BUF_SIZE];
				int len;
				if (comm_encode(&RX_BalanceCfg[1], (cnt-1)*sizeof(SBalanceCfg), buf, sizeof(buf), &len))
				{
					RX_BalanceStat[0].next_msg_sent++;
					uart_write_next(buf, len);
				}	
			}
			_handle_cfg();
		}
	}
}

//--- _handle_cfg --------------------------------------------------
static void _handle_cfg(void)
{
	//----------------------------------------------------------------------------------
	lamp_switch(RX_BalanceCfg[0].cmd.lamp);				// lamp_switch 
	next_led_0(RX_BalanceCfg[0].cmd.led_next_0); 		// next_led_0
	next_led_1(RX_BalanceCfg[0].cmd.led_next_1);
	prev_led_0(RX_BalanceCfg[0].cmd.led_prev_0);
	prev_led_1(RX_BalanceCfg[0].cmd.led_prev_1); 		// next_led_1
	
}

//--- RxMessage_Handler_Stat ---------------------------
void RxMessage_Handler_Stat(void)
{
	static BYTE _StatBuf[sizeof(RX_BalanceStat)];
	int length;
	if (comm_get_data(1, _StatBuf, sizeof(_StatBuf), &length))
	{	
		if (length==sizeof(RX_BalanceStat))
		{		
			memcpy(&RX_BalanceStat[1], _StatBuf, sizeof(RX_BalanceStat)-sizeof(SBalanceStat));
			RX_BalanceStat[0].next_msg_received++;
		}
	}
}

//--- _send_status ----------------------------------------------------
static void _send_status(void)
{
	static UCHAR buf[COMM_BUF_SIZE];
	int len;
	if (comm_encode(RX_BalanceStat, sizeof(RX_BalanceStat), buf, sizeof(buf), &len))
	{
		uart_write_prev(buf, len);
		RX_BalanceStat[0].prev_msg_send++;
	}
}

/**
 * Check PCB revision
 *
 * \return  ASCII character indicating PCB revision 
 * e.g. R4_Balance/Ventilation Board#c returns 'c'
 **/
char check_revision()
{
    char revision = 'b';

    // set pull-up for P66
    Gpio1pin_InitIn(GPIO1PIN_P66, Gpio1pin_InitPullup(1u));

    // Check if pin P66 is tied to GND for revision #c
    if (!Gpio1pin_Get(GPIO1PIN_P66))
        revision = 'c';

    return revision;
}
