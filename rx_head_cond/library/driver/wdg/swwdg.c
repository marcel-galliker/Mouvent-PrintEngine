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
/** \file swwdg.c
 **
 ** A detailed description is available at 
 ** @link SwwdgGroup Software Watchdog Module description @endlink
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
 ** \addtogroup SwwdgGroup
 ******************************************************************************/
//@{

#if (PDL_PERIPHERAL_ENABLE_SWWDG == PDL_ON)
/******************************************************************************/
/* Local pre-processor symbols/macros ('#define')                             */
/******************************************************************************/
/* for WDOGLOAD */
#define SWWDG_DEFAULT_VAL   (0xFFFFFFFFu)

/* for WDOGCONTROL */
#define SWWDG_CTL_INTEN     (0x01u)
#define SWWDG_CTL_RESEN     (0x02u)
#define SWWDG_CTL_TWD100    (0x00u)
#define SWWDG_CTL_TWD75     (0x04u)
#define SWWDG_CTL_TWD50     (0x08u)
#define SWWDG_CTL_TWD25     (0x0Cu)
#define SWWDG_CTL_SPM       (0x10u)

/* for WDOGSPMC */
#define SWWDG_SPMC_TGR      (0x01u)

/******************************************************************************/
/* Global variable definitions (declared in header file with 'extern')        */
/******************************************************************************/
static func_ptr_t pfnSwwdgCallback; ///< callback function pointer for SW-Wdg Irq

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
 ** \brief Software Watchdog Interrupt Handler
 ******************************************************************************/
void SwwdgIrqHandler(void)
{
    /* Check software interrupt status */
    if (TRUE == FM4_SWWDT->WDOGRIS_f.RIS)
    {
        if (NULL != pfnSwwdgCallback)
        {
            pfnSwwdgCallback();
        }
    }
} /* SwwdgIrqHandler */

/**
 ******************************************************************************
 ** \brief Initialize Software Watchdog
 **
 ** \param [in] pstcConfig   Pointer to Software Watchdog configuration
 **
 ** \retval Ok                      Setup successful
 ** \retval ErrorInvalidParameter   pstcConfig == NULL
 **
 ** \note This function only initializes the Software Watchdog configuration.
 **       If Swwdg_Start() is called, MCU start the Software Watchdog.
 **
 ******************************************************************************/
en_result_t Swwdg_Init(stc_swwdg_config_t* pstcConfig)
{
    en_result_t enResult;
    uint8_t     u8WdogControl = 0;  /* Preset Watchdog Control Register */
    uint8_t     u8WdogSpmc    = 0;  /* Preset Window Watchdog Control Register */

    enResult = ErrorInvalidParameter;

    /* Check for NULL Pointer */
    if (NULL != pstcConfig)
    {
        enResult = Ok;
        /* Un Lock */
        FM4_SWWDT->WDOGLOCK = SWWDG_REG_UNLOCK;

        /* Window watchdog mode */
        if (TRUE == pstcConfig->bWinWdgEnable)
        {
            /* Set SPM bit */
            u8WdogControl |= SWWDG_CTL_SPM;
            /* Set reset enable when reload without timing window was occured */
            if (TRUE == pstcConfig->bWinWdgResetEnable)
            {
                u8WdogSpmc = SWWDG_SPMC_TGR;
            }
            /* Set timing window for window watchdog mode */
            switch (pstcConfig->u8TimingWindow)
            {
                case en_swwdg_timing_window_100:
                    u8WdogControl |= SWWDG_CTL_TWD100;
                    break;
                case en_swwdg_timing_window_75:
                    u8WdogControl |= SWWDG_CTL_TWD75;
                    break;
                case en_swwdg_timing_window_50:
                    u8WdogControl |= SWWDG_CTL_TWD50;
                    break;
                case en_swwdg_timing_window_25:
                    u8WdogControl |= SWWDG_CTL_TWD25;
                    break;
                default:
                    enResult = ErrorInvalidParameter;
                    break;
            }
        }

        if (Ok == enResult)
        {
            /* Set reset enable */
            if (TRUE == pstcConfig->bResetEnable)
            {
                /* Set RESEN bit */
                u8WdogControl |= SWWDG_CTL_RESEN;
            }

            // Set interval
            FM4_SWWDT->WDOGLOAD = pstcConfig->u32LoadValue;

            FM4_SWWDT->WDOGSPMC    = u8WdogSpmc;    /* Setup Window watchdog and */
            FM4_SWWDT->WDOGCONTROL = u8WdogControl; /* Setup SW-Watchdog */
            FM4_SWWDT->WDOGLOCK    = 0;             /* Lock it! */

            /* Initialize callback to NULL */
            pfnSwwdgCallback = NULL;

            /* Enable NVIC */
            NVIC_ClearPendingIRQ(SWDT_IRQn);
            NVIC_EnableIRQ(SWDT_IRQn);
            NVIC_SetPriority(SWDT_IRQn, PDL_IRQ_LEVEL_SWWDG);
        }
    }

    return (enResult);
} /* Swwdg_Init */

/**
 ******************************************************************************
 ** \brief Un-Initialize Software Watchdog
 **
 ******************************************************************************/
void Swwdg_DeInit(void)
{
    /* Disable NVIC */
    NVIC_ClearPendingIRQ(SWDT_IRQn);
    NVIC_DisableIRQ(SWDT_IRQn);
    NVIC_SetPriority(SWDT_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);

    FM4_SWWDT->WDOGLOCK    = SWWDG_REG_UNLOCK;  /* Un Lock */
    FM4_SWWDT->WDOGCONTROL = 0;                 /* Reset SW-Watchdog */
    FM4_SWWDT->WDOGSPMC    = 0;                 /* Reset Window watchdog */
    FM4_SWWDT->WDOGLOAD    = SWWDG_DEFAULT_VAL; /* Set default value to load value */

    pfnSwwdgCallback       = NULL;              /* Initialize callback to NULL */
} /* Swwdg_DeInit */

/**
 ******************************************************************************
 ** \brief Start the Software Watchdog
 **
 ** \param [in] pfnSwwdgCb   Pointer to callback function
 **
 ** \retval Ok                          Setup successful
 ** \retval ErrorOperationInProgress    Software Watchdog is active now
 **
 ** \note Please initialize by calling Swwdt_Init() before executing this function.
 **
 ******************************************************************************/
en_result_t Swwdg_Start(func_ptr_t pfnSwwdgCb)
{
    en_result_t enResult;

    /* If software watchdog is active, error is returned. */
    if (TRUE == FM4_SWWDT->WDOGCONTROL_f.INTEN)
    {
        enResult = ErrorOperationInProgress;
    }
    else
    {
        /* Set callback pointer to global variable for SW-Wdg-ISR */
        pfnSwwdgCallback = pfnSwwdgCb;

        FM4_SWWDT->WDOGLOCK = SWWDG_REG_UNLOCK; /* Un Lock */
        FM4_SWWDT->WDOGCONTROL_f.INTEN = TRUE;  /* Enable interrupt and count */
        FM4_SWWDT->WDOGLOCK = 0;                /* Lock */

        enResult = Ok;
    }

    return (enResult);
} /* Swwdg_Start */

/**
 ******************************************************************************
 ** \brief Stop the Software Watchdog
 **
 ******************************************************************************/
void Swwdg_Stop(void)
{
    if (TRUE == FM4_SWWDT->WDOGCONTROL_f.INTEN)
    {
        FM4_SWWDT->WDOGLOCK = SWWDG_REG_UNLOCK; /* Un Lock */
        FM4_SWWDT->WDOGCONTROL_f.INTEN = FALSE; /* Disable interrupt and count */
        FM4_SWWDT->WDOGLOCK = 0;                /* Lock */
    }
} /* Swwdg_Stop */

/**
 ******************************************************************************
 ** \brief Write the load value for Software Watchdog
 **
 ** \param [in]  u32LoadValue   Load value
 **
 ******************************************************************************/
void Swwdg_WriteWdgLoad(uint32_t u32LoadValue)
{
    FM4_SWWDT->WDOGLOCK = SWWDG_REG_UNLOCK;     /* Un Lock */
    FM4_SWWDT->WDOGLOAD = u32LoadValue;         /* Write the load value */
    FM4_SWWDT->WDOGLOCK = 0;                    /* Lock */
} /* Swwdg_WriteWdgLoad */

/**
 ******************************************************************************
 ** \brief Read the count value for Software Watchdog
 **
 ** \retval uint32_t:Value of value register
 **
 ******************************************************************************/
uint32_t Swwdg_ReadWdgValue(void)
{
    return (FM4_SWWDT->WDOGVALUE);
} /* Swwdg_ReadWdgValue */

/**
 ******************************************************************************
 ** \brief Feed Software Watchdog (Call function)
 **
 ** This function feeds the Software Watchdog with the unlock, feed, and lock
 ** sequence.
 **
 ******************************************************************************/
void Swwdg_Feed(void)
{
    FM4_SWWDT->WDOGLOCK   = SWWDG_REG_UNLOCK;   /* Release Lock */
    FM4_SWWDT->WDOGINTCLR = 1;                  /* Clear possible interrupt and reload value */
    FM4_SWWDT->WDOGLOCK   = 0;                  /* Lock again it! */
} /* Swwdg_Feed */

/**
 ******************************************************************************
 ** \brief Enable Debug Break Watchdog Timer Control
 **
 ******************************************************************************/
void Swwdg_EnableDbgBrkWdgCtl(void)
{
    stc_crg_dbwdt_ctl_field_t stcDBWDT_CTL;

    stcDBWDT_CTL         = FM4_CRG->DBWDT_CTL_f;
    stcDBWDT_CTL.DPSWBE  = TRUE;
    FM4_CRG->DBWDT_CTL_f = stcDBWDT_CTL;
} /* Swwdt_EnableDbgBrkWdtCtl */

/**
 ******************************************************************************
 ** \brief Disable Debug Break Watchdog Timer Control
 **
 ******************************************************************************/
void Swwdg_DisableDbgBrkWdgCtl(void)
{
    stc_crg_dbwdt_ctl_field_t stcDBWDT_CTL;

    stcDBWDT_CTL         = FM4_CRG->DBWDT_CTL_f;
    stcDBWDT_CTL.DPSWBE  = FALSE;
    FM4_CRG->DBWDT_CTL_f = stcDBWDT_CTL;
} /* Swwdt_DisableDbgBrkWdtCtl */

#endif /* #if (PDL_PERIPHERAL_ENABLE_SWWDG == PDL_ON) */

//@} // SwwdgGroup

#endif /* #if (defined(PDL_PERIPHERAL_WDG_ACTIVE)) */

/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
