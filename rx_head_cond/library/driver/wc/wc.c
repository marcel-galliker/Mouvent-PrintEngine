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
/** \file wc.c
 **
 ** A detailed description is available at 
 ** @link WcGroup WC Module description @endlink
 **
 ** History:
 **   - 2013-05-17  0.1  Edison Zhang  First version.
 **   - 2014-05-28  1.0  MWi           Doxygen comments corrected.
 **
 ******************************************************************************/

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "wc.h"

#if (defined(PDL_PERIPHERAL_WC_ACTIVE))

/**
 ******************************************************************************
 ** \addtogroup WcGroup
 ******************************************************************************/
//@{

/******************************************************************************/
/* Local pre-processor symbols/macros ('#define')                             */
/******************************************************************************/
#define WC_INSTANCE_COUNT (uint32_t)(sizeof(m_astcWcInstanceDataLut) / sizeof(m_astcWcInstanceDataLut[0]))

/******************************************************************************/
/* Global variable definitions (declared in header file with 'extern')        */
/******************************************************************************/

/// Look-up table for all enabled WC instances and their internal data
stc_wc_instance_data_t m_astcWcInstanceDataLut[] =
{
    #if (PDL_PERIPHERAL_ENABLE_WC0 == PDL_ON)
    { 
        &WC0,   // pstcInstance
        NULL    // stcInternData (not initialized yet)
    },
    #endif
};

#if (PDL_INTERRUPT_ENABLE_WC0 == PDL_ON)
/******************************************************************************/
/* Local Functions                                                            */
/******************************************************************************/
/**
 ******************************************************************************
 ** \brief Set NVIC Interrupt depending on ADC instance
 **
 ** \param pstcWc Pointer to ADC instance
 **
 ******************************************************************************/
static void Wc_InitNvic(volatile stc_wcn_t* pstcWc)
{
    if (pstcWc == (stc_wcn_t*)(&WC0))
    {
        NVIC_ClearPendingIRQ(WC_IRQn);
        NVIC_EnableIRQ(WC_IRQn);
        NVIC_SetPriority(WC_IRQn, PDL_IRQ_LEVEL_CLK_WC0);   
    }

}

/**
 ******************************************************************************
 ** \brief Clear NVIC Interrupt depending on ADC instance
 **
 ** \param pstcWc Pointer to ADC instance
 **
 ******************************************************************************/
static void Wc_DeInitNvic(volatile stc_wcn_t* pstcWc)
{
    if (pstcWc == (stc_wcn_t*)(&WC0))
    {
        NVIC_ClearPendingIRQ(WC_IRQn);
        NVIC_DisableIRQ(WC_IRQn);
        NVIC_SetPriority(WC_IRQn, PDL_IRQ_LEVEL_CLK_WC0);   
    }
}
/*!
 ******************************************************************************
 ** \brief  Watch counter interrupt function
 **
 ** \param [in] pstcWc WC register definition
 ** \arg    structure of WC register
 **
 ** \param [in]  pstcWcInternData  structure of WC callback function
 ** \arg    structure of stc_wc_intern_data_t
 **
 ** \retval void
 **
 ******************************************************************************/
void Wc_IrqHandler( volatile stc_wcn_t* pstcWc,
                    stc_wc_intern_data_t* pstcWcInternData)
{
    if (PdlSet == Wc_GetIntFlag(pstcWc))
    {
        Wc_ClearIntFlag(pstcWc);
        if (pstcWcInternData->pfnIntCallback != NULL)
        {
            pstcWcInternData->pfnIntCallback();
        }
    }
}
#endif

/**
 ******************************************************************************
 ** \brief Return the internal data for a certain WC instance.
 **
 ** \param pstcWc Pointer to WC instance
 **
 ** \return Pointer to internal data or NULL if instance is not enabled (or not known)
 **
 ******************************************************************************/
static stc_wc_intern_data_t* WcGetInternDataPtr(volatile stc_wcn_t* pstcWc) 
{
    uint32_t u32Instance;
   
    for (u32Instance = 0; u32Instance < WC_INSTANCE_COUNT; u32Instance++)
    {
        if (pstcWc == m_astcWcInstanceDataLut[u32Instance].pstcInstance)
        {
            return &m_astcWcInstanceDataLut[u32Instance].stcInternData;
        }
    }

    return NULL;
}

/**
 ******************************************************************************
 ** \brief Select the input  clock an and set the division clock to be output.
 **
 ** The Function can set SEL_OUT, SEL_IN of Watch counter prescaler (hereafter WCP)
 **
 ** \param [in]  pstcWc            Pointer to WC  instance register area
 ** \param [in]  pstcWcPresClk     WC prescaler clock configuration
 **
 ** \retval Ok                     Write data successfully done or started.
 ** \retval ErrorInvalidParameter  If one of following conditions are met:
 **                                - pstcWc == NULL
 **
 ******************************************************************************/
en_result_t Wc_Pres_SelClk(volatile stc_wcn_t* pstcWc, stc_wc_pres_clk_t* pstcWcPresClk)
{
    // Check for NULL pointers
    if (NULL == pstcWc)
    {
        return ErrorInvalidParameter;
    }
    pstcWc->CLK_SEL_f.SEL_IN = pstcWcPresClk->enInputClk;
    pstcWc->CLK_SEL_f.SEL_OUT = pstcWcPresClk->enOutputClk;
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Enable oscilation of the divison clock.
 **
 ** The Function can set CLK_EN:CLK_EN to 1 of WCP
 **
 ** \param [in]  pstcWc                Pointer to WC  instance register area
 **
 ** \retval Ok                         Write data successfully done or started.
 ** \retval ErrorInvalidParameter      If one of following conditions are met:
 **                                    - pstcWc == NULL
 **
 ******************************************************************************/
en_result_t Wc_Pres_EnableDiv(volatile stc_wcn_t* pstcWc)
{
    // Check for NULL pointers
    if (NULL == pstcWc)
    {
        return ErrorInvalidParameter;
    }
    pstcWc->CLK_EN_f.CLK_EN = 1;
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Disable oscilation of the divison clock.
 **
 ** The Function can clear CLK_EN:CLK_EN to 0 of WCP
 **
 ** \param [in]  pstcWc                Pointer to WC  instance register area
 **
 ** \retval Ok                         Write data successfully done or started.
 ** \retval ErrorInvalidParameter      If one of following conditions are met:
 **                                    - pstcWc == NULL
 **
 ******************************************************************************/
en_result_t Wc_Pres_DisableDiv(volatile stc_wcn_t* pstcWc)
{
    // Check for NULL pointers
    if (NULL == pstcWc)
    {
        return ErrorInvalidParameter;
    }
    pstcWc->CLK_EN_f.CLK_EN = 0;
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Get the operation status of the division counter.
 **
 ** The Function can get CLK_EN:CLK_EN_R to 0 of WCP
 **
 ** \param [in]  pstcWc          Pointer to WC  instance register area
 **
 ** \retval PdlClr               CLK_EN_R is 0, oscilation of the division clock is not performed
 ** \retval PdlSet               CLK_EN R is 1, oscilation of the division clock is performed
 **
 ******************************************************************************/
en_stat_flag_t Wc_GetDivStat(volatile stc_wcn_t* pstcWc)
{
    return ((en_stat_flag_t)(pstcWc->CLK_EN_f.CLK_EN_R));
}

/**
 ******************************************************************************
 ** \brief Select the input  clock an and set the division clock to be output.
 **
 ** The Function can set SEL_OUT, SEL_IN of Watch counter prescaler (hereafter WCP)
 **
 ** \param [in]  pstcWc            Pointer to WC  instance register area
 ** \param [in]  pstcWcConfig      Pointer to WC configuration
 **
 ** \retval Ok                     Write data successfully done or started.
 ** \retval ErrorInvalidParameter  If one of following conditions are met:
 **                                - pstcWc == NULL
 **
 ******************************************************************************/
en_result_t Wc_Init(volatile stc_wcn_t* pstcWc, stc_wc_config_t* pstcWcConfig)
{
    // Pointer to internal data
    stc_wc_intern_data_t* pstcWcInternData ;
    // Check for NULL pointers
    if ((NULL == pstcWc) || (NULL == pstcWcConfig))
    {
        return ErrorInvalidParameter;
    }
    
    // Get pointer to internal data structure ...
    pstcWcInternData = WcGetInternDataPtr( pstcWc );
    
    if(pstcWcInternData == NULL)
    {
        return ErrorInvalidParameter ;
    }
    
    pstcWc->WCCR_f.CS0 = pstcWcConfig->enCntClk;
    pstcWc->WCCR_f.CS1 = ((pstcWcConfig->enCntClk)>>1);
    return Ok;
}


/**
 ******************************************************************************
 ** \brief Enable WC operation.
 **
 ** The Function can set WCCR:WCEN to 1 of WC
 **
 ** \param [in]  pstcWc                Pointer to WC  instance register area
 **
 ** \retval Ok                         Write data successfully done or started.
 ** \retval ErrorInvalidParameter      If one of following conditions are met:
 **                                    - pstcWc == NULL
 **
 ******************************************************************************/
en_result_t Wc_EnableCount(volatile stc_wcn_t* pstcWc)
{
    // Check for NULL pointers
    if (NULL == pstcWc)
    {
        return ErrorInvalidParameter;
    }
    pstcWc->WCCR_f.WCEN = 1;
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Disable WC operation.
 **
 ** The Function can clear WCCR:WCEN to 0 of WC
 **
 ** \param [in]  pstcWc                Pointer to WC  instance register area
 **
 ** \retval Ok                         Write data successfully done or started.
 ** \retval ErrorInvalidParameter      If one of following conditions are met:
 **                                    - pstcWc == NULL
 **
 ******************************************************************************/
en_result_t Wc_DisableCount(volatile stc_wcn_t* pstcWc)
{
    // Check for NULL pointers
    if (NULL == pstcWc)
    {
        return ErrorInvalidParameter;
    }
    pstcWc->WCCR_f.WCEN = 0;
    return Ok;
}

#if (PDL_INTERRUPT_ENABLE_WC0 == PDL_ON)
/**
 ******************************************************************************
 ** \brief Enable WC underflow interrupt.
 **
 ** The Function can set WCIE of WC
 **
 ** \param [in]  pstcWc             Pointer to WC  instance register area
 ** \param [in]  pfnIntCallback     WC interrupt to be enabled
 **
 ** \retval Ok                      Write data successfully done or started.
 ** \retval ErrorInvalidParameter   If one of following conditions are met:
 **                                 - pstcWc == NULL
 **
 ******************************************************************************/
en_result_t Wc_EnableInt(volatile stc_wcn_t* pstcWc, func_ptr_t  pfnIntCallback)
{
    stc_wc_intern_data_t* pstcWcInternData;
    // check for non-null pointers
    if (NULL == pstcWc)
    {
        return ErrorInvalidParameter;
    }
    pstcWc->WCCR_f.WCIE = 1;
    // Get ptr to internal data struct ...
    pstcWcInternData = WcGetInternDataPtr((stc_wcn_t*) pstcWc);
    // ... and check
    if (NULL == pstcWcInternData)
    {
        return ErrorInvalidParameter;
    }
    pstcWcInternData->pfnIntCallback = pfnIntCallback;
    Wc_InitNvic(pstcWc);
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Disable WC underflow interrupt.
 **
 ** The Function can clear WCIE of WC
 **
 ** \param [in]  pstcWc               Pointer to WC instance register area
 **
 ** \retval Ok                        Write data successfully done or started.
 ** \retval ErrorInvalidParameter     If one of following conditions are met:
 **                                   - pstcWc == NULL
 **
 ******************************************************************************/
en_result_t Wc_DisableInt(volatile stc_wcn_t* pstcWc)
{
    // check for non-null pointers
    if (NULL == pstcWc)
    {
        return ErrorInvalidParameter;
    }
    pstcWc->WCCR_f.WCIE = 0;
    Wc_DeInitNvic(pstcWc);
    return Ok;
}
#endif
/**
 ******************************************************************************
 ** \brief Set the counter value of WC
 **
 ** The Function can set value to WCRL of WC
 **
 ** \param [in]  pstcWc               Pointer to WC instance register area
 ** \param [in]  u8Val                Value of counter
 **
 ** \retval Ok                        Write data successfully done.
 ** \retval ErrorInvalidParameter     If one of following conditions are met:
 **                                   - pstcWc == NULL
 **
 ******************************************************************************/
en_result_t Wc_WriteReloadVal(volatile stc_wcn_t* pstcWc, uint8_t u8Val)
{
    // check for non-null pointers
    if (NULL == pstcWc)
    {
        return ErrorInvalidParameter;
    }
    pstcWc->WCRL = u8Val;
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Read the value in the 6-bit down counter
 **
 ** The Function can read value of WCRD of WC
 **
 ** \param [in]  pstcWc      Pointer to WC instance register area
 **
 ** \retval Ok               Write data successfully done or started.
 ** \return                  The value of WC 6-bit down counter
 **                          If pstcWc == NULL 0xFF is returned.
 **
 ******************************************************************************/
uint8_t Wc_ReadCurCnt(volatile stc_wcn_t* pstcWc)
{
    // check for non-null pointers
    if (NULL == pstcWc)
    {
        return 0xFF;
    }
    return (pstcWc->WCRD);
}

/**
 ******************************************************************************
 ** \brief Clear WC underflow flag
 **
 ** The Function clear WCIF to 0 of WC
 **
 ** \param [in]  pstcWc      Pointer to WC instance register area
 **
 ** \retval Ok               Write data successfully done or started.
 ** \return                  The value of WC 6-bit down counter
 **                          If pstcWc == NULL 0xFF is returned.
 **
 ******************************************************************************/
en_result_t Wc_ClearIntFlag(volatile stc_wcn_t* pstcWc)
{
    // check for non-null pointers
    if (NULL == pstcWc)
    {
        return ErrorInvalidParameter;
    }
    pstcWc->WCCR_f.WCIF = 0;
    return Ok;    
}

/**
 ******************************************************************************
 ** \brief Get WC underflow flag status
 **
 ** The Function get value WCIF of WC
 **
 ** \param [in]  pstcWc          Pointer to WC instance register area
 **
 ** \retval PdlClr               WCIF is 0, WC underflow does not occur
 ** \retval PdlSet               WCIF is 1, WC underflow occurs
 **
 ******************************************************************************/
en_int_flag_t Wc_GetIntFlag(volatile stc_wcn_t* pstcWc)
{
    return ((en_int_flag_t)(pstcWc->WCCR_f.WCIF));
}

/**
 ******************************************************************************
 ** \brief Get WC operation state
 **
 ** The Function get value WCOP of WC
 **
 ** \param [in]  pstcWc          Pointer to WC instance register area
 **
 ** \retval PdlClr               WCOP is 0, The WC is stopped
 ** \retval PdlSet               WCOP is 1, The WC is active
 **
 ******************************************************************************/
en_stat_flag_t Wc_GetOperationFlag(volatile stc_wcn_t* pstcWc)
{
    return ((en_stat_flag_t)(pstcWc->WCCR_f.WCOP));
}

//@}

#endif // #if (defined(PDL_PERIPHERAL_WC_ACTIVE))

/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/

