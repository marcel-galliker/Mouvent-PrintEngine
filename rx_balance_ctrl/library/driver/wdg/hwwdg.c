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
/** \file hwwdg.c
 **
 ** A detailed description is available at 
 ** @link HwwdgGroup Hardware Watchdog Module description @endlink
 **
 ** History:
 **   - 2013-04-02  1.0  NT   First version.
 **
 ******************************************************************************/

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "pdl.h"

#if (defined(PDL_PERIPHERAL_WDG_ACTIVE))

/**
 ******************************************************************************
 ** \addtogroup HwwdgGroup
 ******************************************************************************/
//@{

#if (PDL_PERIPHERAL_ENABLE_HWWDG == PDL_ON)
/******************************************************************************/
/* Local pre-processor symbols/macros ('#define')                             */
/******************************************************************************/
/* for WDG_CTL */
#define HWWDG_CTL_INTEN     (0x01u)
#define HWWDG_CTL_RESEN     (0x02u)

/* for checking magic word (Hwwdg_DeInit) */
#define HWWDG_MAGIC_WORD_CHK1       (0x38D1AE5Cu)
#define HWWDG_MAGIC_WORD_CHK2       (0x7624D1BCu)
#define HWWDG_MAGIC_WORD_CHK_RESULT (0xFFFFFFFFu)

/******************************************************************************/
/* Global variable definitions (declared in header file with 'extern')        */
/******************************************************************************/
static func_ptr_t pfnHwwdgCallback; ///< callback function pointer for HW-Wdg Irq

/******************************************************************************/
/* Local type definitions ('typedef')                                         */
/******************************************************************************/

/******************************************************************************/
/* Local function prototypes ('static')                                       */
/******************************************************************************/

/******************************************************************************/
/* Local variable definitions ('static')                                      */
/******************************************************************************/

/******************************************************************************/
/* Function implementation - global ('extern') and local ('static')           */
/******************************************************************************/

/**
 ******************************************************************************
 ** \brief Hardware Watchdog Interrupt Handler
 ******************************************************************************/
void HwwdgIrqHandler(void)
{
    /* Check hardware interrupt status */
    if (TRUE == FM4_HWWDT->WDG_RIS_f.RIS)
    {
        if (NULL != pfnHwwdgCallback)
        {
            pfnHwwdgCallback();
        }
    }
} /* HwwdgIrqHandler */

/**
 ******************************************************************************
 ** \brief Initialize Hardware Watchdog
 **
 ** \param [in] pstcConfig   Pointer to Hardware Watchdog configuration
 **
 ** \retval Ok                      Setup successful
 ** \retval ErrorInvalidParameter   pstcConfig == NULL
 **
 ** \note This function only set the Hardware Watchdog configuration.
 **       If Hwwdg_Start() is called, MCU start the Hardware Watchdog.
 ******************************************************************************/
en_result_t Hwwdg_Init(stc_hwwdg_config_t* pstcConfig)
{
    en_result_t enResult;
    uint8_t     u8WdogControl = 0;      /* Preset register */

    enResult = ErrorInvalidParameter;
    /* Check for NULL Pointer */
    if (NULL != pstcConfig)
    {
        /* Release Lock */
        FM4_HWWDT->WDG_LCK = HWWDG_REG_UNLOCK_1;
        FM4_HWWDT->WDG_LDR = pstcConfig->u32LoadValue; 

        if (TRUE == pstcConfig->bResetEnable)
        {
            /* RESEN bit */
            u8WdogControl |= HWWDG_CTL_RESEN;
        }

        /* HW Watchdog Control Register unlock sequence */
        FM4_HWWDT->WDG_LCK = HWWDG_REG_UNLOCK_1;
        FM4_HWWDT->WDG_LCK = HWWDG_REG_UNLOCK_2;

        /* Setup HW-Watchdog and start */
        FM4_HWWDT->WDG_CTL = u8WdogControl;

        /* Initialize callback to NULL */
        pfnHwwdgCallback = NULL;

        NVIC_ClearPendingIRQ(NMI_IRQn);
        NVIC_EnableIRQ(NMI_IRQn);
        NVIC_SetPriority(NMI_IRQn, PDL_IRQ_LEVEL_HWWDG);

        enResult = Ok;
    }

    return (enResult);
} /* Hwwdg_Init */

/**
 ******************************************************************************
 ** \brief Un-Initialize Hardware Watchdog
 **
 ** This function disables and un-initializes the Hardware Watchdog, when the
 ** first argument is 0xC72E51A3 and the second agrument 0x89DB2E43.
 ** The magic words are together 64-bit wide and have a balanced entropy.
 ** (32 zero and 32 one bits)
 **
 ** \param [in] u32MagicWord1   1st Magic word for disabling (0xC72E51A3)
 ** \param [in] u32MagicWord2   2nd Magic word for disabling (0x89DB2E43)
 **
 ** \retval Ok                      disable sucessful
 ** \retval ErrorInvalidParameter   not diabled => magic word is wrong!
 **
 ******************************************************************************/
en_result_t Hwwdg_DeInit(uint32_t u32MagicWord1,
                         uint32_t u32MagicWord2
                        )
{
    en_result_t enResult;

    enResult = ErrorInvalidParameter;

    /* Inverted magic word check is done to avoid "plain text magic word" in ROM. */
    if ((HWWDG_MAGIC_WORD_CHK_RESULT == (u32MagicWord1 ^ HWWDG_MAGIC_WORD_CHK1)) &&
        (HWWDG_MAGIC_WORD_CHK_RESULT == (u32MagicWord2 ^ HWWDG_MAGIC_WORD_CHK2))
       )
    {
        /* Disable NVIC */
        NVIC_ClearPendingIRQ(NMI_IRQn);
        NVIC_DisableIRQ(NMI_IRQn);
        NVIC_SetPriority(NMI_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);

        /* HW Watchdog Control Register unlock sequence */
        FM4_HWWDT->WDG_LCK = HWWDG_REG_UNLOCK_1;
        FM4_HWWDT->WDG_LCK = HWWDG_REG_UNLOCK_2;

        /* HW Watchdog stop */
        FM4_HWWDT->WDG_CTL = 0;

        /* Initialize callback to NULL */
        pfnHwwdgCallback = NULL;

        enResult = Ok;
    }

    return (enResult);
} /* Hwwdg_DeInit */

/**
 ******************************************************************************
 ** \brief Start the Hardware Watchdog
 **
 ** \param [in] pfnHwwdgCb   Pointer to callback function (Can set to NULL)
 **
 ** \retval Ok                          Setup successful
 ** \retval ErrorOperationInProgress    Hardware Watchdog is active now
 **
 ******************************************************************************/
en_result_t Hwwdg_Start(func_ptr_t pfnHwwdgCb)
{
    en_result_t enResult;

    /* If hardware watchdog is active, error is returned. */
    if (TRUE == FM4_HWWDT->WDG_CTL_f.INTEN)
    {
        enResult = ErrorOperationInProgress;
    }
    else
    {
        /* Set callback pointer to global variable for HW-Wdg-ISR */
        pfnHwwdgCallback = pfnHwwdgCb;

        /* HW Watchdog Control Register unlock sequence */
        FM4_HWWDT->WDG_LCK = HWWDG_REG_UNLOCK_1;
        FM4_HWWDT->WDG_LCK = HWWDG_REG_UNLOCK_2;

        /* Enable interrupt and count */
        FM4_HWWDT->WDG_CTL_f.INTEN = TRUE;

        enResult = Ok;
    }

    return (enResult);
} /* Hwwdg_Start */

/**
 ******************************************************************************
 ** \brief Stop the Hardware Watchdog
 **
 ******************************************************************************/
void Hwwdg_Stop(void)
{
    if (TRUE == FM4_HWWDT->WDG_CTL_f.INTEN)
    {
        /* HW Watchdog Control Register unlock sequence */
        FM4_HWWDT->WDG_LCK = HWWDG_REG_UNLOCK_1;
        FM4_HWWDT->WDG_LCK = HWWDG_REG_UNLOCK_2;

        /* Disable interrupt and count */
        FM4_HWWDT->WDG_CTL_f.INTEN = FALSE;
    }
} /* Hwwdg_Stop */

/**
 ******************************************************************************
 ** \brief Write the load value for Hardware Watchdog
 **
 ** \param [in]  u32LoadValue   Load value
 **
 ******************************************************************************/
void Hwwdg_WriteWdgLoad(uint32_t u32LoadValue)
{
    FM4_HWWDT->WDG_LCK = HWWDG_REG_UNLOCK_1;    /* Release Lock */
    FM4_HWWDT->WDG_LDR = u32LoadValue;          /* Write load value */
} /* Hwwdg_WriteWdgLoad */

/**
 ******************************************************************************
 ** \brief Read the count value for Hardware Watchdog
 **
 ** \retval uint32_t:Value of value register
 **
 ******************************************************************************/
uint32_t Hwwdg_ReadWdgValue(void)
{
    return (FM4_HWWDT->WDG_VLR);
} /* Hwwdg_ReadWdgValue */

/**
 ******************************************************************************
 ** \brief Feed Hardware Watchdog (Call function)
 **
 ** This function feeds the Hardware Watchdog with the unlock, feed, and lock
 ** sequence. Take care of the arbitrary values, because there are not checked
 ** for plausibility!
 **
 ** \param [in] u8ClearPattern1   Pattern of arbitrary value
 ** \param [in] u8ClearPattern2   Inverted arbitrary value
 **
 ******************************************************************************/
void Hwwdg_Feed(uint8_t u8ClearPattern1,
                uint8_t u8ClearPattern2
               )
{
    FM4_HWWDT->WDG_LCK = HWWDG_REG_UNLOCK_1;    /* Release Lock */
    FM4_HWWDT->WDG_ICL = u8ClearPattern1;       /* Clear possible interrupt and reload value, step 1 */
    FM4_HWWDT->WDG_ICL = u8ClearPattern2;       /* Clear possible interrupt and reload value, step 2 */
} /* Hwwdg_Feed */

/**
 ******************************************************************************
 ** \brief Enable Debug Break Watchdog Timer Control
 **
 ******************************************************************************/
void Hwwdg_EnableDbgBrkWdgCtl(void)
{
    stc_crg_dbwdt_ctl_field_t stcDBWDT_CTL;

    stcDBWDT_CTL         = FM4_CRG->DBWDT_CTL_f;
    stcDBWDT_CTL.DPHWBE  = TRUE;
    FM4_CRG->DBWDT_CTL_f = stcDBWDT_CTL;
} /* Hwwdt_EnableDbgBrkWdtCtl */

/**
 ******************************************************************************
 ** \brief Disable Debug Break Watchdog Timer Control
 **
 ******************************************************************************/
void Hwwdg_DisableDbgBrkWdgCtl(void)
{
    stc_crg_dbwdt_ctl_field_t stcDBWDT_CTL;

    stcDBWDT_CTL         = FM4_CRG->DBWDT_CTL_f;
    stcDBWDT_CTL.DPHWBE  = FALSE;
    FM4_CRG->DBWDT_CTL_f = stcDBWDT_CTL;
} /* Hwwdt_DisableDbgBrkWdtCtl */

#endif /* #if (PDL_PERIPHERAL_ENABLE_HWWDG == PDL_ON) */

//@} // HwwdgGroup

#endif /* #if (defined(PDL_PERIPHERAL_WDG_ACTIVE)) */

/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
