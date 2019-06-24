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
/** \file mft_icu.h
 **
 ** Headerfile for MFT's Input Capture Unit functions
 **  
 ** History:
 **   - 2013-03-21  0.1  Edison  First version.
 **   - 2013-04-25  0.2  Chamber Modify
 **   - 2013-07-16  0.3  Edison  Add comments about how to use ICU with APIs 
 **
 *****************************************************************************/

#ifndef __MFT_ICU_H__
#define __MFT_ICU_H__

#include "pdl.h"   
   
#if (defined(PDL_PERIPHERAL_MFT_ICU_ACTIVE))

/**
 ******************************************************************************
 ** \defgroup IcuGroup Input Capture Unit (ICU)
 **
 ** Provided functions of ICU module:
 ** 
 ** - Mft_Icu_SelFrt()
 ** - Mft_Icu_ConfigDetectMode() 
 ** - Mft_Icu_EnableInt()
 ** - Mft_Icu_DisableInt()
 ** - Mft_Icu_GetIntFlag()
 ** - Mft_Icu_ClrIntFlag()
 ** - Mft_Icu_GetLastEdge()
 ** - Mft_Icu_GetCaptureData()
 ** - Mft_Icu_IrqHandler()
 **
 ** \brief How to use ICU module
 ** 
 ** Before using OCU, a FRT used to connect with applying OCU must be initialed.
 ** For how to configure FRT, see the description in the mft_frt.h. With 
 ** Mft_Icu_SelFrt(), a FRT can be connected with OCU.
 **
 ** A ICU interrupt can be enabled by the function Mft_Icu_EnableInt().
 ** This function can set callback function for each channel too.
 **
 ** After above setting, calling Mft_Icu_ConfigDetectMode() will select a 
 ** detection mode and start ICU operation at the same time. Following detection
 ** mode cna be select:
 ** - Disable
 ** - Rising edge detection mode
 ** - Falling edge detection mode
 ** - Both edge detection mode
 **
 ** With interrupt mode, the interrupt occurs when valid edge is detected, the 
 ** interrupt flag will be cleared and run into user interrupt callback 
 ** function. In the callback fucntion, the capture value can be read with
 ** Mft_Icu_GetCaptureData().
 **
 ** With polling mode, user can use Mft_Icu_GetIntFlag() to check if the 
 ** interrupt occurs, and clear the interrupt flag by Mft_Icu_ClrIntFlag().
 ** when interrupt is detected, the capture value can be read with
 ** Mft_Icu_GetCaptureData().
 **
 ** When the valid edge is detected, Mft_Icu_GetLastEdge() can get the edge
 ** information of last valid edge.
 ** 
 ** When stopping the ICU, use Mft_Icu_ConfigDetectMode() to disable ICU and 
 ** Mft_Icu_DisableInt() to disable ICU interrupt.
 **
 ******************************************************************************/  
//@{
/******************************************************************************
 * Global type definitions
 ******************************************************************************/
#define stc_mftn_icu_t   FM4_MFT_ICU_TypeDef
#define MFT0_ICU  (*((volatile stc_mftn_icu_t *) FM4_MFT0_ICU_BASE))
#define MFT1_ICU  (*((volatile stc_mftn_icu_t *) FM4_MFT1_ICU_BASE))
#define MFT2_ICU  (*((volatile stc_mftn_icu_t *) FM4_MFT2_ICU_BASE))   

#define  MFT_ICU_CH0        0u
#define  MFT_ICU_CH1        1u
#define  MFT_ICU_CH2        2u
#define  MFT_ICU_CH3        3u

#define  MFT_ICU_CHx_MAX    3u

#define MFT_ICU_INSTANCE_COUNT \
        (uint8_t)(PDL_PERIPHERAL_ENABLE_MFT0_ICU == PDL_ON) + \
        (uint8_t)(PDL_PERIPHERAL_ENABLE_MFT1_ICU == PDL_ON) + \
        (uint8_t)(PDL_PERIPHERAL_ENABLE_MFT2_ICU == PDL_ON)

/**
 ******************************************************************************
 ** \brief Enumeration to define an index for each ICU instance
 ******************************************************************************/   
typedef enum en_icu_instance_index
{
    IcuInstanceIndexIcu0 = 0u,              ///< Instance index of ICU0
    IcuInstanceIndexIcu1 = 1u,              ///< Instance index of ICU1
    IcuInstanceIndexIcu2 = 2u,              ///< Instance index of ICU2

} en_icu_instance_index_t;  
   
/**
 ******************************************************************************
 ** \brief Frt channel
 ** 
 ** To select FRT channel to connect to ICU
 ******************************************************************************/
typedef enum en_mft_icu_frt
{
    Frt0ToIcu    = 0u,                      ///< connect Frt channel 0 to Icu
    Frt1ToIcu    = 1u,                      ///< connect Frt channel 1 to Icu
    Frt2ToIcu    = 2u,                      ///< connect Frt channel 2 to Icu
    IcuFrtToExt0 = 3u,                      ///< connect extern Frt channel 0 to Icu
    IcuFrtToExt1 = 4u,                      ///< connect extern Frt channel 1 to Icu
  
}en_mft_icu_frt_t;
/**
 ******************************************************************************
 ** \brief Icu mode
 ** 
 ** To select Icu edge detection mode
 ******************************************************************************/
typedef enum en_mft_icu_mode
{
    IcuDisable       = 0u,                  ///< disable Icu edge detection
    IcuRisingDetect  = 1u,                  ///< detect Icu rising edge
    IcuFallingDetect = 2u,                  ///< detect Icu falling edge
    IcuBothDetect    = 3u,                  ///< detect Icu rising/falling edge

}en_mft_icu_mode_t;
/**
 ******************************************************************************
 ** \brief Icu edge
 ** 
 ** To select Icu edge
 ******************************************************************************/
typedef enum en_icu_edge
{
    IcuFallingEdge = 0u,                    ///< select Icu falling edge
    IcuRisingEdge  = 1u,                    ///< select Icu rising edge
    
}en_mft_icu_edge_t;
/**
 ******************************************************************************
 ** \brief Structure of Mft icu configuration
 ******************************************************************************/
typedef struct stc_mft_icu_config
{
    en_mft_icu_frt_t      enFrt;            ///< Frt channel
    en_mft_icu_mode_t     enMode;           ///< Icu detect mode
    en_mft_icu_edge_t     enEdge;           ///< Icu edge

}stc_mft_icu_config_t;
/**
 ******************************************************************************
 ** \brief Icu instance internal data,
 **        storing internal information for each enabled Icu instance
 ******************************************************************************/
typedef struct stc_mft_icu_intern_data
{
    func_ptr_t  pfnIcu0Callback;            ///< Callback function pointer of ICU0 interrupt
    func_ptr_t  pfnIcu1Callback;            ///< Callback function pointer of ICU1 interrupt
    func_ptr_t  pfnIcu2Callback;            ///< Callback function pointer of ICU2 interrupt
    func_ptr_t  pfnIcu3Callback;            ///< Callback function pointer of ICU3 interrupt
    
}stc_mft_icu_intern_data_t;
/**
 ******************************************************************************
 ** \brief Mft_icu instance data type
 ******************************************************************************/
typedef struct stc_mft_icu_instance_data
{
    volatile stc_mftn_icu_t*  pstcInstance; ///< pointer to registers of an instance
    stc_mft_icu_intern_data_t stcInternData;///< module internal data of instance
    
}stc_mft_icu_instance_data_t;
/******************************************************************************/
/* Global variable definitions ('extern')                                     */
/******************************************************************************/

/** Look-up table for all enabled MFT_ICU instances and their internal data */
extern stc_mft_icu_instance_data_t m_astcMftIcuInstanceDataLut[MFT_ICU_INSTANCE_COUNT];

/** C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif
/** Clock */
en_result_t Mft_Icu_SelFrt(volatile stc_mftn_icu_t*pstcMft, uint8_t u8Ch, en_mft_icu_frt_t enFrt);
/** mode set */
en_result_t Mft_Icu_ConfigDetectMode(volatile stc_mftn_icu_t*pstcMft, uint8_t u8Ch, en_mft_icu_mode_t enMode);
/** Interrupt */
#if (PDL_INTERRUPT_ENABLE_MFT0_ICU == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_MFT1_ICU == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_MFT2_ICU == PDL_ON)
en_result_t Mft_Icu_EnableInt(volatile stc_mftn_icu_t*pstcMft, uint8_t u8Ch, func_ptr_t pfnCallback);
en_result_t Mft_Icu_DisableInt(volatile stc_mftn_icu_t*pstcMft, uint8_t u8Ch);
#endif
en_int_flag_t Mft_Icu_GetIntFlag(volatile stc_mftn_icu_t*pstcMft, uint8_t u8Ch);
en_result_t Mft_Icu_ClrIntFlag(volatile stc_mftn_icu_t*pstcMft, uint8_t u8Ch);
/** Status read/write */
en_mft_icu_edge_t Mft_Icu_GetLastEdge(volatile stc_mftn_icu_t*pstcMft, uint8_t u8Ch);
/** Count read */
uint16_t Mft_Icu_GetCaptureData(volatile stc_mftn_icu_t*pstcMft, uint8_t u8Ch);
/** IRQ handler */
void Mft_Icu_IrqHandler(volatile stc_mftn_icu_t*pstcMft, stc_mft_icu_intern_data_t* pstcMftIcuInternData) ;

#ifdef __cplusplus
}
#endif

//@}

#endif

#endif
