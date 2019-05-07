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

//static void Iomux_BT_Init(void); //only used for debug, else 	toogle_watchdog(watchdog); used !

static void Iomux_DAC_Init(void);

static void Iomux_MFS2_I2C_Init(void);

static void Iomux_MFS3_I2C_Init(void);

static void Iomux_MFS0_UART_Init(void);

/******************************************************************************/
/* Local variable definitions ('static')                                      */
/******************************************************************************/

/*****************************************************************************/
/* Function implementation - global ('extern') and local ('static')          */
/*****************************************************************************/


/**
 *****************************************************************************
 ** \brief This function initializes ADC0
 **
 *****************************************************************************/
static void Iomux_ADC0_Init(void)
{
	// Init ADC0 pin function
	SetPinFunc_AN02();    // Set ADC analog input pin
}

/**
 *****************************************************************************
 ** \brief This function initializes GPIO
 **
 *****************************************************************************/
static void Iomux_GPIO_Init(void)
{
	// inputs
    Gpio1pin_InitIn( GPIO1PIN_P61, 		Gpio1pin_InitVal( 0u ) ); // HeaterFLG
	Gpio1pin_InitIn( GPIO1PIN_P11, 		Gpio1pin_InitVal( 0u ) ); // HeaterPG	
    Gpio1pin_InitIn( GPIO1PIN_P13, 		Gpio1pin_InitVal( 0u ) ); // Solenoid_Status
    Gpio1pin_InitIn( GPIO1PIN_P81, 		Gpio1pin_InitVal( 0u ) ); // SolenoidR_Status
    Gpio1pin_InitIn( GPIO1PIN_P46, 		Gpio1pin_InitVal( 0u ) ); // 24V_FLG
	Gpio1pin_InitIn( GPIO1PIN_P47, 		Gpio1pin_InitVal( 0u ) ); // 24V_PG	
    
    // outputs
	Gpio1pin_InitOut( GPIO1PIN_P33, 	Gpio1pin_InitVal( 0u ) ); // Heater
	Gpio1pin_InitOut( GPIO1PIN_P12, 	Gpio1pin_InitVal( 0u ) ); // Solenoid
	Gpio1pin_InitOut( GPIO1PIN_P80, 	Gpio1pin_InitVal( 0u ) ); // SolenoidR
	Gpio1pin_InitOut( GPIO1PIN_P00, 	Gpio1pin_InitVal( 0u ) ); // 24VEnable
	Gpio1pin_InitOut( GPIO1PIN_P30, 	Gpio1pin_InitVal( 0u ) ); // DS2_3V3
	Gpio1pin_InitOut( GPIO1PIN_P35, 	Gpio1pin_InitVal( 0u ) ); // DS1_3V3
	Gpio1pin_InitOut( GPIO1PIN_P34, 	Gpio1pin_InitVal( 0u ) ); // Watchdog
    Gpio1pin_InitOut( GPIO1PIN_P02, 	Gpio1pin_InitVal( 0u ) ); // Reserve
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
 ** \brief This function initializes BT
 **  only used for debug, else toogle_watchdog(watchdog); is used !
 *****************************************************************************/
#if (0)
static void Iomux_BT_Init(void)
{
	//stc_bt_ppg_config_t stcPpgConfig;
	
	// Init BT pin function
	// Init Base Timer BT_I/O Mode0 pin function
	//TIOA6_0 => PCB rev. f !
	
	SetPinFunc_TIOA6_0_OUT();
	//	SetPinFunc_TIOA4_0_OUT();
	
	/*** BT IO Select Function ******************************/
	
	// Select Register BTSEL0123 => Standard 16-bit Timer mode
	FM4_BTIOSEL03->BTSEL0123_f.SEL23_0 = 0;
	FM4_BTIOSEL03->BTSEL0123_f.SEL23_1 = 0;
	FM4_BTIOSEL03->BTSEL0123_f.SEL23_2 = 0;
	FM4_BTIOSEL03->BTSEL0123_f.SEL23_3 = 0;
	
	FM4_BTIOSEL03->BTSEL0123_f.SEL01_0 = 0;	
	FM4_BTIOSEL03->BTSEL0123_f.SEL01_1 = 0;	
	FM4_BTIOSEL03->BTSEL0123_f.SEL01_2 = 0;
	FM4_BTIOSEL03->BTSEL0123_f.SEL01_3 = 0;	
		
	// Select Register BTSEL4567 => Standard 16-bit Timer mode
	FM4_BTIOSEL47->BTSEL4567_f.SEL67_0 = 0;
	FM4_BTIOSEL47->BTSEL4567_f.SEL67_1 = 0;
	FM4_BTIOSEL47->BTSEL4567_f.SEL67_2 = 0;
	FM4_BTIOSEL47->BTSEL4567_f.SEL67_3 = 0;
	
	FM4_BTIOSEL47->BTSEL4567_f.SEL45_0 = 0;
	FM4_BTIOSEL47->BTSEL4567_f.SEL45_1 = 0;
	FM4_BTIOSEL47->BTSEL4567_f.SEL45_2 = 0;
	FM4_BTIOSEL47->BTSEL4567_f.SEL45_3 = 0;

	// Select Register BTSEL89AB => Standard 16-bit Timer mode
	FM4_BTIOSEL8B->BTSEL89AB_f.SELAB_0 = 0;
	FM4_BTIOSEL8B->BTSEL89AB_f.SELAB_1 = 0;
	FM4_BTIOSEL8B->BTSEL89AB_f.SELAB_2 = 0;
	FM4_BTIOSEL8B->BTSEL89AB_f.SELAB_3 = 0;	
	
	FM4_BTIOSEL8B->BTSEL89AB_f.SEL89_0 = 0;
	FM4_BTIOSEL8B->BTSEL89AB_f.SEL89_1 = 0;
	FM4_BTIOSEL8B->BTSEL89AB_f.SEL89_2 = 0;
	FM4_BTIOSEL8B->BTSEL89AB_f.SEL89_3 = 0;

	// Select Register BTSELCDEF => Standard 16-bit Timer mode
	FM4_BTIOSELCF->BTSELCDEF_f.SELEF_0 = 0;
	FM4_BTIOSELCF->BTSELCDEF_f.SELEF_1 = 0;
	FM4_BTIOSELCF->BTSELCDEF_f.SELEF_2 = 0;
	FM4_BTIOSELCF->BTSELCDEF_f.SELEF_3 = 0;
	
	FM4_BTIOSELCF->BTSELCDEF_f.SELCD_0 = 0;
	FM4_BTIOSELCF->BTSELCDEF_f.SELCD_1 = 0;
	FM4_BTIOSELCF->BTSELCDEF_f.SELCD_2 = 0;
	FM4_BTIOSELCF->BTSELCDEF_f.SELCD_3 = 0;
	
	
	/*** BT PPG Timer Option ********************************/
	
	/* Count clock selection
     *
     * CK3 CK2 CK1 CK0 Divider
     *  0   0   0   0   /1
     *  0   0   0   1   /4
     *  0   0   1   0   /16
     *  0   0   1   1   /128
     *  0   1   0   0   /256
     *  0   1   0   1   External clock (rising edge event)
     *  0   1   1   0   External clock (falling edge event)
     *  0   1   1   1   External clock (both edge event)
     *  1   0   0   0   /512
     *  1   0   0   1   /1024
     *  1   0   1   0   /2048
     */
	BT4.TMCR_f.CKS0=0;
	BT4.TMCR_f.CKS1=0;
	BT4.TMCR_f.CKS2=0;
	BT4.TMCR2_f.CKS3=0;
	
	/* Restart Timer
     * 0 disabled
     * 1 enabled
     */
	BT4.TMCR_f.RTGEN=0;
	
	/* Pulse output mask
     * This bit controls the output level of PWM output waveforms.
     * 0 normal output
     * 1 fixed to low output
     */
	BT4.TMCR_f.PMSK=0;	

	/* Trigger input edge selection
     * 
     * EGS1 EGS0 
     *  0    0   Trigger input disabled
     *  0    1   Rising edge
     *  1    0   Falling edge
     *  1    1   Both edges
     */
	BT4.TMCR_f.EGS0=0;
	BT4.TMCR_f.EGS1=0;
	
	/* Timer function
     * Changes must be made while the timer is stopped (CTEN = 0).
     *
     * FMD2 FMD1 FMD0 
     *  0    0    0   Reset mode
     *  0    0    1   16-bit PWM timer
     *  0    1    0   16-bit PPG timer
     *  0    1    1   16/32-bit reload timer
     *  1    0    0   16/32-bit PWC timer
     */
	BT4.TMCR_f.FMD0=0;
	BT4.TMCR_f.FMD1=1;
	BT4.TMCR_f.FMD2=0;
	
	/* Output polaritys
     * 0 Normal polarity
     * 1 Inverted polarity
     */
	BT4.TMCR_f.OSEL=0;
	
	/* Mode select
     * 0 Continuous operation
     * 1 One-shot operation
     */
	BT4.TMCR_f.MDSE=0;
	
	// Set cycle and duty value
	FM4_BT4_PPG->PRLL=3200;			// 395 us Periode
	FM4_BT4_PPG->PRLH=3200;			// 395 us Periode

	//499 + 499 => 6.3 us Periode
	// 32000+32000 = 395 us Periode
	
	/* Count operation enable
     * 0 Stop
     * 1 Operation enabled
     */
	BT4.TMCR_f.CTEN=1;

	/* Software Trigger 
     * 0 = disabled (regardless of the EGS1 and EGS0)
     * 1 = enabled
     */
	BT4.TMCR_f.STRG=1;


//	// Set cycle and duty value
//	Bt_Ppg_WriteLowWidthVal(&BT0, 499);   // Cycle = (1+m)*PPG clock cycle = 100us
//	Bt_Ppg_WriteHighWidthVal(&BT0, 499);  // Duty = (1+m)*PPG clock cycle = 100us

}
#endif

/**
 *****************************************************************************
 ** \brief This function initializes DAC
 **
 *****************************************************************************/
static void Iomux_DAC_Init(void)
{
	// Do not need to init DAC pin function
	FM4_DAC->DACR0_f.DAC10 = 0; 			// 12bit Mode	
	FM4_DAC->DADR0_f.DA = 0; 				// write DAC 12bit output value to 0 to stop pump at startup
	FM4_DAC->DACR0_f.DAE = 1;  				// enable DAC
	
	while(FM4_DAC->DACR0_f.DRDY!=1) 	// wait till dac is ready
	{
	}
	
	FM4_DAC->DADR0_f.DA = 0; 			// write DAC 12bit output value to 0 to stop pump at startup
	
// 1. Set the operation mode using DAC10 and DDAS in the D/A Control Register (DACR), and set the DAE bit
// to 1.
// 2. Wait until the DRDY bit in the D/A Control Register (DACR) becomes 1.
// 3. Write to the D/A Data Register (DADR) the digital value to be converted into an analog value.
	
}

/**
 *****************************************************************************
 ** \brief This function initializes MFS2_I2C
 **
 *****************************************************************************/
static void Iomux_MFS2_I2C_Init(void)
{
	// Init MFS2_I2C pin function
	// Init channel 2
	SetPinFunc_SCK2_0();
	SetPinFunc_SOT2_0();
	
	// Initialize UART I2C mode
	MFS2.BGR  = 20000000 / 100000 - 1;  // 100k Bit/s @ 40 MHz
	MFS2.SCR |= 0x80;     				// Reset UART2 (UPCL = 1)
	MFS2.SMR  = 0x80;     				// operation mode 4, no interrupts,		
                                        // no test mode, no wake up    
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
    MFS3.BGR  = 20000000 / 100000 - 1;  // 100k Bit/s @ 40 MHz
    MFS3.SCR |= 0x80;     				// Reset UART3 (UPCL = 1)
    MFS3.SMR  = 0x80;      				// operation mode 4, no interrupts, 
                                        // no test mode, no wake up
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
 ** \brief This function initializes the GPIO ports and peripheral pin
 **        functions according to the user settings of the Pin Wizard.
 **
 *****************************************************************************/
void Iomux_Init(void)
{
    // use the two input clock pins as gpio
	FM4_RTC->VDET_f.PON=0;      // clear power-on signal
    FM4_RTC->VB_CLKDIV = 0x4Eu;	// set clock div to 78 => 160/4/(87+2) = 500kHz < 1MHz
	FM4_RTC->VBPFR_f.SPSR0=0x0; // use P46 and P47 as gpio pins
	FM4_RTC->VBPFR_f.SPSR1=0x0; 
	
	FM4_RTC->VBPFR_f.VPFR2=0x0; // use P47 as GPIO 
	FM4_RTC->VBPFR_f.VPFR3=0x0; // use P46 as GPIO 	
	
	FM4_RTC->VBDDR_f.VDDR2=0x0; // set P47 as Input
	FM4_RTC->VBDDR_f.VDDR3=0x0;	// set P46 as Input
	
	FM4_RTC->WTCR20_f.PWRITE = 1u;	//start data transfer to vbat domain
	
	while(0u != FM4_RTC->WTCR10_f.TRANS); //wait till transfer is completed
	
	FM4_RTC->WTCR20_f.PREAD=1u;//start data transfer from vbat to normal domain
	
	while(0u != FM4_RTC->WTCR10_f.TRANS); //wait till transfer is completed
	
	Iomux_DEBUG_Init();	
	Iomux_DAC_Init();

	Iomux_GPIO_Init();

	//Iomux_BT_Init(); //only used for debug, else 	toogle_watchdog(watchdog); used !
	Iomux_MFS2_I2C_Init();	
	Iomux_MFS3_I2C_Init();	
	Iomux_MFS0_UART_Init();	
	Iomux_ADC0_Init();	
}

//@} // IomuxGroup

/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
