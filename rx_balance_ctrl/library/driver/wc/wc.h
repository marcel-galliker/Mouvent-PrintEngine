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
/** \file wc.h
 **
 ** Headerfile for WC functions
 **
 ** History:
 **   - 2013-05-17  0.1  Edison Zhang  First version.
 **
 ******************************************************************************/

#ifndef __WC_H__
#define __WC_H__

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "pdl.h"

#if (defined(PDL_PERIPHERAL_WC_ACTIVE))

/**
 ******************************************************************************
 ** \defgroup WcGroup Watch Counter (WC)
 **
 ** Provided functions of WC module:
 ** 
 ** - Wc_Pres_SelClk()
 ** - Wc_Pres_EnableDiv() 
 ** - Wc_Pres_DisableDiv()
 ** - Wc_Init()
 ** - Wc_EnableCount()
 ** - Wc_DisableCount()
 ** - Wc_EnableInt()
 ** - Wc_DisableInt()
 ** - Wc_WriteReloadVal()
 ** - Wc_ReadCurCnt()
 ** - Wc_ClearIntFlag()
 ** - Wc_GetIntFlag()
 ** - Wc_GetOperationFlag()
 ** - Wc_IrqHandler()
 **
 ** \brief How to use WC module
 **
 ** Before using WC, WC prescaler must be configured first. Use Wc_Pres_SelClk()
 ** to select input clock of prescaler. Following clocks can be selected:
 ** - Sub clock
 ** - Main clock
 ** - High-speed CR
 ** - CLKLC (divided by low speed CR)
 ** 
 ** Wc_Pres_EnableDiv() is used to enable watch counter prescaler. 
 ** Wc_Pres_DisableDiv() is used to disable watch counter prescaler. 
 **
 ** Wc_Init() must be used for configuration of watch counter with a structure 
 ** of the type #stc_wc_config_t.
 **
 ** A WC interrupt can be enabled by the function Wc_EnableInt().
 ** This function can set callback function for each channel too.
 **
 ** With Wc_WriteReloadVal() the WC reloader value is set to the value
 ** given in the parameter Wc_WriteReloadVal#u8Val.
 **
 ** After above setting, calling Wc_EnableCount() will start WC.
 **
 ** With Wc_ReadCurCnt() the current WC count can be read when WC is 
 ** counting. with Wc_GetOperationFlag() the current WC operaiton status
 ** can be read.
 **
 ** With interrupt mode, when the interrupt occurs, the interrupt flag will
 ** be cleared and run into user interrupt callback function.
 **
 ** With polling mode, user can use Wc_GetIntFlag() to check if the 
 ** interrupt occurs, and clear the interrupt flag by Wc_ClrIntFlag().
 ** 
 ** When stopping the WC, use Wc_DisableCount() to disable WC and 
 ** Wc_DisableInt() to disable WC interrupt.
 **
 ******************************************************************************/
//@{
/*****************************************************************************/
/* Global pre-processor symbols/macros ('#define')                           */
/*****************************************************************************/
#define stc_wcn_t    FM4_WC_TypeDef  
#define WC0          (*((FM4_WC_TypeDef *) FM4_WC_BASE)) 

/**
 ******************************************************************************
 ** \brief Enumeration to define an index for each WC instance
 ******************************************************************************/   
typedef enum en_wc_instance_index
{
    WcInstanceIndexWc0 = 0,  ///< Instance index of WC0

} en_wc_instance_index_t; 

/**
 ******************************************************************************
 ** \brief Enumeration to define source clock of watch counter precaler
 ******************************************************************************/ 
typedef enum en_input_clk
{
    WcPresInClkSubOsc = 0, ///< Watch counter prescaler source clock: sub oscillator
    WcPresInClkMainOsc,    ///< Watch counter prescaler source clock: main oscillator
    WcPresInClkHighCr,     ///< Watch counter prescaler source clock: high-speed CR
    WcPresInClkLowCr,      ///< Watch counter prescaler source clock: low-speed CR 

}en_input_clk_t;

/**
 ******************************************************************************
 ** \brief Enumeration to define output clock of watch counter precaler
 ******************************************************************************/ 
typedef enum en_output_clk
{                           ///                                         WCCk3           WCCk2           WCCk1           WCCk0                         
    WcPresOutClkArray0 = 0, ///< Watch counter prescaler output array0: 2^15/src clock, 2^14/src clock, 2^13/src clock, 2^12/src clock
    WcPresOutClkArray1,     ///< Watch counter prescaler output array0: 2^25/src clock, 2^24/src clock, 2^23/src clock, 2^22/src clock
    WcPresOutClkArray2,     ///< Watch counter prescaler output array0: 2^4/src clock, 2^3/src clock, 2^2/src clock, 2/src clock
    WcPresOutClkArray3,     ///< Watch counter prescaler output array0: 2^8/src clock, 2^7/src clock, 2^6/src clock, 2^5/src clock
    WcPresOutClkArray4,     ///< Watch counter prescaler output array0: 2^12/src clock, 2^11/src clock, 2^10/src clock, 2^9/src clock
    WcPresOutClkArray5,     ///< Watch counter prescaler output array0: 2^19/src clock, 2^18/src clock, 2^17/src clock, 2^16/src clock
    WcPresOutClkArray6,     ///< Watch counter prescaler output array0: 2^23/src clock, 2^22/src clock, 2^21/src clock, 2^20/src clock
    
}en_output_clk_t;

/**
 ******************************************************************************
 ** \brief Structure to configure watch counter prescaler
 ******************************************************************************/
typedef struct stc_wc_pres_clk
{
    en_input_clk_t      enInputClk;   ///< Watch counter prescaler input clock setting
    en_output_clk_t     enOutputClk;  ///< Watch counter prescaler output clock setting
   
}stc_wc_pres_clk_t;


/**
 ******************************************************************************
 ** \brief Enumeration to set the clock of watch counter
 ******************************************************************************/
typedef enum en_wc_cnt_clk
{
    WcCntClkWcck0 = 0,   ///< Watch counter source clock: WCCK0
    WcCntClkWcck1,       ///< Watch counter source clock: WCCK1
    WcCntClkWcck2,       ///< Watch counter source clock: WCCK2
    WcCntClkWcck3,       ///< Watch counter source clock: WCCK3
    
}en_wc_cnt_clk_t;

/**
 ******************************************************************************
 ** \brief Structure to configure the watch counter
 ******************************************************************************/
typedef struct stc_wc_config
{
    en_wc_cnt_clk_t  enCntClk;   ///< Watch counter source clock setting
  
}stc_wc_config_t;

/**
 ******************************************************************************
 ** \brief Structure of watch counter internal data
 ******************************************************************************/
typedef struct stc_wc_intern_data
{
    func_ptr_t  pfnIntCallback; ///< Pointer to watch counter interrupt callback function
} stc_wc_intern_data_t ; 

/**
 ******************************************************************************
 ** \brief Structure of watch counter instance data
 ******************************************************************************/
typedef struct stc_wc_instance_data
{
    volatile stc_wcn_t* pstcInstance;   ///< pointer to registers of an instance
    stc_wc_intern_data_t stcInternData; ///< module internal data of instance
} stc_wc_instance_data_t;


/******************************************************************************/
/* Global variable definitions ('extern')                                     */
/******************************************************************************/

/// Look-up table for all enabled WC instances and their internal data
extern stc_wc_instance_data_t m_astcWcInstanceDataLut[];

/******************************************************************************/
/* Global function prototypes (definition in C source)                        */
/******************************************************************************/
/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/* 1. WC prescaler setting */
en_result_t Wc_Pres_SelClk(volatile stc_wcn_t* pstcWc, stc_wc_pres_clk_t* pstcWcPresClk);
en_result_t Wc_Pres_EnableDiv(volatile stc_wcn_t* pstcWc);
en_result_t Wc_Pres_DisableDiv(volatile stc_wcn_t* pstcWc);
en_stat_flag_t Wc_GetDivStat(volatile stc_wcn_t* pstcWc);
/* 2. WC setting */
/* Init */
en_result_t Wc_Init(volatile stc_wcn_t* pstcWc, stc_wc_config_t* pstcWcConfig);
/* Func/Int enable/disable */
en_result_t Wc_EnableCount(volatile stc_wcn_t* pstcWc);
en_result_t Wc_DisableCount(volatile stc_wcn_t* pstcWc);
#if (PDL_INTERRUPT_ENABLE_WC0 == PDL_ON)
en_result_t Wc_EnableInt(volatile stc_wcn_t* pstcWc, func_ptr_t  pfnIntCallback);
en_result_t Wc_DisableInt(volatile stc_wcn_t* pstcWc);
#endif
/* Count write/read */
en_result_t Wc_WriteReloadVal(volatile stc_wcn_t* pstcWc, uint8_t u8Val);
uint8_t Wc_ReadCurCnt(volatile stc_wcn_t* pstcWc);
/* Status clear */
en_result_t Wc_ClearIntFlag(volatile stc_wcn_t* pstcWc);
/* Status read */
en_int_flag_t Wc_GetIntFlag(volatile stc_wcn_t* pstcWc);
en_stat_flag_t Wc_GetOperationFlag(volatile stc_wcn_t* pstcWc);
/* 3. IRQ handler */
void Wc_IrqHandler( volatile stc_wcn_t* pstcWc,
                    stc_wc_intern_data_t* pstcWcInternData) ;

#ifdef __cplusplus
}
#endif

//@} // WcGroup

#endif // #if (defined(PDL_PERIPHERAL_WC_ACTIVE))

#endif /* __WC_H__ */
/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
