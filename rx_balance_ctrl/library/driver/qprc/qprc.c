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
/** \file qprc.c
 **
 ** A detailed description is available at 
 ** @link QprcGroup QPRC Module description @endlink
 **
 ** History:
 **   - 2013-03-21  0.1  Daniel  First version.
 **
 *****************************************************************************/

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "qprc.h"

#if (defined(PDL_PERIPHERAL_QPRC_ACTIVE))

/**
 ******************************************************************************
 ** \addtogroup QprcGroup
 ******************************************************************************/
//@{

/******************************************************************************/
/* Local pre-processor symbols/macros ('#define')                             */
/******************************************************************************/
#define QPRC_INSTANCE_COUNT (uint32_t)(sizeof(m_astcQprcInstanceDataLut) / sizeof(m_astcQprcInstanceDataLut[0]))

/******************************************************************************/
/* Global variable definitions (declared in header file with 'extern')        */
/******************************************************************************/

/******************************************************************************/
/* Local type definitions ('typedef')                                         */
/******************************************************************************/

/******************************************************************************/
/* Local function prototypes ('static')                                       */
/******************************************************************************/
static en_result_t QprcIsValidQprcnPtr(volatile stc_qprcn_t *pstcQprc);
static volatile stc_qprc_nfn_t* QprcGetNoisFilterPtr( volatile stc_qprcn_t *pstcQprc );
#if (PDL_INTERRUPT_ENABLE_QPRC0 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_QPRC1 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_QPRC2 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_QPRC3 == PDL_ON)
static stc_qprc_intern_data_t *QprcGetInternDataPtr( volatile stc_qprcn_t *pstcQprc );
#endif
/******************************************************************************/
/* Local variable definitions ('static')                                      */
/******************************************************************************/

/******************************************************************************/
/* Function implementation - global ('extern') and local ('static')           */
/******************************************************************************/
/// Look-up table for all enabled ADC instances and their internal data
stc_qprc_instance_data_t m_astcQprcInstanceDataLut[] =
{
#if (PDL_PERIPHERAL_ENABLE_QPRC0 == PDL_ON)
    {
        &QPRC0,   // pstcInstance QPRC
        &QPRCNF0, // pstcInstanceNf QPRC-NF
        NULL      // stcInternData (not initialized yet)
    },
#endif
#if (PDL_PERIPHERAL_ENABLE_QPRC1 == PDL_ON)
    {
        &QPRC1,   // pstcInstance QPRC
        &QPRCNF1, // pstcInstanceNf QPRC-NF
        NULL      // stcInternData (not initialized yet)
    },
#endif
#if (PDL_PERIPHERAL_ENABLE_QPRC2 == PDL_ON)
    {
        &QPRC2,   // pstcInstance QPRC
        &QPRCNF2, // pstcInstanceNf QPRC-NF
        NULL      // stcInternData (not initialized yet)
    },
#endif
#if (PDL_PERIPHERAL_ENABLE_QPRC3 == PDL_ON)
    {
        &QPRC3,   // pstcInstance QPRC
        &QPRCNF3, // pstcInstanceNf QPRC-NF
        NULL      // stcInternData (not initialized yet)
    },
#endif

};

/**
 ******************************************************************************
 ** \brief  A certain QPRC instance is valid?
 **
 ** \param   [in]  pstcQprc          QPRC instance
 **
 ** \retval    Ok    Pointer to a certain QPRC instance is valid
 **              NULL if instance is not enabled (or not known)
 **
 ******************************************************************************/
static en_result_t QprcIsValidQprcnPtr(volatile stc_qprcn_t *pstcQprc)
{
    uint32_t u32Instance;

    if (NULL == pstcQprc)
    {
        return ErrorInvalidParameter;
    }

    for (u32Instance = 0; u32Instance < QPRC_INSTANCE_COUNT; u32Instance++)
    {
        if (pstcQprc == m_astcQprcInstanceDataLut[u32Instance].pstcInstance)
        {
            return Ok;
        }
    }

    return ErrorInvalidParameter;
}

/**
 ******************************************************************************
 ** \brief Get the noise filter for a certain QPRC instance
 **
 ** \param   [in]  pstcQprc          QPRC instance
 **
 ** \retval    Pointer to the noise filter
 **              NULL if instance is not enabled (or not known)
 **
 ******************************************************************************/
volatile stc_qprc_nfn_t *QprcGetNoisFilterPtr( volatile stc_qprcn_t *pstcQprc )
{
    uint32_t u32Instance;

    for (u32Instance = 0; u32Instance < QPRC_INSTANCE_COUNT; u32Instance++)
    {
        if (pstcQprc == m_astcQprcInstanceDataLut[u32Instance].pstcInstance)
        {
            return m_astcQprcInstanceDataLut[u32Instance].pstcInstanceNf;
        }
    }

    return (stc_qprc_nfn_t *)NULL;
}


#if (PDL_INTERRUPT_ENABLE_QPRC0 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_QPRC1 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_QPRC2 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_QPRC3 == PDL_ON)

/**
 ******************************************************************************
 ** \brief Get the internal data for a certain QPRC instance
 **
 ** \param   [in]  pstcQprc          QPRC instance
 **
 ** \retval    Pointer to internal data
 **              NULL if instance is not enabled (or not known)
 **
 ******************************************************************************/
static  stc_qprc_intern_data_t *QprcGetInternDataPtr( volatile stc_qprcn_t *pstcQprc )
{
    uint32_t u32Instance;

    for (u32Instance = 0; u32Instance < QPRC_INSTANCE_COUNT; u32Instance++)
    {
        if (pstcQprc == m_astcQprcInstanceDataLut[u32Instance].pstcInstance)
        {
            return &m_astcQprcInstanceDataLut[u32Instance].stcInternData;
        }
    }

    return NULL;
}      
      
/**
 ******************************************************************************
 ** \brief Set NVIC Interrupt depending on QPRC instance
 **
 ** \param pstcQprc  Pointer to QPRC instance
 **
 ******************************************************************************/
static void Qprc_InitNvic(volatile stc_qprcn_t* pstcQprc)
{
    if (pstcQprc == (stc_qprcn_t*)(&QPRC0))
    {
        NVIC_ClearPendingIRQ(QPRC0_IRQn);
        NVIC_EnableIRQ(QPRC0_IRQn);
        NVIC_SetPriority(QPRC0_IRQn, PDL_IRQ_LEVEL_QPRC0);
    }
    else if (pstcQprc == (stc_qprcn_t*)(&QPRC1))
    {
        NVIC_ClearPendingIRQ(QPRC1_IRQn);
        NVIC_EnableIRQ(QPRC1_IRQn);
        NVIC_SetPriority(QPRC1_IRQn, PDL_IRQ_LEVEL_QPRC1);
    }
    else if (pstcQprc == (stc_qprcn_t*)(&QPRC2))
    {
        NVIC_ClearPendingIRQ(QPRC2_IRQn);
        NVIC_EnableIRQ(QPRC2_IRQn);
        NVIC_SetPriority(QPRC2_IRQn, PDL_IRQ_LEVEL_QPRC2);
    }
}

/**
 ******************************************************************************
 ** \brief Clear NVIC Interrupt depending on QPRC instance
 **
 ** \param pstcQprc  Pointer to QPRC instance
 **
 ******************************************************************************/
static void Qprc_DeInitNvic(volatile stc_qprcn_t* pstcQprc)
{
    if (pstcQprc == (stc_qprcn_t*)(&QPRC0))
    {
        NVIC_ClearPendingIRQ(QPRC0_IRQn);
        NVIC_DisableIRQ(QPRC0_IRQn);
        NVIC_SetPriority(QPRC0_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);
    }
    else if (pstcQprc == (stc_qprcn_t*)(&QPRC1))
    {
        NVIC_ClearPendingIRQ(QPRC1_IRQn);
        NVIC_DisableIRQ(QPRC1_IRQn);
        NVIC_SetPriority(QPRC1_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);
    }
    else if (pstcQprc == (stc_qprcn_t*)(&QPRC2))
    {
        NVIC_ClearPendingIRQ(QPRC2_IRQn);
        NVIC_DisableIRQ(QPRC2_IRQn);
        NVIC_SetPriority(QPRC2_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);
    }
}

/**
 ******************************************************************************
 ** \brief QPRC instance interrupt service routine
 **
 ** \param [in]  pstcQprc                       QPRC instance
 ** \param [in]  pstcQprcInternData             Pointer to internal data
 ******************************************************************************/

void Qprc_IrqHandler ( volatile stc_qprcn_t *pstcQprc,
                       stc_qprc_intern_data_t *pstcQprcInternData )
{
    if ((NULL == pstcQprc) ||
        (NULL == pstcQprcInternData))
    {
        return;
    }

    if (PdlSet == pstcQprc->QICRL_f.QPCMF)    // QPCCR match ?
    {
        pstcQprc->QICRL_f.QPCMF = PdlClr;    // Clear interrupt
        if (NULL != pstcQprcInternData->pfnPcMatchCallback)
        {
            pstcQprcInternData->pfnPcMatchCallback();
        }
    }

    if (PdlSet == pstcQprc->QICRL_f.QPRCMF)   // QPRCR match ?
    {
        pstcQprc->QICRL_f.QPRCMF = PdlClr;   // Clear interrupt
        if (NULL != pstcQprcInternData->pfnPcRcMatchCallback)
        {
            pstcQprcInternData->pfnPcRcMatchCallback();
        }
    }

    if (PdlSet == pstcQprc->QICRL_f.OFDF)    //Overflow Interrupe ?
    {
        pstcQprc->QICRL_f.OFDF = PdlClr;     // Clear interrupt
        if (NULL != pstcQprcInternData->pfnPcUfOfZeroCallback)
        {
            pstcQprcInternData->pfnPcUfOfZeroCallback(QPRC_PC_OVERFLOW_INT);
        }
    }

    if (PdlSet == pstcQprc->QICRL_f.UFDF)  //Underflow Interrupe ?
    {
        pstcQprc->QICRL_f.UFDF = PdlClr;   // Clear interrupt
        if (NULL != pstcQprcInternData->pfnPcUfOfZeroCallback)
        {
            pstcQprcInternData->pfnPcUfOfZeroCallback(QPRC_PC_UNDERFLOW_INT);
        }
    }

    if (PdlSet == pstcQprc->QICRL_f.ZIIF)    //Zero Interrupe ?
    {
        pstcQprc->QICRL_f.ZIIF = PdlClr;     // Clear interrupt
        if (NULL != pstcQprcInternData->pfnPcUfOfZeroCallback)
        {
            pstcQprcInternData->pfnPcUfOfZeroCallback(QPRC_PC_ZERO_INDEX_INT);
        }
    }

    if (PdlSet == pstcQprc->QICRH_f.CDCF)    //PC invert match ?
    {
        pstcQprc->QICRH_f.CDCF = PdlClr;     // Clear interrupt
        if (NULL != pstcQprcInternData->pfnPcCountInvertCallback)
        {
            pstcQprcInternData->pfnPcCountInvertCallback();
        }
    }

    if (PdlSet == pstcQprc->QECR_f.ORNGF)    //RC outrange ?
    {
        pstcQprc->QECR_f.ORNGF = PdlClr;     // Clear interrupt
        if (NULL != pstcQprcInternData->pfnRcOutrangeCallback)
        {
            pstcQprcInternData->pfnRcOutrangeCallback();
        }
    }

    if (PdlSet == pstcQprc->QICRH_f.QPCNRCMF)     //PcMatch and RcMatch ?
    {
        pstcQprc->QICRH_f.QPCNRCMF = PdlClr;      // Clear interrupt
        if (NULL != pstcQprcInternData->pfnPcMatchRcMatchCallback)
        {
            pstcQprcInternData->pfnPcMatchRcMatchCallback();
        }
    }

    return;
}

#endif

/**
 ******************************************************************************
 ** \brief Initialize QPRC
 **
 ** This function initializes an QPRC module and sets up the internal
 ** data structures
 **
 ** \param [in]  pstcQprc          QPRC instance
 ** \param [in]  pstcConfig       QPRC module configuration
 **
 ** \retval Ok                    Internal data has been setup
 ** \retval ErrorInvalidParameter pstcQprc == NULL
 **                                          pstcConfig == NULL
 **                                          Other invalid configuration
 ******************************************************************************/
en_result_t Qprc_Init( volatile stc_qprcn_t* pstcQprc,
                            stc_qprc_config_t*  pstcConfig )
{
    volatile stc_qprc_nfn_t *pstcQprcInternNfn;

    if ( (Ok != QprcIsValidQprcnPtr(pstcQprc)) ||
         (NULL == pstcConfig)
       )
    {
        return ErrorInvalidParameter;
    }

    if (((pstcConfig->bSwapAinBin != TRUE) && (pstcConfig->bSwapAinBin != FALSE))||
        (pstcConfig->enComapreMode > QprcComapreWithRevolution) ||
        (pstcConfig->enZinEdge > QprcZinHighLevel) ||
        (pstcConfig->enBinEdge > QprcBinBothEdges) ||
        (pstcConfig->enAinEdge > QprcAinBothEdges) ||
        (pstcConfig->enPcResetMask > QprcResetMask8Times) ||
        (pstcConfig->stcAinFilter.enWidth > QprcFilterWidth256Pclk) ||
        (pstcConfig->stcBinFilter.enWidth > QprcFilterWidth256Pclk) ||
        (pstcConfig->stcCinFilter.enWidth > QprcFilterWidth256Pclk) ||
        ((pstcConfig->stcAinFilter.bInputMask != TRUE) && (pstcConfig->stcAinFilter.bInputMask != FALSE))||
        ((pstcConfig->stcBinFilter.bInputMask != TRUE) && (pstcConfig->stcBinFilter.bInputMask != FALSE))||
        ((pstcConfig->stcCinFilter.bInputMask != TRUE) && (pstcConfig->stcCinFilter.bInputMask != FALSE))||
        ((pstcConfig->stcAinFilter.bInputInvert != TRUE) && (pstcConfig->stcAinFilter.bInputInvert != FALSE))||
        ((pstcConfig->stcBinFilter.bInputInvert != TRUE) && (pstcConfig->stcBinFilter.bInputInvert != FALSE))||
        ((pstcConfig->stcCinFilter.bInputInvert != TRUE) && (pstcConfig->stcCinFilter.bInputInvert != FALSE)))

    {
        return ErrorInvalidParameter;
    }

    pstcQprc->QCRL_f.SWAP = (TRUE == pstcConfig->bSwapAinBin) ? 1 : 0 ;
    pstcQprc->QCRL_f.RSEL = pstcConfig->enComapreMode;

    switch (pstcConfig->enZinEdge)
    {
        case QprcZinDisable: // Disables edge and level detection
        {
            pstcQprc->QCRH_f.CGE0 = 0;
            pstcQprc->QCRH_f.CGE1 = 0;
        }
        break;

        case QprcZinFallingEdge: // ZIN active at falling edge
        {
            pstcQprc->QCRL_f.CGSC = 0;
            pstcQprc->QCRH_f.CGE0 = 1;
            pstcQprc->QCRH_f.CGE1 = 0;
        }
        break;

        case QprcZinRisingEdge: // ZIN active at rising edge
        {
            pstcQprc->QCRL_f.CGSC = 0;
            pstcQprc->QCRH_f.CGE0 = 0;
            pstcQprc->QCRH_f.CGE1 = 1;
        }
        break;

        case QprcZinBothEdges: // ZIN active at falling or rising edge
        {
            pstcQprc->QCRL_f.CGSC = 0;
            pstcQprc->QCRH_f.CGE0 = 1;
            pstcQprc->QCRH_f.CGE1 = 1;
        }
        break;

        case QprcZinLowLevel: // ZIN active at low level detected
        {
            pstcQprc->QCRL_f.CGSC = 1;
            pstcQprc->QCRH_f.CGE0 = 1;
            pstcQprc->QCRH_f.CGE1 = 0;
        }
        break;

        case QprcZinHighLevel: // ZIN active at high level detected
        {
            pstcQprc->QCRL_f.CGSC = 1;
            pstcQprc->QCRH_f.CGE0 = 0;
            pstcQprc->QCRH_f.CGE1 = 1;
        }
        break;

        default:
            break;
    }

    switch (pstcConfig->enBinEdge)
    {
        case QprcBinDisable: // Disables edge detection
        {
            pstcQprc->QCRH_f.BES0 = 0;
            pstcQprc->QCRH_f.BES1 = 0;
        }
        break;

        case QprcBinFallingEdge: // BIN active at falling edge
        {
            pstcQprc->QCRH_f.BES0 = 1;
            pstcQprc->QCRH_f.BES1 = 0;
        }
        break;

        case QprcBinRisingEdge: // BIN active at rising edge
        {
            pstcQprc->QCRH_f.BES0 = 0;
            pstcQprc->QCRH_f.BES1 = 1;
        }
        break;

        case QprcBinBothEdges: // BIN active at falling or rising edge
        {
            pstcQprc->QCRH_f.BES0 = 1;
            pstcQprc->QCRH_f.BES1 = 1;
        }
        break;

        default:
            break;
    }

    switch (pstcConfig->enAinEdge)
    {
        case QprcBinDisable: // Disables edge detection
        {
            pstcQprc->QCRH_f.AES0 = 0;
            pstcQprc->QCRH_f.AES1 = 0;
        }
        break;

        case QprcBinFallingEdge: // BIN active at falling edge
        {
            pstcQprc->QCRH_f.AES0 = 1;
            pstcQprc->QCRH_f.AES1 = 0;
        }
        break;

        case QprcBinRisingEdge: // BIN active at rising edge
        {
            pstcQprc->QCRH_f.AES0 = 0;
            pstcQprc->QCRH_f.AES1 = 1;
        }
        break;

        case QprcBinBothEdges: // BIN active at falling or rising edge
        {
            pstcQprc->QCRH_f.AES0 = 1;
            pstcQprc->QCRH_f.AES1 = 1;
        }
        break;

        default:
            break;
    }

    switch (pstcConfig->enPcResetMask)
    {
        case QprcResetMaskDisable: // No reset mask
        {
            pstcQprc->QCRH_f.PCRM0 = 0;
            pstcQprc->QCRH_f.PCRM1 = 0;
        }
        break;

        case QprcResetMask2Times:
        {
            pstcQprc->QCRH_f.PCRM0 = 1;
            pstcQprc->QCRH_f.PCRM1 = 0;
        }
        break;

        case QprcResetMask4Times:
        {
            pstcQprc->QCRH_f.PCRM0 = 0;
            pstcQprc->QCRH_f.PCRM1 = 1;
        }
        break;

        case QprcResetMask8Times:
        {
            pstcQprc->QCRH_f.PCRM0 = 1;
            pstcQprc->QCRH_f.PCRM1 = 1;
        }
        break;

        default:
            break;
    }

    pstcQprc->QECR_f.ORNGMD = (TRUE == pstcConfig->b8KValue) ? 1 : 0 ; // TRUE: Outrange mode from 0 to 0x7FFF, FALSE: Outrange mode from 0 to 0xFFFF:

    /* Set QPRC Noise Filter */
    pstcQprcInternNfn = QprcGetNoisFilterPtr(pstcQprc);

    pstcQprcInternNfn->NFCTLA_f.AINMD = (TRUE == pstcConfig->stcAinFilter.bInputMask) ? 1 : 0 ;
    pstcQprcInternNfn->NFCTLA_f.AINLV = (TRUE == pstcConfig->stcAinFilter.bInputInvert) ? 1 : 0 ;
    pstcQprcInternNfn->NFCTLA_f.AINNWS = pstcConfig->stcAinFilter.enWidth;

    pstcQprcInternNfn->NFCTLB_f.BINMD = (TRUE == pstcConfig->stcBinFilter.bInputMask) ? 1 : 0 ;
    pstcQprcInternNfn->NFCTLB_f.BINLV = (TRUE == pstcConfig->stcBinFilter.bInputInvert) ? 1 : 0 ;
    pstcQprcInternNfn->NFCTLB_f.BINNWS = pstcConfig->stcBinFilter.enWidth;

    pstcQprcInternNfn->NFRCTLZ_f.ZINMD = (TRUE == pstcConfig->stcCinFilter.bInputMask) ? 1 : 0 ;
    pstcQprcInternNfn->NFRCTLZ_f.ZINLV = (TRUE == pstcConfig->stcCinFilter.bInputInvert) ? 1 : 0 ;
    pstcQprcInternNfn->NFRCTLZ_f.ZINNWS = pstcConfig->stcCinFilter.enWidth;

    return Ok;
}

/**
 ******************************************************************************
 ** \brief Enable Position Counter
 **
 ** \param [in]  pstcQprc           QPRC instance
 **
 ** \retval Ok                    Enable Position Counter counting  successfully
 ** \retval ErrorInvalidParameter pstcQprc == NULL
 ******************************************************************************/
en_result_t Qprc_StopPcCount(volatile stc_qprcn_t *pstcQprc)
{
    // Check for NULL pointer
    if (NULL == pstcQprc)
    {
        return ErrorInvalidParameter ;
    }

    pstcQprc->QCRL_f.PSTP = 0;
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Disable Position Counter
 **
 ** \param [in]  pstcQprc           QPRC instance
 **
 ** \retval Ok                    Disable Position Counter counting  successfully
 ** \retval ErrorInvalidParameter pstcQprc == NULL
 ******************************************************************************/
en_result_t Qprc_RestartPcCount(volatile stc_qprcn_t *pstcQprc)
{
    // Check for NULL pointer
    if (NULL == pstcQprc)
    {
        return ErrorInvalidParameter ;
    }

    pstcQprc->QCRL_f.PSTP = 1;
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Set count value of Position counter
 **
 ** \param [in]  pstcQprc          QPRC instance
 ** \param [in]  u16PcValue      Count value
 **
 ** \retval Ok                    Count value has been setup
 ** \retval ErrorInvalidParameter pstcQprc == NULL
 **                                          Other invalid configuration
 ******************************************************************************/
en_result_t Qprc_SetPcCount ( volatile stc_qprcn_t *pstcQprc,
                                    uint16_t u16PcValue )
{
    if (NULL == pstcQprc)
    {
        return ErrorInvalidParameter;
    }

    pstcQprc->QPCR = u16PcValue;
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Get count value of Position counter
 **
 ** \param [in]  pstcQprc          QPRC instance
 **
 ** \retval Count value
 ******************************************************************************/
uint16_t Qprc_GetPcCount ( volatile stc_qprcn_t *pstcQprc )
{
    if (NULL == pstcQprc)
    {
        return ErrorInvalidParameter;
    }

    return (uint16_t)pstcQprc->QPCR;
}

/**
 ******************************************************************************
 ** \brief Set count value of Revolution Counter
 **
 ** \param [in]  pstcQprc          QPRC instance
 ** \param [in]  u16RcValue      Count value
 **
 ** \retval Ok                    Count value has been setup
 ** \retval ErrorInvalidParameter pstcQprc == NULL
 **                                          Other invalid configuration
 ******************************************************************************/
en_result_t Qprc_SetRcCount ( volatile stc_qprcn_t *pstcQprc,
                                    uint16_t u16RcValue )
{
    if (NULL == pstcQprc)
    {
        return ErrorInvalidParameter;
    }

    pstcQprc->QRCR = u16RcValue;
    return Ok;
}


/**
 ******************************************************************************
 ** \brief Get count value of Revolution Counter
 **
 ** \param [in]  pstcQprc          QPRC instance
 **
 ** \retval Count value
 ******************************************************************************/
uint16_t Qprc_GetRcCount ( volatile stc_qprcn_t *pstcQprc )
{
    if (NULL == pstcQprc)
    {
        return ErrorInvalidParameter;
    }

    return (uint16_t)pstcQprc->QPCR;
}

/**
 ******************************************************************************
 ** \brief Set maximum count value of Position Counter
 **
 ** \param [in]  pstcQprc           QPRC instance
 ** \param [in]  u16PcMaxValue      Maximum count value
 **
 ** \retval Ok                    Maximum count value has been setup
 ** \retval ErrorInvalidParameter pstcQprc == NULL
 **                               Other invalid configuration
 ******************************************************************************/
en_result_t Qprc_SetPcMaxValue( volatile stc_qprcn_t *pstcQprc,
                                        uint16_t u16PcMaxValue )
{
    if (NULL == pstcQprc)
    {
        return ErrorInvalidParameter;
    }

    pstcQprc->QMPR = u16PcMaxValue;
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Get maximum count value of Position Counter
 **
 ** \param [in]  pstcQprc           QPRC instance
 **
 ** \retval  PcMaxValue
 ******************************************************************************/
uint16_t Qprc_GetPcMaxValue(volatile stc_qprcn_t *pstcQprc)
{
    if (NULL == pstcQprc)
    {
        return ErrorInvalidParameter;
    }

    return (uint16_t)pstcQprc->QMPR;
}

/**
 ******************************************************************************
 ** \brief Set compare value of Position counter
 **
 ** \param [in]  pstcQprc        QPRC instance
 ** \param [in]  u16PcValue      Compare value
 **
 ** \retval Ok                    Compare value has been setup
 ** \retval ErrorInvalidParameter pstcQprc == NULL
 **                                          Other invalid configuration
 ******************************************************************************/

en_result_t Qprc_SetPcCompareValue( volatile stc_qprcn_t *pstcQprc,
                                            uint16_t u16PcValue )
{
    if (NULL == pstcQprc)
    {
        return ErrorInvalidParameter;
    }

    pstcQprc->QPCCR = u16PcValue;

    return Ok;
}

/**
 ******************************************************************************
 ** \brief Get compare value of Position counter
 **
 ** \param [in]  pstcQprc               QPRC instance
 **
 ** \retval Count value
 ******************************************************************************/

uint16_t Qprc_GetPcCompareValue( volatile stc_qprcn_t *pstcQprc)
{
    if (NULL == pstcQprc)
    {
        return ErrorInvalidParameter;
    }

    return (uint16_t)pstcQprc->QPCCR;
}


/**
 ******************************************************************************
 ** \brief Set compare value of Position and Revolution Counter
 **
 ** \param [in]  pstcQprc          QPRC instance
 ** \param [in]  u16PcRcValue      Compare value
 **
 ** \retval Ok                     Compare value has been setup
 ** \retval ErrorInvalidParameter  pstcQprc == NULL
 **                                Other invalid configuration
 ******************************************************************************/
en_result_t Qprc_SetPcRcCompareValue( volatile stc_qprcn_t *pstcQprc,
                                                uint16_t u16PcRcValue )
{
    if (NULL == pstcQprc)
    {
        return ErrorInvalidParameter;
    }

    pstcQprc->QPRCR = u16PcRcValue;

    return Ok;
}

/**
 ******************************************************************************
 ** \brief Get compare value of Position and Revolution Counter
 **
 ** \param [in]  pstcQprc               QPRC instance
 **
 ** \retval Count value
 ******************************************************************************/
uint16_t Qprc_GetPcRcCompareValue(volatile stc_qprcn_t *pstcQprc)
{
    if (NULL == pstcQprc)
    {
        return ErrorInvalidParameter;
    }

    return (uint16_t)pstcQprc->QPRCR;
}

/**
 ******************************************************************************
 ** \brief Set Position Counter mode
 **
 ** \param [in]  pstcQprc               QPRC instance
 ** \param [in]  enMode                 Position Counter mode
 **
 ** \retval Ok                          Position Counter mode has been setup
 ** \retval ErrorInvalidParameter       pstcQprc == NULL
 **                                     enMode  > QprcPcMode3
 **                                     Other invalid configuration
 ******************************************************************************/

en_result_t Qprc_ConfigPcMode( volatile stc_qprcn_t *pstcQprc,
                                        en_qprc_pcmode_t enMode )
{
    if ((NULL == pstcQprc) ||
        (enMode > QprcPcMode3))
    {
        return ErrorInvalidParameter;
    }

    switch(enMode)
    {
        case QprcPcMode0: //Disable position counter
        {
            pstcQprc->QCRL_f.PCM0 = 0;
            pstcQprc->QCRL_f.PCM1 = 0;
        }
        break;

        case QprcPcMode1: //Increments with AIN active edge and decrements with BIN active edge
        {
            pstcQprc->QCRL_f.PCM0 = 1;
            pstcQprc->QCRL_f.PCM1 = 0;
        }
        break;

        case QprcPcMode2: //Phase difference count mode: Counts up if AIN is leading BIN, down if BIN leading.
        {
            pstcQprc->QCRL_f.PCM0 = 0;
            pstcQprc->QCRL_f.PCM1 = 1;
        }
        break;

        case QprcPcMode3: // Directional count mode: Counts up/down with BIN active edge and AIN level
        {
            pstcQprc->QCRL_f.PCM0 = 1;
            pstcQprc->QCRL_f.PCM1 = 1;
        }
        break;

        default:
            return ErrorInvalidParameter;
    }

    return Ok;
}

/**
 ******************************************************************************
 ** \brief Set Revolution Counter mode
 **
 ** \param [in]  pstcQprc         QPRC instance
 ** \param [in]  enMode           Revolution Counter mode
 **
 ** \retval Ok                    Revolution Counter mode has been setup
 ** \retval ErrorInvalidParameter pstcQprc == NULL
 **                               enMode  > QprcRcMode3
 **                               Other invalid configuration
 ******************************************************************************/
en_result_t Qprc_ConfigRcMode( volatile stc_qprcn_t *pstcQprc,
                                        en_qprc_rcmode_t enMode )
{
    if ((NULL == pstcQprc) ||
        (enMode > QprcRcMode3))
    {
        return ErrorInvalidParameter;
    }

    switch(enMode)
    {
        case QprcRcMode0:  ///< RC_Mode0: Disable revolution counter
        {
            pstcQprc->QCRL_f.RCM0 = 0;
            pstcQprc->QCRL_f.RCM1 = 0;
        }
        break;

        case QprcRcMode1: //Up/down count of RC with ZIN active edge
        {
            pstcQprc->QCRL_f.RCM0 = 1;
            pstcQprc->QCRL_f.RCM1 = 0;
        }
        break;

        case QprcRcMode2: //Up/down count of RC on over or underflow in position count match
        {
            pstcQprc->QCRL_f.RCM0 = 0;
            pstcQprc->QCRL_f.RCM1 = 1;
        }
        break;

        case QprcRcMode3: //Up/down count of RC on over or underflow in position count match and ZIN active edge
        {
            pstcQprc->QCRL_f.RCM0 = 1;
            pstcQprc->QCRL_f.RCM1 = 1;
        }
        break;

        default:
            return ErrorInvalidParameter;
    }

    return Ok;
}

#if (PDL_INTERRUPT_ENABLE_QPRC0 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_QPRC1 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_QPRC2 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_QPRC3 == PDL_ON)
/**
 ******************************************************************************
 ** \brief Enable PC match interrupt
 **
 ** \param [in]  pstcQprc               QPRC instance
 ** \param [in]  pstcIntSel             Interrupt selection structure
 ** \param [in]  pstcIntCallback        Interrupt Callback function pointer
 **
 ** \retval Ok                    PC match interrupt enable bit has been setup
 ** \retval ErrorInvalidParameter pstcQprc invalid
 **                               Other invalid configuration
 ******************************************************************************/
en_result_t Qprc_EnableInt( volatile stc_qprcn_t* pstcQprc, stc_qprc_int_sel_t* pstcIntSel,
                            stc_qprc_int_cb_t*  pstcIntCallback )
{
    stc_qprc_intern_data_t *pstcQprcInternData = NULL;
    if ((NULL == pstcQprc) ||
       (pstcIntSel->bQprcPcCountInvertInt > 1) ||
       (pstcIntSel->bQprcPcMatchInt > 1) ||
       (pstcIntSel->bQprcPcMatchRcMatchInt > 1) ||
       (pstcIntSel->bQprcPcOfUfZeroInt > 1) ||
       (pstcIntSel->bQprcPcRcMatchInt > 1) ||
       (pstcIntSel->bQprcRcOutrangeInt > 1) ||
       (NULL == pstcIntCallback)  )
    {
        return ErrorInvalidParameter;
    }

    pstcQprcInternData = QprcGetInternDataPtr(pstcQprc);

    if (NULL == pstcQprcInternData)
    {
        return ErrorInvalidParameter;
    }
    if (TRUE == pstcIntSel->bQprcPcMatchInt)
    {
        pstcQprcInternData->pfnPcMatchCallback = pstcIntCallback->pfnPcMatchIntCallback;
        pstcQprc->QICRL_f.QPCMIE = 1;
    }
    if (TRUE == pstcIntSel->bQprcPcRcMatchInt)
    {
        pstcQprcInternData->pfnPcRcMatchCallback = pstcIntCallback->pfnPcRcMatchIntCallback;
        pstcQprc->QICRL_f.QPRCMIE = 1;
    }
    if (TRUE == pstcIntSel->bQprcPcMatchRcMatchInt)
    {
        pstcQprcInternData->pfnPcMatchRcMatchCallback = pstcIntCallback->pfnPcMatchRcMatchIntCallback;
        pstcQprc->QICRH_f.QPCNRCMIE = 1;
    }
    if (TRUE == pstcIntSel->bQprcPcOfUfZeroInt)
    {
        pstcQprcInternData->pfnPcUfOfZeroCallback = pstcIntCallback->pfnPcOfUfZeroIntCallback;
        pstcQprc->QICRL_f.OUZIE = 1;
    }
    if (TRUE == pstcIntSel->bQprcPcCountInvertInt)
    {
        pstcQprcInternData->pfnPcCountInvertCallback = pstcIntCallback->pfnPcCountInvertIntCallback;
        pstcQprc->QICRH_f.CDCIE = 1;
    }
    if (TRUE == pstcIntSel->bQprcRcOutrangeInt)
    {
        pstcQprcInternData->pfnRcOutrangeCallback = pstcIntCallback->pfnRcOutrangeIntCallback;
        pstcQprc->QECR_f.ORNGIE = 1;
    }

    Qprc_InitNvic(pstcQprc);

    return Ok;
}

/**
 ******************************************************************************
 ** \brief Disable PC match interrupt
 **
 ** \param [in]  pstcQprc         QPRC instance
 ** \param [in]  pstcIntSel       Interrupt Callback function structure pointer
 **
 ** \retval Ok                    PC match interrupt enable bit has been setup
 ** \retval ErrorInvalidParameter pstcQprc invalid
 **                               Other invalid configuration
 ******************************************************************************/
en_result_t Qprc_DisableInt( volatile stc_qprcn_t* pstcQprc, stc_qprc_int_sel_t* pstcIntSel )
{
    stc_qprc_intern_data_t *pstcQprcInternData = NULL;
    if((NULL == pstcQprc) ||
       (pstcIntSel->bQprcPcCountInvertInt > 1) ||
       (pstcIntSel->bQprcPcMatchInt > 1) ||
       (pstcIntSel->bQprcPcMatchRcMatchInt > 1) ||
       (pstcIntSel->bQprcPcOfUfZeroInt > 1) ||
       (pstcIntSel->bQprcPcRcMatchInt > 1) ||
       (pstcIntSel->bQprcRcOutrangeInt > 1))
    {
        return ErrorInvalidParameter;
    }

    pstcQprcInternData = QprcGetInternDataPtr(pstcQprc);

    if (NULL == pstcQprcInternData)
    {
        return ErrorInvalidParameter;
    }
    if (TRUE == pstcIntSel->bQprcPcMatchInt)
    {
        pstcQprcInternData->pfnPcMatchCallback = NULL;
        pstcQprc->QICRL_f.QPCMIE = 0;
    }
    if (TRUE == pstcIntSel->bQprcPcRcMatchInt)
    {
        pstcQprcInternData->pfnPcRcMatchCallback = NULL;
        pstcQprc->QICRL_f.QPRCMIE = 0;
    }
    if (TRUE == pstcIntSel->bQprcPcMatchRcMatchInt)
    {
        pstcQprcInternData->pfnPcMatchRcMatchCallback = NULL;
        pstcQprc->QICRH_f.QPCNRCMIE = 0;
    }
    if (TRUE == pstcIntSel->bQprcPcOfUfZeroInt)
    {
        pstcQprcInternData->pfnPcUfOfZeroCallback = NULL;
        pstcQprc->QICRL_f.OUZIE = 0;
    }
    if (TRUE == pstcIntSel->bQprcPcCountInvertInt)
    {
        pstcQprcInternData->pfnPcCountInvertCallback = NULL;
        pstcQprc->QICRH_f.CDCIE = 0;
    }
    if (TRUE == pstcIntSel->bQprcRcOutrangeInt)
    {
        pstcQprcInternData->pfnRcOutrangeCallback = NULL;
        pstcQprc->QECR_f.ORNGIE = 0;
    }

    if((0 == pstcQprc->QICRL_f.QPCMIE)    &&
       (0 == pstcQprc->QICRL_f.QPRCMIE)   &&
       (0 == pstcQprc->QICRH_f.QPCNRCMIE) &&
       (0 == pstcQprc->QICRL_f.OUZIE) &&
       (0 == pstcQprc->QICRH_f.CDCIE) &&
       (0 == pstcQprc->QECR_f.ORNGIE)  )
    {
        Qprc_DeInitNvic(pstcQprc);
    }

    return Ok;
}

#endif

/**
 ******************************************************************************
 ** \brief Get interrupt flag of QPRC
 **
 ** \param [in]  pstcQprc            QPRC instance
 ** \param [in]  enIntType           Interrupt type
 **
 ** \retval PdlClr                   Don't occur interrupt which corresponds with enIntType
 ** \retval PdlSet                   Occur interrupt which corresponds with enIntType
 ******************************************************************************/
en_int_flag_t Qprc_GetIntFlag( volatile stc_qprcn_t *pstcQprc,
                                    en_qprc_int_t enIntType)
{
    en_int_flag_t enIntFlag = PdlClr;

    if ((NULL == pstcQprc) ||
        (enIntType > QprcRcOutrangeInt))
    {
        return PdlClr;
    }

    switch(enIntType)
    {
        case QprcPcOfUfZeroInt:
        {
            if((PdlSet == pstcQprc->QICRL_f.ZIIF) ||
               (PdlSet == pstcQprc->QICRL_f.OFDF) ||
               (PdlSet == pstcQprc->QICRL_f.UFDF))
            {
                enIntFlag = PdlSet;
            }
        }
        break;

        case QprcPcMatchInt:
        {
            enIntFlag = (1 == pstcQprc->QICRL_f.QPCMF) ? PdlSet : PdlClr ;
        }
        break;

        case QprcPcRcMatchInt:
        {
            enIntFlag = (1 == pstcQprc->QICRL_f.QPRCMF) ? PdlSet : PdlClr ;
        }
        break;

        case QprcPcMatchRcMatchInt:
        {
            enIntFlag = (1 == pstcQprc->QICRH_f.QPCNRCMF) ? PdlSet : PdlClr ;
        }
        break;

        case QprcPcCountInvertInt:
        {
            enIntFlag = (1 == pstcQprc->QICRH_f.CDCF) ? PdlSet : PdlClr ;
        }
        break;

        case QprcRcOutrangeInt:
        {
            enIntFlag = (1 == pstcQprc->QECR_f.ORNGF) ? PdlSet : PdlClr ;
        }
        break;

        default:
            break;
    }

    return enIntFlag;
}


/**
 ******************************************************************************
 ** \brief Clear interrupt flag of QPRC
 **
 ** \param [in]  pstcQprc            QPRC instance
 ** \param [in]  enIntType           Interrupt type
 **
 ** \retval Ok                       Cleared interrupt flag which corresponds with enIntType
 ** \retval ErrorInvalidParameter    pstcQprc invalid
 **                                  enIntType > QprcRcOutrangeInt
 **                                  Other invalid configuration
 ******************************************************************************/
en_result_t Qprc_ClrIntFlag( volatile stc_qprcn_t *pstcQprc,
                                en_qprc_int_t enIntType)
{
    if ((NULL == pstcQprc) ||
        (enIntType > QprcRcOutrangeInt))
    {
        return ErrorInvalidParameter;
    }

    switch(enIntType)
    {
        case QprcPcOfUfZeroInt:
        {
            pstcQprc->QICRL_f.ZIIF = PdlClr;
            pstcQprc->QICRL_f.OFDF = PdlClr;
            pstcQprc->QICRL_f.UFDF = PdlClr;
        }
        break;

        case QprcPcMatchInt:
        {
            pstcQprc->QICRL_f.QPCMF = PdlClr;
        }
        break;

        case QprcPcRcMatchInt:
        {
            pstcQprc->QICRL_f.QPRCMF = PdlClr;
        }
        break;

        case QprcPcMatchRcMatchInt:
        {
            pstcQprc->QICRH_f.QPCNRCMF = PdlClr;
        }
        break;

        case QprcPcCountInvertInt:
        {
            pstcQprc->QICRH_f.CDCF = PdlClr;
        }
        break;

        case QprcRcOutrangeInt:
        {
            pstcQprc->QECR_f.ORNGF = PdlClr;
        }
        break;

        default:
            break;
    }

    return Ok;

}

/**
 ******************************************************************************
 ** \brief Get last position counter flow direction
 **
 ** \param [in]  pstcQprc             QPRC instance
 **
 ** \retval PdlClr                    The position counter was incremented.
 ** \retval PdlSet                    The position counter was decremented.
 ******************************************************************************/
en_stat_flag_t Qprc_GetPcOfUfDir( volatile stc_qprcn_t *pstcQprc )
{
    en_stat_flag_t enStatFlag = PdlClr;

    if (NULL == pstcQprc)
    {
        return PdlClr;
    }

    enStatFlag = (1 == pstcQprc->QICRH_f.DIROU) ? PdlSet : PdlClr ;
    return enStatFlag;
}

/**
 ******************************************************************************
 ** \brief Get last position counter direction
 **
 ** \param [in]  pstcQprc             QPRC instance
 **
 ** \retval PdlClr                    The position counter was incremented.
 ** \retval PdlSet                    The position counter was decremented.
 ******************************************************************************/
en_stat_flag_t Qprc_GetPcDir( volatile stc_qprcn_t *pstcQprc )
{
    en_stat_flag_t enStatFlag = PdlClr;

    if (NULL == pstcQprc)
    {
        return PdlClr;
    }

    enStatFlag = (1 == pstcQprc->QICRH_f.DIRPC) ? PdlSet : PdlClr ;

    return enStatFlag;
}



//@} // QprcGroup


#endif // #if (defined(PDL_PERIPHERAL_QPRC_ACTIVE))

/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
