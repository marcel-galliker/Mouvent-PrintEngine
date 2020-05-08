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
/** \file lvd.h
 **
 ** Headerfile for LVD functions
 **  
 ** History:
 **   - 2012-04-03  1.0  NT   First version.
 **
 ******************************************************************************/

#ifndef __LVD_H__
#define __LVD_H__

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "pdl.h"

#if (defined(PDL_PERIPHERAL_LVD_ACTIVE))

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/**
 ******************************************************************************
 ** \defgroup LvdGroup Low Voltage Detection (LVD)
 **
 ** Provided functions of LVD module:
 ** - Lvd_Init()
 ** - Lvd_DeInit()
 ** - Lvd_GetIntStatus()
 ** - Lvd_ClearIntStatus()
 ** - Lvd_GetIntOperationStatus()
 **
 ** Lvd_Init() enables the LVD function with the given voltage threshold.
 ** If a callback function is not specified when Lvd_Init() is called, NVIC is
 ** is not enabled. So user has to check interrupt by Lvd_GetIntStatus()
 ** and clear interrupt by Lvd_ClearIntStatus().
 ** Lvd_DeInit() disables the LVD function. 
 ** Lvd_GetIntOperationStatus() is used for checking the operation status of
 ** a LVD interrupt.
 **
 ******************************************************************************/
//@{

/******************************************************************************/
/* Global pre-processor symbols/macros ('#define')                            */
/******************************************************************************/
 
/******************************************************************************
 * Global type definitions
 ******************************************************************************/

/**
 ******************************************************************************
 ** \brief Interruption voltage settings for Low Voltage Detection
 **
 ** \note The enumerated values do not correspond with the bit patterns of
 **       SVHI for upward compatibility reasons.
 ******************************************************************************/  
typedef enum en_lvd_irq_voltage
{
    LvdIrqVoltage220 = 0,   ///< Interrupt when voltage is vicinity of 2.20 volts
    LvdIrqVoltage240 = 1,   ///< Interrupt when voltage is vicinity of 2.40 volts
    LvdIrqVoltage245 = 2,   ///< Interrupt when voltage is vicinity of 2.45 volts
    LvdIrqVoltage260 = 3,   ///< Interrupt when voltage is vicinity of 2.60 volts
    LvdIrqVoltage270 = 4,   ///< Interrupt when voltage is vicinity of 2.70 volts
    LvdIrqVoltage280 = 5,   ///< Interrupt when voltage is vicinity of 2.80 volts
    LvdIrqVoltage290 = 6,   ///< Interrupt when voltage is vicinity of 2.90 volts
    LvdIrqVoltage300 = 7,   ///< Interrupt when voltage is vicinity of 3.00 volts
    LvdIrqVoltage320 = 8,   ///< Interrupt when voltage is vicinity of 3.20 volts
    LvdIrqVoltage350 = 9,   ///< Interrupt when voltage is vicinity of 3.50 volts
    LvdIrqVoltage360 = 10,  ///< Interrupt when voltage is vicinity of 3.60 volts
    LvdIrqVoltage370 = 11,  ///< Interrupt when voltage is vicinity of 3.70 volts
    LvdIrqVoltage380 = 12,  ///< Interrupt when voltage is vicinity of 3.80 volts
    LvdIrqVoltage390 = 13,  ///< Interrupt when voltage is vicinity of 3.90 volts
    LvdIrqVoltage400 = 14,  ///< Interrupt when voltage is vicinity of 4.00 volts
    LvdIrqVoltage410 = 15,  ///< Interrupt when voltage is vicinity of 4.10 volts
    LvdIrqVoltage420 = 16,  ///< Interrupt when voltage is vicinity of 4.20 volts
    LvdIrqVoltage430 = 17,  ///< Interrupt when voltage is vicinity of 4.30 volts
    LvdIrqVoltage452 = 18   ///< Interrupt when voltage is vicinity of 4.52 volts
} en_lvd_irq_voltage_t;

/**
 ******************************************************************************
 ** \brief Low Voltage Detection internal data
 ******************************************************************************/ 
typedef struct stc_lvd_intern_data
{
    func_ptr_t  pfnCallback;    ///< LVD interrupt callback function
} stc_lvd_intern_data_t ;

/**
 ******************************************************************************
 ** \brief Clock Supervisor configuration
 ** 
 ** The Clock Supervisor configuration settings
 ******************************************************************************/
typedef struct stc_lvd_config
{
    en_lvd_irq_voltage_t enIrqVoltage;  ///< LVD interrupt voltage, see #en_lvd_irq_voltage_t for details.
    func_ptr_t           pfnCallback;   ///< LVD interrupt callback function
} stc_lvd_config_t;

/******************************************************************************/
/* Global variable definitions ('extern')                                     */
/******************************************************************************/

/******************************************************************************/
/* Global function prototypes (definition in C source)                        */
/******************************************************************************/
extern void LvdIrqHandler(void);

extern en_result_t Lvd_Init(stc_lvd_config_t* pstcConfig);

extern en_result_t Lvd_DeInit(void);

extern boolean_t Lvd_GetIntStatus(void);

extern void Lvd_ClearIntStatus(void);

extern boolean_t Lvd_GetIntOperationStatus(void);

//@} // LvdGroup

#ifdef __cplusplus
}
#endif

#endif /* #if (defined(PDL_PERIPHERAL_LVD_ACTIVE)) */

#endif /* __LVD_H__ */
/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
