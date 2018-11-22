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
/** \file hwwdg.h
 **
 ** Headerfile for Hardware Watchdog functions
 **  
 ** History:
 **   - 2013-04-02  1.0  NT   First version.
 **
 ******************************************************************************/

#ifndef __HWWDG_H__
#define __HWWDG_H__

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "pdl.h"

#if (defined(PDL_PERIPHERAL_WDG_ACTIVE))

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/**
 ******************************************************************************
 ** \defgroup HwwdgGroup Hardware Watchdog (HWWDG)
 **
 ** Provided functions of HWWDG module:
 ** 
 ** - Hwwdg_Init()
 ** - Hwwdg_DeInit()
 ** - Hwwdg_Start()
 ** - Hwwdg_Stop()
 ** - Hwwdg_WriteWdgLoad()
 ** - Hwwdg_ReadWdgValue()
 ** - Hwwdg_Feed()
 ** - Hwwdg_QuickFeed()
 ** - Hwwdg_EnableDbgBrkWdgCtl()
 ** - Hwwdg_DisableDbgBrkWdgCtl()
 **
 ** This module has dedicated interrupt callback functions, in which the user 
 ** has to feed the Hardware Watchdog.
 **
 ** For the Hardware Watchdog Hwwdg_Init() is used for setting the interval
 ** time. Hwwdg_Feed() resets the Watchdog timer by a function call.
 ** Hwwdg_QuickFeed() does the same, but the code is inline expanded for
 ** e.g. time-critical polling loops.
 **
 ** Hwwdg_Init() sets the Hardware Watchdog interval.
 ** Hwwdg_DeInit() stops and disables the Hardware Watchdog, if two magic words
 ** as parameters are used.
 ** Hwwdg_Start() starts interruption and count of the Hardware Watchdog.
 ** Hwwdg_Stop() stops interruption and count of the Hardware Watchdog.
 ** Hwwdg_WriteWdgLoad() writes load value for the Hardware Watchdog.
 ** Hwwdg_ReadWdgValue() reads counter value of the Hardware Watchdog.
 ** Hwwdg_Feed() and Hwwdg_QuickFeed() do the same as their correspondig
 ** functions for the Software Watchdog, but here are two parameter needed,
 ** the 2nd one the inverted value of the 1st.
 **
 ** \note 
 **    - The Hardware Watchdog shares its interrupt vector with the NMI.
 **    - The Hardware Watchdog is also switched off in System_Init() in
 **      system_mb9[ab]xyz.c. Set the definition for HWWD_DISABLE to 0 in 
 **      system_mb9[ab]xyz.h for letting the Hardware Watchdog running during
 **      start-up phase.
 **
 ******************************************************************************/
//@{

/*****************************************************************************/
/* Global pre-processor symbols/macros ('#define')                           */
/*****************************************************************************/
/* for WDG_LCK */
#define HWWDG_REG_UNLOCK_1  (0x1ACCE551u)
#define HWWDG_REG_UNLOCK_2  (0xE5331AAEu)

/******************************************************************************
 * Global type definitions
 ******************************************************************************/

/**
 *****************************************************************************
 ** \brief Hardware Watchdog configuration
 *****************************************************************************/
typedef struct stc_hwwdg_config
{
    uint32_t   u32LoadValue;    ///< Timer interval
    boolean_t  bResetEnable;    ///< TRUE:  Enables Hardware watchdog reset
} stc_hwwdg_config_t;

/******************************************************************************/
/* Local type definitions ('typedef')                                         */
/******************************************************************************/

/******************************************************************************/
/* Global variable definitions ('extern')                                     */
/******************************************************************************/

/******************************************************************************/
/* Global function prototypes (definition in C source)                        */
/******************************************************************************/
/* Interrupt */
extern void HwwdgIrqHandler(void);

/* Setup(Initialize)/Disable(Un-initialize) */
extern en_result_t Hwwdg_Init(stc_hwwdg_config_t* pstcConfig);
extern en_result_t Hwwdg_DeInit(uint32_t u32MagicWord1,
                                uint32_t u32MagicWord2
                               );

/* Start/Stop */
extern en_result_t Hwwdg_Start(func_ptr_t pfnHwwdgCb);
extern void Hwwdg_Stop(void);

/* Write/Read for counter */
extern void Hwwdg_WriteWdgLoad(uint32_t u32LoadValue);
extern uint32_t Hwwdg_ReadWdgValue(void);

/* Feed watchdog */
extern void Hwwdg_Feed(uint8_t u8ClearPattern1,
                       uint8_t u8ClearPattern2
                      );

extern void Hwwdg_EnableDbgBrkWdgCtl(void);
extern void Hwwdg_DisableDbgBrkWdgCtl(void);

static void Hwwdg_QuickFeed(uint8_t u8ClearPattern1, uint8_t u8ClearPattern2);

/******************************************************************************/
/* Static inline functions                                                    */
/******************************************************************************/

/**
 ******************************************************************************
 ** \brief Quickly feed Hardware Watchdog (inline function)
 **
 ** This function feeds the Hardware Watchdog with the unlock, feed, and lock
 ** sequence as an inline function for quick execution in polling loops.
 ** Take care of the arbitrary values, because there are not checked for
 ** plausibility!
 **
 ** \param [in] u8ClearPattern1   Pattern of arbitrary value
 ** \param [in] u8ClearPattern2   Inverted arbitrary value
 ******************************************************************************/
static __INLINE void Hwwdg_QuickFeed(uint8_t u8ClearPattern1, uint8_t u8ClearPattern2)
{
    FM4_HWWDT->WDG_LCK = HWWDG_REG_UNLOCK_1;/* Release Lock */
    FM4_HWWDT->WDG_ICL = u8ClearPattern1;   /* Clear possible interrupt and reload value, step 1 */
    FM4_HWWDT->WDG_ICL = u8ClearPattern2;   /* Clear possible interrupt and reload value, step 2 */
} /* Hwwdg_QuickFeed */

//@} // HwwdgGroup

#ifdef __cplusplus
}
#endif

#endif /* #if (defined(PDL_PERIPHERAL_WDG_ACTIVE)) */

#endif /* __HWWDG_H__ */
/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
