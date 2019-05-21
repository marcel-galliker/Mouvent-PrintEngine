/*******************************************************************************
* Copyright (C) 2014 Spansion LLC. All Rights Reserved. 
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
/** \file exint.c
 **
 ** A detailed description is available at 
 ** @link ExintGroup Clock Module description @endlink
 **
 ** History:
 **   - 2013-08-16  1.0  MWi  First version.
 **   - 2014-05-09  1.1  MWi  Channel enable bug in Exint_Init() fixed.
 **
 ******************************************************************************/

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "exint.h"

#if (defined(PDL_PERIPHERAL_EXINT_ACTIVE))

/**
 ******************************************************************************
 ** \addtogroup ExintGroup
 ******************************************************************************/
//@{

/******************************************************************************/
/* Local pre-processor symbols/macros ('#define')                             */
/******************************************************************************/

/******************************************************************************/
/* Global variable definitions (declared in header file with 'extern')        */
/******************************************************************************/
stc_exint_intern_data_t stcExintInternData;
stc_exint_nmi_intern_data_t stcExintNMIInternData;

const uint8_t cau8ExintReferenceLut[PDL_EXINT_CHANNELS + 1] = 
{
#if (PDL_INTERRUPT_ENABLE_EXINT0 == PDL_ON)
  0,
#endif
#if (PDL_INTERRUPT_ENABLE_EXINT1 == PDL_ON)
  1,
#endif
#if (PDL_INTERRUPT_ENABLE_EXINT2 == PDL_ON)
  2,
#endif
#if (PDL_INTERRUPT_ENABLE_EXINT3 == PDL_ON)
  3,
#endif
#if (PDL_INTERRUPT_ENABLE_EXINT4 == PDL_ON)
  4,
#endif
#if (PDL_INTERRUPT_ENABLE_EXINT5 == PDL_ON)
  5,
#endif
#if (PDL_INTERRUPT_ENABLE_EXINT6 == PDL_ON)
  6,
#endif
#if (PDL_INTERRUPT_ENABLE_EXINT7 == PDL_ON)
  7,
#endif
#if (PDL_INTERRUPT_ENABLE_EXINT8 == PDL_ON)
  8,
#endif
#if (PDL_INTERRUPT_ENABLE_EXINT9 == PDL_ON)
  9,
#endif
#if (PDL_INTERRUPT_ENABLE_EXINT10 == PDL_ON)
  10,
#endif
#if (PDL_INTERRUPT_ENABLE_EXINT11 == PDL_ON)
  11,
#endif
#if (PDL_INTERRUPT_ENABLE_EXINT12 == PDL_ON)
  12,
#endif
#if (PDL_INTERRUPT_ENABLE_EXINT13 == PDL_ON)
  13,
#endif
#if (PDL_INTERRUPT_ENABLE_EXINT14 == PDL_ON)
  14,
#endif
#if (PDL_INTERRUPT_ENABLE_EXINT15 == PDL_ON)
  15,
#endif
#if (PDL_INTERRUPT_ENABLE_EXINT16_17_18_19 == PDL_ON)
  16, 17, 18, 19,
#endif
#if (PDL_INTERRUPT_ENABLE_EXINT20_21_22_23 == PDL_ON)
  20, 21, 22, 23,
#endif
#if (PDL_INTERRUPT_ENABLE_EXINT24_25_26_27 == PDL_ON)
  24, 25, 26, 27,
#endif
#if (PDL_INTERRUPT_ENABLE_EXINT28_29_30_31 == PDL_ON)
  28, 29, 20, 31,
#endif
  255   // End of table
};

/******************************************************************************/
/* Local type definitions ('typedef')                                         */
/******************************************************************************/

/******************************************************************************/
/* Local function prototypes ('static')                                       */
/******************************************************************************/

/**
 ******************************************************************************
 ** \brief Return the internal reference index of an Exint channel
 **
 ** \param u8Channel   Exint channel number
 **
 ** \return uint8_t    Reference index (255 if not found or u8Channel > 31)
 **
 ******************************************************************************/
static uint8_t ExintGetInternDataIdx(uint8_t u8Channel)
{
  uint8_t u8Index;
  
  if (u8Channel > 31)
  {
     return 255;
  }
  
  for (u8Index = 0; u8Index < PDL_EXINT_CHANNELS; u8Index++)
  {
    if (u8Channel == cau8ExintReferenceLut[u8Index])
    {
      return u8Index;
    }
  }
        
  return 255;  // Should necer happen ...
}

/******************************************************************************/
/* Local variable definitions ('static')                                      */
/******************************************************************************/

/******************************************************************************/
/* Function implementation - global ('extern') and local ('static')           */
/******************************************************************************/

/**
 ******************************************************************************
 ** \brief Interrupt handlers for External interrupts 0
 **
 ** This function clears the cause bit and calls the callback function, if
 ** specified.
 ******************************************************************************/
void Exint_IrqHandler(uint8_t u8Channel)
{
  uint8_t u8Reference;
  
  u8Reference = ExintGetInternDataIdx(u8Channel);
  
  FM4_EXTI->EICL &= (0xFFFFFFFFu ^ (1u << u8Channel));
  
  if (0 != stcExintInternData.apfnExintCallback[u8Reference])
  {
    stcExintInternData.apfnExintCallback[u8Reference]();
  }
} // Exint_IrqHandler

/**
 ******************************************************************************
 ** \brief Interrupt handler for None-Maskable Interrupt (NMI)
 **
 ** This function clears the cause bit and calls the NMI callback function, if
 ** specified.
 ******************************************************************************/
void ExintNMIrqHandler(void)
{
  if ((FM4_EXTI->NMIRR & 0x01) == 0x01)
  {
    FM4_EXTI->NMICL = 0;

    if (0 != stcExintNMIInternData.pfnNMICallback)
    {
      stcExintNMIInternData.pfnNMICallback();
    }
  }
}

/**
 ******************************************************************************
 ** \brief Init External Interrupts
 **
 ** This function initializes the external interrupts according the given
 ** configuration.
 **
 ** \param [in]  pstcConfig        EXINT configuration parameters
 ** 
 ** \retval Ok                     EXINT initialized
 ** \retval ErrorInvalidParameter  pstcConfig == NULL or Illegal mode
 ******************************************************************************/
en_result_t Exint_Init(stc_exint_config_t* pstcConfig)
{
  uint8_t  u8Index;
  uint8_t  u8Reference;
  uint32_t u32Elvr;
  uint32_t u32Elvr1;
  uint32_t u32Enir;
  
  u32Elvr  = 0u;
  u32Elvr1 = 0u;
  u32Enir  = 0u;
  
  if ( pstcConfig == NULL )
  {
    return ErrorInvalidParameter;
  }
  
  for (u8Index = 0u; u8Index < PDL_EXINT_CHANNELS; u8Index++)
  {
    u8Reference = cau8ExintReferenceLut[u8Index];
    
    if (u8Reference < 16u)
    {
      switch (pstcConfig->aenLevel[u8Reference])
      {
        case ExIntLowLevel:
          break;        // bits remain 2'b00
        case ExIntHighLevel:
          u32Elvr |= 1u << (2u * u8Reference);
          break;
        case ExIntRisingEdge:
          u32Elvr |= 2u << (2u * u8Reference);
          break;
        case ExIntFallingEdge:
          u32Elvr |= 3u << (2u * u8Reference);
          break;
        default:
          return ErrorInvalidParameter;
      }
    }
    else
    {
      switch (pstcConfig->aenLevel[u8Reference])
      {
        case ExIntLowLevel:
          break;        // bits remain 2'b00
        case ExIntHighLevel:
          u32Elvr1 |= 1u << (2u * (u8Reference - 8u));
          break;
        case ExIntRisingEdge:
          u32Elvr1 |= 2u << (2u * (u8Reference - 8u));
          break;
        case ExIntFallingEdge:
          u32Elvr1 |= 3u << (2u * (u8Reference - 8u));
          break;
        default:
          return ErrorInvalidParameter;
      }
    }
    
    if (TRUE == pstcConfig->abEnable[u8Reference])
    {
      u32Enir |= 1u << u8Reference;
    }
    stcExintInternData.apfnExintCallback[u8Index] = pstcConfig->apfnExintCallback[u8Reference];
  }
  
  // update hardware
  FM4_EXTI->ELVR  = u32Elvr;
  FM4_EXTI->ELVR1 = u32Elvr1;
  FM4_EXTI->EICL  = 0u;         // Clear all possible interrupts due to level change
  FM4_EXTI->ENIR  = u32Enir;
    
#if (PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT0) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT0)
  NVIC_ClearPendingIRQ(EXINT0_IRQn); 
  NVIC_SetPriority(EXINT0_IRQn, PDL_IRQ_LEVEL_EXINT0);
  NVIC_EnableIRQ(EXINT0_IRQn);
#endif
#if (PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT1) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT1)
  NVIC_ClearPendingIRQ(EXINT1_IRQn); 
  NVIC_SetPriority(EXINT1_IRQn, PDL_IRQ_LEVEL_EXINT1);
  NVIC_EnableIRQ(EXINT1_IRQn);
#endif
#if (PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT2) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT2)
  NVIC_ClearPendingIRQ(EXINT2_IRQn);
  NVIC_SetPriority(EXINT2_IRQn, PDL_IRQ_LEVEL_EXINT2);
  NVIC_EnableIRQ(EXINT2_IRQn);
#endif
#if (PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT3) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT3)
  NVIC_ClearPendingIRQ(EXINT3_IRQn);
  NVIC_SetPriority(EXINT3_IRQn, PDL_IRQ_LEVEL_EXINT3);
  NVIC_EnableIRQ(EXINT3_IRQn);
#endif
#if (PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT4) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT4)
  NVIC_ClearPendingIRQ(EXINT4_IRQn);
  NVIC_SetPriority(EXINT4_IRQn, PDL_IRQ_LEVEL_EXINT4);
  NVIC_EnableIRQ(EXINT4_IRQn);
#endif
#if (PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT5) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT5)
  NVIC_ClearPendingIRQ(EXINT5_IRQn);
  NVIC_SetPriority(EXINT5_IRQn, PDL_IRQ_LEVEL_EXINT5);
  NVIC_EnableIRQ(EXINT5_IRQn);
#endif
#if (PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT6) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT6)
  NVIC_ClearPendingIRQ(EXINT6_IRQn);
  NVIC_SetPriority(EXINT6_IRQn, PDL_IRQ_LEVEL_EXINT6);
  NVIC_EnableIRQ(EXINT6_IRQn);
#endif
#if (PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT7) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT7)
  NVIC_ClearPendingIRQ(EXINT7_IRQn);
  NVIC_SetPriority(EXINT7_IRQn, PDL_IRQ_LEVEL_EXINT7);
  NVIC_EnableIRQ(EXINT7_IRQn);
#endif
#if (PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT8) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT8)
  NVIC_ClearPendingIRQ(EXINT8_IRQn);
  NVIC_SetPriority(EXINT8_IRQn, PDL_IRQ_LEVEL_EXINT8);
  NVIC_EnableIRQ(EXINT8_IRQn);
#endif
#if (PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT9) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT9)
  NVIC_ClearPendingIRQ(EXINT9_IRQn);
  NVIC_SetPriority(EXINT9_IRQn, PDL_IRQ_LEVEL_EXINT9);
  NVIC_EnableIRQ(EXINT9_IRQn);
#endif
#if (PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT10) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT10)
  NVIC_ClearPendingIRQ(EXINT10_IRQn);
  NVIC_SetPriority(EXINT10_IRQn, PDL_IRQ_LEVEL_EXINT10);
  NVIC_EnableIRQ(EXINT10_IRQn);
#endif
#if (PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT11) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT11)
  NVIC_ClearPendingIRQ(EXINT11_IRQn);
  NVIC_SetPriority(EXINT11_IRQn, PDL_IRQ_LEVEL_EXINT11);
  NVIC_EnableIRQ(EXINT11_IRQn);
#endif
#if (PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT12) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT12)
  NVIC_ClearPendingIRQ(EXINT12_IRQn);
  NVIC_SetPriority(EXINT12_IRQn, PDL_IRQ_LEVEL_EXINT12);
  NVIC_EnableIRQ(EXINT12_IRQn);
#endif
#if (PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT13) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT13)
  NVIC_ClearPendingIRQ(EXINT13_IRQn);
  NVIC_SetPriority(EXINT13_IRQn, PDL_IRQ_LEVEL_EXINT13);
  NVIC_EnableIRQ(EXINT13_IRQn);
#endif
#if (PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT14) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT14)
  NVIC_ClearPendingIRQ(EXINT14_IRQn);
  NVIC_SetPriority(EXINT14_IRQn, PDL_IRQ_LEVEL_EXINT14);
  NVIC_EnableIRQ(EXINT14_IRQn);
#endif
#if (PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT15) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT15)
  NVIC_ClearPendingIRQ(EXINT15_IRQn);
  NVIC_SetPriority(EXINT15_IRQn, PDL_IRQ_LEVEL_EXINT15);
  NVIC_EnableIRQ(EXINT15_IRQn);
#endif
#if ((PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT16) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT16)) || \
    ((PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT17) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT17)) || \
    ((PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT18) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT18)) || \
    ((PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT19) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT19))
  NVIC_ClearPendingIRQ(EXINT16_17_18_19_IRQn);
  NVIC_SetPriority(EXINT16_17_18_19_IRQn, PDL_IRQ_LEVEL_EXINT16_17_18_19);
  NVIC_EnableIRQ(EXINT16_17_18_19_IRQn);
#endif
#if ((PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT20) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT20)) || \
    ((PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT21) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT21)) || \
    ((PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT22) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT22)) || \
    ((PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT23) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT23))
  NVIC_ClearPendingIRQ(EXINT20_21_22_23_IRQn);
  NVIC_SetPriority(EXINT20_21_22_23_IRQn, PDL_IRQ_LEVEL_EXINT20_21_22_23);
  NVIC_EnableIRQ(EXINT20_21_22_23_IRQn);
#endif
#if ((PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT24) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT24)) || \
    ((PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT25) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT25)) || \
    ((PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT26) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT26)) || \
    ((PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT27) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT27))
  NVIC_ClearPendingIRQ(EXINT24_25_26_27_IRQn);
  NVIC_SetPriority(EXINT24_25_26_27_IRQn, PDL_IRQ_LEVEL_EXINT24_25_26_27);
  NVIC_EnableIRQ(EXINT24_25_26_27_IRQn);
#endif
#if ((PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT28) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT28)) || \
    ((PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT29) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT29)) || \
    ((PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT30) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT30)) || \
    ((PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT31) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT31))
  NVIC_ClearPendingIRQ(EXINT28_29_30_31_IRQn);
  NVIC_SetPriority(EXINT28_29_30_31_IRQn, PDL_IRQ_LEVEL_EXINT28_29_30_31);
  NVIC_EnableIRQ(EXINT28_29_30_31_IRQn);
#endif

  return Ok;
} // Exint_DisableChannel

/**
 ******************************************************************************
 ** \brief Init External Interrupts
 **
 ** This function de-initializes all external interrupts.
 ** 
 ** \retval Ok                     EXINT sucessful disabled
 ******************************************************************************/
en_result_t Exint_DeInit(void)
{
  FM4_EXTI->ENIR = 0;   // Disable all channels
  
  // Disable interrupts
#if (PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT0) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT0)
  NVIC_SetPriority(EXINT0_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);
  NVIC_DisableIRQ(EXINT0_IRQn);
#endif
#if (PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT1) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT1)
  NVIC_SetPriority(EXINT1_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);
  NVIC_DisableIRQ(EXINT1_IRQn);
#endif
#if (PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT2) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT2)
  NVIC_SetPriority(EXINT2_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);
  NVIC_DisableIRQ(EXINT2_IRQn);
#endif
#if (PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT3) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT3)
  NVIC_SetPriority(EXINT3_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);
  NVIC_DisableIRQ(EXINT3_IRQn);
#endif
#if (PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT4) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT4)
  NVIC_SetPriority(EXINT4_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);
  NVIC_DisableIRQ(EXINT4_IRQn);
#endif
#if (PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT5) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT5)
  NVIC_SetPriority(EXINT5_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);
  NVIC_DisableIRQ(EXINT5_IRQn);
#endif
#if (PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT6) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT6)
  NVIC_SetPriority(EXINT6_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);
  NVIC_DisableIRQ(EXINT6_IRQn);
#endif
#if (PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT7) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT7)
  NVIC_SetPriority(EXINT7_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);
  NVIC_DisableIRQ(EXINT7_IRQn);
#endif
#if (PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT8) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT8)
  NVIC_SetPriority(EXINT8_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);
  NVIC_DisableIRQ(EXINT8_IRQn);
#endif
#if (PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT9) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT9)
  NVIC_SetPriority(EXINT9_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);
  NVIC_DisableIRQ(EXINT9_IRQn);
#endif
#if (PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT10) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT10)
  NVIC_SetPriority(EXINT10_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);
  NVIC_DisableIRQ(EXINT10_IRQn);
#endif
#if (PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT11) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT11)
  NVIC_SetPriority(EXINT11_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);
  NVIC_DisableIRQ(EXINT11_IRQn);
#endif
#if (PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT12) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT12)
  NVIC_SetPriority(EXINT12_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);
  NVIC_DisableIRQ(EXINT12_IRQn);
#endif
#if (PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT13) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT13)
  NVIC_SetPriority(EXINT13_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);
  NVIC_DisableIRQ(EXINT13_IRQn);
#endif
#if (PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT14) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT14)
  NVIC_SetPriority(EXINT14_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);
  NVIC_DisableIRQ(EXINT14_IRQn);
#endif
#if (PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT15) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT15)
  NVIC_SetPriority(EXINT15_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);
  NVIC_DisableIRQ(EXINT15_IRQn);
#endif
#if ((PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT16) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT16)) || \
    ((PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT17) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT17)) || \
    ((PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT18) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT18)) || \
    ((PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT19) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT19))
  NVIC_SetPriority(EXINT16_17_18_19_IRQn, PDL_IRQ_LEVEL_EXINT16_17_18_19);
  NVIC_DisableIRQ(EXINT16_17_18_19_IRQn);
#endif
#if ((PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT20) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT20)) || \
    ((PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT21) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT21)) || \
    ((PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT22) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT22)) || \
    ((PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT23) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT23))
  NVIC_SetPriority(EXINT20_21_22_23_IRQn, PDL_IRQ_LEVEL_EXINT20_21_22_23);
  NVIC_DisableIRQ(EXINT20_21_22_23_IRQn);
#endif
#if ((PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT24) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT24)) || \
    ((PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT25) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT25)) || \
    ((PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT26) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT26)) || \
    ((PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT27) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT27))
  NVIC_SetPriority(EXINT24_25_26_27_IRQn, PDL_IRQ_LEVEL_EXINT24_25_26_27);
  NVIC_DisableIRQ(EXINT24_25_26_27_IRQn);
#endif
#if ((PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT28) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT28)) || \
    ((PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT29) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT29)) || \
    ((PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT30) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT30)) || \
    ((PDL_ON == PDL_PERIPHERAL_ENABLE_EXINT31) && (PDL_ON == PDL_INTERRUPT_ENABLE_EXINT31))
  NVIC_SetPriority(EXINT28_29_30_31_IRQn, PDL_IRQ_LEVEL_EXINT28_29_30_31);
  NVIC_DisableIRQ(EXINT28_29_30_31_IRQn);
#endif  
  
  return Ok;  
} // Exint_DeInit

/**
 ******************************************************************************
 ** \brief Enable a single External Interrupt
 **
 ** This function enables a single External Interrupt.
 **
 ** \note This function does not set
 **       stc_exint_config_t#abEnable[u8Channel]!
 **
 ** \param [in]  u8Channel         External Interrupt channel number
 ** 
 ** \retval Ok                     Channel enabled
 ** \retval ErrorInvalidParameter  Invalid channel number
 ******************************************************************************/
en_result_t Exint_EnableChannel(uint8_t u8Channel)
{
  if (u8Channel > 32)
  {
    return ErrorInvalidParameter;
  }
  
  FM4_EXTI->EICL &= 0xFFFFFFFFu ^ (1u << u8Channel);
  FM4_EXTI->ENIR |= 1u << u8Channel;
  
  return Ok;
 
} // Exint_EnableChannel

/**
 ******************************************************************************
 ** \brief Disable a single External Interrupt
 **
 ** This function disables a single External Interrupt.
 **
 ** \note This function does not clear
 **       stc_exint_config_t#abEnable[u8Channel]!
 **
 ** \param [in]  u8Channel         External Interrupt channel number
 ** 
 ** \retval Ok                     Channel enabled
 ** \retval ErrorInvalidParameter  Invalid channel number or channel
 ******************************************************************************/
en_result_t Exint_DisableChannel(uint8_t u8Channel)
{
  if (u8Channel > 32)
  {
    return ErrorInvalidParameter;
  }
    
  FM4_EXTI->ENIR &= (1u << u8Channel) ^ 0xFFFFFFFFu;
  
  return Ok;
} // Exint_DisableChannel

/**
 ******************************************************************************
 ** \brief Init Non-Maskable Interrupt (NMI)
 **
 ** This function initializes the NMI according to the given
 ** configuration.
 **
 ** \param [in]  pstcConfig        NMI configuration parameters
 ** 
 ** \retval Ok                     NMI initialized
 ** \retval ErrorInvalidParameter  pstcConfig == NULL
 **
 ** \note NMI shares the interrupt vector with the Hardware Watchog. To avoid
 **       cross influence of initialization of the NVIC use
 **       stc_exint_nmi_config_t#bTouchNVIC to determine whether to touch the
 **       corresponding NVIC registers or not.
 ******************************************************************************/
en_result_t Exint_Nmi_Init(stc_exint_nmi_config_t* pstcConfig)
{
  if ( pstcConfig == NULL )
  {
    return ErrorInvalidParameter ;
  }

  // Set internal data
  stcExintNMIInternData.pfnNMICallback = pstcConfig->pfnNMICallback;
  
  if (pstcConfig->bTouchNVIC == TRUE)
  {
    NVIC_ClearPendingIRQ(NMI_IRQn);
    NVIC_EnableIRQ(NMI_IRQn);
    NVIC_SetPriority(NMI_IRQn, PDL_IRQ_LEVEL_NMI);    
  }
  
  return Ok; 
} // Exint_Nmi_Init

/**
 ******************************************************************************
 ** \brief De-Init Non-Maskable Interrupt (NMI)
 **
 ** This function de-initializes the NMI according to the given
 ** configuration.
 **
 ** \param [in]  pstcConfig        NMI configuration parameters
 ** 
 ** \retval Ok                     NMI initialized
 ** \retval ErrorInvalidParameter  pstcConfig == NULL
 **
 ** \note NMI shares the interrupt vector with the Hardware Watchog. To avoid
 **       cross influence of initialization of the NVIC use
 **       stc_exint_nmi_config_t#bTouchNVIC to determine whether to touch the
 **       corresponding NVIC registers or not.
 ******************************************************************************/
en_result_t Exint_Nmi_DeInit(stc_exint_nmi_config_t* pstcConfig)
{
  if ( pstcConfig == NULL )
  {
    return ErrorInvalidParameter ;
  }

  // Set internal data
  stcExintNMIInternData.pfnNMICallback = pstcConfig->pfnNMICallback;
  
  if (pstcConfig->bTouchNVIC == TRUE)
  {
    NVIC_ClearPendingIRQ(NMI_IRQn);
    NVIC_DisableIRQ(NMI_IRQn);
    NVIC_SetPriority(NMI_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);    
  }
  
  return Ok; 
} // Exint_Nmi_DeInit


//@} // ExitGroup

#endif // #if (defined(PDL_PERIPHERAL_EXINT_ACTIVE))

/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
