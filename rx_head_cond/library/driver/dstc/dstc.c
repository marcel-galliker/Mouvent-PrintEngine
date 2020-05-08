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
/** \file dstc.c
 **
 ** A detailed description is available at 
 ** @link DstcGroup DSTC description @endlink
 **
 ** History:
 **   - 2013-04-24  1.0  MWi  First version
 **   - 2014-04-16  1.1  MWi  Typos in comments corrected
 **
 ******************************************************************************/

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "dstc.h"

#if (defined(PDL_PERIPHERAL_DSTC_ACTIVE))

/**
 ******************************************************************************
 ** \addtogroup DstcGroup
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

/******************************************************************************/
/* Local variable definitions ('static')                                      */
/******************************************************************************/
static stc_dstc_intern_data_t stcDstcInternData;

/******************************************************************************/
/* Function implementation - global ('extern') and local ('static')           */
/******************************************************************************/

/**
 ******************************************************************************
 ** \brief Interrupt Service Routine of DSTC
 ******************************************************************************/
void Dstc_IrqHandler(void)
{
  en_dstc_est_error_t enEstError = NoError;
  uint8_t             u8ErrorStatus;
  uint16_t            u16ErrorChannel;     ///< Error channel, if HS == 1; 0xFFFF, if HS == 0
  uint16_t            u16ErrorDesPointer;
  boolean_t           bSoftwareError;
  boolean_t           bDoubleError   = FALSE;
  boolean_t           bErrorStop     = FALSE;
  boolean_t           bError         = FALSE;
  
  // Check for SW transfer flag
  if (TRUE == (FM4_INTREQ->IRQ091MON_f.DSTCINT0))
  {
    Dstc_SetCommand(CmdSwclr);    // SW transfer interrupt clear
  }

  if (FM4_DSTC->MONERS_f.EST != 0u)              // Error Status
  {
    bError = TRUE;
    
    u8ErrorStatus = (uint8_t)FM4_DSTC->MONERS_f.EST;            // Read Error Status
    
    switch (u8ErrorStatus)
    {
      case 1:
        enEstError = SourceAccessError;
        break;
      case 2:        
        enEstError = DestinationAccessError;
        break;
      case 3:
        enEstError = ForcedTransferStop;
        break;
      case 4:
        enEstError = DesAccessError;
        break;
      case 5:
        enEstError = DesOpenError;
        break;
      default:
        enEstError = UnknownError;
    }
    
    if (TRUE == FM4_DSTC->MONERS_f.EHS)
    {
      u16ErrorChannel = (uint16_t) (0x00FFu & FM4_DSTC->MONERS_f.ECH);     // Error Channel
    }
    else
    {
      u16ErrorChannel = 0xFFFFu; // '-1'
    }
    
    u16ErrorDesPointer = (uint16_t)(0x3FFu & FM4_DSTC->MONERS_f.EDESP); // Error DES Pointer

    bSoftwareError = (FALSE == FM4_DSTC->MONERS_f.EHS)   ? TRUE : FALSE;
    bDoubleError   = (TRUE  == FM4_DSTC->MONERS_f.DER)   ? TRUE : FALSE;
    bErrorStop     = (TRUE  == FM4_DSTC->MONERS_f.ESTOP) ? TRUE : FALSE;
    
    Dstc_SetCommand(CmdErclr);                                  // Clear Error
      
    if (stcDstcInternData.pfnErrorCallback != NULL)
    {
      stcDstcInternData.pfnErrorCallback( enEstError,
                                          u16ErrorChannel,
                                          u16ErrorDesPointer,
                                          bSoftwareError,
                                          bDoubleError,
                                          bErrorStop);
    }
  }
  
  if (FALSE == bError)
  {
    if (stcDstcInternData.pfnNotifySwCallback != NULL)
    {         
      stcDstcInternData.pfnNotifySwCallback();
    }
  }
}

/**
 ******************************************************************************
 ** \brief Release DSTC from Standby State
 **
 ** \retval Ok                      DSTC released from standby mode
 ** \retval ErrorNotReady           DSTC standby release failed
 ******************************************************************************/
en_result_t Dstc_ReleaseStandBy(void)
{
  uint32_t u32DstcTimeOut;
 
  Dstc_SetCommand(CmdStandyRelease);
  
  u32DstcTimeOut = DSTC_TRANSITION_TIMEOUT;
  while ((0x02u == FM4_DSTC->CMD) && (0u != u32DstcTimeOut))
  {
    u32DstcTimeOut--;
  }
  
  if ((0u == u32DstcTimeOut) || (0u != FM4_DSTC->CMD))
  {
    return ErrorNotReady;
  }
  
  return Ok;
} // Dstc_ReleaseStandBy

/**
 ******************************************************************************
 ** \brief Initializes the DSTC according the Configuration
 **
 ** \param [in] pstcConfig          Pointer to DSTC Configuration
 **
 ** \retval Ok                      DSTC Initialized
 ** \retval ErrorNotReady           DSTC standby initialization failed
 ** \retval ErrorInvalidParameter   pstcConfig == NULL or other configuration wrong
 ** \retval ErrorAddressAlignment   DES Base Address not aligned to 32 Bit
 ******************************************************************************/
en_result_t Dstc_Init( stc_dstc_config_t* pstcConfig )
{
  stc_dstc_cfg_field_t stcCfg;

  if (NULL == pstcConfig)
  {
    return ErrorInvalidParameter;
  }
  
  if (0u != ((pstcConfig->u32Destp) & 0x00000003ul))
  {
    return ErrorAddressAlignment;
  }
  
  if (Ok != Dstc_ReleaseStandBy())
  {
    return ErrorNotReady;
  }  

  stcDstcInternData.pfnNotifySwCallback = pstcConfig->pfnNotifySwCallback;
  stcDstcInternData.pfnErrorCallback    = pstcConfig->pfnErrorCallback;
  
#if (PDL_ON == PDL_DSTC_ENABLE_ADC0_PRIO)
  stcDstcInternData.pfnDstcAdc0PrioCallback = pstcConfig->pfnDstcAdc0PrioCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_ADC0_SCAN)
  stcDstcInternData.pfnDstcAdc0ScanCallback = pstcConfig->pfnDstcAdc0ScanCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_ADC1_PRIO)
  stcDstcInternData.pfnDstcAdc1PrioCallback = pstcConfig->pfnDstcAdc1PrioCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_ADC1_SCAN)
  stcDstcInternData.pfnDstcAdc1ScanCallback = pstcConfig->pfnDstcAdc1ScanCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_ADC2_PRIO)
  stcDstcInternData.pfnDstcAdc2PrioCallback = pstcConfig->pfnDstcAdc2PrioCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_ADC2_SCAN)
  stcDstcInternData.pfnDstcAdc2ScanCallback = pstcConfig->pfnDstcAdc2ScanCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT0_IRQ0)
  stcDstcInternData.pfnDstcBt0Irq0Callback = pstcConfig->pfnDstcBt0Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT0_IRQ1)
  stcDstcInternData.pfnDstcBt0Irq1Callback = pstcConfig->pfnDstcBt0Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT1_IRQ0)
  stcDstcInternData.pfnDstcBt1Irq0Callback = pstcConfig->pfnDstcBt1Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT1_IRQ1)
  stcDstcInternData.pfnDstcBt1Irq1Callback = pstcConfig->pfnDstcBt1Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT2_IRQ0)
  stcDstcInternData.pfnDstcBt2Irq0Callback = pstcConfig->pfnDstcBt2Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT2_IRQ1)
  stcDstcInternData.pfnDstcBt2Irq1Callback = pstcConfig->pfnDstcBt2Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT3_IRQ0)
  stcDstcInternData.pfnDstcBt3Irq0Callback = pstcConfig->pfnDstcBt3Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT3_IRQ1)
  stcDstcInternData.pfnDstcBt3Irq1Callback = pstcConfig->pfnDstcBt3Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT4_IRQ0)
  stcDstcInternData.pfnDstcBt4Irq0Callback = pstcConfig->pfnDstcBt4Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT4_IRQ1)
  stcDstcInternData.pfnDstcBt4Irq1Callback = pstcConfig->pfnDstcBt4Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT5_IRQ0)
  stcDstcInternData.pfnDstcBt5Irq0Callback = pstcConfig->pfnDstcBt5Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT5_IRQ1)
  stcDstcInternData.pfnDstcBt5Irq1Callback = pstcConfig->pfnDstcBt5Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT6_IRQ0)
  stcDstcInternData.pfnDstcBt6Irq0Callback = pstcConfig->pfnDstcBt6Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT6_IRQ1)
  stcDstcInternData.pfnDstcBt6Irq1Callback = pstcConfig->pfnDstcBt6Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT7_IRQ0)
  stcDstcInternData.pfnDstcBt7Irq0Callback = pstcConfig->pfnDstcBt7Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT7_IRQ1)
  stcDstcInternData.pfnDstcBt7Irq1Callback = pstcConfig->pfnDstcBt7Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT8_IRQ0)
  stcDstcInternData.pfnDstcBt8Irq0Callback = pstcConfig->pfnDstcBt8Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT8_IRQ1)
  stcDstcInternData.pfnDstcBt8Irq1Callback = pstcConfig->pfnDstcBt8Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT9_IRQ0)
  stcDstcInternData.pfnDstcBt9Irq0Callback = pstcConfig->pfnDstcBt9Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT9_IRQ1)
  stcDstcInternData.pfnDstcBt9Irq1Callback = pstcConfig->pfnDstcBt9Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT10_IRQ0)
  stcDstcInternData.pfnDstcBt10Irq0Callback = pstcConfig->pfnDstcBt10Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT10_IRQ1)
  stcDstcInternData.pfnDstcBt10Irq1Callback = pstcConfig->pfnDstcBt10Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT11_IRQ0)
  stcDstcInternData.pfnDstcBt11Irq0Callback = pstcConfig->pfnDstcBt11Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT11_IRQ1)
  stcDstcInternData.pfnDstcBt11Irq1Callback = pstcConfig->pfnDstcBt11Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT12_IRQ0)
  stcDstcInternData.pfnDstcBt12Irq0Callback = pstcConfig->pfnDstcBt12Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT12_IRQ1)
  stcDstcInternData.pfnDstcBt12Irq1Callback = pstcConfig->pfnDstcBt12Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT13_IRQ0)
  stcDstcInternData.pfnDstcBt13Irq0Callback = pstcConfig->pfnDstcBt13Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT13_IRQ1)
  stcDstcInternData.pfnDstcBt13Irq1Callback = pstcConfig->pfnDstcBt13Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT14_IRQ0)
  stcDstcInternData.pfnDstcBt14Irq0Callback = pstcConfig->pfnDstcBt14Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT14_IRQ1)
  stcDstcInternData.pfnDstcBt14Irq1Callback = pstcConfig->pfnDstcBt14Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT15_IRQ0)
  stcDstcInternData.pfnDstcBt15Irq0Callback = pstcConfig->pfnDstcBt15Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT15_IRQ1)
  stcDstcInternData.pfnDstcBt15Irq1Callback = pstcConfig->pfnDstcBt15Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXTINT0)
  stcDstcInternData.pfnDstcExtint0Callback = pstcConfig->pfnDstcExtint0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXTINT1)
  stcDstcInternData.pfnDstcExtint1Callback = pstcConfig->pfnDstcExtint1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXTINT2)
  stcDstcInternData.pfnDstcExtint2Callback = pstcConfig->pfnDstcExtint2Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXTINT3)
  stcDstcInternData.pfnDstcExtint3Callback = pstcConfig->pfnDstcExtint3Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXTINT4)
  stcDstcInternData.pfnDstcExtint4Callback = pstcConfig->pfnDstcExtint4Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXTINT5)
  stcDstcInternData.pfnDstcExtint5Callback = pstcConfig->pfnDstcExtint5Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXTINT6)
  stcDstcInternData.pfnDstcExtint6Callback = pstcConfig->pfnDstcExtint6Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXTINT7)
  stcDstcInternData.pfnDstcExtint7Callback = pstcConfig->pfnDstcExtint7Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXTINT8)
  stcDstcInternData.pfnDstcExtint8Callback = pstcConfig->pfnDstcExtint8Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXTINT9)
  stcDstcInternData.pfnDstcExtint9Callback = pstcConfig->pfnDstcExtint9Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXTINT10)
  stcDstcInternData.pfnDstcExtint10Callback = pstcConfig->pfnDstcExtint10Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXTINT11)
  stcDstcInternData.pfnDstcExtint11Callback = pstcConfig->pfnDstcExtint11Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXTINT12)
  stcDstcInternData.pfnDstcExtint12Callback = pstcConfig->pfnDstcExtint12Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXTINT13)
  stcDstcInternData.pfnDstcExtint13Callback = pstcConfig->pfnDstcExtint13Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXTINT14)
  stcDstcInternData.pfnDstcExtint14Callback = pstcConfig->pfnDstcExtint14Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXTINT15)
  stcDstcInternData.pfnDstcExtint15Callback = pstcConfig->pfnDstcExtint15Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXTINT16)
  stcDstcInternData.pfnDstcExtint16Callback = pstcConfig->pfnDstcExtint16Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXTINT17)
  stcDstcInternData.pfnDstcExtint17Callback = pstcConfig->pfnDstcExtint17Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXTINT18)
  stcDstcInternData.pfnDstcExtint18Callback = pstcConfig->pfnDstcExtint18Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXTINT19)
  stcDstcInternData.pfnDstcExtint19Callback = pstcConfig->pfnDstcExtint19Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXTINT20)
  stcDstcInternData.pfnDstcExtint20Callback = pstcConfig->pfnDstcExtint20Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXTINT21)
  stcDstcInternData.pfnDstcExtint21Callback = pstcConfig->pfnDstcExtint21Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXTINT22)
  stcDstcInternData.pfnDstcExtint22Callback = pstcConfig->pfnDstcExtint22Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXTINT23)
  stcDstcInternData.pfnDstcExtint23Callback = pstcConfig->pfnDstcExtint23Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXTINT24)
  stcDstcInternData.pfnDstcExtint24Callback = pstcConfig->pfnDstcExtint24Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXTINT25)
  stcDstcInternData.pfnDstcExtint25Callback = pstcConfig->pfnDstcExtint25Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXTINT26)
  stcDstcInternData.pfnDstcExtint26Callback = pstcConfig->pfnDstcExtint26Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXTINT27)
  stcDstcInternData.pfnDstcExtint27Callback = pstcConfig->pfnDstcExtint27Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXTINT28)
  stcDstcInternData.pfnDstcExtint28Callback = pstcConfig->pfnDstcExtint28Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXTINT29)
  stcDstcInternData.pfnDstcExtint29Callback = pstcConfig->pfnDstcExtint29Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXTINT30)
  stcDstcInternData.pfnDstcExtint30Callback = pstcConfig->pfnDstcExtint30Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXTINT31)
  stcDstcInternData.pfnDstcExtint31Callback = pstcConfig->pfnDstcExtint31Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS0_RX)
  stcDstcInternData.pfnDstcMfs0RxCallback = pstcConfig->pfnDstcMfs0RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS0_TX)
  stcDstcInternData.pfnDstcMfs0TxCallback = pstcConfig->pfnDstcMfs0TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS1_RX)
  stcDstcInternData.pfnDstcMfs1RxCallback = pstcConfig->pfnDstcMfs1RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS1_TX)
  stcDstcInternData.pfnDstcMfs1TxCallback = pstcConfig->pfnDstcMfs1TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS2_RX)
  stcDstcInternData.pfnDstcMfs2RxCallback = pstcConfig->pfnDstcMfs2RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS2_TX)
  stcDstcInternData.pfnDstcMfs2TxCallback = pstcConfig->pfnDstcMfs2TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS3_RX)
  stcDstcInternData.pfnDstcMfs3RxCallback = pstcConfig->pfnDstcMfs3RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS3_TX)
  stcDstcInternData.pfnDstcMfs3TxCallback = pstcConfig->pfnDstcMfs3TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS4_RX)
  stcDstcInternData.pfnDstcMfs4RxCallback = pstcConfig->pfnDstcMfs4RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS4_TX)
  stcDstcInternData.pfnDstcMfs4TxCallback = pstcConfig->pfnDstcMfs4TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS5_RX)
  stcDstcInternData.pfnDstcMfs5RxCallback = pstcConfig->pfnDstcMfs5RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS5_TX)
  stcDstcInternData.pfnDstcMfs5TxCallback = pstcConfig->pfnDstcMfs5TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS6_RX)
  stcDstcInternData.pfnDstcMfs6RxCallback = pstcConfig->pfnDstcMfs6RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS6_TX)
  stcDstcInternData.pfnDstcMfs6TxCallback = pstcConfig->pfnDstcMfs6TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS7_RX)
  stcDstcInternData.pfnDstcMfs7RxCallback = pstcConfig->pfnDstcMfs7RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS7_TX)
  stcDstcInternData.pfnDstcMfs7TxCallback = pstcConfig->pfnDstcMfs7TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS8_RX)
  stcDstcInternData.pfnDstcMfs8RxCallback = pstcConfig->pfnDstcMfs8RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS8_TX)
  stcDstcInternData.pfnDstcMfs8TxCallback = pstcConfig->pfnDstcMfs8TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS9_RX)
  stcDstcInternData.pfnDstcMfs9RxCallback = pstcConfig->pfnDstcMfs9RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS9_TX)
  stcDstcInternData.pfnDstcMfs9TxCallback = pstcConfig->pfnDstcMfs9TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS10_RX)
  stcDstcInternData.pfnDstcMfs10RxCallback = pstcConfig->pfnDstcMfs10RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS10_TX)
  stcDstcInternData.pfnDstcMfs10TxCallback = pstcConfig->pfnDstcMfs10TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS11_RX)
  stcDstcInternData.pfnDstcMfs11RxCallback = pstcConfig->pfnDstcMfs11RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS11_TX)
  stcDstcInternData.pfnDstcMfs11TxCallback = pstcConfig->pfnDstcMfs11TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS12_RX)
  stcDstcInternData.pfnDstcMfs12RxCallback = pstcConfig->pfnDstcMfs12RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS12_TX)
  stcDstcInternData.pfnDstcMfs12TxCallback = pstcConfig->pfnDstcMfs12TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS13_RX)
  stcDstcInternData.pfnDstcMfs13RxCallback = pstcConfig->pfnDstcMfs13RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS13_TX)
  stcDstcInternData.pfnDstcMfs13TxCallback = pstcConfig->pfnDstcMfs13TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS14_RX)
  stcDstcInternData.pfnDstcMfs14RxCallback = pstcConfig->pfnDstcMfs14RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS14_TX)
  stcDstcInternData.pfnDstcMfs14TxCallback = pstcConfig->pfnDstcMfs14TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS15_RX)
  stcDstcInternData.pfnDstcMfs15RxCallback = pstcConfig->pfnDstcMfs15RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS15_TX)
  stcDstcInternData.pfnDstcMfs15TxCallback = pstcConfig->pfnDstcMfs15TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_FRT0_PEAK)
  stcDstcInternData.pfnDstcMft0Frt0PeakCallback = pstcConfig->pfnDstcMft0Frt0PeakCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_FRT0_ZERO)
  stcDstcInternData.pfnDstcMft0Frt0ZeroCallback = pstcConfig->pfnDstcMft0Frt0ZeroCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_FRT1_PEAK)
  stcDstcInternData.pfnDstcMft0Frt1PeakCallback = pstcConfig->pfnDstcMft0Frt1PeakCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_FRT1_ZERO)
  stcDstcInternData.pfnDstcMft0Frt1ZeroCallback = pstcConfig->pfnDstcMft0Frt1ZeroCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_FRT2_PEAK)
  stcDstcInternData.pfnDstcMft0Frt2PeakCallback = pstcConfig->pfnDstcMft0Frt2PeakCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_FRT2_ZERO)
  stcDstcInternData.pfnDstcMft0Frt2ZeroCallback = pstcConfig->pfnDstcMft0Frt2ZeroCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_ICU0)
  stcDstcInternData.pfnDstcMft0Icu0Callback = pstcConfig->pfnDstcMft0Icu0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_ICU1)
  stcDstcInternData.pfnDstcMft0Icu1Callback = pstcConfig->pfnDstcMft0Icu1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_ICU2)
  stcDstcInternData.pfnDstcMft0Icu2Callback = pstcConfig->pfnDstcMft0Icu2Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_ICU3)
  stcDstcInternData.pfnDstcMft0Icu3Callback = pstcConfig->pfnDstcMft0Icu3Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_OCU0)
  stcDstcInternData.pfnDstcMft0Ocu0Callback = pstcConfig->pfnDstcMft0Ocu0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_OCU1)
  stcDstcInternData.pfnDstcMft0Ocu1Callback = pstcConfig->pfnDstcMft0Ocu1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_OCU2)
  stcDstcInternData.pfnDstcMft0Ocu2Callback = pstcConfig->pfnDstcMft0Ocu2Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_OCU3)
  stcDstcInternData.pfnDstcMft0Ocu3Callback = pstcConfig->pfnDstcMft0Ocu3Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_OCU4)
  stcDstcInternData.pfnDstcMft0Ocu4Callback = pstcConfig->pfnDstcMft0Ocu4Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_OCU5)
  stcDstcInternData.pfnDstcMft0Ocu5Callback = pstcConfig->pfnDstcMft0Ocu5Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_WFG10)
  stcDstcInternData.pfnDstcMft0Wfg10Callback = pstcConfig->pfnDstcMft0Wfg10Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_WFG32)
  stcDstcInternData.pfnDstcMft0Wfg32Callback = pstcConfig->pfnDstcMft0Wfg32Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_WFG54)
  stcDstcInternData.pfnDstcMft0Wfg54Callback = pstcConfig->pfnDstcMft0Wfg54Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_FRT0_PEAK)
  stcDstcInternData.pfnDstcMft1Frt0PeakCallback = pstcConfig->pfnDstcMft1Frt0PeakCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_FRT0_ZERO)
  stcDstcInternData.pfnDstcMft1Frt0ZeroCallback = pstcConfig->pfnDstcMft1Frt0ZeroCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_FRT1_PEAK)
  stcDstcInternData.pfnDstcMft1Frt1PeakCallback = pstcConfig->pfnDstcMft1Frt1PeakCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_FRT1_ZERO)
  stcDstcInternData.pfnDstcMft1Frt1ZeroCallback = pstcConfig->pfnDstcMft1Frt1ZeroCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_FRT2_PEAK)
  stcDstcInternData.pfnDstcMft1Frt2PeakCallback = pstcConfig->pfnDstcMft1Frt2PeakCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_FRT2_ZERO)
  stcDstcInternData.pfnDstcMft1Frt2ZeroCallback = pstcConfig->pfnDstcMft1Frt2ZeroCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_ICU0)
  stcDstcInternData.pfnDstcMft1Icu0Callback = pstcConfig->pfnDstcMft1Icu0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_ICU1)
  stcDstcInternData.pfnDstcMft1Icu1Callback = pstcConfig->pfnDstcMft1Icu1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_ICU2)
  stcDstcInternData.pfnDstcMft1Icu2Callback = pstcConfig->pfnDstcMft1Icu2Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_ICU3)
  stcDstcInternData.pfnDstcMft1Icu3Callback = pstcConfig->pfnDstcMft1Icu3Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_OCU0)
  stcDstcInternData.pfnDstcMft1Ocu0Callback = pstcConfig->pfnDstcMft1Ocu0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_OCU1)
  stcDstcInternData.pfnDstcMft1Ocu1Callback = pstcConfig->pfnDstcMft1Ocu1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_OCU2)
  stcDstcInternData.pfnDstcMft1Ocu2Callback = pstcConfig->pfnDstcMft1Ocu2Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_OCU3)
  stcDstcInternData.pfnDstcMft1Ocu3Callback = pstcConfig->pfnDstcMft1Ocu3Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_OCU4)
  stcDstcInternData.pfnDstcMft1Ocu4Callback = pstcConfig->pfnDstcMft1Ocu4Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_OCU5)
  stcDstcInternData.pfnDstcMft1Ocu5Callback = pstcConfig->pfnDstcMft1Ocu5Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_WFG10)
  stcDstcInternData.pfnDstcMft1Wfg10Callback = pstcConfig->pfnDstcMft1Wfg10Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_WFG32)
  stcDstcInternData.pfnDstcMft1Wfg32Callback = pstcConfig->pfnDstcMft1Wfg32Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_WFG54)
  stcDstcInternData.pfnDstcMft1Wfg54Callback = pstcConfig->pfnDstcMft1Wfg54Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_FRT0_PEAK)
  stcDstcInternData.pfnDstcMft2Frt0PeakCallback = pstcConfig->pfnDstcMft2Frt0PeakCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_FRT0_ZERO)
  stcDstcInternData.pfnDstcMft2Frt0ZeroCallback = pstcConfig->pfnDstcMft2Frt0ZeroCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_FRT1_PEAK)
  stcDstcInternData.pfnDstcMft2Frt1PeakCallback = pstcConfig->pfnDstcMft2Frt1PeakCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_FRT1_ZERO)
  stcDstcInternData.pfnDstcMft2Frt1ZeroCallback = pstcConfig->pfnDstcMft2Frt1ZeroCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_FRT2_PEAK)
  stcDstcInternData.pfnDstcMft2Frt2PeakCallback = pstcConfig->pfnDstcMft2Frt2PeakCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_FRT2_ZERO)
  stcDstcInternData.pfnDstcMft2Frt2ZeroCallback = pstcConfig->pfnDstcMft2Frt2ZeroCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_ICU0)
  stcDstcInternData.pfnDstcMft2Icu0Callback = pstcConfig->pfnDstcMft2Icu0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_ICU1)
  stcDstcInternData.pfnDstcMft2Icu1Callback = pstcConfig->pfnDstcMft2Icu1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_ICU2)
  stcDstcInternData.pfnDstcMft2Icu2Callback = pstcConfig->pfnDstcMft2Icu2Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_ICU3)
  stcDstcInternData.pfnDstcMft2Icu3Callback = pstcConfig->pfnDstcMft2Icu3Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_OCU0)
  stcDstcInternData.pfnDstcMft2Ocu0Callback = pstcConfig->pfnDstcMft2Ocu0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_OCU1)
  stcDstcInternData.pfnDstcMft2Ocu1Callback = pstcConfig->pfnDstcMft2Ocu1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_OCU2)
  stcDstcInternData.pfnDstcMft2Ocu2Callback = pstcConfig->pfnDstcMft2Ocu2Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_OCU3)
  stcDstcInternData.pfnDstcMft2Ocu3Callback = pstcConfig->pfnDstcMft2Ocu3Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_OCU4)
  stcDstcInternData.pfnDstcMft2Ocu4Callback = pstcConfig->pfnDstcMft2Ocu4Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_OCU5)
  stcDstcInternData.pfnDstcMft2Ocu5Callback = pstcConfig->pfnDstcMft2Ocu5Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_WFG10)
  stcDstcInternData.pfnDstcMft2Wfg10Callback = pstcConfig->pfnDstcMft2Wfg10Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_WFG32)
  stcDstcInternData.pfnDstcMft2Wfg32Callback = pstcConfig->pfnDstcMft2Wfg32Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_WFG54)
  stcDstcInternData.pfnDstcMft2Wfg54Callback = pstcConfig->pfnDstcMft2Wfg54Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_PPG0)
  stcDstcInternData.pfnDstcPpg0Callback = pstcConfig->pfnDstcPpg0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_PPG2)
  stcDstcInternData.pfnDstcPpg2Callback = pstcConfig->pfnDstcPpg2Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_PPG4)
  stcDstcInternData.pfnDstcPpg4Callback = pstcConfig->pfnDstcPpg4Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_PPG8)
  stcDstcInternData.pfnDstcPpg8Callback = pstcConfig->pfnDstcPpg8Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_PPG10)
  stcDstcInternData.pfnDstcPpg10Callback = pstcConfig->pfnDstcPpg10Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_PPG12)
  stcDstcInternData.pfnDstcPpg12Callback = pstcConfig->pfnDstcPpg12Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_PPG16)
  stcDstcInternData.pfnDstcPpg16Callback = pstcConfig->pfnDstcPpg16Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_PPG18)
  stcDstcInternData.pfnDstcPpg18Callback = pstcConfig->pfnDstcPpg18Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_PPG20)
  stcDstcInternData.pfnDstcPpg20Callback = pstcConfig->pfnDstcPpg20Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC0_COUNT_INVERSION)
  stcDstcInternData.pfnDstcQprc0CountInversionCallback = pstcConfig->pfnDstcQprc0CountInversionCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC0_OUT_OF_RANGE)
  stcDstcInternData.pfnDstcQprc0OutOfRangeCallback = pstcConfig->pfnDstcQprc0OutOfRangeCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC0_PC_MATCH)
  stcDstcInternData.pfnDstcQprc0PcMatchCallback = pstcConfig->pfnDstcQprc0PcMatchCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC0_PC_MATCH_RC_MATCH)
  stcDstcInternData.pfnDstcQprc0PcMatchRcMatchCallback = pstcConfig->pfnDstcQprc0PcMatchRcMatchCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC0_PC_RC_MATCH)
  stcDstcInternData.pfnDstcQprc0PcRcMatchCallback = pstcConfig->pfnDstcQprc0PcRcMatchCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC0_UFL_OFL_Z)
  stcDstcInternData.pfnDstcQprc0UflOflZCallback = pstcConfig->pfnDstcQprc0UflOflZCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC1_COUNT_INVERSION)
  stcDstcInternData.pfnDstcQprc1CountInversionCallback = pstcConfig->pfnDstcQprc1CountInversionCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC1_OUT_OF_RANGE)
  stcDstcInternData.pfnDstcQprc1OutOfRangeCallback = pstcConfig->pfnDstcQprc1OutOfRangeCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC1_PC_MATCH)
  stcDstcInternData.pfnDstcQprc1PcMatchCallback = pstcConfig->pfnDstcQprc1PcMatchCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC1_PC_MATCH_RC_MATCH)
  stcDstcInternData.pfnDstcQprc1PcMatchRcMatchCallback = pstcConfig->pfnDstcQprc1PcMatchRcMatchCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC1_PC_RC_MATCH)
  stcDstcInternData.pfnDstcQprc1PcRcMatchCallback = pstcConfig->pfnDstcQprc1PcRcMatchCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC1_UFL_OFL_Z)
  stcDstcInternData.pfnDstcQprc1UflOflZCallback = pstcConfig->pfnDstcQprc1UflOflZCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC2_COUNT_INVERSION)
  stcDstcInternData.pfnDstcQprc2CountInversionCallback = pstcConfig->pfnDstcQprc2CountInversionCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC2_OUT_OF_RANGE)
  stcDstcInternData.pfnDstcQprc2OutOfRangeCallback = pstcConfig->pfnDstcQprc2OutOfRangeCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC2_PC_MATCH)
  stcDstcInternData.pfnDstcQprc2PcMatchCallback = pstcConfig->pfnDstcQprc2PcMatchCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC2_PC_MATCH_RC_MATCH)
  stcDstcInternData.pfnDstcQprc2PcMatchRcMatchCallback = pstcConfig->pfnDstcQprc2PcMatchRcMatchCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC2_PC_RC_MATCH)
  stcDstcInternData.pfnDstcQprc2PcRcMatchCallback = pstcConfig->pfnDstcQprc2PcRcMatchCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC2_UFL_OFL_Z)
  stcDstcInternData.pfnDstcQprc2UflOflZCallback = pstcConfig->pfnDstcQprc2UflOflZCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC3_COUNT_INVERSION)
  stcDstcInternData.pfnDstcQprc3CountInversionCallback = pstcConfig->pfnDstcQprc3CountInversionCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC3_OUT_OF_RANGE)
  stcDstcInternData.pfnDstcQprc3OutOfRangeCallback = pstcConfig->pfnDstcQprc3OutOfRangeCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC3_PC_MATCH)
  stcDstcInternData.pfnDstcQprc3PcMatchCallback = pstcConfig->pfnDstcQprc3PcMatchCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC3_PC_MATCH_RC_MATCH)
  stcDstcInternData.pfnDstcQprc3PcMatchRcMatchCallback = pstcConfig->pfnDstcQprc3PcMatchRcMatchCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC3_PC_RC_MATCH)
  stcDstcInternData.pfnDstcQprc3PcRcMatchCallback = pstcConfig->pfnDstcQprc3PcRcMatchCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC3_UFL_OFL_Z)
  stcDstcInternData.pfnDstcQprc3UflOflZCallback = pstcConfig->pfnDstcQprc3UflOflZCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_USB0_EP1)
  stcDstcInternData.pfnDstcUsb0Ep1Callback = pstcConfig->pfnDstcUsb0Ep1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_USB0_EP2)
  stcDstcInternData.pfnDstcUsb0Ep2Callback = pstcConfig->pfnDstcUsb0Ep2Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_USB0_EP3)
  stcDstcInternData.pfnDstcUsb0Ep3Callback = pstcConfig->pfnDstcUsb0Ep3Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_USB0_EP4)
  stcDstcInternData.pfnDstcUsb0Ep4Callback = pstcConfig->pfnDstcUsb0Ep4Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_USB0_EP5)
  stcDstcInternData.pfnDstcUsb0Ep5Callback = pstcConfig->pfnDstcUsb0Ep5Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_USB1_EP1)
  stcDstcInternData.pfnDstcUsb1Ep1Callback = pstcConfig->pfnDstcUsb1Ep1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_USB1_EP2)
  stcDstcInternData.pfnDstcUsb1Ep2Callback = pstcConfig->pfnDstcUsb1Ep2Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_USB1_EP3)
  stcDstcInternData.pfnDstcUsb1Ep3Callback = pstcConfig->pfnDstcUsb1Ep3Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_USB1_EP4)
  stcDstcInternData.pfnDstcUsb1Ep4Callback = pstcConfig->pfnDstcUsb1Ep4Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_USB1_EP5)
  stcDstcInternData.pfnDstcUsb1Ep5Callback = pstcConfig->pfnDstcUsb1Ep5Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_WC)
  stcDstcInternData.pfnDstcWcCallback = pstcConfig->pfnDstcWcCallback;
#endif
  
  if ((NULL != pstcConfig->pfnNotifySwCallback) ||
      (NULL != pstcConfig->pfnErrorCallback))
  {
    NVIC_ClearPendingIRQ(DSTC_IRQn);
    NVIC_EnableIRQ(DSTC_IRQn);
    NVIC_SetPriority(DSTC_IRQn, PDL_IRQ_LEVEL_DSTC); 
  }
  
  FM4_DSTC->DESTP = (pstcConfig->u32Destp);
  
  stcCfg.SWINTE = (TRUE == (pstcConfig->bSwInterruptEnable))     ? 1u : 0u;
  stcCfg.ERINTE = (TRUE == (pstcConfig->bErInterruptEnable))     ? 1u : 0u;
  stcCfg.RBDIS  = (TRUE == (pstcConfig->bReadSkipBufferDisable)) ? 1u : 0u;
  stcCfg.ESTE   = (TRUE == (pstcConfig->bErrorStopEnable))       ? 1u : 0u;
  
  switch (pstcConfig->enSwTransferPriority)
  {
    case PriorityHighest:
      stcCfg.SWPR = 0u;
      break;
    case Priority1_2:
      stcCfg.SWPR = 1u;
      break;
    case Priority1_3:
      stcCfg.SWPR = 2u;
      break;
    case Priority1_7:
      stcCfg.SWPR = 3u;
      break;
    case Priority1_15:
      stcCfg.SWPR = 4u;
      break;
    case Priority1_31:
      stcCfg.SWPR = 5u;
      break;
    case Priority1_63:
      stcCfg.SWPR = 6u;
      break;
    case PriorityLowest:
      stcCfg.SWPR = 7u;
      break;
    default:
      return ErrorInvalidParameter;
  }
  
  // Update Hardware
  FM4_DSTC->CFG_f = stcCfg;
  
  return Ok;
} // Dsct_Init
  
/**
 ******************************************************************************
 ** \brief De-Initializes the the DSTC
 **
 ** \retval Ok       DSTC de-initialized and in standby mode
 ******************************************************************************/
en_result_t Dstc_DeInit(void)
{
  NVIC_ClearPendingIRQ(DSTC_IRQn);
  NVIC_DisableIRQ(DSTC_IRQn);
  NVIC_SetPriority(DSTC_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);  
    
  stcDstcInternData.pfnNotifySwCallback = NULL;
  stcDstcInternData.pfnErrorCallback    = NULL;
  
  Dstc_SetCommand(CmdStandyTransition);    // Switch to standy mode
  
  return Ok;
}

/**
 ******************************************************************************
 ** \brief Initializes a DSTC channel
 **
 ** \param [in] u8Channel           DSTC Channel to be initialized
 ** \param [in] u16HwDesp           Offset to Channel Configuration
 **
 ** \retval Ok                      DSTC Initialized
 ** \retval ErrorInvalidParameter   DES Address > 16K Bytes
 ** \retval ErrorAddressAlignment   DES Address not aligned to 32 Bit
 ******************************************************************************/
en_result_t Dstc_SetHwdesp( uint8_t  u8Channel, 
                            uint16_t u16HwDesp )
{
  if (u16HwDesp >= 0x4000u)
  {
    return ErrorInvalidParameter;
  }
  
  if (0u != (u16HwDesp & 0x0003u))
  {
    return ErrorAddressAlignment;
  }

  // 32-Bit access to HWDESP
  FM4_DSTC->HWDESP = (uint32_t)u8Channel | (uint32_t)(u16HwDesp << 16u);

  return Ok;
} // Dsct_ChannelInit

/**
 ******************************************************************************
 ** \brief Read the HWDESP Address Offset of a Channel
 **
 ** \param [in] u8Channel     DSTC Channel to be initialized
 **
 ** \return uint16_t          Current HWDESP DES Offset
 ******************************************************************************/
uint16_t Dstc_ReadHwdesp( uint8_t u8Channel )
{
  FM4_DSTC->HWDESP_f.CHANNEL = u8Channel;
  
  return (uint16_t)(0x0000FFFFul & FM4_DSTC->HWDESP_f.HWDESP);
} // Dstc_ReadHwdesp
                              
/**
 ******************************************************************************
 ** \brief Set a Command to CMD Register
 **
 ** \param [in] enCommand           Command of type of #en_dstc_cmd_t
 **
 ** \retval Ok                      Command Set
 ** \retval ErrorInvalidParameter   Wrong Command Enumerator used
 ******************************************************************************/
en_result_t Dstc_SetCommand( en_dstc_cmd_t enCommand )
{
  switch (enCommand)
  {
    case CmdStandyRelease:
      FM4_DSTC->CMD = 0x04u;
      break;
    case CmdStandyTransition:
      FM4_DSTC->CMD = 0x08u;
      break;
    case CmdSwclr:
      FM4_DSTC->CMD = 0x10u;
      break;
    case CmdErclr:
      FM4_DSTC->CMD = 0x20u;
      break;
    case CmdRbclr:
      FM4_DSTC->CMD = 0x40u;
      break;
    case CmdMkclr:
      FM4_DSTC->CMD = 0x80u;
      break;
    default:
      return ErrorInvalidParameter;
  }
  
  return Ok;
} // Dstc_SetCommand

/**
 ******************************************************************************
 ** \brief Set SWDESP offset and trigger SW transfer
 **
 ** \param [in] u16SwDesPointer     Address offset to DES
 **
 ** \retval Ok                      SWDESP set
 ** \retval ErrorInvalidParameter   Offset > 16K Bytes
 ** \retval ErrorAddressAlignment   Offset not 32-bit aligned
 ******************************************************************************/
en_result_t Dstc_SwTrigger( uint16_t u16SwDesPointer )
{
  if (u16SwDesPointer >= 0x4000u)
  {
    return ErrorInvalidParameter;
  }
  
  if (0u != (u16SwDesPointer & 0x0003u))
  {
    return ErrorAddressAlignment;
  }
  
  FM4_DSTC->SWTR_f.SWDESP = (0x3FFFu & u16SwDesPointer);
  
  return Ok;
}

/**
 ******************************************************************************
 ** \brief Check Software Trigger Start State
 **
 ** \retval Ok                      SW Transfer ended
 ** \retval OperationInProgress     SW Transfer pending
 ******************************************************************************/
en_result_t Dstc_SwTrqansferStartStatus( void )
{
  if (TRUE == FM4_DSTC->SWTR_f.SWREQ)
  {
    return Ok;
  }
    
  return OperationInProgress;
}

/**
 ******************************************************************************
 ** \brief Set DREQENB Register
 **
 ** \param [in] pstcDreqenb    Pointer to DREQENB Structure
 **
 ** \retval Ok                 DREQENB setting successful
 ******************************************************************************/
en_result_t Dstc_SetDreqenb( stc_dstc_dreqenb_t* pstcDreqenb )
{
  FM4_DSTC->DREQENB0 = pstcDreqenb->u32Dreqenb0;
  FM4_DSTC->DREQENB1 = pstcDreqenb->u32Dreqenb1;
  FM4_DSTC->DREQENB2 = pstcDreqenb->u32Dreqenb2;
  FM4_DSTC->DREQENB3 = pstcDreqenb->u32Dreqenb3;
  FM4_DSTC->DREQENB4 = pstcDreqenb->u32Dreqenb4;
  FM4_DSTC->DREQENB5 = pstcDreqenb->u32Dreqenb5;
  FM4_DSTC->DREQENB6 = pstcDreqenb->u32Dreqenb6;
  FM4_DSTC->DREQENB7 = pstcDreqenb->u32Dreqenb7;
    
  return Ok;
}

/**
 ******************************************************************************
 ** \brief Read DREQENB Register
 **
 ** \param [out] pstcDreqenb   Pointer to DREQENB Structure
 **
 ** \retval Ok                 DREQENB read-out successful
 ******************************************************************************/
en_result_t Dstc_ReadDreqenb( stc_dstc_dreqenb_t* pstcDreqenb )
{
  pstcDreqenb->u32Dreqenb0 = FM4_DSTC->DREQENB0;
  pstcDreqenb->u32Dreqenb1 = FM4_DSTC->DREQENB1;
  pstcDreqenb->u32Dreqenb2 = FM4_DSTC->DREQENB2;
  pstcDreqenb->u32Dreqenb3 = FM4_DSTC->DREQENB3;
  pstcDreqenb->u32Dreqenb4 = FM4_DSTC->DREQENB4;
  pstcDreqenb->u32Dreqenb5 = FM4_DSTC->DREQENB5;
  pstcDreqenb->u32Dreqenb6 = FM4_DSTC->DREQENB6;
  pstcDreqenb->u32Dreqenb7 = FM4_DSTC->DREQENB7;
    
  return Ok;
}

/**
 ******************************************************************************
 ** \brief Set DREQENB Register Bit
 **
 ** \param [in] u8BitPos  Bit Position (IRQ Number)
 **
 ** \retval Ok            DREQENB Bit set successfully
 ******************************************************************************/
en_result_t Dstc_SetDreqenbBit( uint8_t u8BitPos )
{
  uint8_t  u8BitPosValue     = (u8BitPos % 32u);
  uint32_t u32BitPosRelative = (1ul << u8BitPosValue);
  uint8_t  u8WordPos         = (u8BitPos / 32u);
  
  *(uint32_t*)(uint32_t)(((uint32_t)(&FM4_DSTC->DREQENB0)) + ((4ul * (uint32_t)u8WordPos))) |= u32BitPosRelative;
  
  return Ok;
}

/**
 ******************************************************************************
 ** \brief Clear DREQENB Register Bit
 **
 ** \param [in] u8BitPos  Bit Position (IRQ Number)
 **
 ** \retval Ok            DREQENB Bit set successfully
 ******************************************************************************/
en_result_t Dstc_ClearDreqenbBit( uint8_t u8BitPos )
{
  uint8_t  u8BitPosValue     = (u8BitPos % 32u);
  uint32_t u32BitPosRelative = (1ul << u8BitPosValue);
  uint8_t  u8WordPos         = (u8BitPos / 32u);
  
  *(uint32_t*)(uint32_t)(((uint32_t)(&FM4_DSTC->DREQENB0)) + ((4ul * (uint32_t)u8WordPos))) |= u32BitPosRelative;
  
  return Ok;
}

/**
 ******************************************************************************
 ** \brief Read HWINT Register
 **
 ** \param [out] pstcHwint     Pointer to HWINT Structure
 **
 ** \retval Ok                 HWINT read-out successful
 ******************************************************************************/
en_result_t Dstc_ReadHwint( stc_dstc_hwint_t* pstcHwint )
{
  pstcHwint->u32Hwint0 = FM4_DSTC->HWINT0;
  pstcHwint->u32Hwint1 = FM4_DSTC->HWINT1;
  pstcHwint->u32Hwint2 = FM4_DSTC->HWINT2;
  pstcHwint->u32Hwint3 = FM4_DSTC->HWINT3;
  pstcHwint->u32Hwint4 = FM4_DSTC->HWINT4;
  pstcHwint->u32Hwint5 = FM4_DSTC->HWINT5;
  pstcHwint->u32Hwint6 = FM4_DSTC->HWINT6;
  pstcHwint->u32Hwint7 = FM4_DSTC->HWINT7;
    
  return Ok;
}

/**
 ******************************************************************************
 ** \brief Set HWINTCLR Register
 **
 ** \param [in] pstcHwintclr   Pointer to HWINTCLR Structure
 **
 ** \retval Ok                 HWINTCLR setting successful
 ******************************************************************************/
en_result_t Dstc_SetHwintclr( stc_dstc_hwintclr_t* pstcHwintclr )
{
  FM4_DSTC->HWINTCLR0 = pstcHwintclr->u32Hwintclr0;
  FM4_DSTC->HWINTCLR1 = pstcHwintclr->u32Hwintclr1;
  FM4_DSTC->HWINTCLR2 = pstcHwintclr->u32Hwintclr2;
  FM4_DSTC->HWINTCLR3 = pstcHwintclr->u32Hwintclr3;
  FM4_DSTC->HWINTCLR4 = pstcHwintclr->u32Hwintclr4;
  FM4_DSTC->HWINTCLR5 = pstcHwintclr->u32Hwintclr5;
  FM4_DSTC->HWINTCLR6 = pstcHwintclr->u32Hwintclr6;
  FM4_DSTC->HWINTCLR7 = pstcHwintclr->u32Hwintclr7;
    
  return Ok;
}

/**
 ******************************************************************************
 ** \brief Read HWINT Register Bit
 **
 ** \param [in] u8BitPos  Bit Position (IRQ Number)
 **
 ** \retval TRUE        HWINT bit set
 ** \retval FALSE       HWINT bit not set
 ******************************************************************************/
boolean_t Dstc_ReadHwintBit( uint8_t u8BitPos )
{
  uint8_t  u8BitPosValue     = (u8BitPos % 32u);
  uint32_t u32BitPosRelative = (1ul << u8BitPosValue);
  uint8_t  u8WordPos         = (u8BitPos / 32u);
  
  if(((*(uint32_t*)(uint32_t)(((uint32_t)(&FM4_DSTC->HWINT0)) + ((4ul * (uint32_t)u8WordPos)))) & u32BitPosRelative) == u32BitPosRelative)
  {
    return TRUE;
  }

  return FALSE;
}

/**
 ******************************************************************************
 ** \brief Set HWINTCLR Register Bit
 **
 ** \param [in] u8BitPos  Bit Position (IRQ Number)
 **
 ** \retval Ok            HWINTCLR Bit set successfully
 ******************************************************************************/
en_result_t Dstc_SetHwintclrBit( uint8_t u8BitPos )
{
  uint8_t  u8BitPosValue     = (u8BitPos % 32u);
  uint32_t u32BitPosRelative = (1ul << u8BitPosValue);
  uint8_t  u8WordPos         = (u8BitPos / 32u);
  
  *(uint32_t*)(uint32_t)(((uint32_t)(&FM4_DSTC->HWINTCLR0)) + ((4ul * (uint32_t)u8WordPos))) = u32BitPosRelative;

  return Ok;
}

/**
 ******************************************************************************
 ** \brief Read DQMSK Register
 **
 ** \param [out] pstcDqmsk     Pointer to DQMSK Structure
 **
 ** \retval Ok                 HWINT read-out successful
 ******************************************************************************/
en_result_t Dstc_ReadDqmsk( stc_dstc_dqmsk_t* pstcDqmsk )
{
  pstcDqmsk->u32Dqmsk0 = FM4_DSTC->DQMSK0;
  pstcDqmsk->u32Dqmsk1 = FM4_DSTC->DQMSK1;
  pstcDqmsk->u32Dqmsk2 = FM4_DSTC->DQMSK2;
  pstcDqmsk->u32Dqmsk3 = FM4_DSTC->DQMSK3;
  pstcDqmsk->u32Dqmsk4 = FM4_DSTC->DQMSK4;
  pstcDqmsk->u32Dqmsk5 = FM4_DSTC->DQMSK5;
  pstcDqmsk->u32Dqmsk6 = FM4_DSTC->DQMSK6;
  pstcDqmsk->u32Dqmsk7 = FM4_DSTC->DQMSK7;
    
  return Ok;
}

/**
 ******************************************************************************
 ** \brief Set DQMSKCLR Register
 **
 ** \param [in] pstcDqmskclr   Pointer to DQMSKCLR Structure
 **
 ** \retval Ok                 DQMSKCLR setting successful
 ******************************************************************************/
en_result_t Dstc_SetDqmskclr( stc_dstc_dqmskclr_t* pstcDqmskclr )
{
  FM4_DSTC->DQMSKCLR0 = pstcDqmskclr->u32Dqmskclr0;
  FM4_DSTC->DQMSKCLR1 = pstcDqmskclr->u32Dqmskclr1;
  FM4_DSTC->DQMSKCLR2 = pstcDqmskclr->u32Dqmskclr2;
  FM4_DSTC->DQMSKCLR3 = pstcDqmskclr->u32Dqmskclr3;
  FM4_DSTC->DQMSKCLR4 = pstcDqmskclr->u32Dqmskclr4;
  FM4_DSTC->DQMSKCLR5 = pstcDqmskclr->u32Dqmskclr5;
  FM4_DSTC->DQMSKCLR6 = pstcDqmskclr->u32Dqmskclr6;
  FM4_DSTC->DQMSKCLR7 = pstcDqmskclr->u32Dqmskclr7;
    
  return Ok;
}

/**
 ******************************************************************************
 ** \brief Set DQMSKCLR Register Bit
 **
 ** \param [in] u8BitPos  Bit Position (IRQ Number)
 **
 ** \retval Ok            DQMSKCLR Bit set successfully
 ******************************************************************************/
en_result_t Dstc_SetDqmskclrBit( uint8_t u8BitPos )
{
  uint8_t  u8BitPosValue     = (u8BitPos % 32u);
  uint32_t u32BitPosRelative = (1ul << u8BitPosValue);
  uint8_t  u8WordPos         = (u8BitPos / 32u);
  
  *(uint32_t*)(uint32_t)(((uint32_t)(&FM4_DSTC->DQMSKCLR0)) + ((4ul * (uint32_t)u8WordPos))) &= (0xFFFFFFFFul ^ u32BitPosRelative);
  
  return Ok;
}

/**
 ******************************************************************************
 ** \brief DSTC Peripheral Interrupt Service Routines
 ******************************************************************************/
/// ADC
#if (PDL_ON == PDL_DSTC_ENABLE_ADC0_PRIO) || (PDL_ON == PDL_DSTC_ENABLE_ADC0_SCAN) || \
    (PDL_ON == PDL_DSTC_ENABLE_ADC1_PRIO) || (PDL_ON == PDL_DSTC_ENABLE_ADC1_SCAN) || \
    (PDL_ON == PDL_DSTC_ENABLE_ADC2_PRIO) || (PDL_ON == PDL_DSTC_ENABLE_ADC2_SCAN)
void Dstc_AdcIrqHandler(uint8_t u8IrqChannel0, uint8_t u8IrqChannel1)
{
  func_ptr_t pfnCallback;
  
  #if (PDL_ON == PDL_DSTC_ENABLE_ADC0_PRIO) || \
      (PDL_ON == PDL_DSTC_ENABLE_ADC1_PRIO) || \
      (PDL_ON == PDL_DSTC_ENABLE_ADC2_PRIO)
  switch (u8IrqChannel0)
  {
    #if (PDL_ON == PDL_DSTC_ENABLE_ADC0_PRIO)  
    case DSTC_IRQ_NUMBER_ADC0_PRIO:
      pfnCallback = stcDstcInternData.pfnDstcAdc0PrioCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_ADC1_PRIO)  
    case DSTC_IRQ_NUMBER_ADC1_PRIO:
      pfnCallback = stcDstcInternData.pfnDstcAdc1PrioCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_ADC2_PRIO)  
    case DSTC_IRQ_NUMBER_ADC2_PRIO:
      pfnCallback = stcDstcInternData.pfnDstcAdc2PrioCallback;
      break;
    #endif
    default:
      pfnCallback = NULL;
  }
  
  if (TRUE == Dstc_ReadHwintBit(u8IrqChannel0))
  {
    Dstc_SetHwintclrBit(u8IrqChannel0);
    
    if (pfnCallback != NULL)
    {
      pfnCallback();
    }
  }
  #endif
  
  #if (PDL_ON == PDL_DSTC_ENABLE_ADC0_SCAN) || \
      (PDL_ON == PDL_DSTC_ENABLE_ADC1_SCAN) || \
      (PDL_ON == PDL_DSTC_ENABLE_ADC2_SCAN)
  switch (u8IrqChannel1)
  {
    #if (PDL_ON == PDL_DSTC_ENABLE_ADC0_SCAN)  
    case DSTC_IRQ_NUMBER_ADC0_SCAN:
      pfnCallback = stcDstcInternData.pfnDstcAdc0ScanCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_ADC1_SCAN)  
    case DSTC_IRQ_NUMBER_ADC1_SCAN:
      pfnCallback = stcDstcInternData.pfnDstcAdc1ScanCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_ADC2_SCAN)  
    case DSTC_IRQ_NUMBER_ADC2_SCAN:
      pfnCallback = stcDstcInternData.pfnDstcAdc2ScanCallback;
      break;
    #endif
    default:
      pfnCallback = NULL;
  }
  
  if (TRUE == Dstc_ReadHwintBit(u8IrqChannel1))
  {
    Dstc_SetHwintclrBit(u8IrqChannel1);
    
    if (pfnCallback != NULL)
    {
      pfnCallback();
    }
  }
  #endif
}
#endif
   
/// BT
#if (PDL_ON == PDL_DSTC_ENABLE_BT0_IRQ0)  || (PDL_ON == PDL_DSTC_ENABLE_BT0_IRQ1)  || \
    (PDL_ON == PDL_DSTC_ENABLE_BT1_IRQ0)  || (PDL_ON == PDL_DSTC_ENABLE_BT1_IRQ1)  || \
    (PDL_ON == PDL_DSTC_ENABLE_BT2_IRQ0)  || (PDL_ON == PDL_DSTC_ENABLE_BT2_IRQ1)  || \
    (PDL_ON == PDL_DSTC_ENABLE_BT3_IRQ0)  || (PDL_ON == PDL_DSTC_ENABLE_BT3_IRQ1)  || \
    (PDL_ON == PDL_DSTC_ENABLE_BT4_IRQ0)  || (PDL_ON == PDL_DSTC_ENABLE_BT4_IRQ1)  || \
    (PDL_ON == PDL_DSTC_ENABLE_BT5_IRQ0)  || (PDL_ON == PDL_DSTC_ENABLE_BT5_IRQ1)  || \
    (PDL_ON == PDL_DSTC_ENABLE_BT6_IRQ0)  || (PDL_ON == PDL_DSTC_ENABLE_BT6_IRQ1)  || \
    (PDL_ON == PDL_DSTC_ENABLE_BT7_IRQ0)  || (PDL_ON == PDL_DSTC_ENABLE_BT7_IRQ1)  || \
    (PDL_ON == PDL_DSTC_ENABLE_BT8_IRQ0)  || (PDL_ON == PDL_DSTC_ENABLE_BT8_IRQ1)  || \
    (PDL_ON == PDL_DSTC_ENABLE_BT9_IRQ0)  || (PDL_ON == PDL_DSTC_ENABLE_BT9_IRQ1)  || \
    (PDL_ON == PDL_DSTC_ENABLE_BT10_IRQ0) || (PDL_ON == PDL_DSTC_ENABLE_BT10_IRQ1) || \
    (PDL_ON == PDL_DSTC_ENABLE_BT11_IRQ0) || (PDL_ON == PDL_DSTC_ENABLE_BT11_IRQ1) || \
    (PDL_ON == PDL_DSTC_ENABLE_BT12_IRQ0) || (PDL_ON == PDL_DSTC_ENABLE_BT12_IRQ1) || \
    (PDL_ON == PDL_DSTC_ENABLE_BT13_IRQ0) || (PDL_ON == PDL_DSTC_ENABLE_BT13_IRQ1) || \
    (PDL_ON == PDL_DSTC_ENABLE_BT14_IRQ0) || (PDL_ON == PDL_DSTC_ENABLE_BT14_IRQ1)
void Dstc_BtIrqHandler(uint8_t u8IrqChannel0, uint8_t u8IrqChannel1)
{
  func_ptr_t pfnCallback;
  
  #if (PDL_ON == PDL_DSTC_ENABLE_BT0_IRQ0)  || \
      (PDL_ON == PDL_DSTC_ENABLE_BT1_IRQ0)  || \
      (PDL_ON == PDL_DSTC_ENABLE_BT2_IRQ0)  || \
      (PDL_ON == PDL_DSTC_ENABLE_BT3_IRQ0)  || \
      (PDL_ON == PDL_DSTC_ENABLE_BT4_IRQ0)  || \
      (PDL_ON == PDL_DSTC_ENABLE_BT5_IRQ0)  || \
      (PDL_ON == PDL_DSTC_ENABLE_BT6_IRQ0)  || \
      (PDL_ON == PDL_DSTC_ENABLE_BT7_IRQ0)  || \
      (PDL_ON == PDL_DSTC_ENABLE_BT8_IRQ0)  || \
      (PDL_ON == PDL_DSTC_ENABLE_BT9_IRQ0)  || \
      (PDL_ON == PDL_DSTC_ENABLE_BT10_IRQ0) || \
      (PDL_ON == PDL_DSTC_ENABLE_BT11_IRQ0) || \
      (PDL_ON == PDL_DSTC_ENABLE_BT12_IRQ0) || \
      (PDL_ON == PDL_DSTC_ENABLE_BT13_IRQ0) || \
      (PDL_ON == PDL_DSTC_ENABLE_BT14_IRQ0)
  switch (u8IrqChannel0)
  {
    #if (PDL_ON == PDL_DSTC_ENABLE_BT0_IRQ0)  
    case DSTC_IRQ_NUMBER_BT0_IRQ0:
      pfnCallback = stcDstcInternData.pfnDstcBt0Irq0Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_BT1_IRQ0) 
    case DSTC_IRQ_NUMBER_BT1_IRQ0:
      pfnCallback = stcDstcInternData.pfnDstcBt1Irq0Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_BT2_IRQ0) 
    case DSTC_IRQ_NUMBER_BT2_IRQ0:
      pfnCallback = stcDstcInternData.pfnDstcBt2Irq0Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_BT3_IRQ0) 
    case DSTC_IRQ_NUMBER_BT3_IRQ0:
      pfnCallback = stcDstcInternData.pfnDstcBt3Irq0Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_BT4_IRQ0) 
    case DSTC_IRQ_NUMBER_BT4_IRQ0:
      pfnCallback = stcDstcInternData.pfnDstcBt4Irq0Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_BT5_IRQ0) 
    case DSTC_IRQ_NUMBER_BT5_IRQ0:
      pfnCallback = stcDstcInternData.pfnDstcBt5Irq0Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_BT6_IRQ0)  
    case DSTC_IRQ_NUMBER_BT6_IRQ0:
      pfnCallback = stcDstcInternData.pfnDstcBt6Irq0Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_BT7_IRQ0) 
    case DSTC_IRQ_NUMBER_BT7_IRQ0:
      pfnCallback = stcDstcInternData.pfnDstcBt7Irq0Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_BT8_IRQ0) 
    case DSTC_IRQ_NUMBER_BT8_IRQ0:
      pfnCallback = stcDstcInternData.pfnDstcBt8Irq0Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_BT9_IRQ0) 
    case DSTC_IRQ_NUMBER_BT9_IRQ0:
      pfnCallback = stcDstcInternData.pfnDstcBt9Irq0Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_BT10_IRQ0) 
    case DSTC_IRQ_NUMBER_BT10_IRQ0:
      pfnCallback = stcDstcInternData.pfnDstcBt10Irq0Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_BT11_IRQ0) 
    case DSTC_IRQ_NUMBER_BT11_IRQ0:
      pfnCallback = stcDstcInternData.pfnDstcBt11Irq0Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_BT12_IRQ0) 
    case DSTC_IRQ_NUMBER_BT12_IRQ0:
      pfnCallback = stcDstcInternData.pfnDstcBt12Irq0Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_BT13_IRQ0) 
    case DSTC_IRQ_NUMBER_BT13_IRQ0:
      pfnCallback = stcDstcInternData.pfnDstcBt13Irq0Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_BT14_IRQ0) 
    case DSTC_IRQ_NUMBER_BT14_IRQ0:
      pfnCallback = stcDstcInternData.pfnDstcBt14Irq0Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_BT15_IRQ0) 
    case DSTC_IRQ_NUMBER_BT15_IRQ0:
      pfnCallback = stcDstcInternData.pfnDstcBt15Irq0Callback;
      break;
    #endif
    default:
      pfnCallback = NULL;
  }
  
  if (TRUE == Dstc_ReadHwintBit(u8IrqChannel0))
  {
    Dstc_SetHwintclrBit(u8IrqChannel0);
    
    if (pfnCallback != NULL)
    {
      pfnCallback();
    }
  }
  #endif
  
  #if (PDL_ON == PDL_DSTC_ENABLE_BT0_IRQ1)  || \
      (PDL_ON == PDL_DSTC_ENABLE_BT1_IRQ1)  || \
      (PDL_ON == PDL_DSTC_ENABLE_BT2_IRQ1)  || \
      (PDL_ON == PDL_DSTC_ENABLE_BT3_IRQ1)  || \
      (PDL_ON == PDL_DSTC_ENABLE_BT4_IRQ1)  || \
      (PDL_ON == PDL_DSTC_ENABLE_BT5_IRQ1)  || \
      (PDL_ON == PDL_DSTC_ENABLE_BT6_IRQ1)  || \
      (PDL_ON == PDL_DSTC_ENABLE_BT7_IRQ1)  || \
      (PDL_ON == PDL_DSTC_ENABLE_BT8_IRQ1)  || \
      (PDL_ON == PDL_DSTC_ENABLE_BT9_IRQ1)  || \
      (PDL_ON == PDL_DSTC_ENABLE_BT10_IRQ1) || \
      (PDL_ON == PDL_DSTC_ENABLE_BT11_IRQ1) || \
      (PDL_ON == PDL_DSTC_ENABLE_BT12_IRQ1) || \
      (PDL_ON == PDL_DSTC_ENABLE_BT13_IRQ1) || \
      (PDL_ON == PDL_DSTC_ENABLE_BT14_IRQ1)
  switch (u8IrqChannel1)
  {
    #if (PDL_ON == PDL_DSTC_ENABLE_BT0_IRQ1)  
    case DSTC_IRQ_NUMBER_BT0_IRQ1:
      pfnCallback = stcDstcInternData.pfnDstcBt0Irq1Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_BT1_IRQ1) 
    case DSTC_IRQ_NUMBER_BT1_IRQ1:
      pfnCallback = stcDstcInternData.pfnDstcBt1Irq1Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_BT2_IRQ1) 
    case DSTC_IRQ_NUMBER_BT2_IRQ1:
      pfnCallback = stcDstcInternData.pfnDstcBt2Irq1Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_BT3_IRQ1) 
    case DSTC_IRQ_NUMBER_BT3_IRQ1:
      pfnCallback = stcDstcInternData.pfnDstcBt3Irq1Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_BT4_IRQ1) 
    case DSTC_IRQ_NUMBER_BT4_IRQ1:
      pfnCallback = stcDstcInternData.pfnDstcBt4Irq1Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_BT5_IRQ1) 
    case DSTC_IRQ_NUMBER_BT5_IRQ1:
      pfnCallback = stcDstcInternData.pfnDstcBt5Irq1Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_BT6_IRQ1)  
    case DSTC_IRQ_NUMBER_BT6_IRQ1:
      pfnCallback = stcDstcInternData.pfnDstcBt6Irq1Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_BT7_IRQ1) 
    case DSTC_IRQ_NUMBER_BT7_IRQ1:
      pfnCallback = stcDstcInternData.pfnDstcBt7Irq1Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_BT8_IRQ1) 
    case DSTC_IRQ_NUMBER_BT8_IRQ1:
      pfnCallback = stcDstcInternData.pfnDstcBt8Irq1Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_BT9_IRQ1) 
    case DSTC_IRQ_NUMBER_BT9_IRQ1:
      pfnCallback = stcDstcInternData.pfnDstcBt9Irq1Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_BT10_IRQ1) 
    case DSTC_IRQ_NUMBER_BT10_IRQ1:
      pfnCallback = stcDstcInternData.pfnDstcBt11Irq0Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_BT11_IRQ1) 
    case DSTC_IRQ_NUMBER_BT11_IRQ1:
      pfnCallback = stcDstcInternData.pfnDstcBt11Irq1Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_BT12_IRQ1) 
    case DSTC_IRQ_NUMBER_BT12_IRQ1:
      pfnCallback = stcDstcInternData.pfnDstcBt12Irq1Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_BT13_IRQ1) 
    case DSTC_IRQ_NUMBER_BT13_IRQ1:
      pfnCallback = stcDstcInternData.pfnDstcBt13Irq1Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_BT14_IRQ1) 
    case DSTC_IRQ_NUMBER_BT14_IRQ1:
      pfnCallback = stcDstcInternData.pfnDstcBt14Irq1Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_BT15_IRQ1) 
    case DSTC_IRQ_NUMBER_BT15_IRQ1:
      pfnCallback = stcDstcInternData.pfnDstcBt15Irq1Callback;
      break;
    #endif
    default:
      pfnCallback = NULL;
  }
  
  if (TRUE == Dstc_ReadHwintBit(u8IrqChannel0))
  {
    Dstc_SetHwintclrBit(u8IrqChannel0);
    
    if (pfnCallback != NULL)
    {
      pfnCallback();
    }
  }
  #endif
}
#endif // #if (PDL_ON == PDL_DSTC_ENABLE_BTn_IRQ0) || (PDL_ON == PDL_DSTC_ENABLE_BTn_IRQ1) ...

/// EXT INT
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT0)  || (PDL_ON == PDL_DSTC_ENABLE_EXINT1)  || \
    (PDL_ON == PDL_DSTC_ENABLE_EXINT2)  || (PDL_ON == PDL_DSTC_ENABLE_EXINT3)  || \
    (PDL_ON == PDL_DSTC_ENABLE_EXINT4)  || (PDL_ON == PDL_DSTC_ENABLE_EXINT5)  || \
    (PDL_ON == PDL_DSTC_ENABLE_EXINT6)  || (PDL_ON == PDL_DSTC_ENABLE_EXINT7)  || \
    (PDL_ON == PDL_DSTC_ENABLE_EXINT8)  || (PDL_ON == PDL_DSTC_ENABLE_EXINT9)  || \
    (PDL_ON == PDL_DSTC_ENABLE_EXINT10) || (PDL_ON == PDL_DSTC_ENABLE_EXINT11) || \
    (PDL_ON == PDL_DSTC_ENABLE_EXINT12) || (PDL_ON == PDL_DSTC_ENABLE_EXINT13) || \
    (PDL_ON == PDL_DSTC_ENABLE_EXINT14) || (PDL_ON == PDL_DSTC_ENABLE_EXINT15) || \
    (PDL_ON == PDL_DSTC_ENABLE_EXINT16) || (PDL_ON == PDL_DSTC_ENABLE_EXINT17) || \
    (PDL_ON == PDL_DSTC_ENABLE_EXINT18) || (PDL_ON == PDL_DSTC_ENABLE_EXINT19) || \
    (PDL_ON == PDL_DSTC_ENABLE_EXINT20) || (PDL_ON == PDL_DSTC_ENABLE_EXINT21) || \
    (PDL_ON == PDL_DSTC_ENABLE_EXINT22) || (PDL_ON == PDL_DSTC_ENABLE_EXINT23) || \
    (PDL_ON == PDL_DSTC_ENABLE_EXINT24) || (PDL_ON == PDL_DSTC_ENABLE_EXINT25) || \
    (PDL_ON == PDL_DSTC_ENABLE_EXINT26) || (PDL_ON == PDL_DSTC_ENABLE_EXINT27) || \
    (PDL_ON == PDL_DSTC_ENABLE_EXINT28) || (PDL_ON == PDL_DSTC_ENABLE_EXINT29) || \
    (PDL_ON == PDL_DSTC_ENABLE_EXINT30) || (PDL_ON == PDL_DSTC_ENABLE_EXINT31)      
void Dstc_ExintIrqHandler(uint8_t u8IrqChannel)
{
  func_ptr_t pfnCallback;
  
  switch (u8IrqChannel)
  {
    #if (PDL_ON == PDL_DSTC_ENABLE_EXINT0)  
    case DSTC_IRQ_NUMBER_EXTINT0:
      pfnCallback = stcDstcInternData.pfnDstcExint0Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_EXINT1)  
    case DSTC_IRQ_NUMBER_EXTINT1:
      pfnCallback = stcDstcInternData.pfnDstcExint1Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_EXINT2)  
    case DSTC_IRQ_NUMBER_EXTINT2:
      pfnCallback = stcDstcInternData.pfnDstcExint2Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_EXINT3)  
    case DSTC_IRQ_NUMBER_EXTINT3:
      pfnCallback = stcDstcInternData.pfnDstcExint3Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_EXINT4)  
    case DSTC_IRQ_NUMBER_EXTINT4:
      pfnCallback = stcDstcInternData.pfnDstcExint4Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_EXINT5)  
    case DSTC_IRQ_NUMBER_EXTINT5:
      pfnCallback = stcDstcInternData.pfnDstcExint5Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_EXINT6)  
    case DSTC_IRQ_NUMBER_EXTINT6:
      pfnCallback = stcDstcInternData.pfnDstcExint6Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_EXINT7)  
    case DSTC_IRQ_NUMBER_EXTINT7:
      pfnCallback = stcDstcInternData.pfnDstcExint7Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_EXINT8)  
    case DSTC_IRQ_NUMBER_EXTINT8:
      pfnCallback = stcDstcInternData.pfnDstcExint8Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_EXINT9)  
    case DSTC_IRQ_NUMBER_EXTINT9:
      pfnCallback = stcDstcInternData.pfnDstcExint9Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_EXINT10)  
    case DSTC_IRQ_NUMBER_EXTINT10:
      pfnCallback = stcDstcInternData.pfnDstcExint10Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_EXINT11)  
    case DSTC_IRQ_NUMBER_EXTINT11:
      pfnCallback = stcDstcInternData.pfnDstcExint11Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_EXINT12)  
    case DSTC_IRQ_NUMBER_EXTINT12:
      pfnCallback = stcDstcInternData.pfnDstcExint12Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_EXINT13)  
    case DSTC_IRQ_NUMBER_EXTINT13:
      pfnCallback = stcDstcInternData.pfnDstcExint13Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_EXINT14)  
    case DSTC_IRQ_NUMBER_EXTINT14:
      pfnCallback = stcDstcInternData.pfnDstcExint14Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_EXINT15)  
    case DSTC_IRQ_NUMBER_EXTINT15:
      pfnCallback = stcDstcInternData.pfnDstcExint15Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_EXINT16)  
    case DSTC_IRQ_NUMBER_EXTINT16:
      pfnCallback = stcDstcInternData.pfnDstcExint16Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_EXINT17)  
    case DSTC_IRQ_NUMBER_EXTINT17:
      pfnCallback = stcDstcInternData.pfnDstcExint17Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_EXINT18)  
    case DSTC_IRQ_NUMBER_EXTINT18:
      pfnCallback = stcDstcInternData.pfnDstcExint18Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_EXINT19)  
    case DSTC_IRQ_NUMBER_EXTINT19:
      pfnCallback = stcDstcInternData.pfnDstcExint19Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_EXINT20)  
    case DSTC_IRQ_NUMBER_EXTINT20:
      pfnCallback = stcDstcInternData.pfnDstcExint20Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_EXINT21)  
    case DSTC_IRQ_NUMBER_EXTINT21:
      pfnCallback = stcDstcInternData.pfnDstcExint21Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_EXINT22)  
    case DSTC_IRQ_NUMBER_EXTINT22:
      pfnCallback = stcDstcInternData.pfnDstcExint22Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_EXINT23)  
    case DSTC_IRQ_NUMBER_EXTINT23:
      pfnCallback = stcDstcInternData.pfnDstcExint23Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_EXINT24)  
    case DSTC_IRQ_NUMBER_EXTINT24:
      pfnCallback = stcDstcInternData.pfnDstcExint24Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_EXINT25)  
    case DSTC_IRQ_NUMBER_EXTINT25:
      pfnCallback = stcDstcInternData.pfnDstcExint25Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_EXINT26)  
    case DSTC_IRQ_NUMBER_EXTINT26:
      pfnCallback = stcDstcInternData.pfnDstcExint26Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_EXINT27)  
    case DSTC_IRQ_NUMBER_EXTINT27:
      pfnCallback = stcDstcInternData.pfnDstcExint27Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_EXINT28)  
    case DSTC_IRQ_NUMBER_EXTINT28:
      pfnCallback = stcDstcInternData.pfnDstcExint28Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_EXINT29)  
    case DSTC_IRQ_NUMBER_EXTINT29:
      pfnCallback = stcDstcInternData.pfnDstcExint29Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_EXINT30)  
    case DSTC_IRQ_NUMBER_EXTINT30:
      pfnCallback = stcDstcInternData.pfnDstcExint30Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_EXINT31)  
    case DSTC_IRQ_NUMBER_EXTINT31:
      pfnCallback = stcDstcInternData.pfnDstcExint31Callback;
      break;
    #endif
    default:
      pfnCallback = NULL;
  }
  
  if (TRUE == Dstc_ReadHwintBit(u8IrqChannel))
  {
    Dstc_SetHwintclrBit(u8IrqChannel);
    
    if (pfnCallback != NULL)
    {
      pfnCallback();
    }
  }
}
#endif // #if (PDL_ON == PDL_DSTC_ENABLE_EXINTn)  || ...

/// MFS
#if (PDL_ON == PDL_DSTC_ENABLE_MFS0_RX)  || (PDL_ON == PDL_DSTC_ENABLE_MFS1_RX)  || \
    (PDL_ON == PDL_DSTC_ENABLE_MFS2_RX)  || (PDL_ON == PDL_DSTC_ENABLE_MFS3_RX)  || \
    (PDL_ON == PDL_DSTC_ENABLE_MFS4_RX)  || (PDL_ON == PDL_DSTC_ENABLE_MFS5_RX)  || \
    (PDL_ON == PDL_DSTC_ENABLE_MFS6_RX)  || (PDL_ON == PDL_DSTC_ENABLE_MFS7_RX)  || \
    (PDL_ON == PDL_DSTC_ENABLE_MFS8_RX)  || (PDL_ON == PDL_DSTC_ENABLE_MFS9_RX)  || \
    (PDL_ON == PDL_DSTC_ENABLE_MFS10_RX) || (PDL_ON == PDL_DSTC_ENABLE_MFS11_RX) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFS12_RX) || (PDL_ON == PDL_DSTC_ENABLE_MFS13_RX) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFS14_RX) || (PDL_ON == PDL_DSTC_ENABLE_MFS15_RX)
extern void Dstc_MfsRxIrqHandler(uint8_t u8IrqChannel)
{
  func_ptr_t pfnCallback;
  
  switch (u8IrqChannel)
  {
    #if (PDL_ON == PDL_DSTC_ENABLE_MFS0_RX)  
    case DSTC_IRQ_NUMBER_MFS0_RX:
      pfnCallback = stcDstcInternData.pfnDstcMfs0RxCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFS1_RX)  
    case DSTC_IRQ_NUMBER_MFS1_RX:
      pfnCallback = stcDstcInternData.pfnDstcMfs1RxCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFS2_RX)  
    case DSTC_IRQ_NUMBER_MFS2_RX:
      pfnCallback = stcDstcInternData.pfnDstcMfs2RxCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFS3_RX)  
    case DSTC_IRQ_NUMBER_MFS3_RX:
      pfnCallback = stcDstcInternData.pfnDstcMfs3RxCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFS4_RX)  
    case DSTC_IRQ_NUMBER_MFS4_RX:
      pfnCallback = stcDstcInternData.pfnDstcMfs4RxCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFS5_RX)  
    case DSTC_IRQ_NUMBER_MFS5_RX:
      pfnCallback = stcDstcInternData.pfnDstcMfs5RxCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFS6_RX)  
    case DSTC_IRQ_NUMBER_MFS6_RX:
      pfnCallback = stcDstcInternData.pfnDstcMfs6RxCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFS7_RX)  
    case DSTC_IRQ_NUMBER_MFS7_RX:
      pfnCallback = stcDstcInternData.pfnDstcMfs7RxCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFS8_RX)  
    case DSTC_IRQ_NUMBER_MFS8_RX:
      pfnCallback = stcDstcInternData.pfnDstcMfs8RxCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFS9_RX)  
    case DSTC_IRQ_NUMBER_MFS9_RX:
      pfnCallback = stcDstcInternData.pfnDstcMfs9RxCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFS10_RX)  
    case DSTC_IRQ_NUMBER_MFS10_RX:
      pfnCallback = stcDstcInternData.pfnDstcMfs10RxCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFS11_RX)  
    case DSTC_IRQ_NUMBER_MFS11_RX:
      pfnCallback = stcDstcInternData.pfnDstcMfs11RxCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFS12_RX)  
    case DSTC_IRQ_NUMBER_MFS12_RX:
      pfnCallback = stcDstcInternData.pfnDstcMfs12RxCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFS13_RX)  
    case DSTC_IRQ_NUMBER_MFS13_RX:
      pfnCallback = stcDstcInternData.pfnDstcMfs13RxCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFS14_RX)  
    case DSTC_IRQ_NUMBER_MFS14_RX:
      pfnCallback = stcDstcInternData.pfnDstcMfs14RxCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFS15_RX)  
    case DSTC_IRQ_NUMBER_MFS15_RX:
      pfnCallback = stcDstcInternData.pfnDstcMfs15RxCallback;
      break;
    #endif
    default:
      pfnCallback = NULL;
  }
  
  if (TRUE == Dstc_ReadHwintBit(u8IrqChannel))
  {
    Dstc_SetHwintclrBit(u8IrqChannel);
    
    if (pfnCallback != NULL)
    {
      pfnCallback();
    }
  }
}
#endif // #if (PDL_ON == PDL_DSTC_ENABLE_MFS0_RX)  || (PDL_ON == PDL_DSTC_ENABLE_MFS1_RX)  || ...

#if (PDL_ON == PDL_DSTC_ENABLE_MFS0_TX)  || (PDL_ON == PDL_DSTC_ENABLE_MFS1_TX)  || \
    (PDL_ON == PDL_DSTC_ENABLE_MFS2_TX)  || (PDL_ON == PDL_DSTC_ENABLE_MFS3_TX)  || \
    (PDL_ON == PDL_DSTC_ENABLE_MFS4_TX)  || (PDL_ON == PDL_DSTC_ENABLE_MFS5_TX)  || \
    (PDL_ON == PDL_DSTC_ENABLE_MFS6_TX)  || (PDL_ON == PDL_DSTC_ENABLE_MFS7_TX)  || \
    (PDL_ON == PDL_DSTC_ENABLE_MFS8_TX)  || (PDL_ON == PDL_DSTC_ENABLE_MFS9_TX)  || \
    (PDL_ON == PDL_DSTC_ENABLE_MFS10_TX) || (PDL_ON == PDL_DSTC_ENABLE_MFS11_TX) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFS12_TX) || (PDL_ON == PDL_DSTC_ENABLE_MFS13_TX) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFS14_TX) || (PDL_ON == PDL_DSTC_ENABLE_MFS15_TX)
extern void Dstc_MfsTxIrqHandler(uint8_t u8IrqChannel)
{
  func_ptr_t pfnCallback;
  
  switch (u8IrqChannel)
  {
    #if (PDL_ON == PDL_DSTC_ENABLE_MFS0_RX)  
    case DSTC_IRQ_NUMBER_MFS0_RX:
      pfnCallback = stcDstcInternData.pfnDstcMfs0TxCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFS1_RX)  
    case DSTC_IRQ_NUMBER_MFS1_RX:
      pfnCallback = stcDstcInternData.pfnDstcMfs1TxCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFS2_RX)  
    case DSTC_IRQ_NUMBER_MFS2_RX:
      pfnCallback = stcDstcInternData.pfnDstcMfs2TxCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFS3_RX)  
    case DSTC_IRQ_NUMBER_MFS3_RX:
      pfnCallback = stcDstcInternData.pfnDstcMfs3TxCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFS4_RX)  
    case DSTC_IRQ_NUMBER_MFS4_RX:
      pfnCallback = stcDstcInternData.pfnDstcMfs4TxCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFS5_RX)  
    case DSTC_IRQ_NUMBER_MFS5_RX:
      pfnCallback = stcDstcInternData.pfnDstcMfs5TxCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFS6_RX)  
    case DSTC_IRQ_NUMBER_MFS6_RX:
      pfnCallback = stcDstcInternData.pfnDstcMfs6TxCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFS7_RX)  
    case DSTC_IRQ_NUMBER_MFS7_RX:
      pfnCallback = stcDstcInternData.pfnDstcMfs7TxCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFS8_RX)  
    case DSTC_IRQ_NUMBER_MFS8_RX:
      pfnCallback = stcDstcInternData.pfnDstcMfs8TxCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFS9_RX)  
    case DSTC_IRQ_NUMBER_MFS9_RX:
      pfnCallback = stcDstcInternData.pfnDstcMfs9TxCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFS10_RX)  
    case DSTC_IRQ_NUMBER_MFS10_RX:
      pfnCallback = stcDstcInternData.pfnDstcMfs10TxCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFS11_RX)  
    case DSTC_IRQ_NUMBER_MFS11_RX:
      pfnCallback = stcDstcInternData.pfnDstcMfs11TxCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFS12_RX)  
    case DSTC_IRQ_NUMBER_MFS12_RX:
      pfnCallback = stcDstcInternData.pfnDstcMfs12TxCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFS13_RX)  
    case DSTC_IRQ_NUMBER_MFS13_RX:
      pfnCallback = stcDstcInternData.pfnDstcMfs13TxCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFS14_RX)  
    case DSTC_IRQ_NUMBER_MFS14_RX:
      pfnCallback = stcDstcInternData.pfnDstcMfs14TxCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFS15_RX)  
    case DSTC_IRQ_NUMBER_MFS15_RX:
      pfnCallback = stcDstcInternData.pfnDstcMfs15TxCallback;
      break;
    #endif
    default:
      pfnCallback = NULL;
  }
  
  if (TRUE == Dstc_ReadHwintBit(u8IrqChannel))
  {
    Dstc_SetHwintclrBit(u8IrqChannel);
    
    if (pfnCallback != NULL)
    {
      pfnCallback();
    }
  }
}
#endif // #if (PDL_ON == PDL_DSTC_ENABLE_MFS0_TX)  || (PDL_ON == PDL_DSTC_ENABLE_MFS1_TX)  || ...

/// MFT FRT
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_FRT0_PEAK) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT0_FRT1_PEAK) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT0_FRT2_PEAK) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT1_FRT0_PEAK) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT1_FRT1_PEAK) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT1_FRT2_PEAK) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT2_FRT0_PEAK) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT2_FRT1_PEAK) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT2_FRT2_PEAK) 
void Dstc_MftFrtPeakIrqHandler(uint8_t u8IrqChannel)
{
  func_ptr_t pfnCallback;
  
  switch (u8IrqChannel)
  {
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT0_FRT0_PEAK) 
    case DSTC_IRQ_NUMBER_MFT0_FRT0_PEAK:
      pfnCallback = stcDstcInternData.pfnDstcMft0Frt0PeakCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT0_FRT1_PEAK) 
    case DSTC_IRQ_NUMBER_MFT0_FRT1_PEAK:
      pfnCallback = stcDstcInternData.pfnDstcMft0Frt1PeakCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT0_FRT2_PEAK) 
    case DSTC_IRQ_NUMBER_MFT0_FRT2_PEAK:
      pfnCallback = stcDstcInternData.pfnDstcMft0Frt2PeakCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT1_FRT0_PEAK) 
    case DSTC_IRQ_NUMBER_MFT1_FRT0_PEAK:
      pfnCallback = stcDstcInternData.pfnDstcMft1Frt0PeakCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT1_FRT1_PEAK) 
    case DSTC_IRQ_NUMBER_MFT1_FRT1_PEAK:
      pfnCallback = stcDstcInternData.pfnDstcMft1Frt1PeakCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT1_FRT2_PEAK) 
    case DSTC_IRQ_NUMBER_MFT1_FRT2_PEAK:
      pfnCallback = stcDstcInternData.pfnDstcMft1Frt2PeakCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT2_FRT0_PEAK) 
    case DSTC_IRQ_NUMBER_MFT2_FRT0_PEAK:
      pfnCallback = stcDstcInternData.pfnDstcMft2Frt0PeakCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT2_FRT1_PEAK) 
    case DSTC_IRQ_NUMBER_MFT2_FRT1_PEAK:
      pfnCallback = stcDstcInternData.pfnDstcMft2Frt1PeakCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT2_FRT2_PEAK) 
    case DSTC_IRQ_NUMBER_MFT2_FRT2_PEAK:
      pfnCallback = stcDstcInternData.pfnDstcMft2Frt2PeakCallback;
      break;
    #endif
    default:
      pfnCallback = NULL;
  }
  
  if (TRUE == Dstc_ReadHwintBit(u8IrqChannel))
  {
    Dstc_SetHwintclrBit(u8IrqChannel);
    
    if (pfnCallback != NULL)
    {
      pfnCallback();
    }
  }
}
#endif // #if (PDL_ON != PDL_DSTC_ENABLE_MFT0_FRT0_PEAK) || (PDL_ON != PDL_DSTC_ENABLE_MFT0_FRT1_PEAK) || ...

#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_FRT0_ZERO) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT0_FRT1_ZERO) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT0_FRT2_ZERO) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT1_FRT0_ZERO) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT1_FRT1_ZERO) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT1_FRT2_ZERO) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT2_FRT0_ZERO) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT2_FRT1_ZERO) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT2_FRT2_ZERO) 
void Dstc_MftFrtZeroIrqHandler(uint8_t u8IrqChannel)
{
  func_ptr_t pfnCallback;
  
  switch (u8IrqChannel)
  {
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT0_FRT0_ZERO) 
    case DSTC_IRQ_NUMBER_MFT0_FRT0_ZERO:
      pfnCallback = stcDstcInternData.pfnDstcMft0Frt0ZeroCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT0_FRT1_ZERO) 
    case DSTC_IRQ_NUMBER_MFT0_FRT1_ZERO:
      pfnCallback = stcDstcInternData.pfnDstcMft0Frt1ZeroCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT0_FRT2_ZERO) 
    case DSTC_IRQ_NUMBER_MFT0_FRT2_ZERO:
      pfnCallback = stcDstcInternData.pfnDstcMft0Frt2ZeroCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT1_FRT0_ZERO) 
    case DSTC_IRQ_NUMBER_MFT1_FRT0_ZERO:
      pfnCallback = stcDstcInternData.pfnDstcMft1Frt0ZeroCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT1_FRT1_ZERO) 
    case DSTC_IRQ_NUMBER_MFT1_FRT1_ZERO:
      pfnCallback = stcDstcInternData.pfnDstcMft1Frt1ZeroCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT1_FRT2_ZERO) 
    case DSTC_IRQ_NUMBER_MFT1_FRT2_ZERO:
      pfnCallback = stcDstcInternData.pfnDstcMft1Frt2ZeroCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT2_FRT0_ZERO) 
    case DSTC_IRQ_NUMBER_MFT2_FRT0_ZERO:
      pfnCallback = stcDstcInternData.pfnDstcMft2Frt0ZeroCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT2_FRT1_ZERO) 
    case DSTC_IRQ_NUMBER_MFT2_FRT1_ZERO:
      pfnCallback = stcDstcInternData.pfnDstcMft2Frt1ZeroCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT2_FRT2_ZERO) 
    case DSTC_IRQ_NUMBER_MFT2_FRT2_ZERO:
      pfnCallback = stcDstcInternData.pfnDstcMft2Frt2ZeroCallback;
      break;
    #endif
    default:
      pfnCallback = NULL;
  }
  
  if (TRUE == Dstc_ReadHwintBit(u8IrqChannel))
  {
    Dstc_SetHwintclrBit(u8IrqChannel);
    
    if (pfnCallback != NULL)
    {
      pfnCallback();
    }
  }
}
#endif // #if (PDL_ON != PDL_DSTC_ENABLE_MFT0_FRT0_PEAK) || (PDL_ON != PDL_DSTC_ENABLE_MFT0_FRT1_PEAK) || ...

#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_OCU0) || (PDL_ON == PDL_DSTC_ENABLE_MFT0_OCU1) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT0_OCU2) || (PDL_ON == PDL_DSTC_ENABLE_MFT0_OCU3) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT0_OCU4) || (PDL_ON == PDL_DSTC_ENABLE_MFT0_OCU5) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT1_OCU0) || (PDL_ON == PDL_DSTC_ENABLE_MFT1_OCU1) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT1_OCU2) || (PDL_ON == PDL_DSTC_ENABLE_MFT1_OCU3) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT1_OCU4) || (PDL_ON == PDL_DSTC_ENABLE_MFT1_OCU5) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT2_OCU0) || (PDL_ON == PDL_DSTC_ENABLE_MFT2_OCU1) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT2_OCU2) || (PDL_ON == PDL_DSTC_ENABLE_MFT2_OCU3) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT2_OCU4) || (PDL_ON == PDL_DSTC_ENABLE_MFT2_OCU5)
void Dstc_MftOcuIrqHandler(uint8_t u8IrqChannel)
{
  func_ptr_t pfnCallback;
  
  switch (u8IrqChannel)
  {
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT0_OCU0) 
    case DSTC_IRQ_NUMBER_MFT0_OCU0:
      pfnCallback = stcDstcInternData.pfnDstcMft0Ocu0Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT0_OCU1) 
    case DSTC_IRQ_NUMBER_MFT0_OCU1:
      pfnCallback = stcDstcInternData.pfnDstcMft0Ocu1Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT0_OCU2) 
    case DSTC_IRQ_NUMBER_MFT0_OCU2:
      pfnCallback = stcDstcInternData.pfnDstcMft0Ocu2Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT0_OCU3) 
    case DSTC_IRQ_NUMBER_MFT0_OCU3:
      pfnCallback = stcDstcInternData.pfnDstcMft0Ocu3Callback;
      break;
    #endif
     #if (PDL_ON == PDL_DSTC_ENABLE_MFT0_OCU4) 
    case DSTC_IRQ_NUMBER_MFT0_OCU4:
      pfnCallback = stcDstcInternData.pfnDstcMft0Ocu4Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT0_OCU5) 
    case DSTC_IRQ_NUMBER_MFT0_OCU5:
      pfnCallback = stcDstcInternData.pfnDstcMft0Ocu5Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT1_OCU0) 
    case DSTC_IRQ_NUMBER_MFT1_OCU0:
      pfnCallback = stcDstcInternData.pfnDstcMft1Ocu0Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT1_OCU1) 
    case DSTC_IRQ_NUMBER_MFT1_OCU1:
      pfnCallback = stcDstcInternData.pfnDstcMft1Ocu1Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT1_OCU2) 
    case DSTC_IRQ_NUMBER_MFT1_OCU2:
      pfnCallback = stcDstcInternData.pfnDstcMft1Ocu2Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT1_OCU3) 
    case DSTC_IRQ_NUMBER_MFT1_OCU3:
      pfnCallback = stcDstcInternData.pfnDstcMft1Ocu3Callback;
      break;
    #endif
     #if (PDL_ON == PDL_DSTC_ENABLE_MFT1_OCU4) 
    case DSTC_IRQ_NUMBER_MFT1_OCU4:
      pfnCallback = stcDstcInternData.pfnDstcMft1Ocu4Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT1_OCU5) 
    case DSTC_IRQ_NUMBER_MFT1_OCU5:
      pfnCallback = stcDstcInternData.pfnDstcMft1Ocu5Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT2_OCU0) 
    case DSTC_IRQ_NUMBER_MFT2_OCU0:
      pfnCallback = stcDstcInternData.pfnDstcMft2Ocu0Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT2_OCU1) 
    case DSTC_IRQ_NUMBER_MFT2_OCU1:
      pfnCallback = stcDstcInternData.pfnDstcMft2Ocu1Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT2_OCU2) 
    case DSTC_IRQ_NUMBER_MFT2_OCU2:
      pfnCallback = stcDstcInternData.pfnDstcMft2Ocu2Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT2_OCU3) 
    case DSTC_IRQ_NUMBER_MFT2_OCU3:
      pfnCallback = stcDstcInternData.pfnDstcMft2Ocu3Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT2_OCU4) 
    case DSTC_IRQ_NUMBER_MFT2_OCU4:
      pfnCallback = stcDstcInternData.pfnDstcMft2Ocu4Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT2_OCU5) 
    case DSTC_IRQ_NUMBER_MFT2_OCU5:
      pfnCallback = stcDstcInternData.pfnDstcMft2Ocu5Callback;
      break;
    #endif
    default:
      pfnCallback = NULL;
  }
  
  if (TRUE == Dstc_ReadHwintBit(u8IrqChannel))
  {
    Dstc_SetHwintclrBit(u8IrqChannel);
    
    if (pfnCallback != NULL)
    {
      pfnCallback();
    }
  }
}
#endif // #if (PDL_ON == PDL_DSTC_ENABLE_MFT0_OCU0) || (PDL_ON == PDL_DSTC_ENABLE_MFT0_OCU1) || ...

#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_WFG10) || (PDL_ON == PDL_DSTC_ENABLE_MFT0_WFG32) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT0_WFG54) || (PDL_ON == PDL_DSTC_ENABLE_MFT1_WFG10) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT1_WFG32) || (PDL_ON == PDL_DSTC_ENABLE_MFT1_WFG54) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT2_WFG10) || (PDL_ON == PDL_DSTC_ENABLE_MFT2_WFG32) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT2_WFG54)
void Dstc_MftWfgIrqHandler(uint8_t u8IrqChannel)
{
  func_ptr_t pfnCallback;
  
  switch (u8IrqChannel)
  {
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT0_WFG10) 
    case DSTC_IRQ_NUMBER_MFT0_WFG10:
      pfnCallback = stcDstcInternData.pfnDstcMft0Wfg10Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT0_WFG32) 
    case DSTC_IRQ_NUMBER_MFT0_WFG32:
      pfnCallback = stcDstcInternData.pfnDstcMft0Wfg32Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT0_WFG54) 
    case DSTC_IRQ_NUMBER_MFT0_WFG54:
      pfnCallback = stcDstcInternData.pfnDstcMft0Wfg54Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT1_WFG10) 
    case DSTC_IRQ_NUMBER_MFT1_WFG10:
      pfnCallback = stcDstcInternData.pfnDstcMft1Wfg10Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT1_WFG32) 
    case DSTC_IRQ_NUMBER_MFT1_WFG32:
      pfnCallback = stcDstcInternData.pfnDstcMft1Wfg32Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT1_WFG54) 
    case DSTC_IRQ_NUMBER_MFT1_WFG54:
      pfnCallback = stcDstcInternData.pfnDstcMft1Wfg54Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT2_WFG10) 
    case DSTC_IRQ_NUMBER_MFT2_WFG10:
      pfnCallback = stcDstcInternData.pfnDstcMft2Wfg10Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT2_WFG32) 
    case DSTC_IRQ_NUMBER_MFT2_WFG32:
      pfnCallback = stcDstcInternData.pfnDstcMft2Wfg32Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT2_WFG54) 
    case DSTC_IRQ_NUMBER_MFT2_WFG54:
      pfnCallback = stcDstcInternData.pfnDstcMft2Wfg54Callback;
      break;
    #endif
    default:
      pfnCallback = NULL;
  }
  
  if (TRUE == Dstc_ReadHwintBit(u8IrqChannel))
  {
    Dstc_SetHwintclrBit(u8IrqChannel);
    
    if (pfnCallback != NULL)
    {
      pfnCallback();
    }
  }
}
#endif // #if (PDL_ON == PDL_DSTC_ENABLE_MFT0_WFG10) || (PDL_ON == PDL_DSTC_ENABLE_MFT0_WFG32) ...
        
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_ICU0) || (PDL_ON == PDL_DSTC_ENABLE_MFT0_ICU1) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT0_ICU2) || (PDL_ON == PDL_DSTC_ENABLE_MFT0_ICU3) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT1_ICU0) || (PDL_ON == PDL_DSTC_ENABLE_MFT1_ICU1) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT1_ICU2) || (PDL_ON == PDL_DSTC_ENABLE_MFT1_ICU3) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT2_ICU0) || (PDL_ON == PDL_DSTC_ENABLE_MFT2_ICU1) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT2_ICU2) || (PDL_ON == PDL_DSTC_ENABLE_MFT2_ICU3)
void Dstc_MftIcuIrqHandler(uint8_t u8IrqChannel)
{
  func_ptr_t pfnCallback;
  
  switch (u8IrqChannel)
  {
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT0_ICU0) 
    case DSTC_IRQ_NUMBER_MFT0_ICU0:
      pfnCallback = stcDstcInternData.pfnDstcMft0Icu0Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT0_ICU1) 
    case DSTC_IRQ_NUMBER_MFT0_ICU1:
      pfnCallback = stcDstcInternData.pfnDstcMft0Icu1Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT0_ICU2) 
    case DSTC_IRQ_NUMBER_MFT0_ICU2:
      pfnCallback = stcDstcInternData.pfnDstcMft0Icu2Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT0_ICU3) 
    case DSTC_IRQ_NUMBER_MFT0_ICU3:
      pfnCallback = stcDstcInternData.pfnDstcMft0Icu3Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT1_ICU0) 
    case DSTC_IRQ_NUMBER_MFT1_ICU0:
      pfnCallback = stcDstcInternData.pfnDstcMft1Icu0Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT1_ICU1) 
    case DSTC_IRQ_NUMBER_MFT1_ICU1:
      pfnCallback = stcDstcInternData.pfnDstcMft1Icu1Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT1_ICU2) 
    case DSTC_IRQ_NUMBER_MFT1_ICU2:
      pfnCallback = stcDstcInternData.pfnDstcMft1Icu2Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT1_ICU3) 
    case DSTC_IRQ_NUMBER_MFT1_ICU3:
      pfnCallback = stcDstcInternData.pfnDstcMft1Icu3Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT2_ICU0) 
    case DSTC_IRQ_NUMBER_MFT2_ICU0:
      pfnCallback = stcDstcInternData.pfnDstcMft2Icu0Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT2_ICU1) 
    case DSTC_IRQ_NUMBER_MFT2_ICU1:
      pfnCallback = stcDstcInternData.pfnDstcMft2Icu1Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT2_ICU2) 
    case DSTC_IRQ_NUMBER_MFT2_ICU2:
      pfnCallback = stcDstcInternData.pfnDstcMft2Icu2Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_MFT2_ICU3) 
    case DSTC_IRQ_NUMBER_MFT2_ICU3:
      pfnCallback = stcDstcInternData.pfnDstcMft2Icu3Callback;
      break;
    #endif
    default:
      pfnCallback = NULL;
  }
  
  if (TRUE == Dstc_ReadHwintBit(u8IrqChannel))
  {
    Dstc_SetHwintclrBit(u8IrqChannel);
    
    if (pfnCallback != NULL)
    {
      pfnCallback();
    }
  }
}
#endif // #if (PDL_ON == PDL_DSTC_ENABLE_MFT0_ICU0) || (PDL_ON == PDL_DSTC_ENABLE_MFT0_ICU1) ...

/// PPG
#if (PDL_ON == PDL_DSTC_ENABLE_PPG0)  || (PDL_ON == PDL_DSTC_ENABLE_PPG2)  || \
    (PDL_ON == PDL_DSTC_ENABLE_PPG4)  || (PDL_ON == PDL_DSTC_ENABLE_PPG8)  || \
    (PDL_ON == PDL_DSTC_ENABLE_PPG4)  || (PDL_ON == PDL_DSTC_ENABLE_PPG8)  || \
    (PDL_ON == PDL_DSTC_ENABLE_PPG10) || (PDL_ON == PDL_DSTC_ENABLE_PPG12) || \
    (PDL_ON == PDL_DSTC_ENABLE_PPG16) || (PDL_ON == PDL_DSTC_ENABLE_PPG18) || \
    (PDL_ON == PDL_DSTC_ENABLE_PPG20)
void Dstc_PpgIrqHandler(uint8_t u8IrqChannel)
{
  func_ptr_t pfnCallback;
  
  switch (u8IrqChannel)
  {
    #if (PDL_ON == PDL_DSTC_ENABLE_PPG0) 
    case DSTC_IRQ_NUMBER_PPG0:
      pfnCallback = stcDstcInternData.pfnDstcPpg0Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_PPG2) 
    case DSTC_IRQ_NUMBER_PPG2:
      pfnCallback = stcDstcInternData.pfnDstcPpg2Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_PPG4) 
    case DSTC_IRQ_NUMBER_PPG4:
      pfnCallback = stcDstcInternData.pfnDstcPpg4Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_PPG8) 
    case DSTC_IRQ_NUMBER_PPG8:
      pfnCallback = stcDstcInternData.pfnDstcPpg8Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_PPG10) 
    case DSTC_IRQ_NUMBER_PPG10:
      pfnCallback = stcDstcInternData.pfnDstcPpg10Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_PPG12) 
    case DSTC_IRQ_NUMBER_PPG12:
      pfnCallback = stcDstcInternData.pfnDstcPpg12Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_PPG16) 
    case DSTC_IRQ_NUMBER_PPG16:
      pfnCallback = stcDstcInternData.pfnDstcPpg16Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_PPG18) 
    case DSTC_IRQ_NUMBER_PPG18:
      pfnCallback = stcDstcInternData.pfnDstcPpg18Callback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_PPG20) 
    case DSTC_IRQ_NUMBER_PPG20:
      pfnCallback = stcDstcInternData.pfnDstcPpg20Callback;
      break;
    #endif
    default:
      pfnCallback = NULL;
  }
  
  if (TRUE == Dstc_ReadHwintBit(u8IrqChannel))
  {
    Dstc_SetHwintclrBit(u8IrqChannel);
    
    if (pfnCallback != NULL)
    {
      pfnCallback();
    }
  }
}
#endif // #if (PDL_ON == PDL_DSTC_ENABLE_PPG0)  || (PDL_ON == PDL_DSTC_ENABLE_PPG2) ...
  
/// QPRC
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC0_COUNT_INVERSION)   || \
    (PDL_ON == PDL_DSTC_ENABLE_QPRC0_OUT_OF_RANGE)      || \
    (PDL_ON == PDL_DSTC_ENABLE_QPRC0_PC_MATCH)          || \
    (PDL_ON == PDL_DSTC_ENABLE_QPRC0_PC_MATCH_RC_MATCH) || \
    (PDL_ON == PDL_DSTC_ENABLE_QPRC0_PC_RC_MATCH)       || \
    (PDL_ON == PDL_DSTC_ENABLE_QPRC0_UFL_OFL_Z)         || \
    (PDL_ON == PDL_DSTC_ENABLE_QPRC1_COUNT_INVERSION)   || \
    (PDL_ON == PDL_DSTC_ENABLE_QPRC1_OUT_OF_RANGE)      || \
    (PDL_ON == PDL_DSTC_ENABLE_QPRC1_PC_MATCH)          || \
    (PDL_ON == PDL_DSTC_ENABLE_QPRC1_PC_MATCH_RC_MATCH) || \
    (PDL_ON == PDL_DSTC_ENABLE_QPRC1_PC_RC_MATCH)       || \
    (PDL_ON == PDL_DSTC_ENABLE_QPRC1_UFL_OFL_Z)         || \
    (PDL_ON == PDL_DSTC_ENABLE_QPRC2_COUNT_INVERSION)   || \
    (PDL_ON == PDL_DSTC_ENABLE_QPRC2_OUT_OF_RANGE)      || \
    (PDL_ON == PDL_DSTC_ENABLE_QPRC2_PC_MATCH)          || \
    (PDL_ON == PDL_DSTC_ENABLE_QPRC2_PC_MATCH_RC_MATCH) || \
    (PDL_ON == PDL_DSTC_ENABLE_QPRC2_PC_RC_MATCH)       || \
    (PDL_ON == PDL_DSTC_ENABLE_QPRC2_UFL_OFL_Z)
void Dstc_QprcIrqHandler(uint8_t u8IrqChannel)
{
  func_ptr_t pfnCallback;
  
  switch (u8IrqChannel)
  {
    #if (PDL_ON == PDL_DSTC_ENABLE_QPRC0_COUNT_INVERSION) 
    case DSTC_IRQ_NUMBER_QPRC0_COUNT_INVERSION:
      pfnCallback = stcDstcInternData.pfnDstcQprc0CountInversionCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_QPRC0_OUT_OF_RANGE) 
    case DSTC_IRQ_NUMBER_QPRC0_OUT_OF_RANGE:
      pfnCallback = stcDstcInternData.pfnDstcQprc0OutOfRangeCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_QPRC0_PC_MATCH) 
    case DSTC_IRQ_NUMBER_QPRC0_PC_MATCH:
      pfnCallback = stcDstcInternData.pfnDstcQprc0PcMatchCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_QPRC0_PC_MATCH_RC_MATCH) 
    case DSTC_IRQ_NUMBER_QPRC0_PC_MATCH_RC_MATCH:
      pfnCallback = stcDstcInternData.pfnDstcQprc0PcMatchRcMatchCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_QPRC0_PC_RC_MATCH) 
    case DSTC_IRQ_NUMBER_QPRC0_PC_RC_MATCH:
      pfnCallback = stcDstcInternData.pfnDstcQprc0PcRcMatchCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_QPRC0_UFL_OFL_Z) 
    case DSTC_IRQ_NUMBER_QPRC0_UFL_OFL_Z:
      pfnCallback = stcDstcInternData.pfnDstcQprc0UflOflZCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_QPRC1_COUNT_INVERSION) 
    case DSTC_IRQ_NUMBER_QPRC1_COUNT_INVERSION:
      pfnCallback = stcDstcInternData.pfnDstcQprc1CountInversionCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_QPRC1_OUT_OF_RANGE) 
    case DSTC_IRQ_NUMBER_QPRC1_OUT_OF_RANGE:
      pfnCallback = stcDstcInternData.pfnDstcQprc1OutOfRangeCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_QPRC1_PC_MATCH) 
    case DSTC_IRQ_NUMBER_QPRC1_PC_MATCH:
      pfnCallback = stcDstcInternData.pfnDstcQprc1PcMatchCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_QPRC1_PC_MATCH_RC_MATCH) 
    case DSTC_IRQ_NUMBER_QPRC1_PC_MATCH_RC_MATCH:
      pfnCallback = stcDstcInternData.pfnDstcQprc1PcMatchRcMatchCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_QPRC1_PC_RC_MATCH) 
    case DSTC_IRQ_NUMBER_QPRC1_PC_RC_MATCH:
      pfnCallback = stcDstcInternData.pfnDstcQprc1PcRcMatchCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_QPRC1_UFL_OFL_Z) 
    case DSTC_IRQ_NUMBER_QPRC1_UFL_OFL_Z:
      pfnCallback = stcDstcInternData.pfnDstcQprc1UflOflZCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_QPRC2_COUNT_INVERSION) 
    case DSTC_IRQ_NUMBER_QPRC2_COUNT_INVERSION:
      pfnCallback = stcDstcInternData.pfnDstcQprc2CountInversionCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_QPRC2_OUT_OF_RANGE) 
    case DSTC_IRQ_NUMBER_QPRC2_OUT_OF_RANGE:
      pfnCallback = stcDstcInternData.pfnDstcQprc2OutOfRangeCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_QPRC2_PC_MATCH) 
    case DSTC_IRQ_NUMBER_QPRC2_PC_MATCH:
      pfnCallback = stcDstcInternData.pfnDstcQprc2PcMatchCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_QPRC2_PC_MATCH_RC_MATCH) 
    case DSTC_IRQ_NUMBER_QPRC2_PC_MATCH_RC_MATCH:
      pfnCallback = stcDstcInternData.pfnDstcQprc2PcMatchRcMatchCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_QPRC2_PC_RC_MATCH) 
    case DSTC_IRQ_NUMBER_QPRC2_PC_RC_MATCH:
      pfnCallback = stcDstcInternData.pfnDstcQprc2PcRcMatchCallback;
      break;
    #endif
    #if (PDL_ON == PDL_DSTC_ENABLE_QPRC2_UFL_OFL_Z) 
    case DSTC_IRQ_NUMBER_QPRC2_UFL_OFL_Z:
      pfnCallback = stcDstcInternData.pfnDstcQprc2UflOflZCallback;
      break;
    #endif
    default:
      pfnCallback = NULL;
  }
  
  if (TRUE == Dstc_ReadHwintBit(u8IrqChannel))
  {
    Dstc_SetHwintclrBit(u8IrqChannel);
    
    if (pfnCallback != NULL)
    {
      pfnCallback();
    }
  }
}
#endif // #if (PDL_ON == PDL_DSTC_ENABLE_QPRC0_COUNT_INVERSION) ...

#if (PDL_ON == PDL_DSTC_ENABLE_WC)
void Dstc_WcIrqHandler(void)
{
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_WC))
  {
    Dstc_SetHwintclrBit(DSTC_IRQ_NUMBER_WC);
    
    if (stcDstcInternData.pfnDstcWcCallback != NULL)
    {
      stcDstcInternData.pfnDstcWcCallback();
    }
  }
}
#endif // #if (PDL_ON == PDL_DSTC_ENABLE_WC)

//@} // DstcGroup

#endif // #if (defined(PDL_PERIPHERAL_DSTC_ACTIVE)
/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
