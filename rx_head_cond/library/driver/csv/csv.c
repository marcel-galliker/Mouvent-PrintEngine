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
/** \file csv.c
 **
 ** A detailed description is available at 
 ** @link CsvGroup CSV Module description @endlink
 **
 ** History:
 **   - 2013-05-09  1.0  Edison Zhang  First version.
 **
 ******************************************************************************/ 

/*---------------------------------------------------------------------------*/
/* include files                                                             */
/*---------------------------------------------------------------------------*/
#include "csv.h"

#if (defined(PDL_PERIPHERAL_CSV_ACTIVE))

/**
 ******************************************************************************
 ** \addtogroup CsvGroup
 ******************************************************************************/
//@{

/*---------------------------------------------------------------------------*/
/* local defines                                                             */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* local datatypes                                                           */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* local data                                                                */
/*---------------------------------------------------------------------------*/
#if (PDL_INTERRUPT_ENABLE_CSV == PDL_ON) 
static fn_fcs_int_callback *m_pfnIntCallback = NULL;
#endif

/*---------------------------------------------------------------------------*/
/* local functions prototypes                                                */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* global data                                                               */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* global functions                                                          */
/*---------------------------------------------------------------------------*/

#if (PDL_INTERRUPT_ENABLE_CSV == PDL_ON) 
/*!
 ******************************************************************************
 ** \brief CSV interrupt handler
 ******************************************************************************
 */
void Csv_IrqHandler(void)
{
    bFM4_CRG_INT_CLR_FCSC = 1;
    m_pfnIntCallback();
}

/*!
 ******************************************************************************
 ** \brief Enables FCS interrupts
 ******************************************************************************
 */
en_result_t Csv_EnableFcsInt(fn_fcs_int_callback* pfnIntCallback)
{
    if(pfnIntCallback == NULL)
    {
        return ErrorInvalidParameter;
    }
    
    m_pfnIntCallback = pfnIntCallback;
    bFM4_CRG_INT_ENR_FCSE = 1;
    
    NVIC_ClearPendingIRQ(CSV_IRQn); 
    NVIC_EnableIRQ(CSV_IRQn); 
    NVIC_SetPriority(CSV_IRQn, PDL_IRQ_LEVEL_CSV);
    
    return Ok;
}

/*!
 ******************************************************************************
 ** \brief Disables FCS interrupts
 ******************************************************************************
 */
void Csv_DisableFcsInt(void) 
{
    bFM4_CRG_INT_ENR_FCSE = 0;  
    
    NVIC_ClearPendingIRQ(CSV_IRQn);
    NVIC_DisableIRQ(CSV_IRQn);
    NVIC_SetPriority(CSV_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);   
    
}
#endif

/*!
 ******************************************************************************
 ** \brief The main CSV function is enabled
 ******************************************************************************
 */
void Csv_EnableMainCsv(void) 
{
    bFM4_CRG_CSV_CTL_MCSVE = 1;
}

/*!
 ******************************************************************************
 ** \brief The main CSV function is disabled
 ******************************************************************************
 */
void Csv_DisableMainCsv(void) 
{
    bFM4_CRG_CSV_CTL_MCSVE = 0;  
}

/*!
 ******************************************************************************
 ** \brief The sub CSV function is enabled.
 ******************************************************************************
 */
void Csv_EnableSubCsv(void) 
{
    bFM4_CRG_CSV_CTL_SCSVE = 1;  
}

/*!
 ******************************************************************************
 ** \brief The sub CSV function is disabled¡£
 ******************************************************************************
 */
void Csv_DisableSubCsv(void) 
{
    bFM4_CRG_CSV_CTL_SCSVE = 0; 
}

/*!
 ******************************************************************************
 ** \brief Get CSV status
 **
 ** \param pstcCsvStatus Pointer to status information struture of CSV 
 **
 ** \retval Ok
 ******************************************************************************
 */
uint8_t Csv_GetCsvFailCause(stc_csv_status_t* pstcCsvStatus)
{ 
    uint8_t u8Status;
    
    u8Status = FM4_CRG->CSV_STR;
    
    if((u8Status & 0x01) == 0x01)
    {
        pstcCsvStatus->bCsvMainClockStatus = TRUE;
    }
    else
    {
        pstcCsvStatus->bCsvMainClockStatus = FALSE;
    }
    
    if((u8Status & 0x02) == 0x02)
    {
        pstcCsvStatus->bCsvSubClockStatus = TRUE;
    }
    else
    {
        pstcCsvStatus->bCsvSubClockStatus = FALSE;
    }
    
    return Ok;;
}

/*!
 ******************************************************************************
 ** \brief The FCS function is enabled.
 ******************************************************************************
 */
void Csv_EnableFcs(void) 
{
    bFM4_CRG_CSV_CTL_FCSDE = 1;
}

/*!
 ******************************************************************************
 ** \brief The FCS function is disabled
 ******************************************************************************
 */
void Csv_DisableFcs(void) 
{
    bFM4_CRG_CSV_CTL_FCSDE = 0;
}

/*!
 ******************************************************************************
 ** \brief The FCS reset is enabled.
 ******************************************************************************
 */
void Csv_EnableFcsReset(void) 
{
    bFM4_CRG_CSV_CTL_FCSRE = 1;
}


/*!
 ******************************************************************************
 ** \brief The FCS reset is disabled.
 ******************************************************************************
 */
void Csv_DisableFcsReset(void) 
{
    bFM4_CRG_CSV_CTL_FCSRE = 0;
}

/*!
 ******************************************************************************
 ** \brief Clears the FCS interrupt cause.
 ******************************************************************************
 */
void Csv_ClrFcsIntFlag(void) 
{
    bFM4_CRG_INT_CLR_FCSC = 1;  
}

/*!
 ******************************************************************************
 ** \brief Get Anomalous frequency detection interrupt status
 **
 ** \return interrupt status
 ** \retval 0 No FCS interrupt has been asserted.
 ** \retval 1 An FCS interrupt has been asserted.
 ******************************************************************************
 */
uint8_t Csv_GetFcsIntFlag(void) 
{
    return bFM4_CRG_INT_STR_FCSI;
}

/*!
 ******************************************************************************
 ** \brief FCS count cycle setting
 **
 ** \param enDiv 
 ** \arg FcsCrDiv256      1/256 frequency of high-speed CR oscillation   
 ** \arg FcsCrDiv512      1/512 frequency of high-speed CR oscillation
 ** \arg FcsCrDiv1024     1/1024 frequency of high-speed CR oscillation
 **
 ** \retval Ok
 ******************************************************************************
 */
en_result_t Csv_SetFcsCrDiv(en_fcs_cr_div_t enDiv) 
{ 
    if ((enDiv != FcsCrDiv256) && 
        (enDiv != FcsCrDiv512) && 
        (enDiv != FcsCrDiv1024))
    {
        return ErrorInvalidParameter;
    }

    FM4_CRG->CSV_CTL &= 0x0fff;
    FM4_CRG->CSV_CTL |= (enDiv<<12);
    
    return Ok;
}


/*!
 ******************************************************************************
 ** \brief Frequency lower detection window setting
 **
 ** \param u16LowerVal Lower value 
 ** \param u16UpperVal limit value
 ******************************************************************************
 */
void Csv_SetFcsDetectRange(uint16_t u16LowerVal, uint16_t u16UpperVal) 
{
    FM4_CRG->FCSWL_CTL = u16LowerVal;  
    FM4_CRG->FCSWH_CTL = u16UpperVal;
}

/*!
 ******************************************************************************
 ** \brief Get the counter value of frequency detection using the main clock.
 **
 ** \return Frequency detection counter value
 ******************************************************************************
 */
uint16_t Csv_GetFcsDetectCount(void) 
{
    return FM4_CRG->FCSWD_CTL;        
}

//@}

#endif

/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
