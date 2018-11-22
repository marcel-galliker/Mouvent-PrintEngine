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
/** \file mft_wfg.c
 **
 ** A detailed description is available at 
 ** @link WfgGroup WFG Module description @endlink
 **  
 ** History:
 **   - 2013-03-21  0.1  Frank  First version.
 **
 *****************************************************************************/


/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "mft_wfg.h"


#if (defined(PDL_PERIPHERAL_MFT_WFG_ACTIVE))

/**
 ******************************************************************************
 ** \addtogroup WfgGroup
 ******************************************************************************/
//@{

/******************************************************************************/
/* Local pre-processor symbols/macros ('#define')                             */
/******************************************************************************/
///  Macro to return the number of enabled MFT instances
#define WFG_INSTANCE_COUNT (uint32_t)(sizeof(m_astcMftWfgInstanceDataLut) \
                                      / sizeof(m_astcMftWfgInstanceDataLut[0]))

/******************************************************************************/
/* Global variable definitions (declared in header file with 'extern')        */
/******************************************************************************/
/// Look-up table for all enabled WFG  instances and their internal data
stc_mft_wfg_instance_data_t m_astcMftWfgInstanceDataLut[] =
{
#if (PDL_PERIPHERAL_ENABLE_MFT0_WFG == PDL_ON)
    { 
        &MFT0_WFG,  // pstcInstance
        NULL    // stcInternData (not initialized yet)
    },
#endif
#if (PDL_PERIPHERAL_ENABLE_MFT1_WFG == PDL_ON)
    { 
        &MFT1_WFG,  // pstcInstance
        NULL    // stcInternData (not initialized yet)
    },
#endif
#if (PDL_PERIPHERAL_ENABLE_MFT2_WFG == PDL_ON)
    { 
        &MFT2_WFG,  // pstcInstance
        NULL    // stcInternData (not initialized yet)
    },
#endif
};

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
 *****************************************************************************
 ** \brief Return the internal data for a certain WFG instance.
 **
 ** \param pstcWfg  Pointer to WFG instance
 **
 ** \return Pointer to internal data or NULL if instance is not enabled 
 **         (or not known)
 **
 *****************************************************************************/
static stc_mft_wfg_intern_data_t* MftGetInternDataPtr(volatile stc_mftn_wfg_t* pstcWfg)
{
    uint32_t u32Instance;
   
    for (u32Instance = 0; u32Instance < WFG_INSTANCE_COUNT; u32Instance++)
    {
        if (pstcWfg == m_astcMftWfgInstanceDataLut[u32Instance].pstcInstance)
        {
            return &m_astcMftWfgInstanceDataLut[u32Instance].stcInternData;
        }
    }

    return NULL;
}

#if (PDL_INTERRUPT_ENABLE_MFT0_WFG == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_MFT1_WFG == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_MFT2_WFG == PDL_ON)   
/*!
 ******************************************************************************
 ** \brief  WFG interrupt handler sub function
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 **
 ** \param [in]  pstcMftWfgInternData  structure of WFG callback function
 ** \arg    structure of stc_mft_intern_data
 **
 **
 ** \retval void
 **
 *****************************************************************************
*/
void Mft_Wfg_IrqHandler( volatile  stc_mftn_wfg_t*pstcMft,
		         stc_mft_wfg_intern_data_t* pstcMftWfgInternData)
{
    func_ptr_t funCallBack;
    uint32_t* ptemp;
    uint8_t ch;

    /* timer interrupt*/
    for(ch=0;ch<3;ch++)
    {
        if(PdlSet == Mft_Wfg_GetTimerIntFlag(pstcMft,ch))
        {
            /* Clear Interrupt */
            Mft_Wfg_ClrTimerIntFlag(pstcMft,ch);

            //get peak timer interrupt callback address of each channel
            ptemp = (uint32_t*)&(pstcMftWfgInternData->pfnWfg10TimerCallback);
            funCallBack = (func_ptr_t)(*(uint32_t*)((uint32_t)ptemp + 
                                          (sizeof(func_ptr_t) * ch)));

            if(funCallBack != NULL)
            {
               funCallBack();
            }
        }
    }

    /* DTIF Interrupt */
    //Digital filter interrupt
    if(PdlSet == Mft_Wfg_GetDigitalFilterIntFlag(pstcMft))
    {
        //Mft_Wfg_ClrDigitalFilterIntFlag(pstcMft);
        funCallBack = pstcMftWfgInternData->pfnWfgDigtalFilterCallback;
        if(funCallBack != NULL)
        {
           funCallBack();
        }
    }

    //analog filter interrupt
    if(PdlSet == Mft_Wfg_GetAnalogFilterIntFlag(pstcMft))
    {
        //Mft_Wfg_ClrAnalogFilterIntFlag(pstcMft);
        funCallBack = pstcMftWfgInternData->pfnWfgAnalogFilterCallback;
        if(funCallBack != NULL)
        {
           funCallBack();
        }
    }
}

/**
 ******************************************************************************
 ** \brief Device dependent initialization of interrupts according CMSIS with
 **        level defined in l3.h
 **
 ** \param pstcMft   Pointer to WFG instance
 **
 ** \return Ok    Successful initialization
 **
 ******************************************************************************/
static en_result_t Mft_Wfg_InitIrq( volatile stc_mftn_wfg_t* pstcMft )
{
#if (PDL_INTERRUPT_ENABLE_MFT0_WFG == PDL_ON)
    if (pstcMft == (stc_mftn_wfg_t*)(&MFT0_WFG))
    {
        NVIC_ClearPendingIRQ(WFG0_DTIF0_IRQn);
        NVIC_EnableIRQ(WFG0_DTIF0_IRQn);
        NVIC_SetPriority(WFG0_DTIF0_IRQn, PDL_IRQ_LEVEL_MFT0_WFG);
    }
#endif
#if (PDL_INTERRUPT_ENABLE_MFT1_WFG == PDL_ON)
    if (pstcMft == (stc_mftn_wfg_t*)(&MFT1_WFG))
    {
        NVIC_ClearPendingIRQ(WFG1_DTIF1_IRQn);
        NVIC_EnableIRQ(WFG1_DTIF1_IRQn);
        NVIC_SetPriority(WFG1_DTIF1_IRQn, PDL_IRQ_LEVEL_MFT1_WFG);
    }
#endif
#if (PDL_INTERRUPT_ENABLE_MFT2_WFG == PDL_ON)
    if (pstcMft == (stc_mftn_wfg_t*)(&MFT2_WFG))
    {
        NVIC_ClearPendingIRQ(WFG2_DTIF2_IRQn);
        NVIC_EnableIRQ(WFG2_DTIF2_IRQn);
        NVIC_SetPriority(WFG2_DTIF2_IRQn, PDL_IRQ_LEVEL_MFT2_WFG);
    }
#endif
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Device dependent de-initialization of interrupts according CMSIS
 **
 ** \param pstcMft Icu Pointer to WFG instance
 **
 ** \return Ok    Successful initialization
 **
 ******************************************************************************/
static en_result_t Mft_Wfg_DeInitIrq( volatile stc_mftn_wfg_t* pstcMft )
{
    if((pstcMft->NZCL & 0x7300) == 0x7300) // DIMA=DIMB=WIM10=WIM32=WIM54=1
    {   
    #if (PDL_INTERRUPT_ENABLE_MFT0_WFG == PDL_ON)
        if (pstcMft == (stc_mftn_wfg_t*)(&MFT0_WFG))
        {
            NVIC_ClearPendingIRQ(WFG0_DTIF0_IRQn);
            NVIC_DisableIRQ(WFG0_DTIF0_IRQn);
            NVIC_SetPriority(WFG0_DTIF0_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);
        }
    #endif
    #if (PDL_INTERRUPT_ENABLE_MFT1_WFG == PDL_ON)
        if (pstcMft == (stc_mftn_wfg_t*)(&MFT1_WFG))
        {
            NVIC_ClearPendingIRQ(WFG1_DTIF1_IRQn);
            NVIC_DisableIRQ(WFG1_DTIF1_IRQn);
            NVIC_SetPriority(WFG1_DTIF1_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);
        }
    #endif
    #if (PDL_INTERRUPT_ENABLE_MFT2_WFG == PDL_ON)
        if (pstcMft == (stc_mftn_wfg_t*)(&MFT2_WFG))
        {
            NVIC_ClearPendingIRQ(WFG2_DTIF2_IRQn);
            NVIC_DisableIRQ(WFG2_DTIF2_IRQn);
            NVIC_SetPriority(WFG2_DTIF2_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);
        }
    #endif
    }
    return Ok;
}

#endif

/******************************************************************************/
/* Local Functions                                                            */
/******************************************************************************/

/*!
 ******************************************************************************
 ** \brief  configure WFG mode
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 ** \param [in]  u8CoupleCh  channel of WFG couple
 ** \arg    MFT_WFG_CH10 ~ MFT_WFG_CH54
 ** \param [in] enMode enum of WFG mode
 ** \arg    en_mft_wfg_mode_t
 **
 ** \retval Ok                    Internal data has been setup
 ** \retval ErrorInvalidParameter pstcMft == NULL
 **                               u8CoupleCh out of range
 **                               Other invalid configuration
 **
 *****************************************************************************
*/
en_result_t Mft_Wfg_ConfigMode( volatile stc_mftn_wfg_t*pstcMft,
                               uint8_t u8CoupleCh, en_mft_wfg_mode_t enMode)
{
    stc_mft_wfg_wfsa10_field_t* pstcWFSA;
    stc_mft_wfg_intern_data_t* pstcWfgInternData;
    
    // Check for NULL pointer
    if ( (pstcMft == NULL)
         || (u8CoupleCh >= MFT_WFG_MAXCH))
    {
        return ErrorInvalidParameter ;
    }
    // Get pointer to internal data structure ...
    pstcWfgInternData = MftGetInternDataPtr( pstcMft ) ;
    
    // ... and check for NULL
    if ( pstcWfgInternData != NULL )
    {
        pstcWfgInternData->pfnWfg10TimerCallback = NULL;
        pstcWfgInternData->pfnWfg32TimerCallback = NULL;
        pstcWfgInternData->pfnWfg54TimerCallback = NULL;
    }

    //Get pointer of current channel WFG register address
    pstcWFSA = (stc_mft_wfg_wfsa10_field_t*)((uint8_t*)pstcMft + 0xA4UL + u8CoupleCh*4);

    //configure WFG mode
    pstcWFSA->TMD = (uint16_t)enMode;

    return Ok;
}

/*!
 ******************************************************************************
 ** \brief  configure WFG control bit
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 ** \param [in]  u8CoupleCh  channel of WFG couple
 ** \arg    MFT_WFG_CH10 ~ MFT_WFG_CH54
 ** \param [in] pstcCtrlBits structure of control bit
 ** \arg    stc_wfg_ctrl_bits, GTEN, PSEL,PGEN,DMOD
 **
 ** \retval Ok                    Internal data has been setup
 ** \retval ErrorInvalidParameter pstcMft == NULL
 **                               u8CoupleCh out of range
 **
 *****************************************************************************
*/
en_result_t Mft_Wfg_ConfigCtrlBits( volatile stc_mftn_wfg_t*pstcMft,uint8_t u8CoupleCh,
                                   stc_wfg_ctrl_bits* pstcCtrlBits)
{
    stc_mft_wfg_wfsa10_field_t* pstcWFSA;

    // Check for NULL pointer
    if ( (pstcMft == NULL)
         || (u8CoupleCh >= MFT_WFG_MAXCH))
    {
        return ErrorInvalidParameter ;
    }

    //Get pointer of current channel WFG register address
    pstcWFSA = (stc_mft_wfg_wfsa10_field_t*)((uint8_t*)pstcMft + 0xA4UL + u8CoupleCh*4);

    //set register
    pstcWFSA->DMOD = pstcCtrlBits->enDmodBit;
    pstcWFSA->PGEN0 = (pstcCtrlBits->enPgenBits >> 0) & 0x01;
    pstcWFSA->PGEN1 = (pstcCtrlBits->enPgenBits >> 1) & 0x01;
    pstcWFSA->PSEL0 = (pstcCtrlBits->enPselBits >> 0) & 0x01;
    pstcWFSA->PSEL1 = (pstcCtrlBits->enPselBits >> 1) & 0x01;
    pstcWFSA->GTEN0 = (pstcCtrlBits->enGtenBits >> 0) & 0x01;
    pstcWFSA->GTEN1 = (pstcCtrlBits->enGtenBits >> 1) & 0x01;

    return Ok;
}

/*!
 ******************************************************************************
 ** \brief  init timer clock
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 ** \param [in]  u8CoupleCh  channel of WFG couple
 ** \arg    MFT_WFG_CH10 ~ MFT_WFG_CH54
 ** \param [in] enClk count clock cycle to PCLK multiplied
 ** \arg    en_wfg_timer_clock_t
 **
 ** \retval Ok                    Internal data has been setup
 ** \retval ErrorInvalidParameter pstcMft == NULL
 **                               u8CoupleCh out of range
 **
 *****************************************************************************
*/
en_result_t Mft_Wfg_InitTimerClock( volatile stc_mftn_wfg_t*pstcMft,uint8_t u8CoupleCh,
                                   en_wfg_timer_clock_t enClk)
{
    stc_mft_wfg_wfsa10_field_t* pstcWFSA;

    // Check for NULL pointer
    if ( (pstcMft == NULL)
         || (u8CoupleCh >= MFT_WFG_MAXCH))
    {
        return ErrorInvalidParameter ;
    }

    //Get pointer of current channel WFG register address
    pstcWFSA = (stc_mft_wfg_wfsa10_field_t*)((uint8_t*)pstcMft + 0xA4UL + u8CoupleCh*4);

    //set count clock
    pstcWFSA->DCK0 = (enClk >> 0) & 0x01;
    pstcWFSA->DCK1 = (enClk >> 1) & 0x01;
    pstcWFSA->DCK2 = (enClk >> 2) & 0x01;

    return Ok;
}

#if (PDL_INTERRUPT_ENABLE_MFT0_WFG == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_MFT1_WFG == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_MFT2_WFG == PDL_ON)  
/*!
 ******************************************************************************
 ** \brief  enable WFG timer interrupt
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 ** \param [in]  u8CoupleCh  channel of WFG couple
 ** \arg    MFT_WFG_CH10 ~ MFT_WFG_CH54
 ** \param [in] pfnCallback callback function
 ** \arg    func_ptr_t
 **
 ** \retval Ok                    Internal data has been setup
 ** \retval ErrorInvalidParameter pstcMft == NULL
 **                               u8CoupleCh out of range
 **
 *****************************************************************************
*/
en_result_t Mft_Wfg_EnableTimerInt( volatile stc_mftn_wfg_t*pstcMft,
                                   uint8_t u8CoupleCh, func_ptr_t pfnCallback)
{
    stc_mft_wfg_intern_data_t* pstcWfgInternData;

    // Check for NULL pointer
    if ( (pstcMft == NULL)
         || (u8CoupleCh >= MFT_WFG_MAXCH))
    {
        return ErrorInvalidParameter ;
    }

    // Get pointer to internal data structure ...
    pstcWfgInternData = MftGetInternDataPtr( pstcMft ) ;

    switch(u8CoupleCh)
    {
        case MFT_WFG_CH10:
            pstcWfgInternData->pfnWfg10TimerCallback = pfnCallback;
            pstcMft->NZCL_f.WIM10 = 0;
            break;
        case MFT_WFG_CH32:
            pstcWfgInternData->pfnWfg32TimerCallback = pfnCallback;
            pstcMft->NZCL_f.WIM32 = 0;
            break;
        case MFT_WFG_CH54:
            pstcWfgInternData->pfnWfg54TimerCallback = pfnCallback;
            pstcMft->NZCL_f.WIM54 = 0;
            break;
    }
 
    Mft_Wfg_InitIrq(pstcMft);
 
    return Ok;
}

/*!
 ******************************************************************************
 ** \brief  disable WFG timer interrupt
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 ** \param [in]  u8CoupleCh  channel of WFG couple
 ** \arg    MFT_WFG_CH10 ~ MFT_WFG_CH54
 **
 ** \retval Ok                    Internal data has been setup
 ** \retval ErrorInvalidParameter pstcMft == NULL
 **                               u8CoupleCh out of range
 **
 *****************************************************************************
*/
en_result_t Mft_Wfg_DisableTimerInt( volatile stc_mftn_wfg_t*pstcMft, uint8_t u8CoupleCh)
{
    stc_mft_wfg_intern_data_t* pstcWfgInternData;

    // Check for NULL pointer
    if ( (pstcMft == NULL)
         || (u8CoupleCh >= MFT_WFG_MAXCH))
    {
        return ErrorInvalidParameter ;
    }

    // Get pointer to internal data structure ...
    pstcWfgInternData = MftGetInternDataPtr( pstcMft ) ;

    switch(u8CoupleCh)
    {
        case MFT_WFG_CH10:
            pstcWfgInternData->pfnWfg10TimerCallback = NULL;
            pstcMft->NZCL_f.WIM10 = 1;
            break;
        case MFT_WFG_CH32:
            pstcWfgInternData->pfnWfg32TimerCallback = NULL;
            pstcMft->NZCL_f.WIM32 = 1;
            break;
        case MFT_WFG_CH54:
            pstcWfgInternData->pfnWfg54TimerCallback = NULL;
            pstcMft->NZCL_f.WIM54 = 1;
            break;
    }
  
    Mft_Wfg_DeInitIrq(pstcMft);

    return Ok;
}

#endif   

/*!
 ******************************************************************************
 ** \brief  Start WFG timer
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 ** \param [in]  u8CoupleCh  channel of WFG couple
 ** \arg    MFT_WFG_CH10 ~ MFT_WFG_CH54
 **
 ** \retval Ok                    Start timer successfully
 ** \retval ErrorInvalidParameter pstcMft == NULL
 **                               u8CoupleCh out of range
 **
 *****************************************************************************
*/          
en_result_t Mft_Wfg_StartTimer(volatile stc_mftn_wfg_t*pstcMft, uint8_t u8CoupleCh)
{
    if ( (pstcMft == NULL)
         || (u8CoupleCh >= MFT_WFG_MAXCH))
    {
        return ErrorInvalidParameter ;
    }
    
    switch(u8CoupleCh)
    {
        case MFT_WFG_CH10:
            pstcMft->WFIR_f.TMIE10 = 1;
            break;
        case MFT_WFG_CH32:
            pstcMft->WFIR_f.TMIE32 = 1;
            break;
        case MFT_WFG_CH54:
            pstcMft->WFIR_f.TMIE54 = 1;
            break;
        default:
            break;
    }
    
    return Ok;
}

/*!
 ******************************************************************************
 ** \brief  Stop WFG timer
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 ** \param [in]  u8CoupleCh  channel of WFG couple
 ** \arg    MFT_WFG_CH10 ~ MFT_WFG_CH54
 **
 ** \retval Ok                    Start timer successfully
 ** \retval ErrorInvalidParameter pstcMft == NULL
 **                               u8CoupleCh out of range
 **
 *****************************************************************************
*/  
en_result_t Mft_Wfg_StopTimer(volatile stc_mftn_wfg_t*pstcMft, uint8_t u8CoupleCh)
{
    if ( (pstcMft == NULL)
         || (u8CoupleCh >= MFT_WFG_MAXCH))
    {
        return ErrorInvalidParameter ;
    }
    
    switch(u8CoupleCh)
    {
        case MFT_WFG_CH10:
            pstcMft->WFIR_f.TMIS10 = 1;
            break;
        case MFT_WFG_CH32:
            pstcMft->WFIR_f.TMIS32 = 1;
            break;
        case MFT_WFG_CH54:
            pstcMft->WFIR_f.TMIS54 = 1;
            break;
        default:
            break;
    }
    
    return Ok;
}


/*!
 ******************************************************************************
 ** \brief  get WFG timer interrupt flag
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 ** \param [in]  u8CoupleCh  channel of WFG couple
 ** \arg    MFT_WFG_CH10 ~ MFT_WFG_CH54
 **
 ** \retval en_int_flag_t            interrupt falg
 ** \retval PdlSet,PdlClr
 **
 *****************************************************************************
*/
en_int_flag_t Mft_Wfg_GetTimerIntFlag( volatile stc_mftn_wfg_t*pstcMft,uint8_t u8CoupleCh)
{
    en_int_flag_t retval;

    switch(u8CoupleCh)
    {
    case MFT_WFG_CH10:
        retval = (en_int_flag_t)pstcMft->WFIR_f.TMIF10;
        break;
    case MFT_WFG_CH32:
        retval = (en_int_flag_t)pstcMft->WFIR_f.TMIF32;
        break;
    case MFT_WFG_CH54:
        retval = (en_int_flag_t)pstcMft->WFIR_f.TMIF54;
        break;
    }

    return retval;
}

/*!
 ******************************************************************************
 ** \brief  clear WFG timer interrupt flag
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 ** \param [in]  u8CoupleCh  channel of WFG couple
 ** \arg    MFT_WFG_CH10 ~ MFT_WFG_CH54
 **
 ** \retval Ok                    Internal data has been setup
 ** \retval ErrorInvalidParameter pstcMft == NULL
 **                               u8CoupleCh out of range
 **
 *****************************************************************************
*/
en_result_t Mft_Wfg_ClrTimerIntFlag( volatile stc_mftn_wfg_t*pstcMft, uint8_t u8CoupleCh)
{
    // Check for NULL pointer
    if ( (pstcMft == NULL)
         || (u8CoupleCh >= MFT_WFG_MAXCH))
    {
        return ErrorInvalidParameter ;
    }

    //set register value
    switch(u8CoupleCh)
    {
    case MFT_WFG_CH10:
        pstcMft->WFIR_f.TMIC10 = 1;
        break;
    case MFT_WFG_CH32:
        pstcMft->WFIR_f.TMIC32 = 1;
        break;
    case MFT_WFG_CH54:
        pstcMft->WFIR_f.TMIC54 = 1;
        break;
    }

    return Ok;
}

/*!
 ******************************************************************************
 ** \brief  write timer count cycle
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 ** \param [in]  u8CoupleCh  channel of WFG couple
 ** \arg    MFT_WFG_CH10 ~ MFT_WFG_CH54
 ** \param [in] u16CycleA WFTA value
 ** \arg    0~65535
 ** \param [in]  u16CycleB  WFTB value
 ** \arg    0~65535
 **
 ** \retval Ok                    Internal data has been setup
 ** \retval ErrorInvalidParameter pstcMft == NULL
 **                               u8CoupleCh out of range
 **
 *****************************************************************************
*/
en_result_t Mft_Wfg_WriteTimerCountCycle( volatile stc_mftn_wfg_t*pstcMft,
                                         uint8_t u8CoupleCh,
                                         uint16_t u16CycleA,
                                         uint16_t u16CycleB)
{
    uint16_t* pWFTA;
    uint16_t* pWFTB;

    // Check for NULL pointer
    if ( (pstcMft == NULL)
         || (u8CoupleCh >= MFT_WFG_MAXCH))
    {
        return ErrorInvalidParameter ;
    }

    //Get pointer of current channel WFG register address
    pWFTA = (uint16_t*)((uint8_t*)pstcMft + 0x90UL + u8CoupleCh*8);
    pWFTB = (uint16_t*)((uint8_t*)pstcMft + 0x92UL + u8CoupleCh*8);

    //set the register
    *pWFTA = u16CycleA;
    *pWFTB = u16CycleB;

    return Ok;

}

/*!
 ******************************************************************************
 ** \brief  set wfg pulse counter
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 ** \param [in]  u8CoupleCh  channel of WFG couple
 ** \arg    MFT_WFG_CH10 ~ MFT_WFG_CH54
 ** \param [in]  u16Count  wfg pulse counter value
 ** \arg    0~65535
 **
 ** \retval Ok                    Internal data has been setup
 ** \retval ErrorInvalidParameter pstcMft == NULL
 **                               u8CoupleCh out of range
 **
 *****************************************************************************
*/
en_result_t Mft_Wfg_SetTimerCycle( volatile stc_mftn_wfg_t*pstcMft,
                                      uint8_t u8CoupleCh, uint16_t u16Count)
{
    uint16_t* pWFTF;

    //Get pointer of current channel WFG register address
    pWFTF = (uint16_t*)((uint8_t*)pstcMft + 0x8EUL + u8CoupleCh*8);

    *pWFTF =u16Count;
    return Ok;
}
/*!
 ******************************************************************************
 ** \brief  get current pulse counter
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 ** \param [in]  u8CoupleCh  channel of WFG couple
 ** \arg    MFT_WFG_CH10 ~ MFT_WFG_CH54
 **
 ** \retval uint16_t   value of pulse counter
 ** \retval 0~65535
 **
 *****************************************************************************
*/
uint16_t Mft_Wfg_GetTimerCurCycle( volatile stc_mftn_wfg_t*pstcMft, uint8_t u8CoupleCh)
{
    uint16_t retval;
    uint16_t* pWFTF;

    //Get pointer of current channel WFG register address
    pWFTF = (uint16_t*)((uint8_t*)pstcMft + 0x8EUL + u8CoupleCh*8);

    retval = *pWFTF;
    return retval;
}

/*!
 ******************************************************************************
 ** \brief  configure NZCL
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 ** \param [in]  pstcNzclConfig  configure of NZCL
 ** \arg    structure of stc_wfg_nzcl_config_t
 **
 ** \retval Ok                    Internal data has been setup
 ** \retval ErrorInvalidParameter pstcMft == NULL
 **                               u8CoupleCh out of range
 **
 *****************************************************************************
*/
en_result_t Mft_Wfg_ConfigNzcl( volatile stc_mftn_wfg_t*pstcMft,
                               stc_wfg_nzcl_config_t* pstcNzclConfig)
{
    // Check for NULL pointer
    if ( (pstcMft == NULL))
    {
        return ErrorInvalidParameter ;
    }

    //set register value
    pstcMft->NZCL_f.DTIEA = pstcNzclConfig->bEnDigitalFilter;
    pstcMft->NZCL_f.DTIEB = pstcNzclConfig->bEnAnalogFilter;
    pstcMft->NZCL &= ~(7ul<<1);
    pstcMft->NZCL |= (pstcNzclConfig->enDigitalFilterWidth <<1 );
    if(pstcMft == &MFT0_WFG)
    {
        bFM4_GPIO_EPFR01_DTTI0C = pstcNzclConfig->bSwitchToGpio;
    }
    else if (pstcMft == &MFT1_WFG)
    {
        bFM4_GPIO_EPFR02_DTTI1C = pstcNzclConfig->bSwitchToGpio;
    }
    else if(pstcMft == &MFT2_WFG)
    {
        bFM4_GPIO_EPFR03_DTTI2C = pstcNzclConfig->bSwitchToGpio;
    }
    return Ok;
}

#if (PDL_INTERRUPT_ENABLE_MFT0_WFG == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_MFT1_WFG == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_MFT2_WFG == PDL_ON) 
/*!
 ******************************************************************************
 ** \brief  Enable DTIF interrupt
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 ** \param [in] pstcIntSel  DTIF interrupt selection
 ** \arg    structure of DTIF interrupt selection
 ** \param [in] pstcCallback  Pointer to DTIF interrupt callback function array
 **
 ** \retval Ok                    Interrupt enable normally
 ** \retval ErrorInvalidParameter pstcMft == NULL
 **                               pstcIntSel == NULL
 **                               pstcCallback == NULL                        
 **
 ******************************************************************************/
en_result_t Mft_Wfg_EnableDtifInt( volatile stc_mftn_wfg_t*pstcMft, stc_dtif_int_sel_t* pstcIntSel, 
                                  stc_dtif_int_cb_t* pstcCallback)
{
    stc_mft_wfg_intern_data_t* pstcWfgInternData;
    // Check for NULL pointer
    if ((pstcMft == NULL ) || 
        (pstcIntSel->bDtifAnalogFilterInt > 1) ||
        (pstcIntSel->bDtifDigitalFilterInt > 1)) 
    {
        return ErrorInvalidParameter ;
    }
    
    // Get pointer to internal data structure ...
    pstcWfgInternData = MftGetInternDataPtr( pstcMft ) ;
    
    if(TRUE == pstcIntSel->bDtifDigitalFilterInt)
    {
        pstcMft->NZCL_f.DIMA = 0;
        pstcWfgInternData->pfnWfgDigtalFilterCallback = pstcCallback->pfnWfgDigtalFilterCallback;

    }
    if(TRUE == pstcIntSel->bDtifAnalogFilterInt)
    {
        pstcMft->NZCL_f.DIMB = 0;
        pstcWfgInternData->pfnWfgAnalogFilterCallback = pstcCallback->pfnWfgAnalogFilterCallback;
    }
    
    Mft_Wfg_InitIrq(pstcMft);
    
    return Ok;
}

/*!
 ******************************************************************************
 ** \brief  Disable DTIF interrupt
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 ** \param [in] pstcIntSel  DTIF interrupt selection
 ** \arg    structure of DTIF interrupt selection
 **
 ** \retval Ok                    Interrupt disable normally
 ** \retval ErrorInvalidParameter pstcMft == NULL
 **                               pstcIntSel == NULL                  
 **
 ******************************************************************************/
en_result_t Mft_Wfg_DisableDtifInt( volatile stc_mftn_wfg_t*pstcMft, stc_dtif_int_sel_t* pstcIntSel)
{
    stc_mft_wfg_intern_data_t* pstcWfgInternData;
    // Check for NULL pointer
    if ( (pstcMft == NULL) || 
         (pstcIntSel->bDtifAnalogFilterInt > 1) ||
         (pstcIntSel->bDtifDigitalFilterInt > 1)  
       )
    {
        return ErrorInvalidParameter ;
    }
    
    // Get pointer to internal data structure ...
    pstcWfgInternData = MftGetInternDataPtr( pstcMft ) ;
    
    if(TRUE == pstcIntSel->bDtifDigitalFilterInt)
    {
        pstcMft->NZCL_f.DIMA = 1;
        pstcWfgInternData->pfnWfgDigtalFilterCallback = NULL;

    }
    if(TRUE == pstcIntSel->bDtifAnalogFilterInt)
    {
        pstcMft->NZCL_f.DIMB = 1;
        pstcWfgInternData->pfnWfgAnalogFilterCallback = NULL;
    }
    
    Mft_Wfg_DeInitIrq(pstcMft);
    
    return Ok;
}

#endif

/*!
 ******************************************************************************
 ** \brief  set software trigger DTIF
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 **
 ** \retval Ok                    Internal data has been setup
 ** \retval ErrorInvalidParameter pstcMft == NULL
 **                               u8CoupleCh out of range
 **
 *****************************************************************************
*/
en_result_t Mft_Wfg_SwTiggerDtif( volatile stc_mftn_wfg_t*pstcMft)
{
    // Check for NULL pointer
    if ( (pstcMft == NULL))
    {
        return ErrorInvalidParameter ;
    }

    pstcMft->NZCL_f.SDTI = 1;
    return Ok;
}

/*!
 ******************************************************************************
 ** \brief  get Digital filter interrupt flag
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 **
 ** \retval en_int_flag_t            interrupt falg
 ** \retval PdlSet,PdlClr
 **
 *****************************************************************************
*/
en_int_flag_t Mft_Wfg_GetDigitalFilterIntFlag( volatile stc_mftn_wfg_t*pstcMft)
{
    en_int_flag_t retval;

    //get interrupt flag via digital noise-canceler
    retval = (en_int_flag_t)pstcMft->WFIR_f.DTIFA;
    return retval;
}

/*!
 ******************************************************************************
 ** \brief  clear Digital filter interrupt flag
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 **
 ** \retval Ok                    Internal data has been setup
 ** \retval ErrorInvalidParameter pstcMft == NULL
 **                               u8CoupleCh out of range
 **
 *****************************************************************************
*/
en_result_t Mft_Wfg_ClrDigitalFilterIntFlag( volatile stc_mftn_wfg_t*pstcMft)
{
    // Check for NULL pointer
    if ( (pstcMft == NULL))
    {
        return ErrorInvalidParameter ;
    }

    pstcMft->WFIR_f.DTICA = 1;
    return Ok;
}

/*!
 ******************************************************************************
 ** \brief  get Analgo filter interrupt flag
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 **
 ** \retval en_int_flag_t            interrupt falg
 ** \retval PdlSet,PdlClr
 **
 *****************************************************************************
*/
en_int_flag_t Mft_Wfg_GetAnalogFilterIntFlag( volatile stc_mftn_wfg_t*pstcMft)
{
    en_int_flag_t retval;

    //get interrupt flag via analog noise filter
    retval =(en_int_flag_t) pstcMft->WFIR_f.DTIFB;
    return retval;
}
/*!
 ******************************************************************************
 ** \brief  clear analog filter interrupt flag
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 **
 ** \retval Ok                    Internal data has been setup
 ** \retval ErrorInvalidParameter pstcMft == NULL
 **                               u8CoupleCh out of range
 **
 *****************************************************************************
*/
en_result_t Mft_Wfg_ClrAnalogFilterIntFlag( volatile stc_mftn_wfg_t*pstcMft)
{
    // Check for NULL pointer
    if ( (pstcMft == NULL))
    {
        return ErrorInvalidParameter ;
    }

    pstcMft->WFIR_f.DTICB = 1;
    return Ok;
}

//@}

#endif // #if (defined(PDL_PERIPHERAL_$$X_ACTIVE))

/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
