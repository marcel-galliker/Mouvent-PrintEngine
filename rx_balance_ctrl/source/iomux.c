/******************************************************************************/
/** \file iomux.c
 ** 
 ** basic peripheral initialisation
 **
 **	Copyright 2016 by radex AG, Switzerland. All rights reserved.
 **	Written by Stefan Weber
 **
 ******************************************************************************/

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "iomux.h"

/**
 ******************************************************************************
 ** \addtogroup IomuxGroup
 ******************************************************************************/
//@{

/******************************************************************************/
/* Local pre-processor symbols/macros ('#define')                             */
/******************************************************************************/

/******************************************************************************/
/* Global variable definitions (declared in header file with 'extern')        */
/******************************************************************************/

/******************************************************************************/
/* Local type definitions ('typedef')                                         */
/******************************************************************************/

/******************************************************************************/
/* Local function prototypes ('static')                                       */
/******************************************************************************/
static void Iomux_ADC0_Init(void);

static void Iomux_GPIO_Init(void);

static void Iomux_DEBUG_Init(void);

static void Iomux_MFS3_I2C_Init(void);

static void Iomux_MFS2_UART_Init(void);

static void Iomux_MFS0_UART_Init(void);

/******************************************************************************/
/* Local variable definitions ('static')                                      */
/******************************************************************************/

/*****************************************************************************/
/* Function implementation - global ('extern') and local ('static')          */
/*****************************************************************************/

/**
 *****************************************************************************
 ** \brief This function initializes BT0
 **
 *****************************************************************************/
static void BT0_Init(void)
{
	// select reload timer function
	FM4_BT0_RT->TMCR_f.FMD0=1;
	FM4_BT0_RT->TMCR_f.FMD1=1;
	FM4_BT0_RT->TMCR_f.FMD2=0;
	
	// count clock selection 0.25MHz/256=976.53Hz
	FM4_BT0_RT->TMCR2_f.CKS3=0;
	FM4_BT0_RT->TMCR_f.CKS2=1;
	FM4_BT0_RT->TMCR_f.CKS1=0;
	FM4_BT0_RT->TMCR_f.CKS0=0;	
	
	// operating mode selection
	FM4_BT0_RT->TMCR_f.MDSE=0;	// reload mode
	
	// load pcsr value
	FM4_BT0_RT->PCSR=0xfff;

  // Setting up interrupt and its priority
  NVIC_SetPriority(BT0_IRQn, 7u);           			
  NVIC_ClearPendingIRQ(BT0_IRQn);           								
  NVIC_EnableIRQ(BT0_IRQn);
	
	// clear interrupt flac
	FM4_BT0_RT->STC_f.TGIR=0; 
	FM4_BT0_RT->STC_f.UDIR=0; 

	// enable interrupt
	FM4_BT0_RT->STC_f.UDIE=1;
	
	// start timer
	FM4_BT0_RT->TMCR_f.CTEN=1;
}

/**
 ******************************************************************************
 ** \brief  Base Timer ch. 0 Interrupt Handler
 ******************************************************************************/
void BT0_IRQHandler(void)
{ 
	static int toogle_pump=0;
	static int count=0;
	count++;
  FM4_BT0_RT->STC_f.UDIR = 0u;                             // Clear the interrupt flag
	FM4_BT0_RT->STC_f.TGIR = 0u;                             // Clear the interrupt flag
	if(count>=39)		//1.024ms *39 =39.93ms~25.04Hz*2=50.08Hz
	{
		if(toogle_pump==0)	
		{
			toogle_pump=1; 
			pump_pwm_0(1); 		// pump_pwm_0 ON
			pump_pwm_1(0); 		// pump_pwm_1 OFF
		}
		else 
		{
			toogle_pump=0;
			pump_pwm_0(0); 		// pump_pwm_0 OFF
			pump_pwm_1(1); 		// pump_pwm_1 ON
		}
		count=0;
	}
}

/**
 *****************************************************************************
 ** \brief This function initializes ADC0
 **
 *****************************************************************************/
static void Iomux_ADC0_Init(void)
{
	// Init ADC0 pin function
	SetPinFunc_AN01();    // Set ADC analog input pin
	SetPinFunc_AN02();    // Set ADC analog input pin
	SetPinFunc_AN05();    // Set ADC analog input pin
}

/**
 *****************************************************************************
 ** \brief This function initializes GPIO
 **
 *****************************************************************************/
static void Iomux_GPIO_Init(void)
{
	// Init GPIO pin function
	Gpio1pin_InitOut( GPIO1PIN_P10, Gpio1pin_InitVal( 0u ) );		// prev_led_0 	OUTPUT
	Gpio1pin_InitOut( GPIO1PIN_P11, Gpio1pin_InitVal( 0u ) );		// prev_led_1 	OUTPUT
	Gpio1pin_InitOut( GPIO1PIN_P12, Gpio1pin_InitVal( 0u ) );		// lamp_switch 	OUTPUT 8mA
	Gpio1pin_InitOut( GPIO1PIN_P32, Gpio1pin_InitVal( 0u ) );		// pump_pwm_0 	OUTPUT
	Gpio1pin_InitOut( GPIO1PIN_P33, Gpio1pin_InitVal( 0u ) );		// pump_pwm_1 	OUTPUT
	Gpio1pin_InitOut( GPIO1PIN_P34, Gpio1pin_InitVal( 0u ) );		// pump_rtry 		OUTPUT
	Gpio1pin_InitIn( GPIO1PIN_P35, Gpio1pin_InitPullup( 0u ) );	// pump_flt 		INPUT
	Gpio1pin_InitOut( GPIO1PIN_P54, Gpio1pin_InitVal( 0u ) );		// next_led_0 	OUTPUT
	Gpio1pin_InitOut( GPIO1PIN_P55, Gpio1pin_InitVal( 0u ) );		// next_led_1 	OUTPUT
}


/**
 *****************************************************************************
 ** \brief This function initializes DEBUG
 **
 *****************************************************************************/
static void Iomux_DEBUG_Init(void)
{
	// Init DEBUG pin function
	bFM4_GPIO_PFR0_P1 = 1u;
	bFM4_GPIO_PFR0_P3 = 1u;
	bFM4_GPIO_PFR0_P4 = 1u;
	PINRELOC_SET_EPFR( FM4_GPIO->EPFR00, 16u, 1u, 0x1u );
}

/**
 *****************************************************************************
 ** \brief This function initializes MFS3_I2C
 **
 *****************************************************************************/
static void Iomux_MFS3_I2C_Init(void)
{
	// Init MFS3_I2C pin function
	// Init channel 3
	SetPinFunc_SCK3_0();
	SetPinFunc_SOT3_0();	
	
	// Initialize UART I2C mode
  MFS3.BGR  = 2000000 / 100000 - 1;  // 100k Bit/s @ 2 MHz
  MFS3.SCR |= 0x80;     							// Reset UART3 (UPCL = 1)
  MFS3.SMR  = 0x80;      							// operation mode 4, no interrupts, no test
																			// mode, no wake up
}

/**
 *****************************************************************************
 ** \brief This function initializes MFS0_UART
 **
 *****************************************************************************/
static void Iomux_MFS0_UART_Init(void)
{
	// Init MFS0_UART pin function
	// Init channel 0
	SetPinFunc_SIN0_0();
	SetPinFunc_SOT0_0();
}

/**
 *****************************************************************************
 ** \brief This function initializes MFS2_UART
 **
 *****************************************************************************/
static void Iomux_MFS2_UART_Init(void)
{
	// Init MFS2_UART pin function
	// Init channel 0
	SetPinFunc_SIN2_0();
	SetPinFunc_SOT2_0();
}
	
/**
 *****************************************************************************
 ** \brief This function initializes the GPIO ports and peripheral pin
 **        functions according to the user settings of the Pin Wizard.
 **
 *****************************************************************************/
void Iomux_Init(void)
{	
	Iomux_DEBUG_Init();

	Iomux_GPIO_Init();
	
	Iomux_MFS3_I2C_Init();
	
	Iomux_MFS0_UART_Init();
	
	Iomux_MFS2_UART_Init();
	
	Iomux_ADC0_Init();	
	
	BT0_Init();
}

void prev_led_0(int state)
{
	Gpio1pin_Put(GPIO1PIN_P10, state);
}

void prev_led_1(int state)
{
	Gpio1pin_Put(GPIO1PIN_P11, state);
}

void next_led_0(int state)
{
	Gpio1pin_Put(GPIO1PIN_P54, state);
}

void next_led_1(int state)
{
	Gpio1pin_Put(GPIO1PIN_P55, state);
}

void lamp_switch(int state)
{
	Gpio1pin_Put(GPIO1PIN_P12, state);
}

void pump_pwm_0(int state)
{
	Gpio1pin_Put(GPIO1PIN_P32, state);
}

void pump_pwm_1(int state)
{
	Gpio1pin_Put(GPIO1PIN_P33, state);
}

void pump_rtry(int state)
{
	Gpio1pin_Put(GPIO1PIN_P34, state);
}

//@} // IomuxGroup

/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
