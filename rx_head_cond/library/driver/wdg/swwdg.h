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
/** \file swwdg.h
 **
 ** Headerfile for Software Watchdog functions
 **  
 ** History:
 **   - 2013-04-02  1.0  NT   First version.
 **
 ******************************************************************************/

#ifndef __SWWDG_H__
#define __SWWDG_H__

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
 ** \defgroup SwwdgGroup Software Watchdog (SWWDG)
 **
 ** Provided functions of SWWDG module:
 ** 
 ** - Swwdg_Init()
 ** - Swwdg_DeInit()
 ** - Swwdg_Start()
 ** - Swwdg_Stop()
 ** - Swwdg_WriteWdgLoad()
 ** - Swwdg_ReadWdgValue()
 ** - Swwdg_Feed()
 ** - Swwdg_QuickFeed()
 ** - Swwdg_EnableDbgBrkWdgCtl()
 ** - Swwdg_DisableDbgBrkWdgCtl()
 **
 ** This module has dedicated interrupt callback functions, in which the
 ** user has to feed the Software Watchdog.
 **
 ** For the Software Watchdog Swwdg_Init() is used for setting the interval
 ** time. Swwdg_Feed() resets the Watchdog timer by a function call.
 ** Swwdg_QuickFeed() does the same, but the code is inline expanded for
 ** e.g. time-critical polling loops.
 **
 ** Additionaly Software Watchdog has the window watchdog mode.
 ** The Window Watchdog Mode detects whether counter reload by software is
 ** implemented at the right timing.
 ** This mode can be set on Swwdg_Init() by setting the parameter of
 ** stc_swwdg_config_t#bWinWdgEnable in #stc_swwdg_config_t to TRUE.
 ** The timing is set by parameter of stc_swwdg_config_t#bu8TimingWindow.
 ** If the parameter of stc_swwdg_config_t#bWinWdgResetEnable is set to TRUE,
 ** when the counter is reloaded at timing outside the timing window set previously,
 ** or when the counter is underflow, the Software Watchdog timer issues an reset.
 **
 ** Swwdg_Init() sets the Software Watchdog interval and mode.
 ** Mode is set by configuration bWinWdgEnable#stc_swwdg_config_t.
 ** This function has to call before using Software Watchdog.
 ** Swwdg_DeInit() disables the Watchdog.
 ** Swwdg_Start() starts interruption and count of the Software Watchdog.
 ** Swwdg_Stop() stops interruption and count of the Software Watchdog.
 ** Swwdg_WriteWdgLoad() writes load value for the Software Watchdog.
 ** Swwdg_ReadWdgValue() reads counter value of the Software Watchdog.
 ** Swwdg_Feed() resets the Watchdog timer by a function call.
 ** Swwdg_QuickFeed() does the same, but the code is inline expanded for
 ** e.g. time-critical polling loops.
 **
 ******************************************************************************/
//@{

/*****************************************************************************/
/* Global pre-processor symbols/macros ('#define')                           */
/*****************************************************************************/
/* for WDOGLOCK */
#define SWWDG_REG_UNLOCK    (0x1ACCE551u)

/******************************************************************************
 * Global type definitions
 ******************************************************************************/

/**
 ******************************************************************************
 ** \brief Software watchdog timing window settings
 ******************************************************************************/
typedef enum en_swwdg_timing_window
{
    en_swwdg_timing_window_100 = 0x00,    ///< Reload can be executed at less than or equal to WDOGLOAD.
    en_swwdg_timing_window_75  = 0x01,    ///< Reload can be executed at less than or equal to 75% of WDOGLOAD.
    en_swwdg_timing_window_50  = 0x02,    ///< Reload can be executed at less than or equal to 50% of WDOGLOAD
    en_swwdg_timing_window_25  = 0x03     ///< Reload can be executed at less than or equal to 25% of WDOGLOAD
} en_swwdg_timing_window_t;

/**
 *****************************************************************************
 ** \brief Software and Hardware Watchdog configuration
 *****************************************************************************/
typedef struct stc_swwdg_config
{
  uint32_t   u32LoadValue;          ///< Timer interval
  boolean_t  bResetEnable;          ///< TRUE:  Enables SW watchdog reset
  boolean_t  bWinWdgEnable;         ///< TRUE:  Enables Window watchdog mode
  boolean_t  bWinWdgResetEnable;    ///< TRUE:  Enables reset when reload without timing window was occured.
  uint8_t    u8TimingWindow;        ///< Timing window setting, see description of #en_swwdg_timing_window_t
} stc_swwdg_config_t;

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
extern void SwwdgIrqHandler(void);

/* Setup(Initialize)/Disable(Un-initialize) */
extern en_result_t Swwdg_Init(stc_swwdg_config_t* pstcConfig);
extern void Swwdg_DeInit(void);

/* Start/Stop */
extern en_result_t Swwdg_Start(func_ptr_t pfnSwwdgCb);
extern void Swwdg_Stop(void);

/* Write/Read for counter */
extern void Swwdg_WriteWdgLoad(uint32_t u32LoadValue);
extern uint32_t Swwdg_ReadWdgValue(void);

/* Feed watchdog */
extern void Swwdg_Feed(void) ;
static void Swwdg_QuickFeed(void);

/* Setting Debug Break Watchdog Timer Control */
extern void Swwdg_EnableDbgBrkWdgCtl(void);
extern void Swwdg_DisableDbgBrkWdgCtl(void);

/******************************************************************************/
/* Static inline functions                                                    */
/******************************************************************************/

/**
 ******************************************************************************
 ** \brief Quickly feed Software Watchdog (inline function)
 **
 ** This function feeds the Software Watchdog with the unlock, feed, and lock
 ** sequence as an inline function for quick execution in polling loops.
 ******************************************************************************/
static __INLINE void Swwdg_QuickFeed(void)
{
    FM4_SWWDT->WDOGLOCK   = SWWDG_REG_UNLOCK;   /* Release Lock */
    FM4_SWWDT->WDOGINTCLR = 1;                  /* Clear possible interrupt and reload value */
    FM4_SWWDT->WDOGLOCK   = 0;                  /* Lock again it! */
} /* Swwdg_QuickFeed */

//@} // SwwdgGroup

#ifdef __cplusplus
}
#endif

#endif /* #if (defined(PDL_PERIPHERAL_WDG_ACTIVE)) */

#endif /* __SWWDG_H__ */
/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
