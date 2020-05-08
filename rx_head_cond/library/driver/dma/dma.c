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
/** \file dma.c
 **
 ** A detailed description is available at 
 ** @link DmaGroup DMA description @endlink
 **
 ** History:
 **   - 2013-03-19  0.1  MWi  First version
 **
 ******************************************************************************/

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "dma.h"

#if (defined(PDL_PERIPHERAL_DMA_ACTIVE))

/**
 ******************************************************************************
 ** \addtogroup DmaGroup
 ******************************************************************************/
//@{

/******************************************************************************/
/* Local pre-processor symbols/macros ('#define')                             */
/******************************************************************************/
// DMACAn => FM3_DMAC_BASE + size of register set + offset of DMACA0
// DMACAn => 0x40060000    + 0x10 * n             + 0x10
#define GET_DMA_ADDRESS(x) (FM4_DMAC_BASE + (0x10ul * (x)) + 0x10ul)

/******************************************************************************/
/* Global variable definitions (declared in header file with 'extern')        */
/******************************************************************************/
stc_dma_intern_data_t stcDmaInternData;

/******************************************************************************/
/* Local type definitions ('typedef')                                         */
/******************************************************************************/
typedef union
{
	uint32_t                 u32DMACA;
	stc_dmac_dmaca0_field_t  stcDMACA;
	
}un_dmac_dmaca_t;
  
typedef union
{
	stc_dmac_dmacb0_field_t  stcDMACB;
	uint32_t                 u32DMACB;
	
}un_dmac_dmacb_t;

/******************************************************************************/
/* Local function prototypes ('static')                                       */
/******************************************************************************/

/******************************************************************************/
/* Local variable definitions ('static')                                      */
/******************************************************************************/

/******************************************************************************/
/* Function implementation - global ('extern') and local ('static')           */
/******************************************************************************/
static en_result_t Dma_Irq_Init(uint8_t u8DmaChannel);
static en_result_t Dma_Irq_DeInit(uint8_t u8DmaChannel);

/**
 ******************************************************************************
 ** \brief DMA interrupt handler
 **
 ** \param [in]  u8DmaChannel     Channel number 
 **
 ** \note depending on the DMA stop status the DmaCallback function is called
 **       on successful transfer or DmaErrorCallback function on errornous
 **       transfer. The ErrorCallback has the error code as an (uint8_t)
 **       argument which reflects the 3 bits of the stop status.
 ******************************************************************************/
void DmaIrqHandler(uint8_t u8DmaChannel)
{
  uint32_t u32DmaRegisterBaseAddress;
  uint32_t u32RegisterReadOut;
  uint8_t  u8StopStatus;
  
  u32DmaRegisterBaseAddress = GET_DMA_ADDRESS(u8DmaChannel);
  
  // Readout DMACB and update
  u32RegisterReadOut = *(uint32_t*)((uint32_t)(u32DmaRegisterBaseAddress + DMA_DMACB_OFFSET));
  
  u8StopStatus = (uint8_t)((u32RegisterReadOut & 0x00070000ul)>> 16u);
  
  // Clear interrupt cause
  u32RegisterReadOut &= 0xFFF8FFFF;     
  *(uint32_t*)(uint32_t)((u32DmaRegisterBaseAddress + DMA_DMACB_OFFSET)) 
    = u32RegisterReadOut;  
  
  if (0x5u == u8StopStatus)  // Successful DMA transfer
  {
    stcDmaInternData.apfnDmaCallback[u8DmaChannel](); 
  }
  else                      // Errornous DMA transfer
  {
    stcDmaInternData.apfnDmaErrorCallback[u8DmaChannel](u8StopStatus);
  }
}

/**
 ******************************************************************************
 ** \brief Init DMA interrupt
 **
 ** Sets up an DMA error/completion interrupt.
 **
 ** \param [in]  u8DmaChannel     Channel number 
 **
 ** \retval Ok                    init successful
 **
 ******************************************************************************/
static en_result_t Dma_Irq_Init(uint8_t u8DmaChannel)
{
  switch(u8DmaChannel)
  {
    case 0u:
      NVIC_ClearPendingIRQ(DMAC0_IRQn);
      NVIC_EnableIRQ(DMAC0_IRQn);
      NVIC_SetPriority(DMAC0_IRQn, PDL_DMA0_LEVEL);    
    break;
    case 1u:
      NVIC_ClearPendingIRQ(DMAC1_IRQn);
      NVIC_EnableIRQ(DMAC1_IRQn);
      NVIC_SetPriority(DMAC1_IRQn, PDL_DMA1_LEVEL);    
    break;
    case 2u:
      NVIC_ClearPendingIRQ(DMAC2_IRQn);
      NVIC_EnableIRQ(DMAC2_IRQn);
      NVIC_SetPriority(DMAC2_IRQn, PDL_DMA2_LEVEL);    
    break;
    case 3u:
      NVIC_ClearPendingIRQ(DMAC3_IRQn);
      NVIC_EnableIRQ(DMAC3_IRQn);
      NVIC_SetPriority(DMAC3_IRQn, PDL_DMA3_LEVEL);    
    break;
    case 4u:
      NVIC_ClearPendingIRQ(DMAC4_IRQn);
      NVIC_EnableIRQ(DMAC4_IRQn);
      NVIC_SetPriority(DMAC4_IRQn, PDL_DMA4_LEVEL);    
    break;
    case 5u:
      NVIC_ClearPendingIRQ(DMAC5_IRQn);
      NVIC_EnableIRQ(DMAC5_IRQn);
      NVIC_SetPriority(DMAC5_IRQn, PDL_DMA5_LEVEL);    
    break;
    case 6u:
      NVIC_ClearPendingIRQ(DMAC6_IRQn);
      NVIC_EnableIRQ(DMAC6_IRQn);
      NVIC_SetPriority(DMAC6_IRQn, PDL_DMA6_LEVEL);    
    break;
    case 7u:
      NVIC_ClearPendingIRQ(DMAC7_IRQn);
      NVIC_EnableIRQ(DMAC7_IRQn);
      NVIC_SetPriority(DMAC7_IRQn, PDL_DMA7_LEVEL);    
    break;   
    default:
    break;
  }
  return Ok;
}

/**
 ******************************************************************************
 ** \brief De-Init DMA interrupt
 **
 ** Sets up an DMA error/completion interrupt.
 **
 ** \param [in]  u8DmaChannel     Channel number 
 **
 ** \retval Ok                    de-init successful
 **
 ******************************************************************************/
static en_result_t Dma_Irq_DeInit(uint8_t u8DmaChannel)
{
  switch(u8DmaChannel)
  {
    case 0:
      NVIC_ClearPendingIRQ(DMAC0_IRQn);
      NVIC_DisableIRQ(DMAC0_IRQn);
      NVIC_SetPriority(DMAC0_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);    
    break;
    case 1:
      NVIC_ClearPendingIRQ(DMAC1_IRQn);
      NVIC_DisableIRQ(DMAC1_IRQn);
      NVIC_SetPriority(DMAC1_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);    
    break;
    case 2:
      NVIC_ClearPendingIRQ(DMAC2_IRQn);
      NVIC_DisableIRQ(DMAC2_IRQn);
      NVIC_SetPriority(DMAC2_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);    
    break;
    case 3:
      NVIC_ClearPendingIRQ(DMAC3_IRQn);
      NVIC_DisableIRQ(DMAC3_IRQn);
      NVIC_SetPriority(DMAC3_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);    
    break;
    case 4:
      NVIC_ClearPendingIRQ(DMAC4_IRQn);
      NVIC_DisableIRQ(DMAC4_IRQn);
      NVIC_SetPriority(DMAC4_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);    
    break;
    case 5:
      NVIC_ClearPendingIRQ(DMAC5_IRQn);
      NVIC_DisableIRQ(DMAC5_IRQn);
      NVIC_SetPriority(DMAC5_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);    
    break;
    case 6:
      NVIC_ClearPendingIRQ(DMAC6_IRQn);
      NVIC_DisableIRQ(DMAC6_IRQn);
      NVIC_SetPriority(DMAC6_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);    
    break;
    case 7:
      NVIC_ClearPendingIRQ(DMAC7_IRQn);
      NVIC_DisableIRQ(DMAC7_IRQn);
      NVIC_SetPriority(DMAC7_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);    
    break;    
    default:
    break;
  }
  return Ok;
}

/**
 ******************************************************************************
 ** \brief Initializes a DMA channel
 **
 ** Sets up an DMA channel without starting immediate DMA transfer.
 ** Enable_Dma_Channel() is used for starting DMA transfer.
 **
 ** \param [in]  pstcConfig       DMA module configuration 
 **
 ** \retval Ok                    init successful
 ** \retval ErrorInvalidParameter pstcAdc == NULL or other invalid configuration
 ** \retval OperationInProgress   DMA channel already in use
 **
 ******************************************************************************/
en_result_t Dma_Init_Channel(volatile stc_dma_config_t* pstcConfig)                   
{  
  un_dmac_dmaca_t unDmacDmaca;
  un_dmac_dmacb_t unDmacDmacb;
  uint32_t        u32DmaRegisterBaseAddress;
  func_ptr_t      pfnCallBackPointer;
  func_ptr_arg1_t pfnErrorCallBackPointer;
	boolean_t bAvoidSideEffects;

  // Check for NULL pointer
  if (pstcConfig == NULL)
  {
    return ErrorInvalidParameter ;
  }
  
  unDmacDmaca.u32DMACA = 0u;
  unDmacDmacb.u32DMACB = 0u;

  u32DmaRegisterBaseAddress = GET_DMA_ADDRESS(pstcConfig->u8DmaChannel);
  
  // Set Callback function pointers
  pfnCallBackPointer = pstcConfig->pfnCallback;
  stcDmaInternData.apfnDmaCallback[pstcConfig->u8DmaChannel] = pfnCallBackPointer;
  
  pfnErrorCallBackPointer = pstcConfig->pfnErrorCallback;
  stcDmaInternData.apfnDmaErrorCallback[pstcConfig->u8DmaChannel] = pfnErrorCallBackPointer;
  
  // Check if DMA channel already used
  if (stcDmaInternData.au8DmaChannelActive[pstcConfig->u8DmaChannel] == DMA_CHANNEL_ACTIVE)
  {
    return OperationInProgress;
  }
  
  // Set DMA channel as active
  stcDmaInternData.au8DmaChannelActive[pstcConfig->u8DmaChannel] = DMA_CHANNEL_ACTIVE;
  
  // Setup DMACA ...
  //
  // Block Count
  unDmacDmaca.stcDMACA.BC = 0x0Fu & (uint32_t)(pstcConfig->u8BlockCount);
  
  // Transfer Count
  unDmacDmaca.stcDMACA.TC = 0xFFFFu & (uint32_t)(pstcConfig->u16TransferCount);
  
  // Interrupt Request Channel
  if (pstcConfig->enDmaIdrq == Software)
  {
    unDmacDmaca.stcDMACA.IS = 0u;
  }
  else
  {
    unDmacDmaca.stcDMACA.IS = 0x20u | (0x1Fu &  (uint32_t)(pstcConfig->enDmaIdrq));
  }
  
  // ... and update hardware
  *(uint32_t*)(uint32_t)((u32DmaRegisterBaseAddress + DMA_DMACA_OFFSET)) = unDmacDmaca.u32DMACA ;
  
  // Setup DMACB ...
  //
  // First Transfer mode
  switch (pstcConfig->enTransferMode)
  {
    case DmaBlockTransfer:
      unDmacDmacb.stcDMACB.MS = 0u;
      break;
    case DmaBurstTransfer:
      unDmacDmacb.stcDMACB.MS = 1u;
      break;
    case DmaDemandTransfer:
      unDmacDmacb.stcDMACB.MS = 2u;
      break;
    default :
      return ErrorInvalidParameter;
  }
  
  // Transfer width
  switch (pstcConfig->enTransferWdith)
  {
    case Dma8Bit:
      unDmacDmacb.stcDMACB.TW = 0u;
      break;
    case Dma16Bit:
      unDmacDmacb.stcDMACB.TW = 1u;
      break;
    case Dma32Bit:
      unDmacDmacb.stcDMACB.TW = 2u;
      break;
    default :
      return ErrorInvalidParameter;
  }
 
  // Fixed source
  unDmacDmacb.stcDMACB.FS = (uint8_t)((pstcConfig->bFixedSource == TRUE) ? 1u : 0u);
  
  // Fixed destination
  unDmacDmacb.stcDMACB.FD = (uint8_t)((pstcConfig->bFixedDestination == TRUE) ? 1u : 0u);
  
  // Reload Count (BC/TC reload)
  unDmacDmacb.stcDMACB.RC = (uint8_t)((pstcConfig->bReloadCount == TRUE) ? 1u : 0u);
  
  // Reload Source
  unDmacDmacb.stcDMACB.RS = (uint8_t)((pstcConfig->bReloadSource == TRUE) ? 1u : 0u);
  
  // Reload Destination
  unDmacDmacb.stcDMACB.RD = (uint8_t)((pstcConfig->bReloadDestination == TRUE) ? 1u : 0u);
  
  // Error interrupt enable
  unDmacDmacb.stcDMACB.EI = (uint8_t)((pstcConfig->bErrorInterruptEnable == TRUE) ? 1u : 0u);
  
  // Completion interrupt enable
  unDmacDmacb.stcDMACB.CI = (uint8_t)((pstcConfig->bCompletionInterruptEnable == TRUE) ? 1u : 0u);
  
  // Workaround for MISRA rule 12.4!!! pstcConfig is not modified in this context!!!
  bAvoidSideEffects = ((pstcConfig->bErrorInterruptEnable == TRUE) ? 1u : 0u);
  bAvoidSideEffects |= ((pstcConfig->bCompletionInterruptEnable == TRUE) ? 1u : 0u);
  if (bAvoidSideEffects == TRUE)
  {
    Dma_Irq_Init(pstcConfig->u8DmaChannel);
  }
  
  // Enable bit mask
  unDmacDmacb.stcDMACB.EM = (uint8_t)((pstcConfig->bEnableBitMask == TRUE) ? 1u : 0u);
  
  // ... and update hardware
  *(uint32_t*)((uint32_t)(u32DmaRegisterBaseAddress + DMA_DMACB_OFFSET))
    = unDmacDmacb.u32DMACB ;
  
  // Setup source address
  *(uint32_t*)((uint32_t)(u32DmaRegisterBaseAddress + DMA_DMASA_OFFSET))
    = pstcConfig->u32SourceAddress;
  
  // Setup destination address
  *(uint32_t*)((uint32_t)(u32DmaRegisterBaseAddress + DMA_DMADA_OFFSET))
    = pstcConfig->u32DestinationAddress;
  
  // Switch resource interrupt to DMA (except software DMA)
  if (pstcConfig->enDmaIdrq != Software)
  {
    uint32_t u32DrqselBit = 1ul << (uint32_t)(pstcConfig->enDmaIdrq);
    FM4_INTREQ->DRQSEL |= u32DrqselBit;
  }
  
  return Ok;
}

/**
 ******************************************************************************
 ** \brief Enable, disable, pause, or trigger a DMA channel via configuration
 **
 ** This function enables, disables, pauses or triggers a DMA transfer according
 ** to the settings in the configuration bits for EB (Enable), PB (Pause) and
 ** ST (Software Trigger)
 **
 ** \param [in]  pstcConfig       DMA module configuration 
 **
 ** \retval Ok                    Setting finished
 **
 ******************************************************************************/
en_result_t Dma_Set_Channel(volatile stc_dma_config_t* pstcConfig)
{
	un_dmac_dmaca_t unDmacDmaca;
  uint32_t u32RegisterReadOut ;
  uint32_t u32DmaRegisterBaseAddress ;

  // Check for NULL pointer
  if (pstcConfig == NULL)
  {
    return ErrorInvalidParameter ;
  }
  
  unDmacDmaca.u32DMACA = 0u;
  unDmacDmaca.stcDMACA.EB = (uint8_t)((pstcConfig->bEnable == TRUE) ? 1u : 0u);
  unDmacDmaca.stcDMACA.PB = (uint8_t)((pstcConfig->bPause == TRUE) ? 1u : 0u);
  unDmacDmaca.stcDMACA.ST = (uint8_t)((pstcConfig->bSoftwareTrigger == TRUE) ? 1u : 0u);
  
  u32DmaRegisterBaseAddress = GET_DMA_ADDRESS(pstcConfig->u8DmaChannel);
  
  // Readout DMACA and update
  u32RegisterReadOut = *(uint32_t*)((uint32_t)(u32DmaRegisterBaseAddress + DMA_DMACA_OFFSET)) ;
  unDmacDmaca.u32DMACA |= (u32RegisterReadOut & 0x1FFFFFFFu);   // masking EB, PB, ST ...
  *(uint32_t*)((uint32_t)(u32DmaRegisterBaseAddress + DMA_DMACA_OFFSET)) = unDmacDmaca.u32DMACA ;
  
  return Ok;
}

/**
 ******************************************************************************
 ** \brief Enable DMA globally
 **
 ** \retval Ok                    Enable finished
 **
 ******************************************************************************/
en_result_t Dma_Enable(void)
{
  FM4_DMAC->DMACR = 0x80000000ul; /// Set DE Bit (DMA enable all channels)
  
  return Ok;
}

/**
 ******************************************************************************
 ** \brief Disable DMA globally
 **
 ** \retval Ok                    Disable finished
 **
 ******************************************************************************/
en_result_t Dma_Disable(void)
{
  FM4_DMAC->DMACR = 0u; /// Clear everything
  
  return Ok;
}

/**
 ******************************************************************************
 ** \brief De-Initializes a DMA channel
 **
 ** Clears an DMA channel.
 **
 ** \param [in]  pstcConfig       DMA module configuration 
 **
 ** \retval Ok                    init successful
 ** \retval ErrorInvalidParameter pstcAdc == NULL or other invalid configuration
 **
 ******************************************************************************/
en_result_t Dma_DeInit_Channel(volatile stc_dma_config_t* pstcConfig)  
{
  uint32_t u32DmaRegisterBaseAddress;
  
  // Check for NULL pointer
  if (pstcConfig == NULL)
  {
    return ErrorInvalidParameter ;
  }
  
  Dma_Irq_DeInit(pstcConfig->u8DmaChannel);
  
  u32DmaRegisterBaseAddress = GET_DMA_ADDRESS(pstcConfig->u8DmaChannel);
  
  // clear all registers of DMA channel
  *(uint32_t*)((uint32_t)(u32DmaRegisterBaseAddress + DMA_DMACA_OFFSET)) = 0u;
  *(uint32_t*)((uint32_t)(u32DmaRegisterBaseAddress + DMA_DMACB_OFFSET)) = 0u;
  *(uint32_t*)((uint32_t)(u32DmaRegisterBaseAddress + DMA_DMASA_OFFSET)) = 0u;
  *(uint32_t*)((uint32_t)(u32DmaRegisterBaseAddress + DMA_DMADA_OFFSET)) = 0u;

  // Mark DMA channel as inactive
  stcDmaInternData.au8DmaChannelActive[pstcConfig->u8DmaChannel]
    = DMA_CHANNEL_INACTIVE;
  
  return Ok;
} // Dma_DeInit_Channel

/**
 ******************************************************************************
 ** \brief Enable, disable, pause, or trigger a DMA channel via arguments
 **
 ** This function enables, disables, pauses or triggers a DMA transfer according
 ** to the settings in the configuration bits for EB (Enable), PB (Pause) and
 ** ST (Software Trigger)
 **
 ** \param [in]  u8DmaChannel         DMA channel to be set
 ** \param [in]  bEnable              TRUE = Channel enabled, FALSE = Channel
 **                                   disabled
 ** \param [in]  bPause               TRUE = Channel paused, FALSE = Channel
 **                                   working
 ** \param [in]  bSoftwareTrigger     TRUE = Channel trigger, FALSE = Channel
 **                                   no trigger
 **
 ** \retval Ok                        Setting finished
 **
 ******************************************************************************/
en_result_t Dma_Set_ChannelParam( uint8_t   u8DmaChannel,
                                  boolean_t bEnable,
                                  boolean_t bPause,
                                  boolean_t bSoftwareTrigger
                                )
{
	un_dmac_dmaca_t unDmacDmaca;
  uint32_t   u32RegisterReadOut ;
  uint32_t   u32DmaRegisterBaseAddress ;
  
  unDmacDmaca.u32DMACA = 0u;
  unDmacDmaca.stcDMACA.EB = (uint8_t)((bEnable          == TRUE) ? 1u : 0u);
  unDmacDmaca.stcDMACA.PB = (uint8_t)((bPause           == TRUE) ? 1u : 0u);
  unDmacDmaca.stcDMACA.ST = (uint8_t)((bSoftwareTrigger == TRUE) ? 1u : 0u);
  
  u32DmaRegisterBaseAddress = GET_DMA_ADDRESS(u8DmaChannel);
  
  // Readout DMACA and update
  u32RegisterReadOut = *(uint32_t*)((uint32_t)(u32DmaRegisterBaseAddress + DMA_DMACA_OFFSET)) ;
  unDmacDmaca.u32DMACA |= (u32RegisterReadOut & 0x1FFFFFFFul);   // masking EB, PB, ST ...
  *(uint32_t*)((uint32_t)(u32DmaRegisterBaseAddress + DMA_DMACA_OFFSET)) = unDmacDmaca.u32DMACA ;
  
  return Ok;
} // Dma_Set_ChannelParam

/**
 ******************************************************************************
 ** \brief De-Initializes a DMA channel via channel parameter
 **
 ** Clears an DMA channel.
 **
 ** \param [in]  u8DmaChannel     DMA channel to be de-initialized
 **
 ** \retval Ok                    init successful
 **
 ******************************************************************************/
en_result_t Dma_DeInit_ChannelParam(uint8_t u8DmaChannel)  
{
  uint32_t u32DmaRegisterBaseAddress;
  
  Dma_Irq_DeInit(u8DmaChannel);
  
  u32DmaRegisterBaseAddress = GET_DMA_ADDRESS(u8DmaChannel);
  
  // clear all registers of DMA channel
  *(uint32_t*)((uint32_t)(u32DmaRegisterBaseAddress + DMA_DMACA_OFFSET)) = 0ul;
  *(uint32_t*)((uint32_t)(u32DmaRegisterBaseAddress + DMA_DMACB_OFFSET)) = 0ul;
  *(uint32_t*)((uint32_t)(u32DmaRegisterBaseAddress + DMA_DMASA_OFFSET)) = 0ul;
  *(uint32_t*)((uint32_t)(u32DmaRegisterBaseAddress + DMA_DMADA_OFFSET)) = 0ul;

  // Mark DMA channel as inactive
  stcDmaInternData.au8DmaChannelActive[u8DmaChannel]
    = DMA_CHANNEL_INACTIVE;
  
  return Ok;
} // Dma_DeInit_ChannelParam

//@} // DmaGroup

#endif // #if (defined(PDL_PERIPHERAL_DMA_ACTIVE))

/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/

