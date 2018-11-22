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
/**************************************************************************************************/
/** \file extif.c
 **
 ** A detailed description is available at 
 ** @link ExtifGroup EXTIF description @endlink
 **
 ** History:
 **   - 2013-04-16  1.0  MWi  First version
 **   - 2013-12-02  1.1  MWi  Timing constraints check introduced for TIM.WACC,
 **                           TIM.WADC, TIM.WWEC, TIM.RADC, and TIM.RACC;
 **                           pstcConfig->enWriteAccessCycle configuration
 **                           corrected
 **   - 2013-01-16  1.2  MWi  Possible area number from 7 to 8 corrected
 **
 ******************************************************************************/

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "extif.h"

#if (defined(PDL_PERIPHERAL_EXTIF_ACTIVE))

/**
 ******************************************************************************
 ** \addtogroup ExtifGroup
 ******************************************************************************/
//@{

/******************************************************************************/
/* Local pre-processor symbols/macros ('#define')                             */
/******************************************************************************/

/******************************************************************************/
/* Global variable definitions (declared in header file with 'extern')        */
/******************************************************************************/
stc_extif_intern_data_t stcExtifInternData;

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
 ** \brief External Bus Interface Error Interrupt Service Routine
 **
 ** This function provides the user callback functions, if defined.
 **
 ******************************************************************************/
void ExtifIrqHandler( void )
{
  if ((TRUE == stcExtifInternData.bSdramErrorInterruptEnable) &&
      (stcExtifInternData.pfnSdramErrorCallback != NULL))
  {
    if (1u == FM4_EXBUS->MEMCERR_f.SDER)
    {
      FM4_EXBUS->MEMCERR_f.SDER = 1u;    // Clear SDRAM error interrupt
      stcExtifInternData.pfnSdramErrorCallback();
    }
  }
      
  if ((TRUE == stcExtifInternData.bSramFlashErrorInterruptEnable) &&
      (stcExtifInternData.pfnSramFlashErrorCallback != NULL))
  {
    if (1u == FM4_EXBUS->MEMCERR_f.SFER)
    {
      FM4_EXBUS->MEMCERR_f.SFER = 1u;    // Clear SRAM/Flash error interrupt
      stcExtifInternData.pfnSramFlashErrorCallback();
    }
  }
}

/**
 ******************************************************************************
 ** \brief Setup (init) an EXT-I/F area
 **
 ** \param u8Area       Extbus I/F area (chip select) number
 ** \param pstcConfig   Pointer to area configuration
 **
 ** \retval Ok                      Setup successful
 ** \retval ErrorInvalidParameter   pstcConfig == NULL, Area number wrong,
 **                                 other invalid setting.
 ** \retval ErrorInvalidMode        SDMODE is set for area different from 8
 ******************************************************************************/
en_result_t Extif_InitArea( uint8_t                  u8Area,
                            stc_extif_area_config_t* pstcConfig
                          )
{
  // Local register predefinitions
  stc_exbus_mode_field_t    stcMODE;
  stc_exbus_tim_field_t     stcTIM;
  stc_exbus_area_field_t    stcAREA;
  stc_exbus_atim_field_t    stcATIM;
  stc_exbus_sdmode_field_t  stcSDMODE;
  stc_exbus_reftim_field_t  stcREFTIM;
  stc_exbus_pwrdwn_field_t  stcPWRDWN;
  stc_exbus_sdtim_field_t   stcSDTIM;
  stc_exbus_memcerr_field_t stcMEMCERR;
  stc_exbus_dclkr_field_t   stcDCLKR;
  stc_exbus_amode_field_t   stcAMODE;
  uint8_t                   u8Dummy;
  
  PDL_ZERO_STRUCT(stcMODE);
  PDL_ZERO_STRUCT(stcTIM);
  PDL_ZERO_STRUCT(stcAREA);
  PDL_ZERO_STRUCT(stcATIM);
  PDL_ZERO_STRUCT(stcSDMODE);
  PDL_ZERO_STRUCT(stcREFTIM);
  PDL_ZERO_STRUCT(stcPWRDWN);
  PDL_ZERO_STRUCT(stcSDTIM);
  PDL_ZERO_STRUCT(stcMEMCERR);
  PDL_ZERO_STRUCT(stcDCLKR);
  PDL_ZERO_STRUCT(stcAMODE);
  
  stcExtifInternData.pfnSdramErrorCallback     = NULL;
  stcExtifInternData.pfnSramFlashErrorCallback = NULL;
  
  if ((u8Area > 8u) || (NULL == pstcConfig))
  {
    return ErrorInvalidParameter;
  }
  
  switch (pstcConfig->enWidth)
  {
    case Extif8Bit:
      stcMODE.WDTH = 0u;
      break;
    case Extif16Bit:
      stcMODE.WDTH = 1u;
      break;
    default:
      return ErrorInvalidParameter;
  }
   
  // MODE Register preparation
  stcMODE.RBMON    = (TRUE == pstcConfig->bReadByteMask)           ? 1ul : 0ul ;
  stcMODE.WEOFF    = (TRUE == pstcConfig->bWriteEnableOff)         ? 1ul : 0ul ;
  stcMODE.NAND     = (TRUE == pstcConfig->bNandFlash)              ? 1ul : 0ul ;
  stcMODE.PAGE     = (TRUE == pstcConfig->bPageAccess)             ? 1ul : 0ul ;
  stcMODE.RDY      = (TRUE == pstcConfig->bRdyOn)                  ? 1ul : 0ul ;
  stcMODE.RDY      = (TRUE == pstcConfig->bRdyOn)                  ? 1ul : 0ul ;
  stcMODE.SHRTDOUT = (TRUE == pstcConfig->bStopDataOutAtFirstIdle) ? 1ul : 0ul ;
  stcMODE.MPXMODE  = (TRUE == pstcConfig->bMultiplexMode)          ? 1ul : 0ul ;
  stcMODE.ALEINV   = (TRUE == pstcConfig->bAleInvert)              ? 1ul : 0ul ;
  stcMODE.MPXDOFF  = (TRUE == pstcConfig->bMpxcsOff)               ? 1ul : 0ul ;
  stcMODE.MPXDOFF  = (TRUE == pstcConfig->bAddrOnDataLinesOff)     ? 1ul : 0ul ;
  stcMODE.MPXCSOF  = (TRUE == pstcConfig->bMpxcsOff)               ? 1ul : 0ul ;
  stcMODE.MOEXEUP  = (TRUE == pstcConfig->bMoexWidthAsFradc)       ? 1ul : 0ul ;
  
  // TIM Register preparation
  if ((Extif0Cycle   == pstcConfig->enReadAccessCycle) ||
      (ExtifDisabled == pstcConfig->enReadAccessCycle))
  {
    return ErrorInvalidParameter;
  }
  stcTIM.RACC  = (uint8_t) (pstcConfig->enReadAccessCycle - 1u);
  
  if ((Extif16Cycle  == pstcConfig->enReadAddressSetupCycle) ||
      (ExtifDisabled == pstcConfig->enReadAddressSetupCycle))
  {
    return ErrorInvalidParameter;
  }
  stcTIM.RADC  = (uint8_t) pstcConfig->enReadAddressSetupCycle;
  
  if ((FALSE == pstcConfig->bPageAccess) && (FALSE == pstcConfig->bMoexWidthAsFradc))
  {
    if ((Extif0Cycle   == pstcConfig->enFirstReadAddressCycle) ||
        (ExtifDisabled == pstcConfig->enFirstReadAddressCycle))
    {
      return ErrorInvalidParameter;
    }
    stcTIM.FRADC = (uint8_t) (pstcConfig->enFirstReadAddressCycle - 1u);
  }
  else if ((TRUE == pstcConfig->bPageAccess) && (FALSE == pstcConfig->bMoexWidthAsFradc)) 
  {
    if ((Extif16Cycle  == pstcConfig->enFirstReadAddressCycle) ||
        (ExtifDisabled == pstcConfig->enFirstReadAddressCycle))
    {
      return ErrorInvalidParameter;
    }
    stcTIM.FRADC = (uint8_t) pstcConfig->enFirstReadAddressCycle;
  }
  else
  {
    return ErrorInvalidParameter;       // Other setting for PAGE and MOEXEUP not allowed!
  }
  
  if ((Extif0Cycle   == pstcConfig->enReadIdleCycle) ||
      (ExtifDisabled == pstcConfig->enReadIdleCycle))
  {
    return ErrorInvalidParameter;
  }
  stcTIM.RIDLC = (uint8_t) (pstcConfig->enReadIdleCycle - 1u);
  
  if ((Extif0Cycle   == pstcConfig->enWriteAccessCycle) ||
      (Extif1Cycle   == pstcConfig->enWriteAccessCycle) ||
      (Extif2Cycle   == pstcConfig->enWriteAccessCycle) ||
      (ExtifDisabled == pstcConfig->enWriteAccessCycle))
  {
    return ErrorInvalidParameter;
  }  
  stcTIM.WACC  = (uint8_t) (pstcConfig->enWriteAccessCycle - 1u);
  
  if ((Extif16Cycle  == pstcConfig->enWriteAddressSetupCycle) ||
      (Extif0Cycle   == pstcConfig->enWriteAddressSetupCycle) ||
      (ExtifDisabled == pstcConfig->enWriteAddressSetupCycle))
  {
    return ErrorInvalidParameter;
  }
  stcTIM.WADC  = (uint8_t) (pstcConfig->enWriteAddressSetupCycle - 1u);
  
  if ((Extif16Cycle  == pstcConfig->enWriteEnableCycle) ||
      (Extif0Cycle   == pstcConfig->enWriteEnableCycle) ||
      (ExtifDisabled == pstcConfig->enWriteEnableCycle))
  {
    return ErrorInvalidParameter;
  }
  stcTIM.WWEC  = (uint8_t) (pstcConfig->enWriteEnableCycle - 1u);
  
  if ((Extif0Cycle   == pstcConfig->enWriteIdleCycle) ||
      (ExtifDisabled == pstcConfig->enWriteIdleCycle))
  {
    return ErrorInvalidParameter;
  }  
  stcTIM.WIDLC = (uint8_t) (pstcConfig->enWriteIdleCycle - 1u);
  
  // Check timing contraints
  // WACC >= WADC + WWEC
  u8Dummy =  (uint8_t)stcTIM.WADC;      // u8Dummy provided because of volatile
  u8Dummy += (uint8_t)stcTIM.WWEC;      //  qualifiers, which cannot be mixed
  if ((uint8_t)stcTIM.WACC < u8Dummy)   //  in one expression
  {
    return ErrorInvalidParameter;
  }
  
  // RADC < RACC
  u8Dummy =  (uint8_t)stcTIM.RACC;      // see above
  if ((uint8_t)stcTIM.RADC >= u8Dummy)
  {
    return ErrorInvalidParameter;
  }
  
  // AREA Register preparation
  stcAREA.ADDR = (uint8_t) pstcConfig->u8AreaAddress;
  stcAREA.MASK = (uint8_t) pstcConfig->enAreaMask;
  
  // ATIM Register preparation
  if ((Extif0Cycle   == pstcConfig->enAddressLatchCycle) ||
      (ExtifDisabled == pstcConfig->enAddressLatchCycle))
  {
    return ErrorInvalidParameter;
  }
  stcATIM.ALC = (uint8_t) (pstcConfig->enAddressLatchCycle - 1u);
  
  if ((Extif16Cycle  == pstcConfig->enReadAddressSetupCycle) ||
      (ExtifDisabled == pstcConfig->enReadAddressSetupCycle))
  {
    return ErrorInvalidParameter;
  }
  stcATIM.ALES = (uint8_t) pstcConfig->enReadAddressSetupCycle;
  
  if ((Extif0Cycle   == pstcConfig->enAddressLatchWidthCycle) ||
      (ExtifDisabled == pstcConfig->enAddressLatchWidthCycle))
  {
    return ErrorInvalidParameter;
  }
  stcATIM.ALEW = (uint8_t) (pstcConfig->enAddressLatchWidthCycle - 1u);
  
  // Check for SDMODE and cheip select area 8
  if ((TRUE == pstcConfig->bSdramEnable) && (8u != u8Area))
  {
    return ErrorInvalidMode;
  }
  
  // SDMODE Register preparation
  stcSDMODE.SDON = (TRUE == pstcConfig->bSdramEnable)        ? 1ul : 0ul ;
  stcSDMODE.PDON = (TRUE == pstcConfig->bSdramPowerDownMode) ? 1ul : 0ul ;
  stcSDMODE.ROFF = (TRUE == pstcConfig->bSdramRefreshOff)    ? 1ul : 0ul ;
  
  switch (pstcConfig->enCasel)
  {
    case ExtifCas16Bit:
      stcSDMODE.CASEL = 0ul;
      break;
    case ExtifCas32Bit:
      stcSDMODE.CASEL = 1ul;
      break;
    default:
      return ErrorInvalidParameter;
  }
  
  stcSDMODE.RASEL  = (uint8_t) pstcConfig->enRasel;
  stcSDMODE.BASEL  = (uint8_t) pstcConfig->enBasel;
  stcSDMODE.MSDCLK = (TRUE == pstcConfig->bMpxcsOff) ? 1ul : 0ul ;
                      
  // REFTIM Register preparation
  stcREFTIM.REFC = pstcConfig->u16RefreshCount;
  stcREFTIM.NREF = pstcConfig->u8RefreshNumber;
  stcREFTIM.PREF = (TRUE == pstcConfig->bRefreshEnable) ? 1ul : 0ul ;
  
  // PWRDWN Register preparation
  stcPWRDWN.PDC = pstcConfig->u16PowerDownCount;
  
  // SDTIM Register preparation
  stcSDTIM.CL    = (uint8_t) pstcConfig->enSdramCasLatencyCycle;
  stcSDTIM.TRC   = (uint8_t) pstcConfig->enSdramRasCycle;
  stcSDTIM.TRP   = (uint8_t) pstcConfig->enSdramRasPrechargeCycle;
  stcSDTIM.TRCD  = (uint8_t) pstcConfig->enSdramRasCasDelayCycle;
  stcSDTIM.TRAS  = (uint8_t) pstcConfig->enSdramRasActiveCycle;
  stcSDTIM.TREFC = (uint8_t) pstcConfig->enSdramRefreshCycle;
  stcSDTIM.TDPL  = (uint8_t) pstcConfig->enSdramPrechargeCycle;
  
  // MEMCERR Register preparation and Callback Pointer intern data settings
  if (TRUE == pstcConfig->bSramFlashErrorInterruptEnable)
  {
    stcMEMCERR.SFION = 1u;
    stcExtifInternData.bSramFlashErrorInterruptEnable = TRUE;
    stcExtifInternData.pfnSramFlashErrorCallback = pstcConfig->pfnSramFlashErrorCallback;
  }
  
  if (TRUE == pstcConfig->bSdramErrorInterruptEnable)
  {
    stcMEMCERR.SDION = 1u;
    stcExtifInternData.bSdramErrorInterruptEnable = TRUE;
    stcExtifInternData.pfnSdramErrorCallback = pstcConfig->pfnSdramErrorCallback;
  }
  
  if ((TRUE == pstcConfig->bSramFlashErrorInterruptEnable) ||
      (TRUE == pstcConfig->bSdramErrorInterruptEnable))
  {
    NVIC_ClearPendingIRQ(EXTBUS_ERR_IRQn);
    NVIC_EnableIRQ(EXTBUS_ERR_IRQn);
    NVIC_SetPriority(EXTBUS_ERR_IRQn, PDL_IRQ_LEVEL_EXTIF);
  }
    
  // DCLKR Register preparation
  if ((pstcConfig->u8MclkDivision == 0u) ||
      (pstcConfig->u8MclkDivision > 16u))
  {
    return ErrorInvalidParameter;
  }
  else
  {
    stcDCLKR.MDIV = (uint8_t) pstcConfig->u8MclkDivision;
  }
  
  stcDCLKR.MCLKON = (TRUE == pstcConfig->bMclkoutEnable) ? 1ul : 0ul ;
  
  // AMODE Register preparation
  stcAMODE.WAEN = (TRUE == pstcConfig->bPrecedReadContinuousWrite) ? 1ul : 0ul ;
  
  // Finally setup hardware
  EXTIF.astcMODE[u8Area] = stcMODE;
  EXTIF.astcTIM[u8Area]  = stcTIM;
  EXTIF.astcAREA[u8Area] = stcAREA;
  EXTIF.astcATIM[u8Area] = stcATIM;
  FM4_EXBUS->SDMODE_f  = stcSDMODE;
  FM4_EXBUS->REFTIM_f  = stcREFTIM;
  FM4_EXBUS->PWRDWN_f  = stcPWRDWN;
  FM4_EXBUS->SDTIM_f   = stcSDTIM;
  FM4_EXBUS->MEMCERR_f = stcMEMCERR;
  FM4_EXBUS->DCLKR_f   = stcDCLKR;
  FM4_EXBUS->AMODE_f   = stcAMODE;
  
  return Ok;
} // Extif_InitArea

/**
 ******************************************************************************
 ** \brief Read Error Status Register
 ** 
 ** \retval Ok      No error response exists
 ** \retval Error   Error response exists
 ******************************************************************************/
en_result_t Extif_ReadErrorStatus( void )
{
  if (TRUE == FM4_EXBUS->EST_f.WERR)
  {
    return Error;
  }
  
  return Ok;
} // Extif_ReadErrorStatus

/**
 ******************************************************************************
 ** \brief Read Error Address Register
 ** 
 ** \return  uint32_t  Error Address 
 ******************************************************************************/
uint32_t Extif_ReadErrorAddress( void )
{
  return FM4_EXBUS->WEAD;
} // Extif_ReadErrorAddress

/**
 ******************************************************************************
 ** \brief Clear Error Status Register
 ** 
 ** \retval Ok      Status Error cleared
 ******************************************************************************/
en_result_t Extif_ClearErrorStatus( void )
{
  FM4_EXBUS->ESCLR_f.WERRCLR = 0u;
  
  return Ok;
} // Extif_ClearErrorStatus

/**
 ******************************************************************************
 ** \brief Check SDRAM command register is ready
 ** 
 ** \retval Ok              Writing to SDCMD register is possible
 ** \retval ErrorNotReady   Access to SDCMD register not possible
 ******************************************************************************/
en_result_t Extif_CheckSdcmdReady( void )
{
  if (TRUE == FM4_EXBUS->SDCMD_f.PEND)
  {
    return ErrorNotReady;
  }
  
  return Ok;
} // Extif_CheckSdcmdReady

/**
 ******************************************************************************
 ** \brief Set a SDRAM command
 **
 ** \note This function calls Extif_CheckSdcmdReady().
 **
 ** \param u16Address   SDRAM address (MAD[15:00] pin values)
 ** \param bMsdwex      MDSWEX pin value
 ** \param bMcasx       MCASX pin value
 ** \param bMrasx       MRASX pin value
 ** \param bMcsx8       MCSX8 pin value
 ** \param bMadcke      MADCKE pin value
 **
 ** \retval Ok              Writing to SDCMD register was successful
 ** \retval ErrorNotReady   Access to SDCMD register was not possible
 ******************************************************************************/
en_result_t Extif_SetSdramCommand( uint16_t  u16Address,
                                   boolean_t bMsdwex,
                                   boolean_t bMcasx,
                                   boolean_t bMrasx,
                                   boolean_t bMcsx8,
                                   boolean_t bMadcke
                                 )
{
  stc_exbus_sdcmd_field_t stcSDCMD;
  
  if (ErrorNotReady ==  Extif_CheckSdcmdReady())
  {
    return ErrorNotReady;
  }
  
  stcSDCMD.SDAD  = u16Address;
  stcSDCMD.SDWE  = bMsdwex;
  stcSDCMD.SDCAS = bMcasx;
  stcSDCMD.SDRAS = bMrasx;
  stcSDCMD.SDCS  = bMcsx8;
  stcSDCMD.SDCKE = bMadcke;
  
  FM4_EXBUS->SDCMD_f = stcSDCMD;
  
  return Ok;
}
//@} // ExtifGroup

#endif // #if (defined(PDL_PERIPHERAL_EXTIF_ACTIVE)
/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
