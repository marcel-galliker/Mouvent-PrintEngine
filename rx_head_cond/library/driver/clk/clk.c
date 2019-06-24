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
/** \file clk.c
 **
 ** A detailed description is available at 
 ** @link ClkGroup Clock Module description @endlink
 **
 ** History:
 **   - 2012-02-21  1.0  MWi  First version.
 **   - 2013-06-03  1.1  MWi  VBAT settings added
 **   - 2013-07-12  1.2  MWi  Clk_SwitchTo* functions added, bVbPXXeripheral
 **                           GPIO initialization corrected
 **   - 2013-10-24  1.3  MWi  Clk_DisablePllClock, ClkDeepStop mode added,
 **                           Deep standby modes corrected
 **
 ******************************************************************************/

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "clk.h"

#if (defined(PDL_PERIPHERAL_CLK_ACTIVE))

/**
 ******************************************************************************
 ** \addtogroup ClkGroup
 ******************************************************************************/
//@{

/******************************************************************************/
/* Local pre-processor symbols/macros ('#define')                             */
/******************************************************************************/

/******************************************************************************/
/* Global variable definitions (declared in header file with 'extern')        */
/******************************************************************************/
#if (PDL_INTERRUPT_ENABLE_CLK == PDL_ON)
stc_clk_intern_data_t stcClkInternData;
#endif

/******************************************************************************/
/* Local type definitions ('typedef')                                         */
/******************************************************************************/

/******************************************************************************/
/* Local function prototypes ('static')                                       */
/******************************************************************************/

/******************************************************************************/
/* Local variable definitions ('static')                                      */
/******************************************************************************/

/******************************************************************************/
/* Function implementation - global ('extern') and local ('static')           */
/******************************************************************************/

/**
 ******************************************************************************
 ** \brief Clock Stabilization Interrupt Handler
 ******************************************************************************/
#if (PDL_INTERRUPT_ENABLE_CLK == PDL_ON)
void ClkIrqHandler(void)
{
  uint8_t u8IntStrReadOut;
  
  u8IntStrReadOut = FM4_CRG->INT_STR;
  
  // PLL stabilization ready?
  if (0u != (u8IntStrReadOut & FM4_INT_STR_PCSI_BITPOS))
  {
    FM4_CRG->INT_CLR |= FM4_INT_CLR_PCSC_BITPOS;  // Clear Irq
    
    // PLL ready callback if defined
    if (stcClkInternData.pfnPllStabCb != NULL)
    {
      stcClkInternData.pfnPllStabCb();
    }
  }

  // Sub Clock stabilization ready?  
  if (0u != (u8IntStrReadOut & FM4_INT_STR_SCSI_BITPOS))
  {
    FM4_CRG->INT_CLR |= FM4_INT_CLR_SCSC_BITPOS;  // Clear Irq
    
    // Sub Clock ready callback if defined
    if (stcClkInternData.pfnScoStabCb != NULL)
    {
      stcClkInternData.pfnScoStabCb();
    }
  }

  // Main Clock stabilization ready?  
  if (0u != (u8IntStrReadOut & FM4_INT_STR_MCSI_BITPOS))
  {
    FM4_CRG->INT_CLR |= FM4_INT_CLR_MCSC_BITPOS;  // Clear Irq
    
    // Main Clock ready callback if defined
    if (stcClkInternData.pfnMcoStabCb != NULL)
    {
      stcClkInternData.pfnMcoStabCb();
    }
  }
}
#endif

/**
 ******************************************************************************
 ** \brief Get current Main/PLL Clock parameters
 **
 ** This function sets the 'current' elements of the configuration according
 ** the main and PLL clock registers.
 ** 
 ** \note This function overwrites any configuration. To avoid this, a second
 **       configuration structure like 'ConfigRecent' may be used.
 **
 ** \note This function does not set any hook function pointer! If this function
 **       is used to get the current main and PLL clock settings as a base for
 **       new settings, a possible hook function pointer must be set explicitly
 **       after copying the configuration!
 **
 ** \param [out]  pstcConfig       Clock configuration parameters
 **
 ** \retval Ok                     Main and PLL Clock configuration has been set
 ** \retval ErrorInvalidParameter  If pstcConfig == NULL
 ** \retval ErrorInvalidMode       Illegal clock mode has been detected
 **
 ******************************************************************************/
en_result_t Clk_MainGetParameters(stc_clk_main_config_t* pstcConfig)
{
  uint8_t u8Dummy;
  
  en_clk_mcowaittime_t aenClkMcoWaitTime[16u] =
  {
    McoWaitExp11,  McoWaitExp15,  McoWaitExp16,  McoWaitExp17,
    McoWaitExp18,  McoWaitExp19,  McoWaitExp110, McoWaitExp111,
    McoWaitExp112, McoWaitExp113, McoWaitExp114, McoWaitExp115,
    McoWaitExp117, McoWaitExp119, McoWaitExp121, McoWaitExp123 
  };
	
  if ( pstcConfig == NULL )
  {
      return ErrorInvalidParameter ;
  } 
  
  u8Dummy = (0x07u & (FM4_CRG->SCM_STR >> 5u));            
  switch (u8Dummy)
  {
    case 0:
      pstcConfig->enSource = ClkHsCr ;
      break;
    case 1:
      pstcConfig->enSource = ClkMain ;
      break;   
    case 2:
      pstcConfig->enSource = ClkPll ;
      break;      
    case 4:
      pstcConfig->enSource = ClkLsCr ;
      break;    
    case 5:
      pstcConfig->enSource = ClkSub ;
      break;  
    default:
      return ErrorInvalidMode ;
  }

  pstcConfig->bEnablePll       = (FM4_CRG->SCM_CTL_f.PLLE == 1u)  ? 1u : 0u ;
  pstcConfig->bEnableMainClock = (FM4_CRG->SCM_CTL_f.MOSCE == 1u) ? 1u : 0u ;
  
  switch(0x07u & FM4_CRG->BSC_PSR)
  {
    case 0u:
      pstcConfig->enBaseClkDiv = BaseClkDiv1;
      break;
    case 1u:
      pstcConfig->enBaseClkDiv = BaseClkDiv2;
      break;
    case 2u:
      pstcConfig->enBaseClkDiv = BaseClkDiv3;
      break;
    case 3u:
      pstcConfig->enBaseClkDiv = BaseClkDiv4;
      break;
    case 4u:
      pstcConfig->enBaseClkDiv = BaseClkDiv6;
      break;
    case 5u:
      pstcConfig->enBaseClkDiv = BaseClkDiv8;
      break;
    case 6u:
      pstcConfig->enBaseClkDiv = BaseClkDiv16;
      break;
    default:
      pstcConfig->enBaseClkDiv = BaseClkErr;
  }

  switch(0x03u & FM4_CRG->APBC0_PSR)
  {
    case 0u:
      pstcConfig->enAPB0Div = Apb0Div1;
      break;
    case 1u:
      pstcConfig->enAPB0Div = Apb0Div2;
      break;
    case 2u:
      pstcConfig->enAPB0Div = Apb0Div4;
      break;
    case 4u:
      pstcConfig->enAPB0Div = Apb0Div8;
      break;
    default:
      break;
  }

  switch(0x03u & FM4_CRG->APBC1_PSR)
  {
    case 0u:
      pstcConfig->enAPB1Div = Apb1Div1;
      break;
    case 1u:
      pstcConfig->enAPB1Div = Apb1Div2;
      break;
    case 2u:
      pstcConfig->enAPB1Div = Apb1Div4;
      break;
    case 4u:
      pstcConfig->enAPB1Div = Apb1Div8;
      break;
    default:
      break;
  }
  
  switch(0x03u & FM4_CRG->APBC2_PSR)
  {
    case 0u:
      pstcConfig->enAPB2Div = Apb2Div1;
      break;
    case 1u:
      pstcConfig->enAPB2Div = Apb2Div2;
      break;
    case 2u:
      pstcConfig->enAPB2Div = Apb2Div4;
      break;
    case 4u:
      pstcConfig->enAPB2Div = Apb2Div8;
      break;
    default:
      break;
  }

  
  pstcConfig->enMCOWaitTime = aenClkMcoWaitTime[0x0Fu & FM4_CRG->CSW_TMR];

  pstcConfig->u8PllK = ((0x0Fu & (FM4_CRG->PLL_CTL1)) >> 4u) + 1u ;
  pstcConfig->u8PllM = (0x0Fu & (FM4_CRG->PLL_CTL1)) + 1u ;
  pstcConfig->u8PllN = (0x1Fu & (FM4_CRG->PLL_CTL2)) + 1u ;
  
#if (PDL_INTERRUPT_ENABLE_CLK == PDL_ON)
  pstcConfig->bPllIrq = (FM4_CRG->INT_ENR_f.PCSE) ? 1u : 0u ;
  pstcConfig->bMcoIrq = (FM4_CRG->INT_ENR_f.MCSE) ? 1u : 0u ;
#endif
  
  pstcConfig->enMode = ClkRun ; // Function can only be called in run mode
  
  return Ok;
} // Clk_GetParameters

/**
 ******************************************************************************
 ** \brief Get current Sub Clock parameters
 ** 
 ** \note This function overwrites any configuration. To avoid this, a second
 **       configuration structure like 'ConfigRecent' may be used.
 **
 ** \note This function does not set any hook function pointer! If this function
 **       is used to get the current sub clock settings as a base for
 **       new settings, a possible hook function pointer must be set explicitly
 **       after copying the configuration!
 **
 ** \param [out]  pstcConfig       Clock configuration parameters
 **
 ** \retval Ok                     Sub Clock configuration has been set
 ** \retval ErrorInvalidParameter  If pstcConfig == NULL
 ** \retval ErrorInvalidMode       Illegal clock mode has been detected
 **
 ******************************************************************************/
en_result_t Clk_SubGetParameters(stc_clk_sub_config_t* pstcConfig)
{
  switch(0x0Fu & (FM4_CRG->CSW_TMR >> 4u))
  {
    case 0u:
      pstcConfig->enSCOWaitTime = ScoWaitExp10;
      break;
    case 1u:
      pstcConfig->enSCOWaitTime = ScoWaitExp11;
      break;
    case 2u:
      pstcConfig->enSCOWaitTime = ScoWaitExp12;
      break;
    case 3u:
      pstcConfig->enSCOWaitTime = ScoWaitExp13;
      break;
    case 4u:
      pstcConfig->enSCOWaitTime = ScoWaitExp14;
      break;
    case 5u:
      pstcConfig->enSCOWaitTime = ScoWaitExp15;
      break;
    case 6u:
      pstcConfig->enSCOWaitTime = ScoWaitExp16;
      break;
    case 8u:
      pstcConfig->enSCOWaitTime = ScoWaitExp18;
      break;
    case 9u:
      pstcConfig->enSCOWaitTime = ScoWaitExp19;
      break;
    case 10u:
      pstcConfig->enSCOWaitTime = ScoWaitExp20;
      break;
    case 11u:
      pstcConfig->enSCOWaitTime = ScoWaitExp21;
      break;
    default:
      pstcConfig->enSCOWaitTime = ScoWaitErr;
  }
  
#if (PDL_INTERRUPT_ENABLE_CLK == PDL_ON)
  pstcConfig->bScoIrq = (FM4_CRG->INT_ENR_f.SCSE) ? 1u : 0u ;
#endif
  
  return Ok;
}

/**
 ******************************************************************************
 ** \brief Get current VBAT parameters
 ** 
 ** \note This function overwrites any configuration. To avoid this, a second
 **       configuration structure like 'ConfigRecent' may be used.
 **
 ** \note This function does not set any hook function pointer! If this function
 **       is used to get the current VBAT settings as a base for
 **       new settings, a possible hook function pointer must be set explicitly
 **       after copying the configuration!
 **
 ** \param [out]  pstcConfig       Clock configuration parameters
 **
 ** \retval Ok                     VBAT configuration has been set
 ** \retval ErrorInvalidParameter  If pstcConfig == NULL
 ** \retval ErrorInvalidMode       Illegal VBAT mode has been detected
 **
 ******************************************************************************/
en_result_t Clk_VbatGetParameters(stc_clk_vbat_config_t* pstcConfig)
{
  uint8_t u8ReadOut;
  uint8_t u8ReadOut2;
  uint8_t u8ReadMask;
  
  pstcConfig->bLinkClock       = (TRUE == (FM4_RTC->WTOSCCNT_f.SOSCNTL)) ? 1u : 0u ;
  pstcConfig->bVbP48Peripheral = (0x00u == (0x01u & (FM4_RTC->VBPFR)))   ? 1u : 0u ; 
  pstcConfig->bVbP49Peripheral = (0x00u == (0x02u & (FM4_RTC->VBPFR)))   ? 1u : 0u ; 
  pstcConfig->bVbP47Peripheral = (0x00u == (0x04u & (FM4_RTC->VBPFR)))   ? 1u : 0u ; 
  pstcConfig->bVbP46Peripheral = (0x00u == (0x08u & (FM4_RTC->VBPFR)))   ? 1u : 0u ; 
  pstcConfig->bVbP48PullUp     = (0x01u == (0x01u & (FM4_RTC->VBPCR)))   ? 1u : 0u ;
  pstcConfig->bVbP49PullUp     = (0x02u == (0x02u & (FM4_RTC->VBPCR)))   ? 1u : 0u ;
  pstcConfig->bVbP47PullUp     = (0x03u == (0x03u & (FM4_RTC->VBPCR)))   ? 1u : 0u ;
  pstcConfig->bVbP46PullUp     = (0x08u == (0x08u & (FM4_RTC->VBPCR)))   ? 1u : 0u ;
  pstcConfig->bVbP48OpenDrain  = (0x01u == (0x01u & (FM4_RTC->VBPZR)))   ? 1u : 0u ;
  pstcConfig->bVbP49OpenDrain  = (0x02u == (0x02u & (FM4_RTC->VBPZR)))   ? 1u : 0u ;
  
  u8ReadOut = FM4_RTC->VBPFR;
  u8ReadMask = 0x30u & u8ReadOut;
  
  if ((0x00u == u8ReadMask) &&
      (0x20u == u8ReadMask))
  {
    pstcConfig->enVbatPins = ClkVbatGpio;
  }
  else if (0x01u == u8ReadMask)
  {
    pstcConfig->enVbatPins = ClkVbatOsc;
  }
  else
  {
    pstcConfig->enVbatPins = ClkVbatGpioOsc;
  }
  
  pstcConfig->u8VbClockDiv = FM4_RTC->VB_CLKDIV; 
  
  switch(FM4_RTC->CCS)
  {
    case 0x00u:
      pstcConfig->enClkSustainCurrent = Clk0nA;
      break;
    case 0x01u:
      pstcConfig->enClkSustainCurrent = Clk135nA;
      break;
    case 0x02u:
      pstcConfig->enClkSustainCurrent = Clk195nA;
      break;
    case 0x04u:
      pstcConfig->enClkSustainCurrent = Clk385nA;
      break;
    case 0x08u:
      pstcConfig->enClkSustainCurrent = Clk445nA;
      break;
    case 0x10u:
      pstcConfig->enClkSustainCurrent = Clk510nA;
      break;
    default:
      pstcConfig->enClkSustainCurrent = ClkErrorCurrent;
  }
  
  switch(FM4_RTC->CCB)
  {
    case 0x00u:
      pstcConfig->enClkBoostCurrent = Clk0nA;
      break;
    case 0x01u:
      pstcConfig->enClkBoostCurrent = Clk135nA;
      break;
    case 0x02u:
      pstcConfig->enClkBoostCurrent = Clk195nA;
      break;
    case 0x04u:
      pstcConfig->enClkBoostCurrent = Clk385nA;
      break;
    case 0x08u:
      pstcConfig->enClkBoostCurrent = Clk445nA;
      break;
    case 0x10u:
      pstcConfig->enClkBoostCurrent = Clk510nA;
      break;
    default:
      pstcConfig->enClkBoostCurrent = ClkErrorCurrent;
  }
  
  u8ReadOut  = FM4_RTC->VBDDR;
  u8ReadOut2 = FM4_RTC->VBDOR;
  
  if (0x00 == (0x01 & u8ReadOut))
  {
    pstcConfig->enVbP48InOut = ClkVbatInput;
  }
  else
  {
    if (0x00 == (0x01 & u8ReadOut2))
    {
      pstcConfig->enVbP48InOut = ClkVbatOutputL;
    }
    else
    {
      pstcConfig->enVbP48InOut = ClkVbatOutputH;
    }
  }
  
  if (0x00 == (0x02 & u8ReadOut))
  {
    pstcConfig->enVbP49InOut = ClkVbatInput;
  }
  else
  {
    if (0x00 == (0x02 & u8ReadOut2))
    {
      pstcConfig->enVbP49InOut = ClkVbatOutputL;
    }
    else
    {
      pstcConfig->enVbP49InOut = ClkVbatOutputH;
    }
  }
  
  if (0x00 == (0x04 & u8ReadOut))
  {
    pstcConfig->enVbP47InOut = ClkVbatInput;
  }
  else
  {
    if (0x00 == (0x04 & u8ReadOut2))
    {
      pstcConfig->enVbP47InOut = ClkVbatOutputL;
    }
    else
    {
      pstcConfig->enVbP47InOut = ClkVbatOutputH;
    }
  }

  if (0x00 == (0x08 & u8ReadOut))
  {
    pstcConfig->enVbP46InOut = ClkVbatInput;
  }
  else
  {
    if (0x00 == (0x08 & u8ReadOut2))
    {
      pstcConfig->enVbP46InOut = ClkVbatOutputL;
    }
    else
    {
      pstcConfig->enVbP46InOut = ClkVbatOutputH;
    }
  }
  
  return Ok;
}  // Clk_VbatGetParameters
 
/**
 ******************************************************************************
 ** \brief Set Clock Dividers
 **
 ** This function sets the clock dividers.
 **
 ** \note It is strongly recommended to disable any resource of its
 **       corresponding bus, if the bus division setting is changed! Malfunction
 **       of the resources may result (i.e. wrong baud rates, lost data, etc.).
 **
 ** \note Do not access any of the resource registers, if the corresponding
 **       resource's bus is disabled! An immediate bus fault exception will
 **       occur in this case!
 **
 ** \param [in]  pstcConfig        Clock configuration parameters
 ** 
 ** \retval Ok                     Dividers set
 ** \retval ErrorInvalidParameter  pstcConfig == NULL or Illegal divider
 **                                setting
 ******************************************************************************/
en_result_t Clk_SetDividers(stc_clk_main_config_t* pstcConfig)
{
  if ( pstcConfig == NULL )
  {
      return ErrorInvalidParameter ;
  } 
  
  // Set Base Clock divider
  if (pstcConfig->enBaseClkDiv > BSC_PSR_MAX)
  {
    return ErrorInvalidParameter ;
  }
  else
  {
    FM4_CRG->BSC_PSR = pstcConfig->enBaseClkDiv;
  }

  // Set APB0 Clock divider
  if (pstcConfig->enAPB0Div > APBC0_PSR_MAX)
  {
    return ErrorInvalidParameter ;
  }
  else
  {
    // Setting APCC0[1:0]
    FM4_CRG->APBC0_PSR = pstcConfig->enAPB0Div;
  }  

  // Set APB1 Clock divider
  if (pstcConfig->enAPB1Div > APBC1_PSR_MAX)
  {
    return ErrorInvalidParameter ;
  }
  else
  {
    // Setting APCC1[1:0] and APBC1EN (APBC1RST is always set to 0)
    FM4_CRG->APBC1_PSR = (pstcConfig->enAPB1Div) | (((pstcConfig->bAPB1Disable == 1u) ? 0u : 1u) << 7u);
  } 
  
  // Set APB2 Clock divider
  if (pstcConfig->enAPB2Div > APBC2_PSR_MAX)
  {
    return ErrorInvalidParameter ;
  }
  else
  {
    // Setting APCC2[1:0] and APBC2EN (APBC2RST is always set to 0)
    FM4_CRG->APBC2_PSR = (pstcConfig->enAPB2Div) | (((pstcConfig->bAPB2Disable == 1u) ? 0u : 1u) << 7u);
  }   
  
  return Ok;
} // Clk_SetDividers

/**
 ******************************************************************************
 ** \brief Set Main Oscillation Stabilization Wait Time
 **
 ** This function sets the stabilization wait time.
 **
 ** \param [in]  pstcConfig        Clock configuration parameters
 ** 
 ** \retval Ok                     Time set
 ** \retval ErrorInvalidParameter  pstcConfig == NULL or Illegal timing
 **                                setting
 ******************************************************************************/
en_result_t Clk_MainSetStabilizationWaitTime(stc_clk_main_config_t* pstcConfig)
{
  uint8_t u8Dummy;
  uint8_t u8Pinc = 0u;
  
  if ( pstcConfig == NULL )
  {
      return ErrorInvalidParameter ;
  } 
  
  if (pstcConfig->enMCOWaitTime > CSW_TMR_MOWT_MAX)
  {
    return ErrorInvalidParameter ;
  }
  else
  {
    u8Dummy = pstcConfig->enMCOWaitTime ;
  }
  
  // update hardware
  FM4_CRG->CSW_TMR &= 0xF0u;
  FM4_CRG->CSW_TMR |= u8Dummy ;
  
  if (pstcConfig->enPLLOWaitTime > PSW_TMR_POWT_MAX)
  {
    return ErrorInvalidParameter ;
  }
  else
  { 
    if (pstcConfig->enPllSource == PllSrcClkHc)
    {
      u8Pinc = FM4_PSW_TMR_PINC_BITPOS;
    }
    FM4_CRG->PSW_TMR = (pstcConfig->enPLLOWaitTime) | u8Pinc ;
  }
  
  return Ok;
} // Clk_SetStabilizationWaitTime

/**
 ******************************************************************************
 ** \brief Wait for Main Oscillation Stabilization
 **
 ** This function waits for the Main Oscillator stabilization via polling.
 ** PDL_WAIT_LOOP_HOOK() is called during polling. It should be called, if the
 ** system needs a stable main clock (i.e. for communication or switching to
 ** PLL clock, etc.).
 **
 ** \param [in]  u32MaxTimeOut   Time out counter start value
 ** 
 ** \retval Ok                   Clock stabilized
 ** \retval ErrorTimeout         Clock not stabilized after timeout count
 ******************************************************************************/
en_result_t Clk_WaitForMainOscillator(uint32_t u32MaxTimeOut)
{
  uint32_t u32Timeout = u32MaxTimeOut;
  
  while (u32Timeout--)
  {
    if (FM4_CRG->SCM_STR_f.MORDY == 1u)
    {
      return Ok;
    }
    
    PDL_WAIT_LOOP_HOOK();
  }
  
  return ErrorTimeout;
} // Clk_WaitForMainOscillator

/**
 ******************************************************************************
 ** \brief Wait for PLL Oscillation Stabilization
 **
 ** This function waits for the PLL Oscillator stabilization via polling.
 ** PDL_WAIT_LOOP_HOOK() is called during polling. It should be called, if the
 ** system needs a stable PLL clock (i.e. for communication).
 **
 ** \param [in]  u32MaxTimeOut   Time out counter start value
 ** 
 ** \retval Ok                   Clock stabilized
 ** \retval ErrorTimeout         Clock not stabilized after timeout count
 ******************************************************************************/
en_result_t Clk_WaitForPllOscillator(uint32_t u32MaxTimeOut)
{
  uint32_t u32Timeout = u32MaxTimeOut;
  
  while (u32Timeout--)
  {
    if (FM4_CRG->SCM_STR_f.PLRDY == 1u)
    {
      return Ok;
    }
    
    PDL_WAIT_LOOP_HOOK();
  }
  
  return ErrorTimeout;
} // Clk_WaitForPllOscillator

/**
 ******************************************************************************
 ** \brief Wait for Clock Mode ready (transition finished)
 **
 ** This function waits for a clock source, meaning a clock source already
 ** available or a clock source transition to be expected to be ready soon or
 ** already available. PDL_WAIT_LOOP_HOOK() is called during polling.
 **
 ** \note This function is not needed to be called, if the user has performed
 **       the stabilization wait time for the desired source clock before.
 **       For safety reasons, this function can be called anyhow with a small
 **       timeout count (<<10).
 **
 ** \param [in]  enSource          Clock Source to be checked
 ** \param [in]  u32MaxTimeOut     Time out counter start value
 ** 
 ** \retval Ok                     Clock Source ready
 ** \retval ErrorTimeout           Clock Source not ready within time out count
 ** \retval ErrorInvalidParameter  Not a valid Clock Mode to be checked
 ******************************************************************************/
en_result_t Clk_WaitForClockSourceReady(en_clk_source_t enSource,
                                        uint32_t        u32MaxTimeOut)
{
  uint32_t u32Timeout = u32MaxTimeOut;
  uint8_t  u8SourcePattern;
  
  switch(enSource)
  {
    case ClkHsCr:
      u8SourcePattern = 0x0u;
      break;   
    case ClkMain:
      u8SourcePattern = 0x1u;
      break;
    case ClkPll:
      u8SourcePattern = 0x2u;
      break; 
    case ClkLsCr:
      u8SourcePattern = 0x4u;
      break; 
    case ClkSub:
      u8SourcePattern = 0x5u;
      break;  
    default:
      return ErrorInvalidParameter;
  }
  
  while (u32Timeout--)
  {
    if ((0x07u & (FM4_CRG->SCM_STR >> 5u)) == u8SourcePattern)
    {
      return Ok;
    }
    
    PDL_WAIT_LOOP_HOOK();
  }
  
  return ErrorTimeout;
} // Clk_WaitForClockSourceReady

/**
 ******************************************************************************
 ** \brief Main Oscillation Ready?
 **
 ** This function checks the availability of a stable Main Clock.
 **
 ** \retval Ok                   Main Clock stabilized
 ** \retval ErrorNotReady        Main Clock not stabilized yet
 ******************************************************************************/
en_result_t Clk_MainOscillatorReady(void)
{
  if (FM4_CRG->SCM_STR_f.MORDY == 1u)
  {
    return Ok;
  }

  return ErrorNotReady;
} // Clk_MainOscillatorReady

/**
 ******************************************************************************
 ** \brief Sub Oscillation Ready?
 **
 ** This function checks the availability of a stable Sub Clock.
 **
 ** \retval Ok                   Sub Clock stabilized
 ** \retval ErrorNotReady        Sub Clock not stabilized yet
 ******************************************************************************/
en_result_t Clk_SubOscillatorReady(void)
{
  if (FM4_CRG->SCM_STR_f.SORDY == 1u)
  {
    return Ok;
  }

  return ErrorNotReady;
} // Clk_SubOscillatorReady

/**
 ******************************************************************************
 ** \brief PLL Oscillation Ready?
 **
 ** This function checks the availability of a stable PLL Clock.
 **
 ** \retval Ok                   PLL Clock stabilized
 ** \retval ErrorNotReady        PLL Clock not stabilized yet
 ******************************************************************************/
en_result_t Clk_PllOscillatorReady(void)
{
  if (FM4_CRG->SCM_STR_f.PLRDY == 1u)
  {
    return Ok;
  }

  return ErrorNotReady;
} // Clk_PllOscillatorReady

/**
 ******************************************************************************
 ** \brief Set Clock Source
 **
 ** This function sets the clock source and performs transition, if wanted.
 **
 ** \param [in]  pstcConfigMain    Main/PLL Clock configuration parameters
 ** \param [in]  pstcConfigSub     Sub Clock configuration parameters
 ** 
 ** \retval Ok                     Clock source set
 ** \retval ErrorInvalidParameter  pstcConfig == NULL or Illegal mode
 ** \retval ErrorInvalidMode       Clock setting not possible
 ******************************************************************************/
en_result_t Clk_SetSource(stc_clk_main_config_t* pstcConfigMain,
                          stc_clk_sub_config_t*  pstcConfigSub)
{
  stc_clk_main_config_t    stcCurrentConfigMain;  // local read-out configuration
  stc_clk_sub_config_t     stcCurrentConfigSub;   // local read-out configuration
  stc_crg_scm_ctl_field_t  stcSCM_CTL;

  if ( pstcConfigMain == NULL )
  {
    return ErrorInvalidParameter ;
  } 
  
  if ( pstcConfigSub == NULL )
  {
    return ErrorInvalidParameter ;
  } 
  
  if (Ok != Clk_MainGetParameters(&stcCurrentConfigMain))
  {
    return ErrorInvalidParameter ;
  }
  
  if (Ok != Clk_SubGetParameters(&stcCurrentConfigSub))
  {
    return ErrorInvalidParameter ;
  }
  
  switch(pstcConfigMain->enSource)
  {
    case ClkMain:
      if ((FM4_CRG->SCM_CTL_f.MOSCE != TRUE) ||  // Main Oscillator ready?
          (FM4_CRG->SCM_STR_f.MORDY != TRUE))
      {
        return ErrorInvalidParameter ;
      }
      FM4_CRG->SCM_CTL_f.RCS = 0x1u;
      break;
    case ClkSub:
      if ((FM4_CRG->SCM_CTL_f.SOSCE != TRUE) ||  // Sub Oscillator ready?
          (FM4_CRG->SCM_STR_f.SORDY != TRUE))
      {
        return ErrorInvalidParameter ;
      }
      FM4_CRG->SCM_CTL_f.RCS = 0x5u;
      break;
    case ClkHsCr:                                // Always possible
      FM4_CRG->SCM_CTL_f.RCS = 0x0u;
      break;
    case ClkLsCr:                                // Always possible
      FM4_CRG->SCM_CTL_f.RCS = 0x4u;
    case ClkPll:                           
      if ((FM4_CRG->SCM_CTL_f.PLLE != TRUE) ||  // PLL ready?
          (FM4_CRG->SCM_STR_f.PLRDY != TRUE))
      {
        return ErrorInvalidParameter ;
      }
      FM4_CRG->SCM_CTL_f.RCS = 0x2u;
      break;
    default:
      return ErrorInvalidParameter ;
  }

  return Ok;
} // Clk_SetSource

/**
 ******************************************************************************
 ** \brief Set Clock Mode
 **
 ** This function sets the clock mode and performs the transition. For
 ** individual settings (such as USB and CAN low power configuration) a hook
 ** function is called after setting SLEEPDEEP to 1 and final WFI instruction.
 ** This function is only called, if the function pointer is unequal to NULL.
 ** Additionally the ports will go into Hi-Z state, if
 ** stc_clk_config_t#bPortHiZState is TRUE.
 **
 ** \param [in]  pstcConfig        Clock configuration parameters
 ** 
 ** \retval Ok                     Clock mode set
 ** \retval ErrorInvalidParameter  pstcConfig == NULL or Illegal mode
 ******************************************************************************/
en_result_t Clk_SetMode(stc_clk_main_config_t* pstcConfig)
{
  uint32_t u32PortState; 
  
  if ( pstcConfig == NULL )
  {
      return ErrorInvalidParameter ;
  } 
  
  u32PortState = (pstcConfig->bLpmPortHiZState == TRUE) ? 0x00000010ul : 0ul ; // SPL setting
  
  switch(pstcConfig->enMode)
  {
    case ClkRun : // obsolete, just for the sake of completeness ...
      break;
      
    case ClkSleep :
      FM4_PMD_CTL = 0x00u;                // PMD_CTL.RTCE = 0 (no effect, if RTC not available)
      SCB->SCR = 0u;                      // Sleepdeep = 0
      
      if (pstcConfig->pfnHook != NULL)    // Hook function specified?
      {
         pstcConfig->pfnHook();
      }
      __WFI();                            // wait for interrupt -> CPU stopped.
      break;
      
    case ClkTimer :
      FM4_PMD_CTL = 0x00u;                // PMD_CTL.RTCE = 0 (no effect, if RTC not available)
      FM4_CRG->STB_CTL = 0x1ACC0000ul | u32PortState; // STM = 0b00
      
      if (pstcConfig->pfnHook != NULL)    // Hook function specified?
      {
         pstcConfig->pfnHook();
      }
      __WFI();                            // wait for interrupt -> CPU stopped.
      break;
      
    case ClkStop :
      FM4_PMD_CTL = 0x00u;                // PMD_CTL.RTCE = 0 (no effect, if RTC not available)
      FM4_CRG->STB_CTL = 0x1ACC0002ul | u32PortState; // STM = 0b10
      
      if (pstcConfig->pfnHook != NULL)    // Hook function specified?
      {
         pstcConfig->pfnHook();
      }
      __WFI();                            // wait for interrupt -> CPU stopped.      
      break;

    case ClkRtc :
      FM4_PMD_CTL = 0x01u;                // PMD_CTL.RTCE = 1
      FM4_CRG->STB_CTL = 0x1ACC0002ul | u32PortState; // STM = 0b10
      
      if (pstcConfig->pfnHook != NULL)    // Hook function specified?
      {
         pstcConfig->pfnHook();
      }
      __WFI();                            // wait for interrupt -> CPU stopped.      
      break;      
      
    case ClkDeepRtc :
      FM4_PMD_CTL = 0x01u;                // PMD_CTL.RTCE = 1
      FM4_CRG->STB_CTL = 0x1ACC0002ul | u32PortState | FM4_STB_CTL_DSTM_BITPOS ; // STM = 0b10, DSTM = 1
      CLK_SCB_SCR = CLK_SCB_SCR_SLEEPDEEP_Mask;	      // Sleepdeep = 1
       
      if (pstcConfig->pfnHook != NULL)    // Hook function specified?
      {
         pstcConfig->pfnHook();
      }
      __WFI();                            // wait for interrupt -> CPU stopped.      
      break;
      
    case ClkDeepStop :
      FM4_PMD_CTL = 0x00u;                // PMD_CTL.RTCE = 0 (no effect, if RTC not available)
      FM4_CRG->STB_CTL = 0x1ACC0002ul | u32PortState | FM4_STB_CTL_DSTM_BITPOS ; // STM = 0b10, DSTM = 1
      CLK_SCB_SCR = CLK_SCB_SCR_SLEEPDEEP_Mask;	    // Sleepdeep = 1
      
      if (pstcConfig->pfnHook != NULL)    // Hook function specified?
      {
         pstcConfig->pfnHook();
      }
      __WFI();                            // wait for interrupt -> CPU stopped.      
      break;
      
    default :
      return ErrorInvalidParameter ;
  }
  
  return Ok;
} // Clk_SetMode

/**
 ******************************************************************************
 ** \brief Disable Sub Clock
 **
 ** This function easily disables the Sub Clock. No configuration is needed.
 **
 ** \retval Ok                Sub Clock disabled
 ******************************************************************************/
en_result_t Clk_DisableSubClock(void)
{
  FM4_CRG->SCM_CTL_f.SOSCE = 0u;

  return Ok;
} // Clk_DisableSubClock

/**
 ******************************************************************************
 ** \brief Enable Main Clock
 **
 ** This function easily enables the Main Clock. No configuration is needed.
 ** For stabilization wait time Clk_WaitForMainOscillator() has to be called
 ** afterwards.
 **
 ** \retval Ok                Main Clock enabled
 ******************************************************************************/
en_result_t Clk_EnableMainClock(void)
{
  FM4_CRG->SCM_CTL_f.MOSCE = 1u;

  return Ok;
} // Clk_EnableMainClock

/**
 ******************************************************************************
 ** \brief Enable PLL Clock with parameter
 **
 ** This function easily enables the PLL Clock. It uses the Pll configuration
 ** from #stc_clk_main_config_t.
 ** For stabilization wait time Clk_WaitForPllOscillator() has to be called
 ** afterwards.
 **
 ** \retval Ok                        Main Clock enabled
 ** \retval ErrorInvalidParameter     PLL settings wrong, pstcConfigMain == NULL
 ** \retval ErrorOperationInProgress  PLL already running
 ******************************************************************************/
en_result_t Clk_EnablePllClock(stc_clk_main_config_t* pstcConfigMain)
{
  uint8_t u8Dummy;
  
  if ( pstcConfigMain == NULL )
  {
    return ErrorInvalidParameter ;
  } 
  
  if (1u == FM4_CRG->SCM_CTL_f.PLLE)
  {
    return ErrorOperationInProgress;
  }
  
  // PLLK
  if ((pstcConfigMain->u8PllK > PLL_CTL1_PLLK_MAX) ||
      (pstcConfigMain->u8PllK == 0u)
     )
  {
    return ErrorInvalidParameter ;
  }
  else
  {
    u8Dummy = ((pstcConfigMain->u8PllK) - 1u) << 4u ; // PLLK value (0000) + 1 => 1/1
  }
  
  // PLLM
  if ((pstcConfigMain->u8PllM > PLL_CTL1_PLLM_MAX) ||
      (pstcConfigMain->u8PllM == 0u)
     )
  {
    return ErrorInvalidParameter ;
  }
  else
  {
    u8Dummy |= (pstcConfigMain->u8PllM) - 1u ; // PLLM value (0000) + 1 => 1/1
  }
  
  FM4_CRG->PLL_CTL1 = u8Dummy ;
  
  // PLLN
  if ((pstcConfigMain->u8PllN > PLL_CTL2_PLLN_MAX) ||
      (pstcConfigMain->u8PllN == 0u)
     )
  {
    return ErrorInvalidParameter ;
  }
  else
  {
    FM4_CRG->PLL_CTL2 =( pstcConfigMain->u8PllN) - 1u ; // PLLM value (00000) + 1 => 1/1
  }
  
  FM4_CRG->SCM_CTL_f.PLLE = 1u;

  return Ok;
} // Clk_EnablePllClock

/**
 ******************************************************************************
 ** \brief Disable Main Clock
 **
 ** This function easily disables the Main Clock. No configuration is needed.
 **
 ** \retval Ok                Main Clock disabled
 ******************************************************************************/
en_result_t Clk_DisableMainClock(void)
{
  FM4_CRG->SCM_CTL_f.MOSCE = 0u;

  return Ok;
} // Clk_DisableMainClock

/**
 ******************************************************************************
 ** \brief Disable PLL Clock
 **
 ** This function easily disables the PLL Clock. No configuration is needed.
 **
 ** \retval Ok                PLL Clock disabled
 ******************************************************************************/
en_result_t Clk_DisablePllClock(void)
{
  FM4_CRG->SCM_CTL_f.PLLE = 0u;

  return Ok;
} // Clk_DisablePllClock

#if (PDL_INTERRUPT_ENABLE_CLK == PDL_ON)
/**
 ******************************************************************************
 ** \brief Set/Clear Clock interrupts
 **
 ** This function enables or disables the clock stabilization interrupts 
 ** according configuration
 **
 ** \param [in]  pstcConfigMain    Main/PLL Clock configuration parameters
 ** \param [in]  pstcConfigSub     Sub Clock configuration parameters
 ** \param [in]  bTouchNvic        TRUE = Touch NVIC registers, FALSE = Do not
 **                                touch NVIC registers
 **
 ** \retval Ok                     Interrupts have been set
 ** \retval ErrorInvalidParameter  If pstcConfig == NULL
 ******************************************************************************/
en_result_t Clk_SetIrq( stc_clk_main_config_t* pstcConfigMain,
                        stc_clk_sub_config_t*  pstcConfigSub,
                        boolean_t              bTouchNvic
                      )
{
  uint8_t u8SetNvic = 0u;      // 1 = Enable NVIC interrupts, 0 = Disable NVIC interrupts
  
  if ( pstcConfigMain == NULL )
  {
      return ErrorInvalidParameter ;
  } 
  
  if ( pstcConfigSub == NULL )
  {
      return ErrorInvalidParameter ;
  } 
  
  // PLL Stabilization Interrupt
  if (pstcConfigMain->bPllIrq == TRUE)
  {
    FM4_CRG->INT_ENR |= FM4_INT_ENR_PCSE_BITPOS;
    
    if (pstcConfigMain->pfnPllStabCb != NULL)
    {
      stcClkInternData.pfnPllStabCb = pstcConfigMain->pfnPllStabCb;
      u8SetNvic = 1u;
    }
  }
  else
  {
    FM4_CRG->INT_ENR &= 0xFFu ^ FM4_INT_ENR_PCSE_BITPOS;
  }

  // Sub Clock Stabilization Interrupt
  if (pstcConfigSub->bScoIrq == TRUE)
  {
    FM4_CRG->INT_ENR |= FM4_INT_ENR_SCSE_BITPOS;
    
    if (pstcConfigSub->pfnScoStabCb != NULL)
    {
      stcClkInternData.pfnScoStabCb = pstcConfigSub->pfnScoStabCb;
      u8SetNvic |= 1u;
    }
  }
  else
  {
    FM4_CRG->INT_ENR &= 0xFFu ^ FM4_INT_ENR_SCSE_BITPOS;
  }  

  // Main Clock Stabilization Interrupt
  if (pstcConfigMain->bMcoIrq == TRUE)
  {
    FM4_CRG->INT_ENR |= FM4_INT_ENR_MCSE_BITPOS;
    
    if (pstcConfigMain->pfnMcoStabCb != NULL)
    {
      stcClkInternData.pfnMcoStabCb = pstcConfigMain->pfnMcoStabCb;
      u8SetNvic |= 1u;
    }
  }
  else
  {
    FM4_CRG->INT_ENR &= 0xFFu ^ FM4_INT_ENR_MCSE_BITPOS;
  }  
  
  if (TRUE == bTouchNvic)
  {
    if (0u != u8SetNvic)
    {
      NVIC_ClearPendingIRQ(TIM_IRQn);
      NVIC_EnableIRQ(TIM_IRQn);
      NVIC_SetPriority(TIM_IRQn, PDL_IRQ_LEVEL_CAN0);         
    }
    else
    {
      NVIC_ClearPendingIRQ(TIM_IRQn);
      NVIC_DisableIRQ(TIM_IRQn);
      NVIC_SetPriority(TIM_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL); 
    }
  }
  
  return Ok;
} // Clk_SetIrq
#endif

/**
 ******************************************************************************
 ** \brief Init Sub Clock with VBAT settings
 **
 ** THis function initializes the sub clock with VBAT setting. It does not
 ** start the clock.
 **
 ** \param [in]  pstcConfig        Sub Clock/VBAT configuration parameters
 **
 ** \retval Ok                     Interrupts have been set
 ** \retval ErrorInvalidParameter  If pstcConfig == NULL
 ******************************************************************************/
en_result_t Clk_ClockVbatInit(stc_clk_vbat_config_t* pstcConfig)
{
  uint8_t                  u8VbClkDiv;
  uint8_t                  u8Ccs;
  uint8_t                  u8Ccb;
  stc_rtc_boost_field_t    stcBoost;
  stc_rtc_vbpfr_field_t    stcVbpfr;
  stc_rtc_vbpcr_field_t    stcVbpcr;
  stc_rtc_vbddr_field_t    stcVbddr;
  stc_rtc_vbpzr_field_t    stcVbpzr;
  stc_rtc_vbdor_field_t    stcVbdor;
     
  if ( pstcConfig == NULL )
  {
    return ErrorInvalidParameter ;
  }

  PDL_ZERO_STRUCT(stcBoost);
  PDL_ZERO_STRUCT(stcVbpfr);
  PDL_ZERO_STRUCT(stcVbddr);
  PDL_ZERO_STRUCT(stcVbpzr);
  PDL_ZERO_STRUCT(stcVbdor);     
  
  u8VbClkDiv = pstcConfig->u8VbClockDiv;

  switch(pstcConfig->enClkSustainCurrent)
  {
    case Clk0nA:
      u8Ccs = 0x00u;
      break;
    case Clk135nA:
      u8Ccs = 0x01u;
      break;
    case Clk195nA:
      u8Ccs = 0x02u;
      break;
    case Clk385nA:
      u8Ccs = 0x04u;
      break;
    case Clk445nA:
      u8Ccs = 0x08u;
      break;
    case Clk510nA:
      u8Ccs = 0x10u;
      break;
    default:
      return ErrorInvalidParameter;
  }

  switch(pstcConfig->enClkBoostCurrent)
  {
    case Clk0nA:
      u8Ccb = 0x00u;
      break;
    case Clk135nA:
      u8Ccb = 0x01u;
      break;
    case Clk195nA:
      u8Ccb = 0x02u;
      break;
    case Clk385nA:
      u8Ccb = 0x04u;
      break;
    case Clk445nA:
      u8Ccb = 0x08u;
      break;
    case Clk510nA:
      u8Ccb = 0x10u;
      break;
    default:
      return ErrorInvalidParameter;
  }
  
  switch(pstcConfig->enClkBoostTime)
  {
    case ClkBoost50ms:
      stcBoost.BOOST = 3u;
      break;
    case ClkBoost63ms:
      stcBoost.BOOST = 0u;
      break;
    case ClkBoost125ms:
      stcBoost.BOOST = 1u;
      break;
    case ClkBoost250ms:
      stcBoost.BOOST = 2u;
      break;
    default:
      return ErrorInvalidParameter;
  }
  
  switch(pstcConfig->enVbatPins)
  {
    case ClkVbatGpio:
      stcVbpfr.SPSR0 = 0;
      stcVbpfr.SPSR1 = 0;
      break;
    case ClkVbatOsc:
      stcVbpfr.SPSR0 = 1;
      stcVbpfr.SPSR1 = 0;
      break;
    case ClkVbatGpioOsc:
      stcVbpfr.SPSR0 = 1;
      stcVbpfr.SPSR1 = 1;
      break;
    default:
      return ErrorInvalidParameter;
  }
  
  stcVbpfr.VPFR0 = (TRUE == (pstcConfig->bVbP48Peripheral)) ? 1u : 0u ;
  stcVbpfr.VPFR1 = (TRUE == (pstcConfig->bVbP49Peripheral)) ? 1u : 0u ;
  stcVbpfr.VPFR2 = (TRUE == (pstcConfig->bVbP47Peripheral)) ? 1u : 0u ;
  stcVbpfr.VPFR3 = (TRUE == (pstcConfig->bVbP46Peripheral)) ? 1u : 0u ;
    
  stcVbpcr.VPCR0 = (TRUE == (pstcConfig->bVbP48PullUp)) ? 1u : 0u ;
  stcVbpcr.VPCR1 = (TRUE == (pstcConfig->bVbP49PullUp)) ? 1u : 0u ;
  stcVbpcr.VPCR2 = (TRUE == (pstcConfig->bVbP47PullUp)) ? 1u : 0u ;
  stcVbpcr.VPCR3 = (TRUE == (pstcConfig->bVbP46PullUp)) ? 1u : 0u ;
  
  if (FALSE == pstcConfig->bVbP48Peripheral)
  {
    switch(pstcConfig->enVbP48InOut)
    {
      case ClkVbatInput:
        stcVbddr.VDDR0 = 0u;
        break;
      case ClkVbatOutputL:
        stcVbdor.VDOR0 = 0u;
        stcVbddr.VDDR0 = 1u;
        break;
      case ClkVbatOutputH:
        stcVbdor.VDOR0 = 1u;
        stcVbddr.VDDR0 = 1u;
        break;
      default:
        return ErrorInvalidParameter;
    }
  }
  
  if (FALSE == pstcConfig->bVbP49Peripheral)
  {
    switch(pstcConfig->enVbP49InOut)
    {
      case ClkVbatInput:
        stcVbddr.VDDR1 = 0u;
        break;
      case ClkVbatOutputL:
        stcVbdor.VDOR1 = 0u;
        stcVbddr.VDDR1 = 1u;
        break;
      case ClkVbatOutputH:
        stcVbdor.VDOR1 = 1u;
        stcVbddr.VDDR1 = 1u;
        break;
      default:
        return ErrorInvalidParameter;
    }
  }
  
  if ((ClkVbatOsc != pstcConfig->enVbatPins) &&
      (FALSE == pstcConfig->bVbP47Peripheral))
  {
    switch(pstcConfig->enVbP47InOut)
    {
      case ClkVbatInput:
        stcVbddr.VDDR2 = 0u;
        break;
      case ClkVbatOutputL:
        stcVbdor.VDOR2 = 0u;
        stcVbddr.VDDR2 = 1u;
        break;
      case ClkVbatOutputH:
        stcVbdor.VDOR2 = 1u;
        stcVbddr.VDDR2 = 1u;
        break;
      default:
        return ErrorInvalidParameter;
    }
  }
  
  if (FALSE == pstcConfig->bVbP46Peripheral)
  {
    switch(pstcConfig->enVbP46InOut)
    {
      case ClkVbatInput:
        stcVbddr.VDDR3 = 0u;
        break;
      case ClkVbatOutputL:
        stcVbdor.VDOR3 = 0u;
        stcVbddr.VDDR3 = 1u;
        break;
      case ClkVbatOutputH:
        stcVbdor.VDOR3 = 1u;
        stcVbddr.VDDR3 = 1u;
        break;
      default:
        return ErrorInvalidParameter;
    }
  }
  
  stcVbpzr.VPZR0 = (TRUE == (pstcConfig->bVbP48OpenDrain)) ? 1u : 0u ;
  stcVbpzr.VPZR1 = (TRUE == (pstcConfig->bVbP49OpenDrain)) ? 1u : 0u ;
  
  // Update hardware
  if (TRUE == (FM4_RTC->VDET_f.PON))    // Power-on detection bit set?
  {
    FM4_RTC->VDET_f.PON = 0u;            // Release Power-on detection bit
  }

  FM4_RTC->VBPFR_f   = stcVbpfr;
  FM4_RTC->VBPCR_f   = stcVbpcr;
  FM4_RTC->VBDOR_f   = stcVbdor;
  FM4_RTC->VBDDR_f   = stcVbddr;
  FM4_RTC->VBPZR_f   = stcVbpzr;
 
  FM4_RTC->CCB       = u8Ccb;
  FM4_RTC->CCS       = u8Ccs;
  FM4_RTC->VB_CLKDIV = u8VbClkDiv;
  FM4_RTC->BOOST_f   = stcBoost;

  // Write to VBAT domain
  FM4_RTC->WTCR20_f.PWRITE = 1;

  // Wait to complete transmission
  while(0u != FM4_RTC->WTCR10_f.TRANS)
  {}
    
  return Ok;
} // Clk_ClockVbatInit

/**
 ******************************************************************************
 ** \brief Enable Sub Clock
 **
 ** This function easily enables the Sub Clock. No configuration is needed.
 ** For stabilization wait time Clk_WaitForSubOscillator() has to be called
 ** afterwards.
 **
 ** \retval Ok                Sub Clock enabled
 ** \retval ErrorTimeout      Data transition to VBAT domain failed
 ******************************************************************************/
en_result_t Clk_EnableSubClock(void)
{
  uint32_t u32WaitCount = CLK_SUBCLK_TRANSITION_TIMEOUT;
  
  FM4_RTC->WTOSCCNT_f.SOSCEX  = 0u;     // Start 32 kHz oscillation
  FM4_RTC->WTOSCCNT_f.SOSCNTL = 1u;     // Disable 32 kHz stop control
  
  // Transmit to VBAT domain
  FM4_RTC->WTCR20_f.PWRITE = 1;
  
  // Wait to complete transmission
  while((u32WaitCount--) && (0u != FM4_RTC->WTCR10_f.TRANS))
  {}                                    // no hook function here
  
  if (0 == u32WaitCount)
  {
	return ErrorTimeout;
  }
    
  FM4_CRG->SCM_CTL_f.SOSCE = 1u; 	// Finally sub clock can be enabled 
  
  return Ok;
} // Clk_EnableSubClock

/**
 ******************************************************************************
 ** \brief Set Sub Oscillation Stabilization Wait Time
 **
 ** This function sets the stabilization wait time.
 **
 ** \param [in]  pstcConfig        Clock configuration parameters
 ** 
 ** \retval Ok                     Time set
 ** \retval ErrorInvalidParameter  pstcConfig == NULL or Illegal timing
 **                                setting
 ******************************************************************************/
en_result_t Clk_SubSetStabilizationWaitTime(stc_clk_sub_config_t* pstcConfig)
{
  uint8_t u8Dummy;
  
  // local variable for CSW_TMR
  u8Dummy = (0x0Fu & FM4_CRG->CSW_TMR);
  
  if (pstcConfig->enSCOWaitTime > CSW_TMR_SOWT_MAX)
  {
    return ErrorInvalidParameter ;
  }
  else
  {
    switch(pstcConfig->enSCOWaitTime)
    {
      case ScoWaitExp10:
        u8Dummy |=  0x00;
        break;
      case ScoWaitExp11:
        u8Dummy |=  0x10;
        break;
      case ScoWaitExp12:
        u8Dummy |=  0x20;
        break;
      case ScoWaitExp13:
        u8Dummy |=  0x30;
        break;
      case ScoWaitExp14:
        u8Dummy |=  0x40;
        break;
      case ScoWaitExp15:
        u8Dummy |=  0x50;
        break;
      case ScoWaitExp16:
        u8Dummy |=  0x60;
        break;
      case ScoWaitExp17:
        u8Dummy |=  0x70;
        break;
      case ScoWaitExp18:
        u8Dummy |=  0x80;
        break;
      case ScoWaitExp19:
        u8Dummy |=  0x90;
        break;
      case ScoWaitExp20:
        u8Dummy |=  0xA0;
        break;
      case ScoWaitExp21:
        u8Dummy |=  0xB0;
        break;
      default:
        return ErrorInvalidParameter ;
    }
  }
  
  // Update hardware
  FM4_CRG->CSW_TMR = u8Dummy;
  
  return Ok;
}

/**
 ******************************************************************************
 ** \brief Wait for Sub Oscillation Stabilization
 **
 ** This function waits for the Sub Oscillator stabilization via polling.
 ** PDL_WAIT_LOOP_HOOK() is called during polling. It should be called, if the
 ** system needs a stable sub clock (i.e. for communication).
 **
 ** \param [in]  u32MaxTimeOut  Time out counter start value
 ** 
 ** \retval Ok                  Clock stabilized
 ** \retval ErrorTimeout        Clock not stabilized after timeout count
 ******************************************************************************/
en_result_t Clk_WaitForSubOscillator(uint32_t u32MaxTimeOut)
{
  uint32_t u32Timeout = u32MaxTimeOut;
  
  while (u32Timeout--)
  {
    if (FM4_CRG->SCM_STR_f.SORDY == 1u)
    {
      return Ok;
    }
    
    PDL_WAIT_LOOP_HOOK();
  }
  
  return ErrorTimeout;
} // Clk_WaitForSubOscillator

/**
 ******************************************************************************
 ** \brief Request VCC Power-Down
 **
 ** This function sets the 32 kHz oscillation control disable of the RTC.
 **
 ** \retval Ok                  Disable set
 ******************************************************************************/
en_result_t Clk_RequestVccPowerDown(void)
{
  FM4_RTC->WTOSCCNT_f.SOSCNTL = 0u;
  
  return Ok;
} // Clk_RequestVccPowerDown

/**
 ******************************************************************************
 ** \brief Enables the clock gate of a peripheral
 **
 ** This function sets the corresponding bit in the CKENn register to enable
 ** the clock of a peripheral.
 **
 ** \param  enPeripheral           Enumerator of a peripheral, see
 **                                #en_clk_gate_peripheral_t for details
 **
 ** \retval Ok                     Peripheral clock enabled
 ** \retval ErrorInvalidParameter  Peripheral enumerator does not exist
 ******************************************************************************/
en_result_t Clk_PeripheralClockEnable(en_clk_gate_peripheral_t enPeripheral)
{
  switch (enPeripheral)
  {
    case ClkGateGpio:
      FM4_CLK_GATING->CKEN0_f.GIOCK = 1u;
      break;
    case ClkGateExtif:
      FM4_CLK_GATING->CKEN0_f.EXBCK = 1u;
      break;
    case ClkGateDma:
      FM4_CLK_GATING->CKEN0_f.DMACK = 1u;
      break;
    case ClkGateAdc0:
      FM4_CLK_GATING->CKEN0_f.ADCCK0 = 1u;
      break;
    case ClkGateAdc1:
      FM4_CLK_GATING->CKEN0_f.ADCCK1 = 1u;
      break;
    case ClkGateAdc2:
      FM4_CLK_GATING->CKEN0_f.ADCCK2 = 1u;
      break;
    case ClkGateAdc3:
      FM4_CLK_GATING->CKEN0_f.ADCCK3 = 1u;
      break;
    case ClkGateMfs0:
      FM4_CLK_GATING->CKEN0_f.MFSCK0 = 1u;
      break;
    case ClkGateMfs1:
      FM4_CLK_GATING->CKEN0_f.MFSCK1 = 1u;
      break;
    case ClkGateMfs2:
      FM4_CLK_GATING->CKEN0_f.MFSCK2 = 1u;
      break;
    case ClkGateMfs3:
      FM4_CLK_GATING->CKEN0_f.MFSCK3 = 1u;
      break;
    case ClkGateMfs4:
      FM4_CLK_GATING->CKEN0_f.MFSCK4 = 1u;
      break;
    case ClkGateMfs5:
      FM4_CLK_GATING->CKEN0_f.MFSCK5 = 1u;
      break;
    case ClkGateMfs6:
      FM4_CLK_GATING->CKEN0_f.MFSCK6 = 1u;
      break;
    case ClkGateMfs7:
      FM4_CLK_GATING->CKEN0_f.MFSCK7 = 1u;
      break;
    case ClkGateMfs8:
      FM4_CLK_GATING->CKEN0_f.MFSCK8 = 1u;
      break;
    case ClkGateMfs9:
      FM4_CLK_GATING->CKEN0_f.MFSCK9 = 1u;
      break;
    case ClkGateMfs10:
      FM4_CLK_GATING->CKEN0_f.MFSCK10 = 1u;
      break;
    case ClkGateMfs11:
      FM4_CLK_GATING->CKEN0_f.MFSCK11 = 1u;
      break;
    case ClkGateMfs12:
      FM4_CLK_GATING->CKEN0_f.MFSCK12 = 1u;
      break;
    case ClkGateMfs13:
      FM4_CLK_GATING->CKEN0_f.MFSCK13 = 1u;
      break;
    case ClkGateMfs14:
      FM4_CLK_GATING->CKEN0_f.MFSCK14 = 1u;
      break;
    case ClkGateMfs15:
      FM4_CLK_GATING->CKEN0_f.MFSCK15 = 1u;
      break;
    case ClkGateQprc0:
      FM4_CLK_GATING->CKEN1_f.QDUCK0 = 1u;
      break;
    case ClkGateQprc1:
      FM4_CLK_GATING->CKEN1_f.QDUCK1 = 1u;
      break;
    case ClkGateQprc2:
      FM4_CLK_GATING->CKEN1_f.QDUCK2 = 1u;
      break;
    case ClkGateQprc3:
      FM4_CLK_GATING->CKEN1_f.QDUCK3 = 1u;
      break;
    case ClkGateMft0:
      FM4_CLK_GATING->CKEN1_f.MFTCK0 = 1u;
      break;
    case ClkGateMft1:
      FM4_CLK_GATING->CKEN1_f.MFTCK1 = 1u;
      break;
    case ClkGateMft2:
      FM4_CLK_GATING->CKEN1_f.MFTCK2 = 1u;
      break;
    case ClkGateMft3:
      FM4_CLK_GATING->CKEN1_f.MFTCK3 = 1u;
      break;
    case ClkGateBt0:
      FM4_CLK_GATING->CKEN1_f.BTMCK0 = 1u;
      break;
    case ClkGateBt4:
      FM4_CLK_GATING->CKEN1_f.BTMCK1 = 1u;
      break;
    case ClkGateBt8:
      FM4_CLK_GATING->CKEN1_f.BTMCK2 = 1u;
      break;
    case ClkGateBt12:
      FM4_CLK_GATING->CKEN1_f.BTMCK3 = 1u;
      break;
    case ClkGateSdIf:
      FM4_CLK_GATING->CKEN2_f.SDCCK = 1u;
      break;
    case ClkGateCan0:
      FM4_CLK_GATING->CKEN2_f.CANCK0 = 1u;
      break;
    case ClkGateCan1:
      FM4_CLK_GATING->CKEN2_f.CANCK1 = 1u;
      break;
    case ClkGateUsb0:
      FM4_CLK_GATING->CKEN2_f.USBCK0 = 1u;
      break;
    case ClkGateUsb1:
      FM4_CLK_GATING->CKEN2_f.USBCK1 = 1u;
      break;
    default:
      return ErrorInvalidParameter;
  }
  
  return Ok;
} // Clk_PeripheralClockEnable

/**
 ******************************************************************************
 ** \brief Read the clock gate state of a peripheral
 **
 ** This function reads out the corresponding bit in the CKENn register.
 **
 ** \param  enPeripheral           Enumerator of a peripheral, see
 **                                #en_clk_gate_peripheral_t for details
 **
 ** \retval TRUE                   Peripheral clock enabled
 ** \retval FALSE                  Peripheral clock not enabled, peripheral
 **                                not existing
 ******************************************************************************/
boolean_t Clk_PeripheralGetClockState(en_clk_gate_peripheral_t enPeripheral)
{
  switch (enPeripheral)
  {
    case ClkGateGpio:
      return ((1u == FM4_CLK_GATING->CKEN0_f.GIOCK) ? TRUE : FALSE);
    case ClkGateExtif:
      return ((1u == FM4_CLK_GATING->CKEN0_f.EXBCK) ? TRUE : FALSE);
    case ClkGateDma:
      return ((1u == FM4_CLK_GATING->CKEN0_f.DMACK) ? TRUE : FALSE);
    case ClkGateAdc0:
      return ((1u == FM4_CLK_GATING->CKEN0_f.ADCCK0) ? TRUE : FALSE);
    case ClkGateAdc1:
      return ((1u == FM4_CLK_GATING->CKEN0_f.ADCCK1) ? TRUE : FALSE);
    case ClkGateAdc2:
      return ((1u == FM4_CLK_GATING->CKEN0_f.ADCCK2) ? TRUE : FALSE);
    case ClkGateAdc3:
      return ((1u == FM4_CLK_GATING->CKEN0_f.ADCCK3) ? TRUE : FALSE);
    case ClkGateMfs0:
      return ((1u == FM4_CLK_GATING->CKEN0_f.MFSCK0) ? TRUE : FALSE);
    case ClkGateMfs1:
      return ((1u == FM4_CLK_GATING->CKEN0_f.MFSCK1) ? TRUE : FALSE);
    case ClkGateMfs2:
      return ((1u == FM4_CLK_GATING->CKEN0_f.MFSCK2) ? TRUE : FALSE);
    case ClkGateMfs3:
      return ((1u == FM4_CLK_GATING->CKEN0_f.MFSCK3) ? TRUE : FALSE);
    case ClkGateMfs4:
      return ((1u == FM4_CLK_GATING->CKEN0_f.MFSCK4) ? TRUE : FALSE);
    case ClkGateMfs5:
      return ((1u == FM4_CLK_GATING->CKEN0_f.MFSCK5) ? TRUE : FALSE);
    case ClkGateMfs6:
      return ((1u == FM4_CLK_GATING->CKEN0_f.MFSCK6) ? TRUE : FALSE);
    case ClkGateMfs7:
      return ((1u == FM4_CLK_GATING->CKEN0_f.MFSCK7) ? TRUE : FALSE);
    case ClkGateMfs8:
      return ((1u == FM4_CLK_GATING->CKEN0_f.MFSCK8) ? TRUE : FALSE);
    case ClkGateMfs9:
      return ((1u == FM4_CLK_GATING->CKEN0_f.MFSCK9) ? TRUE : FALSE);
    case ClkGateMfs10:
      return ((1u ==  FM4_CLK_GATING->CKEN0_f.MFSCK10) ? TRUE : FALSE);
    case ClkGateMfs11:
      return ((1u == FM4_CLK_GATING->CKEN0_f.MFSCK11) ? TRUE : FALSE);
    case ClkGateMfs12:
      return ((1u == FM4_CLK_GATING->CKEN0_f.MFSCK12) ? TRUE : FALSE);
    case ClkGateMfs13:
      return ((1u == FM4_CLK_GATING->CKEN0_f.MFSCK13) ? TRUE : FALSE);
    case ClkGateMfs14:
      return ((1u == FM4_CLK_GATING->CKEN0_f.MFSCK14) ? TRUE : FALSE);
    case ClkGateMfs15:
      return ((1u == FM4_CLK_GATING->CKEN0_f.MFSCK15) ? TRUE : FALSE);
    case ClkGateQprc0:
      return ((1u == FM4_CLK_GATING->CKEN1_f.QDUCK0) ? TRUE : FALSE);
    case ClkGateQprc1:
      return ((1u == FM4_CLK_GATING->CKEN1_f.QDUCK1) ? TRUE : FALSE);
    case ClkGateQprc2:
      return ((1u == FM4_CLK_GATING->CKEN1_f.QDUCK2) ? TRUE : FALSE);
    case ClkGateQprc3:
      return ((1u == FM4_CLK_GATING->CKEN1_f.QDUCK3) ? TRUE : FALSE);
    case ClkGateMft0:
      return ((1u == FM4_CLK_GATING->CKEN1_f.MFTCK0) ? TRUE : FALSE);
    case ClkGateMft1:
      return ((1u == FM4_CLK_GATING->CKEN1_f.MFTCK1) ? TRUE : FALSE);
    case ClkGateMft2:
      return ((1u == FM4_CLK_GATING->CKEN1_f.MFTCK2) ? TRUE : FALSE);
    case ClkGateMft3:
      return ((1u == FM4_CLK_GATING->CKEN1_f.MFTCK3) ? TRUE : FALSE);
    case ClkGateBt0:
      return ((1u == FM4_CLK_GATING->CKEN1_f.BTMCK0) ? TRUE : FALSE);
    case ClkGateBt4:
      return ((1u == FM4_CLK_GATING->CKEN1_f.BTMCK1) ? TRUE : FALSE);
    case ClkGateBt8:
      return ((1u == FM4_CLK_GATING->CKEN1_f.BTMCK2) ? TRUE : FALSE);
    case ClkGateBt12:
      return ((1u == FM4_CLK_GATING->CKEN1_f.BTMCK3) ? TRUE : FALSE);
    case ClkGateSdIf:
      return ((1u == FM4_CLK_GATING->CKEN2_f.SDCCK) ? TRUE : FALSE);
    case ClkGateCan0:
      return ((1u == FM4_CLK_GATING->CKEN2_f.CANCK0) ? TRUE : FALSE);
    case ClkGateCan1:
      return ((1u == FM4_CLK_GATING->CKEN2_f.CANCK1) ? TRUE : FALSE);
    case ClkGateUsb0:
      return ((1u == FM4_CLK_GATING->CKEN2_f.USBCK0) ? TRUE : FALSE);
    case ClkGateUsb1:
      return ((1u == FM4_CLK_GATING->CKEN2_f.USBCK1) ? TRUE : FALSE);
    default:
      break;
  }
  
  return FALSE; // Peripheral not found -> always FALSE
} // Clk_PeripheralGetClockState

/**
 ******************************************************************************
 ** \brief Disables the clock gate of a peripheral
 **
 ** This function clears the corresponding bit in the CKENn register to enable
 ** the clock of a peripheral.
 **
 ** \param  enPeripheral           Enumerator of a peripheral, see
 **                                #en_clk_gate_peripheral_t for details
 **
 ** \retval Ok                     Peripheral clock disabled
 ** \retval ErrorInvalidParameter  Peripheral enumerator does not exist
 ******************************************************************************/
en_result_t Clk_PeripheralClockDisable(en_clk_gate_peripheral_t enPeripheral)
{
  switch (enPeripheral)
  {
    case ClkGateGpio:
      FM4_CLK_GATING->CKEN0_f.GIOCK = 0u;
      break;
    case ClkGateExtif:
      FM4_CLK_GATING->CKEN0_f.EXBCK = 0u;
      break;
    case ClkGateDma:
      FM4_CLK_GATING->CKEN0_f.DMACK = 0u;
      break;
    case ClkGateAdc0:
      FM4_CLK_GATING->CKEN0_f.ADCCK0 = 0u;
      break;
    case ClkGateAdc1:
      FM4_CLK_GATING->CKEN0_f.ADCCK1 = 0u;
      break;
    case ClkGateAdc2:
      FM4_CLK_GATING->CKEN0_f.ADCCK2 = 0u;
      break;
    case ClkGateAdc3:
      FM4_CLK_GATING->CKEN0_f.ADCCK3 = 0u;
      break;
    case ClkGateMfs0:
      FM4_CLK_GATING->CKEN0_f.MFSCK0 = 0u;
      break;
    case ClkGateMfs1:
      FM4_CLK_GATING->CKEN0_f.MFSCK1 = 0u;
      break;
    case ClkGateMfs2:
      FM4_CLK_GATING->CKEN0_f.MFSCK2 = 0u;
      break;
    case ClkGateMfs3:
      FM4_CLK_GATING->CKEN0_f.MFSCK3 = 0u;
      break;
    case ClkGateMfs4:
      FM4_CLK_GATING->CKEN0_f.MFSCK4 = 0u;
      break;
    case ClkGateMfs5:
      FM4_CLK_GATING->CKEN0_f.MFSCK5 = 0u;
      break;
    case ClkGateMfs6:
      FM4_CLK_GATING->CKEN0_f.MFSCK6 = 0u;
      break;
    case ClkGateMfs7:
      FM4_CLK_GATING->CKEN0_f.MFSCK7 = 0u;
      break;
    case ClkGateMfs8:
      FM4_CLK_GATING->CKEN0_f.MFSCK8 = 0u;
      break;
    case ClkGateMfs9:
      FM4_CLK_GATING->CKEN0_f.MFSCK9 = 0u;
      break;
    case ClkGateMfs10:
      FM4_CLK_GATING->CKEN0_f.MFSCK10 = 0u;
      break;
    case ClkGateMfs11:
      FM4_CLK_GATING->CKEN0_f.MFSCK11 = 0u;
      break;
    case ClkGateMfs12:
      FM4_CLK_GATING->CKEN0_f.MFSCK12 = 0u;
      break;
    case ClkGateMfs13:
      FM4_CLK_GATING->CKEN0_f.MFSCK13 = 0u;
      break;
    case ClkGateMfs14:
      FM4_CLK_GATING->CKEN0_f.MFSCK14 = 0u;
      break;
    case ClkGateMfs15:
      FM4_CLK_GATING->CKEN0_f.MFSCK15 = 0u;
      break;
    case ClkGateQprc0:
      FM4_CLK_GATING->CKEN1_f.QDUCK0 = 0u;
      break;
    case ClkGateQprc1:
      FM4_CLK_GATING->CKEN1_f.QDUCK1 = 0u;
      break;
    case ClkGateQprc2:
      FM4_CLK_GATING->CKEN1_f.QDUCK2 = 0u;
      break;
    case ClkGateQprc3:
      FM4_CLK_GATING->CKEN1_f.QDUCK3 = 0u;
      break;
    case ClkGateMft0:
      FM4_CLK_GATING->CKEN1_f.MFTCK0 = 0u;
      break;
    case ClkGateMft1:
      FM4_CLK_GATING->CKEN1_f.MFTCK1 = 0u;
      break;
    case ClkGateMft2:
      FM4_CLK_GATING->CKEN1_f.MFTCK2 = 0u;
      break;
    case ClkGateMft3:
      FM4_CLK_GATING->CKEN1_f.MFTCK3 = 0u;
      break;
    case ClkGateBt0:
      FM4_CLK_GATING->CKEN1_f.BTMCK0 = 0u;
      break;
    case ClkGateBt4:
      FM4_CLK_GATING->CKEN1_f.BTMCK1 = 0u;
      break;
    case ClkGateBt8:
      FM4_CLK_GATING->CKEN1_f.BTMCK2 = 0u;
      break;
    case ClkGateBt12:
      FM4_CLK_GATING->CKEN1_f.BTMCK3 = 0u;
      break;
    case ClkGateSdIf:
      FM4_CLK_GATING->CKEN2_f.SDCCK = 0u;
      break;
    case ClkGateCan0:
      FM4_CLK_GATING->CKEN2_f.CANCK0 = 0u;
      break;
    case ClkGateCan1:
      FM4_CLK_GATING->CKEN2_f.CANCK1 = 0u;
      break;
    case ClkGateUsb0:
      FM4_CLK_GATING->CKEN2_f.USBCK0 = 0u;
      break;
    case ClkGateUsb1:
      FM4_CLK_GATING->CKEN2_f.USBCK1 = 0u;
      break;
    default:
      return ErrorInvalidParameter;
  }
  
  return Ok;
} // Clk_PeripheralClockDisable

/**
 ******************************************************************************
 ** \brief Set reset bit a peripheral
 **
 ** This function sets the corresponding bit in the MRSTn register to set
 ** a peripheral in reset state.
 **
 ** \param  enPeripheral           Enumerator of a peripheral, see
 **                                #en_clk_reset_peripheral_t for details
 **
 ** \retval Ok                     Peripheral clock enabled
 ** \retval ErrorInvalidParameter  Peripheral enumerator does not exist
 ******************************************************************************/
en_result_t Clk_PeripheralSetReset(en_clk_reset_peripheral_t enPeripheral)
{
  switch (enPeripheral)
  {
    case ClkResetExtif:
      FM4_CLK_GATING->MRST0_f.EXBRST = 1u;
      break;
    case ClkResetDma:
      FM4_CLK_GATING->MRST0_f.DMARST = 1u;
      break;
    case ClkResetAdc0:
      FM4_CLK_GATING->MRST0_f.ADCRST0 = 1u;
      break;
    case ClkResetAdc1:
      FM4_CLK_GATING->MRST0_f.ADCRST1 = 1u;
      break;
    case ClkResetAdc2:
      FM4_CLK_GATING->MRST0_f.ADCRST2 = 1u;
      break;
    case ClkResetAdc3:
      FM4_CLK_GATING->MRST0_f.ADCRST3 = 1u;
      break;
    case ClkResetMfs0:
      FM4_CLK_GATING->MRST0_f.MFSRST0 = 1u;
      break;
    case ClkResetMfs1:
      FM4_CLK_GATING->MRST0_f.MFSRST1 = 1u;
      break;
    case ClkResetMfs2:
      FM4_CLK_GATING->MRST0_f.MFSRST2 = 1u;
      break;
    case ClkResetMfs3:
      FM4_CLK_GATING->MRST0_f.MFSRST3 = 1u;
      break;
    case ClkResetMfs4:
      FM4_CLK_GATING->MRST0_f.MFSRST4 = 1u;
      break;
    case ClkResetMfs5:
      FM4_CLK_GATING->MRST0_f.MFSRST5 = 1u;
      break;
    case ClkResetMfs6:
      FM4_CLK_GATING->MRST0_f.MFSRST6 = 1u;
      break;
    case ClkResetMfs7:
      FM4_CLK_GATING->MRST0_f.MFSRST7 = 1u;
      break;
    case ClkResetMfs8:
      FM4_CLK_GATING->MRST0_f.MFSRST8 = 1u;
      break;
    case ClkResetMfs9:
      FM4_CLK_GATING->MRST0_f.MFSRST9 = 1u;
      break;
    case ClkResetMfs10:
      FM4_CLK_GATING->MRST0_f.MFSRST10 = 1u;
      break;
    case ClkResetMfs11:
      FM4_CLK_GATING->MRST0_f.MFSRST11 = 1u;
      break;
    case ClkResetMfs12:
      FM4_CLK_GATING->MRST0_f.MFSRST12 = 1u;
      break;
    case ClkResetMfs13:
      FM4_CLK_GATING->MRST0_f.MFSRST13 = 1u;
      break;
    case ClkResetMfs14:
      FM4_CLK_GATING->MRST0_f.MFSRST14 = 1u;
      break;
    case ClkResetMfs15:
      FM4_CLK_GATING->MRST0_f.MFSRST15 = 1u;
      break;
    case ClkResetQprc0:
      FM4_CLK_GATING->MRST1_f.QDURST0 = 1u;
      break;
    case ClkResetQprc1:
      FM4_CLK_GATING->MRST1_f.QDURST1 = 1u;
      break;
    case ClkResetQprc2:
      FM4_CLK_GATING->MRST1_f.QDURST2 = 1u;
      break;
    case ClkResetQprc3:
      FM4_CLK_GATING->MRST1_f.QDURST3 = 1u;
      break;
    case ClkResetMft0:
      FM4_CLK_GATING->MRST1_f.MFTRST0 = 1u;
      break;
    case ClkResetMft1:
      FM4_CLK_GATING->MRST1_f.MFTRST1 = 1u;
      break;
    case ClkResetMft2:
      FM4_CLK_GATING->MRST1_f.MFTRST2 = 1u;
      break;
    case ClkResetMft3:
      FM4_CLK_GATING->MRST1_f.MFTRST3 = 1u;
      break;
    case ClkResetBt0:
      FM4_CLK_GATING->MRST1_f.BTMRST0 = 1u;
      break;
    case ClkResetBt4:
      FM4_CLK_GATING->MRST1_f.BTMRST1 = 1u;
      break;
    case ClkResetBt8:
      FM4_CLK_GATING->MRST1_f.BTMRST2 = 1u;
      break;
    case ClkResetBt12:
      FM4_CLK_GATING->MRST1_f.BTMRST3 = 1u;
      break;
    case ClkResetSdIf:
      FM4_CLK_GATING->MRST2_f.SDCRST = 1u;
      break;
    case ClkResetCan0:
      FM4_CLK_GATING->MRST2_f.CANRST0 = 1u;
      break;
    case ClkResetCan1:
      FM4_CLK_GATING->MRST2_f.CANRST1 = 1u;
      break;
    case ClkResetUsb0:
      FM4_CLK_GATING->MRST2_f.USBRST0 = 1u;
      break;
    case ClkResetUsb1:
      FM4_CLK_GATING->MRST2_f.USBRST1 = 1u;
      break;
    default:
      return ErrorInvalidParameter;
  }
  
  return Ok;
} // Clk_PeripheralSetReset

/**
 ******************************************************************************
 ** \brief Clear reset bit a peripheral
 **
 ** This function clears the corresponding bit in the MRSTn register to release
 ** a peripheral from reset state.
 **
 ** \param  enPeripheral           Enumerator of a peripheral, see
 **                                #en_clk_reset_peripheral_t for details
 **
 ** \retval Ok                     Peripheral clock enabled
 ** \retval ErrorInvalidParameter  Peripheral enumerator does not exist
 ******************************************************************************/
en_result_t Clk_PeripheralClearReset(en_clk_reset_peripheral_t enPeripheral)
{
  switch (enPeripheral)
  {
    case ClkResetExtif:
      FM4_CLK_GATING->MRST0_f.EXBRST = 0u;
      break;
    case ClkResetDma:
      FM4_CLK_GATING->MRST0_f.DMARST = 0u;
      break;
    case ClkResetAdc0:
      FM4_CLK_GATING->MRST0_f.ADCRST0 = 0u;
      break;
    case ClkResetAdc1:
      FM4_CLK_GATING->MRST0_f.ADCRST1 = 0u;
      break;
    case ClkResetAdc2:
      FM4_CLK_GATING->MRST0_f.ADCRST2 = 0u;
      break;
    case ClkResetAdc3:
      FM4_CLK_GATING->MRST0_f.ADCRST3 = 0u;
      break;
    case ClkResetMfs0:
      FM4_CLK_GATING->MRST0_f.MFSRST0 = 0u;
      break;
    case ClkResetMfs1:
      FM4_CLK_GATING->MRST0_f.MFSRST1 = 0u;
      break;
    case ClkResetMfs2:
      FM4_CLK_GATING->MRST0_f.MFSRST2 = 0u;
      break;
    case ClkResetMfs3:
      FM4_CLK_GATING->MRST0_f.MFSRST3 = 0u;
      break;
    case ClkResetMfs4:
      FM4_CLK_GATING->MRST0_f.MFSRST4 = 0u;
      break;
    case ClkResetMfs5:
      FM4_CLK_GATING->MRST0_f.MFSRST5 = 0u;
      break;
    case ClkResetMfs6:
      FM4_CLK_GATING->MRST0_f.MFSRST6 = 0u;
      break;
    case ClkResetMfs7:
      FM4_CLK_GATING->MRST0_f.MFSRST7 = 0u;
      break;
    case ClkResetMfs8:
      FM4_CLK_GATING->MRST0_f.MFSRST8 = 0u;
      break;
    case ClkResetMfs9:
      FM4_CLK_GATING->MRST0_f.MFSRST9 = 0u;
      break;
    case ClkResetMfs10:
      FM4_CLK_GATING->MRST0_f.MFSRST10 = 0u;
      break;
    case ClkResetMfs11:
      FM4_CLK_GATING->MRST0_f.MFSRST11 = 0u;
      break;
    case ClkResetMfs12:
      FM4_CLK_GATING->MRST0_f.MFSRST12 = 0u;
      break;
    case ClkResetMfs13:
      FM4_CLK_GATING->MRST0_f.MFSRST13 = 0u;
      break;
    case ClkResetMfs14:
      FM4_CLK_GATING->MRST0_f.MFSRST14 = 0u;
      break;
    case ClkResetMfs15:
      FM4_CLK_GATING->MRST0_f.MFSRST15 = 0u;
      break;
    case ClkResetQprc0:
      FM4_CLK_GATING->MRST1_f.QDURST0 = 0u;
      break;
    case ClkResetQprc1:
      FM4_CLK_GATING->MRST1_f.QDURST1 = 0u;
      break;
    case ClkResetQprc2:
      FM4_CLK_GATING->MRST1_f.QDURST2 = 0u;
      break;
    case ClkResetQprc3:
      FM4_CLK_GATING->MRST1_f.QDURST3 = 0u;
      break;
    case ClkResetMft0:
      FM4_CLK_GATING->MRST1_f.MFTRST0 = 0u;
      break;
    case ClkResetMft1:
      FM4_CLK_GATING->MRST1_f.MFTRST1 = 0u;
      break;
    case ClkResetMft2:
      FM4_CLK_GATING->MRST1_f.MFTRST2 = 0u;
      break;
    case ClkResetMft3:
      FM4_CLK_GATING->MRST1_f.MFTRST3 = 0u;
      break;
    case ClkResetBt0:
      FM4_CLK_GATING->MRST1_f.BTMRST0 = 0u;
      break;
    case ClkResetBt4:
      FM4_CLK_GATING->MRST1_f.BTMRST1 = 0u;
      break;
    case ClkResetBt8:
      FM4_CLK_GATING->MRST1_f.BTMRST2 = 0u;
      break;
    case ClkResetBt12:
      FM4_CLK_GATING->MRST1_f.BTMRST3 = 0u;
      break;
    case ClkResetSdIf:
      FM4_CLK_GATING->MRST2_f.SDCRST = 0u;
      break;
    case ClkResetCan0:
      FM4_CLK_GATING->MRST2_f.CANRST0 = 0u;
      break;
    case ClkResetCan1:
      FM4_CLK_GATING->MRST2_f.CANRST1 = 0u;
      break;
    case ClkResetUsb0:
      FM4_CLK_GATING->MRST2_f.USBRST0 = 0u;
      break;
    case ClkResetUsb1:
      FM4_CLK_GATING->MRST2_f.USBRST1 = 0u;
      break;
    default:
      return ErrorInvalidParameter;
  }
  
  return Ok;
} // Clk_PeripheralClearReset
       
/**
 ******************************************************************************
 ** \brief Switch to Main Clock
 **
 ** This function sets HCLK to Main Clock oscillation.
 **
 ** \retval Ok                  HCLK is Main Clock oscillation
 ** \retval ErrorInvalidMode    Main clock not available or not enabled
 ** \retval ErrorNotReady       Main clock enabled but oscillation not stable
 **                             yet
 ******************************************************************************/
en_result_t Clk_SwitchToMainClock(void)
{
  if (FALSE == FM4_CRG->SCM_CTL_f.MOSCE)
  {
    return ErrorInvalidMode;
  }
  
  if (FALSE == FM4_CRG->SCM_STR_f.MORDY)
  {
    return ErrorNotReady;
  }
  
  FM4_CRG->SCM_CTL_f.RCS = 1u;  // Main clock mode
  
  return Ok;
} // Clk_SwitchToMainClock

/**
 ******************************************************************************
 ** \brief Switch to Main PLL Clock
 **
 ** This function sets HCLK to Main PLL Clock oscillation. The function expects
 ** enabled Main and PLL clock oscillation.
 **
 ** \note By transition from HS-CR PLL clock mode, no waiting stabilization time
 **       is done for Main Clock PLL mode. 
 **
 ** \retval Ok                  HCLK is Main Clock PLL oscillation
 ** \retval ErrorInvalidMode    Main/PLL clock not available or not enabled
 ** \retval ErrorNotReady       Main/PLL clock enabled but oscillation not
 **                             stable yet
 ******************************************************************************/
en_result_t Clk_SwitchToMainPllClock(void)
{
  if ((FALSE == FM4_CRG->SCM_CTL_f.MOSCE) ||
      (FALSE == FM4_CRG->SCM_CTL_f.PLLE))
  {
    return ErrorInvalidMode;
  }
  
  if ((FALSE == FM4_CRG->SCM_STR_f.MORDY) ||
      (FALSE == FM4_CRG->SCM_STR_f.PLRDY))
  {
    return ErrorNotReady;
  }
  
  if (FALSE == FM4_CRG->PSW_TMR_f.PINC) // Main PLL
  {
    FM4_CRG->SCM_CTL_f.RCS  = 2u;       // Main clock PLL mode
  }
  else  // if HS-CR PLL
  {
    FM4_CRG->SCM_CTL_f.RCS  = 0u;       // First switch to HS-CR temporary
    FM4_CRG->SCM_CTL_f.PLLE = 0u;       // Switch off PLL temporary
    FM4_CRG->PSW_TMR_f.PINC = 0u;       // Set PLL-Main clock connection
    FM4_CRG->SCM_CTL_f.PLLE = 1u;       // Turn on PLL again
    FM4_CRG->SCM_CTL_f.RCS  = 2u;       // Main clock PLL mode
  }
  
  return Ok;
} // Clk_SwitchToMainPllClock

/**
 ******************************************************************************
 ** \brief Switch to Sub Clock
 **
 ** This function sets HCLK to Sub Clock oscillation.
 **
 ** \retval Ok                  HCLK is Sub Clock oscillation
 ** \retval ErrorInvalidMode    Sub clock not available or not enabled
 ******************************************************************************/
en_result_t Clk_SwitchToSubClock(void)
{
  if (FALSE == FM4_CRG->SCM_CTL_f.SOSCE)
  {
    return ErrorInvalidMode;
  }
  
  if (FALSE == FM4_CRG->SCM_STR_f.SORDY)
  {
    return ErrorNotReady;
  }
  
  FM4_CRG->SCM_CTL_f.RCS = 5u;  // Sub clock mode
  
  return Ok;
} // Clk_SwitchToSubClock

/**
 ******************************************************************************
 ** \brief Switch to Low Speed CR Clock
 **
 ** This function sets HCLK to Low Speed CR Clock oscillation.
 **
 ** \retval Ok                  HCLK is Low Speed CR Clock oscillation
 ******************************************************************************/
en_result_t Clk_SwitchToLsCrClock(void)
{
  FM4_CRG->SCM_CTL_f.RCS = 4u;  // LS-CR clock mode
  
  return Ok;
} // Clk_SwitchToLsCrClock

/**
 ******************************************************************************
 ** \brief Switch to High Speed CR Clock
 **
 ** This function sets HCLK to High Speed CR Clock oscillation.
 **
 ** \retval Ok                  HCLK is High Speed CR Clock oscillation
 ******************************************************************************/
en_result_t Clk_SwitchToHsCrClock(void)
{
  FM4_CRG->SCM_CTL_f.RCS = 0u;  // HS-CR clock mode
  
  return Ok;
} // Clk_SwitchToHsCrClock

//@} // ClkGroup

/**
 ******************************************************************************
 ** \brief Switch to High Speed CR PLL Clock
 **
 ** This function sets HCLK to High Speed CR Pll Clock oscillation.
 **
 ** \note If PLL is enabled by this function no wait for stabilization is
 **       performed.
 **
 ** \retval Ok                  HCLK is High Speed CR Pll Clock oscillation
 ** \retval ErrorInvalidMode    High Speed CR Pll Clock not available or not
 **                             enabled
 ******************************************************************************/
en_result_t Clk_SwitchToHsCrPllClock(void)
{
  FM4_CRG->SCM_CTL_f.RCS  = 0u;         // First switch to HS-CR temporary
  FM4_CRG->SCM_CTL_f.PLLE = 0u;         // Switch off PLL temporary (if not already disabled)
  
  if (FALSE == FM4_CRG->PSW_TMR_f.PINC) // Main PLL?
  {    
    FM4_CRG->PSW_TMR_f.PINC = 1u;       // Set PLL-Main clock connection
  }
    
  FM4_CRG->SCM_CTL_f.PLLE = 1u;         // Turn on PLL again
  FM4_CRG->SCM_CTL_f.RCS  = 2u;         // HS-CR PLL mode
  
  return Ok;
} // Clk_SwitchToHsCrPllClock

//@} // ClkGroup

#endif // #if (defined(PDL_PERIPHERAL_ENABLE_CLK))

/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
