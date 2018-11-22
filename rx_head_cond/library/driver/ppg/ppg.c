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
/** \file ppg.c
 **
 ** A detailed description is available at 
 ** @link PpgGroup PPG Module description @endlink
 **  
 ** History:
 **   - 2013-05-15  0.1  Edison Zhang  First version.
 **
 *****************************************************************************/

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "ppg.h"

#if (defined(PDL_PERIPHERAL_PPG_ACTIVE))

/**
 ******************************************************************************
 ** \addtogroup PpgGroup
 ******************************************************************************/
//@{

#if (PDL_INTERRUPT_ENABLE_PPG00_02_04 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_PPG08_10_12 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_PPG16_18_20 == PDL_ON)   
/******************************************************************************/
/* Static variable definitions                                                */
/******************************************************************************/
static func_ptr_t  pfnPpgCallback[PPG_INT_MAX_CH];      ///< Callback function pointer of PPG interrupt 

/**
 ******************************************************************************
 ** \brief Set NVIC Interrupt depending on PPG channel
 **
 ** \param u8Ch PPG interrupt channel
 **
 ******************************************************************************/
static void Ppg_InitNvic(uint8_t u8Ch)
{
    if ((u8Ch == PPG_CH0) || (u8Ch == PPG_CH2) || (u8Ch == PPG_CH4))
    {
        NVIC_ClearPendingIRQ(PPG00_02_04_IRQn);
        NVIC_EnableIRQ(PPG00_02_04_IRQn);
        NVIC_SetPriority(PPG00_02_04_IRQn, PDL_IRQ_LEVEL_PPG00_02_04);   
    }

    if ((u8Ch == PPG_CH8) || (u8Ch == PPG_CH10) || (u8Ch == PPG_CH12))
    {
        NVIC_ClearPendingIRQ(PPG08_10_12_IRQn);
        NVIC_EnableIRQ(PPG08_10_12_IRQn);
        NVIC_SetPriority(PPG08_10_12_IRQn, PDL_IRQ_LEVEL_PPG08_10_12);   
    }
    
    if ((u8Ch == PPG_CH16) || (u8Ch == PPG_CH18) || (u8Ch == PPG_CH20))
    {
        NVIC_ClearPendingIRQ(PPG16_18_20_IRQn);
        NVIC_EnableIRQ(PPG16_18_20_IRQn);
        NVIC_SetPriority(PPG16_18_20_IRQn, PDL_IRQ_LEVEL_PPG08_10_12);   
    }
}

/**
 ******************************************************************************
 ** \brief Clear NVIC Interrupt depending on PPG channel
 **
 ** \param u8Ch PPG interrupt channel
 **
 ******************************************************************************/
static void Ppg_DeInitNvic(uint8_t u8Ch)
{
    if ((u8Ch == PPG_CH0) || (u8Ch == PPG_CH2) || (u8Ch == PPG_CH4))
    {
        if((bFM3_MFT_PPG_PPGC0_PIE == 0) && (bFM3_MFT_PPG_PPGC2_PIE == 0) && (bFM3_MFT_PPG_PPGC4_PIE == 0))
        {
            NVIC_ClearPendingIRQ(PPG00_02_04_IRQn);
            NVIC_DisableIRQ(PPG00_02_04_IRQn);
            NVIC_SetPriority(PPG00_02_04_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);   
        }
    }
    
    if ((u8Ch == PPG_CH8) || (u8Ch == PPG_CH10) || (u8Ch == PPG_CH12))
    {
        if((bFM3_MFT_PPG_PPGC8_PIE == 0) && (bFM3_MFT_PPG_PPGC10_PIE == 0) && (bFM3_MFT_PPG_PPGC12_PIE == 0))
        {
            NVIC_ClearPendingIRQ(PPG08_10_12_IRQn);
            NVIC_DisableIRQ(PPG08_10_12_IRQn);
            NVIC_SetPriority(PPG08_10_12_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);   
        }
    }
    
    if ((u8Ch == PPG_CH16) || (u8Ch == PPG_CH18) || (u8Ch == PPG_CH20))
    {
        if((bFM3_MFT_PPG_PPGC16_PIE == 0) && (bFM3_MFT_PPG_PPGC18_PIE == 0) && (bFM3_MFT_PPG_PPGC20_PIE == 0))
        {
            NVIC_ClearPendingIRQ(PPG16_18_20_IRQn);
            NVIC_DisableIRQ(PPG16_18_20_IRQn);
            NVIC_SetPriority(PPG16_18_20_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);   
        }
    }
}

/**
 ******************************************************************************
 ** \brief PPG interrupt service routine
 **
 ** \param u8Ch  Irq channel of PPG 
 **        
 ******************************************************************************/      
void Ppg_IrqHandler(en_ppg_irq_ch_t u8Ch) 
{
    if((u8Ch != PpgCh024) && (u8Ch != PpgCh81012) && (u8Ch != PpgCh161820))
    {
        return;
    }
     
    switch (u8Ch)
    {
        case PpgCh024:
            if(bFM3_MFT_PPG_PPGC0_PUF == 1) // IRQ triggered by ch.0?
            {
                bFM3_MFT_PPG_PPGC0_PUF = 0;
                bFM3_MFT_PPG_PPGC1_PUF = 0;
                pfnPpgCallback[0]();
            }
            if(bFM3_MFT_PPG_PPGC2_PUF == 1) // IRQ triggered by ch.2? 
            {
                bFM3_MFT_PPG_PPGC2_PUF = 0;
                bFM3_MFT_PPG_PPGC3_PUF = 0;
                pfnPpgCallback[1]();
            }
            if(bFM3_MFT_PPG_PPGC4_PUF == 1) // IRQ triggered by ch.4? 
            {
                bFM3_MFT_PPG_PPGC4_PUF = 0;
                bFM3_MFT_PPG_PPGC5_PUF = 0;
                pfnPpgCallback[2]();
            }
            break;
        case PpgCh81012:
            if(bFM3_MFT_PPG_PPGC8_PUF == 1) // IRQ triggered by ch.8?
            {
                bFM3_MFT_PPG_PPGC8_PUF = 0;
                bFM3_MFT_PPG_PPGC9_PUF = 0;
                pfnPpgCallback[3]();
            }
            if(bFM3_MFT_PPG_PPGC10_PUF == 1) // IRQ triggered by ch.10? 
            {
                bFM3_MFT_PPG_PPGC10_PUF = 0;
                bFM3_MFT_PPG_PPGC11_PUF = 0;
                pfnPpgCallback[4]();
            }
            if(bFM3_MFT_PPG_PPGC12_PUF == 1) // IRQ triggered by ch.12? 
            {
                bFM3_MFT_PPG_PPGC12_PUF = 0;
                bFM3_MFT_PPG_PPGC13_PUF = 0;
                pfnPpgCallback[5]();
            }
            break;
        case PpgCh161820:
            if(bFM3_MFT_PPG_PPGC16_PUF == 1) // IRQ triggered by ch.16?
            {
                bFM3_MFT_PPG_PPGC16_PUF = 0;
                bFM3_MFT_PPG_PPGC17_PUF = 0;
                pfnPpgCallback[6]();
            }
            if(bFM3_MFT_PPG_PPGC18_PUF == 1) // IRQ triggered by ch.18? 
            {
                bFM3_MFT_PPG_PPGC18_PUF = 0;
                bFM3_MFT_PPG_PPGC19_PUF = 0;
                pfnPpgCallback[7]();
            }
            if(bFM3_MFT_PPG_PPGC20_PUF == 1) // IRQ triggered by ch.20? 
            {
                bFM3_MFT_PPG_PPGC20_PUF = 0;
                bFM3_MFT_PPG_PPGC21_PUF = 0;
                pfnPpgCallback[8]();
            }
            break;
        default:
            break;
    }
}   

#endif

/**
 ******************************************************************************
 ** \brief  Initialize PPG
 **
 ** \param   [in]  u8CoupleCh         a couple PPG channel
 ** \arg           PPG_COUPLE_CH01 ~  PPG_COUPLE_CH2223
 ** \param   [in]  pstcConfig         pointer to PPG configuration structure
 **
 ** \retval  Ok                       Configure the PPG successfully
 ** \retval  ErrorInvalidParameter    u8CoupleCh%2 !=0
 **                                   u8CoupleCh > PPG_COUPLE_CH2223
 **                                   Parameter error of PPG configuration structure
 **                                     
 ******************************************************************************/
en_result_t Ppg_Init( uint8_t u8CoupleCh, 
                      stc_ppg_config_t *pstcConfig)
{
    volatile uint8_t *pu8Ppgc0Addr, *pu8Ppgc1Addr;
    volatile uint16_t *pu8Revc0, *pu8Revc1;
    volatile uint8_t *pu8Gatec;
    uint8_t u8Offset, u8Gap;
    if( ((u8CoupleCh%2) != 0)            ||
        (u8CoupleCh > PPG_COUPLE_CH2223) ||
        (pstcConfig == NULL)             ||  
        (pstcConfig->enEvenClock > PpgPclkDiv64) || 
        (pstcConfig->enEvenLevel > PpgReverseLevel) ||
        (pstcConfig->enOddClock > PpgPclkDiv64) ||
        (pstcConfig->enOddLevel > PpgReverseLevel) ||
        (pstcConfig->enMode > Ppg16Bit16Pres)  )
    {
        return ErrorInvalidParameter ;
    }
    u8Offset = u8CoupleCh%4;
    u8Gap    = u8CoupleCh/4;
    pu8Ppgc1Addr = (uint8_t*)FM4_MFT_PPG_BASE + 0x200 + u8Gap*0x40 + u8Offset*2;
    pu8Ppgc0Addr = (uint8_t*)FM4_MFT_PPG_BASE + 0x200 + u8Gap*0x40 + u8Offset*2 + 1;  
    pu8Revc0     = (uint16_t*)((uint8_t*)FM4_MFT_PPG_BASE + 0x104);
    pu8Revc1     = (uint16_t*)((uint8_t*)FM4_MFT_PPG_BASE + 0x144); 
    pu8Gatec     = (uint8_t*)FM4_MFT_PPG_BASE + 0x218 + u8Gap*0x40;
    // set mode
    *pu8Ppgc0Addr &= ~(3ul<<1);
    *pu8Ppgc0Addr |= (pstcConfig->enMode<<1);
    // set even channel clock, level
    *pu8Ppgc0Addr &= ~(3ul<<3);
    *pu8Ppgc0Addr |= (pstcConfig->enEvenClock<<3);
    if(u8CoupleCh < PPG_CH16)
    {
        *pu8Revc0 &= ~(1ul<<u8CoupleCh);
        *pu8Revc0 |= (pstcConfig->enEvenLevel<<u8CoupleCh);
    }
    else
    {
        *pu8Revc1 &= ~(1ul<<(u8CoupleCh-PPG_CH16));
        *pu8Revc1 |= (pstcConfig->enEvenLevel<<(u8CoupleCh-PPG_CH16));
    }
    // set odd channel clock, level
    *pu8Ppgc1Addr &= ~(3ul<<3);
    *pu8Ppgc1Addr |= (pstcConfig->enOddClock<<3);
    if(u8CoupleCh < PPG_CH16)
    {
        *pu8Revc0 &= ~(1ul<<u8CoupleCh);
        *pu8Revc0 |= (pstcConfig->enOddLevel<<(u8CoupleCh+1));
    }
    else
    {
        *pu8Revc1 &= ~(1ul<<(u8CoupleCh+1-PPG_CH16));
        *pu8Revc1 |= (pstcConfig->enOddLevel<<(u8CoupleCh+1-PPG_CH16));
    }
    // set PPG mode
    switch (pstcConfig->enTrig)
    {
        case PpgSoftwareTrig:
            *pu8Ppgc0Addr &= ~1ul;
            if((u8CoupleCh%4) == 0)
            {
                *pu8Gatec &= ~(1ul<<1);
            }
            else
            {
                *pu8Gatec &= ~(1ul<<5);
            }
            break;  
        case PpgMftGateTrig:
            *pu8Ppgc0Addr &= ~1ul;
            if((u8CoupleCh%4) == 0)
            {
                *pu8Gatec |= (1ul<<1);
            }
            else
            {
                *pu8Gatec |= (1ul<<5);
            }
            break; 
        case PpgTimingGenTrig:
            *pu8Ppgc0Addr |= 1ul;
            break; 
        default:
            break;
    }
    
    return Ok;
}

/**
 ******************************************************************************
 ** \brief  Start PPG by software trigger
 **
 ** \param   [in]  u8Ch               PPG channel
 ** \arg           PPG_CH0~PPG_CH23
 **
 ** \retval  Ok                       Configure the PPG successfully
 ** \retval  ErrorInvalidParameter    u8Ch > PPG_CH23
 **                                     
 ******************************************************************************/
en_result_t Ppg_StartSoftwareTrig(uint8_t u8Ch)
{
    if((u8Ch > PPG_CH23))
    {
        return ErrorInvalidParameter ;
    }
    
    if(u8Ch < PPG_CH16)
    {
        FM4_MFT_PPG->TRG0 |= (1ul<<u8Ch);
    }
    else
    {
        FM4_MFT_PPG->TRG1 |= (1ul<<(u8Ch-PPG_CH16));
    }
    
    return Ok;
}

/**
 ******************************************************************************
 ** \brief   Stop PPG software trigger
 **
 ** \param   [in]  u8Ch               PPG channel
 ** \arg           PPG_CH0~PPG_CH23
 **
 ** \retval  Ok                       Configure the PPG successfully
 ** \retval  ErrorInvalidParameter    u8Ch > PPG_CH23
 **                                     
 ******************************************************************************/
en_result_t Ppg_StopSoftwareTrig(uint8_t u8Ch)
{
    if((u8Ch > PPG_CH23))
    {
        return ErrorInvalidParameter ;
    }
    
    if(u8Ch < PPG_CH16)
    {
        FM4_MFT_PPG->TRG0 &= ~(1ul<<u8Ch);
    }
    else
    {
        FM4_MFT_PPG->TRG1 &= ~(1ul<<(u8Ch-PPG_CH16));
    }
    
    return Ok;
}

/**
 ******************************************************************************
 ** \brief   Set the valid level of GATE signal
 **
 ** \param   [in]  u8EvenCh           an even channel of PPG
 ** \arg           PPG_CH0~PPG_CH22
 ** \param   [in]  enLevel            Gate level
 **
 ** \retval  Ok                       Set the valid level of GATE signal successfully
 ** \retval  ErrorInvalidParameter    u8Ch > PPG_CH22
 **                                   u8EvenCh%2 != 0
 **                                   enLevel > PpgGateLowActive
 **                                     
 ******************************************************************************/
en_result_t Ppg_SelGateLevel(uint8_t u8EvenCh, en_ppg_gate_level_t enLevel)
{
    volatile uint8_t *pu8Gatec;
    if( ((u8EvenCh%2) != 0)        ||
        (u8EvenCh > PPG_CH22)      ||
        (enLevel > PpgGateLowActive)  )
    {
        return ErrorInvalidParameter ;
    }
    
    pu8Gatec     = (uint8_t*)FM4_MFT_PPG_BASE + 0x218 + 0x40*(u8EvenCh/4);
    
    if((u8EvenCh%4) == 0)
    {
        *pu8Gatec &= ~(1ul);
        *pu8Gatec |= enLevel;
    }
    else
    {
        *pu8Gatec &= ~(1ul<<4);
        *pu8Gatec |= (enLevel<<4);
    }
    
    return Ok;
}

/**
 ******************************************************************************
 ** \brief   Configure the up counter of Timing Generator 0
 **
 ** \param   [in]  pstcConfig         Pointer to up counter 0 configuration structure
 **
 ** \retval  Ok                       Configure the up counter successfully
 ** \retval  ErrorInvalidParameter    pstcConfig != NULL
 **                                   pstcConfig->enClk > PpgUpCntPclkDiv64 
 **                                     
 ******************************************************************************/
en_result_t  Ppg_ConfigUpCnt0(stc_ppg_upcnt0_config_t* pstcConfig)
{
    if((pstcConfig != NULL) || 
       (pstcConfig->enClk > PpgUpCntPclkDiv64 ))
    {
        return ErrorInvalidParameter ;
    }
    
    FM4_MFT_PPG->TTCR0 &= ~(3ul<<2);
    FM4_MFT_PPG->TTCR0 |= (pstcConfig->enClk<<2);
    FM4_MFT_PPG->COMP0 = pstcConfig->u8CmpValue0;
    FM4_MFT_PPG->COMP2 = pstcConfig->u8CmpValue2;
    FM4_MFT_PPG->COMP4 = pstcConfig->u8CmpValue4;
    FM4_MFT_PPG->COMP6 = pstcConfig->u8CmpValue6;
    return Ok;
}

/**
 ******************************************************************************
 ** \brief   Start the up counter of Timing Generator 0
 **                                     
 ******************************************************************************/
void Ppg_StartUpCnt0(void)
{
    bFM3_MFT_PPG_TTCR0_STR0 = 1;
}

/**
 ******************************************************************************
 ** \brief   Get the work status of up counter of Timing Generator 0
 **
 ** \retval  PdlSet                   Up counter is counting
 ** \retval  PdlClr                   Up counter stops
 **                                     
 ******************************************************************************/
en_stat_flag_t  Ppg_GetUpCnt0Status(void)
{
    return (en_stat_flag_t) bFM3_MFT_PPG_TTCR0_MONI0;
}

/**
 ******************************************************************************
 ** \brief   Disable start trigger of Timing Generator 0
 **
 ** \param   pstcTimer0GenCh          Pointer to the structure of selected channels 
 **                                   (ch.0,2,4,6)
 **
 ** \retval  Ok                       Disable start trigger
 ** \retval  ErrorInvalidParameter    pstcTimer0GenCh->bPpgChx isn't bool type
 **                                     
 ******************************************************************************/
en_result_t  Ppg_DisableTimerGen0StartTrig(stc_timer0_gen_ch_t* pstcTimer0GenCh)
{
    if((pstcTimer0GenCh->bPpgCh0 > 1) || 
       (pstcTimer0GenCh->bPpgCh2 > 1) ||
       (pstcTimer0GenCh->bPpgCh4 > 1) || 
       (pstcTimer0GenCh->bPpgCh6 > 1)  )
    {
        return ErrorInvalidParameter;
    }
    
    bFM3_MFT_PPG_TTCR0_TRG0O = ((pstcTimer0GenCh->bPpgCh0 == 1) ? 1 : 0);
    bFM3_MFT_PPG_TTCR0_TRG2O = ((pstcTimer0GenCh->bPpgCh2 == 1) ? 1 : 0);
    bFM3_MFT_PPG_TTCR0_TRG4O = ((pstcTimer0GenCh->bPpgCh4 == 1) ? 1 : 0);
    bFM3_MFT_PPG_TTCR0_TRG6O = ((pstcTimer0GenCh->bPpgCh6 == 1) ? 1 : 0);
    return Ok;
}

/**
 ******************************************************************************
 ** \brief   Configure the up counter of Timing Generator 1
 **
 ** \param   [in]  pstcConfig         Pointer to up counter 1 configuration structure
 **
 ** \retval  Ok                       Configure the up counter successfully
 ** \retval  ErrorInvalidParameter    pstcConfig != NULL
 **                                   pstcConfig->enClk > PpgUpCntPclkDiv64 
 **                                     
 ******************************************************************************/
en_result_t  Ppg_ConfigUpCnt1(stc_ppg_upcnt1_config_t* pstcConfig)
{
    if(pstcConfig->enClk > PpgUpCntPclkDiv64 )
    {
        return ErrorInvalidParameter ;
    }
    
    FM4_MFT_PPG->TTCR1 &= ~(3ul<<2);
    FM4_MFT_PPG->TTCR1 |= (pstcConfig->enClk<<2);
    FM4_MFT_PPG->COMP1 = pstcConfig->u8CmpValue8;
    FM4_MFT_PPG->COMP3 = pstcConfig->u8CmpValue10;
    FM4_MFT_PPG->COMP5 = pstcConfig->u8CmpValue12;
    FM4_MFT_PPG->COMP7 = pstcConfig->u8CmpValue14;
    return Ok;
}

/**
 ******************************************************************************
 ** \brief   Start the up counter of Timing Generator 1
 **                                     
 ******************************************************************************/
void  Ppg_StartUpCnt1(void)
{
    bFM3_MFT_PPG_TTCR1_STR1 = 1;
}

/**
 ******************************************************************************
 ** \brief   Get the work status of up counter of Timing Generator 1
 **
 ** \retval  PdlSet                   Up counter is counting
 ** \retval  PdlClr                   Up counter stops
 **                                     
 ******************************************************************************/
en_stat_flag_t  Ppg_GetUpCnt1Status(void)
{
    return (en_stat_flag_t) bFM3_MFT_PPG_TTCR1_MONI1;
}

/**
 ******************************************************************************
 ** \brief   Disable start trigger of Timing Generator 1
 **
 ** \param   pstcTimer1GenCh          Pointer to the structure of selected channels 
 **                                   (ch.8,10,12,14)
 **
 ** \retval  Ok                       Disable start trigger
 ** \retval  ErrorInvalidParameter    pstcTimer0GenCh->bPpgChx isn't bool type
 **                                     
 ******************************************************************************/
en_result_t  Ppg_DisableTimerGen1StartTrig(stc_timer1_gen_ch_t* pstcTimer1GenCh)
{
    if((pstcTimer1GenCh->bPpgCh8 > 1)  || 
       (pstcTimer1GenCh->bPpgCh10 > 1) ||
       (pstcTimer1GenCh->bPpgCh12 > 1) || 
       (pstcTimer1GenCh->bPpgCh14 > 1)  )
    {
        return ErrorInvalidParameter;
    }
    
    bFM3_MFT_PPG_TTCR1_TRG1O = ((pstcTimer1GenCh->bPpgCh8 == 1) ? 1 : 0);
    bFM3_MFT_PPG_TTCR1_TRG3O = ((pstcTimer1GenCh->bPpgCh10 == 1) ? 1 : 0);
    bFM3_MFT_PPG_TTCR1_TRG5O = ((pstcTimer1GenCh->bPpgCh12 == 1) ? 1 : 0);
    bFM3_MFT_PPG_TTCR1_TRG7O = ((pstcTimer1GenCh->bPpgCh14 == 1) ? 1 : 0);
    return Ok;
}

/**
 ******************************************************************************
 ** \brief   Configure the up counter of Timing Generator 2
 **
 ** \param   [in]  pstcConfig         Pointer to up counter 2 configuration structure
 **
 ** \retval  Ok                       Configure the up counter successfully
 ** \retval  ErrorInvalidParameter    pstcConfig != NULL
 **                                   pstcConfig->enClk > PpgUpCntPclkDiv64 
 **                                     
 ******************************************************************************/
en_result_t  Ppg_ConfigUpCnt2(stc_ppg_upcnt2_config_t* pstcConfig)
{
    if(pstcConfig->enClk > PpgUpCntPclkDiv64 )
    {
        return ErrorInvalidParameter ;
    }
    
    FM4_MFT_PPG->TTCR1 &= ~(3ul<<2);
    FM4_MFT_PPG->TTCR1 |= (pstcConfig->enClk<<2);
    FM4_MFT_PPG->COMP8 = pstcConfig->u8CmpValue16;
    FM4_MFT_PPG->COMP10 = pstcConfig->u8CmpValue18;
    FM4_MFT_PPG->COMP12 = pstcConfig->u8CmpValue20;
    FM4_MFT_PPG->COMP14 = pstcConfig->u8CmpValue22;
    return Ok;
}

/**
 ******************************************************************************
 ** \brief   Start the up counter of Timing Generator 2
 **                                     
 ******************************************************************************/
void  Ppg_StartUpCnt2(void)
{
    bFM3_MFT_PPG_TTCR2_STR2 = 1;
}

/**
 ******************************************************************************
 ** \brief   Get the work status of up counter of Timing Generator 2
 **
 ** \retval  PdlSet                   Up counter is counting
 ** \retval  PdlClr                   Up counter stops
 **                                     
 ******************************************************************************/
en_stat_flag_t  Ppg_GetUpCnt2Status(void)
{
    return (en_stat_flag_t) bFM3_MFT_PPG_TTCR2_MONI2;
}

/**
 ******************************************************************************
 ** \brief   Disable start trigger of Timing Generator 2
 **
 ** \param   pstcTimer2GenCh          Pointer to the structure of selected channels 
 **                                   (ch.16,18,20,22)
 **
 ** \retval  Ok                       Disable start trigger
 ** \retval  ErrorInvalidParameter    pstcTimer0GenCh->bPpgChx isn't bool type
 **                                     
 ******************************************************************************/
en_result_t  Ppg_DisableTimerGen2StartTrig(stc_timer2_gen_ch_t* pstcTimer2GenCh)
{
    if((pstcTimer2GenCh->bPpgCh16 > 1)  || 
       (pstcTimer2GenCh->bPpgCh18 > 1) ||
       (pstcTimer2GenCh->bPpgCh20 > 1) || 
       (pstcTimer2GenCh->bPpgCh22 > 1)  )
    {
        return ErrorInvalidParameter;
    }
    
    bFM3_MFT_PPG_TTCR2_TRG16O = ((pstcTimer2GenCh->bPpgCh16 == 1) ? 1 : 0);
    bFM3_MFT_PPG_TTCR2_TRG18O = ((pstcTimer2GenCh->bPpgCh18 == 1) ? 1 : 0);
    bFM3_MFT_PPG_TTCR2_TRG20O = ((pstcTimer2GenCh->bPpgCh20 == 1) ? 1 : 0);
    bFM3_MFT_PPG_TTCR2_TRG22O = ((pstcTimer2GenCh->bPpgCh22 == 1) ? 1 : 0);
    return Ok;
}

#if (PDL_INTERRUPT_ENABLE_PPG00_02_04 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_PPG08_10_12 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_PPG16_18_20 == PDL_ON)
/**
 ******************************************************************************
 ** \brief   Enable PPG interrupt
 **
 ** \param   [in] u8Ch                PPG channnel
 ** \arg     PPG_CHx, x=0,2,4,8,10,12,16,18,20
 ** \param   [in] enIntMode           Interrupt mode
 ** \arg     PpgHighUnderflow         Generate interrupt when high width count underflows
 ** \arg     PpgHighAndLowUnderflow   Generate interrupt when high or low width count underflows
 ** \param   [in] pfnCallback         Pointer to interrupt callback function
 **
 ** \retval  Ok                       Enable PPG interrupt successfully
 ** \retval  ErrorInvalidParameter    u8Ch != ch.0,2,4,8,10,12,16,18,20
 **                                   enIntMode > PpgHighAndLowUnderflow
 **                                   pfnCallback == NULL
 **                                     
 ******************************************************************************/
en_result_t  Ppg_EnableInt(uint8_t u8Ch, en_ppg_int_mode_t enIntMode, func_ptr_t pfnCallback)
{
    volatile uint8_t *pu8Ppgc0Addr;
    uint8_t u8Offset, u8Gap;
    // Only Ch.0,2,4,8,10,12,16,18,20 can produce interrupt request
    if((u8Ch != PPG_CH0) &&
       (u8Ch != PPG_CH2) &&
       (u8Ch != PPG_CH4) &&
       (u8Ch != PPG_CH8) && 
       (u8Ch != PPG_CH10) &&
       (u8Ch != PPG_CH12) &&
       (u8Ch != PPG_CH16) &&
       (u8Ch != PPG_CH18) && 
       (u8Ch != PPG_CH20)  )
    {
        return ErrorInvalidParameter;
    }
    
    // Check other parameter
    if((enIntMode > PpgHighAndLowUnderflow) ||
       (pfnCallback == NULL))
    {
         return ErrorInvalidParameter;
    }
    
    u8Offset = u8Ch%4;
    u8Gap    = u8Ch/4;
    pu8Ppgc0Addr = (uint8_t*)FM4_MFT_PPG_BASE + 0x200 + u8Gap*0x40 + u8Offset*2 + 1;  
    
    if((u8Ch == PPG_CH0) || (u8Ch == PPG_CH2) || (u8Ch == PPG_CH4))
    {
        pfnPpgCallback[u8Ch/2] = pfnCallback;
    }
    else if((u8Ch == PPG_CH8) || (u8Ch == PPG_CH10) || (u8Ch == PPG_CH12))
    {
        pfnPpgCallback[(u8Ch-2)/2] = pfnCallback;
    }
    else if((u8Ch == PPG_CH16) || (u8Ch == PPG_CH18) || (u8Ch == PPG_CH20))
    {
        pfnPpgCallback[(u8Ch-4)/2] = pfnCallback;
    }
    *pu8Ppgc0Addr |= (1ul<<7);
    
    Ppg_InitNvic(u8Ch);
    
    return Ok;
}

/**
 ******************************************************************************
 ** \brief   Disable PPG interrupt
 **
 ** \param   [in] u8Ch                PPG channnel
 ** \arg     PPG_CHx, x=0,2,4,8,10,12,16,18,20
 **
 ** \retval  Ok                       Disable PPG interrupt successfully
 ** \retval  ErrorInvalidParameter    u8Ch != ch.0,2,4,8,10,12,16,18,20
 **                                     
 ******************************************************************************/
en_result_t  Ppg_DisableInt(uint8_t u8Ch)
{
    volatile uint8_t *pu8Ppgc0Addr;
    uint8_t u8Offset, u8Gap;
    // Only Ch.0,2,4,8,10,12,16,18,20 can produce interrupt request
    if((u8Ch != PPG_CH0) &&
       (u8Ch != PPG_CH2) &&
       (u8Ch != PPG_CH4) &&
       (u8Ch != PPG_CH8) && 
       (u8Ch != PPG_CH10) &&
       (u8Ch != PPG_CH12) &&
       (u8Ch != PPG_CH16) &&
       (u8Ch != PPG_CH18) && 
       (u8Ch != PPG_CH20)  )
    {
        return ErrorInvalidParameter;
    }
    
    u8Offset = u8Ch%4;
    u8Gap    = u8Ch/4;
    pu8Ppgc0Addr = (uint8_t*)FM4_MFT_PPG_BASE + 0x200 + u8Gap*0x40 + u8Offset*2 + 1;  
    
    if((u8Ch == PPG_CH0) || (u8Ch == PPG_CH2) || (u8Ch == PPG_CH4))
    {
        pfnPpgCallback[u8Ch/2] = NULL;
    }
    else if((u8Ch == PPG_CH8) || (u8Ch == PPG_CH10) || (u8Ch == PPG_CH12))
    {
        pfnPpgCallback[(u8Ch-2)/2] = NULL;
    }
    else if((u8Ch == PPG_CH16) || (u8Ch == PPG_CH18) || (u8Ch == PPG_CH20))
    {
        pfnPpgCallback[(u8Ch-4)/2] = NULL;
    }
    *pu8Ppgc0Addr &= ~(1ul<<7);
    
    Ppg_DeInitNvic(u8Ch);
    
    return Ok;
}

#endif

/**
 ******************************************************************************
 ** \brief   Get the interrupt flag of PPG
 **
 ** \param   [in] u8Ch                PPG channnel
 ** \arg     PPG_CHx, x=0,2,4,8,10,12,16,18,20
 **
 ** \retval  PdlSet                   Interrupt flag set
 ** \retval  PdlClr                   Interrupt flag clear
 **                                     
 ******************************************************************************/
en_int_flag_t  Ppg_GetIntFlag(uint8_t u8Ch)
{
    volatile uint8_t *pu8Ppgc0Addr;
    uint8_t u8Offset, u8Gap;
    en_int_flag_t enFlag;
    // Only Ch.0,2,4,8,10,12,16,18,20 can produce interrupt request
    if((u8Ch != PPG_CH0) &&
       (u8Ch != PPG_CH2) &&
       (u8Ch != PPG_CH4) &&
       (u8Ch != PPG_CH8) && 
       (u8Ch != PPG_CH10) &&
       (u8Ch != PPG_CH12) &&
       (u8Ch != PPG_CH16) &&
       (u8Ch != PPG_CH18) && 
       (u8Ch != PPG_CH20)  )
    {
        return PdlClr;
    }
    
    u8Offset = u8Ch%4;
    u8Gap    = u8Ch/4;
    pu8Ppgc0Addr = (uint8_t*)FM4_MFT_PPG_BASE + 0x200 + u8Gap*0x40 + u8Offset*2 + 1;  
    
    if((*pu8Ppgc0Addr & 0x40) == 0x40)
    {
        enFlag = PdlSet;
    }
    else
    {
        enFlag = PdlClr;
    }
    
    return enFlag;
}

/**
 ******************************************************************************
 ** \brief   Clear PPG interrupt flag
 **
 ** \param   [in] u8Ch                PPG channnel
 ** \arg     PPG_CHx, x=0,2,4,8,10,12,16,18,20
 **
 ** \retval  Ok                       Clear PPG interrupt flag successfully
 ** \retval  ErrorInvalidParameter    u8Ch != ch.0,2,4,8,10,12,16,18,20
 **                                     
 ******************************************************************************/
en_result_t  Ppg_ClrIntFlag(uint8_t u8Ch)
{
    volatile uint8_t *pu8Ppgc0Addr, *pu8Ppgc1Addr;
    uint8_t u8Offset, u8Gap;
    // Only Ch.0,2,4,8,10,12,16,18,20 can produce interrupt request
    if((u8Ch != PPG_CH0) &&
       (u8Ch != PPG_CH2) &&
       (u8Ch != PPG_CH4) &&
       (u8Ch != PPG_CH8) && 
       (u8Ch != PPG_CH10) &&
       (u8Ch != PPG_CH12) &&
       (u8Ch != PPG_CH16) &&
       (u8Ch != PPG_CH18) && 
       (u8Ch != PPG_CH20)  )
    {
        return ErrorInvalidParameter;
    }
    
    u8Offset = u8Ch%4;
    u8Gap    = u8Ch/4;
    pu8Ppgc0Addr = (uint8_t*)FM4_MFT_PPG_BASE + 0x200 + u8Gap*0x40 + u8Offset*2 + 1; 
    pu8Ppgc1Addr = (uint8_t*)FM4_MFT_PPG_BASE + 0x200 + u8Gap*0x40 + u8Offset*2 ; 
        
    // Clear interrupt flag of both even and odd channel 
    *pu8Ppgc0Addr &= ~(1ul<<6);
    *pu8Ppgc1Addr &= ~(1ul<<6);
    
    return Ok;
}

/**
 ******************************************************************************
 ** \brief   Set the pulse width of PPG
 **
 ** \param   [in] u8Ch                PPG channnel
 ** \arg     PPG_CH0~PPG_CH23
 ** \param   [in] u8LowWidth          Low level width of PPG
 ** \param   [in] u8HighWidth         High level width of PPG
 **
 ** \retval  Ok                       Set the pulse width of PPG successfully
 ** \retval  ErrorInvalidParameter    u8Ch > PPG_CH23
 **                                     
 ******************************************************************************/
en_result_t  Ppg_SetLevelWidth(uint8_t u8Ch, uint8_t u8LowWidth, uint8_t u8HighWidth)
{
    volatile uint8_t *pu8Prll0Addr, *pu8Prlh0Addr;
    uint8_t u8Offset, u8Gap;
    
    if(u8Ch > PPG_CH23)
    {
        return ErrorInvalidParameter;
    }
    
    u8Offset = u8Ch%4;
    u8Gap    = u8Ch/4;
    pu8Prll0Addr = (uint8_t*)FM4_MFT_PPG_BASE + 0x208 + u8Gap*0x40 + u8Offset*4;
    pu8Prlh0Addr = (uint8_t*)FM4_MFT_PPG_BASE + 0x208 + u8Gap*0x40 + u8Offset*4 + 1;
    *pu8Prll0Addr = u8LowWidth;
    *pu8Prlh0Addr = u8HighWidth;
    return Ok;
}

/**
 ******************************************************************************
 ** \brief  Initialize IGBT mode
 **
 ** \param   [in]  pstcPpgIgbt        pointer to IBGT configuration structure

 ** \retval  ErrorInvalidParameter    pstcPpgIgbt == NULL
 **                                   Other error configuration
 **                                     
 ******************************************************************************/
en_result_t  Ppg_InitIgbt(stc_ppg_igbt_config_t* pstcPpgIgbt)
{
    if((pstcPpgIgbt == NULL) ||
       (pstcPpgIgbt->enMode > IgbtStopProhibitionMode) ||
       (pstcPpgIgbt->enWidth > IgbtFilter32Pclk) ||
       (pstcPpgIgbt->enTrigInputLevel  > IgbtLevelInvert)  ||
       (pstcPpgIgbt->enIgbt0OutputLevel > IgbtLevelInvert) ||
       (pstcPpgIgbt->enIgbt1OutputLevel > IgbtLevelInvert)   )
    {
        return ErrorInvalidParameter;
    }
    // Prohibition mode setting  
    FM4_MFT_PPG->IGBTC &= ~(1ul<<7); 
    FM4_MFT_PPG->IGBTC |= (pstcPpgIgbt->enMode<<7);
    // Filter width setting
    FM4_MFT_PPG->IGBTC &= ~(7ul<<4);
    FM4_MFT_PPG->IGBTC |= (pstcPpgIgbt->enWidth<<4);
    // Trigger input level setting
    FM4_MFT_PPG->IGBTC &= ~(1ul<<1); 
    FM4_MFT_PPG->IGBTC |= (pstcPpgIgbt->enTrigInputLevel<<1); 
    // IGBT output level setting
    FM4_MFT_PPG->IGBTC &= ~(1ul<<2); 
    FM4_MFT_PPG->IGBTC |= (pstcPpgIgbt->enIgbt0OutputLevel<<2); 
    FM4_MFT_PPG->IGBTC &= ~(1ul<<3); 
    FM4_MFT_PPG->IGBTC |= (pstcPpgIgbt->enIgbt0OutputLevel<<3); 
    
    return Ok;
    
}

/**
 ******************************************************************************
 ** \brief   Enable IGBT mode 
 ** \note    Only PPG0 and PP4 supports IGBT mode. 
 **
 ******************************************************************************/
void Ppg_EnableIgbtMode(void)
{
    FM4_MFT_PPG->IGBTC |= 1ul;
}

/**
 ******************************************************************************
 ** \brief   Disable IGBT mode 
 ** \note    PPG0 and PPG4 outputs normal PPG wave.
 **
 ******************************************************************************/
void Ppg_DisableIgbtMode(void)
{
    FM4_MFT_PPG->IGBTC &= ~1ul;
}

//@}
                      
#endif

/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
