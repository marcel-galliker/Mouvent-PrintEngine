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
/** \file exint.h
 **
 ** Headerfile for EXINT functions
 **  
 ** History:
 **   - 2013-08-16  1.0  MWi  First version.
 **   - 2013-10-25  1.1  MWi  PDL_ITERRUPT_ENABLE_x corrected
 **
 ******************************************************************************/

#ifndef __EXINT_H__
#define __EXINT_H__

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "pdl.h"

#if (defined(PDL_PERIPHERAL_EXINT_ACTIVE))


/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif
  
/**
 ******************************************************************************
 ** \defgroup ExintGroup External Interrupts (EXINT)
 **
 ** Provided functions of EXINT module:
 ** 
 ** - Exint_Init()
 ** - Exint_DeInit()
 ** - Exint_EnableChannel()
 ** - Exint_DisableChannel()
 ** - Exint_Nmi_Init()
 ** - Exint_Nmi_DeInit()
 **
 ** \brief How to use External Interrupts
 **
 ** With Exint_Init() the external interrupts are enabled as given in the
 ** configuration. Note, that both external interrupt groups (0-7 and 8-15)
 ** can be enabled individually.
 **
 ** If a single channel may be disabled and reenabled during runtime the
 ** functions Exint_DisableChannel() and Exint_EnableChannel() can be
 ** used.
 **
 ** The external interrupts are disabled globally by Exint_DeInit().
 **
 ** Each channel provides an individually callback function which is handled
 ** in the interrupt service routine.
 **
 ** Exint_Nmi_Init() "initializes" the None-Maskable Interrupt, which means
 ** that the callback function is set to the dedicated internal data.
 **
 ******************************************************************************/
//@{
  
/******************************************************************************/
/* Global pre-processor symbols/macros ('#define')                            */
/******************************************************************************/
#define PDL_EXINT_CHANNELS (uint8_t)(PDL_INTERRUPT_ENABLE_EXINT0  == PDL_ON) + \
                           (uint8_t)(PDL_INTERRUPT_ENABLE_EXINT1  == PDL_ON) + \
                           (uint8_t)(PDL_INTERRUPT_ENABLE_EXINT2  == PDL_ON) + \
                           (uint8_t)(PDL_INTERRUPT_ENABLE_EXINT3  == PDL_ON) + \
                           (uint8_t)(PDL_INTERRUPT_ENABLE_EXINT4  == PDL_ON) + \
                           (uint8_t)(PDL_INTERRUPT_ENABLE_EXINT5  == PDL_ON) + \
                           (uint8_t)(PDL_INTERRUPT_ENABLE_EXINT6  == PDL_ON) + \
                           (uint8_t)(PDL_INTERRUPT_ENABLE_EXINT7  == PDL_ON) + \
                           (uint8_t)(PDL_INTERRUPT_ENABLE_EXINT8  == PDL_ON) + \
                           (uint8_t)(PDL_INTERRUPT_ENABLE_EXINT9  == PDL_ON) + \
                           (uint8_t)(PDL_INTERRUPT_ENABLE_EXINT10 == PDL_ON) + \
                           (uint8_t)(PDL_INTERRUPT_ENABLE_EXINT11 == PDL_ON) + \
                           (uint8_t)(PDL_INTERRUPT_ENABLE_EXINT12 == PDL_ON) + \
                           (uint8_t)(PDL_INTERRUPT_ENABLE_EXINT13 == PDL_ON) + \
                           (uint8_t)(PDL_INTERRUPT_ENABLE_EXINT14 == PDL_ON) + \
                           (uint8_t)(PDL_INTERRUPT_ENABLE_EXINT15 == PDL_ON) + \
                           (uint8_t)(4u*(PDL_INTERRUPT_ENABLE_EXINT16_17_18_19 == PDL_ON)) + \
                           (uint8_t)(4u*(PDL_INTERRUPT_ENABLE_EXINT20_21_22_23 == PDL_ON)) + \
                           (uint8_t)(4u*(PDL_INTERRUPT_ENABLE_EXINT24_25_26_27 == PDL_ON)) + \
                           (uint8_t)(4u*(PDL_INTERRUPT_ENABLE_EXINT28_29_30_31 == PDL_ON)) 
  
#define EXINT0_BITPOS      0x00000001u
#define EXINT1_BITPOS      0x00000002u
#define EXINT2_BITPOS      0x00000004u
#define EXINT3_BITPOS      0x00000008u
#define EXINT4_BITPOS      0x00000010u
#define EXINT5_BITPOS      0x00000020u
#define EXINT6_BITPOS      0x00000040u
#define EXINT7_BITPOS      0x00000080u
#define EXINT8_BITPOS      0x00000100u
#define EXINT9_BITPOS      0x00000200u
#define EXINT10_BITPOS     0x00000400u
#define EXINT11_BITPOS     0x00000800u
#define EXINT12_BITPOS     0x00001000u
#define EXINT13_BITPOS     0x00002000u
#define EXINT14_BITPOS     0x00004000u
#define EXINT15_BITPOS     0x00008000u
#define EXINT16_BITPOS     0x00010000u
#define EXINT17_BITPOS     0x00020000u
#define EXINT18_BITPOS     0x00040000u
#define EXINT19_BITPOS     0x00080000u
#define EXINT20_BITPOS     0x00100000u
#define EXINT21_BITPOS     0x00200000u
#define EXINT22_BITPOS     0x00400000u
#define EXINT23_BITPOS     0x00800000u
#define EXINT24_BITPOS     0x01000000u
#define EXINT25_BITPOS     0x02000000u
#define EXINT26_BITPOS     0x04000000u
#define EXINT27_BITPOS     0x08000000u
#define EXINT28_BITPOS     0x10000000u
#define EXINT29_BITPOS     0x20000000u
#define EXINT30_BITPOS     0x40000000u
#define EXINT31_BITPOS     0x80000000u

#define EXINT0_BITMASK     0xFFFFFFFEu
#define EXINT1_BITMASK     0xFFFFFFFDu
#define EXINT2_BITMASK     0xFFFFFFFBu
#define EXINT3_BITMASK     0xFFFFFFF7u
#define EXINT4_BITMASK     0xFFFFFFEFu
#define EXINT5_BITMASK     0xFFFFFFDFu
#define EXINT6_BITMASK     0xFFFFFFBFu
#define EXINT7_BITMASK     0FFFFFFF7Fu
#define EXINT8_BITMASK     0xFFFFFEFFu
#define EXINT9_BITMASK     0xFFFFFDFFu
#define EXINT10_BITMASK    0xFFFFFBFFu
#define EXINT11_BITMASK    0xFFFFF7FFu
#define EXINT12_BITMASK    0xFFFFEFFFu
#define EXINT13_BITMASK    0xFFFFDFFFu
#define EXINT14_BITMASK    0xFFFFBFFFu
#define EXINT15_BITMASK    0xFFFF7FFFu
#define EXINT16_BITMASK    0xFFFEFFFFu
#define EXINT17_BITMASK    0xFFFDFFFFu
#define EXINT18_BITMASK    0xFFFBFFFFu
#define EXINT19_BITMASK    0xFFF7FFFFu
#define EXINT20_BITMASK    0xFFEFFFFFu
#define EXINT21_BITMASK    0xFFDFFFFFu
#define EXINT22_BITMASK    0xFFBFFFFFu
#define EXINT23_BITMASK    0xFF7FFFFFu
#define EXINT24_BITMASK    0xFEFFFFFFu
#define EXINT25_BITMASK    0xFDFFFFFFu
#define EXINT26_BITMASK    0xFBFFFFFFu
#define EXINT27_BITMASK    0xF7FFFFFFu
#define EXINT28_BITMASK    0xEFFFFFFFu
#define EXINT29_BITMASK    0xDFFFFFFFu
#define EXINT30_BITMASK    0xBFFFFFFFu
#define EXINT31_BITMASK    0x7FFFFFFFu

/******************************************************************************/
/* Local type definitions ('typedef')                                         */
/******************************************************************************/
  
typedef enum en_exint_level
{
  ExIntLowLevel     = 0,  ///< "L" level detection
  ExIntHighLevel    = 1,  ///< "H" level detection
  ExIntRisingEdge   = 2,  ///< Rising edge detection
  ExIntFallingEdge  = 3   ///< Falling edge detection
} en_exint_level_t;

/**
 ******************************************************************************
 ** \brief EXINT configuration
 ******************************************************************************/

/**
 *****************************************************************************
 ** \brief External Interrupt configuration
 ** 
 ** The EXINT configuration
 *****************************************************************************/
typedef struct stc_exint_config
{
  boolean_t         abEnable[32u];           ///< TRUE: External Interrupt enable
  en_exint_level_t  aenLevel[32u];           ///< level detection, see #en_exint_level_t for details
  func_ptr_t        apfnExintCallback[32u];  ///< Callback pointers
} stc_exint_config_t;

/**
 *****************************************************************************
 ** \brief NMI configuration
 ** 
 ** The NMI configuration
 *****************************************************************************/
typedef struct stc_exint_nmi_config
{
  boolean_t         bTouchNVIC;
  func_ptr_t        pfnNMICallback;   ///< NMI Callback pointers
} stc_exint_nmi_config_t;

/**
 ******************************************************************************
 ** \brief Datatype for holding internal data needed for EXINT
 ******************************************************************************/
typedef struct stc_exint_intern_data
{
  func_ptr_t    apfnExintCallback[PDL_EXINT_CHANNELS];  ///< Callback pointer array
} stc_exint_intern_data_t ;

/**
 ******************************************************************************
 ** \brief Datatype for holding internal data needed for NMI
 ******************************************************************************/
typedef struct stc_exint_nmi_intern_data
{
  func_ptr_t    pfnNMICallback;  ///< Callback pointer
} stc_exint_nmi_intern_data_t ;


/******************************************************************************/
/* Global variable definitions ('extern')                                     */
/******************************************************************************/
extern stc_exint_intern_data_t stcExintInternData;
extern stc_exint_nmi_intern_data_t stcExintNMIInternData;

/******************************************************************************/
/* Global function prototypes (definition in C source)                        */
/******************************************************************************/
 
extern void Exint_IrqHandler(uint8_t u8Channel) ;

extern void ExintNMIrqHandler(void) ;

en_result_t Exint_Init(stc_exint_config_t* pstcConfig) ;

en_result_t Exint_DeInit(void) ;

en_result_t Exint_EnableChannel(uint8_t u8Channel) ;

en_result_t Exint_DisableChannel(uint8_t u8Channel) ;

en_result_t Exint_Nmi_Init(stc_exint_nmi_config_t* pstcConfig) ;

en_result_t Exint_Nmi_DeInit(stc_exint_nmi_config_t* pstcConfig) ;
  
//@} // ExintGroup

#ifdef __cplusplus
}
#endif

#endif // #if (defined(PDL_PERIPHERAL_EXINT_ACTIVE))

#endif /* __EXINT_H__ */
/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
