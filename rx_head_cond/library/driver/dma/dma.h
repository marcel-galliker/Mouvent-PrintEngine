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
/** \file dma.h
 **
 ** Headerfile for DMA functions
 **  
 ** History:
 **   - 2013-03-19  0.0.1  MWi  First version
 **
 ******************************************************************************/

#ifndef __DMA_H__
#define __DMA_H__

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "pdl.h"

#if (defined(PDL_PERIPHERAL_DMA_ACTIVE))

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/**
 ******************************************************************************
 ** \defgroup DmaGroup Direct Memory Access (DMA)
 **
 ** Provided functions of DMA module:
 ** 
 ** - Dma_Init_Channel()
 ** - Dma_Set_Channel()
 ** - Dma_Enable()
 ** - Dma_Disable()
 ** - Dma_DeInit_Channel()
 ** - Dma_Set_ChannelParam()
 ** - Dma_DeInit_ChannelParam()
 **
 ** The DMA is configured by Dma_Init_Channel() but not started then.
 ** With the function Dma_Set_Channel() the enable, pause and/or trigger
 ** bits can be set.
 ** Dma_Enable() enables globally the DMA and Dma_Disable() disables DMA
 ** globally.
 ** Dma_DeInit_Channel() clears a channel for a possible new configuration.
 **
 ** Once a DMA channel was setup by Dma_Init_Channel() it cannot be 
 ** re-initialized by this function (with a new configuration) anymore.
 ** result_t#OperationInProgress is returned in this case. Dma_DeInit_Channel()
 ** has to be called before to unlock the channel for a new configuration.
 **
 ** Dma_Set_ChannelParam() and Dma_DeInit_ChannelParam() perform the same
 ** functionality as Dma_Set_Channel() and Dma_DeInit_Channel() instead of
 ** configuration usage. Here direct arguments are used.
 **
 ** \note Set stc_dma_config_t#u16TransferCount to "Number of Transfers - 1"!
 **
 ******************************************************************************/
//@{
  
/******************************************************************************/
/* Global pre-processor symbols/macros ('#define')                            */
/******************************************************************************/
 
// register address offset
#define DMA_DMACA_OFFSET 0x00ul
#define DMA_DMACB_OFFSET 0x04ul
#define DMA_DMASA_OFFSET 0x08ul
#define DMA_DMADA_OFFSET 0x0Cul
  
// DMA channel active markers for internal data:
// au8DmaChannelActive[DMA_MAX_CHANNELS]
#define DMA_CHANNEL_ACTIVE   1u
#define DMA_CHANNEL_INACTIVE 0u
  
/******************************************************************************/
/* Local type definitions ('typedef')                                         */
/******************************************************************************/

/**
 ******************************************************************************
 ** \brief DMA transfer data width
 ******************************************************************************/
typedef enum en_dma_transfermode
{
  DmaBlockTransfer  = 0,
  DmaBurstTransfer  = 1,
  DmaDemandTransfer = 2 
} en_dma_transfermode_t ;

/**
 ******************************************************************************
 ** \brief DMA transfer data width
 ******************************************************************************/
typedef enum en_dma_transferwidth
{
  Dma8Bit  = 0,  ///< 8 bit transfer via DMA
  Dma16Bit = 1,  ///< 16 bit transfer via DMA
  Dma32Bit = 2   ///< 32 bit transfer via DMA
} en_dma_transferwidth_t ;

/**
 ******************************************************************************
 ** \brief DMA IDREQ number
 ******************************************************************************/
typedef enum en_dma_idreq
{
  Ep1DrqUsb0  = 0,   ///< Interrupt signal from EP1 DRQ of USB ch.0
  Ep2DrqUsb0  = 1,   ///< Interrupt signal from EP2 DRQ of USB ch.0
  Ep3DrqUsb0  = 2,   ///< Interrupt signal from EP3 DRQ of USB ch.0
  Ep4DrqUsb0  = 3,   ///< Interrupt signal from EP4 DRQ of USB ch.0
  Ep5DrqUsb0  = 4,   ///< Interrupt signal from EP5 DRQ of USB ch.0
  Adc0        = 5,   ///< Scan conversion interrupt signal from A/D converter unit0
  Adc1        = 6,   ///< Scan conversion interrupt signal from A/D converter unit1
  Adc2        = 7,   ///< Scan conversion interrupt signal from A/D converter unit2
  Bt0Irq0     = 8,   ///< Interrupt signal from IRQ0 of base timer ch.0
  Bt2Irq0     = 9,   ///< Interrupt signal from IRQ0 of base timer ch.2
  Bt4Irq0     = 10,  ///< Interrupt signal from IRQ0 of base timer ch.4
  Bt6Irq0     = 11,  ///< Interrupt signal from IRQ0 of base timer ch.6
  MfsRx0      = 12,  ///< Receiving interrupt signal from MFS ch.0
  MfsTx0      = 13,  ///< Sending interrupt signal from MFS ch.0   
  MfsRx1      = 14,  ///< Receiving interrupt signal from MFS ch.1
  MfsTx1      = 15,  ///< Sending interrupt signal from MFS ch.1
  MfsRx2      = 16,  ///< Receiving interrupt signal from MFS ch.2
  MfsTx2      = 17,  ///< Sending interrupt signal from MFS ch.2
  MfsRx3      = 18,  ///< Receiving interrupt signal from MFS ch.3
  MfsTx3      = 19,  ///< Sending interrupt signal from MFS ch.3 
  MfsRx4      = 20,  ///< Receiving interrupt signal from MFS ch.4
  MfsTx4      = 21,  ///< Sending interrupt signal from MFS ch.4 
  MfsRx5      = 22,  ///< Receiving interrupt signal from MFS ch.5
  MfsTx5      = 23,  ///< Sending interrupt signal from MFS ch.5 
  MfsRx6      = 24,  ///< Receiving interrupt signal from MFS ch.6
  MfsTx6      = 25,  ///< Sending interrupt signal from MFS ch.6 
  MfsRx7      = 26,  ///< Receiving interrupt signal from MFS ch.7
  MfsTx7      = 27,  ///< Sending interrupt signal from MFS ch.7 
  ExtIrq0     = 28,  ///< Interrupt signal from external interrupt unit ch.0
  ExtIrq1     = 29,  ///< Interrupt signal from external interrupt unit ch.1    
  ExtIrq2     = 30,  ///< Interrupt signal from external interrupt unit ch.2
  ExtIrq3     = 31,  ///< Interrupt signal from external interrupt unit ch.3 
  Software    = 1234 ///< Software Demand (just a high number)    
} en_dma_idreq_t ;

/**
 ******************************************************************************
 ** \brief Datatype for holding internal data needed for DMA
 ******************************************************************************/
typedef struct stc_dma_intern_data
{
  uint8_t         au8DmaChannelActive[PDL_DMA_CHANNELS];   ///< Channel active marker array
  func_ptr_t      apfnDmaCallback[PDL_DMA_CHANNELS];       ///< Callback pointer array
  func_ptr_arg1_t apfnDmaErrorCallback[PDL_DMA_CHANNELS];  ///< Error Callback pointer array

} stc_dma_intern_data_t ;

/**
 ******************************************************************************
 ** \brief DMA instance data type
 ******************************************************************************/
typedef struct stc_dma_instance_data
{
  stc_dma_intern_data_t stcInternData; ///< module internal data of instance
} stc_dma_instance_data_t;

/**
 ******************************************************************************
 ** \brief DMA configuration
 ******************************************************************************/
typedef struct stc_dma_config
{
  boolean_t              bEnable ;               ///< Overall DMA channel enable (only set by Enable_Dma_Channel())
  boolean_t              bPause ;                ///< Pause bit of DMA (only set by Enable_Dma_Channel())
  boolean_t              bSoftwareTrigger ;      ///< Trigger bit for software transfer (only set by Enable_Dma_Channel())
  uint8_t                u8DmaChannel ;          ///< DmaChannel for recent configuration
  en_dma_idreq_t         enDmaIdrq ;             ///< ID Request number (see #en_dma_idreq_t for details)
  uint8_t                u8BlockCount ;          ///< Block counter
  uint16_t               u16TransferCount ;      ///< Transfer counter, set <b>TC = Number of Transfers - 1</b>!
  en_dma_transfermode_t  enTransferMode ;        ///< DMA transfer mode (see #en_dma_transfermode_t for details)
  en_dma_transferwidth_t enTransferWdith ;       ///< DMA transfer width (see #en_dma_transferwidth_t for details)
  uint32_t               u32SourceAddress;       ///< Source address
  uint32_t               u32DestinationAddress ; ///< Destination address
  boolean_t              bFixedSource ;          ///< TRUE = source address not increased
  boolean_t              bFixedDestination ;     ///< TRUE = destiantion address not increased
  boolean_t              bReloadCount ;          ///< TRUE = count is reloaded
  boolean_t              bReloadSource ;         ///< TRUE = source address is reloaded
  boolean_t              bReloadDestination ;    ///< TRUE = destination address is reloaded
  boolean_t              bErrorInterruptEnable ; ///< TRUE = interrupt on error ocurrance
  boolean_t              bCompletionInterruptEnable ; ///< TRUE = interrupt on DMA tranfer completion
  boolean_t              bEnableBitMask ;        ///< FALSE = Clear EB (bEnable) bit on completion (mandatory for transfer end!)
  func_ptr_t             pfnCallback ;           ///< Callback on sucessful DMA transfer
  func_ptr_arg1_t        pfnErrorCallback ;      ///< Callback(uint8_t) on errornous DMA transfer
  uint8_t                u8ErrorStopStatus ;     ///< Error code from Stop Status
} stc_dma_config_t ;

/******************************************************************************/
/* Global variable definitions ('extern')                                     */
/******************************************************************************/
extern stc_dma_intern_data_t stcDmaInternData;

/******************************************************************************/
/* Global function prototypes (definition in C source)                        */
/******************************************************************************/  

extern void DmaIrqHandler(uint8_t u8DmaChannel) ;

en_result_t Dma_Init_Channel(volatile stc_dma_config_t* pstcConfig) ;

en_result_t Dma_Set_Channel(volatile stc_dma_config_t* pstcConfig) ;

en_result_t Dma_Enable(void) ;

en_result_t Dma_Disable(void) ;

en_result_t Dma_DeInit_Channel(volatile stc_dma_config_t* pstcConfig) ;

en_result_t Dma_Set_ChannelParam( uint8_t   u8DmaChannel,
                                  boolean_t bEnable,
                                  boolean_t bPause,
                                  boolean_t bSoftwareTrigger
                                ) ;

en_result_t Dma_DeInit_ChannelParam(uint8_t u8DmaChannel) ;
  
//@} // DmaGroup

#ifdef __cplusplus
}
#endif

#endif // #if (defined(PDL_PERIPHERAL_DMA_ACTIVE))

#endif /* __DMA_H__ */
/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
