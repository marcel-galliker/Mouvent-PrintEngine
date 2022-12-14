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
/** \file can.c
 **
 ** A detailed description is available at 
 ** @link CanGroup CAN Module description @endlink
 **
 ** History:
 **   - 2013-08-21  1.0  MWi  First version.
 **
 ******************************************************************************/

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "can.h"

#if (defined(PDL_PERIPHERAL_CAN_ACTIVE))

/**
 ******************************************************************************
 ** \addtogroup CanGroup
 ******************************************************************************/
//@{

/******************************************************************************/
/* Local pre-processor symbols/macros ('#define')                             */
/******************************************************************************/

/// Macro to return the number of enabled CAN instances
#define CAN_INSTANCE_COUNT (uint32_t)(sizeof(m_astcCanInstanceDataLut) / sizeof(m_astcCanInstanceDataLut[0]))

/// Status interrupt detected
#define CAN_STATUS_INTERRUPT       (1 << 15)

/** LEC (Last Error Code) special value indicating that no changes happened
    after the CPU has written this special value to LEC */
#define CAN_NO_LEC_CHANGE       7

#define CAN_MSG_DIR_RX          0 ///< WRRD bit indicating read direction
#define CAN_MSG_DIR_TX          1 ///< WRRD bit indicating write direction
#define CAN_MSG_ID_EXTENDED     1 ///< Extended msg id is used

/******************************************************************************/
/* Global variable definitions (declared in header file with 'extern')        */
/******************************************************************************/

/// Look-up table for all enabled CAN instances and their internal data
stc_can_instance_data_t m_astcCanInstanceDataLut[] =
{
  #if (PDL_PERIPHERAL_ENABLE_CAN0 == PDL_ON)
    { 
      &CAN0,  // pstcInstance
      NULL    // stcInternData (not initialized yet)
    },
  #endif
  #if (PDL_PERIPHERAL_ENABLE_CAN1 == PDL_ON)
    { 
      &CAN1,  // pstcInstance
      NULL    // stcInternData (not initialized yet)
    },
  #endif
};

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

/******************************************************************************/
/* Local Functions                                                            */
/******************************************************************************/

/**
 ******************************************************************************
 ** \brief Return the internal data for a certain CAN instance.
 **
 ** \param pstcCan Pointer to CAN instance
 **
 ** \return Pointer to internal data or NULL if instance is not enabled (or not known)
 **
 ******************************************************************************/
static stc_can_intern_data_t* CanGetInternDataPtr(volatile stc_cann_t* pstcCan) 
{
    uint32_t u32Instance;
   
    for (u32Instance = 0; u32Instance < CAN_INSTANCE_COUNT; u32Instance++)
    {
        if (pstcCan == m_astcCanInstanceDataLut[u32Instance].pstcInstance)
        {
            return &m_astcCanInstanceDataLut[u32Instance].stcInternData;
        }
    }

    return NULL;
}

/**
 ******************************************************************************
 ** \brief Set NVIC Interrupt depending on CAN instance
 **
 ** \param pstcCan Pointer to CAN instance
 **
 ******************************************************************************/
static void Can_InitInterrupt(volatile stc_cann_t* pstcCan)
{
  if (pstcCan == (stc_cann_t*)(&CAN0))
  {
    NVIC_ClearPendingIRQ(CAN0_IRQn);
    NVIC_EnableIRQ(CAN0_IRQn);
    NVIC_SetPriority(CAN0_IRQn, PDL_IRQ_LEVEL_CAN0);   
  }
  else if (pstcCan == (stc_cann_t*)(&CAN1))
  {
    NVIC_ClearPendingIRQ(CAN1_IRQn);
    NVIC_EnableIRQ(CAN1_IRQn);
    NVIC_SetPriority(CAN1_IRQn, PDL_IRQ_LEVEL_CAN1);      
  }
  
  // Do nothing, if wrong instance. (Will never happen, because called
  //   only from Can_Init with previous CAN instance check before!)
}

/**
 ******************************************************************************
 ** \brief Clear NVIC Interrupt depending on CAN instance
 **
 ** \param pstcCan Pointer to CAN instance
 **
 ******************************************************************************/
static void Can_DeInitInterrupt(volatile stc_cann_t* pstcCan)
{
  if (pstcCan == (stc_cann_t*)(&CAN0))
  {
    NVIC_ClearPendingIRQ(CAN0_IRQn);
    NVIC_DisableIRQ(CAN0_IRQn);
    NVIC_SetPriority(CAN0_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);   
  }
  else if (pstcCan == (stc_cann_t*)(&CAN1))
  {
    NVIC_ClearPendingIRQ(CAN1_IRQn);
    NVIC_DisableIRQ(CAN1_IRQn);
    NVIC_SetPriority(CAN1_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);      
  }
  
  // Do nothing, if wrong instance. (Will never happen, because called
  //   only from Can_DeInit with previous CAN instance check before!)
}


/******************************************************************************/
/* Global Functions                                                           */
/******************************************************************************/

/**
 *****************************************************************************
 ** \brief CAN (Status/Error/Rx/Tx) interrupt ISR
 **
 ** Check for any pending interrupt source and process until no more INTs are
 ** active.
 **
 ** \param [in] pstcCan             Pointer to register area of a CAN unit.
 ** \param [in] pstcCanInternData   Pointer to internal data.
 **
 **  Description of flow:
 **  - write new data any time into user buffer
 **  - check bNew, if TRUE (user buffer not released yet)
 **     > set bOverflow
 **  - set bNew
 **  - call callback function
 **  - the MSGLST flag of the HW message buffer
 **    should be OR-mirrored into bOverflow, too
 *****************************************************************************/
void CanIrqHandler( volatile stc_cann_t*   pstcCan,
                    stc_can_intern_data_t* pstcCanInternData
                  )
{
    uint16_t                u16MessageBufferNumber ;
    boolean_t               bDone ;
    stc_can_statr_t         unCanStatusRegister ;
    stc_can_ifncmsk_t       unCanIf2Cmsk;

    bDone = FALSE ;

    while( bDone == FALSE )
    {
        u16MessageBufferNumber = pstcCan->INTR ;
  
        switch ( u16MessageBufferNumber )
        {
            // nothing more to do
            case 0:
                bDone = TRUE ;
                break ;

            // Check for status interrupt
            case CAN_STATUS_INTERRUPT:
                // Get status register, clear of status Irq is done automatically by readout
                unCanStatusRegister = pstcCan->STATR_f;

                // Check for error if the error callback function is defined.
                if (pstcCanInternData->stcCanNotificationCb.pfnCanErrorInterruptFunction != NULL)
                {
                    // Check if bus off error was set (transmit error counter > 255)
                    if (unCanStatusRegister.BOFF != 0)
                    {
                        // Bus off
                        pstcCanInternData->stcCanNotificationCb.pfnCanErrorInterruptFunction(CanBusOff);
                    }

                    // Check if error warning level is reached (receive and/or transmit error counter >= 96).
                    if (unCanStatusRegister.EWARN != 0)
                    {
                        // Warning level reached.
                        pstcCanInternData->stcCanNotificationCb.pfnCanErrorInterruptFunction(CanWarning);
                    }
                }

                // Check for status if the status change callback function is defined.
                if (pstcCanInternData->stcCanNotificationCb.pfnCanStatusInterruptFunction != NULL)
                {
                    // LEC different to CAN_NO_LEC_CHANGE, report last error.
                    if (unCanStatusRegister.LEC != CAN_NO_LEC_CHANGE)
                    {
                        pstcCanInternData->stcCanNotificationCb.pfnCanStatusInterruptFunction((en_can_status_t)(unCanStatusRegister.LEC));

                        // Now reset LEC to special value CAN_NO_LEC_CHANGE
                        pstcCan->STATR_f.LEC = CAN_NO_LEC_CHANGE;
                    }
                }
                break ;

            // Default path is message buffer handling
            default:
                // Write command mask register to perform a selective read based on the bits set
                unCanIf2Cmsk.WRRD     = CAN_MSG_DIR_RX; // Set direction to rx
                unCanIf2Cmsk.MASK     = 1; // Transfer Identifier Mask + MDir + MXtd to Message Object
                unCanIf2Cmsk.ARB      = 1; // Transfer Identifier + Dir + Xtd + MsgVal to Message Object
                unCanIf2Cmsk.CONTROL  = 1; // Transfer Control Bits to Message Object
                unCanIf2Cmsk.CIP      = 1; // Clear Interrupt Pending Bit
                unCanIf2Cmsk.NEWDAT   = 1; // Set TXRQST/NEWDAT bit
                unCanIf2Cmsk.DATAA    = 1; // Transfer Data Bytes 0-3 to Mesage Object
                unCanIf2Cmsk.DATAB    = 1; // Transfer Data Bytes 4-7 to Mesage Object
                pstcCan->IF2CMSK_f = unCanIf2Cmsk;

                // Write message buffer number to request reading
                // the message buffer into the interface registers
                // We can write full 16bit here because BUSY bit is not used in this mode
                pstcCan->IF2CREQ = u16MessageBufferNumber ;

                // Check if message object is configured for reception or transmission
                if (pstcCan->u32IF2ARB.DIR == CAN_MSG_DIR_RX)
                {
                    // CAN message received, check if there is a callback function
                    // for this message buffer registered
                    if (pstcCanInternData->stcCanInterruptHandling[u16MessageBufferNumber - 1].pstcMsgBuffer != NULL)
                    {
                        stc_can_msg_t* pstcMsgBuffer = pstcCanInternData->stcCanInterruptHandling[u16MessageBufferNumber - 1].pstcMsgBuffer;
                        uint16_t* pu16Data = (uint16_t*)&pstcMsgBuffer->stcData.au8Data[0] ;

                        // Check if 11- or 29-bit identifier was used for message.
                        if ( pstcCan->IF2ARB2_f.XTD != CAN_MSG_ID_EXTENDED )
                        {
                            // 11-bit identifier was used.
                            pstcMsgBuffer->stcIdentifier.bExtended = FALSE;
                            pstcMsgBuffer->stcIdentifier.u32Identifier = (pstcCan->u32IF2ARB.ID >> 18);
                        }
                        else
                        {
                            // 29-bit identifier was used.
                            pstcMsgBuffer->stcIdentifier.bExtended = TRUE;
                            pstcMsgBuffer->stcIdentifier.u32Identifier = pstcCan->u32IF2ARB.ID;
                        }

                        // Get data bytes
                        pu16Data[0] = pstcCan->IF2DTA1_L; 
                        pu16Data[1] = pstcCan->IF2DTA2_L;
                        pu16Data[2] = pstcCan->IF2DTB1_L; 
                        pu16Data[3] = pstcCan->IF2DTB2_L; 

                        // Set data length of received data bytes.
                        pstcMsgBuffer->stcData.u8DataLengthCode = pstcCan->IF2MCTR_f.DLC;

                        // Check if message lost flag is set in receive message buffer.
                        if ( pstcCan->IF2MCTR_f.MSGLST == 1 )
                        {
                            pstcMsgBuffer->bOverflow = TRUE ;

                            // Reset message lost flag.
                            pstcCan->IF2MCTR_f.MSGLST = 0 ;

                            // Write command mask register to perform a write to CAN control register.
                            // Pls. note that INTPND and NEWDAT are left unchanged if written with '0'
                            unCanIf2Cmsk = pstcCan->IF2CMSK_f;
                            unCanIf2Cmsk.WRRD     = CAN_MSG_DIR_RX; // Set direction to tx
                            unCanIf2Cmsk.CONTROL  = 1;              // Transfer control bits
                            pstcCan->IF2CMSK_f = unCanIf2Cmsk;

                            // Set message buffer number to update message buffer.
                            // We can write full 16bit here because BUSY bit is not used in this mode
                            pstcCan->IF2CREQ = u16MessageBufferNumber;
                        }

                        // Check if data new flag is already set.
                        if ( pstcMsgBuffer->bNew == TRUE )
                        {
                            // New data flag was already set, that means that
                            // at least one message is lost, so overflow flag
                            // must be set.
                            pstcMsgBuffer->bOverflow = TRUE ;
                        }
                        else
                        {
                            // Set new data flag.
                            pstcMsgBuffer->bNew = TRUE ;
                        }

                        // Call callback function if it was set previously.
                        if (pstcCanInternData->stcCanInterruptHandling[u16MessageBufferNumber - 1].pfnCanRxInterruptFunction != NULL)
                        {
                            pstcCanInternData->stcCanInterruptHandling[u16MessageBufferNumber - 1].pfnCanRxInterruptFunction((uint8_t)u16MessageBufferNumber, pstcMsgBuffer);
                        }
                    }
                }
                else // if (pstcCan->u32IF2ARB.DIR == CAN_MSG_DIR_RX)
                {
                    // CAN message succesfully transmitted.
                    // Call callback function if it was set previously.
                    if (pstcCanInternData->stcCanInterruptHandling[u16MessageBufferNumber - 1].pfnCanTxInterruptFunction != NULL)
                    {
                        pstcCanInternData->stcCanInterruptHandling[u16MessageBufferNumber - 1].pfnCanTxInterruptFunction((uint8_t)u16MessageBufferNumber);
                    }
                }
                break ;
        } // switch ( u16MessageBufferNumber )
    } // while ( !bDone )
} // CanIrqHandler

/**
 *****************************************************************************
 ** \brief Initializes the CAN module.
 **
 ** This function initializes one specific CAN module with the parameters
 ** provided in the given config structure.
 ** After initialization the CAN module has Error, Status and Module-Interrupt
 ** enabled and is ready to use.
 **
 ** Can_Init() has to be called with the parameter pstcConfig of type
 ** stc_can_config_t the basic CAN settings automatic retransmission, the CAN
 ** baudrate, and the error and status change callback function can be set.
 **
 ** All values in pstcConfig have to be in valid range (see can.h for allowed
 ** ranges of dedicated parameters). The error and status change callback
 ** functions can be NULL. In this case no information of error or status
 ** changes will be reported to the API.
 **
 ** To reset and disable the CAN module the function Can_DeInit() has to be used.
 **
 ** The resulting CAN presecaler value is checked, if it is within CAN_MAX_CLK
 ** (normally 16 MHz).
 **
 ** \param [in] pstcCan           Pointer to register area of a CAN unit.
 ** \param [in] pstcConfig        CAN configuration parameters.
 **
 ** \retval Ok                    CAN module has been successfully initialized.
 ** \retval ErrorInvalidParameter If one of the following conditions are met:
 **   - pstcCan == NULL
 **   - pstcConfig == NULL
 **   - pstcCanInternData == NULL (invalid or disabled CAN unit (PDL_PERIPHERAL_ENABLE_CAN))
 **   - pstcConfig->stcBitrate.u8TimeSegment1 out of range
 **   - pstcConfig->stcBitrate.u8TimeSegment2 out of range
 **   - pstcConfig->stcBitrate.u8SyncJumpWidth out of range
 **   - pstcConfig->stcBitrate.u16Prescaler out of range
 **   - pstcConfig->stcBitrate.enCanPrescaler wrong enumerator
 **   - pstcConfig->stcBitrate.enCanPrescaler CAN_MAX_CLK violated
 *****************************************************************************/
en_result_t Can_Init( volatile stc_cann_t* pstcCan, const stc_can_config_t* pstcConfig )
{
    stc_can_intern_data_t*  pstcCanInternData;
    stc_can_btr_t           unBTR;
    stc_can_ifncmsk_t       unIF1CMSK;
    stc_can_ctrlr_t         unCTRLR;
    uint8_t                 u8MessageNumber;
    uint8_t                 u8HclkDiv;
    uint8_t                 u8HclkMul;
    
    PDL_ZERO_STRUCT(unBTR);
    PDL_ZERO_STRUCT(unIF1CMSK);
    PDL_ZERO_STRUCT(unCTRLR);
    
    // Check for NULL pointers
    if ( pstcCan    == NULL ||
         pstcConfig == NULL
       )
    {
      return ErrorInvalidParameter;
    }

    // Check range of input parameters.....
    if  ( ((pstcConfig->stcBitrate.u8TimeSegment1 < CAN_BITRATE_TSEG1_MIN)  || (pstcConfig->stcBitrate.u8TimeSegment1 > CAN_BITRATE_TSEG1_MAX))    ||
          ((pstcConfig->stcBitrate.u8TimeSegment2 < CAN_BITRATE_TSEG2_MIN)  || (pstcConfig->stcBitrate.u8TimeSegment2 > CAN_BITRATE_TSEG2_MAX))     ||
          ((pstcConfig->stcBitrate.u8SyncJumpWidth < CAN_BITRATE_SYNC_JUMP_WIDTH_MIN) || (pstcConfig->stcBitrate.u8SyncJumpWidth > CAN_BITRATE_SYNC_JUMP_WIDTH_MAX))    ||
          ((pstcConfig->stcBitrate.u16Prescaler < CAN_BITRATE_PRESCALER_MIN) || (pstcConfig->stcBitrate.u16Prescaler > CAN_BITRATE_PRESCALER_MAX))
        )
    {
      return ErrorInvalidParameter;
    }

    // Get pointer to internal data structure
    pstcCanInternData = CanGetInternDataPtr( pstcCan ) ;

    // Check for NULL
    if (pstcCanInternData == NULL)
    {
      return ErrorInvalidParameter;
    }

    if (TRUE == (pstcConfig->bTouchPrescaler))
    {
      // CAN dedicated clock prescaler
      switch(pstcConfig->stcBitrate.enCanPrescaler)
      {
        case CanPreDiv11:
          CANPRE = 0x00;
          u8HclkDiv = 1;
          if ((__HCLK / u8HclkDiv) > CAN_MAX_CLK)
          {
            return ErrorInvalidParameter;
          }
          break;
        case CanPreDiv12:
          CANPRE = 0x01;
          u8HclkDiv = 2;
          if ((__HCLK / u8HclkDiv) > CAN_MAX_CLK)
          {
            return ErrorInvalidParameter;
          }
          break;
        case CanPreDiv14:
          CANPRE = 0x02;
          u8HclkDiv = 4;
          if ((__HCLK / u8HclkDiv) > CAN_MAX_CLK)
          {
            return ErrorInvalidParameter;
          }
          break;
        case CanPreDiv18:
          CANPRE = 0x04;
          u8HclkDiv = 8;
          if ((__HCLK / u8HclkDiv) > CAN_MAX_CLK)
          {
            return ErrorInvalidParameter;
          }
          break;
        case CanPreDiv23:
          CANPRE = 0x08;
          u8HclkMul = 2;
          u8HclkDiv = 3;
          if (((__HCLK * u8HclkMul) / u8HclkDiv) > CAN_MAX_CLK)
          {
            return ErrorInvalidParameter;
          }
          break;
        case CanPreDiv13:
          CANPRE = 0x09;
          u8HclkDiv = 3;
          if ((__HCLK / u8HclkDiv) > CAN_MAX_CLK)
          {
            return ErrorInvalidParameter;
          }
          break;
        case CanPreDiv16:
          CANPRE = 0x0A;
          u8HclkDiv = 6;
          if ((__HCLK / u8HclkDiv) > CAN_MAX_CLK)
          {
            return ErrorInvalidParameter;
          }
          break;
        case CanPreDiv112:
          CANPRE = 0x0B;
          u8HclkDiv = 12;
          if ((__HCLK / u8HclkDiv) > CAN_MAX_CLK)
          {
            return ErrorInvalidParameter;
          }
          break;
        case CanPreDiv15:
          CANPRE = 0x0C;
          u8HclkDiv = 5;
          if ((__HCLK / u8HclkDiv) > CAN_MAX_CLK)
          {
            return ErrorInvalidParameter;
          }
          break;
        case CanPreDiv110:
          CANPRE = 0x0E;
          u8HclkDiv = 10;
          if ((__HCLK / u8HclkDiv) > CAN_MAX_CLK)
          {
            return ErrorInvalidParameter;
          }
          break;
        default:
          return ErrorInvalidParameter;
      }
    }
    
    // Enable access to init and bit timing register.
    pstcCan->CTRLR_f.INIT = 1;
    pstcCan->CTRLR_f.CCE  = 1;

    unBTR.BRP    = pstcConfig->stcBitrate.u16Prescaler - 1  ;
    unBTR.SJW    = pstcConfig->stcBitrate.u8SyncJumpWidth - 1 ;
    unBTR.TSEG1  = pstcConfig->stcBitrate.u8TimeSegment1 - 1 ;
    unBTR.TSEG2  = pstcConfig->stcBitrate.u8TimeSegment2 - 1 ;
    pstcCan->BTR_f = unBTR ; // Write to Hardware

    pstcCan->BRPER = ((pstcConfig->stcBitrate.u16Prescaler - 1) >> 6);

    // Now disable access to bit timing register
    pstcCan->CTRLR_f.CCE = 0;

    // Reset status register.
    pstcCan->STATR = 0x0000;

    // Reset message valid bit of each message to invalid.
    pstcCan->IF1ARB = 0 ;

    // Reset message control register.
    pstcCan->IF1MCTR = 0x0000;

    // Write command mask register to for update arbitration and control part of message object.
    unIF1CMSK.WRRD    = CAN_MSG_DIR_TX;
    unIF1CMSK.ARB     = 1;
    unIF1CMSK.CONTROL = 1;
    pstcCan->IF1CMSK_f = unIF1CMSK;

    // Set each message buffer number to update message buffer with reset message valid bit.
    for (u8MessageNumber = 1; u8MessageNumber <= CAN_MESSAGE_BUFFER_COUNT; u8MessageNumber++)
    {
        pstcCan->IF1CREQ = u8MessageNumber ;
    }

    // Set notification callback functions
    pstcCanInternData->stcCanNotificationCb.pfnCanStatusInterruptFunction = pstcConfig->pfnStatusCallback;
    pstcCanInternData->stcCanNotificationCb.pfnCanErrorInterruptFunction  = pstcConfig->pfnErrorCallback;

    // Enable interrupts
    unCTRLR.EIE = 1; // Error interrupt enable
    unCTRLR.SIE = 1; // Status change interrupt enable
    unCTRLR.IE  = 1; // Module interrupt enable   
    
    // Set automatic retransmission field.
    if (pstcConfig->bDisableAutomaticRetransmission == TRUE)
    {
      unCTRLR.DAR = 1; // Disable automatic retransmission 
    }
   
    pstcCan->CTRLR_f = unCTRLR;
    
    Can_InitInterrupt(pstcCan);
    
    // At last leave init phase ...
    pstcCan->CTRLR_f.INIT = 0 ; 

    return Ok;
} // Can_Init

/**
 *****************************************************************************
 ** \brief Deinitializes the CAN module.
 ** Any pending transmission or receiption will be aborted and all CAN related
 ** registers are reset to their default values.
 **
 ** \param [in] pstcCan           Pointer to register area of a CAN unit.
 **
 ** \retval Ok                    CAN module has been successfully deinitialized
 ** \retval ErrorInvalidParameter If one of the following conditions are met:
 **   - pstcCan == NULL
 **   - pstcCanInternData == NULL (invalid or disabled CAN unit
 **                               (PDL_PERIPHERAL_ENABLE_CAN))
 *****************************************************************************/
en_result_t Can_DeInit( volatile stc_cann_t* pstcCan )
{
    stc_can_intern_data_t* pstcCanInternData; // Pointer to internal data
    uint8_t u8MessageNumber;
    
    // Check for NULL pointer
    if ( pstcCan == NULL )
    {
        return ErrorInvalidParameter;
    }

    // Get pointer to internal data structure
    pstcCanInternData = CanGetInternDataPtr( pstcCan ) ;

    // Check for NULL
    if (pstcCanInternData == NULL)
    {
        return ErrorInvalidParameter;
    }

    Can_DeInitInterrupt(pstcCan);

    // Reset CAN control register.
    pstcCan->CTRLR = 0x0000;

    pstcCan->CTRLR_f.INIT = 1; // Enter INIT phase
    pstcCan->CTRLR_f.CCE  = 1; // Enable access to bit timing register.

    // Reset CAN bit timing register (0x2301).
    pstcCan->BTR_f.BRP   = 1;
    pstcCan->BTR_f.SJW   = 0;
    pstcCan->BTR_f.TSEG1 = 3;
    pstcCan->BTR_f.TSEG2 = 2;

    // Reset CAN baud rate prescaler extension register.
    pstcCan->BRPER = 0x00;

    // Disable access to bit timing register.
    pstcCan->CTRLR_f.CCE = 0;

    // Reset CAN status register.
    pstcCan->STATR = 0x00;

    // Reset CAN test register.
    pstcCan->TESTR = 0x00;

    // Reset CAN IFx command mask register.
    pstcCan->IF1CMSK = 0x00;
    pstcCan->IF2CMSK = 0x00;

    // Reset CAN IFx  mask register.
    pstcCan->IF1MSK = UINT32_MAX ;
    pstcCan->IF2MSK = UINT32_MAX ;

    // Reset CAN IFx arbitration register.
    pstcCan->IF1ARB = 0x00;
    pstcCan->IF2ARB = 0x00;
    
    // Reset CAN IFx message control register.
    pstcCan->IF1MCTR = 0x00;
    pstcCan->IF2MCTR = 0x00;

    // Reset CAN IFx data A and Data B register.
    pstcCan->IF1DTA1_L = 0x00;
    pstcCan->IF1DTA2_L = 0x00;
    pstcCan->IF1DTB1_L = 0x00;
    pstcCan->IF1DTB2_L = 0x00;
    pstcCan->IF2DTA1_L = 0x00;
    pstcCan->IF2DTA2_L = 0x00;
    pstcCan->IF2DTB1_L = 0x00;
    pstcCan->IF2DTB2_L = 0x00;
    
    // Reset CAN prescaler
    CANPRE = 0x00;

    // Set each message buffer number to update message buffer with reset message valid bit.
    for (u8MessageNumber = 0; u8MessageNumber < CAN_MESSAGE_BUFFER_COUNT; u8MessageNumber++)
    {
        // Reset transmit callback function.
        pstcCanInternData->stcCanInterruptHandling[u8MessageNumber].pstcMsgBuffer = NULL;
        pstcCanInternData->stcCanInterruptHandling[u8MessageNumber].pfnCanTxInterruptFunction = NULL;
        pstcCanInternData->stcCanInterruptHandling[u8MessageNumber].pfnCanRxInterruptFunction = NULL;
    }

    // Reset notification callback functions
    pstcCanInternData->stcCanNotificationCb.pfnCanStatusInterruptFunction = NULL;
    pstcCanInternData->stcCanNotificationCb.pfnCanErrorInterruptFunction  = NULL;

    return Ok;
} // Can_DeInit

/**
 *****************************************************************************
 ** \brief Check if the given Message Buffer can be updated.
 **
 ** Updating is not possible if a transmission is pending, except we are in 
 ** remote transmission mode.
 **
 ** \param [in] pstcCan              Pointer to register area of a CAN unit.
 ** \param [in] u8MsgBuf             Message buffer index (0 .. CAN_MESSAGE_BUFFER_COUNT - 1)
 **                                  
 ** \retval Ok                       Message buffer can be updated
 ** \retval ErrorOperationInProgress A transmission is pending
 *****************************************************************************/
static en_result_t CanIsMbUpdatePossible( volatile stc_cann_t* pstcCan,
                                          uint8_t u8MsgBuf
                                        )
{
    uint16_t u16CmpMask ;

    // No parameter checking is necessary here because this has been done
    // by the caller already.
    
    // Check if a transmission is pending
    // Get bit mask
    u16CmpMask   =  1 ;
    u16CmpMask <<= (u8MsgBuf % 16) ;


    // If Remote-Frames are active ...
    if( pstcCan->IF1MCTR_f.RMTEN == 1 )
    {
        return Ok ; // Update of Message Buffer is safe now
    }
    
    switch ( u8MsgBuf >> 3 ) // Which register to use (each reg holds 16 flags)
    {
        case 0:
            if ( pstcCan->TREQR1 & u16CmpMask )
            {
                return ErrorOperationInProgress ;
            }
            break ;

        case 1:
            if ( pstcCan->TREQR2 & u16CmpMask )
            {
                return ErrorOperationInProgress ;
            }
            break ;

        default: // hope this will never see the daylight
            return ErrorInvalidParameter ;
    }
    
    return Ok ; // Update of Message Buffer is safe now
} // CanIsMbUpdatePossible

/**
 *****************************************************************************
 ** \brief Configures a message buffer for transmission.
 ** Setting of new values is not possible if a transmission is pending, except
 ** remote transmission mode. The callback function pfnCallback can be NULL, but
 ** there will be no notification of a successful transmission. This function
 ** has to be called at least once before function
 ** Can_UpdateAndTransmitMsgBuffer() can be used with the same message buffer
 ** index.
 **
 ** With the parameter stc_can_msg_id_t#pstcMsgId of type stc_can_msg_id_t
 ** the API can set the identifier (11 bit or 29 bit length) of the CAN transmit
 ** message. It is possible to set a callback function to get notified when a
 ** transmission is successfully finished.
 ** Can_SetTransmitMsgBuffer() must be called before calling
 ** Can_UpdateAndTransmitMsgBuffer(). Update or setting of new values of
 ** function Can_SetTransmitMsgBuffer() or Can_UpdateAndTransmitMsgBuffer() is
 ** not possible if a transmission is pending or ongoing, except remote
 ** transmission mode is used.
 **
 ** \param [in] pstcCan            Pointer to register area of a CAN unit.
 ** \param [in] u8MsgBuf           Message buffer index (0 .. CAN_MESSAGE_BUFFER_COUNT - 1)
 ** \param [in] pstcMsgId          CAN message identifier.
 ** \param [in] pfnCallback        Callback function to be called after successful transmission, can be NULL.
 **
 ** \retval Ok                     Message buffer has been successfully initialized.
 ** \retval ErrorInvalidParameter  If one of the following conditions are met:
 **   - pstcCan == NULL
 **   - pstcCanInternData == NULL (invalid or disabled CAN unit (PDL_PERIPHERAL_ENABLE_CAN))
 **   - pstcMsgId == NULL
 **   - u8MsgBuf out of range
 ** \retval ErrorOperationInProgress  If following condition is met:
 **   - A transmission is pending (either wait or call Can_ResetMsgBuffer() ).
 *****************************************************************************/
en_result_t Can_SetTransmitMsgBuffer( volatile stc_cann_t* pstcCan,
                                      uint8_t u8MsgBuf,
                                      const stc_can_msg_id_t* pstcMsgId,
                                      can_tx_msg_func_ptr_t pfnCallback
                                    )
{
    stc_can_intern_data_t* pstcCanInternData ; // Pointer to internal data

    // Check for NULL pointer
    if ( pstcCan   == NULL ||
         pstcMsgId == NULL
       )
    {
        return ErrorInvalidParameter;
    }

    // Check for message buffer index in range
    if ( u8MsgBuf >= CAN_MESSAGE_BUFFER_COUNT )
    {
        return ErrorInvalidParameter;
    }

    // Get pointer to internal data structure
    pstcCanInternData = CanGetInternDataPtr( pstcCan ) ;

    // Check for NULL
    if (pstcCanInternData == NULL)
    {
      return ErrorInvalidParameter;
    }

    if ( CanIsMbUpdatePossible( pstcCan, u8MsgBuf ) != Ok )
    {
        return ErrorOperationInProgress ;
    }        

    // Set message valid bit to 0, afterwards message objects can be updated...
    pstcCan->u32IF1ARB.MSGVAL = 0 ;

    // Write command mask register for updating arbitration register.
    pstcCan->IF1CMSK_f.WRRD   = CAN_MSG_DIR_TX;
    pstcCan->IF1CMSK_f.ARB    = 1; // Write Arbitraion

    // Set message buffer number to update message buffer.
    pstcCan->IF1CREQ = u8MsgBuf + 1 ;

    // Set transmit callback function.
    pstcCanInternData->stcCanInterruptHandling[u8MsgBuf].pfnCanTxInterruptFunction = pfnCallback;

    // Write configuration to IFx arbitration1/2
    if (pstcMsgId->bExtended != CAN_MSG_ID_EXTENDED)
    {
        // Just 11 bit identifier.
        pstcCan->u32IF1ARB.ID = pstcMsgId->u32Identifier << (2 + 16) ;
        pstcCan->u32IF1ARB.XTD = 0 ;
    }
    else
    {
        // Write 29 bit identifier.
        pstcCan->u32IF1ARB.ID = pstcMsgId->u32Identifier ;
        pstcCan->u32IF1ARB.XTD = 1 ;
    }

    // We can NOT use a combined register write here because this would destroy
    // the u29ID bit field!
    // Set message direction to transmit.
    pstcCan->u32IF1ARB.DIR = CAN_MSG_DIR_TX ;
    // Set message buffer valid.
    pstcCan->u32IF1ARB.MSGVAL = 1 ;


    // ---------

    // Set message buffer number to update message buffer.
    pstcCan->IF1CREQ = u8MsgBuf + 1 ;

    return Ok;
} // Can_SetTransmitMsgBuffer

/**
 *****************************************************************************
 ** \brief Updates the message data of a message buffer and/or start transmission.
 ** Transmits the message immediately (immediate transmission mode) or on
 ** reception of a matching remote frame (remote transmission mode).
 ** Function Can_SetTransmitMsgBuffer() must be called before setup the
 ** identifier and enable this message buffer.
 **
 ** \param [in] pstcCan               Pointer to register area of a CAN unit.
 ** \param [in] u8MsgBuf              Message buffer index (0 .. CAN_MESSAGE_BUFFER_COUNT - 1)
 ** \param [in] pstcMsgData           CAN message data
 ** \param [in] enTxMode              Transmission mode (immediately or remote)
 **
 ** \retval Ok                        Message buffer has been successfully updated.
 ** \retval ErrorInvalidParameter     If one of the following conditions are met:
 **   - pstcCan == NULL
 **   - pstcCanInternData == NULL (invalid or disabled CAN unit (PDl_PERIPHERAL_ENABLE_CAN))
 **   - pstcMsgData == NULL
 **   - u8MsgBuf out of range
 ** \retval ErrorUninitialized        If Can_SetTransmitMsgBuffer() was not 
 **                                    called before.
 ** \retval ErrorOperationInProgress  If a transmission is pending (either wait 
 **                                    or call Can_ResetMsgBuffer() ).
 *****************************************************************************/
en_result_t Can_UpdateAndTransmitMsgBuffer( volatile stc_cann_t* pstcCan,
                                            uint8_t u8MsgBuf,
                                            const stc_can_msg_data_t* pstcMsgData,
                                            en_can_tx_mode_t enTxMode
                                          )
{
    stc_can_intern_data_t* pstcCanInternData; // Pointer to internal data
    uint16_t u16Data;
    uint8_t  u8DataLengthCode;

    // Check for NULL pointers
    if ( pstcCan     == NULL ||
         pstcMsgData == NULL
       )
    {
        return ErrorInvalidParameter;
    }

    // Check for message buffer index in range
    if ( u8MsgBuf >= CAN_MESSAGE_BUFFER_COUNT )
    {
        return ErrorInvalidParameter;
    }

    // Get pointer to internal data structure
    pstcCanInternData = CanGetInternDataPtr( pstcCan ) ;

    // Check for NULL
    if ( pstcCanInternData == NULL )
    {
        return ErrorInvalidParameter;
    }

    if ( CanIsMbUpdatePossible( pstcCan, u8MsgBuf ) != Ok )
    {
        return ErrorOperationInProgress ;
    }        

    // Write command mask register to read arbitration
    pstcCan->IF1CMSK_f.WRRD     = CAN_MSG_DIR_RX;
    pstcCan->IF1CMSK_f.ARB      = 1;  // Write Arbitration

    // Set message buffer number to update message buffer.
    pstcCan->IF1CREQ = u8MsgBuf + 1 ;

    // Check if setup was done previously.
    if ( pstcCan->u32IF1ARB.MSGVAL == 0 )
    {
        return ErrorUninitialized;
    }

    u8DataLengthCode = (pstcMsgData->u8DataLengthCode > 8)
                     ? 8
                     : pstcMsgData->u8DataLengthCode ;


    // Set message valid bit to 0, afterwards message objects can be updated...
    pstcCan->u32IF1ARB.MSGVAL = 0 ;

    // Write command mask register to update arbitration register.
    pstcCan->IF1CMSK_f.WRRD = CAN_MSG_DIR_TX;
    pstcCan->IF1CMSK_f.ARB  = 1; // Write Arbitration

    // Set message buffer number to update message buffer.
    pstcCan->IF1CREQ = u8MsgBuf + 1 ;

    // Set message control register.
    pstcCan->IF1MCTR_f.NEWDAT = 1; // New data has been written
    
    if (TRUE == pstcCanInternData->bEobZero)
    {
      pstcCan->IF1MCTR_f.EOB = 0;  // Use as FIFO message buffer
    }
    else
    {
      pstcCan->IF1MCTR_f.EOB = 1;  // Use as single message buffer
    }
    
    pstcCan->IF1MCTR_f.TXIE   = 1; // Transmit interrrupt enable

    pstcCan->IF1MCTR_f.DLC = u8DataLengthCode;

    if ( enTxMode == CanImmediateTransmit )
    {
        pstcCan->IF1MCTR_f.RMTEN = 0 ; // Remote frame ignored
    }
    else
    {
         pstcCan->IF1MCTR_f.RMTEN = 1 ; // TXRQST is set on recv of a remote frame
    }

    u16Data = 0;

    // Update of data bytes.
    switch (u8DataLengthCode)
    {
        case 8:
            u16Data = pstcMsgData->au8Data[7] << 8;
            // ... fall through to process remaining bytes
        case 7:
            u16Data |= pstcMsgData->au8Data[6] ;
            pstcCan->IF1DTB2_L = u16Data;
            // ... fall through to process remaining bytes
        case 6:
            u16Data = pstcMsgData->au8Data[5] << 8;
            // ... fall through to process remaining bytes
        case 5:
            u16Data |= pstcMsgData->au8Data[4];
            pstcCan->IF1DTB1_L = u16Data;
            // ... fall through to process remaining bytes
        case 4:
            u16Data = pstcMsgData->au8Data[3] << 8;
            // ... fall through to process remaining bytes
        case 3:
            u16Data |= pstcMsgData->au8Data[2];
            pstcCan->IF1DTA2_L = u16Data;
            // ... fall through to process remaining bytes
        case 2:
            u16Data = pstcMsgData->au8Data[1] << 8;
            // ... fall through to process remaining bytes
        case 1:
            u16Data |= pstcMsgData->au8Data[0];
            pstcCan->IF1DTA1_L = u16Data ;
            break ;
        default:
            break ;
    }

    // Set message valid bit to 1, afterwards message object is ready to transmit.
    pstcCan->u32IF1ARB.MSGVAL = 1 ;

    // Write command mask register to update arbitration, control,
    // transmit request and data register.
    pstcCan->IF1CMSK_f.WRRD    = CAN_MSG_DIR_TX;
    pstcCan->IF1CMSK_f.ARB     = 1; // Write Arbitration 
    pstcCan->IF1CMSK_f.CONTROL = 1; // Write Control    
    pstcCan->IF1CMSK_f.NEWDAT  = 1; // Write TXREQ/NEWDAT
    pstcCan->IF1CMSK_f.DATAA   = 1; // Write Data A
    pstcCan->IF1CMSK_f.DATAB   = 1; // Write Data B

    // Set message buffer number to update message buffer
    // This will send the msg out ...
    pstcCan->IF1CREQ = u8MsgBuf + 1 ;

    return Ok;
} // Can_UpdateAndTransmitMsgBuffer

/**
 *****************************************************************************
 ** \brief Updates the message data of a message buffer and/or start
 ** with EOB = 0 (Msg Buffer as FIFO) transmission.
 ** The function calls Can_UpdateAndTransmitMsgBuffer()
 **
 ** \param [in] pstcCan               Pointer to register area of a CAN unit.
 ** \param [in] u8MsgBuf              Message buffer index (0 .. CAN_MESSAGE_BUFFER_COUNT - 1)
 ** \param [in] pstcMsgData           CAN message data
 ** \param [in] enTxMode              Transmission mode (immediately or remote)
 **
 ** \retval Ok                        Message buffer has been successfully updated.
 ** \retval ErrorInvalidParameter     If one of the following conditions are met:
 **   - pstcCan == NULL
 **   - pstcCanInternData == NULL (invalid or disabled CAN unit (PDL_PERIPHERAL_ENABLE_CAN))
 **   - pstcMsgData == NULL
 **   - u8MsgBuf out of range
 ** \retval ErrorUninitialized        If Can_SetTransmitMsgBuffer() was not 
 **                                    called before.
 ** \retval ErrorOperationInProgress  If a transmission is pending (either wait 
 **                                    or call Can_ResetMsgBuffer() ).
 *****************************************************************************/
en_result_t Can_UpdateAndTransmitFifoMsgBuffer( volatile stc_cann_t* pstcCan,
                                                uint8_t u8MsgBuf,
                                                const stc_can_msg_data_t* pstcMsgData,
                                                en_can_tx_mode_t enTxMode
                                              )
{
    stc_can_intern_data_t* pstcCanInternData; // Pointer to internal data
    en_result_t enFuncResult;
    
    // Check for NULL pointers
    if ( pstcCan     == NULL ||
         pstcMsgData == NULL
       )
    {
        return ErrorInvalidParameter;
    }
    
    // Get pointer to internal data structure
    pstcCanInternData = CanGetInternDataPtr( pstcCan ) ;
  
    // Check for NULL
    if ( pstcCanInternData == NULL )
    {
      return ErrorInvalidParameter;
    }
    
    // Preapare EOB = 0
    pstcCanInternData->bEobZero = TRUE;
      
    enFuncResult = Can_UpdateAndTransmitMsgBuffer(pstcCan, u8MsgBuf, pstcMsgData, enTxMode);
    
    // Reset EOB to '1'
    pstcCanInternData->bEobZero = FALSE;
    
    return enFuncResult;
} // Can_UpdateAndTransmitFifoMsgBuffer

/**
 *****************************************************************************
 ** \brief Configure a message buffer for reception.
 ** Configure and enable a message buffer for reception. The acceptance filter is set
 ** by pstcMsgBuffer->stcIdentifier and u32MsgIdMask. Each '0' bit in
 ** u32MsgIdMask masks the corresponding bit of the received message ID before
 ** comparing it to the configured identifier (set by
 ** pstcMsgBuffer->stcIdentifier). This allows receiving messages with
 ** different identifier. Setting all bits of u32MsgIdMask to '1' will only
 ** accept messages that match the configured identifier.
 ** If extended identifier is used, the
 ** u32MsgIdMask will also be interpreted as extended mask identifier. If 11 bit
 ** identifier is used, than u32MsgIdMask will be used as 11 bit mask identifier.
 ** The application must provide a message buffer object (pstcMsgBuffer) to be
 ** filled with received data.
 ** After reception of a message that passed the acceptance filter,
 ** the message's identifier, data and data length is copied into the provided
 ** message buffer and its bNew flag is set to TRUE.
 ** The message buffer
 ** has to be kept valid until this message buffer is reset (Can_ResetMsgBuffer()).
 **
 ** A mask identifier has to be set when calling Can_SetReceiveMsgBuffer(), the length
 ** for the mask identifier will be the same like the one used in pstcMsgBuffer (11-bit or
 ** 29-bit identifier mask). The extended identification mask bit and the direction mask bit are
 ** always set to 1.
 **
 ** The API has
 ** to check the bNew flag of parameter pstcMsgBuffer to get information about if a message
 ** has already been received or not. If a new message has been received while bNew flag
 ** is set (the last received message was not read out by API so far) than the bOverflow flag
 ** will be set. So, if callback function is not used, the API has to reset the bNew flag when
 ** the received message is read out (also the bOverflow flag has to be reset) and furthermore.
 **
 ** \param [in] pstcCan           Pointer to register area of a CAN unit.
 ** \param [in] u8MsgBuf          Message buffer index (0 .. CAN_MESSAGE_BUFFER_COUNT - 1)
 ** \param [in,out] pstcMsgBuffer CAN message object which defines identifier for acceptance filter.
 ** \param [in] u32MsgIdMask      Mask for identifier acceptance filter and later receives the received message (all '1' disables masking).
 ** \param [in] pfnCallback       Callback function which is called when new CAN message was received and accepted by this message buffer.
 **
 ** \retval Ok                    Message buffer for reception has been succesfully set.
 ** \retval ErrorInvalidParameter If one of the following conditions are met:
 **   - pstcCan == NULL
 **   - pstcCanInternData == NULL (invalid or disabled CAN unit (PDL_PERIPHERAL_ENABLE_CAN))
 **   - pstcMsgBuffer == NULL
 **   - u8MsgBuf out of range
 *****************************************************************************/
en_result_t Can_SetReceiveMsgBuffer( volatile stc_cann_t* pstcCan,
                                     uint8_t u8MsgBuf,
                                     stc_can_msg_t* pstcMsgBuffer,
                                     uint32_t u32MsgIdMask,
                                     can_rx_msg_func_ptr_t pfnCallback
                                   )
{
    stc_can_intern_data_t*  pstcCanInternData; // Pointer to internal data
    stc_can_ifncmsk_t       unIF1CMSK;
    stc_can_ifnarb_t        unIF1ARB;
    
    PDL_ZERO_STRUCT(unIF1CMSK);
    PDL_ZERO_STRUCT(unIF1ARB);

    // Check for NULL pointers
    if ( pstcCan       == NULL ||
         pstcMsgBuffer == NULL
       )
    {
        return ErrorInvalidParameter;
    }

    // Check for message buffer index in range
    if ( u8MsgBuf >= CAN_MESSAGE_BUFFER_COUNT )
    {
        return ErrorInvalidParameter;
    }

    // Get pointer to internal data structure
    pstcCanInternData = CanGetInternDataPtr( pstcCan ) ;

    // Check for NULL
    if ( pstcCanInternData == NULL )
    {
      return ErrorInvalidParameter;
    }

    // Disable message buffer at first (MSGVAL = 0) (otherwise a pending
    // reception on this MB could cause an interrupt, using inconsistent data)
    pstcCan->u32IF1ARB.MSGVAL = 0 ;

    // Set receive callback function and message buffer.
    pstcCanInternData->stcCanInterruptHandling[u8MsgBuf].pfnCanRxInterruptFunction = pfnCallback;
    pstcCanInternData->stcCanInterruptHandling[u8MsgBuf].pstcMsgBuffer = pstcMsgBuffer;

    // Configure mask register.
    pstcCan->IF1MSK = 0 ;

    // Set direction to read and message valid bit to 1.
    unIF1ARB.DIR    = CAN_MSG_DIR_RX; // Set direction read
    unIF1ARB.MSGVAL = 1; // Set message valid bit to 1

    // Update arbitration register and mask identifier.
    if ( pstcMsgBuffer->stcIdentifier.bExtended != CAN_MSG_ID_EXTENDED )
    {
        // Just 11-bit identifier.
        unIF1ARB.ID  = ((pstcMsgBuffer->stcIdentifier.u32Identifier) << 18) ;
        unIF1ARB.XTD = 0; // Clear extended bit
        pstcCan->IF1MSK_f.MSK  = (u32MsgIdMask << 18) ;
    }
    else
    {
        // Write 29-bit identifier.
        unIF1ARB.ID  = pstcMsgBuffer->stcIdentifier.u32Identifier ;
        unIF1ARB.XTD = 1 ; // Set extended bit
        pstcCan->IF1MSK_f.MSK  = u32MsgIdMask ;
    }

    // Setup IF1ARB in hardware
    pstcCan->u32IF1ARB = unIF1ARB;
    pstcCan->IF1MSK_f.MXTD = 1;
    
    // Update message control register, enable receive interrupt and mask flag.
    pstcCan->IF1MCTR = 0x0000;
    pstcCan->IF1MCTR_f.RXIE  = 1; // Receive interrupt enable
    pstcCan->IF1MCTR_f.UMASK = 1; // Use acceptance mask
    
    if (TRUE == pstcCanInternData->bEobZero)
    {
      pstcCan->IF1MCTR_f.EOB = 0;  // Use as FIFO message buffer
    }
    else
    {
      pstcCan->IF1MCTR_f.EOB = 1;  // Use as single message buffer
    }

    // Write command mask register
    unIF1CMSK.WRRD    = CAN_MSG_DIR_TX; // Write to message RAM
    unIF1CMSK.MASK    = 1; // Update mask
    unIF1CMSK.ARB     = 1; // Update arbitration
    unIF1CMSK.CONTROL = 1; // Update control
    pstcCan->IF1CMSK_f = unIF1CMSK;

    // Set message buffer number to update message buffer.
    pstcCan->IF1CREQ = u8MsgBuf + 1 ;

    pstcMsgBuffer->bNew      = FALSE ; // Preset to default
    pstcMsgBuffer->bOverflow = FALSE ; // Preset to default

    return Ok;
} // Can_SetReceiveMsgBuffer

/**
 *****************************************************************************
 ** \brief Configure a message buffer for reception with EOB = 0 (FIFO mode).
 ** This function calls Can_SetReceiveMsgBuffer()
 **
 ** \param [in] pstcCan           Pointer to register area of a CAN unit.
 ** \param [in] u8MsgBuf          Message buffer index (0 .. CAN_MESSAGE_BUFFER_COUNT - 1)
 ** \param [in,out] pstcMsgBuffer CAN message object which defines identifier for acceptance filter.
 ** \param [in] u32MsgIdMask      Mask for identifier acceptance filter and later receives the received message (all '1' disables masking).
 ** \param [in] pfnCallback       Callback function which is called when new CAN message was received and accepted by this message buffer.
 **
 ** \retval Ok                    Message buffer for reception has been succesfully set.
 ** \retval ErrorInvalidParameter If one of the following conditions are met:
 **   - pstcCan == NULL
 **   - pstcCanInternData == NULL (invalid or disabled CAN unit (PDL_PERIPHERAL_ENABLE_CAN))
 **   - pstcMsgBuffer == NULL
 **   - u8MsgBuf out of range
 *****************************************************************************/
en_result_t Can_SetReceiveFifoMsgBuffer( volatile stc_cann_t* pstcCan,
                                         uint8_t u8MsgBuf,
                                         stc_can_msg_t* pstcMsgBuffer,
                                         uint32_t u32MsgIdMask,
                                         can_rx_msg_func_ptr_t pfnCallback
                                       )
{
    stc_can_intern_data_t*  pstcCanInternData; // Pointer to internal data
    
    en_result_t enFuncResult;
    
    // Check for NULL pointers
    if ( pstcCan       == NULL ||
         pstcMsgBuffer == NULL
       )
    {
        return ErrorInvalidParameter;
    }
    
    // Get pointer to internal data structure
    pstcCanInternData = CanGetInternDataPtr( pstcCan ) ;
  
    // Check for NULL
    if ( pstcCanInternData == NULL )
    {
      return ErrorInvalidParameter;
    }
    
    // Preapare EOB = 0
    pstcCanInternData->bEobZero = TRUE;
    
    enFuncResult = Can_SetReceiveMsgBuffer(pstcCan, u8MsgBuf, pstcMsgBuffer, u32MsgIdMask, pfnCallback);
    
    // Reset EOB to '1'
    pstcCanInternData->bEobZero = FALSE;
    
    return enFuncResult;
} // Can_SetReceiveFifoMsgBuffer

/**
 *****************************************************************************
 ** \brief Stop any message buffer operation i.e. disable it
 **
 ** - stop pending transmission (reset TXRQST and NEWDAT flag):
 ** - stop reception operation (reset MSGVAL flag)
 ** - reset RXIE and TXIE
 ** - clear pointers to external buffers and callback functions
 **
 ** \param [in] pstcCan               Pointer to register area of a CAN unit.
 ** \param [in] u8MsgBuf              Message buffer index (0 .. CAN_MESSAGE_BUFFER_COUNT - 1)
 **
 ** \retval Ok                        Message buffer operations have been stoped
 ** \retval ErrorInvalidParameter     If one of the following conditions are met:
 **   - pstcCan == NULL
 **   - pstcCanInternData == NULL (invalid or disabled CAN unit (PDL_PERIPHERAL_ENABLE_CAN))
 **   - u8MsgBuf out of range
 *****************************************************************************/
en_result_t Can_ResetMsgBuffer( volatile stc_cann_t* pstcCan, uint8_t u8MsgBuf )
{
    stc_can_intern_data_t* pstcCanInternData ; // Pointer to internal data

    // Check for NULL pointer
    if ( pstcCan == NULL )
    {
        return ErrorInvalidParameter ;
    }

    // Check for message buffer number in range
    if ( u8MsgBuf >= CAN_MESSAGE_BUFFER_COUNT )
    {
        return ErrorInvalidParameter ;
    }

    // Get pointer to internal data structure
    pstcCanInternData = CanGetInternDataPtr( pstcCan ) ;

    // Check for NULL
    if ( pstcCanInternData == NULL )
    {
      return ErrorInvalidParameter ;
    }

    // Reset message control register.
    pstcCan->IF1MCTR = 0 ;

    // Reset message valid bit
    pstcCan->u32IF1ARB.MSGVAL  = 0 ;

    // Reset msg buffer and callback functions
    pstcCanInternData->stcCanInterruptHandling[u8MsgBuf].pstcMsgBuffer = NULL ;
    pstcCanInternData->stcCanInterruptHandling[u8MsgBuf].pfnCanTxInterruptFunction = NULL ;
    pstcCanInternData->stcCanInterruptHandling[u8MsgBuf].pfnCanRxInterruptFunction = NULL ;

    return Ok ;
} // Can_ResetMsgBuffer

//@} // CanGroup

#endif // #if (defined(PDL_PERIPHERAL_CAN_ACTIVE))

/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
