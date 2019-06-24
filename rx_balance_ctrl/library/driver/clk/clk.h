/*******************************************************************************
* Copyright (C) 2013 Spansion LLC. All Rights Reserved. 
*
* This software is owned and published by: 
* Spansion LLC, 915 DeGuigne Dr. Sunnyvale, CA  94088-3453 ("Spansion").
*
* BY DOWNLOADING, INSTALLING OR USING THIS SOFTWARE, YOU AGREE TO BE BOUND 
* BY ALL THE TERMS AND CONDITIONS OF THIS AGREEMENT.
*
* This software contains source code for use with Spansion 
* components. This software is licensed by Spansion to be adapted only 
* for use in systems utilizing Spansion components. Spansion shall not be 
* responsible for misuse or illegal use of this software for devices not 
* supported herein.  Spansion is providing this software "AS IS" and will 
* not be responsible for issues arising from incorrect user implementation 
* of the software.  
*
* SPANSION MAKES NO WARRANTY, EXPRESS OR IMPLIED, ARISING BY LAW OR OTHERWISE,
* REGARDING THE SOFTWARE (INCLUDING ANY ACOOMPANYING WRITTEN MATERIALS), 
* ITS PERFORMANCE OR SUITABILITY FOR YOUR INTENDED USE, INCLUDING, 
* WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, THE IMPLIED 
* WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE OR USE, AND THE IMPLIED 
* WARRANTY OF NONINFRINGEMENT.  
* SPANSION SHALL HAVE NO LIABILITY (WHETHER IN CONTRACT, WARRANTY, TORT, 
* NEGLIGENCE OR OTHERWISE) FOR ANY DAMAGES WHATSOEVER (INCLUDING, WITHOUT 
* LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, 
* LOSS OF BUSINESS INFORMATION, OR OTHER PECUNIARY LOSS) ARISING FROM USE OR 
* INABILITY TO USE THE SOFTWARE, INCLUDING, WITHOUT LIMITATION, ANY DIRECT, 
* INDIRECT, INCIDENTAL, SPECIAL OR CONSEQUENTIAL DAMAGES OR LOSS OF DATA, 
* SAVINGS OR PROFITS, 
* EVEN IF SPANSION HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES. 
* YOU ASSUME ALL RESPONSIBILITIES FOR SELECTION OF THE SOFTWARE TO ACHIEVE YOUR
* INTENDED RESULTS, AND FOR THE INSTALLATION OF, USE OF, AND RESULTS OBTAINED 
* FROM, THE SOFTWARE.  
*
* This software may be replicated in part or whole for the licensed use, 
* with the restriction that this Disclaimer and Copyright notice must be 
* included with each copy of this software, whether used in part or whole, 
* at all times.  
*/
/******************************************************************************/
/** \file clk.h
 **
 ** Headerfile for CLK functions
 **  
 ** History:
 **   - 2013-04-24  1.0  MWi  First version.
 **   - 2013-07-12  1.1  MWi  Clk_SwitchTo* functions added
 **   - 2013-10-23  1.2  MWi  Clk_DisablePllClock and ClkDeepStop mode added
 **
 ******************************************************************************/

#ifndef __CLK_H__
#define __CLK_H__

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "pdl.h"

#if (defined(PDL_PERIPHERAL_CLK_ACTIVE))

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif
  
/**
 ******************************************************************************
 ** \defgroup ClkGroup Clock Functions (CLK)
 **
 ** Provided functions of CLK module:
 ** - Clk_MainGetParameters()
 ** - Clk_SubGetParameters()
 ** - Clk_VbatGetParameters()
 ** - Clk_SetDividers()
 ** - Clk_MainSetStabilizationWaitTime()
 ** - Clk_SubSetStabilizationWaitTime()
 ** - Clk_WaitForMainOscillator()
 ** - Clk_WaitForSubOscillator()
 ** - Clk_WaitForPllOscillator()
 ** - Clk_MainOscillatorReady()
 ** - Clk_SubOscillatorReady()
 ** - Clk_PllOscillatorReady()
 ** - Clk_SetSource()
 ** - Clk_SetPllSource()
 ** - Clk_SetMode()
 ** - Clk_EnableMainClock()
 ** - Clk_EnablePllClock()
 ** - Clk_EnableSubClock()
 ** - Clk_DisableMainClock()
 ** - Clk_DisableSubClock()
 ** - Clk_WaitForClockSourceReady()
 ** - Clk_SetIrq()
 ** - Clk_ClockVbatInit()
 ** - Clk_RequestVccPowerDown()
 ** - Clk_PeripheralClockEnable()
 ** - Clk_PeripheralGetClockState()
 ** - Clk_PeripheralClockDisable()
 ** - Clk_PeripheralSetReset()
 ** - Clk_PeripheralClearReset()
 ** - Clk_SwitchToMainClock()
 ** - Clk_SwitchToMainPllClock()
 ** - Clk_SwitchToSubClock()
 ** - Clk_SwitchToLsCrClock()
 ** - Clk_SwitchToHsCrClock()
 ** - Clk_SwitchToHsCrPllClock()
 **
 ** \brief How to use the CLK module
 **
 ** This driver manages the FM4 clock settings for:
 ** - Clock Source (Main, Sub, HS-CR, LS-CR, Main-PLL, HS-CR-PLL)
 ** - Clock Mode (Run, Sleep, Timer, Stop)
 ** - Bus dividers (HCLK, PCLK0, PCLK1, PCLK2)
 ** - Stabilization Wait Times (Main, Sub, PLL)
 **
 ** Also polling routines for the oscillation stabilization wait are provided,
 ** e.g. Clk_WaitForMainOscillator().
 ** If this waiting/polling is not wanted in the user application, there are
 ** also function, which return the current state of the stability of a clock,
 ** e.g. Clk_MainOscillatorReady().
 **
 ** For Clock Mode transition a hook function is called (if specified) for
 ** making individual power saving settings between setting SLEEPDEEP and
 ** the final WFI instruction (Timer-, Sleep-, Stop modes).
 **
 ** Each wait function has at lest a parameter for time out counting. The
 ** value of this parameter depends on the user's wait time settings.
 ** Additionally these functions call the library's PDL_WAIT_LOOP_HOOK()
 ** function.
 **
 ** \note 
 **       - After each Clock Source change Clk_WaitForClockSourceReady() with
 **         the clock source enumerator #en_clk_source_t and a time out count
 **         value as parameters has to be called, if a previous waiting for
 **         stabilization was not performed.
 **       - If the used device supports RTC and the RTC low power modes should
 **         be used, switch PDL_RTC_AVAILABLE to PDL_ON in pdl_user.h.
 **       - Because the Clock interrupts are shared with other interrupts (e.g.
 **         Watch Counter), Clk_SetIrq() has an argument, whether to touch
 **         the NVIC registers or not to prevent interference with other 
 **         driver parts.
 **       - Deep Standby Modes are only available in DSM driver module
 **       - The CLK driver functions may replace the System_Init() clock
 **         settings in system_mb9[ab]xyz.c. Set the definition for CLOCK_SETUP
 **         to CLOCK_SETTING_NONE in system_mb9[ab]xyz.h in this case.
 **       - The 'Clk_SwitchTo*' functions expect that the destination clock is
 **         enabled and stable.
 **
 ******************************************************************************/
//@{

/******************************************************************************/
/* Global pre-processor symbols/macros ('#define')                            */
/******************************************************************************/
#define CLK_SUBCLK_TRANSITION_TIMEOUT 100000000u        // Counter for VBAT transition time out
   
#define PLLK_MAX  4u // Setting according Peripheral Manual
#define PLLN_MAX 40u // Setting according Peripheral Manual
#define PLLM_MAX  4u // Setting according Peripheral Manual

#define BSC_PSR_MAX       0x06u
#define APBC0_PSR_MAX     0x03u
#define APBC1_PSR_MAX     0x03u 
#define APBC2_PSR_MAX     0x03u 
#define CSW_TMR_SOWT_MAX  0x0Bu 
#define CSW_TMR_MOWT_MAX  0x0Fu 
#define PSW_TMR_POWT_MAX  0x07u 
#define PLL_CTL1_PLLK_MAX ((uint8_t)PLLK_MAX)
#define PLL_CTL1_PLLM_MAX ((uint8_t)PLLM_MAX)
#define PLL_CTL2_PLLN_MAX ((uint8_t)PLLN_MAX)

// Driver's own SCB_SCR definition to stay compatible for IAR and KEIL!
#define	CLK_SCB_SCR *(volatile uint32_t*) 0xE000ED10ul
#define CLK_SCB_SCR_SLEEPDEEP_Position  2u                                  // SCB SCR: SLEEPDEEP Position
#define CLK_SCB_SCR_SLEEPDEEP_Mask      (1ul << CLK_SCB_SCR_SLEEPDEEP_Position) // SCB SCR: SLEEPDEEP Mask
  
// Driver's own PMD_CTL address definition
#define FM4_PMD_CTL *(volatile uint8_t*) 0x40035800ul
  
// Deep Standby Mode Select (DSTM) Bit Position
#define FM4_STB_CTL_DSTM_BITPOS  0x00000004ul
  
// PLL Input Clock Select (PINC) Bit Position
#define FM4_PSW_TMR_PINC_BITPOS 0x10u
  
// Interrupt enable bitpositions in INT_ENR
#define FM4_INT_ENR_PCSE_BITPOS 0x04u
#define FM4_INT_ENR_SCSE_BITPOS 0x02u
#define FM4_INT_ENR_MCSE_BITPOS 0x01u
  
// Interrupt status bitpositions in INT_STR
#define FM4_INT_STR_PCSI_BITPOS 0x04u
#define FM4_INT_STR_SCSI_BITPOS 0x02u
#define FM4_INT_STR_MCSI_BITPOS 0x01u
  
// Interrupt ckear bitpositions in INT_CLR
#define FM4_INT_CLR_PCSC_BITPOS 0x04u
#define FM4_INT_CLR_SCSC_BITPOS 0x02u
#define FM4_INT_CLR_MCSC_BITPOS 0x01u
  
/******************************************************************************
 * Global type definitions
 ******************************************************************************/

/**
 ******************************************************************************
 ** \brief Clock Source
 ** 
 ** Differentiator for the different clock sources
 **
 ** \note The enumerated values do not correspond to the RCS/RCM bits of the
 **       clock control and status registers due to having upward compatibility,
 **       if this bit coding may change in future devices. The correct bit
 **       patterns are set by switch(en_clk_source)/case statements individually
 **       in the corresponding function(s).
 ******************************************************************************/
typedef enum en_clk_source
{
  ClkMain = 0,  ///< Main Clock Oscillator
  ClkSub  = 1,  ///< Sub Clock Oscillator
  ClkHsCr = 2,  ///< High-Speed CR Clock Oscillator
  ClkLsCr = 3,  ///< Low-Speed CR Clock Oscillator
  ClkPll     = 4,  ///< PLL Clock
  ClkHsCrPll = 5   ///< High-Speed CR PLL Clock
} en_clk_source_t;

/**
 ******************************************************************************
 ** \brief Clock Mode
 ** 
 ** Differentiator for the different clock modes
 **
 ** \note
 ** This configuration uses together the STB_CTL register settings like in
 ** the DSM module.
 ******************************************************************************/
typedef enum en_clk_mode
{
  ClkRun      = 0,  ///< Run modes
  ClkSleep    = 1,  ///< Sleep modes
  ClkTimer    = 2,  ///< Timer modes
  ClkStop     = 3,  ///< Stop mode
  ClkRtc      = 4,  ///< RTC mode
  ClkDeepRtc  = 5,  ///< Deep Standby RTC mode
  ClkDeepStop = 6   ///< Deep Standby Stop mode
} en_clk_mode_t;

/**
 ******************************************************************************
 ** \brief Base Clock Prescaler Settings
 ** 
 ** Enumeration of the dividers of the Base Clock (HCLK)
 ******************************************************************************/
typedef enum en_clk_baseclkdiv
{
  BaseClkDiv1  = 0,  ///< HCLK Division 1/1
  BaseClkDiv2  = 1,  ///< HCLK Division 1/2
  BaseClkDiv3  = 2,  ///< HCLK Division 1/3
  BaseClkDiv4  = 3,  ///< HCLK Division 1/4
  BaseClkDiv6  = 4,  ///< HCLK Division 1/6
  BaseClkDiv8  = 5,  ///< HCLK Division 1/8    
  BaseClkDiv16 = 6,  ///< HCLK Division 1/16
  BaseClkErr   = 7   ///< HCLK prohibited setting
} en_clk_baseclkdiv_t;

/**
 ******************************************************************************
 ** \brief APB0 Prescaler Settings
 ** 
 ** Enumeration of the dividers of the APB0 (PCLK0)
 ******************************************************************************/
typedef enum en_clk_apb0div
{
  Apb0Div1  = 0,  ///< PCLK0 Division 1/1
  Apb0Div2  = 1,  ///< PCLK0 Division 1/2
  Apb0Div4  = 2,  ///< PCLK0 Division 1/4
  Apb0Div8  = 3   ///< PCLK0 Division 1/8
} en_clk_apb0div_t;

/**
 ******************************************************************************
 ** \brief APB1 Prescaler Settings
 ** 
 ** Enumeration of the dividers of the APB0 (PCLK1)
 ******************************************************************************/
typedef enum en_clk_apb1div
{
  Apb1Div1  = 0,  ///< PCLK1 Division 1/1
  Apb1Div2  = 1,  ///< PCLK1 Division 1/2
  Apb1Div4  = 2,  ///< PCLK1 Division 1/4
  Apb1Div8  = 3   ///< PCLK1 Division 1/8
} en_clk_apb1div_t;

/**
 ******************************************************************************
 ** \brief APB2 Prescaler Settings
 ** 
 ** Enumeration of the dividers of the APB0 (PCLK2)
 ******************************************************************************/
typedef enum en_clk_apb2div
{
  Apb2Div1  = 0,  ///< PCLK2 Division 1/1
  Apb2Div2  = 1,  ///< PCLK2 Division 1/2
  Apb2Div4  = 2,  ///< PCLK2 Division 1/4
  Apb2Div8  = 3   ///< PCLK2 Division 1/8
} en_clk_apb2div_t;

/**
 ******************************************************************************
 ** \brief Sub Clock oscillation stablilization wait time
 ** 
 ** Enumeration for the Sub Clock oscillation stabilization wait time settings
 ******************************************************************************/
typedef enum en_clk_scowaittime
{
  ScoWaitExp10  =  0,  ///< 2^10 / F(CL) => ~10.3  ms
  ScoWaitExp11  =  1,  ///< 2^11 / F(CL) => ~20.5  ms
  ScoWaitExp12  =  2,  ///< 2^12 / F(CL) => ~41    ms
  ScoWaitExp13  =  3,  ///< 2^13 / F(CL) => ~82    ms
  ScoWaitExp14  =  4,  ///< 2^14 / F(CL) => ~164   ms
  ScoWaitExp15  =  5,  ///< 2^15 / F(CL) => ~327   ms
  ScoWaitExp16  =  6,  ///< 2^16 / F(CL) => ~655   ms
  ScoWaitExp17  =  7,  ///< 2^17 / F(CL) => ~1.31  s      
  ScoWaitExp18  =  8,  ///< 2^17 / F(CL) => ~2.62  s
  ScoWaitExp19  =  9,  ///< 2^17 / F(CL) => ~5.24  s 
  ScoWaitExp20  = 10,  ///< 2^17 / F(CL) => ~10.48 s
  ScoWaitExp21  = 11,  ///< 2^17 / F(CL) => ~20.96 s 
  ScoWaitErr    = 12   ///< Prohibited Setting
} en_clk_scowaittime_t;

/**
 ******************************************************************************
 ** \brief Main Clock oscillation stablilization wait time
 ** 
 ** Enumeration for the Main Clock oscillation stabilization wait time settings
 ******************************************************************************/
typedef enum en_clk_mcowaittime
{
  McoWaitExp11   = 0,   ///< 2^1  / F(CH) => ~500  ns (F(CH) = 4 MHz)
  McoWaitExp15   = 1,   ///< 2^5  / F(CH) => ~8    us (F(CH) = 4 MHz)
  McoWaitExp16   = 2,   ///< 2^6  / F(CH) => ~16   us (F(CH) = 4 MHz)
  McoWaitExp17   = 3,   ///< 2^7  / F(CH) => ~32   us (F(CH) = 4 MHz)
  McoWaitExp18   = 4,   ///< 2^8  / F(CH) => ~64   us (F(CH) = 4 MHz)
  McoWaitExp19   = 5,   ///< 2^9  / F(CH) => ~128  us (F(CH) = 4 MHz)
  McoWaitExp110  = 6,   ///< 2^10 / F(CH) => ~256  us (F(CH) = 4 MHz)
  McoWaitExp111  = 7,   ///< 2^11 / F(CH) => ~512  us (F(CH) = 4 MHz)
  McoWaitExp112  = 8,   ///< 2^12 / F(CH) => ~1.0  ms (F(CH) = 4 MHz)
  McoWaitExp113  = 9,   ///< 2^13 / F(CH) => ~2.0  ms (F(CH) = 4 MHz)
  McoWaitExp114  = 10,  ///< 2^14 / F(CH) => ~4.0  ms (F(CH) = 4 MHz)
  McoWaitExp115  = 11,  ///< 2^15 / F(CH) => ~8.0  ms (F(CH) = 4 MHz)
  McoWaitExp117  = 12,  ///< 2^17 / F(CH) => ~33.0 ms (F(CH) = 4 MHz)
  McoWaitExp119  = 13,  ///< 2^18 / F(CH) => ~131  ms (F(CH) = 4 MHz)
  McoWaitExp121  = 14,  ///< 2^21 / F(CH) => ~524  ms (F(CH) = 4 MHz)
  McoWaitExp123  = 15   ///< 2^23 / F(CH) => ~2.0  s  (F(CH) = 4 MHz)
} en_clk_mcowaittime_t;

/**
 ******************************************************************************
 ** \brief PLL Clock oscillation stablilization wait time
 ** 
 ** Enumeration for the PLL Clock oscillation stabilization wait time settings
 ******************************************************************************/
typedef enum en_clk_pllowaittime
{
  PlloWaitExp19   = 0,   ///< 2^9  / F(CH) => ~128  us (F(CH) = 4 MHz)
  PlloWaitExp110  = 1,   ///< 2^10 / F(CH) => ~256  us (F(CH) = 4 MHz)
  PlloWaitExp111  = 2,   ///< 2^11 / F(CH) => ~512  us (F(CH) = 4 MHz)
  PlloWaitExp112  = 3,   ///< 2^12 / F(CH) => ~1.02 ms (F(CH) = 4 MHz)
  PlloWaitExp113  = 4,   ///< 2^13 / F(CH) => ~2.05 ms (F(CH) = 4 MHz)
  PlloWaitExp114  = 5,   ///< 2^14 / F(CH) => ~4.10 ms (F(CH) = 4 MHz)
  PlloWaitExp115  = 6,   ///< 2^15 / F(CH) => ~8.20 ms (F(CH) = 4 MHz)
  PlloWaitExp116  = 7    ///< 2^16 / F(CH) => ~16.4 ms (F(CH) = 4 MHz)
} en_clk_pllowaittime_t;

/**
 ******************************************************************************
 ** \brief PLL SourceClock (PINC bit of PSW_TMR)
 ** 
 ** Enumeration for the PLL Clock Source.
 **
 ** \attention HS-RC source only available, if used device supports PLL-CLKHC
 **            setting! PINC bit availability is <b>not</b> checked in this
 **            driver!
 ******************************************************************************/
typedef enum en_clk_pll_src
{
  PllSrcClkMo = 0,   ///< Use Main Clock as PLL source (always available, default)
  PllSrcClkHc = 123  ///< Use HS-RC Clock as PLL source (only if available!)
} en_clk_pll_src_t;

/**
 ******************************************************************************
 ** \brief Datatype for holding internal data needed for CLK
 ******************************************************************************/
#if (PDL_INTERRUPT_ENABLE_CLK == PDL_ON)
typedef struct stc_clk_intern_data
{
  func_ptr_t            pfnPllStabCb;     ///< PLL stabilization callback function pointer
  func_ptr_t            pfnScoStabCb;     ///< Sub Clock stabilization callback function pointer
  func_ptr_t            pfnMcoStabCb;     ///< Main Clock stabilization callback function pointer
} stc_clk_intern_data_t ;
#endif

/**
 ******************************************************************************
 ** \brief Sustain and Boost Current
 ******************************************************************************/
typedef enum en_clk_current
{
  Clk0nA          = 0,  ///< 0 nA sustain/boost, not allowed, if subclock is enabled
  Clk135nA        = 1,  ///< 135 nA
  Clk195nA        = 2,  ///< 195 nA
  Clk385nA        = 3,  ///< 385 nA
  Clk445nA        = 4,  ///< 445 nA, initial value for current sustain
  Clk510nA        = 5,  ///< 510 nA, initial value for current boost
  ClkErrorCurrent = 6   ///< Errornous setting (for read-out function)
} en_clk_current_t;

/**
 ******************************************************************************
 ** \brief Sustain and Boost Time
 ******************************************************************************/
typedef enum en_clk_boost_time
{
  ClkBoost50ms  = 0,    ///< Boost time 50 ms (initial value)
  ClkBoost63ms  = 1,    ///< Boost time 62.5 ms
  ClkBoost125ms = 2,    ///< Boost time 125 ms
  ClkBoost250ms = 3     ///< Boost time 250 ms
} en_clk_boost_time_t;

/**
 ******************************************************************************
 ** \brief VBAT pin usage
 ******************************************************************************/
typedef enum en_clk_vbat_pins
{
  ClkVbatGpio    = 0,   ///< P46 and P47 used as GPIOs
  ClkVbatOsc     = 1,   ///< P46 and P47 as 32 KHz oscillation pins (initial value)
  ClkVbatGpioOsc = 2    ///< P46 is uaed as GPIO, P47 is used as clock input pin
} en_clk_vbat_pins_t;

/**
 ******************************************************************************
 ** \brief VBAT pin direction and default values
 ******************************************************************************/
typedef enum en_clk_vbat_pins_ddr
{
  ClkVbatInput   = 0,   ///< VBAT pin input function
  ClkVbatOutputL = 1,   ///< VBAT pin output "low" function
  ClkVbatOutputH = 2    ///< VBAT pin output "high" function
} en_clk_vbat_pins_ddr_t;

/**
 ******************************************************************************
 ** \brief Clock Gate peripheral enumerators
 ******************************************************************************/
typedef enum en_clk_gate_peripheral
{
  ClkGateGpio   = 0,    ///< GPIO clock gate
  ClkGateExtif  = 1,    ///< External bus interface clock gate
  ClkGateDma    = 2,    ///< DMA clock gate
  ClkGateAdc0   = 3,    ///< ADC0 clock gate
  ClkGateAdc1   = 4,    ///< ADC1 clock gate
  ClkGateAdc2   = 5,    ///< ADC2 clock gate
  ClkGateAdc3   = 6,    ///< ADC3 clock gate
  ClkGateMfs0   = 7,    ///< MFS0 clock gate
  ClkGateMfs1   = 8,    ///< MFS1 clock gate
  ClkGateMfs2   = 9,    ///< MFS2 clock gate
  ClkGateMfs3   = 10,   ///< MFS3 clock gate
  ClkGateMfs4   = 11,   ///< MFS4 clock gate
  ClkGateMfs5   = 12,   ///< MFS5 clock gate
  ClkGateMfs6   = 13,   ///< MFS6 clock gate
  ClkGateMfs7   = 14,   ///< MFS7 clock gate
  ClkGateMfs8   = 15,   ///< MFS8 clock gate
  ClkGateMfs9   = 16,   ///< MFS9 clock gate
  ClkGateMfs10  = 17,   ///< MFS10 clock gate
  ClkGateMfs11  = 18,   ///< MFS11 clock gate
  ClkGateMfs12  = 19,   ///< MFS12 clock gate
  ClkGateMfs13  = 20,   ///< MFS13 clock gate
  ClkGateMfs14  = 21,   ///< MFS14 clock gate
  ClkGateMfs15  = 22,   ///< MFS15 clock gate
  ClkGateQprc0  = 23,   ///< QPRC0 clock gate
  ClkGateQprc1  = 24,   ///< QPRC1 clock gate
  ClkGateQprc2  = 25,   ///< QPRC2 clock gate
  ClkGateQprc3  = 26,   ///< QPRC3 clock gate
  ClkGateMft0   = 27,   ///< MFT0, PPG0/2/4/6 clock gate
  ClkGateMft1   = 28,   ///< MFT1, PPG8/10/12/14 clock gate
  ClkGateMft2   = 29,   ///< MFT2, PPG16/18/20/22 clock gate
  ClkGateMft3   = 30,   ///< MFT3, PPG24/26/28/30 clock gate
  ClkGateBt0    = 31,   ///< BT0/1/2/3 clock gate
  ClkGateBt4    = 32,   ///< BT4/5/6/7 clock gate
  ClkGateBt8    = 33,   ///< BT8/9/10/11 clock gate
  ClkGateBt12   = 34,   ///< BT12/13/14/15 clock gate
  ClkGateSdIf   = 35,   ///< SD Card I/F clock gate
  ClkGateCan0   = 36,   ///< CAN0 clock gate
  ClkGateCan1   = 37,   ///< CAN1 clock gate
  ClkGateUsb0   = 38,   ///< USB0 clock gate
  ClkGateUsb1   = 39    ///< USB1 clock gate
} en_clk_gate_peripheral_t;

/**
 ******************************************************************************
 ** \brief Reset peripheral enumerators
 ******************************************************************************/
typedef enum en_clk_reset_peripheral
{
  ClkResetExtif  = 1,    ///< External bus interface Reset
  ClkResetDma    = 2,    ///< DMA Reset
  ClkResetAdc0   = 3,    ///< ADC0 Reset
  ClkResetAdc1   = 4,    ///< ADC1 Reset
  ClkResetAdc2   = 5,    ///< ADC2 Reset
  ClkResetAdc3   = 6,    ///< ADC3 Reset
  ClkResetMfs0   = 7,    ///< MFS0 Reset
  ClkResetMfs1   = 8,    ///< MFS1 Reset
  ClkResetMfs2   = 9,    ///< MFS2 Reset
  ClkResetMfs3   = 10,   ///< MFS3 Reset
  ClkResetMfs4   = 11,   ///< MFS4 Reset
  ClkResetMfs5   = 12,   ///< MFS5 Reset
  ClkResetMfs6   = 13,   ///< MFS6 Reset
  ClkResetMfs7   = 14,   ///< MFS7 Reset
  ClkResetMfs8   = 15,   ///< MFS8 Reset
  ClkResetMfs9   = 16,   ///< MFS9 Reset
  ClkResetMfs10  = 17,   ///< MFS10 Reset
  ClkResetMfs11  = 18,   ///< MFS11 Reset
  ClkResetMfs12  = 19,   ///< MFS12 Reset
  ClkResetMfs13  = 20,   ///< MFS13 Reset
  ClkResetMfs14  = 21,   ///< MFS14 Reset
  ClkResetMfs15  = 22,   ///< MFS15 Reset
  ClkResetQprc0  = 23,   ///< QPRC0 Reset
  ClkResetQprc1  = 24,   ///< QPRC1 Reset
  ClkResetQprc2  = 25,   ///< QPRC2 Reset
  ClkResetQprc3  = 26,   ///< QPRC3 Reset
  ClkResetMft0   = 27,   ///< MFT0, PPG0/2/4/6 Reset
  ClkResetMft1   = 28,   ///< MFT1, PPG8/10/12/14 Reset
  ClkResetMft2   = 29,   ///< MFT2, PPG16/18/20/22 Reset
  ClkResetMft3   = 30,   ///< MFT3, PPG24/26/28/30 Reset
  ClkResetBt0    = 31,   ///< BT0/1/2/3 Reset
  ClkResetBt4    = 32,   ///< BT4/5/6/7 Reset
  ClkResetBt8    = 33,   ///< BT8/9/19/11 Reset
  ClkResetBt12   = 34,   ///< BT12/13/14/15 Reset
  ClkResetSdIf   = 35,   ///< SD Card I/F Reset
  ClkResetCan0   = 36,   ///< CAN0 Reset
  ClkResetCan1   = 37,   ///< CAN1 Reset
  ClkResetUsb0   = 38,   ///< USB0 Reset
  ClkResetUsb1   = 39    ///< USB1 Reset
} en_clk_reset_peripheral_t;

/**
 ******************************************************************************
 ** \brief Main/PLL Clock configuration
 ** 
 ** The Main/PLL Clock configuration settings
 ******************************************************************************/
typedef struct stc_clk_main_config
{
  en_clk_source_t       enSource;         ///< See description of #en_clk_source_t
  boolean_t             bEnablePll;       ///< PLL enable
  boolean_t             bEnableMainClock; ///< Main Clock enable 
  en_clk_mode_t         enMode;           ///< See description of #en_clk_mode_t
  boolean_t             bLpmPortHiZState; ///< TRUE: Sets the status of each external port pin
                                          ///        to high impedance (Hi-Z) in Timer or Stop mode
                                          ///< FALSE: Holds the status of each external port pin
  en_clk_baseclkdiv_t   enBaseClkDiv;     ///< See description of #en_clk_baseclkdiv_t
  en_clk_apb0div_t      enAPB0Div;        ///< See description of #en_clk_apb0div_t
  en_clk_apb1div_t      enAPB1Div;        ///< See description of #en_clk_apb1div_t
  en_clk_apb2div_t      enAPB2Div;        ///< See description of #en_clk_apb2div_t
  boolean_t             bAPB1Disable;     ///< TRUE: Disables APB1 regardless of divider settings
  boolean_t             bAPB2Disable;     ///< TRUE: Disables APB2 regardless of divider settings 
  en_clk_mcowaittime_t  enMCOWaitTime;    ///< See description of #en_clk_mcowaittime_t
  en_clk_pllowaittime_t enPLLOWaitTime;   ///< See description of #en_clk_pllowaittime_t
  uint8_t               u8PllK;           ///< PLL input clock frequency division ratio, PLLK
  uint8_t               u8PllM;           ///< PLL VCO clock frequency division ratio, PLLM
  uint8_t               u8PllN;           ///< PLL feedback frequency division ration, PLLN
  en_clk_pll_src_t      enPllSource;      ///< Only available, if HC clock as PLL clock source supported! See #en_clk_pll_src_t for details!
#if (PDL_INTERRUPT_ENABLE_CLK == PDL_ON)
  boolean_t             bPllIrq;          ///< TRUE: enable PLL oscillation stabilization completion interrupt
  boolean_t             bMcoIrq;          ///< TRUE: enable Main Clock oscillation stabilization completion interrupt
  func_ptr_t            pfnPllStabCb;     ///< PLL stabilization callback function pointer
  func_ptr_t            pfnMcoStabCb;     ///< Main Clock stabilization callback function pointer
#endif
  func_ptr_t            pfnHook;          ///< Hook call for low power modes (called between SLEEPDEEP = 1 and WFI)      
} stc_clk_main_config_t;

/**
 ******************************************************************************
 ** \brief Sub Clock configuration
 ** 
 ** The Main/PLL Clock configuration settings
 ******************************************************************************/
typedef struct stc_clk_sub_config
{
  boolean_t             bEnableSubClock;  ///< Sub Clock enable  
  en_clk_scowaittime_t  enSCOWaitTime;    ///< See description of #en_clk_scowaittime_t
#if (PDL_INTERRUPT_ENABLE_CLK == PDL_ON)
  boolean_t             bScoIrq;          ///< TRUE: enable Sub Clock oscillation stabilization completion interrupt
  func_ptr_t            pfnScoStabCb;     ///< Sub Clock stabilization callback function pointer
#endif  
} stc_clk_sub_config_t;

/**
 ******************************************************************************
 ** \brief Clock VBAT configuration
 ** 
 ** The Clock VBAT configuration settings
 ******************************************************************************/
typedef struct stc_clk_vbat_config
{
  boolean_t              bLinkClock;            ///< TRUE: 32 kHz oscillation circuit linked with clock oscillation circuit,
                                                ///< FALSE: 32 kHz oscillation circuit operates independly
  uint8_t                u8VbClockDiv;          ///< Value for VB_CLKDIV register
  en_clk_current_t       enClkSustainCurrent;   ///< Clock oscillator current sustain setting. See #en_clk_current_t for details
  en_clk_current_t       enClkBoostCurrent;     ///< Clock oscillator current boost setting. See #en_clk_current_t for details
  en_clk_boost_time_t    enClkBoostTime;        ///< Clock oscillator current boost time. See #en_clk_boost_time_t for details
  en_clk_vbat_pins_t     enVbatPins;            ///< Setting of VBAT pin usage, see #stc_clk_vbat_config_t for details
  boolean_t              bVbP48Peripheral;      ///< TRUE: P48 pin (VREGCTL) as I/O of peripheral function, FALSE: P48 pin as GPIO
  boolean_t              bVbP49Peripheral;      ///< TRUE: P49 pin (VWAKEUP) as I/O of peripheral function, FALSE: P49 pin as GPIO
  boolean_t              bVbP47Peripheral;      ///< TRUE: P47 pin (X1A) as I/O of peripheral function, FALSE: P47 pin as GPIO
  boolean_t              bVbP46Peripheral;      ///< TRUE: P46 pin (X0A) as I/O of peripheral function, FALSE: P46 pin as GPIO
  boolean_t              bVbP48PullUp;          ///< TRUE: Enable pull-up on P48 (VREGCTL), FALSE: disbale pull-up
  boolean_t              bVbP49PullUp;          ///< TRUE: Enable pull-up on P49 (VWAKEUP), FALSE: disbale pull-up
  boolean_t              bVbP47PullUp;          ///< TRUE: Enable pull-up on P47 (X1A), FALSE: disbale pull-up
  boolean_t              bVbP46PullUp;          ///< TRUE: Enable pull-up on P46 (X0A), FALSE: disbale pull-up
  en_clk_vbat_pins_ddr_t enVbP48InOut;          ///< P48 (VREGCTL) port direction and init value, if output. See #en_clk_vbat_pins_ddr_t for details
  en_clk_vbat_pins_ddr_t enVbP49InOut;          ///< P49 (VWAKEUP) port direction and init value, if output. See #en_clk_vbat_pins_ddr_t for details
  en_clk_vbat_pins_ddr_t enVbP47InOut;          ///< P47 (X1A) port direction and init value, if output. See #en_clk_vbat_pins_ddr_t for details
  en_clk_vbat_pins_ddr_t enVbP46InOut;          ///< P46 (X0A )port direction and init value, if output. See #en_clk_vbat_pins_ddr_t for details
  boolean_t              bVbP48OpenDrain;       ///< TRUE: P48 (VREGCTL) Pseudo Open Drain enabled, FALSE: port push-pull output
  boolean_t              bVbP49OpenDrain;       ///< TRUE: P49 (VWAKEUP) Pseudo Open Drain enabled, FALSE: port push-pull output
} stc_clk_vbat_config_t;

/******************************************************************************/
/* Global variable definitions ('extern')                                     */
/******************************************************************************/
#if (PDL_INTERRUPT_ENABLE_CLK == PDL_ON)
extern stc_clk_intern_data_t stcClkInternData;
#endif

/******************************************************************************/
/* Global function prototypes (definition in C source)                        */
/******************************************************************************/

#if (PDL_INTERRUPT_ENABLE_CLK == PDL_ON)
extern void ClkIrqHandler(void) ;
#endif

extern en_result_t Clk_MainGetParameters(stc_clk_main_config_t* pstcConfig) ;
extern en_result_t Clk_SubGetParameters(stc_clk_sub_config_t* pstcConfig) ;
extern en_result_t Clk_VbatGetParameters(stc_clk_vbat_config_t* pstcConfig) ;

extern en_result_t Clk_SetDividers(stc_clk_main_config_t* pstcConfig) ;

extern en_result_t Clk_MainSetStabilizationWaitTime(stc_clk_main_config_t* pstcConfig) ;
extern en_result_t Clk_SubSetStabilizationWaitTime(stc_clk_sub_config_t* pstcConfig) ;

extern en_result_t Clk_WaitForMainOscillator(uint32_t u32MaxTimeOut) ;
extern en_result_t Clk_WaitForSubOscillator(uint32_t u32MaxTimeOut) ;

extern en_result_t Clk_WaitForPllOscillator(uint32_t u32MaxTimeOut) ;

extern en_result_t Clk_MainOscillatorReady(void) ;
extern en_result_t Clk_SubOscillatorReady(void) ;

extern en_result_t Clk_PllOscillatorReady(void) ;

extern en_result_t Clk_SetSource(stc_clk_main_config_t* pstcConfigMain,
                                 stc_clk_sub_config_t*  pstcConfigSub) ;

extern en_result_t Clk_SetPllSource(stc_clk_main_config_t* pstcConfig) ;

extern en_result_t Clk_SetMode(stc_clk_main_config_t* pstcConfig) ;

extern en_result_t Clk_EnableMainClock(void) ;
extern en_result_t Clk_EnablePllClock(stc_clk_main_config_t* pstcConfigMain) ;
extern en_result_t Clk_EnableSubClock(void) ;

extern en_result_t Clk_DisableMainClock(void) ;
extern en_result_t Clk_DisablePllClock(void) ;
extern en_result_t Clk_DisableSubClock(void) ;

extern en_result_t Clk_WaitForClockSourceReady(en_clk_source_t enSource,
                                        uint32_t        u32MaxTimeOut) ;

#if (PDL_INTERRUPT_ENABLE_CLK == PDL_ON)
extern en_result_t Clk_SetIrq( stc_clk_main_config_t* pstcConfigMain,
                               stc_clk_sub_config_t*  pstcConfigSub,
                               boolean_t              bTouchNvic
                             ) ;
#endif

extern en_result_t Clk_ClockVbatInit(stc_clk_vbat_config_t* pstcConfig) ;

extern en_result_t Clk_RequestVccPowerDown(void) ;

extern en_result_t Clk_PeripheralClockEnable(en_clk_gate_peripheral_t enPeripheral) ;

extern boolean_t Clk_PeripheralGetClockState(en_clk_gate_peripheral_t enPeripheral) ;

extern en_result_t Clk_PeripheralClockDisable(en_clk_gate_peripheral_t enPeripheral) ;

extern en_result_t Clk_PeripheralSetReset(en_clk_reset_peripheral_t enPeripheral) ;

extern en_result_t Clk_PeripheralClearReset(en_clk_reset_peripheral_t enPeripheral) ;

extern en_result_t Clk_SwitchToMainClock(void) ;

extern en_result_t Clk_SwitchToMainPllClock(void) ;

extern en_result_t Clk_SwitchToSubClock(void) ;

extern en_result_t Clk_SwitchToLsCrClock(void) ;

extern en_result_t Clk_SwitchToHsCrClock(void) ;

extern en_result_t Clk_SwitchToHsCrPllClock(void) ;


//@} // ClkGroup

#ifdef __cplusplus
}
#endif

#endif // #if (defined(PDL_PERIPHERAL_CLK_ACTIVE))

#endif /* __CLK_H__ */
/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
