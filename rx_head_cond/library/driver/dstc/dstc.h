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
/** \file dstc.h
 **
 ** Headerfile for DSTC functions
 **  
 ** History:
 **   - 2013-04-24  1.0  MWi  First version.
 **
 ******************************************************************************/

#ifndef __DSTC_H__
#define __DSTC_H__

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "pdl.h"

#if (defined(PDL_PERIPHERAL_DSTC_ACTIVE))

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/**
 ******************************************************************************
 ** \defgroup DstcGroup Descriptor System Data Transfer Controller (DSTC)
 **
 ** Provided functions of DSTC:
 **
 ** - Dstc_ReleaseStandBy()
 ** - Dstc_Init()
 ** - Dstc_ReadHwdesp()
 ** - Dstc_SetCommand()
 ** - Dstc_SwTrigger()
 ** - Dstc_SwTrqansferStartStatus()
 ** - Dstc_SetDreqenb()
 ** - Dstc_ReadDreqenb()
 ** - Dstc_SetDreqenbBit()
 ** - Dstc_ClearDreqenbBit()
 ** - Dstc_ReadHwint()
 ** - Dstc_SetHwintclr()
 ** - Dstc_SetHwintclrBit()
 ** - Dstc_ReadDqmsk()
 ** - Dstc_SetDqmskclr()
 ** - Dstc_SetDqmskclrBit()
 **
 ** Dstc_ReleaseStandBy() "wakes up" the DSTC from standby mode.
 ** Dsct_Init() initializes the DSTC with the user configuration (Priority
 ** Settings, Pointers to (Error) Callback Functions, etc.); Dstc_ReleaseStandBy()
 ** is called automatically via initialization.
 ** Dstc_ReadHwdesp() returns the DES area start address of a dedicated channel.
 ** Dstc_SetCommand() sets a command to the CMD register.
 ** Dstc_SwTrigger() performs a software trigger for a configured transfer.
 ** Dstc_SwTrqansferStartStatus() returns the status of a triggered Software
 ** transfer.
 ** Dstc_SetDreqenb() sets the DRQENB register set via the structure
 ** #stc_dstc_dreqenb_t.
 ** Dstc_ReadDreqenb() returns the value of the DRQENB register to the given
 ** structure of the type of #stc_dstc_dreqenb_t.
 ** Dstc_SetDreqenbBit() sets a bit in the 256-Bitfield of DRQENB register.
 ** Dstc_ClearDreqenbBit() clears a bit in the 256-Bitfield of DRQENB register.
 ** Dstc_ReadHwint() returns the contents of the HWINT register into a
 ** strucutre of the type of #stc_dstc_hwint_t.
 ** Dstc_SetHwintclr() sets clear bits in the HWINTCLR register via the
 ** structure of the type of #stc_dstc_hwintclr_t.
 ** Dstc_SetHwintclrbBit() sets clear bits in the 256-Bitfield of the HWINTCLR
 ** register.
 ** Dstc_ReadDqmsk() reads the contents of the DQMSK register into a structure
 ** of the type of #stc_dstc_dqmsk_t.
 ** Dstc_SetDqmskclr() sets clear bits in the DQMSKCLR register via the
 ** structure of the type of #stc_dstc_dqmskclr_t.
 ** Dstc_SetDqmskclrbBit() sets a clear bit in the 256-Bitfield of the DQMSKCLR
 ** register.
 **
 ** In dstc.h there are predefined DES structure types for every combination of 
 ** the descriptors. Note that for DES1 two different structures are existing 
 ** for mode0 and mode1. The user should use these structures within an
 ** enclosing structure, so that the descriptors are located on consecutive
 ** addresses. The address of this structure then should be used for
 ** stc_dstc_config_t#u32Destp.
 **
 ** <b>Example:</b><br>
 ** <table class="params">
 ** <tr><td class="paramdir">struct stc_dstc_des_area</td></tr>
 ** <tr><td class="paramdir">{</td></tr>
 ** <tr><td class="paramdir">&nbsp;&nbsp;stc_dstc_des01236_t&nbsp;&nbsp;&nbsp;stcDesList0;</td></tr>
 ** <tr><td class="paramdir">&nbsp;&nbsp;stc_dstc_des0123_t&nbsp;&nbsp;&nbsp;&nbsp;stcDesList1;</td></tr>
 ** <tr><td class="paramdir">&nbsp;&nbsp;stc_dstc_des0123_t&nbsp;&nbsp;&nbsp;&nbsp;stcDesList2;</td></tr>
 ** <tr><td class="paramdir">&nbsp;&nbsp;stc_dstc_des0123456_t&nbsp;stcDesList3;</td></tr>
 ** <tr><td class="paramdir">&nbsp;&nbsp;. . .</td></tr>
 ** <tr><td class="paramdir">};</td></tr>
 ** </table>
 ** 
 ******************************************************************************/
//@{
  
/******************************************************************************/
/* Global pre-processor symbols/macros ('#define')                            */
/******************************************************************************/ 
#define DSTC_TRANSITION_TIMEOUT 100000u          ///< Timeout counter for DSTC mode transition
   
/// Parity checksum for DES0 PCHKS bits
#define DSTC_PCHK_CALC(x) ((((x) & 0x0F000000ul) >> 24u) ^ \
                           (((x) & 0x00F00000ul) >> 20u) ^ \
                           (((x) & 0x000F0000ul) >> 16u) ^ \
                           (((x) & 0x0000F000ul) >> 12u) ^ \
                           (((x) & 0x00000F00ul) >>  8u) ^ \
                           (((x) & 0x000000F0ul) >>  4u))

/// DSTC interrupt numbers
#define DSTC_IRQ_NUMBER_ADC0_PRIO               110u
#define DSTC_IRQ_NUMBER_ADC0_SCAN               111u
#define DSTC_IRQ_NUMBER_ADC1_PRIO               112u
#define DSTC_IRQ_NUMBER_ADC1_SCAN               113u
#define DSTC_IRQ_NUMBER_ADC2_PRIO               114u
#define DSTC_IRQ_NUMBER_ADC2_SCAN               115u
#define DSTC_IRQ_NUMBER_BT0_IRQ0                 16u
#define DSTC_IRQ_NUMBER_BT0_IRQ1                 17u
#define DSTC_IRQ_NUMBER_BT10_IRQ0               148u
#define DSTC_IRQ_NUMBER_BT10_IRQ1               149u
#define DSTC_IRQ_NUMBER_BT11_IRQ0               150u
#define DSTC_IRQ_NUMBER_BT11_IRQ1               151u
#define DSTC_IRQ_NUMBER_BT12_IRQ0               152u
#define DSTC_IRQ_NUMBER_BT12_IRQ1               153u
#define DSTC_IRQ_NUMBER_BT13_IRQ0               154u
#define DSTC_IRQ_NUMBER_BT13_IRQ1               155u
#define DSTC_IRQ_NUMBER_BT14_IRQ0               156u
#define DSTC_IRQ_NUMBER_BT14_IRQ1               157u
#define DSTC_IRQ_NUMBER_BT15_IRQ0               158u
#define DSTC_IRQ_NUMBER_BT15_IRQ1               159u
#define DSTC_IRQ_NUMBER_BT1_IRQ0                 18u
#define DSTC_IRQ_NUMBER_BT1_IRQ1                 19u
#define DSTC_IRQ_NUMBER_BT2_IRQ0                 20u
#define DSTC_IRQ_NUMBER_BT2_IRQ1                 21u
#define DSTC_IRQ_NUMBER_BT3_IRQ0                 22u
#define DSTC_IRQ_NUMBER_BT3_IRQ1                 23u
#define DSTC_IRQ_NUMBER_BT4_IRQ0                 24u
#define DSTC_IRQ_NUMBER_BT4_IRQ1                 25u
#define DSTC_IRQ_NUMBER_BT5_IRQ0                 26u
#define DSTC_IRQ_NUMBER_BT5_IRQ1                 27u
#define DSTC_IRQ_NUMBER_BT6_IRQ0                 28u
#define DSTC_IRQ_NUMBER_BT6_IRQ1                 29u
#define DSTC_IRQ_NUMBER_BT7_IRQ0                 30u
#define DSTC_IRQ_NUMBER_BT7_IRQ1                 31u
#define DSTC_IRQ_NUMBER_BT8_IRQ0                144u
#define DSTC_IRQ_NUMBER_BT8_IRQ1                145u
#define DSTC_IRQ_NUMBER_BT9_IRQ0                146u
#define DSTC_IRQ_NUMBER_BT9_IRQ1                147u
#define DSTC_IRQ_NUMBER_EXTINT0                   0u
#define DSTC_IRQ_NUMBER_EXTINT1                   1u
#define DSTC_IRQ_NUMBER_EXTINT10                 10u
#define DSTC_IRQ_NUMBER_EXTINT11                 11u
#define DSTC_IRQ_NUMBER_EXTINT12                 12u
#define DSTC_IRQ_NUMBER_EXTINT13                 13u
#define DSTC_IRQ_NUMBER_EXTINT14                 14u
#define DSTC_IRQ_NUMBER_EXTINT15                 15u
#define DSTC_IRQ_NUMBER_EXTINT16                128u
#define DSTC_IRQ_NUMBER_EXTINT17                129u
#define DSTC_IRQ_NUMBER_EXTINT18                130u
#define DSTC_IRQ_NUMBER_EXTINT19                131u
#define DSTC_IRQ_NUMBER_EXTINT2                   2u
#define DSTC_IRQ_NUMBER_EXTINT20                132u
#define DSTC_IRQ_NUMBER_EXTINT21                133u
#define DSTC_IRQ_NUMBER_EXTINT22                134u
#define DSTC_IRQ_NUMBER_EXTINT23                135u
#define DSTC_IRQ_NUMBER_EXTINT24                136u
#define DSTC_IRQ_NUMBER_EXTINT25                137u
#define DSTC_IRQ_NUMBER_EXTINT26                138u
#define DSTC_IRQ_NUMBER_EXTINT27                139u
#define DSTC_IRQ_NUMBER_EXTINT28                140u
#define DSTC_IRQ_NUMBER_EXTINT29                141u
#define DSTC_IRQ_NUMBER_EXTINT3                   3u
#define DSTC_IRQ_NUMBER_EXTINT30                142u
#define DSTC_IRQ_NUMBER_EXTINT31                143u
#define DSTC_IRQ_NUMBER_EXTINT4                   4u
#define DSTC_IRQ_NUMBER_EXTINT5                   5u
#define DSTC_IRQ_NUMBER_EXTINT6                   6u
#define DSTC_IRQ_NUMBER_EXTINT7                   7u
#define DSTC_IRQ_NUMBER_EXTINT8                   8u
#define DSTC_IRQ_NUMBER_EXTINT9                   9u
#define DSTC_IRQ_NUMBER_MFS0_RX                  89u
#define DSTC_IRQ_NUMBER_MFS0_TX                  90u
#define DSTC_IRQ_NUMBER_MFS10_RX                198u
#define DSTC_IRQ_NUMBER_MFS10_TX                199u
#define DSTC_IRQ_NUMBER_MFS11_RX                200u
#define DSTC_IRQ_NUMBER_MFS11_TX                201u
#define DSTC_IRQ_NUMBER_MFS12_RX                209u
#define DSTC_IRQ_NUMBER_MFS12_TX                210u
#define DSTC_IRQ_NUMBER_MFS13_RX                211u
#define DSTC_IRQ_NUMBER_MFS13_TX                212u
#define DSTC_IRQ_NUMBER_MFS14_RX                213u
#define DSTC_IRQ_NUMBER_MFS14_TX                214u
#define DSTC_IRQ_NUMBER_MFS15_RX                215u
#define DSTC_IRQ_NUMBER_MFS15_TX                216u
#define DSTC_IRQ_NUMBER_MFS1_RX                  91u
#define DSTC_IRQ_NUMBER_MFS1_TX                  92u
#define DSTC_IRQ_NUMBER_MFS2_RX                  93u
#define DSTC_IRQ_NUMBER_MFS2_TX                  94u
#define DSTC_IRQ_NUMBER_MFS3_RX                  95u
#define DSTC_IRQ_NUMBER_MFS3_TX                  96u
#define DSTC_IRQ_NUMBER_MFS4_RX                  97u
#define DSTC_IRQ_NUMBER_MFS4_TX                  98u
#define DSTC_IRQ_NUMBER_MFS5_RX                  99u
#define DSTC_IRQ_NUMBER_MFS5_TX                 100u
#define DSTC_IRQ_NUMBER_MFS6_RX                 101u
#define DSTC_IRQ_NUMBER_MFS6_TX                 102u
#define DSTC_IRQ_NUMBER_MFS7_RX                 103u
#define DSTC_IRQ_NUMBER_MFS7_TX                 104u
#define DSTC_IRQ_NUMBER_MFS8_RX                 194u
#define DSTC_IRQ_NUMBER_MFS8_TX                 195u
#define DSTC_IRQ_NUMBER_MFS9_RX                 196u
#define DSTC_IRQ_NUMBER_MFS9_TX                 197u
#define DSTC_IRQ_NUMBER_MFT0_FRT0_PEAK           35u
#define DSTC_IRQ_NUMBER_MFT0_FRT0_ZERO           38u
#define DSTC_IRQ_NUMBER_MFT0_FRT1_PEAK           36u
#define DSTC_IRQ_NUMBER_MFT0_FRT1_ZERO           39u
#define DSTC_IRQ_NUMBER_MFT0_FRT2_PEAK           37u
#define DSTC_IRQ_NUMBER_MFT0_FRT2_ZERO           40u
#define DSTC_IRQ_NUMBER_MFT0_ICU0                41u
#define DSTC_IRQ_NUMBER_MFT0_ICU1                42u
#define DSTC_IRQ_NUMBER_MFT0_ICU2                43u
#define DSTC_IRQ_NUMBER_MFT0_ICU3                44u
#define DSTC_IRQ_NUMBER_MFT0_OCU0                45u
#define DSTC_IRQ_NUMBER_MFT0_OCU1                46u
#define DSTC_IRQ_NUMBER_MFT0_OCU2                47u
#define DSTC_IRQ_NUMBER_MFT0_OCU3                48u
#define DSTC_IRQ_NUMBER_MFT0_OCU4                49u
#define DSTC_IRQ_NUMBER_MFT0_OCU5                50u
#define DSTC_IRQ_NUMBER_MFT0_WFG10               32u
#define DSTC_IRQ_NUMBER_MFT0_WFG32               33u
#define DSTC_IRQ_NUMBER_MFT0_WFG54               34u
#define DSTC_IRQ_NUMBER_MFT1_FRT0_PEAK           54u
#define DSTC_IRQ_NUMBER_MFT1_FRT0_ZERO           57u
#define DSTC_IRQ_NUMBER_MFT1_FRT1_PEAK           55u
#define DSTC_IRQ_NUMBER_MFT1_FRT1_ZERO           58u
#define DSTC_IRQ_NUMBER_MFT1_FRT2_PEAK           56u
#define DSTC_IRQ_NUMBER_MFT1_FRT2_ZERO           59u
#define DSTC_IRQ_NUMBER_MFT1_ICU0                60u
#define DSTC_IRQ_NUMBER_MFT1_ICU1                61u
#define DSTC_IRQ_NUMBER_MFT1_ICU2                62u
#define DSTC_IRQ_NUMBER_MFT1_ICU3                63u
#define DSTC_IRQ_NUMBER_MFT1_OCU0                64u
#define DSTC_IRQ_NUMBER_MFT1_OCU1                65u
#define DSTC_IRQ_NUMBER_MFT1_OCU2                66u
#define DSTC_IRQ_NUMBER_MFT1_OCU3                67u
#define DSTC_IRQ_NUMBER_MFT1_OCU4                68u
#define DSTC_IRQ_NUMBER_MFT1_OCU5                69u
#define DSTC_IRQ_NUMBER_MFT1_WFG10               51u
#define DSTC_IRQ_NUMBER_MFT1_WFG32               52u
#define DSTC_IRQ_NUMBER_MFT1_WFG54               53u
#define DSTC_IRQ_NUMBER_MFT2_FRT0_PEAK          163u
#define DSTC_IRQ_NUMBER_MFT2_FRT0_ZERO          166u
#define DSTC_IRQ_NUMBER_MFT2_FRT1_PEAK          164u
#define DSTC_IRQ_NUMBER_MFT2_FRT1_ZERO          167u
#define DSTC_IRQ_NUMBER_MFT2_FRT2_PEAK          165u
#define DSTC_IRQ_NUMBER_MFT2_FRT2_ZERO          168u
#define DSTC_IRQ_NUMBER_MFT2_ICU0               169u
#define DSTC_IRQ_NUMBER_MFT2_ICU1               170u
#define DSTC_IRQ_NUMBER_MFT2_ICU2               171u
#define DSTC_IRQ_NUMBER_MFT2_ICU3               172u
#define DSTC_IRQ_NUMBER_MFT2_OCU0               173u
#define DSTC_IRQ_NUMBER_MFT2_OCU1               174u
#define DSTC_IRQ_NUMBER_MFT2_OCU2               175u
#define DSTC_IRQ_NUMBER_MFT2_OCU3               176u
#define DSTC_IRQ_NUMBER_MFT2_OCU4               177u
#define DSTC_IRQ_NUMBER_MFT2_OCU5               178u
#define DSTC_IRQ_NUMBER_MFT2_WFG10              160u
#define DSTC_IRQ_NUMBER_MFT2_WFG32              161u
#define DSTC_IRQ_NUMBER_MFT2_WFG54              162u
#define DSTC_IRQ_NUMBER_PPG0                     70u
#define DSTC_IRQ_NUMBER_PPG10                    74u
#define DSTC_IRQ_NUMBER_PPG12                    75u
#define DSTC_IRQ_NUMBER_PPG16                   179u
#define DSTC_IRQ_NUMBER_PPG18                   180u
#define DSTC_IRQ_NUMBER_PPG2                     71u
#define DSTC_IRQ_NUMBER_PPG20                   181u
#define DSTC_IRQ_NUMBER_PPG4                     72u
#define DSTC_IRQ_NUMBER_PPG8                     73u
#define DSTC_IRQ_NUMBER_QPRC0_COUNT_INVERSION    79u
#define DSTC_IRQ_NUMBER_QPRC0_OUT_OF_RANGE       80u
#define DSTC_IRQ_NUMBER_QPRC0_PC_MATCH           76u
#define DSTC_IRQ_NUMBER_QPRC0_PC_MATCH_RC_MATCH  81u
#define DSTC_IRQ_NUMBER_QPRC0_PC_RC_MATCH        77u
#define DSTC_IRQ_NUMBER_QPRC0_UFL_OFL_Z          78u
#define DSTC_IRQ_NUMBER_QPRC1_COUNT_INVERSION    85u
#define DSTC_IRQ_NUMBER_QPRC1_OUT_OF_RANGE       86u
#define DSTC_IRQ_NUMBER_QPRC1_PC_MATCH           82u
#define DSTC_IRQ_NUMBER_QPRC1_PC_MATCH_RC_MATCH  87u
#define DSTC_IRQ_NUMBER_QPRC1_PC_RC_MATCH        83u
#define DSTC_IRQ_NUMBER_QPRC1_UFL_OFL_Z          84u
#define DSTC_IRQ_NUMBER_QPRC2_COUNT_INVERSION   185u
#define DSTC_IRQ_NUMBER_QPRC2_OUT_OF_RANGE      186u
#define DSTC_IRQ_NUMBER_QPRC2_PC_MATCH          182u
#define DSTC_IRQ_NUMBER_QPRC2_PC_MATCH_RC_MATCH 187u
#define DSTC_IRQ_NUMBER_QPRC2_PC_RC_MATCH       183u
#define DSTC_IRQ_NUMBER_QPRC2_UFL_OFL_Z         184u
#define DSTC_IRQ_NUMBER_QPRC3_COUNT_INVERSION   191u
#define DSTC_IRQ_NUMBER_QPRC3_OUT_OF_RANGE      192u
#define DSTC_IRQ_NUMBER_QPRC3_PC_MATCH          188u
#define DSTC_IRQ_NUMBER_QPRC3_PC_MATCH_RC_MATCH 193u
#define DSTC_IRQ_NUMBER_QPRC3_PC_RC_MATCH       189u
#define DSTC_IRQ_NUMBER_QPRC3_UFL_OFL_Z         190u
#define DSTC_IRQ_NUMBER_USB0_EP1                105u
#define DSTC_IRQ_NUMBER_USB0_EP2                106u
#define DSTC_IRQ_NUMBER_USB0_EP3                107u
#define DSTC_IRQ_NUMBER_USB0_EP4                108u
#define DSTC_IRQ_NUMBER_USB0_EP5                109u
#define DSTC_IRQ_NUMBER_USB1_EP1                202u
#define DSTC_IRQ_NUMBER_USB1_EP2                203u
#define DSTC_IRQ_NUMBER_USB1_EP3                204u
#define DSTC_IRQ_NUMBER_USB1_EP4                205u
#define DSTC_IRQ_NUMBER_USB1_EP5                206u
#define DSTC_IRQ_NUMBER_WC                       88u


/******************************************************************************
 * Global type definitions
 ******************************************************************************/

/**
 ******************************************************************************
 ** \brief DSTC Commands
 ******************************************************************************/
typedef enum en_dstc_cmd
{
  CmdStandyRelease    = 0u,   ///< Instructs DSTC to return from standby state into normal state
  CmdStandyTransition = 1u,   ///< Instructs DSTC to standby state
  CmdSwclr            = 2u,   ///< Clears SWTR:SWST to '0'; negates SWINT interrupt signal
  CmdErclr            = 3u,   ///< Clears MONERS:EST, MONERS:DER, and MONERS:ESTOP
  CmdRbclr            = 4u,   ///< Clears DESP to which DSTP refers in previous transfer; Sets next DES; Clears HWDESP[n]
  CmdMkclr            = 5u    ///< Clears all DQMSK[n]
} en_dstc_cmd_t;

/**
 ******************************************************************************
 ** \brief Software Transfer Priority
 ******************************************************************************/
typedef enum en_dstc_swpr
{
  PriorityHighest = 0u,   ///< Highest Priority
  Priority1_2     = 1u,   ///< Priority 1/2 Transfer Right
  Priority1_3     = 2u,   ///< Priority 1/2 Transfer Right
  Priority1_7     = 3u,   ///< Priority 1/2 Transfer Right
  Priority1_15    = 4u,   ///< Priority 1/2 Transfer Right
  Priority1_31    = 5u,   ///< Priority 1/2 Transfer Right
  Priority1_63    = 6u,   ///< Priority 1/2 Transfer Right
  PriorityLowest  = 7u    ///< Lowest Priority
} en_dstc_swpr_t;

/**
 ******************************************************************************
 ** \brief MONERS EST Error Code
 ******************************************************************************/
typedef enum en_dstc_est_error
{
  NoError                = 0u,   ///< No error occurred
  SourceAccessError      = 1u,   ///< Source access error
  DestinationAccessError = 2u,   ///< Destination access error
  ForcedTransferStop     = 3u,   ///< Transfer has been stopped compulsorily
  DesAccessError         = 4u,   ///< DES access error
  DesOpenError           = 5u,   ///< DES open error
  UnknownError           = 6u    ///< Undefined state, should never happen
} en_dstc_est_error_t;

/**
 ******************************************************************************
 ** \brief Error Status Callback
 ******************************************************************************/
typedef void (*func_ptr_dstc_args_t)(en_dstc_est_error_t enEstError,
                                     uint16_t            u16ErrorChannel,
                                     uint16_t            u16ErrorDesPointer,
                                     boolean_t           bSoftwareError,
                                     boolean_t           bDoubleError,
                                     boolean_t           bErrorStop);

/**
 ******************************************************************************
 ** \brief DREQENB structure
 ******************************************************************************/
typedef struct stc_dstc_dreqenb
{
  uint32_t u32Dreqenb0; ///< DREQENB[31:0]
  uint32_t u32Dreqenb1; ///< DREQENB[63:32]
  uint32_t u32Dreqenb2; ///< DREQENB[95:64]
  uint32_t u32Dreqenb3; ///< DREQENB[127:96]
  uint32_t u32Dreqenb4; ///< DREQENB[159:128]
  uint32_t u32Dreqenb5; ///< DREQENB[191:160]
  uint32_t u32Dreqenb6; ///< DREQENB[223:192]
  uint32_t u32Dreqenb7; ///< DREQENB[255:224]
} stc_dstc_dreqenb_t;

/**
 ******************************************************************************
 ** \brief HWINT structure
 ******************************************************************************/
typedef struct stc_dstc_hwint
{
  uint32_t u32Hwint0;   ///< HWINT[31:0]
  uint32_t u32Hwint1;   ///< HWINT[63:32]
  uint32_t u32Hwint2;   ///< HWINT[95:64]
  uint32_t u32Hwint3;   ///< HWINT[127:96]
  uint32_t u32Hwint4;   ///< HWINT[159:128]
  uint32_t u32Hwint5;   ///< HWINT[191:160]
  uint32_t u32Hwint6;   ///< HWINT[223:192]
  uint32_t u32Hwint7;   ///< HWINT[255:224]
} stc_dstc_hwint_t;

/**
 ******************************************************************************
 ** \brief HWINTCLR structure
 ******************************************************************************/
typedef struct stc_dstc_hwintclr
{
  uint32_t u32Hwintclr0;   ///< HWINTCLR[31:0]
  uint32_t u32Hwintclr1;   ///< HWINTCLR[63:32]
  uint32_t u32Hwintclr2;   ///< HWINTCLR[95:64]
  uint32_t u32Hwintclr3;   ///< HWINTCLR[127:96]
  uint32_t u32Hwintclr4;   ///< HWINTCLR[159:128]
  uint32_t u32Hwintclr5;   ///< HWINTCLR[191:160]
  uint32_t u32Hwintclr6;   ///< HWINTCLR[223:192]
  uint32_t u32Hwintclr7;   ///< HWINTCLR[255:224]
} stc_dstc_hwintclr_t;

/**
 ******************************************************************************
 ** \brief DQMSK structure
 ******************************************************************************/
typedef struct stc_dstc_dqmsk
{
  uint32_t u32Dqmsk0;   ///< DQMSK[31:0]
  uint32_t u32Dqmsk1;   ///< DQMSK[63:32]
  uint32_t u32Dqmsk2;   ///< DQMSK[95:64]
  uint32_t u32Dqmsk3;   ///< DQMSK[127:96]
  uint32_t u32Dqmsk4;   ///< DQMSK[159:128]
  uint32_t u32Dqmsk5;   ///< DQMSK[191:160]
  uint32_t u32Dqmsk6;   ///< DQMSK[223:192]
  uint32_t u32Dqmsk7;   ///< DQMSK[255:224]
} stc_dstc_dqmsk_t;

/**
 ******************************************************************************
 ** \brief DQMSKCLR structure
 ******************************************************************************/
typedef struct stc_dstc_dqmskclr
{
  uint32_t u32Dqmskclr0;   ///< DQMSKCLR[31:0]
  uint32_t u32Dqmskclr1;   ///< DQMSKCLR[63:32]
  uint32_t u32Dqmskclr2;   ///< DQMSKCLR[95:64]
  uint32_t u32Dqmskclr3;   ///< DQMSKCLR[127:96]
  uint32_t u32Dqmskclr4;   ///< DQMSKCLR[159:128]
  uint32_t u32Dqmskclr5;   ///< DQMSKCLR[191:160]
  uint32_t u32Dqmskclr6;   ///< DQMSKCLR[223:192]
  uint32_t u32Dqmskclr7;   ///< DQMSKCLR[255:224]
} stc_dstc_dqmskclr_t;

/**
 ******************************************************************************
 ** \brief Definition of Descriptor 0 (DES0 - Transfer Basic Setting)
 ******************************************************************************/
typedef struct stc_dstc_des0
{
  uint32_t DV       : 2;
  uint32_t ST       : 2;
  uint32_t MODE     : 1;
  uint32_t ORL      : 3;
  uint32_t TW       : 2;
  uint32_t SAC      : 3;
  uint32_t DAC      : 3;
  uint32_t CHRS     : 6;
  uint32_t DMSET    : 1;
  uint32_t CHLK     : 1;
  uint32_t ACK      : 2;
  uint32_t RESERVED : 2;
  uint32_t PCHK     : 4;
} stc_dstc_des0_t;

/**
 ******************************************************************************
 ** \brief Definition of Descriptor 1 (DES1 - Number of Transfers) in Mode 0
 ******************************************************************************/
typedef struct stc_dstc_des1_mode0
{
  uint32_t IIN      : 16;
  uint32_t ORM      : 16;
} stc_dstc_des1_mode0_t;

/**
 ******************************************************************************
 ** \brief Definition of Descriptor 1 (DES1 - Number of Transfers) in Mode 1
 ******************************************************************************/
typedef struct stc_dstc_des1_mode1
{
  uint32_t IIN      : 8;
  uint32_t IRM      : 8;
  uint32_t ORM      : 16;
} stc_dstc_des1_mode1_t;

/**
 ******************************************************************************
 ** \brief Definition of Descriptor Group DES0 - DES3
 ******************************************************************************/
typedef struct stc_dstc_des0123
{
  union
  {
    uint32_t        u32DES0;            // Needed for PCHK calculation
    stc_dstc_des0_t DES0;               // Transfer basic setting
  };
  union
  {
    stc_dstc_des1_mode0_t DES1_mode0;   // Number of Transfers in Mode 0
    stc_dstc_des1_mode1_t DES1_mode1;   // Number of Transfers in Mode 1
  };
  uint32_t        DES2;                 // Source Address
  uint32_t        DES3;                 // Destination Address
} stc_dstc_des0123_t;

/**
 ******************************************************************************
 ** \brief Definition of Descriptor Group DES0 - DES4
 ******************************************************************************/
typedef struct stc_dstc_des01234
{
  union
  {
    uint32_t        u32DES0;            // Needed for PCHK calculation
    stc_dstc_des0_t DES0;               // Transfer basic setting
  };
  union
  {
    stc_dstc_des1_mode0_t DES1_mode0;   // Number of Transfers in Mode 0
    stc_dstc_des1_mode1_t DES1_mode1;   // Number of Transfers in Mode 1
  };
  uint32_t        DES2;                 // Source Address
  uint32_t        DES3;                 // Destination Address
  union
  {
    stc_dstc_des1_mode0_t DES4_mode0;   // Number of Transfers in Mode 0 for OuterReload (copied in DES1 Mode 0)
    stc_dstc_des1_mode1_t DES4_mode1;   // Number of Transfers in Mode 1 for OuterReload (copied in DES1 Mode 1)
  };
} stc_dstc_des01234_t;

/**
 ******************************************************************************
 ** \brief Definition of Descriptor Group DES0 - DES5
 ******************************************************************************/
typedef struct stc_dstc_des012345
{
  union
  {
    uint32_t        u32DES0;            // Needed for PCHK calculation
    stc_dstc_des0_t DES0;               // Transfer basic setting
  };
    union
  {
    stc_dstc_des1_mode0_t DES1_mode0;   // Number of Transfers in Mode 0
    stc_dstc_des1_mode1_t DES1_mode1;   // Number of Transfers in Mode 1
  };
  uint32_t        DES2;                 // Source Address
  uint32_t        DES3;                 // Destination Address
  union
  {
    stc_dstc_des1_mode0_t DES4_mode0;   // Number of Transfers in Mode 0 for OuterReload (copied in DES1 Mode 0)
    stc_dstc_des1_mode1_t DES4_mode1;   // Number of Transfers in Mode 1 for OuterReload (copied in DES1 Mode 1)
  };
  uint32_t        DES5;                 // Source Address in OuterReload (copied in DES2)
} stc_dstc_des012345_t;

/**
 ******************************************************************************
 ** \brief Definition of Descriptor Group DES0 - DES6
 ******************************************************************************/
typedef struct stc_dstc_des0123456
{ 
  union
  {
    uint32_t        u32DES0;            // Needed for PCHK calculation
    stc_dstc_des0_t DES0;               // Transfer basic setting
  };
  union
  {
    stc_dstc_des1_mode0_t DES1_mode0;   // Number of Transfers in Mode 0
    stc_dstc_des1_mode1_t DES1_mode1;   // Number of Transfers in Mode 1
  };
  uint32_t        DES2;                 // Source Address
  uint32_t        DES3;                 // Destination Address
  union
  {
    stc_dstc_des1_mode0_t DES4_mode0;   // Number of Transfers in Mode 0 for OuterReload (copied in DES1 Mode 0)
    stc_dstc_des1_mode1_t DES4_mode1;   // Number of Transfers in Mode 1 for OuterReload (copied in DES1 Mode 1)
  };
  uint32_t        DES5;                 // Source Address in OuterReload (copied in DES2)
  uint32_t        DES6;                 // Destination Address in OuterReload (copied in DES3)
} stc_dstc_des0123456_t;

/**
 ******************************************************************************
 ** \brief Definition of Descriptor Group DES0 - DES3, DES5
 ******************************************************************************/
typedef struct stc_dstc_des01235
{
  union
  {
    uint32_t        u32DES0;            // Needed for PCHK calculation
    stc_dstc_des0_t DES0;               // Transfer basic setting
  };
  union
  {
    stc_dstc_des1_mode0_t DES1_mode0;   // Number of Transfers in Mode 0
    stc_dstc_des1_mode1_t DES1_mode1;   // Number of Transfers in Mode 1
  };
  uint32_t        DES2;                 // Source Address
  uint32_t        DES3;                 // Destination Address
  uint32_t        DES5;                 // Source Address in OuterReload (copied in DES2)
} stc_dstc_des01235_t;

/**
 ******************************************************************************
 ** \brief Definition of Descriptor Group DES0 - DES3, DES6
 ******************************************************************************/
typedef struct stc_dstc_des01236
{
  union
  {
    uint32_t        u32DES0;            // Needed for PCHK calculation
    stc_dstc_des0_t DES0;               // Transfer basic setting
  };
  union
  {
    stc_dstc_des1_mode0_t DES1_mode0;   // Number of Transfers in Mode 0
    stc_dstc_des1_mode1_t DES1_mode1;   // Number of Transfers in Mode 1
  };
  uint32_t        DES2;                 // Source Address
  uint32_t        DES3;                 // Destination Address
  uint32_t        DES6;                 // Destination Address in OuterReload (copied in DES3)
} stc_dstc_des01236_t;

/**
 ******************************************************************************
 ** \brief Definition of Descriptor Group DES0 - DES4, DES6
 ******************************************************************************/
typedef struct stc_dstc_des012346
{
  union
  {
    uint32_t        u32DES0;            // Needed for PCHK calculation
    stc_dstc_des0_t DES0;               // Transfer basic setting
  };
  union
  {
    stc_dstc_des1_mode0_t DES1_mode0;   // Number of Transfers in Mode 0
    stc_dstc_des1_mode1_t DES1_mode1;   // Number of Transfers in Mode 1
  };
  uint32_t        DES2;                 // Source Address
  uint32_t        DES3;                 // Destination Address
  union
  {
    stc_dstc_des1_mode0_t DES4_mode0;   // Number of Transfers in Mode 0 for OuterReload (copied in DES1 Mode 0)
    stc_dstc_des1_mode1_t DES4_mode1;   // Number of Transfers in Mode 1 for OuterReload (copied in DES1 Mode 1)
  };
  uint32_t        DES6;                 // Destination Address in OuterReload (copied in DES3)
} stc_dstc_des012346_t;

/**
 ******************************************************************************
 ** \brief Definition of Descriptor Group DES0 - DES3, DES5, DES6
 ******************************************************************************/
typedef struct stc_dstc_des012356
{
  union
  {
    uint32_t        u32DES0;            // Needed for PCHK calculation
    stc_dstc_des0_t DES0;               // Transfer basic setting
  };
  union
  {
    stc_dstc_des1_mode0_t DES1_mode0;   // Number of Transfers in Mode 0
    stc_dstc_des1_mode1_t DES1_mode1;   // Number of Transfers in Mode 1
  };
  uint32_t        DES2;                 // Source Address
  uint32_t        DES3;                 // Destination Address
  uint32_t        DES5;                 // Source Address in OuterReload (copied in DES2)
  uint32_t        DES6;                 // Destination Address in OuterReload (copied in DES3)
} stc_dstc_des012456_t;

/**
 ******************************************************************************
 ** \brief DSTC configuration.
 ******************************************************************************/
typedef struct stc_dstc_config
{
  uint32_t              u32Destp;               ///< Start Address of DES Area (must be aligned to 32 Bit!)
  boolean_t             bSwInterruptEnable;     ///< TRUE: Software Interrupt enabled
  boolean_t             bErInterruptEnable;     ///< TRUE: Error Interrupt enabled
  boolean_t             bReadSkipBufferDisable; ///< TRUE: Read Skip Buffer disabled
  boolean_t             bErrorStopEnable;       ///< TRUE: Enables Error Stop
  en_dstc_swpr_t        enSwTransferPriority;   ///< see #en_dstc_swpr_t for details
  func_ptr_t            pfnNotifySwCallback;    ///< Notification SW Callback Function Pointer
  func_ptr_dstc_args_t  pfnErrorCallback;       ///< Error Status Callback
  
#if (PDL_ON == PDL_DSTC_ENABLE_ADC0_PRIO)
  func_ptr_t            pfnDstcAdc0PrioCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_ADC0_SCAN)
  func_ptr_t            pfnDstcAdc0ScanCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_ADC1_PRIO)
  func_ptr_t            pfnDstcAdc1PrioCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_ADC1_SCAN)
  func_ptr_t            pfnDstcAdc1ScanCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_ADC2_PRIO)
  func_ptr_t            pfnDstcAdc2PrioCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_ADC2_SCAN)
  func_ptr_t            pfnDstcAdc2ScanCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT0_IRQ0)
  func_ptr_t            pfnDstcBt0Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT0_IRQ1)
  func_ptr_t            pfnDstcBt0Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT1_IRQ0)
  func_ptr_t            pfnDstcBt1Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT1_IRQ1)
  func_ptr_t            pfnDstcBt1Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT2_IRQ0)
  func_ptr_t            pfnDstcBt2Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT2_IRQ1)
  func_ptr_t            pfnDstcBt2Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT3_IRQ0)
  func_ptr_t            pfnDstcBt3Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT3_IRQ1)
  func_ptr_t            pfnDstcBt3Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT4_IRQ0)
  func_ptr_t            pfnDstcBt4Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT4_IRQ1)
  func_ptr_t            pfnDstcBt4Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT5_IRQ0)
  func_ptr_t            pfnDstcBt5Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT5_IRQ1)
  func_ptr_t            pfnDstcBt5Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT6_IRQ0)
  func_ptr_t            pfnDstcBt6Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT6_IRQ1)
  func_ptr_t            pfnDstcBt6Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT7_IRQ0)
  func_ptr_t            pfnDstcBt7Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT7_IRQ1)
  func_ptr_t            pfnDstcBt7Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT8_IRQ0)
  func_ptr_t            pfnDstcBt8Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT8_IRQ1)
  func_ptr_t            pfnDstcBt8Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT9_IRQ0)
  func_ptr_t            pfnDstcBt9Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT9_IRQ1)
  func_ptr_t            pfnDstcBt9Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT10_IRQ0)
  func_ptr_t            pfnDstcBt10Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT10_IRQ1)
  func_ptr_t            pfnDstcBt10Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT11_IRQ0)
  func_ptr_t            pfnDstcBt11Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT11_IRQ1)
  func_ptr_t            pfnDstcBt11Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT12_IRQ0)
  func_ptr_t            pfnDstcBt12Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT12_IRQ1)
  func_ptr_t            pfnDstcBt12Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT13_IRQ0)
  func_ptr_t            pfnDstcBt13Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT13_IRQ1)
  func_ptr_t            pfnDstcBt13Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT14_IRQ0)
  func_ptr_t            pfnDstcBt14Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT14_IRQ1)
  func_ptr_t            pfnDstcBt14Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT15_IRQ0)
  func_ptr_t            pfnDstcBt15Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT0)
  func_ptr_t            pfnDstcExint0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT1)
  func_ptr_t            pfnDstcExint1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT2)
  func_ptr_t            pfnDstcExint2Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT3)
  func_ptr_t            pfnDstcExint3Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT4)
  func_ptr_t            pfnDstcExint4Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT5)
  func_ptr_t            pfnDstcExint5Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT6)
  func_ptr_t            pfnDstcExint6Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT7)
  func_ptr_t            pfnDstcExint7Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT8)
  func_ptr_t            pfnDstcExint8Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT9)
  func_ptr_t            pfnDstcExint9Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT10)
  func_ptr_t            pfnDstcExint10Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT11)
  func_ptr_t            pfnDstcExint11Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT12)
  func_ptr_t            pfnDstcExint12Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT13)
  func_ptr_t            pfnDstcExint13Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT14)
  func_ptr_t            pfnDstcExint14Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT15)
  func_ptr_t            pfnDstcExint15Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT16)
  func_ptr_t            pfnDstcExint16Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT17)
  func_ptr_t            pfnDstcExint17Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT18)
  func_ptr_t            pfnDstcExint18Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT19)
  func_ptr_t            pfnDstcExint19Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT20)
  func_ptr_t            pfnDstcExint20Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT21)
  func_ptr_t            pfnDstcExint21Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT22)
  func_ptr_t            pfnDstcExint22Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT23)
  func_ptr_t            pfnDstcExint23Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT24)
  func_ptr_t            pfnDstcExint24Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT25)
  func_ptr_t            pfnDstcExint25Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT26)
  func_ptr_t            pfnDstcExint26Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT27)
  func_ptr_t            pfnDstcExint27Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT28)
  func_ptr_t            pfnDstcExint28Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT29)
  func_ptr_t            pfnDstcExint29Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT30)
  func_ptr_t            pfnDstcExint30Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT31)
  func_ptr_t            pfnDstcExint31Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS0_RX)
  func_ptr_t            pfnDstcMfs0RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS0_TX)
  func_ptr_t            pfnDstcMfs0TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS1_RX)
  func_ptr_t            pfnDstcMfs1RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS1_TX)
  func_ptr_t            pfnDstcMfs1TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS2_RX)
  func_ptr_t            pfnDstcMfs2RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS2_TX)
  func_ptr_t            pfnDstcMfs2TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS3_RX)
  func_ptr_t            pfnDstcMfs3RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS3_TX)
  func_ptr_t            pfnDstcMfs3TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS4_RX)
  func_ptr_t            pfnDstcMfs4RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS4_TX)
  func_ptr_t            pfnDstcMfs4TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS5_RX)
  func_ptr_t            pfnDstcMfs5RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS5_TX)
  func_ptr_t            pfnDstcMfs5TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS6_RX)
  func_ptr_t            pfnDstcMfs6RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS6_TX)
  func_ptr_t            pfnDstcMfs6TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS7_RX)
  func_ptr_t            pfnDstcMfs7RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS7_TX)
  func_ptr_t            pfnDstcMfs7TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS8_RX)
  func_ptr_t            pfnDstcMfs8RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS8_TX)
  func_ptr_t            pfnDstcMfs8TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS9_RX)
  func_ptr_t            pfnDstcMfs9RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS9_TX)
  func_ptr_t            pfnDstcMfs9TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS10_RX)
  func_ptr_t            pfnDstcMfs10RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS10_TX)
  func_ptr_t            pfnDstcMfs10TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS11_RX)
  func_ptr_t            pfnDstcMfs11RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS11_TX)
  func_ptr_t            pfnDstcMfs11TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS12_RX)
  func_ptr_t            pfnDstcMfs12RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS12_TX)
  func_ptr_t            pfnDstcMfs12TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS13_RX)
  func_ptr_t            pfnDstcMfs13RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS13_TX)
  func_ptr_t            pfnDstcMfs13TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS14_RX)
  func_ptr_t            pfnDstcMfs14RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS14_TX)
  func_ptr_t            pfnDstcMfs14TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS15_RX)
  func_ptr_t            pfnDstcMfs15RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS15_TX)
  func_ptr_t            pfnDstcMfs15TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_FRT0_PEAK)
  func_ptr_t            pfnDstcMft0Frt0PeakCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_FRT0_ZERO)
  func_ptr_t            pfnDstcMft0Frt0ZeroCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_FRT1_PEAK)
  func_ptr_t            pfnDstcMft0Frt1PeakCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_FRT1_ZERO)
  func_ptr_t            pfnDstcMft0Frt1ZeroCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_FRT2_PEAK)
  func_ptr_t            pfnDstcMft0Frt2PeakCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_FRT2_ZERO)
  func_ptr_t            pfnDstcMft0Frt2ZeroCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_ICU0)
  func_ptr_t            pfnDstcMft0Icu0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_ICU1)
  func_ptr_t            pfnDstcMft0Icu1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_ICU2)
  func_ptr_t            pfnDstcMft0Icu2Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_ICU3)
  func_ptr_t            pfnDstcMft0Icu3Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_OCU0)
  func_ptr_t            pfnDstcMft0Ocu0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_OCU1)
  func_ptr_t            pfnDstcMft0Ocu1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_OCU2)
  func_ptr_t            pfnDstcMft0Ocu2Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_OCU3)
  func_ptr_t            pfnDstcMft0Ocu3Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_OCU4)
  func_ptr_t            pfnDstcMft0Ocu4Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_OCU5)
  func_ptr_t            pfnDstcMft0Ocu5Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_WFG10)
  func_ptr_t            pfnDstcMft0Wfg10Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_WFG32)
  func_ptr_t            pfnDstcMft0Wfg32Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_WFG54)
  func_ptr_t            pfnDstcMft0Wfg54Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_FRT0_PEAK)
  func_ptr_t            pfnDstcMft1Frt0PeakCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_FRT0_ZERO)
  func_ptr_t            pfnDstcMft1Frt0ZeroCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_FRT1_PEAK)
  func_ptr_t            pfnDstcMft1Frt1PeakCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_FRT1_ZERO)
  func_ptr_t            pfnDstcMft1Frt1ZeroCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_FRT2_PEAK)
  func_ptr_t            pfnDstcMft1Frt2PeakCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_FRT2_ZERO)
  func_ptr_t            pfnDstcMft1Frt2ZeroCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_ICU0)
  func_ptr_t            pfnDstcMft1Icu0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_ICU1)
  func_ptr_t            pfnDstcMft1Icu1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_ICU2)
  func_ptr_t            pfnDstcMft1Icu2Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_ICU3)
  func_ptr_t            pfnDstcMft1Icu3Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_OCU0)
  func_ptr_t            pfnDstcMft1Ocu0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_OCU1)
  func_ptr_t            pfnDstcMft1Ocu1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_OCU2)
  func_ptr_t            pfnDstcMft1Ocu2Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_OCU3)
  func_ptr_t            pfnDstcMft1Ocu3Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_OCU4)
  func_ptr_t            pfnDstcMft1Ocu4Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_OCU5)
  func_ptr_t            pfnDstcMft1Ocu5Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_WFG10)
  func_ptr_t            pfnDstcMft1Wfg10Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_WFG32)
  func_ptr_t            pfnDstcMft1Wfg32Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_WFG54)
  func_ptr_t            pfnDstcMft1Wfg54Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_FRT0_PEAK)
  func_ptr_t            pfnDstcMft2Frt0PeakCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_FRT0_ZERO)
  func_ptr_t            pfnDstcMft2Frt0ZeroCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_FRT1_PEAK)
  func_ptr_t            pfnDstcMft2Frt1PeakCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_FRT1_ZERO)
  func_ptr_t            pfnDstcMft2Frt1ZeroCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_FRT2_PEAK)
  func_ptr_t            pfnDstcMft2Frt2PeakCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_FRT2_ZERO)
  func_ptr_t            pfnDstcMft2Frt2ZeroCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_ICU0)
  func_ptr_t            pfnDstcMft2Icu0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_ICU1)
  func_ptr_t            pfnDstcMft2Icu1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_ICU2)
  func_ptr_t            pfnDstcMft2Icu2Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_ICU3)
  func_ptr_t            pfnDstcMft2Icu3Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_OCU0)
  func_ptr_t            pfnDstcMft2Ocu0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_OCU1)
  func_ptr_t            pfnDstcMft2Ocu1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_OCU2)
  func_ptr_t            pfnDstcMft2Ocu2Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_OCU3)
  func_ptr_t            pfnDstcMft2Ocu3Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_OCU4)
  func_ptr_t            pfnDstcMft2Ocu4Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_OCU5)
  func_ptr_t            pfnDstcMft2Ocu5Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_WFG10)
  func_ptr_t            pfnDstcMft2Wfg10Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_WFG32)
  func_ptr_t            pfnDstcMft2Wfg32Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_WFG54)
  func_ptr_t            pfnDstcMft2Wfg54Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_PPG0)
  func_ptr_t            pfnDstcPpg0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_PPG2)
  func_ptr_t            pfnDstcPpg2Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_PPG4)
  func_ptr_t            pfnDstcPpg4Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_PPG8)
  func_ptr_t            pfnDstcPpg8Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_PPG10)
  func_ptr_t            pfnDstcPpg10Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_PPG12)
  func_ptr_t            pfnDstcPpg12Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_PPG16)
  func_ptr_t            pfnDstcPpg16Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_PPG18)
  func_ptr_t            pfnDstcPpg18Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_PPG20)
  func_ptr_t            pfnDstcPpg20Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC0_COUNT_INVERSION)
  func_ptr_t            pfnDstcQprc0CountInversionCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC0_OUT_OF_RANGE)
  func_ptr_t            pfnDstcQprc0OutOfRangeCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC0_PC_MATCH)
  func_ptr_t            pfnDstcQprc0PcMatchCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC0_PC_MATCH_RC_MATCH)
  func_ptr_t            pfnDstcQprc0PcMatchRcMatchCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC0_PC_RC_MATCH)
  func_ptr_t            pfnDstcQprc0PcRcMatchCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC0_UFL_OFL_Z)
  func_ptr_t            pfnDstcQprc0UflOflZCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC1_COUNT_INVERSION)
  func_ptr_t            pfnDstcQprc1CountInversionCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC1_OUT_OF_RANGE)
  func_ptr_t            pfnDstcQprc1OutOfRangeCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC1_PC_MATCH)
  func_ptr_t            pfnDstcQprc1PcMatchCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC1_PC_MATCH_RC_MATCH)
  func_ptr_t            pfnDstcQprc1PcMatchRcMatchCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC1_PC_RC_MATCH)
  func_ptr_t            pfnDstcQprc1PcRcMatchCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC1_UFL_OFL_Z)
  func_ptr_t            pfnDstcQprc1UflOflZCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC2_COUNT_INVERSION)
  func_ptr_t            pfnDstcQprc2CountInversionCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC2_OUT_OF_RANGE)
  func_ptr_t            pfnDstcQprc2OutOfRangeCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC2_PC_MATCH)
  func_ptr_t            pfnDstcQprc2PcMatchCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC2_PC_MATCH_RC_MATCH)
  func_ptr_t            pfnDstcQprc2PcMatchRcMatchCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC2_PC_RC_MATCH)
  func_ptr_t            pfnDstcQprc2PcRcMatchCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC2_UFL_OFL_Z)
  func_ptr_t            pfnDstcQprc2UflOflZCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC3_COUNT_INVERSION)
  func_ptr_t            pfnDstcQprc3CountInversionCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC3_OUT_OF_RANGE)
  func_ptr_t            pfnDstcQprc3OutOfRangeCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC3_PC_MATCH)
  func_ptr_t            pfnDstcQprc3PcMatchCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC3_PC_MATCH_RC_MATCH)
  func_ptr_t            pfnDstcQprc3PcMatchRcMatchCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC3_PC_RC_MATCH)
  func_ptr_t            pfnDstcQprc3PcRcMatchCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC3_UFL_OFL_Z)
  func_ptr_t            pfnDstcQprc3UflOflZCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_USB0_EP1)
  func_ptr_t            pfnDstcUsb0Ep1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_USB0_EP2)
  func_ptr_t            pfnDstcUsb0Ep2Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_USB0_EP3)
  func_ptr_t            pfnDstcUsb0Ep3Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_USB0_EP4)
  func_ptr_t            pfnDstcUsb0Ep4Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_USB0_EP5)
  func_ptr_t            pfnDstcUsb0Ep5Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_USB1_EP1)
  func_ptr_t            pfnDstcUsb1Ep1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_USB1_EP2)
  func_ptr_t            pfnDstcUsb1Ep2Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_USB1_EP3)
  func_ptr_t            pfnDstcUsb1Ep3Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_USB1_EP4)
  func_ptr_t            pfnDstcUsb1Ep4Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_USB1_EP5)
  func_ptr_t            pfnDstcUsb1Ep5Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_WC)
  func_ptr_t            pfnDstcWcCallback;
#endif
} stc_dstc_config_t;

/// DSTC Intern Data
typedef struct stc_dstc_intern_data
{
  func_ptr_t            pfnNotifySwCallback;    ///< Notification SW Callback Function Pointer
  func_ptr_dstc_args_t  pfnErrorCallback;       ///< Error Status Callback
  
#if (PDL_ON == PDL_DSTC_ENABLE_ADC0_PRIO)
  func_ptr_t            pfnDstcAdc0PrioCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_ADC0_SCAN)
  func_ptr_t            pfnDstcAdc0ScanCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_ADC1_PRIO)
  func_ptr_t            pfnDstcAdc1PrioCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_ADC1_SCAN)
  func_ptr_t            pfnDstcAdc1ScanCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_ADC2_PRIO)
  func_ptr_t            pfnDstcAdc2PrioCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_ADC2_SCAN)
  func_ptr_t            pfnDstcAdc2ScanCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT0_IRQ0)
  func_ptr_t            pfnDstcBt0Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT0_IRQ1)
  func_ptr_t            pfnDstcBt0Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT1_IRQ0)
  func_ptr_t            pfnDstcBt1Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT1_IRQ1)
  func_ptr_t            pfnDstcBt1Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT2_IRQ0)
  func_ptr_t            pfnDstcBt2Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT2_IRQ1)
  func_ptr_t            pfnDstcBt2Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT3_IRQ0)
  func_ptr_t            pfnDstcBt3Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT3_IRQ1)
  func_ptr_t            pfnDstcBt3Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT4_IRQ0)
  func_ptr_t            pfnDstcBt4Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT4_IRQ1)
  func_ptr_t            pfnDstcBt4Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT5_IRQ0)
  func_ptr_t            pfnDstcBt5Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT5_IRQ1)
  func_ptr_t            pfnDstcBt5Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT6_IRQ0)
  func_ptr_t            pfnDstcBt6Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT6_IRQ1)
  func_ptr_t            pfnDstcBt6Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT7_IRQ0)
  func_ptr_t            pfnDstcBt7Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT7_IRQ1)
  func_ptr_t            pfnDstcBt7Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT8_IRQ0)
  func_ptr_t            pfnDstcBt8Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT8_IRQ1)
  func_ptr_t            pfnDstcBt8Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT9_IRQ0)
  func_ptr_t            pfnDstcBt9Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT9_IRQ1)
  func_ptr_t            pfnDstcBt9Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT10_IRQ0)
  func_ptr_t            pfnDstcBt10Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT10_IRQ1)
  func_ptr_t            pfnDstcBt10Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT11_IRQ0)
  func_ptr_t            pfnDstcBt11Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT11_IRQ1)
  func_ptr_t            pfnDstcBt11Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT12_IRQ0)
  func_ptr_t            pfnDstcBt12Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT12_IRQ1)
  func_ptr_t            pfnDstcBt12Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT13_IRQ0)
  func_ptr_t            pfnDstcBt13Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT13_IRQ1)
  func_ptr_t            pfnDstcBt13Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT14_IRQ0)
  func_ptr_t            pfnDstcBt14Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT14_IRQ1)
  func_ptr_t            pfnDstcBt14Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT15_IRQ0)
  func_ptr_t            pfnDstcBt15Irq0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_BT15_IRQ1)
  func_ptr_t            pfnDstcBt15Irq1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT0)
  func_ptr_t            pfnDstcExint0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT1)
  func_ptr_t            pfnDstcExint1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT2)
  func_ptr_t            pfnDstcExint2Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT3)
  func_ptr_t            pfnDstcExint3Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT4)
  func_ptr_t            pfnDstcExint4Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT5)
  func_ptr_t            pfnDstcExint5Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT6)
  func_ptr_t            pfnDstcExint6Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT7)
  func_ptr_t            pfnDstcExint7Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT8)
  func_ptr_t            pfnDstcExint8Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT9)
  func_ptr_t            pfnDstcExint9Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT10)
  func_ptr_t            pfnDstcExint10Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT11)
  func_ptr_t            pfnDstcExint11Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT12)
  func_ptr_t            pfnDstcExint12Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT13)
  func_ptr_t            pfnDstcExint13Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT14)
  func_ptr_t            pfnDstcExint14Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT15)
  func_ptr_t            pfnDstcExint15Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT16)
  func_ptr_t            pfnDstcExint16Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT17)
  func_ptr_t            pfnDstcExint17Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT18)
  func_ptr_t            pfnDstcExint18Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT19)
  func_ptr_t            pfnDstcExint19Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT20)
  func_ptr_t            pfnDstcExint20Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT21)
  func_ptr_t            pfnDstcExint21Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT22)
  func_ptr_t            pfnDstcExint22Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT23)
  func_ptr_t            pfnDstcExint23Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT24)
  func_ptr_t            pfnDstcExint24Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT25)
  func_ptr_t            pfnDstcExint25Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT26)
  func_ptr_t            pfnDstcExint26Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT27)
  func_ptr_t            pfnDstcExint27Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT28)
  func_ptr_t            pfnDstcExint28Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT29)
  func_ptr_t            pfnDstcExint29Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT30)
  func_ptr_t            pfnDstcExint30Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_EXINT31)
  func_ptr_t            pfnDstcExint31Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS0_RX)
  func_ptr_t            pfnDstcMfs0RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS0_TX)
  func_ptr_t            pfnDstcMfs0TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS1_RX)
  func_ptr_t            pfnDstcMfs1RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS1_TX)
  func_ptr_t            pfnDstcMfs1TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS2_RX)
  func_ptr_t            pfnDstcMfs2RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS2_TX)
  func_ptr_t            pfnDstcMfs2TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS3_RX)
  func_ptr_t            pfnDstcMfs3RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS3_TX)
  func_ptr_t            pfnDstcMfs3TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS4_RX)
  func_ptr_t            pfnDstcMfs4RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS4_TX)
  func_ptr_t            pfnDstcMfs4TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS5_RX)
  func_ptr_t            pfnDstcMfs5RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS5_TX)
  func_ptr_t            pfnDstcMfs5TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS6_RX)
  func_ptr_t            pfnDstcMfs6RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS6_TX)
  func_ptr_t            pfnDstcMfs6TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS7_RX)
  func_ptr_t            pfnDstcMfs7RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS7_TX)
  func_ptr_t            pfnDstcMfs7TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS8_RX)
  func_ptr_t            pfnDstcMfs8RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS8_TX)
  func_ptr_t            pfnDstcMfs8TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS9_RX)
  func_ptr_t            pfnDstcMfs9RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS9_TX)
  func_ptr_t            pfnDstcMfs9TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS10_RX)
  func_ptr_t            pfnDstcMfs10RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS10_TX)
  func_ptr_t            pfnDstcMfs10TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS11_RX)
  func_ptr_t            pfnDstcMfs11RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS11_TX)
  func_ptr_t            pfnDstcMfs11TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS12_RX)
  func_ptr_t            pfnDstcMfs12RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS12_TX)
  func_ptr_t            pfnDstcMfs12TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS13_RX)
  func_ptr_t            pfnDstcMfs13RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS13_TX)
  func_ptr_t            pfnDstcMfs13TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS14_RX)
  func_ptr_t            pfnDstcMfs14RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS14_TX)
  func_ptr_t            pfnDstcMfs14TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS15_RX)
  func_ptr_t            pfnDstcMfs15RxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFS15_TX)
  func_ptr_t            pfnDstcMfs15TxCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_FRT0_PEAK)
  func_ptr_t            pfnDstcMft0Frt0PeakCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_FRT0_ZERO)
  func_ptr_t            pfnDstcMft0Frt0ZeroCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_FRT1_PEAK)
  func_ptr_t            pfnDstcMft0Frt1PeakCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_FRT1_ZERO)
  func_ptr_t            pfnDstcMft0Frt1ZeroCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_FRT2_PEAK)
  func_ptr_t            pfnDstcMft0Frt2PeakCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_FRT2_ZERO)
  func_ptr_t            pfnDstcMft0Frt2ZeroCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_ICU0)
  func_ptr_t            pfnDstcMft0Icu0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_ICU1)
  func_ptr_t            pfnDstcMft0Icu1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_ICU2)
  func_ptr_t            pfnDstcMft0Icu2Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_ICU3)
  func_ptr_t            pfnDstcMft0Icu3Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_OCU0)
  func_ptr_t            pfnDstcMft0Ocu0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_OCU1)
  func_ptr_t            pfnDstcMft0Ocu1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_OCU2)
  func_ptr_t            pfnDstcMft0Ocu2Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_OCU3)
  func_ptr_t            pfnDstcMft0Ocu3Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_OCU4)
  func_ptr_t            pfnDstcMft0Ocu4Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_OCU5)
  func_ptr_t            pfnDstcMft0Ocu5Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_WFG10)
  func_ptr_t            pfnDstcMft0Wfg10Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_WFG32)
  func_ptr_t            pfnDstcMft0Wfg32Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_WFG54)
  func_ptr_t            pfnDstcMft0Wfg54Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_FRT0_PEAK)
  func_ptr_t            pfnDstcMft1Frt0PeakCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_FRT0_ZERO)
  func_ptr_t            pfnDstcMft1Frt0ZeroCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_FRT1_PEAK)
  func_ptr_t            pfnDstcMft1Frt1PeakCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_FRT1_ZERO)
  func_ptr_t            pfnDstcMft1Frt1ZeroCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_FRT2_PEAK)
  func_ptr_t            pfnDstcMft1Frt2PeakCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_FRT2_ZERO)
  func_ptr_t            pfnDstcMft1Frt2ZeroCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_ICU0)
  func_ptr_t            pfnDstcMft1Icu0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_ICU1)
  func_ptr_t            pfnDstcMft1Icu1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_ICU2)
  func_ptr_t            pfnDstcMft1Icu2Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_ICU3)
  func_ptr_t            pfnDstcMft1Icu3Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_OCU0)
  func_ptr_t            pfnDstcMft1Ocu0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_OCU1)
  func_ptr_t            pfnDstcMft1Ocu1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_OCU2)
  func_ptr_t            pfnDstcMft1Ocu2Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_OCU3)
  func_ptr_t            pfnDstcMft1Ocu3Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_OCU4)
  func_ptr_t            pfnDstcMft1Ocu4Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_OCU5)
  func_ptr_t            pfnDstcMft1Ocu5Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_WFG10)
  func_ptr_t            pfnDstcMft1Wfg10Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_WFG32)
  func_ptr_t            pfnDstcMft1Wfg32Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT1_WFG54)
  func_ptr_t            pfnDstcMft1Wfg54Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_FRT0_PEAK)
  func_ptr_t            pfnDstcMft2Frt0PeakCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_FRT0_ZERO)
  func_ptr_t            pfnDstcMft2Frt0ZeroCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_FRT1_PEAK)
  func_ptr_t            pfnDstcMft2Frt1PeakCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_FRT1_ZERO)
  func_ptr_t            pfnDstcMft2Frt1ZeroCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_FRT2_PEAK)
  func_ptr_t            pfnDstcMft2Frt2PeakCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_FRT2_ZERO)
  func_ptr_t            pfnDstcMft2Frt2ZeroCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_ICU0)
  func_ptr_t            pfnDstcMft2Icu0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_ICU1)
  func_ptr_t            pfnDstcMft2Icu1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_ICU2)
  func_ptr_t            pfnDstcMft2Icu2Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_ICU3)
  func_ptr_t            pfnDstcMft2Icu3Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_OCU0)
  func_ptr_t            pfnDstcMft2Ocu0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_OCU1)
  func_ptr_t            pfnDstcMft2Ocu1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_OCU2)
  func_ptr_t            pfnDstcMft2Ocu2Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_OCU3)
  func_ptr_t            pfnDstcMft2Ocu3Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_OCU4)
  func_ptr_t            pfnDstcMft2Ocu4Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_OCU5)
  func_ptr_t            pfnDstcMft2Ocu5Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_WFG10)
  func_ptr_t            pfnDstcMft2Wfg10Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_WFG32)
  func_ptr_t            pfnDstcMft2Wfg32Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_MFT2_WFG54)
  func_ptr_t            pfnDstcMft2Wfg54Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_PPG0)
  func_ptr_t            pfnDstcPpg0Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_PPG2)
  func_ptr_t            pfnDstcPpg2Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_PPG4)
  func_ptr_t            pfnDstcPpg4Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_PPG8)
  func_ptr_t            pfnDstcPpg8Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_PPG10)
  func_ptr_t            pfnDstcPpg10Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_PPG12)
  func_ptr_t            pfnDstcPpg12Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_PPG16)
  func_ptr_t            pfnDstcPpg16Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_PPG18)
  func_ptr_t            pfnDstcPpg18Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_PPG20)
  func_ptr_t            pfnDstcPpg20Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC0_COUNT_INVERSION)
  func_ptr_t            pfnDstcQprc0CountInversionCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC0_OUT_OF_RANGE)
  func_ptr_t            pfnDstcQprc0OutOfRangeCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC0_PC_MATCH)
  func_ptr_t            pfnDstcQprc0PcMatchCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC0_PC_MATCH_RC_MATCH)
  func_ptr_t            pfnDstcQprc0PcMatchRcMatchCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC0_PC_RC_MATCH)
  func_ptr_t            pfnDstcQprc0PcRcMatchCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC0_UFL_OFL_Z)
  func_ptr_t            pfnDstcQprc0UflOflZCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC1_COUNT_INVERSION)
  func_ptr_t            pfnDstcQprc1CountInversionCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC1_OUT_OF_RANGE)
  func_ptr_t            pfnDstcQprc1OutOfRangeCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC1_PC_MATCH)
  func_ptr_t            pfnDstcQprc1PcMatchCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC1_PC_MATCH_RC_MATCH)
  func_ptr_t            pfnDstcQprc1PcMatchRcMatchCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC1_PC_RC_MATCH)
  func_ptr_t            pfnDstcQprc1PcRcMatchCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC1_UFL_OFL_Z)
  func_ptr_t            pfnDstcQprc1UflOflZCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC2_COUNT_INVERSION)
  func_ptr_t            pfnDstcQprc2CountInversionCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC2_OUT_OF_RANGE)
  func_ptr_t            pfnDstcQprc2OutOfRangeCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC2_PC_MATCH)
  func_ptr_t            pfnDstcQprc2PcMatchCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC2_PC_MATCH_RC_MATCH)
  func_ptr_t            pfnDstcQprc2PcMatchRcMatchCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC2_PC_RC_MATCH)
  func_ptr_t            pfnDstcQprc2PcRcMatchCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC2_UFL_OFL_Z)
  func_ptr_t            pfnDstcQprc2UflOflZCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC3_COUNT_INVERSION)
  func_ptr_t            pfnDstcQprc3CountInversionCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC3_OUT_OF_RANGE)
  func_ptr_t            pfnDstcQprc3OutOfRangeCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC3_PC_MATCH)
  func_ptr_t            pfnDstcQprc3PcMatchCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC3_PC_MATCH_RC_MATCH)
  func_ptr_t            pfnDstcQprc3PcMatchRcMatchCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC3_PC_RC_MATCH)
  func_ptr_t            pfnDstcQprc3PcRcMatchCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_QPRC3_UFL_OFL_Z)
  func_ptr_t            pfnDstcQprc3UflOflZCallback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_USB0_EP1)
  func_ptr_t            pfnDstcUsb0Ep1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_USB0_EP2)
  func_ptr_t            pfnDstcUsb0Ep2Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_USB0_EP3)
  func_ptr_t            pfnDstcUsb0Ep3Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_USB0_EP4)
  func_ptr_t            pfnDstcUsb0Ep4Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_USB0_EP5)
  func_ptr_t            pfnDstcUsb0Ep5Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_USB1_EP1)
  func_ptr_t            pfnDstcUsb1Ep1Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_USB1_EP2)
  func_ptr_t            pfnDstcUsb1Ep2Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_USB1_EP3)
  func_ptr_t            pfnDstcUsb1Ep3Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_USB1_EP4)
  func_ptr_t            pfnDstcUsb1Ep4Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_USB1_EP5)
  func_ptr_t            pfnDstcUsb1Ep5Callback;
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_WC)
  func_ptr_t            pfnDstcWcCallback;
#endif
} stc_dstc_intern_data_t;

/******************************************************************************/
/* Global function prototypes ('extern', definition in C source)              */
/******************************************************************************/
// extern en_result_t Dsctc_InitDescriptors( uint32_t u32Des0, ... );
extern void Dstc_IrqHandler( void );
extern en_result_t Dstc_ReleaseStandBy(void);
extern en_result_t Dstc_Init( stc_dstc_config_t* pstcConfig );
extern en_result_t Dstc_DeInit(void);
extern en_result_t Dstc_SetHwdesp( uint8_t  u8Channel, 
                                   uint16_t u16HwDesp );
extern uint16_t    Dstc_ReadHwdesp( uint8_t u8Channel );
extern en_result_t Dstc_SetCommand( en_dstc_cmd_t enCommand );
extern en_result_t Dstc_SwTrigger( uint16_t u16SwDesPointer );
extern en_result_t Dstc_SwTrqansferStartStatus( void );
extern en_result_t Dstc_SetDreqenb( stc_dstc_dreqenb_t* pstcDreqenb );
extern en_result_t Dstc_ReadDreqenb( stc_dstc_dreqenb_t* pstcDreqenb );
extern en_result_t Dstc_SetDreqenbBit( uint8_t u8BitPos );
extern en_result_t Dstc_ClearDreqenbBit( uint8_t u8BitPos );
extern en_result_t Dstc_ReadHwint( stc_dstc_hwint_t* pstcHwint );
extern en_result_t Dstc_SetHwintclr( stc_dstc_hwintclr_t* pstcHwintclr );
extern boolean_t   Dstc_ReadHwintBit( uint8_t u8BitPos );
extern en_result_t Dstc_SetHwintclrBit( uint8_t u8BitPos );
extern en_result_t Dstc_ReadDqmsk( stc_dstc_dqmsk_t* pstcDqmsk );
extern en_result_t Dstc_SetDqmskclr( stc_dstc_dqmskclr_t* pstcDqmskclr );
extern en_result_t Dstc_SetDqmskclrBit( uint8_t u8BitPos );

#if (PDL_ON == PDL_DSTC_ENABLE_ADC0_PRIO) || (PDL_ON == PDL_DSTC_ENABLE_ADC0_SCAN) || \
    (PDL_ON == PDL_DSTC_ENABLE_ADC1_PRIO) || (PDL_ON == PDL_DSTC_ENABLE_ADC1_SCAN) || \
    (PDL_ON == PDL_DSTC_ENABLE_ADC2_PRIO) || (PDL_ON == PDL_DSTC_ENABLE_ADC2_SCAN)
extern void Dstc_AdcIrqHandler(uint8_t u8IrqChannel0, uint8_t u8IrqChannel1);
#endif

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
extern void Dstc_BtIrqHandler(uint8_t u8IrqChannel0, uint8_t u8IrqChannel1);
#endif

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
extern void Dstc_ExintIrqHandler(uint8_t u8IrqChannel);
#endif

#if (PDL_ON == PDL_DSTC_ENABLE_MFS0_RX)  || (PDL_ON == PDL_DSTC_ENABLE_MFS1_RX)  || \
    (PDL_ON == PDL_DSTC_ENABLE_MFS2_RX)  || (PDL_ON == PDL_DSTC_ENABLE_MFS3_RX)  || \
    (PDL_ON == PDL_DSTC_ENABLE_MFS4_RX)  || (PDL_ON == PDL_DSTC_ENABLE_MFS5_RX)  || \
    (PDL_ON == PDL_DSTC_ENABLE_MFS6_RX)  || (PDL_ON == PDL_DSTC_ENABLE_MFS7_RX)  || \
    (PDL_ON == PDL_DSTC_ENABLE_MFS8_RX)  || (PDL_ON == PDL_DSTC_ENABLE_MFS9_RX)  || \
    (PDL_ON == PDL_DSTC_ENABLE_MFS10_RX) || (PDL_ON == PDL_DSTC_ENABLE_MFS11_RX) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFS12_RX) || (PDL_ON == PDL_DSTC_ENABLE_MFS13_RX) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFS14_RX) || (PDL_ON == PDL_DSTC_ENABLE_MFS15_RX)
extern void Dstc_MfsRxIrqHandler(uint8_t u8IrqChannel);
#endif

#if (PDL_ON == PDL_DSTC_ENABLE_MFS0_TX)  || (PDL_ON == PDL_DSTC_ENABLE_MFS1_TX)  || \
    (PDL_ON == PDL_DSTC_ENABLE_MFS2_TX)  || (PDL_ON == PDL_DSTC_ENABLE_MFS3_TX)  || \
    (PDL_ON == PDL_DSTC_ENABLE_MFS4_TX)  || (PDL_ON == PDL_DSTC_ENABLE_MFS5_TX)  || \
    (PDL_ON == PDL_DSTC_ENABLE_MFS6_TX)  || (PDL_ON == PDL_DSTC_ENABLE_MFS7_TX)  || \
    (PDL_ON == PDL_DSTC_ENABLE_MFS8_TX)  || (PDL_ON == PDL_DSTC_ENABLE_MFS9_TX)  || \
    (PDL_ON == PDL_DSTC_ENABLE_MFS10_TX) || (PDL_ON == PDL_DSTC_ENABLE_MFS11_TX) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFS12_TX) || (PDL_ON == PDL_DSTC_ENABLE_MFS13_TX) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFS14_TX) || (PDL_ON == PDL_DSTC_ENABLE_MFS15_TX)
extern void Dstc_MfsTxIrqHandler(uint8_t u8IrqChannel);
#endif

#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_FRT0_PEAK) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT0_FRT1_PEAK) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT0_FRT2_PEAK) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT1_FRT0_PEAK) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT1_FRT1_PEAK) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT1_FRT2_PEAK) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT2_FRT0_PEAK) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT2_FRT1_PEAK) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT2_FRT2_PEAK) 
extern void Dstc_MftFrtPeakIrqHandler(uint8_t u8IrqChannel);
#endif


#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_FRT0_ZERO) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT0_FRT1_ZERO) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT0_FRT2_ZERO) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT1_FRT0_ZERO) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT1_FRT1_ZERO) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT1_FRT2_ZERO) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT2_FRT0_ZERO) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT2_FRT1_ZERO) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT2_FRT2_ZERO) 
extern void Dstc_MftFrtZeroIrqHandler(uint8_t u8IrqChannel);
#endif

#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_OCU0) || (PDL_ON == PDL_DSTC_ENABLE_MFT0_OCU1) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT0_OCU2) || (PDL_ON == PDL_DSTC_ENABLE_MFT0_OCU3) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT0_OCU4) || (PDL_ON == PDL_DSTC_ENABLE_MFT0_OCU5) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT1_OCU0) || (PDL_ON == PDL_DSTC_ENABLE_MFT1_OCU1) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT1_OCU2) || (PDL_ON == PDL_DSTC_ENABLE_MFT1_OCU3) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT1_OCU4) || (PDL_ON == PDL_DSTC_ENABLE_MFT1_OCU5) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT2_OCU0) || (PDL_ON == PDL_DSTC_ENABLE_MFT2_OCU1) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT2_OCU2) || (PDL_ON == PDL_DSTC_ENABLE_MFT2_OCU3) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT2_OCU4) || (PDL_ON == PDL_DSTC_ENABLE_MFT2_OCU5)
extern void Dstc_MftOcuIrqHandler(uint8_t u8IrqChannel);
#endif   

#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_WFG10) || (PDL_ON == PDL_DSTC_ENABLE_MFT0_WFG32) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT0_WFG54) || (PDL_ON == PDL_DSTC_ENABLE_MFT1_WFG10) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT1_WFG32) || (PDL_ON == PDL_DSTC_ENABLE_MFT1_WFG54) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT2_WFG10) || (PDL_ON == PDL_DSTC_ENABLE_MFT2_WFG32) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT2_WFG54)
extern void Dstc_MftWfgIrqHandler(uint8_t u8IrqChannel);
#endif

#if (PDL_ON == PDL_DSTC_ENABLE_MFT0_ICU0) || (PDL_ON == PDL_DSTC_ENABLE_MFT0_ICU1) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT0_ICU2) || (PDL_ON == PDL_DSTC_ENABLE_MFT0_ICU3) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT1_ICU0) || (PDL_ON == PDL_DSTC_ENABLE_MFT1_ICU1) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT1_ICU2) || (PDL_ON == PDL_DSTC_ENABLE_MFT1_ICU3) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT2_ICU0) || (PDL_ON == PDL_DSTC_ENABLE_MFT2_ICU1) || \
    (PDL_ON == PDL_DSTC_ENABLE_MFT2_ICU2) || (PDL_ON == PDL_DSTC_ENABLE_MFT2_ICU3)
extern void Dstc_MftIcuIrqHandler(uint8_t u8IrqChannel);
#endif

#if (PDL_ON == PDL_DSTC_ENABLE_PPG0)  || (PDL_ON == PDL_DSTC_ENABLE_PPG2)  || \
    (PDL_ON == PDL_DSTC_ENABLE_PPG4)  || (PDL_ON == PDL_DSTC_ENABLE_PPG8)  || \
    (PDL_ON == PDL_DSTC_ENABLE_PPG4)  || (PDL_ON == PDL_DSTC_ENABLE_PPG8)  || \
    (PDL_ON == PDL_DSTC_ENABLE_PPG10) || (PDL_ON == PDL_DSTC_ENABLE_PPG12) || \
    (PDL_ON == PDL_DSTC_ENABLE_PPG16) || (PDL_ON == PDL_DSTC_ENABLE_PPG18) || \
    (PDL_ON == PDL_DSTC_ENABLE_PPG20)
extern void Dstc_PpgIrqHandler(uint8_t u8IrqChannel);
#endif
      
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
extern void Dstc_QprcIrqHandler(uint8_t u8IrqChannel);
#endif

#if (PDL_ON == PDL_DSTC_ENABLE_USB0_EP1)
  #error USB0 EP1 DSTC function not implemented yet!
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_USB0_EP2)
  #error USB0 EP2 DSTC function not implemented yet!
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_USB0_EP3)
  #error US0B EP3 DSTC function not implemented yet!
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_USB0_EP4)
  #error USB0 EP4 DSTC function not implemented yet!
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_USB0_EP5)
  #error USB0 EP5 DSTC function not implemented yet!
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_USB1_EP1)
  #error USB1 EP1 DSTC function not implemented yet!
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_USB1_EP2)
  #error USB1 EP2 DSTC function not implemented yet!
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_USB1_EP3)
  #error USB1 EP3 DSTC function not implemented yet!
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_USB1_EP4)
  #error USB1 EP4 DSTC function not implemented yet!
#endif
#if (PDL_ON == PDL_DSTC_ENABLE_USB1_EP5)
  #error USB1 EP5 DSTC function not implemented yet!
#endif

#if (PDL_ON == PDL_DSTC_ENABLE_WC)
extern void Dstc_WcIrqHandler(void);
#endif

//@} // DstcGroup

#ifdef __cplusplus
}
#endif

#endif // #if (defined(PDL_PERIPHERAL_DSTC_ACTIVE))

#endif /* __DSTC_H__ */
/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
