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
/** \file mft_frt.h
 **
 ** Headerfile for MFT's Free Run Timer functions
 **  
 ** History:
 **   - 2013-03-21  0.1  Edison  First version.
 **   - 2013-07-16  0.2  Edison  Add comments about how to use FRT with APIs 
 **
 *****************************************************************************/

#ifndef __MFT_FRT_H__
#define __MFT_FRT_H__

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "pdl.h"

#if (defined(PDL_PERIPHERAL_MFT_FRT_ACTIVE))

/**
 ******************************************************************************
 ** \defgroup FrtGroup Free-Run Timer (FRT)
 **
 ** Provided functions of FRT module:
 ** 
 ** - Mft_Frt_Init()
 ** - Mft_Frt_SetMaskZeroTimes() 
 ** - Mft_Frt_GetMaskZeroTimes()
 ** - Mft_Frt_SetMaskPeakTimes()
 ** - Mft_Frt_GetMaskPeakTimes()
 ** - Mft_Frt_Start()
 ** - Mft_Frt_Stop()
 ** - Mft_Frt_EnableInt()
 ** - Mft_Frt_DisableInt()
 ** - Mft_Frt_GetIntFlag()
 ** - Mft_Frt_ClrIntFlag()
 ** - Mft_Frt_SetCountCycle()
 ** - Mft_Frt_SetCountVal()
 ** - Mft_Frt_GetCurCount()
 **
 ** \brief How to use FRT module
 **
 ** Mft_Frt_Init() must be used for configuration of a Free-Run timer (FRT) 
 ** channel with a structure of the type #stc_mft_frt_config_t.
 ** 
 ** Mft_Frt_SetMaskZeroTimes() is used to set the mask times of Zero match
 ** interrupt. Mft_Frt_GetMaskZeroTimes() is used to get the mask times of 
 ** peak match interrupt. 
 **
 ** Mft_Frt_SetMaskPeakTimes() is used to set the mask times of peak match
 ** interrupt. Mft_Frt_GetMaskPeakTimes() is used to get the mask times of 
 ** peak match interrupt.
 **
 ** A FRT interrupt can be enabled by the function Mft_Frt_EnableInt().
 ** This function can set callback function for each channel too.
 **
 ** With Mft_Frt_SetCountCycle() the FRT cycle is set to the value
 ** given in the parameter Mft_Frt_SetCountCycle#u16Cycle. And the initial
 ** count value can be modified by Mft_Frt_SetCountVal().
 **
 ** After above setting, calling Mft_Frt_Start() will start FRT.
 **
 ** With Mft_Frt_GetCurCount() the current FRT count can be read when FRT is 
 ** running.
 **
 ** With interrupt mode, when the interrupt occurs, the interrupt flag will
 ** be cleared and run into user interrupt callback function.
 **
 ** With polling mode, user can use Mft_Frt_GetIntFlag() to check if the 
 ** interrupt occurs, and clear the interrupt flag by Mft_Frt_ClrIntFlag().
 ** 
 ** When stopping the FRT, use Mft_Frt_Stop() to disable FRT and 
 ** Mft_Frt_DisableInt() to disable FRT interrupt.
 **
 ******************************************************************************/

//@{

/******************************************************************************
 * Global definitions
 ******************************************************************************/
#define stc_mftn_frt_t FM4_MFT_FRT_TypeDef
#define MFT0_FRT       (*((volatile stc_mftn_frt_t *) FM4_MFT0_FRT_BASE))
#define MFT1_FRT       (*((volatile stc_mftn_frt_t *) FM4_MFT1_FRT_BASE))
#define MFT2_FRT       (*((volatile stc_mftn_frt_t *) FM4_MFT2_FRT_BASE))

/**
 ******************************************************************************
 ** \brief MFT Frt channel definition
 ******************************************************************************/  
#define  MFT_FRT_CH0             0
#define  MFT_FRT_CH1             1
#define  MFT_FRT_CH2             2
   
#define  MFT_FRT_MAX_CH          3  

/**
 ******************************************************************************
 ** \brief Enumeration to define an index for each FRT instance
 ******************************************************************************/   
typedef enum en_frt_instance_index
{
    FrtInstanceIndexFrt0 = 0,  ///< Instance index of FRT0
    FrtInstanceIndexFrt1,      ///< Instance index of FRT1
    FrtInstanceIndexFrt2,      ///< Instance index of FRT2

} en_frt_instance_index_t;
   
/**
 ******************************************************************************
 ** \brief Mft Clock Setting
 ******************************************************************************/
typedef enum en_mft_frt_clock
{
    FrtPclkDiv1   = 0,    ///< FRT clock: PCLK
    FrtPclkDiv2   = 1,    ///< FRT clock: PCLK/2
    FrtPclkDiv4   = 2,    ///< FRT clock: PCLK/4
    FrtPclkDiv8   = 3,    ///< FRT clock: PCLK/8
    FrtPclkDiv16  = 4,    ///< FRT clock: PCLK/16
    FrtPclkDiv32  = 5,    ///< FRT clock: PCLK/32
    FrtPclkDiv64  = 6,    ///< FRT clock: PCLK/64
    FrtPclkDiv128 = 7,    ///< FRT clock: PCLK/128
    FrtPclkDiv256 = 8,    ///< FRT clock: PCLK/256
    FrtPclkDiv512 = 9,    ///< FRT clock: PCLK/512
    FrtPclkDiv1024 = 10,    ///< FRT clock: PCLK/1024
} en_mft_frt_clock_t;
  
/**
 ******************************************************************************
 ** \brief FRT count mode
 ******************************************************************************/
typedef enum en_mft_frt_mode
{
    FrtUpCount     = 0,    ///< FRT up-count mode
    FrtUpDownCount = 1     ///< FRT up-/down-count mode
} en_mft_frt_mode_t;

/**
 ******************************************************************************
 ** \brief FRT configure
 ******************************************************************************/
typedef struct stc_mft_frt_config
{
    en_mft_frt_clock_t  enFrtClockDiv;  ///< FRT clock divide
    en_mft_frt_mode_t   enFrtMode;      ///< FRT count mode
    boolean_t           bEnBuffer;      ///< enable buffer
    boolean_t           bEnExtClock;    ///< enable external clock
    
}stc_mft_frt_config_t;

/**
 ******************************************************************************
 ** \brief Enumeration of FRT interrupt selection
 ******************************************************************************/
typedef struct stc_frt_int_sel
{
    boolean_t bFrtZeroMatchInt;       ///<  zero match interrupt selection
    boolean_t bFrtPeakMatchInt;       ///<  peak match interrupt selection
  
}stc_frt_int_sel_t;

/**
 ******************************************************************************
 ** \brief Enumeration of FRT interrupt index
 ******************************************************************************/
typedef enum   en_mft_frt_int
{
    enFrtZeroMatchInt = 0,           ///<  zero match interrupt index
    enFrtPeakMatchInt,               ///<  peak match interrupt index
  
}en_mft_frt_int_t;
    
/**
 ******************************************************************************
 ** \brief FRT interrupt callback function
 ******************************************************************************/
typedef struct stc_frt_int_cb
{
    func_ptr_t  pfnFrtZeroCallback; ///< match zero interrupt callback function
    func_ptr_t  pfnFrtPeakCallback; ///< match peak interrupt callback function
  
}stc_frt_int_cb_t;

/**
 ******************************************************************************
 ** \brief FRT instance internal data, storing internal
 **        information for each enabled FRT instance.
 ******************************************************************************/
typedef struct stc_mft_frt_intern_data
{
    func_ptr_t  pfnFrt0PeakCallback;  ///< Callback function pointer of FRT0 peak detection interrupt 
    func_ptr_t  pfnFrt1PeakCallback;  ///< Callback function pointer of FRT1 peak detection interrupt 
    func_ptr_t  pfnFrt2PeakCallback;  ///< Callback function pointer of FRT2 peak detection interrupt 
    func_ptr_t  pfnFrt0ZeroCallback;  ///< Callback function pointer of FRT0 zero detection interrupt 
    func_ptr_t  pfnFrt1ZeroCallback;  ///< Callback function pointer of FRT1 zero detection interrupt 
    func_ptr_t  pfnFrt2ZeroCallback;  ///< Callback function pointer of FRT2 zero detection interrupt 
}stc_mft_frt_intern_data_t;

/**
 ******************************************************************************
 ** \brief FRT instance data type
 ******************************************************************************/
typedef struct stc_mft_frt_instance_data
{
    volatile stc_mftn_frt_t*  pstcInstance;  ///< pointer to registers of an instance
    stc_mft_frt_intern_data_t stcInternData; ///< module internal data of instance
} stc_mft_frt_instance_data_t;  

/******************************************************************************/
/* Global variable declarations ('extern', definition in C source)            */
/******************************************************************************/  
/// Look-up table for all enabled FRT of MFT  instances and their internal data  
extern stc_mft_frt_instance_data_t m_astcMftFrtInstanceDataLut[];

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif
/******************************************************************************/
/* Global function prototypes ('extern', definition in C source)              */
/******************************************************************************/
/* Init */
en_result_t Mft_Frt_Init(volatile stc_mftn_frt_t*pstcMft, 
                         uint8_t u8Ch, stc_mft_frt_config_t*  pstcFrtConfig);

/* Mask zero/peak write/read */
en_result_t Mft_Frt_SetMaskZeroTimes(volatile stc_mftn_frt_t*pstcMft, uint8_t u8Ch, 
                                     uint8_t u8Times);
uint8_t Mft_Frt_GetMaskZeroTimes(volatile stc_mftn_frt_t*pstcMft, uint8_t u8Ch);
en_result_t Mft_Frt_SetMaskPeakTimes(volatile stc_mftn_frt_t*pstcMft, uint8_t u8Ch, uint8_t u8Times);
uint8_t Mft_Frt_GetMaskPeakTimes(volatile stc_mftn_frt_t*pstcMft, uint8_t u8Ch);

/* Func/Int enable/disable */
en_result_t Mft_Frt_Start(volatile stc_mftn_frt_t*pstcMft, uint8_t u8Ch);
en_result_t Mft_Frt_Stop(volatile stc_mftn_frt_t*pstcMft, uint8_t u8Ch);
#if (PDL_INTERRUPT_ENABLE_MFT0_FRT == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_MFT1_FRT == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_MFT2_FRT == PDL_ON)
en_result_t Mft_Frt_EnableInt(volatile stc_mftn_frt_t*pstcMft, 
                              uint8_t u8Ch, 
                              stc_frt_int_sel_t* pstcIntSel,
                              stc_frt_int_cb_t* pstcFrtIntCallback);
en_result_t Mft_Frt_DisableInt(volatile stc_mftn_frt_t*pstcMft,uint8_t u8Ch,
                               stc_frt_int_sel_t* pstcIntSel);
#endif

/* Status get/clear */
en_int_flag_t Mft_Frt_GetIntFlag(volatile stc_mftn_frt_t*pstcMft, 
                                 uint8_t u8Ch, 
                                 en_mft_frt_int_t enIntType);
en_result_t Mft_Frt_ClrIntFlag(volatile stc_mftn_frt_t*pstcMft, 
                               uint8_t u8Ch, 
                               en_mft_frt_int_t enIntType);

/* Count write/read */
en_result_t Mft_Frt_SetCountCycle(volatile stc_mftn_frt_t*pstcMft, uint8_t u8Ch, uint16_t u16Cycle);
en_result_t Mft_Frt_SetCountVal(volatile stc_mftn_frt_t*pstcMft, uint8_t u8Ch,uint16_t u16Count);
uint16_t Mft_Frt_GetCurCount(volatile stc_mftn_frt_t*pstcMft, uint8_t u8Ch);

/* IRQ handler */
void Mft_Frt_IrqHandler(volatile stc_mftn_frt_t*pstcMft, stc_mft_frt_intern_data_t* pstcMftFrtInternData) ;

#ifdef __cplusplus
}
#endif

//@}

#endif // #if (defined(PDL_PERIPHERAL_ADC_ACTIVE))

#endif /* __MFT_H__ */
  
  
  
  
  
  
  
  
  
