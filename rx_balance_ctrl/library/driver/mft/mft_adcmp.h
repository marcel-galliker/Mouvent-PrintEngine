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
/** \file mft_adcmp.h
 **
 ** Headerfile for MFT functions
 **  
 ** History:
 **   - 2013-04-23  0.1  Edison First version.
 **   - 2013-07-16  0.2  Edison Add comments about how to use ADCMP with APIs 
 **
 ******************************************************************************/

#ifndef __MFT_ADCMP_H__
#define __MFT_ADCMP_H__

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "pdl.h"

#if (defined(PDL_PERIPHERAL_MFT_ADCMP_ACTIVE))

/**
 ******************************************************************************
 ** \defgroup AdcmpGroup ADC Start Compare Unit (ADCMP)
 **
 ** Provided functions of ADCMP module:
 ** 
 ** - Mft_Adcmp_Init()
 ** - Mft_Adcmp_EnableOperation() 
 ** - Mft_Adcmp_DisableOperation()
 ** - Mft_Adcmp_WriteAcmp()
 ** - Mft_Adcmp_ReadAcmp()
 ** - Mft_Adcmp_Fm3_Init()
 ** - Mft_Adcmp_Fm3_EnableOperation()
 ** - Mft_Adcmp_Fm3_DisableOperation()
 ** - Mft_Adcmp_Fm3_WriteAccp()
 ** - Mft_Adcmp_Fm3_ReadAccp()
 ** - Mft_Adcmp_Fm3_WriteAccpdn()
 ** - Mft_Adcmp_Fm3_ReadAccpdn()
 **
 ** \brief How to use ADCMP module
 ** 
 ** Two modes including normal mode and offset mode can be configured for 
 ** ADCMP. This module should be used with ADC. 
 **
 ** Before using ADCMP, a FRT used to connect with applying ADCMP must be 
 ** initialed. For how to configure FRT, see the description in the mft_frt.h. 
 ** In the offset mode, a OCU should also be used and initialed. For how to 
 ** configure OCU, see the description in the mft_ocu.h  
 **
 ** Mft_Adcmp_Init() must be used for configuration of a ADCMP channel with 
 ** a structure of the type #stc_mft_adcmp_config_t. 
 ** 
 ** With Mft_Adcmp_WriteAcmp() the ADC start compare value is set to the value
 ** given in the parameter Mft_Adcmp_WriteAcmp#u16AcmpVal. Whether the compare
 ** value is modified directly depends on buffer function. 
 **
 ** After above setting, calling Mft_Adcmp_EnableOperation() will start ADCMP.
 ** 
 ** When stopping the OCU, use Mft_Adcmp_DisableOperation() to disable 
 ** ADCMP.
 **
 ** In addition, the module is also compatible with FM3 usage.
 **
 ** \brief How to use ADCMP module with FM3 compatibility function
 ** 
 ** Before using ADCMP, a FRT used to connect with applying ADCMP must be 
 ** initialed. For how to configure FRT, see the description in the mft_frt.h. 
 **
 ** Mft_Adcmp_Fm3_Init() must be used for configuration of a ADCMP channel with 
 ** a structure of the type #stc_mft_adcmp_fm3_config_t. 
 **
 ** With Mft_Adcmp_Fm3_WriteAccp() the ADC start up compare value is set to the 
 ** value given in the parameter Mft_Adcmp_WriteAcmp#u16AccpVal. 
 ** With Mft_Adcmp_Fm3_ReadAccp() the ADC start up compare value is read.
 **
 ** With Mft_Adcmp_Fm3_WriteAccpdn() the ADC start down compare value is set 
 ** to the value given in the parameter Mft_Adcmp_WriteAcmp#u16AccpVal. 
 ** With Mft_Adcmp_Fm3_ReadAccpdn() the ADC start down compare value is read.
 **
 ******************************************************************************/ 
//@{
/******************************************************************************/
/* Global pre-processor symbols/macros ('#define')                            */
/******************************************************************************/

#define  MFT_ADCMP_CH0             0
#define  MFT_ADCMP_CH1             1
#define  MFT_ADCMP_CH2             2
#define  MFT_ADCMP_CH3             3
#define  MFT_ADCMP_CH4             4
#define  MFT_ADCMP_CH5             5

#define  MFT_ADCMP_CH10            0
#define  MFT_ADCMP_CH32            1
#define  MFT_ADCMP_CH54            2

#define  MFT_ADCMP_CH_MAX         5
#define  MFT_ADCMP_CPCH_MAX       2
/******************************************************************************
 * Global type definitions
 ******************************************************************************/
#define stc_mftn_adcmp_t   FM4_MFT_ADCMP_TypeDef

#define MFT0_ADCMP  (*(volatile stc_mftn_adcmp_t *) FM4_MFT0_ADCMP_BASE)
#define MFT1_ADCMP  (*(volatile stc_mftn_adcmp_t *) FM4_MFT1_ADCMP_BASE)
#define MFT2_ADCMP  (*(volatile stc_mftn_adcmp_t *) FM4_MFT2_ADCMP_BASE)

/**
 ******************************************************************************
 ** \brief Define Frt channel connect to Adcmp
 ******************************************************************************/  
typedef enum en_adcmp_frt
{
    Frt0ToAdcmp    = 0, ///< connect Frt channel 0 to Icu
    Frt1ToAdcmp    = 1, ///< connect Frt channel 1 to Icu
    Frt2ToAdcmp    = 2, ///< connect Frt channel 2 to Icu
    AdcmpFrtToExt0 = 3, ///< connect extern Frt channel 0 to Icu
    AdcmpFrtToExt1 = 4, ///< connect extern Frt channel 1 to Icu
    
}en_adcmp_frt_t;

/**
 ******************************************************************************
 ** \brief Define Frt channel with FM3 mode connect to Adcmp
 ******************************************************************************/  
typedef enum en_adcmp_fm3_frt
{
    Frt1ToAdcmpFm3    = 1, ///< connect Frt channel 1 to Icu
    Frt2ToAdcmpFm3    = 2, ///< connect Frt channel 2 to Icu
    
}en_adcmp_fm3_frt_t;

/**
 ******************************************************************************
 ** \brief Define Adcmp buffer type
 ******************************************************************************/ 
typedef enum en_adcmp_buf
{
    AdcmpBufDisable     = 0, ///< disable Adcmp buffer function
    AdcmpBufFrtZero     = 1, ///< transfer buffer when counter value of Frt connected= 0x0000
    AdcmpBufFrtPeak     = 2, ///< transfer buffer when counter value of Frt connected= TCCP
    AdcmpBufFrtZeroPeak = 3, ///< transfer buffer both when counter value of Frt connected= 0x0000 and TCCP
  
}en_adcmp_buf_t;

/**
 ******************************************************************************
 ** \brief Define Mft Adcmp output start trig channel
 ******************************************************************************/ 
typedef enum en_adcmp_trig_channel
{
    AdcmpStartTrig0 = 0, ///< Outputs ADC start trigger 0
    AdcmpStartTrig1 = 1, ///< Outputs ADC start trigger 1
    AdcmpStartTrig2 = 2, ///< Outputs ADC start trigger 2
    AdcmpStartTrig3 = 3, ///< Outputs ADC start trigger 3
    AdcmpStartTrig4 = 4, ///< Outputs ADC start trigger 4
    AdcmpStartTrig5 = 5, ///< Outputs ADC start trigger 5
    AdcmpStartTrig6 = 6, ///< Outputs ADC start trigger 6
    AdcmpStartTrig7 = 7, ///< Outputs ADC start trigger 7
  
}en_adcmp_start_trig_t;
/**
 ******************************************************************************
 ** \brief Define Mft Adcmp trig mode
 ******************************************************************************/
typedef enum en_adcmp_trig_sel
{
    AdcmpTrigAdc0Scan = 0, ///< AdcmpStartTrig0
    AdcmpTrigAdc0Prio = 1, ///< AdcmpStartTrig1
    AdcmpTrigAdc1Scan = 2, ///< AdcmpStartTrig2
    AdcmpTrigAdc1Prio = 3, ///< AdcmpStartTrig3
    AdcmpTrigAdc2Scan = 4, ///< AdcmpStartTrig4
    AdcmpTrigAdc2Prio = 5, ///< AdcmpStartTrig5
    
}en_adcmp_trig_sel_t;
/**
 ******************************************************************************
 ** \brief Define Adcmp running mode
 ******************************************************************************/ 
typedef enum en_adcmp_mode
{
    AdcmpNormalMode = 0, ///< select Adcmp Normal mode
    AdcmpOffsetMode = 1, ///< select Adcmp Offset mode
    
}en_adcmp_mode_t;
/**
 ******************************************************************************
 ** \brief Define Occp channel
 ******************************************************************************/ 
typedef enum en_adcmp_occp_sel
{
    AdcmpSelOccp0 = 0, ///< select Occp0 channel
    AdcmpSelOccp1 = 1, ///< select Occp1 channel
  
}en_adcmp_occp_sel_t;
/**
 ******************************************************************************
 ** \brief Define Mft_adcmp configure parameters
 ******************************************************************************/ 
typedef struct stc_mft_adcmp_config
{
    en_adcmp_frt_t        enFrt;         ///< configure Adcmp Frt channel
    en_adcmp_buf_t        enBuf;         ///< configure Adcmp Buffer transfer type
    en_adcmp_trig_sel_t   enTrigSel;     ///< configure Adcmp Trigger type
    en_adcmp_mode_t       enMode;        ///< configure Adcmp Running mode
    en_adcmp_occp_sel_t   enOccpSel;     ///< select Adcmp Occp channel
  
}stc_mft_adcmp_config_t;
/**
 ******************************************************************************
 ** \brief Define Mft_adcmp functions
 ******************************************************************************/ 
typedef struct stc_mft_adcmp_func
{
    boolean_t   bDownEn;  ///< Enable Adcmp Down function
    boolean_t   bPeakEn;  ///< Enable Adcmp Peak function
    boolean_t   bUpEn;    ///< Enable Adcmp Up function
    boolean_t   bZeroEn;  ///< Enable Adcmp Zero function
  
}stc_mft_adcmp_func_t;
/**
 ******************************************************************************
 ** \brief Define Mft_adcmp compatible fm3 mode
 ******************************************************************************/
typedef enum en_adcmp_fm3_mode
{
    AdcmpFm3AccpUpAccpDown   = 0, ///< Fm3 compatible mode: Accp Up and Down
    AdcmpFm3AccpUp           = 1, ///< Fm3 compatible mode: Accp Up
    AdcmpFm3AccpDown         = 2, ///< Fm3 compatible mode: Accp Down
    AdcmpFm3AccpUpAccpdnDown = 3, ///< Fm3 compatible mode: Accp up adn Accpdn Down
    
}en_adcmp_fm3_mode_t;
/**
 ******************************************************************************
 ** \brief Define Mft_adcmp compatible fm3 configure parameters
 ******************************************************************************/
typedef struct stc_mft_adcmp_fm3_config
{
    en_adcmp_fm3_frt_t     enFrt;     ///< configure Adcmp Frt channel
    en_adcmp_fm3_mode_t    enMode;    ///< configure compatible Fm3 mode
    en_adcmp_buf_t         enBuf;     ///< configure Adcmp Buffer transfer type
    en_adcmp_trig_sel_t    enTrigSel; ///< Select trig mode
    
}stc_mft_adcmp_fm3_config_t;
/**
 ******************************************************************************
 ** \brief Global function prototypes (definition in C source)
 ******************************************************************************/
/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/* 1. FM4 */
/* Init */
en_result_t Mft_Adcmp_Init (volatile stc_mftn_adcmp_t *pstcMftAdcmp,  
                            uint8_t u8Ch, 
                            stc_mft_adcmp_config_t *pstcConfig);
/* Func enable/disable */
en_result_t Mft_Adcmp_EnableOperation(volatile stc_mftn_adcmp_t *pstcMftAdcmp,  
                                      uint8_t u8Ch, 
                                      stc_mft_adcmp_func_t* pstcFunc);
en_result_t Mft_Adcmp_DisableOperation(volatile stc_mftn_adcmp_t *pstcMftAdcmp,  
                                       uint8_t u8Ch, 
                                       stc_mft_adcmp_func_t* pstcFunc);
/* Count write/read */
en_result_t Mft_Adcmp_WriteAcmp(volatile stc_mftn_adcmp_t *pstcMftAdcmp, uint8_t u8Ch, 
                                uint16_t u16AcmpVal);
uint16_t    Mft_Adcmp_ReadAcmp(volatile stc_mftn_adcmp_t *pstcMftAdcmp, uint8_t u8Ch);

/* 2. FM3 compatible */
/* Init */
en_result_t Mft_Adcmp_Fm3_Init(volatile stc_mftn_adcmp_t *pstcMftAdcmp, 
                               uint8_t u8CoupleCh,
                               stc_mft_adcmp_fm3_config_t *pstcConfig);
/* Mode configuration */
en_result_t Mft_Adcmp_Fm3_EnableOperation(volatile stc_mftn_adcmp_t *pstcMftAdcmp, uint8_t u8CoupleCh);
en_result_t Mft_Adcmp_Fm3_DisableOperation(volatile stc_mftn_adcmp_t *pstcMftAdcmp, uint8_t u8CoupleCh);
/* Count write/read */
en_result_t Mft_Adcmp_Fm3_WriteAccp(volatile stc_mftn_adcmp_t *pstcMftAdcmp, uint8_t u8CoupleCh, uint16_t u16AccpVal);
uint16_t Mft_Adcmp_Fm3_ReadAccp(volatile stc_mftn_adcmp_t *pstcMftAdcmp, uint8_t u8CoupleCh);
en_result_t Mft_Adcmp_Fm3_WriteAccpdn(volatile stc_mftn_adcmp_t *pstcMftAdcmp, uint8_t u8CoupleCh, uint16_t u16AccpdnVal);
uint16_t Mft_Adcmp_Fm3_ReadAccpdn(volatile stc_mftn_adcmp_t *pstcMftAdcmp, uint8_t u8CoupleCh);
#ifdef __cplusplus
}
#endif

//@}

#endif

#endif




/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
