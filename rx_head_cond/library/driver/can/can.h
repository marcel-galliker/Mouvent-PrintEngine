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
/** \file can.h
 **
 ** Headerfile for CAN functions
 **  
 ** History:
 **   - 2013-08-21  1.0  MWi  First version.
 **
 ******************************************************************************/

#ifndef __CAN_H__
#define __CAN_H__

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "pdl.h"

#if (defined(PDL_PERIPHERAL_CAN_ACTIVE))

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif
  
/**
 ******************************************************************************
 ** \defgroup CanGroup Controller Area Network (CAN)
 **
 ** Provided functions of CAN module:
 ** - Can_Init() (Initialisation of CAN module)
 ** - Can_DeInit() (Reset of all CAN related register to default value)
 ** - Can_SetTransmitMsgBuffer() (Used to initiate and configure a message buffer for a CAN message trasnmission)
 ** - Can_UpdateAndTransmitMsgBuffer() (Used to update the data in a message buffer and to transmit a message)
 ** - Can_SetReceiveMsgBuffer() (Used to initiate and setup a message buffer for reveiving data)
 **
 ** How to use the CAN module:
 ** First, to initialize a CAN module, Can_Init() must be called. 
 ** The callback functions are optional, but recommended, otherwise there is no 
 ** report to the API in case of any error. 
 ** Can_DeInit() has to be used if any of the settings from Can_Init() have
 ** to be changed (use Can_DeInit() and afterwards Can_Init()).
 ** Can_DeInit() is used to completely disable the CAN module. 
 ** With Can_DeInit() all CAN related register values are reset to their default
 ** values. Also any pending or ongoing transmission or reception will be 
 ** aborted. 
 ** Each CAN module has #CAN_MESSAGE_BUFFER_COUNT number of message buffers 
 ** which can be used either for reception or transmission of CAN messages.
 ** Each message buffer for transmission has to be set up first by
 ** calling Can_SetTransmitMsgBuffer().
 ** For receiving CAN messages the function Can_SetReceiveMsgBuffer() has to be
 ** used. 
 ** It is possible to set a callback function which will be notified whenever a
 ** message has been received. 
 **
 ** \note The numbers of the message buffers used in this driver are
 ** indexed from 0 to 31 although the 'physical addresses' of these buffers are
 ** indexed from 1 to 32!
 ******************************************************************************/
//@{
  
/******************************************************************************
 * Global definitions
 ******************************************************************************/
 
#define CAN0       (*((volatile stc_cann_t *) FM4_CAN0_BASE))
#define CAN1       (*((volatile stc_cann_t *) FM4_CAN1_BASE))
#define CANPRE     (*((volatile uint8_t *) FM4_CANPRES_BASE))

/// Size of data bytes in a receive or transmit operation (usually 8)
#define CAN_MESSAGE_DATA_BUFFER_SIZE     8

/// Number of possible message buffer for receive and transmit messages (usually 32)
#define CAN_MESSAGE_BUFFER_COUNT        32

/// Lower limit for TSEG1 in structure stc_can_bitrate_t
#define CAN_BITRATE_TSEG1_MIN            2
/// Upper limit for TSEG1 in structure stc_can_bitrate_t
#define CAN_BITRATE_TSEG1_MAX           16

/// Lower limit for TSEG2 in structure stc_can_bitrate_t
#define CAN_BITRATE_TSEG2_MIN            1
/// Upper limit for TSEG2 in structure stc_can_bitrate_t
#define CAN_BITRATE_TSEG2_MAX            8


/// Lower limit for Sync Jump Width in structure stc_can_bitrate_t
#define CAN_BITRATE_SYNC_JUMP_WIDTH_MIN  1
/// Upper limit for Sync Jump Width in structure stc_can_bitrate_t
#define CAN_BITRATE_SYNC_JUMP_WIDTH_MAX  4

/// Lower limit for Prescaler in structure stc_can_bitrate_t
#define CAN_BITRATE_PRESCALER_MIN        1
/// Upper limit for Prescaler in structure stc_can_bitrate_t
#define CAN_BITRATE_PRESCALER_MAX       64
 
/// Maximum CAN clock frequency (after prescaler)
#define CAN_MAX_CLK                     16000000
  
/******************************************************************************
 * Global type definitions
 ******************************************************************************/

/**
 ******************************************************************************
 ** \brief Redefinition of CTRLR register bitfields
 ******************************************************************************/
typedef struct stc_can_ctrlr
{
  __IO uint16_t INIT       : 1;
  __IO uint16_t IE         : 1;
  __IO uint16_t SIE        : 1;
  __IO uint16_t EIE        : 1;
       uint16_t RESERVED1  : 1;
  __IO uint16_t DAR        : 1;
  __IO uint16_t CCE        : 1;
  __IO uint16_t TEST       : 1;
} stc_can_ctrlr_t;

/**
 ******************************************************************************
 ** \brief Redefinition of STATR register bitfields
 ******************************************************************************/
typedef struct stc_can_statr
{
  __IO uint16_t LEC        : 3;
  __IO uint16_t TXOK       : 1;
  __IO uint16_t RXOK       : 1;
  __IO uint16_t EPASS      : 1;
  __IO uint16_t EWARN      : 1;
  __IO uint16_t BOFF       : 1;
} stc_can_statr_t;

/**
 ******************************************************************************
 ** \brief Redefinition of ERRCNT register bitfields
 ******************************************************************************/
typedef struct stc_can_errcnt
{
  __IO uint16_t TEC0       : 1;
  __IO uint16_t TEC1       : 1;
  __IO uint16_t TEC2       : 1;
  __IO uint16_t TEC3       : 1;
  __IO uint16_t TEC4       : 1;
  __IO uint16_t TEC5       : 1;
  __IO uint16_t TEC6       : 1;
  __IO uint16_t TEC7       : 1;
  __IO uint16_t REC0       : 1;
  __IO uint16_t REC1       : 1;
  __IO uint16_t REC2       : 1;
  __IO uint16_t REC3       : 1;
  __IO uint16_t REC4       : 1;
  __IO uint16_t REC5       : 1;
  __IO uint16_t REC6       : 1;
  __IO uint16_t RP         : 1;
} stc_can_errcnt_t;

/**
 ******************************************************************************
 ** \brief Redefinition of INTR register bitfields
 ******************************************************************************/
typedef struct stc_can_intr
{
  __IO uint16_t INTID0     : 1;
  __IO uint16_t INTID1     : 1;
  __IO uint16_t INTID2     : 1;
  __IO uint16_t INTID3     : 1;
  __IO uint16_t INTID4     : 1;
  __IO uint16_t INTID5     : 1;
  __IO uint16_t INTID6     : 1;
  __IO uint16_t INTID7     : 1;
  __IO uint16_t INTID8     : 1;
  __IO uint16_t INTID9     : 1;
  __IO uint16_t INTID10    : 1;
  __IO uint16_t INTID11    : 1;
  __IO uint16_t INTID12    : 1;
  __IO uint16_t INTID13    : 1;
  __IO uint16_t INTID14    : 1;
  __IO uint16_t INTID15    : 1;
} stc_can_intr_t;

/**
 ******************************************************************************
 ** \brief Redefinition of TESTR register bitfields
 ******************************************************************************/
typedef struct stc_can_testr
{
       uint16_t RESERVED1  : 2;
  __IO uint16_t BASIC      : 1;
  __IO uint16_t SILENT     : 1;
  __IO uint16_t LBACK      : 1;
  __IO uint16_t TX0        : 1;
  __IO uint16_t TX1        : 1;
  __IO uint16_t RX         : 1;
} stc_can_testr_t;

/**
 ******************************************************************************
 ** \brief Redefinition of BRPER register bitfields
 ******************************************************************************/
typedef struct stc_can_brper
{
  __IO uint16_t BRPE0      : 1;
  __IO uint16_t BRPE1      : 1;
  __IO uint16_t BRPE2      : 1;
  __IO uint16_t BRPE3      : 1;
} stc_can_brper_t;

/**
 ******************************************************************************
 ** \brief Redefinition of IFnCREQ register bitfields
 ******************************************************************************/
typedef struct stc_can_ifncreq
{
       uint16_t RESERVED1  : 15;
  __IO uint16_t BUSY       : 1;
} stc_can_ifncreq_t;

/**
 ******************************************************************************
 ** \brief Redefinition of IFnCMSK register bitfields
 ******************************************************************************/
typedef struct stc_can_ifncmsk
{
  __IO uint16_t DATAB      : 1;
  __IO uint16_t DATAA      : 1;
  __IO uint16_t NEWDAT     : 1;
  __IO uint16_t CIP        : 1;
  __IO uint16_t CONTROL    : 1;
  __IO uint16_t ARB        : 1;
  __IO uint16_t MASK       : 1;
  __IO uint16_t WRRD       : 1;
} stc_can_ifncmsk_t;

/**
 ******************************************************************************
 ** \brief Redefinition of IFnMSK register bitfields
 ******************************************************************************/
typedef struct stc_can_ifnmsk
{
  __IO uint32_t MSK        : 29;
       uint32_t RESERVED1  : 1;
  __IO uint32_t MDIR       : 1;
  __IO uint32_t MXTD       : 1;
} stc_can_ifnmsk_t;

/**
 ******************************************************************************
 ** \brief Redefinition of IFnMSK2 register bitfields
 ******************************************************************************/
typedef struct stc_can_ifnmsk2
{
       uint16_t RESERVED1  : 14;
  __IO uint16_t MDIR       : 1;
  __IO uint16_t MXTD       : 1;
} stc_can_ifnmsk2_t;

/**
 ******************************************************************************
 ** \brief Redefinition of IFnARB register bitfields
 ******************************************************************************/
typedef struct stc_can_ifnarb
{
  __IO uint32_t ID     : 29;
  __IO uint32_t DIR    :  1;
  __IO uint32_t XTD    :  1;
  __IO uint32_t MSGVAL :  1;
} stc_can_ifnarb_t;

/**
 ******************************************************************************
 ** \brief Redefinition of IFnARB2 register bitfields
 ******************************************************************************/
typedef struct stc_can_ifnarb2
{
       uint16_t RESERVED1  : 13;
  __IO uint16_t DIR        : 1;
  __IO uint16_t XTD        : 1;
  __IO uint16_t MSGVAL     : 1;
} stc_can_ifnarb2_t;

/**
 ******************************************************************************
 ** \brief Redefinition of IFnMCTR register bitfields
 ******************************************************************************/
typedef struct stc_can_ifnmctr
{
  __IO uint16_t DLC        : 4;
       uint16_t RESERVED1  : 3;
  __IO uint16_t EOB        : 1;
  __IO uint16_t TXRQST     : 1;
  __IO uint16_t RMTEN      : 1;
  __IO uint16_t RXIE       : 1;
  __IO uint16_t TXIE       : 1;
  __IO uint16_t UMASK      : 1;
  __IO uint16_t INTPND     : 1;
  __IO uint16_t MSGLST     : 1;
  __IO uint16_t NEWDAT     : 1;
} stc_can_ifnmctr_t;

/**
 ******************************************************************************
 ** \brief Redefinition of BTR register bitfields
 ******************************************************************************/
typedef struct stc_can_btr
{
  __IO uint16_t BRP        : 6;
  __IO uint16_t SJW        : 2;
  __IO uint16_t TSEG1      : 4;
  __IO uint16_t TSEG2      : 3;
} stc_can_btr_t;

/**
 ******************************************************************************
 ** \brief Redefinition of TREQR register bitfields
 ******************************************************************************/
typedef struct stc_can_treqr
{
  __IO uint32_t TXRQST1    : 1;
  __IO uint32_t TXRQST2    : 1;
  __IO uint32_t TXRQST3    : 1;
  __IO uint32_t TXRQST4    : 1;
  __IO uint32_t TXRQST5    : 1;
  __IO uint32_t TXRQST6    : 1;
  __IO uint32_t TXRQST7    : 1;
  __IO uint32_t TXRQST8    : 1;
  __IO uint32_t TXRQST9    : 1;
  __IO uint32_t TXRQST10   : 1;
  __IO uint32_t TXRQST11   : 1;
  __IO uint32_t TXRQST12   : 1;
  __IO uint32_t TXRQST13   : 1;
  __IO uint32_t TXRQST14   : 1;
  __IO uint32_t TXRQST15   : 1;
  __IO uint32_t TXRQST16   : 1;
  __IO uint32_t TXRQST17   : 1;
  __IO uint32_t TXRQST18   : 1;
  __IO uint32_t TXRQST19   : 1;
  __IO uint32_t TXRQST20   : 1;
  __IO uint32_t TXRQST21   : 1;
  __IO uint32_t TXRQST22   : 1;
  __IO uint32_t TXRQST23   : 1;
  __IO uint32_t TXRQST24   : 1;
  __IO uint32_t TXRQST25   : 1;
  __IO uint32_t TXRQST26   : 1;
  __IO uint32_t TXRQST27   : 1;
  __IO uint32_t TXRQST28   : 1;
  __IO uint32_t TXRQST29   : 1;
  __IO uint32_t TXRQST30   : 1;
  __IO uint32_t TXRQST31   : 1;
  __IO uint32_t TXRQST32   : 1;
} stc_can_treqr_t;

/**
 ******************************************************************************
 ** \brief Redefinition of TREQR1 register bitfields
 ******************************************************************************/
typedef struct stc_can_treqr1
{
  __IO uint16_t TXRQST1    : 1;
  __IO uint16_t TXRQST2    : 1;
  __IO uint16_t TXRQST3    : 1;
  __IO uint16_t TXRQST4    : 1;
  __IO uint16_t TXRQST5    : 1;
  __IO uint16_t TXRQST6    : 1;
  __IO uint16_t TXRQST7    : 1;
  __IO uint16_t TXRQST8    : 1;
  __IO uint16_t TXRQST9    : 1;
  __IO uint16_t TXRQST10   : 1;
  __IO uint16_t TXRQST11   : 1;
  __IO uint16_t TXRQST12   : 1;
  __IO uint16_t TXRQST13   : 1;
  __IO uint16_t TXRQST14   : 1;
  __IO uint16_t TXRQST15   : 1;
  __IO uint16_t TXRQST16   : 1;
} stc_can_treqr1_t;

/**
 ******************************************************************************
 ** \brief Redefinition of TREQR2 register bitfields
 ******************************************************************************/
typedef struct stc_can_treqr2
{
  __IO uint16_t TXRQST17   : 1;
  __IO uint16_t TXRQST18   : 1;
  __IO uint16_t TXRQST19   : 1;
  __IO uint16_t TXRQST20   : 1;
  __IO uint16_t TXRQST21   : 1;
  __IO uint16_t TXRQST22   : 1;
  __IO uint16_t TXRQST23   : 1;
  __IO uint16_t TXRQST24   : 1;
  __IO uint16_t TXRQST25   : 1;
  __IO uint16_t TXRQST26   : 1;
  __IO uint16_t TXRQST27   : 1;
  __IO uint16_t TXRQST28   : 1;
  __IO uint16_t TXRQST29   : 1;
  __IO uint16_t TXRQST30   : 1;
  __IO uint16_t TXRQST31   : 1;
  __IO uint16_t TXRQST32   : 1;
} stc_can_treqr2_t;

/**
 ******************************************************************************
 ** \brief Redefinition of NEWDT register bitfields
 ******************************************************************************/
typedef struct stc_can_newdt
{
  __IO uint32_t NEWDAT1    : 1;
  __IO uint32_t NEWDAT2    : 1;
  __IO uint32_t NEWDAT3    : 1;
  __IO uint32_t NEWDAT4    : 1;
  __IO uint32_t NEWDAT5    : 1;
  __IO uint32_t NEWDAT6    : 1;
  __IO uint32_t NEWDAT7    : 1;
  __IO uint32_t NEWDAT8    : 1;
  __IO uint32_t NEWDAT9    : 1;
  __IO uint32_t NEWDAT10   : 1;
  __IO uint32_t NEWDAT11   : 1;
  __IO uint32_t NEWDAT12   : 1;
  __IO uint32_t NEWDAT13   : 1;
  __IO uint32_t NEWDAT14   : 1;
  __IO uint32_t NEWDAT15   : 1;
  __IO uint32_t NEWDAT16   : 1;
  __IO uint32_t NEWDAT17   : 1;
  __IO uint32_t NEWDAT18   : 1;
  __IO uint32_t NEWDAT19   : 1;
  __IO uint32_t NEWDAT20   : 1;
  __IO uint32_t NEWDAT21   : 1;
  __IO uint32_t NEWDAT22   : 1;
  __IO uint32_t NEWDAT23   : 1;
  __IO uint32_t NEWDAT24   : 1;
  __IO uint32_t NEWDAT25   : 1;
  __IO uint32_t NEWDAT26   : 1;
  __IO uint32_t NEWDAT27   : 1;
  __IO uint32_t NEWDAT28   : 1;
  __IO uint32_t NEWDAT29   : 1;
  __IO uint32_t NEWDAT30   : 1;
  __IO uint32_t NEWDAT31   : 1;
  __IO uint32_t NEWDAT32   : 1;
} stc_can_newdt_t;

/**
 ******************************************************************************
 ** \brief Redefinition of NEWDT1 register bitfields
 ******************************************************************************/
typedef struct stc_can_newdt1
{
  __IO uint16_t NEWDAT1    : 1;
  __IO uint16_t NEWDAT2    : 1;
  __IO uint16_t NEWDAT3    : 1;
  __IO uint16_t NEWDAT4    : 1;
  __IO uint16_t NEWDAT5    : 1;
  __IO uint16_t NEWDAT6    : 1;
  __IO uint16_t NEWDAT7    : 1;
  __IO uint16_t NEWDAT8    : 1;
  __IO uint16_t NEWDAT9    : 1;
  __IO uint16_t NEWDAT10   : 1;
  __IO uint16_t NEWDAT11   : 1;
  __IO uint16_t NEWDAT12   : 1;
  __IO uint16_t NEWDAT13   : 1;
  __IO uint16_t NEWDAT14   : 1;
  __IO uint16_t NEWDAT15   : 1;
  __IO uint16_t NEWDAT16   : 1;
} stc_can_newdt1_t;

/**
 ******************************************************************************
 ** \brief Redefinition of INTPND register bitfields
 ******************************************************************************/
typedef struct stc_can_intpnd
{
  __IO uint32_t INTPND1    : 1;
  __IO uint32_t INTPND2    : 1;
  __IO uint32_t INTPND3    : 1;
  __IO uint32_t INTPND4    : 1;
  __IO uint32_t INTPND5    : 1;
  __IO uint32_t INTPND6    : 1;
  __IO uint32_t INTPND7    : 1;
  __IO uint32_t INTPND8    : 1;
  __IO uint32_t INTPND9    : 1;
  __IO uint32_t INTPND10   : 1;
  __IO uint32_t INTPND11   : 1;
  __IO uint32_t INTPND12   : 1;
  __IO uint32_t INTPND13   : 1;
  __IO uint32_t INTPND14   : 1;
  __IO uint32_t INTPND15   : 1;
  __IO uint32_t INTPND16   : 1;
  __IO uint32_t INTPND17   : 1;
  __IO uint32_t INTPND18   : 1;
  __IO uint32_t INTPND19   : 1;
  __IO uint32_t INTPND20   : 1;
  __IO uint32_t INTPND21   : 1;
  __IO uint32_t INTPND22   : 1;
  __IO uint32_t INTPND23   : 1;
  __IO uint32_t INTPND24   : 1;
  __IO uint32_t INTPND25   : 1;
  __IO uint32_t INTPND26   : 1;
  __IO uint32_t INTPND27   : 1;
  __IO uint32_t INTPND28   : 1;
  __IO uint32_t INTPND29   : 1;
  __IO uint32_t INTPND30   : 1;
  __IO uint32_t INTPND31   : 1;
  __IO uint32_t INTPND32   : 1;
} stc_can_intpnd_t;


/**
 ******************************************************************************
 ** \brief Redefinition of INTPND1 register bitfields
 ******************************************************************************/
typedef struct stc_can_intpnd1
{
  __IO uint16_t INTPND1    : 1;
  __IO uint16_t INTPND2    : 1;
  __IO uint16_t INTPND3    : 1;
  __IO uint16_t INTPND4    : 1;
  __IO uint16_t INTPND5    : 1;
  __IO uint16_t INTPND6    : 1;
  __IO uint16_t INTPND7    : 1;
  __IO uint16_t INTPND8    : 1;
  __IO uint16_t INTPND9    : 1;
  __IO uint16_t INTPND10   : 1;
  __IO uint16_t INTPND11   : 1;
  __IO uint16_t INTPND12   : 1;
  __IO uint16_t INTPND13   : 1;
  __IO uint16_t INTPND14   : 1;
  __IO uint16_t INTPND15   : 1;
  __IO uint16_t INTPND16   : 1;
} stc_can_intpnd1_t;


/**
 ******************************************************************************
 ** \brief Redefinition of INTPND2 register bitfields
 ******************************************************************************/
typedef struct stc_can_intpnd2
{
  __IO uint16_t INTPND17   : 1;
  __IO uint16_t INTPND18   : 1;
  __IO uint16_t INTPND19   : 1;
  __IO uint16_t INTPND20   : 1;
  __IO uint16_t INTPND21   : 1;
  __IO uint16_t INTPND22   : 1;
  __IO uint16_t INTPND23   : 1;
  __IO uint16_t INTPND24   : 1;
  __IO uint16_t INTPND25   : 1;
  __IO uint16_t INTPND26   : 1;
  __IO uint16_t INTPND27   : 1;
  __IO uint16_t INTPND28   : 1;
  __IO uint16_t INTPND29   : 1;
  __IO uint16_t INTPND30   : 1;
  __IO uint16_t INTPND31   : 1;
  __IO uint16_t INTPND32   : 1;
} stc_can_intpnd2_t;

/**
 ******************************************************************************
 ** \brief Redefinition of MSGVAL register bitfields
 ******************************************************************************/
typedef struct stc_can_msgval
{
  __IO uint32_t MSGVAL1    : 1;
  __IO uint32_t MSGVAL2    : 1;
  __IO uint32_t MSGVAL3    : 1;
  __IO uint32_t MSGVAL4    : 1;
  __IO uint32_t MSGVAL5    : 1;
  __IO uint32_t MSGVAL6    : 1;
  __IO uint32_t MSGVAL7    : 1;
  __IO uint32_t MSGVAL8    : 1;
  __IO uint32_t MSGVAL9    : 1;
  __IO uint32_t MSGVAL10   : 1;
  __IO uint32_t MSGVAL11   : 1;
  __IO uint32_t MSGVAL12   : 1;
  __IO uint32_t MSGVAL13   : 1;
  __IO uint32_t MSGVAL14   : 1;
  __IO uint32_t MSGVAL15   : 1;
  __IO uint32_t MSGVAL16   : 1;
  __IO uint32_t MSGVAL17   : 1;
  __IO uint32_t MSGVAL18   : 1;
  __IO uint32_t MSGVAL19   : 1;
  __IO uint32_t MSGVAL20   : 1;
  __IO uint32_t MSGVAL21   : 1;
  __IO uint32_t MSGVAL22   : 1;
  __IO uint32_t MSGVAL23   : 1;
  __IO uint32_t MSGVAL24   : 1;
  __IO uint32_t MSGVAL25   : 1;
  __IO uint32_t MSGVAL26   : 1;
  __IO uint32_t MSGVAL27   : 1;
  __IO uint32_t MSGVAL28   : 1;
  __IO uint32_t MSGVAL29   : 1;
  __IO uint32_t MSGVAL30   : 1;
  __IO uint32_t MSGVAL31   : 1;
  __IO uint32_t MSGVAL32   : 1;
} stc_can_msgval_t;

/**
 ******************************************************************************
 ** \brief Redefinition of MSGVAL1 register bitfields
 ******************************************************************************/
typedef struct stc_can_msgval1
{
  __IO uint16_t MSGVAL1    : 1;
  __IO uint16_t MSGVAL2    : 1;
  __IO uint16_t MSGVAL3    : 1;
  __IO uint16_t MSGVAL4    : 1;
  __IO uint16_t MSGVAL5    : 1;
  __IO uint16_t MSGVAL6    : 1;
  __IO uint16_t MSGVAL7    : 1;
  __IO uint16_t MSGVAL8    : 1;
  __IO uint16_t MSGVAL9    : 1;
  __IO uint16_t MSGVAL10   : 1;
  __IO uint16_t MSGVAL11   : 1;
  __IO uint16_t MSGVAL12   : 1;
  __IO uint16_t MSGVAL13   : 1;
  __IO uint16_t MSGVAL14   : 1;
  __IO uint16_t MSGVAL15   : 1;
  __IO uint16_t MSGVAL16   : 1;
} stc_can_msgval1_t;

/**
 ******************************************************************************
 ** \brief Redefinition of MSGVAL2 register bitfields
 ******************************************************************************/
typedef struct stc_can_msgval2
{
  __IO uint16_t MSGVAL17   : 1;
  __IO uint16_t MSGVAL18   : 1;
  __IO uint16_t MSGVAL19   : 1;
  __IO uint16_t MSGVAL20   : 1;
  __IO uint16_t MSGVAL21   : 1;
  __IO uint16_t MSGVAL22   : 1;
  __IO uint16_t MSGVAL23   : 1;
  __IO uint16_t MSGVAL24   : 1;
  __IO uint16_t MSGVAL25   : 1;
  __IO uint16_t MSGVAL26   : 1;
  __IO uint16_t MSGVAL27   : 1;
  __IO uint16_t MSGVAL28   : 1;
  __IO uint16_t MSGVAL29   : 1;
  __IO uint16_t MSGVAL30   : 1;
  __IO uint16_t MSGVAL31   : 1;
  __IO uint16_t MSGVAL32   : 1;
} stc_can_msgval2_t;

/**
 ******************************************************************************
 ** \brief Redefinition of CAN registers
 ******************************************************************************/
typedef struct stc_cann
{
  union {
    __IO uint16_t CTRLR;
    stc_can_ctrlr_t CTRLR_f;
  };
  union {
    __IO uint16_t STATR;
    stc_can_statr_t STATR_f;
  };
  union {
    __IO uint16_t ERRCNT;
    stc_can_errcnt_t ERRCNT_f;
  };
  union {
    __IO uint16_t BTR;
    stc_can_btr_t BTR_f;
  };
  union {
    __IO uint16_t INTR;
    stc_can_intr_t INTR_f;
  };
  union {
    __IO uint16_t TESTR;
    stc_can_testr_t TESTR_f;
  };
  union {
    __IO uint16_t BRPER;
    stc_can_brper_t BRPER_f;
  };
        uint8_t RESERVED0[2];
  union {
    __IO uint16_t IF1CREQ;
    stc_can_ifncreq_t IF1CREQ_f;
  };
  union {
    __IO uint16_t IF1CMSK;
    stc_can_ifncmsk_t IF1CMSK_f;
  };
  union {
    union {
      __IO uint32_t IF1MSK;
      stc_can_ifnmsk_t IF1MSK_f;
    };
    struct {
      __IO uint16_t IF1MSK1;
      union {
        __IO uint16_t IF1MSK2;
        stc_can_ifnmsk2_t IF1MSK2_f;
      };
    };
  };
  union {
    union {
      __IO uint32_t IF1ARB;
      stc_can_ifnarb_t u32IF1ARB;
    };
    struct {
      __IO uint16_t IF1ARB1;
      union {
        __IO uint16_t IF1ARB2;
        stc_can_ifnarb2_t IF1ARB2_f;
      };
    };
  };
  union {
    __IO uint16_t IF1MCTR;
    stc_can_ifnmctr_t IF1MCTR_f;
  };
        uint8_t RESERVED1[2];
  union {
    __IO uint32_t IF1DTA_L;
    struct {
      __IO uint16_t IF1DTA1_L;
      __IO uint16_t IF1DTA2_L;
    };
  };
  union {
    __IO uint32_t IF1DTB_L;
    struct {
      __IO uint16_t IF1DTB1_L;
      __IO uint16_t IF1DTB2_L;
    };
  };
        uint8_t RESERVED2[8];
  union {
    __IO uint32_t IF1DTA_B;
    struct {
      __IO uint16_t IF1DTA2_B;
      __IO uint16_t IF1DTA1_B;
    };
  };
  union {
    __IO uint32_t IF1DTB_B;
    struct {
      __IO uint16_t IF1DTB2_B;
      __IO uint16_t IF1DTB1_B;
    };
  };
        uint8_t RESERVED3[8];
  union {
    __IO uint16_t IF2CREQ;
    stc_can_ifncreq_t IF2CREQ_f;
  };
  union {
    __IO uint16_t IF2CMSK;
    stc_can_ifncmsk_t IF2CMSK_f;
  };
  union {
    union {
      __IO uint32_t IF2MSK;
      stc_can_ifnmsk_t IF2MSK_f;
    };
    struct {
      __IO uint16_t IF2MSK1;
      union {
        __IO uint16_t IF2MSK2;
        stc_can_ifnmsk2_t IF2MSK2_f;
      };
    };
  };
  union {
    union {
      __IO uint32_t IF2ARB;
      stc_can_ifnarb_t u32IF2ARB;
    };
    struct {
      __IO uint16_t IF2ARB1;
      union {
        __IO uint16_t IF2ARB2;
        stc_can_ifnarb2_t IF2ARB2_f;
      };
    };
  };
  union {
    __IO uint16_t IF2MCTR;
    stc_can_ifnmctr_t IF2MCTR_f;
  };
        uint8_t RESERVED4[2];
  union {
    __IO uint32_t IF2DTA_L;
    struct {
      __IO uint16_t IF2DTA1_L;
      __IO uint16_t IF2DTA2_L;
    };
  };
  union {
    __IO uint32_t IF2DTB_L;
    struct {
      __IO uint16_t IF2DTB1_L;
      __IO uint16_t IF2DTB2_L;
    };
  };
        uint8_t RESERVED5[8];
  union {
    __IO uint32_t IF2DTA_B;
    struct {
      __IO uint16_t IF2DTA2_B;
      __IO uint16_t IF2DTA1_B;
    };
  };
  union {
    __IO uint32_t IF2DTB_B;
    struct {
      __IO uint16_t IF2DTB2_B;
      __IO uint16_t IF2DTB1_B;
    };
  };
        uint8_t RESERVED6[24];
  union {
    union {
      __IO uint32_t TREQR;
      stc_can_treqr_t TREQR_f;
    };
    struct {
      union {
        __IO uint16_t TREQR1;
        stc_can_treqr1_t TREQR1_f;
      };
      union {
        __IO uint16_t TREQR2;
        stc_can_treqr2_t TREQR2_f;
      };
    };
  };
        uint8_t RESERVED7[12];
  union {
    union {
      __IO uint32_t NEWDT;
      stc_can_newdt_t NEWDT_f;
    };
    struct {
      union {
        __IO uint16_t NEWDT1;
        stc_can_newdt1_t NEWDT1_f;
      };
      __IO uint16_t NEWDT2;
    };
  };
        uint8_t RESERVED8[12];
  union {
    union {
      __IO uint32_t INTPND;
      stc_can_intpnd_t INTPND_f;
    };
    struct {
      union {
        __IO uint16_t INTPND1;
        stc_can_intpnd1_t INTPND1_f;
      };
      union {
        __IO uint16_t INTPND2;
        stc_can_intpnd2_t INTPND2_f;
      };
    };
  };
        uint8_t RESERVED9[12];
  union {
    union {
      __IO uint32_t MSGVAL;
      stc_can_msgval_t MSGVAL_f;
    };
    struct {
      union {
        __IO uint16_t MSGVAL1;
        stc_can_msgval1_t MSGVAL1_f;
      };
      union {
        __IO uint16_t MSGVAL2;
        stc_can_msgval2_t MSGVAL2_f;
      };
    };
  };
} stc_cann_t;

/**
 ******************************************************************************
 ** \brief CAN Unit configuration structures
 ******************************************************************************/

/**
 ******************************************************************************
 ** \brief CAN Prescaler
 ******************************************************************************/
typedef enum en_can_prescaler
{
  CanPreDiv11  = 0,   ///< CAN prescaler clock: no division
  CanPreDiv12  = 1,   ///< CAN prescaler clock: 1/2
  CanPreDiv14  = 2,   ///< CAN prescaler clock: 1/4
  CanPreDiv18  = 3,   ///< CAN prescaler clock: 1/8
  CanPreDiv23  = 4,   ///< CAN prescaler clock: 2/3
  CanPreDiv13  = 5,   ///< CAN prescaler clock: 1/3
  CanPreDiv16  = 6,   ///< CAN prescaler clock: 1/6
  CanPreDiv112 = 7,   ///< CAN prescaler clock: 1/12
  CanPreDiv15  = 8,   ///< CAN prescaler clock: 1/5
  CanPreDiv110 = 9    ///< CAN prescaler clock: 1/10
} en_can_prescaler_t;

/**
 ******************************************************************************
 ** \brief CAN bitrate.
 ** 
 ** This structure is used to set the bitrate register. All values are 
 ** decremented by 1 before writing to the CANn_BTR register 
 ** (see HW manual for more details).
 ******************************************************************************/
typedef struct stc_can_bitrate
{
  uint8_t             u8TimeSegment1;  ///< Range CAN_BITRATE_TSEG1_MIN to CAN_BITRATE_TSEG1_MAX (see define section)
  uint8_t             u8TimeSegment2;  ///< Range CAN_BITRATE_TSEG2_MIN to CAN_BITRATE_TSEG2_MAX (see define section)
  uint8_t             u8SyncJumpWidth; ///< Range CAN_BITRATE_SYNC_JUMP_WIDTH_MIN to CAN_BITRATE_SYNC_JUMP_WIDTH_MAX (see define section)
  uint16_t            u16Prescaler;    ///< Range PRESCALER_MIN to CAN_BITRATE_PRESCALER_MAX (see define section, divider for the peripheral clock CLKP2)
  en_can_prescaler_t  enCanPrescaler;  ///< CAN PRESCALER setting. See #en_can_prescaler_t for details
} stc_can_bitrate_t;


/**
 ******************************************************************************
 ** \brief CAN message identifier
 ** 
 ** This structure is used to set the CAN message identifier for transmit and 
 ** receive operations. The identifiction is needed when setting the CAN 
 ** arbitration register. On the one side this structure is used in the 
 ** Can_SetTransmitMsgBuffer() function, on the other side same structure is 
 ** used in the structure stc_can_msg_t which is used for receive data in 
 ** Can_SetReceiveMsgBuffer(...) function.
 ******************************************************************************/
typedef struct stc_can_msg_id
{
  uint32_t  u32Identifier;    ///< 11- or 29-bit identifier. The valid bit length depends on bExtended.
  boolean_t bExtended;        ///< TRUE: 29-bit identifier,  FALSE: 11-bit identifier.
} stc_can_msg_id_t;


/**
 ******************************************************************************
 ** \brief CAN message data.
 ** 
 ** Data structure for transmit and receive operations, maximum value for 
 ** parameter u8DataLengthCode is CAN_MESSAGE_DATA_BUFFER_SIZE.
 ******************************************************************************/
typedef struct stc_can_msg_data
{
  uint8_t au8Data[CAN_MESSAGE_DATA_BUFFER_SIZE];  ///< Data of CAN message.
  uint8_t u8DataLengthCode;                       ///< Number of valid bytes in au8Data and DLC of CAN message.
} stc_can_msg_data_t;


/**
 ******************************************************************************
 ** \brief CAN transmit mode.
 ** 
 ** To select how transmit data should be processed.
 ******************************************************************************/
typedef enum en_can_tx_mode
{
  CanImmediateTransmit   = 0,    ///< Transmit message immediately
  CanRemoteTransmit      = 1     ///< Transmit message on reception of remote frame
} en_can_tx_mode_t;


/**
 ******************************************************************************
 ** \brief CAN status.
 ** 
 ** These state values represent the current CAN state, and are used in the 
 ** CAN status changed callback function (can_status_chg_func_ptr_t).
 ******************************************************************************/
typedef enum en_can_status
{
  CanNoError    = 0,   ///< No error pending.
  CanStuffError = 1,   ///< More than 5 equal bits in a sequence have occurred in a part of a received message.
  CanFormError  = 2,   ///< A fixed format part of a received frame has the wrong format.
  CanAckError   = 3,   ///< The message this CAN Core transmitted was not acknowledged by another node.
  CanBit1Error  = 4,   ///< While trying to send a recessive bit (1) a dominant level (0) was sampled.
  CanBit0Error  = 5,   ///< While trying to send a dominant bit (0) a recessive level (1) was sampled.
  CanCRCError   = 6    ///< The CRC checksum was incorrect.
} en_can_status_t;


/**
 ******************************************************************************
 ** \brief CAN error.
 ** 
 ** These error values are used to report any CAN related error to the API via
 ** the CAN error callback function (can_error_func_ptr_t).
 ******************************************************************************/
typedef enum en_can_error
{
  CanBusOff        = 0,    ///< The CAN module is in busoff state.
  CanWarning       = 1     ///< At least one error counter has reached error warning limit of 96.
} en_can_error_t;

/**
 ******************************************************************************
 ** \brief CAN message.
 ** 
 ** This structure stores a CAN message, including the identifier, data 
 ** and data length code (DLC). It also contains an overflow and new flag 
 ** which indicates the message state for received messages.
 ******************************************************************************/
typedef struct stc_can_msg
{
  stc_can_msg_id_t    stcIdentifier;  ///< 11- or 29-bit identifier (ID).
  stc_can_msg_data_t  stcData;        ///< Data and DLC.
  volatile boolean_t  bOverflow;      ///< TRUE if new message was received while bNew still TRUE 
  volatile boolean_t  bNew;           ///< TRUE if new message received (must be reset to FALSE by application to release message object).
} stc_can_msg_t;

/**
 *****************************************************************************
 ** \brief Message transmission complete callback function (can_tx_msg_func_ptr_t).
 **
 ** Signals a successful completed transmission.
 *****************************************************************************/
typedef void (*can_tx_msg_func_ptr_t)(uint8_t u8MsgBuf);


/**
 *****************************************************************************
 ** \brief Message reception callback function (can_rx_msg_func_ptr_t).
 **
 ** Signals that CAN has received a new message.
 *****************************************************************************/
typedef void (*can_rx_msg_func_ptr_t)(uint8_t u8MsgBuf, stc_can_msg_t* pstcRxMsg);


/**
 *****************************************************************************
 ** \brief Status changed callback function (can_status_chg_func_ptr_t).
 **
 ** Any status change will be reported to the API (see #en_can_status_t for 
 ** possible status change codes).
 *****************************************************************************/
typedef void (*can_status_chg_func_ptr_t)(en_can_status_t enCanStatus);


/**
 *****************************************************************************
 ** \brief Error callback function (can_error_func_ptr_t).
 **
 ** Any error will be reported to the API (see #en_can_error_t for 
 ** possible error codes).
 *****************************************************************************/
typedef void (*can_error_func_ptr_t)(en_can_error_t enCanError);

/**
 *****************************************************************************
 ** \brief CAN interrupt pointer structure
 **
 ** Holds some pointers to callback functions and buffer
 *****************************************************************************/
typedef struct stc_can_interrupt_handling
{
  stc_can_msg_t*        pstcMsgBuffer; ///< pointer to message buffer
  can_tx_msg_func_ptr_t pfnCanTxInterruptFunction; ///< pointer to transmit interrupt callback
  can_rx_msg_func_ptr_t pfnCanRxInterruptFunction; ///< pointer to receive interrupt callback
} stc_can_interrupt_handling_t;

/**
 *****************************************************************************
 ** \brief CAN notification pointer structure
 **
 ** Holds some pointers to callback functions for status and error notification
 *****************************************************************************/
typedef struct stc_can_notification
{
  can_status_chg_func_ptr_t pfnCanStatusInterruptFunction ;
  can_error_func_ptr_t      pfnCanErrorInterruptFunction ;
} stc_can_notification_t;

/**
 ******************************************************************************
 ** \brief CAN configuration.
 ** 
 ** Is used to set the CAN bitrate and allows deactivating of automatic
 ** retransmission in case of a transmission error. Optionally error and
 ** status changed notification can be set.  
 ******************************************************************************/
typedef struct stc_can_config
{
  can_status_chg_func_ptr_t   pfnStatusCallback;                ///< Callback function for CAN status changes, can be NULL.
  can_error_func_ptr_t        pfnErrorCallback;                 ///< Callback function for CAN related errors, can be NULL.
  boolean_t                   bDisableAutomaticRetransmission;  ///< TRUE: Automatic retransmission is disabled, FALSE: Automatic retransmission is enabled.
  stc_can_bitrate_t           stcBitrate;                       ///< CAN bitrate setting.
  boolean_t                   bTouchPrescaler;                  ///< TRUE: Can_Init() initializes CANPRES, FALSE: CANPRES not touched
} stc_can_config_t;

/// Enumeration to define an index for each enabled CAN instance
typedef enum en_can_instance_index
{
  #if (PDL_PERIPHERAL_ENABLE_CAN0 == PDL_ON)
    CanInstanceIndexCan0,
  #endif
  #if (PDL_PERIPHERAL_ENABLE_CAN1 == PDL_ON)
    CanInstanceIndexCan1,
  #endif
    CanInstanceIndexMax
} en_can_instance_index_t;

/// CAN instance internal data, storing internal information for each enabled CAN instance.
typedef struct stc_can_intern_data
{
  stc_can_interrupt_handling_t stcCanInterruptHandling[CAN_MESSAGE_BUFFER_COUNT];
  stc_can_notification_t       stcCanNotificationCb;
  boolean_t                    bEobZero;
} stc_can_intern_data_t;

/// CAN instance data type
typedef struct stc_can_instance_data
{
  volatile stc_cann_t*  pstcInstance;  ///< pointer to registers of an instance
  stc_can_intern_data_t stcInternData; ///< module internal data of instance
} stc_can_instance_data_t;

/******************************************************************************/
/* Global variable declarations ('extern', definition in C source)            */
/******************************************************************************/

/// Look-up table for all enabled CAN instances and their internal data
extern stc_can_instance_data_t m_astcCanInstanceDataLut[];

/******************************************************************************/
/* Global function prototypes ('extern', definition in C source)              */
/******************************************************************************/
extern void CanIrqHandler( volatile stc_cann_t* pstcCan,
                           stc_can_intern_data_t* pstcCanInternData
                         ) ;

extern en_result_t Can_Init( volatile stc_cann_t* pstcCan, 
                             const stc_can_config_t* pstcConfig
                           ) ;

extern en_result_t Can_DeInit( volatile stc_cann_t* pstcCan ) ;

extern en_result_t Can_SetTransmitMsgBuffer( volatile stc_cann_t* pstcCan, 
                                             uint8_t u8MsgBuf, 
                                             const stc_can_msg_id_t* pstcMsgId, 
                                             can_tx_msg_func_ptr_t pfnCallback
                                           ) ;

extern en_result_t Can_UpdateAndTransmitMsgBuffer( 
                                          volatile stc_cann_t* pstcCan, 
                                          uint8_t u8MsgBuf, 
                                          const stc_can_msg_data_t* pstcMsgData, 
                                          en_can_tx_mode_t enTxMode 
                                        ) ;

extern en_result_t Can_UpdateAndTransmitFifoMsgBuffer( volatile stc_cann_t* pstcCan,
                                                uint8_t u8MsgBuf,
                                                const stc_can_msg_data_t* pstcMsgData,
                                                en_can_tx_mode_t enTxMode
                                              ) ;

extern en_result_t Can_SetReceiveMsgBuffer( volatile stc_cann_t* pstcCan, 
                                            uint8_t u8MsgBuf, 
                                            stc_can_msg_t* pstcMsgBuffer, 
                                            uint32_t u32MsgIdMask, 
                                            can_rx_msg_func_ptr_t pfnCallback
                                          ) ;

extern en_result_t Can_SetReceiveFifoMsgBuffer( volatile stc_cann_t* pstcCan,
                                                uint8_t u8MsgBuf,
                                                stc_can_msg_t* pstcMsgBuffer,
                                                uint32_t u32MsgIdMask,
                                                can_rx_msg_func_ptr_t pfnCallback
                                              ) ;

extern en_result_t Can_ResetMsgBuffer( volatile stc_cann_t* pstcCan, uint8_t u8MsgBuf ) ;

//@}
#ifdef __cplusplus
}
#endif

#endif // #if (defined(PDL_PERIPHERAL_CAN_ACTIVE))

#endif /* __CAN_H__ */
/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
