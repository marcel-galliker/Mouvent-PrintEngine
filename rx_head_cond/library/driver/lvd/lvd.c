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
/** \file lvd.c
 **
 ** A detailed description is available at 
 ** @link LvdGroup Low voltage Detection description @endlink
 **
 ** History:
 **   - 2013-04-03  1.0  NT   First version.
 **
 ******************************************************************************/

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "pdl.h"

#if (defined(PDL_PERIPHERAL_LVD_ACTIVE))

/**
 ******************************************************************************
 ** \addtogroup LvdGroup
 ******************************************************************************/
//@{

/******************************************************************************/
/* Local pre-processor symbols/macros ('#define')                             */
/******************************************************************************/

/******************************************************************************/
/* Global variable definitions (declared in header file with 'extern')        */
/******************************************************************************/
static stc_lvd_intern_data_t stcLvdInternData = {NULL};

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
/* for SVHI of LVD_CTL */
#define LvdSVHI_220     (0x00)
#define LvdSVHI_240     (0x01)
#define LvdSVHI_245     (0x03)
#define LvdSVHI_260     (0x02)
#define LvdSVHI_270     (0x06)
#define LvdSVHI_280     (0x07)
#define LvdSVHI_290     (0x05)
#define LvdSVHI_300     (0x04)
#define LvdSVHI_320     (0x0C)
#define LvdSVHI_350     (0x0D)
#define LvdSVHI_360     (0x0F)
#define LvdSVHI_370     (0x0E)
#define LvdSVHI_380     (0x0A)
#define LvdSVHI_390     (0x0B)
#define LvdSVHI_400     (0x09)
#define LvdSVHI_410     (0x08)
#define LvdSVHI_420     (0x18)
#define LvdSVHI_430     (0x19)
#define LvdSVHI_452     (0x1B)

/* Unlock code for LVD_RLR */
#define LvdCtlUnlock    (0x1ACCE553)

/**
 ******************************************************************************
 ** \brief ISR of the LVD module
 ** 
 ******************************************************************************/
void LvdIrqHandler(void)
{
    FM4_LVD->LVD_CLR = 0x00;  /* Clear LVD interrupt (LVDCL (bit #7)) */

    if (NULL != stcLvdInternData.pfnCallback)
    {
        stcLvdInternData.pfnCallback();
    }
} /* LvdIrqHandler */

/**
 ******************************************************************************
 ** \brief Init the LVD module
 ** 
 ** The LVD (reset) is always active.
 ** Generating interrupts is able to use by specifying callback function.
 ** If callback function is not specified (NULL), interrupt (LVD function)
 ** is enabled, but NVIC is not enabled.
 ** So user has to check interrupt status by #Lvd_GetIntStatus.
 ** This function enables the interrupt and the user setting voltage for
 ** the LVD module.
 **
 ** \param [in]  pstcConfig    LVD module configuration 
 **
 ** \retval Ok                    Initializiation of LVD interupt successfully 
 **                               done.
 ** \retval ErrorInvalidParameter pstcConfig == NULL or invalid 
 **                               stc_lvd_config#enIrqVoltage setting.
 ******************************************************************************/
en_result_t Lvd_Init(stc_lvd_config_t* pstcConfig)
{
    en_result_t             enResult;
    stc_lvd_lvd_ctl_field_t stcLVD_CTL = {0};   /* local LVD_CTL predefintion */

    enResult = ErrorInvalidParameter;
    /* Check for valid pointer */
    if (NULL != pstcConfig)
    {
        enResult = Ok;

        /* Enable interrupt */
        stcLVD_CTL.LVDIE = TRUE;

        /* Prepare LVD_CTL */
        switch (pstcConfig->enIrqVoltage)
        {
            case LvdIrqVoltage220:  /* Interrupt when voltage is vicinity of 2.20 volts */
                stcLVD_CTL.SVHI = LvdSVHI_220;
                break;
            case LvdIrqVoltage240:  /* Interrupt when voltage is vicinity of 2.40 volts */
                stcLVD_CTL.SVHI = LvdSVHI_240;
                break;
            case LvdIrqVoltage245:  /* Interrupt when voltage is vicinity of 2.45 volts */
                stcLVD_CTL.SVHI = LvdSVHI_245;
                break;
            case LvdIrqVoltage260:  /* Interrupt when voltage is vicinity of 2.60 volts */
                stcLVD_CTL.SVHI = LvdSVHI_260;
                break;
            case LvdIrqVoltage270:  /* Interrupt when voltage is vicinity of 2.70 volts */
                stcLVD_CTL.SVHI = LvdSVHI_270;
                break;
            case LvdIrqVoltage280:  /* Interrupt when voltage is vicinity of 2.80 volts */
                stcLVD_CTL.SVHI = LvdSVHI_280;
                break;
            case LvdIrqVoltage290:  /* Interrupt when voltage is vicinity of 2.90 volts */
                stcLVD_CTL.SVHI = LvdSVHI_290;
                break;
            case LvdIrqVoltage300:  /* Interrupt when voltage is vicinity of 3.00 volts */
                stcLVD_CTL.SVHI = LvdSVHI_300;
                break;
            case LvdIrqVoltage320:  /* Interrupt when voltage is vicinity of 3.20 volts */
                stcLVD_CTL.SVHI = LvdSVHI_320;
                break;
            case LvdIrqVoltage350:  /* Interrupt when voltage is vicinity of 3.50 volts */
                stcLVD_CTL.SVHI = LvdSVHI_350;
                break;
            case LvdIrqVoltage360:  /* Interrupt when voltage is vicinity of 3.60 volts */
                stcLVD_CTL.SVHI = LvdSVHI_360;
                break;
            case LvdIrqVoltage370:  /* Interrupt when voltage is vicinity of 3.70 volts */
                stcLVD_CTL.SVHI = LvdSVHI_370;
                break;
            case LvdIrqVoltage380:  /* Interrupt when voltage is vicinity of 3.80 volts */
                stcLVD_CTL.SVHI = LvdSVHI_380;
                break;
            case LvdIrqVoltage390:  /* Interrupt when voltage is vicinity of 3.90 volts */
                stcLVD_CTL.SVHI = LvdSVHI_390;
                break;
            case LvdIrqVoltage400:  /* Interrupt when voltage is vicinity of 4.00 volts */
                stcLVD_CTL.SVHI = LvdSVHI_400;
                break;
            case LvdIrqVoltage410:  /* Interrupt when voltage is vicinity of 4.10 volts */
                stcLVD_CTL.SVHI = LvdSVHI_410;
                break;
            case LvdIrqVoltage420:  /* Interrupt when voltage is vicinity of 4.20 volts */
                stcLVD_CTL.SVHI = LvdSVHI_420;
                break;
            case LvdIrqVoltage430:  /* Interrupt when voltage is vicinity of 4.30 volts */
                stcLVD_CTL.SVHI = LvdSVHI_430;
                break;
            case LvdIrqVoltage452:  /* Interrupt when voltage is vicinity of 4.52 volts */
                stcLVD_CTL.SVHI = LvdSVHI_452;
                break;
            default:
                enResult = ErrorInvalidParameter;
                break;
        }

        if (Ok == enResult)
        {
            /* Setup callback function pointer in internal data */
            stcLvdInternData.pfnCallback = pstcConfig->pfnCallback;

            /* If callback function is set, interrupt is used. */
            if (NULL != pstcConfig->pfnCallback)
            {
                /* Init NVIC */
                NVIC_ClearPendingIRQ(LVD_IRQn);
                NVIC_EnableIRQ(LVD_IRQn);
                NVIC_SetPriority(LVD_IRQn, PDL_IRQ_LEVEL_LVD);
            }

            FM4_LVD->LVD_CLR = 0x00;        /* Clear possible pending LVD interrupt (LVDCL (bit #7)) */
            FM4_LVD->LVD_RLR = LvdCtlUnlock;/* unlock LVD_CTL */
            FM4_LVD->LVD_CTL_f = stcLVD_CTL;

            FM4_LVD->LVD_RLR = 0;           /* lock LVD_CTL */
        }
    }

    return (enResult);
} /* Lvd_InitIrq */

/**
 ******************************************************************************
 ** \brief De-Init the Interrupt of the LVD module
 **
 ** \retval Ok                    De-Initializiation of LVD interupt
 **                               successfully done.
 **
 ******************************************************************************/
en_result_t Lvd_DeInit(void)
{
    /* De-Init NVIC */
    if (NULL != stcLvdInternData.pfnCallback)
    {
        NVIC_ClearPendingIRQ(LVD_IRQn);
        NVIC_DisableIRQ(LVD_IRQn);
        NVIC_SetPriority(LVD_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);
    }

    FM4_LVD->LVD_RLR = LvdCtlUnlock;/* unlock LVD_CTL */
    FM4_LVD->LVD_CTL = 0;           /* Clea all (LVDIE (bit #7)) */
    FM4_LVD->LVD_RLR = 0;           /* lock LVD_CTL */

    stcLvdInternData.pfnCallback = NULL;

    return (Ok);
} /* Lvd_DeInitIrq */

/**
 ******************************************************************************
 ** \brief Get the LVD interrupt status
 ** 
 ** \retval FALSE       A low-voltage detection is not detected.
 ** \retval TRUE        A low-voltage detection has been detected.
 ** 
 ******************************************************************************/
boolean_t Lvd_GetIntStatus(void)
{
    boolean_t bRetVal = FALSE;

    /* Check the LVD interrupt status */
    if (TRUE == FM4_LVD->LVD_STR_f.LVDIR)
    {
        bRetVal = TRUE;
    }
    return (bRetVal);
} /* Lvd_GetIntOperationStatus */

/**
 ******************************************************************************
 ** \brief Clear the LVD interrupt status
 **
 ******************************************************************************/
void Lvd_ClearIntStatus(void)
{
    FM4_LVD->LVD_CLR = 0x00;  /* Clear LVD interrupt (LVDCL (bit #7)) */
} /* Lvd_ClearIntStatus */

/**
 ******************************************************************************
 ** \brief Get the operation status of the LVD interrupt
 **
 ** \retval FALSE       Stabilization wait state or monitoring stop state
 ** \retval TRUE        Monitoring state
 **
 ******************************************************************************/
boolean_t Lvd_GetIntOperationStatus(void)
{
    boolean_t bRetVal = FALSE;

    /* Check the operation status of LVD interrupt */
    if (TRUE == FM4_LVD->LVD_STR2_f.LVDIRDY)
    {
        bRetVal = TRUE;
    }
    return (bRetVal);
} /* Lvd_GetIntOperationStatus */

//@} // LvdGroup

#endif /* #if (defined(PDL_PERIPHERAL_LVD_ACTIVE)) */

/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
