/**************************************************************************/ /**
 \file main.c

 Main Module Conditioner PCB

 \author 	Cyril Andreatta
 \author    Stefan Weber

 \copyright 2017 by radex AG, Switzerland. All rights reserved.
 
 Hardware Revisions:
 #g:    EEPROM
 #h:    Second thermistor directly on heater cartridge
 ******************************************************************************/

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "main.h"
#include "mcu.h"
#include <string.h>
#include "IOMux.h" 
#include "clk/clk.h"
#include "i2c.h"
#include "uart.h"
#include <stdio.h>
#include "pid.h"
#include "gpio/gpio.h"
#include "watchdog.h"
#include "temp_ctrl.h"
#include "pres.h"
#include "pump_ctrl.h"
#include "cond_def_head.h"
#include "version.h"
#include "comm.h"
#include "ctr.h"
#include "eeprom.h"
#include "debug_printf.h"
#include "i2c_bitbang.h"
#include "pres.h"

//defines --------------------------------------

#define TIMER_INTERVAL	10 // ms
//#define PERFORM_EEPROM_TEST

// Global variabels

SConditionerStat_mcu RX_Status;
SConditionerCfg_mcu	 RX_Config;

volatile uint32_t timer_second=0;

int		_test_received_char =0;
int		_test_received_msg  =0;
int		_overcurrent		=0;

/*****************************************************************************/
/* Function prototype                                                        */
/*****************************************************************************/
static void _tick___10ms(void);
static void _tick___50ms(void);
static void _tick__100ms(void);
static void _tick_1000ms(void);

static char _get_pcb_revision(void);
static int  _get_revision_voltage(void);
		
/**
 ******************************************************************************
 ** \brief  Main function of project for MB9B160L series.
 **
 ** \return uint32_t return value, if needed
 ******************************************************************************/

int32_t main(void) 
{   
	UINT32 returnCode;		// for systick init
	
	// Reset all values and set Conditioner mode to normal	
	memset(&RX_Status, 0, sizeof(RX_Status));
    memset(&RX_Config, 0, sizeof(RX_Config));
	
	// Init used Peripherals
    Iomux_Init();
	InitSwWatchdog();                          // Init Software Watchdog Timer  
    
	RX_Status.gpio_state.watchdog_reset=(FM4_CRG->RST_STR==SWDG_RESET);
	RX_Status.pressure_in	    = INVALID_VALUE;
	RX_Status.pressure_out	    = INVALID_VALUE;
	sscanf(version, "%d.%d.%d.%d", &RX_Status.version.major, &RX_Status.version.minor, &RX_Status.version.revision, &RX_Status.version.build);
	RX_Status.info.connected    = TRUE;
	RX_Status.pumptime     		= INVALID_VALUE;
    RX_Status.tempIn            = INVALID_VALUE;
    RX_Status.tempHeater        = INVALID_VALUE;
    RX_Status.error		        = 0;

	// Disable not used Peripheral
	Clk_PeripheralClockDisable(ClkGateUsb0);
	Clk_PeripheralClockDisable(ClkGateUsb1);
	Clk_PeripheralClockDisable(ClkGateDma);
	Clk_PeripheralClockDisable(ClkGateQprc0);
	Clk_PeripheralClockDisable(ClkGateQprc1);
	Clk_PeripheralClockDisable(ClkGateQprc2);
	Clk_PeripheralClockDisable(ClkGateQprc3);
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
	Clk_PeripheralClockDisable(ClkGateMft1);	
	Clk_PeripheralClockDisable(ClkGateMft2);		
	Clk_PeripheralClockDisable(ClkGateMft3);		
	Clk_PeripheralClockDisable(ClkGateAdc1);	
	Clk_PeripheralClockDisable(ClkGateAdc2);	
	Clk_PeripheralClockDisable(ClkGateAdc3);		
	
    // comment this line if you like to use bt0 for debugging
    Clk_PeripheralClockDisable(ClkGateBt0);


    /**
     * Clock calculation as defined in 'system_mb9bf16xl.h'
     *
     * PLLK (K) = PLL_CTL1[7..4] + 1 = 0x00 + 1 =  1
     * PLLM (M) = PLL_CTL1[3..0] + 1 = 0x03 + 1 =  4
     * PLLN (N) = PLL_CTL2[5..0] + 1 = 0x13 + 1 = 20
     * BSR      = BSC_PSR[3..0] (max. 1/16)     = 1/2
     *
     * PLL_OUT = CLK_IN * N * M = 4MHz * 20 * 4 = 320 MHz
     * CLKPLL  = PLL_IN * N     = 4MHz * 20     =  80 MHz
     * HCLK    = CLKPLL * BSR   = 80MHz * 1/2   =  40 MHz
     *
     * CLKMO    PLL_OUT     CLKPLL      HCLK
     * 4 MHz    320 MHz     80 MHz      40MHz
     *
     * CLKMO    Main Clock (from external or internal XTAL)
     * CLKPLL   Main PLL Clock = CLKMO * PLL Oscillation Circuit (K, M, N)
     * HCLK     Base Clock used for MCU
     *
     * SystemCoreClock = HCLK = PCLK = 40MHz => t = 25ns
     * Configure SysTick to generate an interrupt every 10ms
     **/

    // Init Board specific Peripherals
	Gpio1pin_Put(GPIO1PIN_P12,0); 		// Solenoid	OFF
	Gpio1pin_Put(GPIO1PIN_P80,0); 		// SolenoidR OFF
	Gpio1pin_Put(GPIO1PIN_P33,0); 		// Heater OFF

	comm_init(0);
	uart_init(115200);

	eeprom_init();
	// Print to debugger console if in DEBUG build
    RX_Status.pcb_rev = _get_pcb_revision();
 //   DBG_PRINTF("RX_Conditioner#%c\n", RX_Status.pcb_rev);
	       
	temp_init();
	pres_init();
    pump_init();
		
    // Read pumptime for Conditioner
	ctr_load();      

	//--- start timers ----------------------------
    returnCode = SysTick_Config(SystemCoreClock / (10 * TIMER_INTERVAL));
	if (returnCode != 0)
    {
        HALT();
	}

	while(1)
	{
		ClearSwWatchdog(); // Reload the watchdog timer
		
		RX_Status.gpio_state.u_24v_pg		= FM4_RTC->VBDIR_f.VDIR2;
		RX_Status.gpio_state.u_24v_flg		= FM4_RTC->VBDIR_f.VDIR3;
		RX_Status.gpio_state.heater_pg		= Gpio1pin_Get(GPIO1PIN_P11);
		RX_Status.gpio_state.heater_flg		= Gpio1pin_Get(GPIO1PIN_P61);
                		
//		if(RX_Status.gpio_state.u_24v_pg) 	RX_Status.error |= COND_ERR_power_24V;
	}
}

/**
 * Determine hardware revision depending on connected peripherals
 *
 * \return  ASCII character indicating PCB revision
 * e.g. R4_Conditioner_Board#g returns 'g'
 **/

static char _get_pcb_revision()
{
    char revision = '?';

    Gpio1pin_InitIn(GPIO1PIN_P80, Gpio1pin_InitPullup(1u));
    Gpio1pin_InitIn(GPIO1PIN_P00, Gpio1pin_InitPullup(1u));
    Gpio1pin_InitIn(GPIO1PIN_P23, Gpio1pin_InitPullup(1u));  

    // needed to read pin correctly    
 //   for (volatile int i = 0; i < 0xffff; i++)
 //       ;
    
    if (Gpio1pin_Get(GPIO1PIN_P80))
    {
        revision = 'e';
    }
    else if (Gpio1pin_Get(GPIO1PIN_P00))
	{        
		revision = 'g';
	}
	else
	{
		// P55 is floating on #f and pulled-up externally revision higher than #h
		Gpio1pin_InitOut(GPIO1PIN_P55, Gpio1pin_InitVal(0u));
		Gpio1pin_InitIn (GPIO1PIN_P55, Gpio1pin_InitPullup(0u));

        if (Gpio1pin_Get(GPIO1PIN_P55))
		{							  
			revision = 'h';	
			Gpio1pin_InitOut(GPIO1PIN_P80, Gpio1pin_InitVal(0u)); 
			
			// read revision voltage back from external adc chip (not present on rev. below 'n')
			int voltage=_get_revision_voltage();
			if((voltage>220)&&(voltage<280)) revision ='n';
			else if((voltage>340)&&(voltage<400)) revision ='s';
		}
        else
            revision = 'f';
    }
        
    switch (revision)
    {
        case 'e':
        case 'f':
            Gpio1pin_InitOut(GPIO1PIN_P00, Gpio1pin_InitVal(0u));
            Gpio1pin_InitOut(GPIO1PIN_P23, Gpio1pin_InitVal(0u));  
            Gpio1pin_InitOut(GPIO1PIN_P80, Gpio1pin_InitVal(0u));  
            break;
        
        case 'g':
            Gpio1pin_InitOut(GPIO1PIN_P00, Gpio1pin_InitVal(0u));
            Gpio1pin_InitIn (GPIO1PIN_P23, Gpio1pin_InitPullup(0u));  
            Gpio1pin_InitOut(GPIO1PIN_P80, Gpio1pin_InitVal(0u)); 
            break;
        
        case 'h':
            Gpio1pin_InitIn(GPIO1PIN_P00, Gpio1pin_InitPullup(0u));
            // P23 as analog input
            SetPinFunc_AN05();
            Gpio1pin_InitOut(GPIO1PIN_P80, Gpio1pin_InitVal(0u)); 
            break;

		case 'n':
		case 's':
			SetPinFunc_AN05();
			Gpio1pin_InitOut(GPIO1PIN_P60, Gpio1pin_InitVal(0u));
            Gpio1pin_InitIn (GPIO1PIN_P00, Gpio1pin_InitPullup(0u));
			Gpio1pin_InitIn (GPIO1PIN_P35, Gpio1pin_InitVal( 0u ) ); // SensorPowerFault
			Gpio1pin_Put(GPIO1PIN_P60, TRUE);  // turn on enable_new_gen
			break;
		
        default:
            // error
            DBG_PRINTF("Unknown PCB hardware revision #%c!\n", revision);
    }
    
    return revision;
}

//--- get_revision_voltage ----------------------
static int _get_revision_voltage(void)
{
	#define REVISION_VOLTAGE_ADDR		0x90	// 0x48 + R/W bit = 0x90/0x91

	int voltage;
	BYTE data[2]={0x0,0x0};

	i2c_bb_read_adc(REVISION_VOLTAGE_ADDR, data, 2);
	i2c_bb_read_adc(REVISION_VOLTAGE_ADDR, data, 2);

	voltage = data[0]<<8 | data[1];
	return voltage;		
}

void RxMessage_Handler(void)
{	
	//--- handle command flags -------------------------    
    if (RX_Config.cmd.reset_errors && !RX_Status.cmdConfirm.reset_errors) 
	{
		RX_Status.error = 0;
		RX_Status.aliveCfg = 0;
		RX_Status.aliveStat = 0;
	}
	RX_Status.cmdConfirm.reset_errors = RX_Config.cmd.reset_errors;
    
	if (RX_Config.cmd.save_eeprom && !RX_Status.cmdConfirm.save_eeprom)
	{
		ctr_save();
	}
	RX_Status.cmdConfirm.save_eeprom = RX_Config.cmd.save_eeprom;
			
	if (RX_Config.cmd.resetPumpTime && !RX_Status.cmdConfirm.resetPumpTime)
	{
		RX_Status.pumptime = 0;
		ctr_save();
	}
	RX_Status.cmdConfirm.resetPumpTime = RX_Config.cmd.resetPumpTime;
	
	//--- pump watchdog ------------------------
	pump_watchdog();
}

//--- SysTick_Handler --------------------------
void SysTick_Handler(void)
{ 
	static int time_ms=0;
	time_ms+=TIMER_INTERVAL;
	
	_tick___10ms();
	if ((time_ms%  50)==0) _tick___50ms();
	if ((time_ms% 100)==0) _tick__100ms();
	if ((time_ms%1000)==0) _tick_1000ms();
}

//--- _tick___10ms ---------------------------------------------
static void _tick___10ms(void)
{
	temp_tick_10ms();
	pres_tick_10ms();
	pump_tick_10ms();
    
    //DBG_PRINTF("%d %d %d %d %d %d %d\n", RX_Status.pressure_in, RX_Status.pressure_out, RX_Status.pump, RX_Status.pump_measured, RX_Status.meniscus, RX_Status.temp, RX_Status.tempHeater);
}

//--- _tick___50ms -------------------------------
static void _tick___50ms(void)
{
	temp_tick_50ms();
}

//--- _tick__100ms ---------------------
static void _tick__100ms(void)
{	
	//--- Send Status ---------------------------------
    UCHAR buffer[COMM_BUF_SIZE];
    int len;		
    comm_encode(&RX_Status, sizeof(RX_Status), buffer, sizeof(buffer), &len);	
    uart_write(buffer, len);

	RX_Status.alive++;
}

//--- _tick_1000ms -----------------------------------------
static void _tick_1000ms(void)
{
	pump_tick_1000ms();
	
	//--- check overcurrent ------------------
	if(RX_Status.gpio_state.u_24v_pg) _overcurrent++;
	else _overcurrent = 0;
	if (_overcurrent>3) RX_Status.error |= COND_ERR_power_24V;
}
