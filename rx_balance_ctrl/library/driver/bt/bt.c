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
/** \file bt.c
 **
 ** A detailed description is available at 
 ** @link BtGroup BT Module description @endlink
 **
 ** History:
 **   - 2013-05-09  1.0  Edison Zhang  First version.
 **
 ******************************************************************************/

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "bt.h"

#if (defined(PDL_PERIPHERAL_BT_ACTIVE))

/**
 ******************************************************************************
 ** \addtogroup BtGroup
 ******************************************************************************/
//@{

/******************************************************************************/
/* Local pre-processor symbols/macros ('#define')                             */
/******************************************************************************/
#define BT_INSTANCE_COUNT (uint32_t)(sizeof(m_astcBtInstanceDataLut) / sizeof(m_astcBtInstanceDataLut[0]))

/******************************************************************************/
/* Global variable definitions (declared in header file with 'extern')        */
/******************************************************************************/
/// Look-up table for all enabled BT instances and their internal data
stc_bt_instance_data_t m_astcBtInstanceDataLut[] =
{
#if (PDL_PERIPHERAL_ENABLE_BT0 == PDL_ON)
    { 
        &BT0,   // pstcInstance
        NULL    // stcInternData (not initialized yet)
    },
#endif
#if (PDL_PERIPHERAL_ENABLE_BT1 == PDL_ON)
    { 
        &BT1,   // pstcInstance
        NULL    // stcInternData (not initialized yet)
    },
#endif
#if (PDL_PERIPHERAL_ENABLE_BT2 == PDL_ON)
    { 
        &BT2,   // pstcInstance
        NULL    // stcInternData (not initialized yet)
    },
#endif
#if (PDL_PERIPHERAL_ENABLE_BT3 == PDL_ON)
    { 
        &BT3,   // pstcInstance
        NULL    // stcInternData (not initialized yet)
    },
#endif
#if (PDL_PERIPHERAL_ENABLE_BT4 == PDL_ON)
    { 
        &BT4,   // pstcInstance
        NULL    // stcInternData (not initialized yet)
    },
#endif
#if (PDL_PERIPHERAL_ENABLE_BT5 == PDL_ON)
    { 
        &BT5,   // pstcInstance
        NULL    // stcInternData (not initialized yet)
    },
#endif
#if (PDL_PERIPHERAL_ENABLE_BT6 == PDL_ON)
    { 
        &BT6,   // pstcInstance
        NULL    // stcInternData (not initialized yet)
    },
#endif
#if (PDL_PERIPHERAL_ENABLE_BT7 == PDL_ON)
    { 
        &BT7,   // pstcInstance
        NULL    // stcInternData (not initialized yet)
    },
#endif
#if (PDL_PERIPHERAL_ENABLE_BT8 == PDL_ON)
    { 
        &BT8,   // pstcInstance
        NULL    // stcInternData (not initialized yet)
    },
#endif
#if (PDL_PERIPHERAL_ENABLE_BT9 == PDL_ON)
    { 
        &BT9,   // pstcInstance
        NULL    // stcInternData (not initialized yet)
    },
#endif
#if (PDL_PERIPHERAL_ENABLE_BT10 == PDL_ON)
    { 
        &BT10,  // pstcInstance
        NULL    // stcInternData (not initialized yet)
    },
#endif
#if (PDL_PERIPHERAL_ENABLE_BT11 == PDL_ON)
    { 
        &BT11,  // pstcInstance
        NULL    // stcInternData (not initialized yet)
    },
#endif
#if (PDL_PERIPHERAL_ENABLE_BT12 == PDL_ON)
    { 
        &BT12,  // pstcInstance
        NULL    // stcInternData (not initialized yet)
    },
#endif
#if (PDL_PERIPHERAL_ENABLE_BT13 == PDL_ON)
    { 
        &BT13,  // pstcInstance
        NULL    // stcInternData (not initialized yet)
    },
#endif
#if (PDL_PERIPHERAL_ENABLE_BT14 == PDL_ON)
    { 
        &BT14,  // pstcInstance
        NULL    // stcInternData (not initialized yet)
    },
#endif
#if (PDL_PERIPHERAL_ENABLE_BT15 == PDL_ON)
    { 
        &BT15,  // pstcInstance
        NULL    // stcInternData (not initialized yet)
    },
#endif
};

/**
 ******************************************************************************
 ** \brief Return the internal data for a certain BT instance.
 **
 ** \param [in] pstcBt   Pointer to BT instance
 **
 ** \return   Pointer to internal data or NULL if instance is not enabled (or not known)
 **
 ******************************************************************************/
static stc_bt_intern_data_t* BtGetInternDataPtr(volatile stc_btn_t* pstcBt) 
{
    uint32_t u32Instance;
   
    for (u32Instance = 0; u32Instance < BT_INSTANCE_COUNT; u32Instance++)
    {
        if (pstcBt == m_astcBtInstanceDataLut[u32Instance].pstcInstance)
        {
            return &m_astcBtInstanceDataLut[u32Instance].stcInternData;
        }
    }

    return NULL;
}

#if (PDL_INTERRUPT_ENABLE_BT0 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT1 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT2 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT3 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT4 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT5 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT6 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT7 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT8 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT9 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT10 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT11 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT12 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT13 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT14 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT15 == PDL_ON)  
/**
 ******************************************************************************
 ** \brief BT interrupt service routine
 **
 ** \param [in] pstcBt               BT instance
 ** \param [in] pstcBtInternData     Pointer to BT internal data      
 **        
 ******************************************************************************/      
void Bt_IrqHandler( volatile stc_btn_t* pstcBt,
                    stc_bt_intern_data_t* pstcBtInternData) 
{
    uint8_t u8Irq;
    
    // Get Interrupt Request
    u8Irq = pstcBt->STC; 
     /*
      * Trigger Interrupt (PWM,PPG,Reload timer)
      * Measurement completion Interrupt (PWC)
      */
    if ((u8Irq & (1ul<<2)) == (1ul<<2)) 
    {
        //  Clear Interrupt  
        pstcBt->STC &= ~(1ul<<2); 

        if (pstcBtInternData->pfnBit2IntCallback != NULL) 
        {
            // Call CallBackIrq 
            pstcBtInternData->pfnBit2IntCallback();
        }
    }
    
    // Duty Match Interrupt (PWM)   
    if ((u8Irq & (1ul<<1)) == (1ul<<1)) 
    {
        // Clear Interrupt 
        pstcBt->STC &= ~(1ul<<1); 

        if (pstcBtInternData->pfnBit1IntCallback != NULL) 
        {
            // Call CallBackIrq
            pstcBtInternData->pfnBit1IntCallback();
        }
    }
  
    /* 
     * Underflow Interrupt (PWM,PPG,Reload timer)
     * Overflow Interrupt (PWC) 
     */
    if ((u8Irq & 1ul) == 1ul) 
    {
        // Clear Interrupt 
        pstcBt->STC &= ~1ul;

        if (pstcBtInternData->pfnBit0IntCallback != NULL) 
        {
            // Call CallBackIrq
            pstcBtInternData->pfnBit0IntCallback();
        }
    }   

}      
  
/**
 ******************************************************************************
 ** \brief Set NVIC Interrupt depending on BT instance
 **
 ** \param [in] pstcBt    Pointer to BT instance
 **
 ******************************************************************************/
static void Bt_InitNvic(volatile stc_btn_t* pstcBt)
{
#if (PDL_INTERRUPT_ENABLE_BT0 == PDL_ON)  
    if (pstcBt == (stc_btn_t*)(&BT0)) 
    {
        NVIC_ClearPendingIRQ(BT0_IRQn); 
        NVIC_EnableIRQ(BT0_IRQn); 
        NVIC_SetPriority(BT0_IRQn, PDL_IRQ_LEVEL_BT0);
    }
#endif
#if (PDL_INTERRUPT_ENABLE_BT1 == PDL_ON)     
    if (pstcBt == (stc_btn_t*)(&BT1)) 
    {
        NVIC_ClearPendingIRQ(BT1_IRQn); 
        NVIC_EnableIRQ(BT1_IRQn); 
        NVIC_SetPriority(BT1_IRQn, PDL_IRQ_LEVEL_BT1);
    }
#endif    
#if (PDL_INTERRUPT_ENABLE_BT2 == PDL_ON)       
    if (pstcBt == (stc_btn_t*)(&BT2)) 
    {
        NVIC_ClearPendingIRQ(BT2_IRQn); 
        NVIC_EnableIRQ(BT2_IRQn); 
        NVIC_SetPriority(BT2_IRQn, PDL_IRQ_LEVEL_BT2);
    }
#endif
#if (PDL_INTERRUPT_ENABLE_BT3 == PDL_ON)    
    if (pstcBt == (stc_btn_t*)(&BT3)) 
    {
        NVIC_ClearPendingIRQ(BT3_IRQn); 
        NVIC_EnableIRQ(BT3_IRQn); 
        NVIC_SetPriority(BT3_IRQn, PDL_IRQ_LEVEL_BT3);
    }
#endif
#if (PDL_INTERRUPT_ENABLE_BT4 == PDL_ON)        
    if (pstcBt == (stc_btn_t*)(&BT4)) 
    {
        NVIC_ClearPendingIRQ(BT4_IRQn); 
        NVIC_EnableIRQ(BT4_IRQn); 
        NVIC_SetPriority(BT4_IRQn, PDL_IRQ_LEVEL_BT4);
    }
#endif 
#if (PDL_INTERRUPT_ENABLE_BT5 == PDL_ON)     
    if (pstcBt == (stc_btn_t*)(&BT5)) 
    {
        NVIC_ClearPendingIRQ(BT5_IRQn); 
        NVIC_EnableIRQ(BT5_IRQn); 
        NVIC_SetPriority(BT5_IRQn, PDL_IRQ_LEVEL_BT5);
    }
#endif 
#if (PDL_INTERRUPT_ENABLE_BT6 == PDL_ON)     
    if (pstcBt == (stc_btn_t*)(&BT6)) 
    {
        NVIC_ClearPendingIRQ(BT6_IRQn); 
        NVIC_EnableIRQ(BT6_IRQn); 
        NVIC_SetPriority(BT6_IRQn, PDL_IRQ_LEVEL_BT6);
    }
#endif
#if (PDL_INTERRUPT_ENABLE_BT7 == PDL_ON)       
    if (pstcBt == (stc_btn_t*)(&BT7)) 
    {
        NVIC_ClearPendingIRQ(BT7_IRQn); 
        NVIC_EnableIRQ(BT7_IRQn); 
        NVIC_SetPriority(BT7_IRQn, PDL_IRQ_LEVEL_BT7);
    }
#endif
#if (PDL_INTERRUPT_ENABLE_BT8 == PDL_ON)       
    if (pstcBt == (stc_btn_t*)(&BT8)) 
    {
        NVIC_ClearPendingIRQ(BT8_IRQn); 
        NVIC_EnableIRQ(BT8_IRQn); 
        NVIC_SetPriority(BT8_IRQn, PDL_IRQ_LEVEL_BT8);
    }
#endif 
#if (PDL_INTERRUPT_ENABLE_BT9 == PDL_ON)     
    if (pstcBt == (stc_btn_t*)(&BT9)) 
    {
        NVIC_ClearPendingIRQ(BT9_IRQn); 
        NVIC_EnableIRQ(BT9_IRQn); 
        NVIC_SetPriority(BT9_IRQn, PDL_IRQ_LEVEL_BT9);
    }
#endif
#if (PDL_INTERRUPT_ENABLE_BT10 == PDL_ON)      
    if (pstcBt == (stc_btn_t*)(&BT10)) 
    {
        NVIC_ClearPendingIRQ(BT10_IRQn); 
        NVIC_EnableIRQ(BT10_IRQn); 
        NVIC_SetPriority(BT10_IRQn, PDL_IRQ_LEVEL_BT10);
    }
#endif
#if (PDL_INTERRUPT_ENABLE_BT11 == PDL_ON)     
    if (pstcBt == (stc_btn_t*)(&BT11)) 
    {
        NVIC_ClearPendingIRQ(BT11_IRQn); 
        NVIC_EnableIRQ(BT11_IRQn); 
        NVIC_SetPriority(BT11_IRQn, PDL_IRQ_LEVEL_BT11);
    }
#endif
#if (PDL_INTERRUPT_ENABLE_BT12_13_14_15 == PDL_ON)    
    if ((pstcBt == (stc_btn_t*)(&BT12)) || (pstcBt == (stc_btn_t*)(&BT13)) ||
        (pstcBt == (stc_btn_t*)(&BT14)) || (pstcBt == (stc_btn_t*)(&BT15)))
    {
        NVIC_ClearPendingIRQ(BT12_13_14_15_IRQn); 
        NVIC_EnableIRQ(BT12_13_14_15_IRQn); 
        NVIC_SetPriority(BT12_13_14_15_IRQn, PDL_IRQ_LEVEL_BT12_13_14_15);
    }
#endif
} 

/**
 ******************************************************************************
 ** \brief Clear NVIC Interrupt depending on BT instance
 **
 ** \param [in] pstcBt   Pointer to BT instance
 **
 ******************************************************************************/
static void Bt_DeInitNvic(volatile stc_btn_t* pstcBt)
{
#if (PDL_INTERRUPT_ENABLE_BT0 == PDL_ON)    
    if (pstcBt == (stc_btn_t*)(&BT0))
    {
        NVIC_ClearPendingIRQ(BT0_IRQn);
        NVIC_DisableIRQ(BT0_IRQn);
        NVIC_SetPriority(BT0_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);   
    }
#endif
#if (PDL_INTERRUPT_ENABLE_BT1 == PDL_ON)    
    if (pstcBt == (stc_btn_t*)(&BT1))
    {
        NVIC_ClearPendingIRQ(BT1_IRQn);
        NVIC_DisableIRQ(BT1_IRQn);
        NVIC_SetPriority(BT1_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);   
    }
#endif  
#if (PDL_INTERRUPT_ENABLE_BT2 == PDL_ON)    
    if (pstcBt == (stc_btn_t*)(&BT2))
    {
        NVIC_ClearPendingIRQ(BT2_IRQn);
        NVIC_DisableIRQ(BT2_IRQn);
        NVIC_SetPriority(BT2_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);   
    }
#endif      
#if (PDL_INTERRUPT_ENABLE_BT3 == PDL_ON)    
    if (pstcBt == (stc_btn_t*)(&BT3))
    {
        NVIC_ClearPendingIRQ(BT3_IRQn);
        NVIC_DisableIRQ(BT3_IRQn);
        NVIC_SetPriority(BT3_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);   
    }
#endif      
#if (PDL_INTERRUPT_ENABLE_BT4 == PDL_ON)    
    if (pstcBt == (stc_btn_t*)(&BT4))
    {
        NVIC_ClearPendingIRQ(BT4_IRQn);
        NVIC_DisableIRQ(BT4_IRQn);
        NVIC_SetPriority(BT4_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);   
    }
#endif  
#if (PDL_INTERRUPT_ENABLE_BT5 == PDL_ON)    
    if (pstcBt == (stc_btn_t*)(&BT5))
    {
        NVIC_ClearPendingIRQ(BT5_IRQn);
        NVIC_DisableIRQ(BT5_IRQn);
        NVIC_SetPriority(BT5_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);   
    }
#endif  
#if (PDL_INTERRUPT_ENABLE_BT6 == PDL_ON)    
    if (pstcBt == (stc_btn_t*)(&BT6))
    {
        NVIC_ClearPendingIRQ(BT6_IRQn);
        NVIC_DisableIRQ(BT6_IRQn);
        NVIC_SetPriority(BT6_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);   
    }
#endif  
#if (PDL_INTERRUPT_ENABLE_BT7 == PDL_ON)    
    if (pstcBt == (stc_btn_t*)(&BT7))
    {
        NVIC_ClearPendingIRQ(BT7_IRQn);
        NVIC_DisableIRQ(BT7_IRQn);
        NVIC_SetPriority(BT7_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);   
    }
#endif  
#if (PDL_INTERRUPT_ENABLE_BT8 == PDL_ON)    
    if (pstcBt == (stc_btn_t*)(&BT8))
    {
        NVIC_ClearPendingIRQ(BT8_IRQn);
        NVIC_DisableIRQ(BT8_IRQn);
        NVIC_SetPriority(BT8_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);   
    }
#endif  
#if (PDL_INTERRUPT_ENABLE_BT9 == PDL_ON)    
    if (pstcBt == (stc_btn_t*)(&BT9))
    {
        NVIC_ClearPendingIRQ(BT9_IRQn);
        NVIC_DisableIRQ(BT9_IRQn);
        NVIC_SetPriority(BT9_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);   
    }
#endif  
#if (PDL_INTERRUPT_ENABLE_BT10 == PDL_ON)    
    if (pstcBt == (stc_btn_t*)(&BT10))
    {
        NVIC_ClearPendingIRQ(BT10_IRQn);
        NVIC_DisableIRQ(BT10_IRQn);
        NVIC_SetPriority(BT10_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);   
    }
#endif  
#if (PDL_INTERRUPT_ENABLE_BT11 == PDL_ON)    
    if (pstcBt == (stc_btn_t*)(&BT11))
    {
        NVIC_ClearPendingIRQ(BT11_IRQn);
        NVIC_DisableIRQ(BT11_IRQn);
        NVIC_SetPriority(BT11_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);   
    }
#endif  
#if (PDL_INTERRUPT_ENABLE_BT12_13_14_15 == PDL_ON)    
    if ((pstcBt == (stc_btn_t*)(&BT12)) || (pstcBt == (stc_btn_t*)(&BT13)) ||
        (pstcBt == (stc_btn_t*)(&BT14)) || (pstcBt == (stc_btn_t*)(&BT15)))
    {
        /* only when interrupts of ch12,13,14,15 are all disabled, disable NVIC */ 
        if(((FM4_BT12_PWM->STC & 0x70u) == 0x00u) && 
           ((FM4_BT13_PWM->STC & 0x70u) == 0x00u) &&
           ((FM4_BT14_PWM->STC & 0x70u) == 0x00u) &&  
           ((FM4_BT15_PWM->STC & 0x70u) == 0x00u) ) 
        {
            NVIC_ClearPendingIRQ(BT12_13_14_15_IRQn);
            NVIC_DisableIRQ(BT12_13_14_15_IRQn);
            NVIC_SetPriority(BT12_13_14_15_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);   
        }
        
    }
#endif
   
} 

#endif 

/**
 ******************************************************************************
 ** \brief Configure BT IO mode
 **
 ** \param [in]  pstcBt           BT instance
 ** \param [in]  enIoMode         BT IO mode
 **
 ** \retval Ok                    BT IO mode has been set successfully
 ** \retval ErrorInvalidParameter pstcBt == NULL
 **                               Other invalid configuration
 ******************************************************************************/
en_result_t Bt_ConfigIOMode(volatile stc_btn_t* pstcBt, en_bt_io_mode_t enIoMode)
{
    // Pointer to internal data
    stc_bt_intern_data_t* pstcBtInternData ;
    // Check for NULL pointer
    if ((pstcBt == NULL) || (enIoMode > BtIoMode8))
    {
        return ErrorInvalidParameter ;
    }  
    
    // Get pointer to internal data structure ...
    pstcBtInternData = BtGetInternDataPtr( pstcBt ) ;
    
    if(pstcBtInternData == NULL)
    {
        return ErrorInvalidParameter ;
    }
    
    if((pstcBt == &BT0) || (pstcBt == &BT1))
    {
        FM4_BTIOSEL03->BTSEL0123 &= 0xF0;
        FM4_BTIOSEL03->BTSEL0123 |= enIoMode;
    }
    else if((pstcBt == &BT2) || (pstcBt == &BT3))
    {
        FM4_BTIOSEL03->BTSEL0123 &= 0x0F;
        FM4_BTIOSEL03->BTSEL0123 |= (enIoMode<<4);
    }
    else if((pstcBt == &BT4) || (pstcBt == &BT5))
    {
        FM4_BTIOSEL47->BTSEL4567 &= 0xF0;
        FM4_BTIOSEL47->BTSEL4567 |= enIoMode;
    }
    else if((pstcBt == &BT6) || (pstcBt == &BT7))
    {
        FM4_BTIOSEL47->BTSEL4567 &= 0x0F;
        FM4_BTIOSEL47->BTSEL4567 |= (enIoMode<<4);
    }
    else if((pstcBt == &BT8) || (pstcBt == &BT9))
    {
        FM4_BTIOSEL8B->BTSEL89AB &= 0xF0;
        FM4_BTIOSEL8B->BTSEL89AB |= enIoMode;
    }
    else if((pstcBt == &BT10) || (pstcBt == &BT11))
    {
        FM4_BTIOSEL8B->BTSEL89AB &= 0x0F;
        FM4_BTIOSEL8B->BTSEL89AB |= (enIoMode<<4);
    }
    else if((pstcBt == &BT12) || (pstcBt == &BT13))
    {
        FM4_BTIOSELCF->BTSELCDEF &= 0xF0;
        FM4_BTIOSELCF->BTSELCDEF |= enIoMode;
    }
    else if((pstcBt == &BT14) || (pstcBt == &BT15))
    {
        FM4_BTIOSELCF->BTSELCDEF &= 0x0F;
        FM4_BTIOSELCF->BTSELCDEF |= (enIoMode<<4);
    }
    
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Select timer function of BT
 **
 ** \param [in]  pstcBt           BT instance
 ** \param [in]  enTimerMode      Timer mode
 **
 ** \retval Ok                    BT timer mode has been selected successfully
 ** \retval ErrorInvalidParameter pstcBt == NULL
 **                               Other invalid configuration
 ******************************************************************************/
en_result_t Bt_SelTimerMode(volatile stc_btn_t* pstcBt, en_bt_timer_mode_t enTimerMode)
{
    // Pointer to internal data
    stc_bt_intern_data_t* pstcBtInternData ;
    // Check for NULL pointer
    if ((pstcBt == NULL) || (enTimerMode > BtPwcMode))
    {
        return ErrorInvalidParameter ;
    }  
    
    // Get pointer to internal data structure ...
    pstcBtInternData = BtGetInternDataPtr( pstcBt ) ;
    
    if(pstcBtInternData == NULL)
    {
        return ErrorInvalidParameter ;
    }
    
    pstcBt->TMCR &= ~(7ul<<4);
    pstcBt->TMCR |= (enTimerMode<<4);
    
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Initialize PWM function of BT
 **
 ** \param [in]  pstcBt           BT instance
 ** \param [in]  pstcPwmConfig    Pointer to PWM configuration
 **
 ** \retval Ok                    PWM function has been configured successfully
 ** \retval ErrorInvalidParameter pstcBt == NULL
 **                               Other invalid configuration
 ******************************************************************************/
en_result_t Bt_Pwm_Init(volatile stc_btn_t* pstcBt, stc_bt_pwm_config_t* pstcPwmConfig)
{
    volatile FM4_BT_PWM_TypeDef* pRegBt = (FM4_BT_PWM_TypeDef*)pstcBt;
    // Pointer to internal data
    stc_bt_intern_data_t* pstcBtInternData ;
    // Check for NULL pointer
    if (pstcBt == NULL)
    {
        return ErrorInvalidParameter ;
    }  
    // Check configuration parameter
    if((pstcPwmConfig->enPres > PwmPres1Div2048)       ||
       (pstcPwmConfig->enRestartEn > PwmRestartEnable) ||
       (pstcPwmConfig->enOutputMask > PwmOutputMask)   || 
       (pstcPwmConfig->enExtTrig > PwmExtTrigBoth)     || 
       (pstcPwmConfig->enOutputPolarity > PwmPolarityHigh) ||
       (pstcPwmConfig->enMode > PwmOneshot) )
    {
        return ErrorInvalidParameter ;
    }
    
    // Get pointer to internal data structure ...
    pstcBtInternData = BtGetInternDataPtr( pstcBt ) ;
    
    if(pstcBtInternData == NULL)
    {
        return ErrorInvalidParameter ;
    }
    
    // Clock prescaler setting
    if(pstcPwmConfig->enPres < PwmPres1Div512)
    {
        pRegBt->TMCR2 = 0x00;
        pRegBt->TMCR &= ~(7ul<<12);
        pRegBt->TMCR |= (pstcPwmConfig->enPres<<12);
    }
    else
    {
        pRegBt->TMCR2 = 0x01;
        pRegBt->TMCR &= ~(7ul<<12);
        pRegBt->TMCR |= ((pstcPwmConfig->enPres & 0x7)<<12);
    }
    // Restart enable setting
    pRegBt->TMCR_f.RTGEN = pstcPwmConfig->enRestartEn;
    // Output Mask setting
    pRegBt->TMCR_f.PMSK = pstcPwmConfig->enOutputMask;
    // Output polarity setting
    pRegBt->TMCR_f.OSEL = pstcPwmConfig->enOutputPolarity;
    // External trigger setting
    pRegBt->TMCR &= ~(3ul<<8);
    pRegBt->TMCR |= (pstcPwmConfig->enExtTrig<<8);
    // Mode setting
    pRegBt->TMCR_f.MDSE = pstcPwmConfig->enMode;
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Enable PWM timer counting
 **
 ** \param [in]  pstcBt           BT instance
 **
 ** \retval Ok                    Enable PWM timer counting successfully
 ** \retval ErrorInvalidParameter pstcBt == NULL
 ******************************************************************************/
en_result_t Bt_Pwm_EnableCount(volatile stc_btn_t* pstcBt)
{
    // Check for NULL pointer
    if (pstcBt == NULL)
    {
        return ErrorInvalidParameter ;
    }  
    
    ((FM4_BT_PWM_TypeDef*)pstcBt)->TMCR_f.CTEN = 1;
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Disable PWM timer counting
 **
 ** \param [in]  pstcBt           BT instance
 **
 ** \retval Ok                    Disable PWM timer counting successfully
 ** \retval ErrorInvalidParameter pstcBt == NULL
 ******************************************************************************/
en_result_t Bt_Pwm_DisableCount(volatile stc_btn_t* pstcBt)
{
    // Check for NULL pointer
    if (pstcBt == NULL)
    {
        return ErrorInvalidParameter ;
    }  
    
    ((FM4_BT_PWM_TypeDef*)pstcBt)->TMCR_f.CTEN = 0;
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Start PWM timer by software
 **
 ** \param [in]  pstcBt           BT instance
 **
 ** \retval Ok                    Start PWM timer successfully
 ** \retval ErrorInvalidParameter pstcBt == NULL
 ******************************************************************************/
en_result_t Bt_Pwm_EnableSwTrig(volatile stc_btn_t* pstcBt)
{
    // Check for NULL pointer
    if (pstcBt == NULL)
    {
        return ErrorInvalidParameter ;
    }  
    
    ((FM4_BT_PWM_TypeDef*)pstcBt)->TMCR_f.STRG = 1;
    return Ok;
}

#if (PDL_INTERRUPT_ENABLE_BT0 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT1 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT2 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT3 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT4 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT5 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT6 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT7 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT8 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT9 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT10 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT11 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT12 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT13 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT14 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT15 == PDL_ON) 
/**
 ******************************************************************************
 ** \brief Enable PWM timer interrupt
 **
 ** \param [in]  pstcBt           BT instance
 ** \param [in]  pstcIntSel       PWM timer interrupt type
 ** \param [in]  pstcIntCallback  Pointer to interrupt callback function structure
 **
 ** \retval Ok                    Enable PWM timer interrupt successfully
 ** \retval ErrorInvalidParameter pstcBt == NULL
 **                               pstcIntCallback == NULL
 **                               Other invalid configuration
 ******************************************************************************/
en_result_t Bt_Pwm_EnableInt(volatile stc_btn_t* pstcBt, stc_pwm_int_sel_t* pstcIntSel, 
                             stc_pwm_int_cb_t*  pstcIntCallback)
{
    // Pointer to internal data
    stc_bt_intern_data_t* pstcBtInternData ;
    // Check parameter
    if ((pstcBt == NULL) ||
        (pstcIntSel->bPwmDutyMatchInt > 1) ||
        (pstcIntSel->bPwmTrigInt > 1) ||
        (pstcIntSel->bPwmUnderflowInt > 1) ||  
        (pstcIntCallback == NULL))
    {
        return ErrorInvalidParameter ;
    }  
    
    // Get pointer to internal data structure ...
    pstcBtInternData = BtGetInternDataPtr( pstcBt ) ;
    
    if(pstcBtInternData == NULL)
    {
        return ErrorInvalidParameter ;
    }
    
    if(pstcIntSel->bPwmTrigInt == 1)
    {
        pstcBtInternData->pfnBit2IntCallback = pstcIntCallback->pfnPwmTrigIntCallback;
        ((FM4_BT_PWM_TypeDef*)pstcBt)->STC_f.TGIE = 1;
    }
    if(pstcIntSel->bPwmDutyMatchInt == 1)
    {
        pstcBtInternData->pfnBit1IntCallback = pstcIntCallback->pfnPwmDutyMatchIntCallback;
        ((FM4_BT_PWM_TypeDef*)pstcBt)->STC_f.DTIE = 1;
    }
    if(pstcIntSel->bPwmUnderflowInt == 1)
    {
        pstcBtInternData->pfnBit0IntCallback = pstcIntCallback->pfnPwmUnderflowIntCallback;
        ((FM4_BT_PWM_TypeDef*)pstcBt)->STC_f.UDIE = 1;
    }

    Bt_InitNvic(pstcBt);    
    
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Disable PWM timer interrupt
 **
 ** \param [in]  pstcBt           BT instance
 ** \param [in]  pstcIntSel       Pointer to interrupt callback function
 **
 ** \retval Ok                    Disable PWM timer interrupt successfully
 ** \retval ErrorInvalidParameter pstcBt == NULL
 **                               Other invalid configuration
 ******************************************************************************/
en_result_t Bt_Pwm_DisableInt(volatile stc_btn_t* pstcBt, stc_pwm_int_sel_t* pstcIntSel)
{
    // Pointer to internal data
    stc_bt_intern_data_t* pstcBtInternData ;
    // Check parameter
    if ((pstcBt == NULL) ||
        (pstcIntSel->bPwmDutyMatchInt > 1) ||
        (pstcIntSel->bPwmTrigInt > 1) ||
        (pstcIntSel->bPwmUnderflowInt > 1))
    {
        return ErrorInvalidParameter ;
    }  
    
    // Get pointer to internal data structure ...
    pstcBtInternData = BtGetInternDataPtr( pstcBt ) ;
    
    if(pstcBtInternData == NULL)
    {
        return ErrorInvalidParameter ;
    }
    
    if(pstcIntSel->bPwmTrigInt == 1)
    {
        pstcBtInternData->pfnBit2IntCallback = NULL;
        ((FM4_BT_PWM_TypeDef*)pstcBt)->STC_f.TGIE = 0;
    }
    if(pstcIntSel->bPwmDutyMatchInt == 1)
    {
        pstcBtInternData->pfnBit1IntCallback = NULL;
        ((FM4_BT_PWM_TypeDef*)pstcBt)->STC_f.DTIE = 0;
    }
    if(pstcIntSel->bPwmUnderflowInt == 1)
    {
        pstcBtInternData->pfnBit0IntCallback = NULL;
        ((FM4_BT_PWM_TypeDef*)pstcBt)->STC_f.UDIE = 0;
    }
    if( (((FM4_BT_PWM_TypeDef*)pstcBt)->STC_f.TGIE == 0) && 
        (((FM4_BT_PWM_TypeDef*)pstcBt)->STC_f.DTIE == 0) && 
        (((FM4_BT_PWM_TypeDef*)pstcBt)->STC_f.UDIE == 0)  )
    {
        Bt_DeInitNvic(pstcBt); 
    }
    
    return Ok;
}

#endif  

/**
 ******************************************************************************
 ** \brief Get interrupt flag of PWM timer 
 **
 ** \param [in]  pstcBt           BT instance
 ** \param [in]  enIntType        PWM timer interrupt type
 **
 ** \retval PdlSet                Interrupt flag is set
 ** \retval PdlClr                Interrupt flag is clear
 ******************************************************************************/
en_int_flag_t Bt_Pwm_GetIntFlag(volatile stc_btn_t* pstcBt, en_bt_pwm_int_t enIntType)
{
    en_int_flag_t enFlag; 
    
    switch (enIntType)
    {
        case PwmTrigInt:
            enFlag = ((FM4_BT_PWM_TypeDef*)pstcBt)->STC_f.TGIR ? PdlSet : PdlClr;
            break;
        case PwmDutyMatchInt:
            enFlag = ((FM4_BT_PWM_TypeDef*)pstcBt)->STC_f.DTIR ? PdlSet : PdlClr;
            break;
        case PwmUnderflowInt:
            enFlag = ((FM4_BT_PWM_TypeDef*)pstcBt)->STC_f.UDIR ? PdlSet : PdlClr;
            break;
        default:
            break;
    }
    
    return enFlag;
}

/**
 ******************************************************************************
 ** \brief Clear interrupt flag of PWM timer 
 **
 ** \param [in]  pstcBt           BT instance
 ** \param [in]  enIntType        PWM timer interrupt type
 **
 ** \retval ErrorInvalidParameter pstcBt == NULL
 **                               enIntType > PwmUnderflowInt
 ** \retval Ok                    Clear interrupt flag successfully
 ******************************************************************************/
en_result_t Bt_Pwm_ClrIntFlag(volatile stc_btn_t* pstcBt, en_bt_pwm_int_t enIntType)
{
    if((pstcBt == NULL) || (enIntType > PwmUnderflowInt))
    {
        return ErrorInvalidParameter ;
    }
  
    switch (enIntType)
    {
        case PwmTrigInt:
            ((FM4_BT_PWM_TypeDef*)pstcBt)->STC_f.TGIR = 0;
            break;
        case PwmDutyMatchInt:
            ((FM4_BT_PWM_TypeDef*)pstcBt)->STC_f.DTIR = 0;
            break;
        case PwmUnderflowInt:
            ((FM4_BT_PWM_TypeDef*)pstcBt)->STC_f.UDIR = 0;
            break;
        default:
            break;
    }
    
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Write Cycle value of PWM timer 
 **
 ** \param [in]  pstcBt           BT instance
 ** \param [in]  u16Cycle         Cycle value
 **
 ** \retval Ok                    Write Cycle value successfully
 ** \retval ErrorInvalidParameter pstcBt == NULL
 ******************************************************************************/
en_result_t Bt_Pwm_WriteCycleVal(volatile stc_btn_t* pstcBt, uint16_t u16Cycle)
{
    // Check parameter
    if ((pstcBt == NULL))
    {
        return ErrorInvalidParameter ;
    }  
    
    ((FM4_BT_PWM_TypeDef*)pstcBt)->PCSR = u16Cycle;
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Write duty value of PWM timer 
 **
 ** \param [in]  pstcBt           BT instance
 ** \param [in]  u16Duty          Cycle value
 **
 ** \retval Ok                    Write duty value successfully
 ** \retval ErrorInvalidParameter pstcBt == NUL
 ******************************************************************************/
en_result_t Bt_Pwm_WriteDutyVal(volatile stc_btn_t* pstcBt, uint16_t u16Duty)
{
    // Check parameter
    if ((pstcBt == NULL))
    {
        return ErrorInvalidParameter ;
    }  
    
    ((FM4_BT_PWM_TypeDef*)pstcBt)->PDUT = u16Duty;
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Read current count value of PWM timer 
 **
 ** \param [in]  pstcBt           BT instance
 **
 ** \retval Current count value
 ******************************************************************************/
uint16_t Bt_Pwm_ReadCurCnt(volatile stc_btn_t* pstcBt)
{ 
    return ((FM4_BT_PWM_TypeDef*)pstcBt)->TMR;
}

/**
 ******************************************************************************
 ** \brief Initialize PPG function of BT
 **
 ** \param [in]  pstcBt           BT instance
 ** \param [in]  pstcPpgConfig    Pointer to PPG configuration
 **
 ** \retval Ok                    PPG function has been configured successfully
 ** \retval ErrorInvalidParameter pstcBt == NULL
 **                               Other invalid configuration
 ******************************************************************************/
en_result_t Bt_Ppg_Init(volatile stc_btn_t* pstcBt, stc_bt_ppg_config_t* pstcPpgConfig)
{
    volatile FM4_BT_PPG_TypeDef* pRegBt = (FM4_BT_PPG_TypeDef*) pstcBt;
    // Pointer to internal data
    stc_bt_intern_data_t* pstcBtInternData ;
    // Check for NULL pointer
    if (pstcBt == NULL)
    {
        return ErrorInvalidParameter ;
    }  
    // Check configuration parameter
    if((pstcPpgConfig->enPres > PpgPres1Div2048)       ||
       (pstcPpgConfig->enRestartEn > PpgRestartEnable) ||
       (pstcPpgConfig->enOutputMask > PpgOutputMask)   || 
       (pstcPpgConfig->enExtTrig > PpgExtTrigBoth)     || 
       (pstcPpgConfig->enOutputPolarity > PpgPolarityHigh) ||
       (pstcPpgConfig->enMode > PpgOneshot) )
    {
        return ErrorInvalidParameter ;
    }
    
    // Get pointer to internal data structure ...
    pstcBtInternData = BtGetInternDataPtr( pstcBt ) ;
    
    if(pstcBtInternData == NULL)
    {
        return ErrorInvalidParameter ;
    }
    
    // Clock prescaler setting
    if(pstcPpgConfig->enPres < PpgPres1Div512)
    {
        pRegBt->TMCR2 = 0x00;
        pRegBt->TMCR &= ~(7ul<<12);
        pRegBt->TMCR |= (pstcPpgConfig->enPres<<12);
    }
    else
    {
        pRegBt->TMCR2 = 0x01;
        pRegBt->TMCR &= ~(7ul<<12);
        pRegBt->TMCR |= ((pstcPpgConfig->enPres & 0x7)<<12);
    }
    // Restart enable setting
    pRegBt->TMCR_f.RTGEN = pstcPpgConfig->enRestartEn;
    // Output Mask setting
    pRegBt->TMCR_f.PMSK = pstcPpgConfig->enOutputMask;
    // Output polarity setting
    pRegBt->TMCR_f.OSEL = pstcPpgConfig->enOutputPolarity;
    // External trigger setting
    pRegBt->TMCR &= ~(3ul<<8);
    pRegBt->TMCR |= pstcPpgConfig->enExtTrig;
    // Mode setting
    pRegBt->TMCR_f.MDSE = pstcPpgConfig->enMode;
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Enable PPG timer counting
 **
 ** \param [in]  pstcBt           BT instance
 **
 ** \retval Ok                    Enable PPG timer counting successfully
 ** \retval ErrorInvalidParameter pstcBt == NULL
 ******************************************************************************/
en_result_t Bt_Ppg_EnableCount(volatile stc_btn_t* pstcBt)
{
    // Check for NULL pointer
    if (pstcBt == NULL)
    {
        return ErrorInvalidParameter ;
    }  
    
    ((FM4_BT_PPG_TypeDef*)pstcBt)->TMCR_f.CTEN = 1;
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Disable PPG timer counting
 **
 ** \param [in]  pstcBt           BT instance
 **
 ** \retval Ok                    Disable PPG timer counting successfully
 ** \retval ErrorInvalidParameter pstcBt == NULL
 ******************************************************************************/
en_result_t Bt_Ppg_DisableCount(volatile stc_btn_t* pstcBt)
{
    // Check for NULL pointer
    if (pstcBt == NULL)
    {
        return ErrorInvalidParameter ;
    }  
    
    ((FM4_BT_PPG_TypeDef*)pstcBt)->TMCR_f.CTEN = 0;
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Start PPG timer by software
 **
 ** \param [in]  pstcBt           BT instance
 **
 ** \retval Ok                    Start PPG timer successfully
 ** \retval ErrorInvalidParameter pstcBt == NULL
 ******************************************************************************/
en_result_t Bt_Ppg_EnableSwTrig(volatile stc_btn_t* pstcBt)
{
    // Check for NULL pointer
    if (pstcBt == NULL)
    {
        return ErrorInvalidParameter ;
    }  
    
    ((FM4_BT_PPG_TypeDef*)pstcBt)->TMCR_f.STRG = 1;
    return Ok;
}

#if (PDL_INTERRUPT_ENABLE_BT0 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT1 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT2 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT3 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT4 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT5 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT6 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT7 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT8 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT9 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT10 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT11 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT12 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT13 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT14 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT15 == PDL_ON)
/**
 ******************************************************************************
 ** \brief Enable PPG timer interrupt
 **
 ** \param [in]  pstcBt           BT instance
 ** \param [in]  pstcIntSel       PPG timer interrupt type
 ** \param [in]  pstcIntCallback  Pointer to interrupt callback function structure
 **
 ** \retval Ok                    Enable PPG timer interrupt successfully
 ** \retval ErrorInvalidParameter pstcBt == NULL
 **                               pstcIntCallback == NULL
 **                               Other invalid configuration
 ******************************************************************************/
en_result_t Bt_Ppg_EnableInt(volatile stc_btn_t* pstcBt, stc_ppg_int_sel_t* pstcIntSel, 
                             stc_ppg_int_cb_t*  pstcIntCallback)
{
    // Pointer to internal data
    stc_bt_intern_data_t* pstcBtInternData ;
    // Check parameter
    if ((pstcBt == NULL) ||
        (pstcIntSel->bPpgTrigInt > 1) ||
        (pstcIntSel->bPpgUnderflowInt > 1) ||  
        (pstcIntCallback == NULL))
    {
        return ErrorInvalidParameter ;
    }  
    
    // Get pointer to internal data structure ...
    pstcBtInternData = BtGetInternDataPtr( pstcBt ) ;
    
    if(pstcBtInternData == NULL)
    {
        return ErrorInvalidParameter ;
    }
    
    if(pstcIntSel->bPpgTrigInt == 1)
    {
        pstcBtInternData->pfnBit2IntCallback = pstcIntCallback->pfnPpgTrigIntCallback;
        ((FM4_BT_PPG_TypeDef*)pstcBt)->STC_f.TGIE = 1;
    }
    if(pstcIntSel->bPpgUnderflowInt == 1)
    {
        pstcBtInternData->pfnBit0IntCallback = pstcIntCallback->pfnPpgUnderflowIntCallback;
        ((FM4_BT_PPG_TypeDef*)pstcBt)->STC_f.UDIE = 1;
    }
    
    Bt_InitNvic(pstcBt);
    
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Disable PPG timer interrupt
 **
 ** \param [in]  pstcBt           BT instance
 ** \param [in]  pstcIntSel       PPG timer interrupt type
 **
 ** \retval Ok                    Disable PPG timer interrupt successfully
 ** \retval ErrorInvalidParameter pstcBt == NULL
 **                               Other invalid configuration
 ******************************************************************************/
en_result_t Bt_Ppg_DisableInt(volatile stc_btn_t* pstcBt, stc_ppg_int_sel_t* pstcIntSel)
{
    // Pointer to internal data
    stc_bt_intern_data_t* pstcBtInternData ;
    // Check parameter
    if ((pstcBt == NULL) ||
        (pstcIntSel->bPpgTrigInt > 1) ||
        (pstcIntSel->bPpgUnderflowInt > 1)  )
    {
        return ErrorInvalidParameter ;
    }  
    
    // Get pointer to internal data structure ...
    pstcBtInternData = BtGetInternDataPtr( pstcBt ) ;
    
    if(pstcBtInternData == NULL)
    {
        return ErrorInvalidParameter ;
    }
    
    if(pstcIntSel->bPpgTrigInt == 1)
    {
        pstcBtInternData->pfnBit2IntCallback = NULL;
        ((FM4_BT_PPG_TypeDef*)pstcBt)->STC_f.TGIE = 0;
    }
    if(pstcIntSel->bPpgUnderflowInt == 1)
    {
        pstcBtInternData->pfnBit0IntCallback = NULL;
        ((FM4_BT_PPG_TypeDef*)pstcBt)->STC_f.UDIE = 0;
    }
    
    if( (((FM4_BT_PPG_TypeDef*)pstcBt)->STC_f.TGIE == 0) &&
        (((FM4_BT_PPG_TypeDef*)pstcBt)->STC_f.UDIE == 0))
    {
        Bt_DeInitNvic(pstcBt);
    }
    return Ok;
}

#endif

/**
 ******************************************************************************
 ** \brief Get interrupt flag of PPG timer 
 **
 ** \param [in]  pstcBt          BT instance
 ** \param [in]  enIntType        PPG timer interrupt type
 **
 ** \retval PdlSet                Interrupt flag is set
 ** \retval PdlClr                Interrupt flag is clear
 ******************************************************************************/
en_int_flag_t Bt_Ppg_GetIntFlag(volatile stc_btn_t* pstcBt, en_bt_ppg_int_t enIntType)
{
    en_int_flag_t enFlag; 
    
    switch (enIntType)
    {
        case PpgTrigInt:
            enFlag = ((FM4_BT_PPG_TypeDef*)pstcBt)->STC_f.TGIR ? PdlSet : PdlClr;
            break;
        case PpgUnderflowInt:
            enFlag = ((FM4_BT_PPG_TypeDef*)pstcBt)->STC_f.UDIR ? PdlSet : PdlClr;
            break;
        default:
            break;
    }
    
    return enFlag;
}

/**
 ******************************************************************************
 ** \brief Clear interrupt flag of PPG timer 
 **
 ** \param [in]  pstcBt           BT instance
 ** \param [in]  enIntType        PPG timer interrupt type
 **
 ** \retval ErrorInvalidParameter pstcBt == NULL
 **                               enIntType > PwmUnderflowInt
 ** \retval Ok                    Clear interrupt flag successfully
 ******************************************************************************/
en_result_t Bt_Ppg_ClrIntFlag(volatile stc_btn_t* pstcBt, en_bt_ppg_int_t enIntType)
{
    if((pstcBt == NULL) || (enIntType > PpgUnderflowInt))
    {
        return ErrorInvalidParameter;
    }
  
    switch (enIntType)
    {
        case PpgTrigInt:
            ((FM4_BT_PPG_TypeDef*)pstcBt)->STC_f.TGIR = 0;
            break;
        case PpgUnderflowInt:
            ((FM4_BT_PPG_TypeDef*)pstcBt)->STC_f.UDIR = 0;
            break;
        default:
            break;
    }
    
    return Ok;
}
                    
/**
 ******************************************************************************
 ** \brief Write low width count value of PPG timer 
 **
 ** \param [in]  pstcBt           BT instance
 ** \param [in]  u16Val           Low width count value
 **
 ** \retval Ok                    Write low width count value successfully
 ** \retval ErrorInvalidParameter pstcBt == NULL
 ******************************************************************************/
en_result_t Bt_Ppg_WriteLowWidthVal(volatile stc_btn_t* pstcBt, uint16_t u16Val)
{
    // Check parameter
    if ((pstcBt == NULL))
    {
        return ErrorInvalidParameter ;
    }  
    
    ((FM4_BT_PPG_TypeDef*)pstcBt)->PRLL = u16Val;
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Write high width count value of PPG timer 
 **
 ** \param [in]  pstcBt           BT instance
 ** \param [in]  u16Val           High width count value
 **
 ** \retval Ok                    Write high width count value successfully
 ** \retval ErrorInvalidParameter pstcBt == NULL
 ******************************************************************************/
en_result_t Bt_Ppg_WriteHighWidthVal(volatile stc_btn_t* pstcBt, uint16_t u16Val)
{
    // Check parameter
    if ((pstcBt == NULL))
    {
        return ErrorInvalidParameter ;
    }  
    
    ((FM4_BT_PPG_TypeDef*)pstcBt)->PRLH = u16Val;
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Read current count value of PPG timer 
 **
 ** \param [in]  pstcBt           BT instance
 **
 ** \retval Current count value
 ******************************************************************************/
uint16_t Bt_Ppg_ReadCurCnt(volatile stc_btn_t* pstcBt)
{ 
    return ((FM4_BT_PPG_TypeDef*)pstcBt)->TMR;
}

/**
 ******************************************************************************
 ** \brief Initialize RT function of BT
 **
 ** \param [in]  pstcBt           BT instance
 ** \param [in]  pstcRtConfig     Pointer to RT configuration
 **
 ** \retval Ok                    RT function has been configured successfully
 ** \retval ErrorInvalidParameter pstcBt == NULL
 **                               Other invalid configuration
 ******************************************************************************/
en_result_t Bt_Rt_Init(volatile stc_btn_t* pstcBt, stc_bt_rt_config_t* pstcRtConfig)
{
    volatile FM4_BT_RT_TypeDef* pRegBt = (FM4_BT_RT_TypeDef*) pstcBt;
    // Pointer to internal data
    stc_bt_intern_data_t* pstcBtInternData ;
    // Check for NULL pointer
    if (pstcBt == NULL)
    {
        return ErrorInvalidParameter ;
    }  
    // Check configuration parameter
    if((pstcRtConfig->enPres > RtPres1Div2048)           ||
       (pstcRtConfig->enSize > RtSize32Bit)              ||
       (pstcRtConfig->enExtTrig > RtExtTiggerHighLevel)  || 
       (pstcRtConfig->enOutputPolarity > RtPolarityHigh) ||
       (pstcRtConfig->enMode > RtOneshot) )
    {
        return ErrorInvalidParameter ;
    }
    
    // Get pointer to internal data structure ...
    pstcBtInternData = BtGetInternDataPtr( pstcBt ) ;
    
    if(pstcBtInternData == NULL)
    {
        return ErrorInvalidParameter ;
    }
    
    // Clock prescaler setting
    if(pstcRtConfig->enPres < RtPres1Div512)
    {
        pRegBt->TMCR2 = 0x00;
        pRegBt->TMCR &= ~(7ul<<12);
        pRegBt->TMCR |= (pstcRtConfig->enPres<<12);
    }
    else
    {
        pRegBt->TMCR2 = 0x01;
        pRegBt->TMCR &= ~(7ul<<12);
        pRegBt->TMCR |= ((pstcRtConfig->enPres & 0x7)<<12);
        
    }
    // Timer size setting
    pRegBt->TMCR_f.T32 = pstcRtConfig->enSize;
    // Output polarity setting
    pRegBt->TMCR_f.OSEL = pstcRtConfig->enOutputPolarity;
    // External trigger setting
    if(pstcRtConfig->enExtTrig <= RtExtTiggerBothEdge)
    {
        pRegBt->TMCR2_f.GATE = 0;
        pRegBt->TMCR &= ~(3ul<<8);
        pRegBt->TMCR |= (pstcRtConfig->enExtTrig)<<8;
    }
    else if (pstcRtConfig->enExtTrig == RtExtTiggerLowLevel)
    {
        pRegBt->TMCR2_f.GATE = 1;
        pRegBt->TMCR &= ~(3ul<<8);
    }
    else if (pstcRtConfig->enExtTrig == RtExtTiggerHighLevel)
    {
        pRegBt->TMCR2_f.GATE = 1;
        pRegBt->TMCR &= ~(3ul<<8);
        pRegBt->TMCR |= (1ul<<8);
    }
    // Mode setting
    pRegBt->TMCR_f.MDSE = pstcRtConfig->enMode;
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Enable Reload timer counting
 **
 ** \param [in]  pstcBt           BT instance
 **
 ** \retval Ok                    Enable RT timer counting successfully
 ** \retval ErrorInvalidParameter pstcBt == NULL
 ******************************************************************************/
en_result_t Bt_Rt_EnableCount(volatile stc_btn_t* pstcBt)
{
    // Check for NULL pointer
    if (pstcBt == NULL)
    {
        return ErrorInvalidParameter ;
    }  
    
    ((FM4_BT_RT_TypeDef*)pstcBt)->TMCR_f.CTEN = 1;
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Disable Reload timer counting
 **
 ** \param [in]  pstcBt           BT instance
 **
 ** \retval Ok                    Disable RT timer counting successfully
 ** \retval ErrorInvalidParameter pstcBt == NULL
 ******************************************************************************/
en_result_t Bt_Rt_DisableCount(volatile stc_btn_t* pstcBt)
{
    // Check for NULL pointer
    if (pstcBt == NULL)
    {
        return ErrorInvalidParameter ;
    }  
    
    ((FM4_BT_RT_TypeDef*)pstcBt)->TMCR_f.CTEN = 0;
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Start Reload timer by software
 **
 ** \param [in]  pstcBt           BT instance
 **
 ** \retval Ok                    Start Reload timer successfully
 ** \retval ErrorInvalidParameter pstcBt == NULL
 ******************************************************************************/
en_result_t Bt_Rt_EnableSwTrig(volatile stc_btn_t* pstcBt)
{
    // Check for NULL pointer
    if (pstcBt == NULL)
    {
        return ErrorInvalidParameter ;
    }  
    
    ((FM4_BT_RT_TypeDef*)pstcBt)->TMCR_f.STRG = 1;
    return Ok;
}

#if (PDL_INTERRUPT_ENABLE_BT0 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT1 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT2 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT3 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT4 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT5 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT6 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT7 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT8 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT9 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT10 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT11 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT12 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT13 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT14 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT15 == PDL_ON)
/**
 ******************************************************************************
 ** \brief Enable Reload timer interrupt
 **
 ** \param [in]  pstcBt           BT instance
 ** \param [in]  pstcIntSel       Reload timer interrupt type
 ** \param [in]  pstcIntCallback  Pointer to interrupt callback function structure
 **
 ** \retval Ok                    Enable Reload timer interrupt successfully
 ** \retval ErrorInvalidParameter pstcBt == NULL
 **                               pstcIntCallback == NULL
 **                               Other invalid configuration
 ******************************************************************************/
en_result_t Bt_Rt_EnableInt(volatile stc_btn_t* pstcBt, stc_rt_int_sel_t* pstcIntSel, 
                            stc_rt_int_cb_t*  pstcIntCallback)
{
    // Pointer to internal data
    stc_bt_intern_data_t* pstcBtInternData ;
    // Check parameter
    if ((pstcBt == NULL) ||
        (pstcIntSel->bRtTrigInt > 1) ||
        (pstcIntSel->bRtUnderflowInt > 1) ||  
        (pstcIntCallback == NULL))
    {
        return ErrorInvalidParameter ;
    }  
    
    // Get pointer to internal data structure ...
    pstcBtInternData = BtGetInternDataPtr( pstcBt ) ;
    
    if(pstcBtInternData == NULL)
    {
        return ErrorInvalidParameter ;
    }
    
    if(pstcIntSel->bRtTrigInt == 1)
    {
        pstcBtInternData->pfnBit2IntCallback = pstcIntCallback->pfnRtTrigIntCallback;
        ((FM4_BT_RT_TypeDef*)pstcBt)->STC_f.TGIE = 1;
    }
    if(pstcIntSel->bRtUnderflowInt == 1)
    {
        pstcBtInternData->pfnBit0IntCallback = pstcIntCallback->pfnRtUnderflowIntCallback;
        ((FM4_BT_RT_TypeDef*)pstcBt)->STC_f.UDIE = 1;
    }

    Bt_InitNvic(pstcBt);
    
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Disable Reload timer interrupt
 **
 ** \param [in]  pstcBt           BT instance
 ** \param [in]  pstcIntSel       Pointer to interrupt callback function structure
 **
 ** \retval Ok                    Disable Reload timer interrupt successfully
 ** \retval ErrorInvalidParameter pstcBt == NULL
 **                               Other invalid configuration
 ******************************************************************************/
en_result_t Bt_Rt_DisableInt(volatile stc_btn_t* pstcBt, stc_rt_int_sel_t* pstcIntSel)
{
    // Pointer to internal data
    stc_bt_intern_data_t* pstcBtInternData ;
    // Check parameter
    if ((pstcBt == NULL) ||
        (pstcIntSel->bRtTrigInt > 1) ||
        (pstcIntSel->bRtUnderflowInt > 1)  )
    {
        return ErrorInvalidParameter ;
    }  
    
    // Get pointer to internal data structure ...
    pstcBtInternData = BtGetInternDataPtr( pstcBt ) ;
    
    if(pstcBtInternData == NULL)
    {
        return ErrorInvalidParameter ;
    }
    
    if(pstcIntSel->bRtTrigInt == 1)
    {
        pstcBtInternData->pfnBit2IntCallback = NULL;
        ((FM4_BT_RT_TypeDef*)pstcBt)->STC_f.TGIE = 0;
    }
    if(pstcIntSel->bRtUnderflowInt == 1)
    {
        pstcBtInternData->pfnBit0IntCallback = NULL;
        ((FM4_BT_RT_TypeDef*)pstcBt)->STC_f.UDIE = 0;
    }
    
    Bt_DeInitNvic(pstcBt);
    
    return Ok;
}

#endif

/**
 ******************************************************************************
 ** \brief Get interrupt flag of Reload timer 
 **
 ** \param [in]  pstcBt           BT instance
 ** \param [in]  enIntType        Reload timer interrupt type
 **
 ** \retval PdlSet                Interrupt flag is set
 ** \retval PdlClr                Interrupt flag is clear
 ******************************************************************************/
en_int_flag_t Bt_Rt_GetIntFlag(volatile stc_btn_t* pstcBt, en_bt_rt_int_t enIntType)
{
    en_int_flag_t enFlag; 
    
    switch (enIntType)
    {
        case RtTrigInt:
            enFlag = ((FM4_BT_RT_TypeDef*)pstcBt)->STC_f.TGIR ? PdlSet : PdlClr;
            break;
        case RtUnderflowInt:
            enFlag = ((FM4_BT_RT_TypeDef*)pstcBt)->STC_f.UDIR ? PdlSet : PdlClr;
            break;
        default:
            break;
    }
    
    return enFlag;
}

/**
 ******************************************************************************
 ** \brief Clear interrupt flag of Reload timer 
 **
 ** \param [in]  pstcBt             BT instance
 ** \param [in]  enIntType          Reload timer interrupt type
 **
 ** \retval ErrorInvalidParameter   pstcBt == NULL
 **                                 enIntType > RtUnderflowInt
 ** \retval Ok                      Clear interrupt flag successfully
 ******************************************************************************/
en_result_t Bt_Rt_ClrIntFlag(volatile stc_btn_t* pstcBt, en_bt_rt_int_t enIntType)
{
    if((pstcBt == NULL) || (enIntType > RtUnderflowInt))
    {
        return ErrorInvalidParameter;
    }
    
    switch (enIntType)
    {
        case RtTrigInt:
            ((FM4_BT_RT_TypeDef*)pstcBt)->STC_f.TGIR = 0;
            break;
        case RtUnderflowInt:
            ((FM4_BT_RT_TypeDef*)pstcBt)->STC_f.UDIR = 0;
            break;
        default:
            break;
    }
    
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Write count cycle of Reload timer 
 **
 ** \param [in]  pstcBt           BT instance
 ** \param [in]  u16Val           Cycle value
 **
 ** \retval Ok                    Write count cycle successfully
 ** \retval ErrorInvalidParameter pstcBt == NULL
 ******************************************************************************/
en_result_t Bt_Rt_WriteCycleVal(volatile stc_btn_t* pstcBt, uint16_t u16Val)
{
    // Check parameter
    if ((pstcBt == NULL))
    {
        return ErrorInvalidParameter ;
    }  
    
    ((FM4_BT_RT_TypeDef*)pstcBt)->PCSR = u16Val;
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Read current count value of Reload timer 
 **
 ** \param [in]  pstcBt           BT instance
 **
 ** \retval Current count value
 ******************************************************************************/
uint16_t Bt_Rt_ReadCurCnt(volatile stc_btn_t* pstcBt)
{ 
    return ((FM4_BT_RT_TypeDef*)pstcBt)->TMR;
}

/**
 ******************************************************************************
 ** \brief Initialize PWC function of BT
 **
 ** \param [in]  pstcBt           BT instance
 ** \param [in]  pstcPwcConfig    Pointer to PWC configuration
 **
 ** \retval Ok                    PWC function has been configured successfully
 ** \retval ErrorInvalidParameter pstcBt == NULL
 **                               Other invalid configuration
 ******************************************************************************/
en_result_t Bt_Pwc_Init(volatile stc_btn_t* pstcBt, stc_bt_pwc_config_t* pstcPwcConfig)
{
    volatile FM4_BT_PWC_TypeDef* pRegBt = (FM4_BT_PWC_TypeDef*) pstcBt;
    // Pointer to internal data
    stc_bt_intern_data_t* pstcBtInternData ;
    // Check for NULL pointer
    if (pstcBt == NULL)
    {
        return ErrorInvalidParameter ;
    }  
    // Check configuration parameter
    if((pstcPwcConfig->enPres > PwcPres1Div2048)           ||
       (pstcPwcConfig->enSize > PwcSize32Bit)              ||
       (pstcPwcConfig->enMeasureEdge > PwcMeasureFallingToRising)  || 
       (pstcPwcConfig->enMode > PwcOneshot) )
    {
        return ErrorInvalidParameter ;
    }
    
    // Get pointer to internal data structure ...
    pstcBtInternData = BtGetInternDataPtr( pstcBt ) ;
    
    if(pstcBtInternData == NULL)
    {
        return ErrorInvalidParameter ;
    }
    
    // Clock prescaler setting
    if(pstcPwcConfig->enPres < PwcPres1Div512)
    {
        pRegBt->TMCR2 = 0x00;
        pRegBt->TMCR &= ~(7ul<<12);
        pRegBt->TMCR |= (pstcPwcConfig->enPres<<12);
    }
    else
    {
        pRegBt->TMCR2 = 0x01;
        pRegBt->TMCR &= ~(7ul<<12);
        pRegBt->TMCR |= ((pstcPwcConfig->enPres & 0x7)<<12);
    }
    // Timer size setting
    pRegBt->TMCR_f.T32 = pstcPwcConfig->enSize;
    // Measurement mode setting
    pRegBt->TMCR &= ~(7ul<<8);
    pRegBt->TMCR |= (pstcPwcConfig->enMeasureEdge<<8);
    // Mode setting
    pRegBt->TMCR_f.MDSE = pstcPwcConfig->enMode;
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Enable PWC timer counting
 **
 ** \param [in]  pstcBt           BT instance
 **
 ** \retval Ok                    Enable PWC timer counting successfully
 ** \retval ErrorInvalidParameter pstcBt == NULL
 ******************************************************************************/
en_result_t Bt_Pwc_EnableCount(volatile stc_btn_t* pstcBt)
{
    // Check for NULL pointer
    if (pstcBt == NULL)
    {
        return ErrorInvalidParameter ;
    }  
    
    ((FM4_BT_PWC_TypeDef*)pstcBt)->TMCR_f.CTEN = 1;
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Disable PWC timer counting
 **
 ** \param [in]  pstcBt           BT instance
 **
 ** \retval Ok                    Disable PWC timer counting successfully
 ** \retval ErrorInvalidParameter pstcBt == NULL
 ******************************************************************************/
en_result_t Bt_Pwc_DisableCount(volatile stc_btn_t* pstcBt)
{
    // Check for NULL pointer
    if (pstcBt == NULL)
    {
        return ErrorInvalidParameter ;
    }  
    
    ((FM4_BT_PWC_TypeDef*)pstcBt)->TMCR_f.CTEN = 0;
    return Ok;
}

#if (PDL_INTERRUPT_ENABLE_BT0 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT1 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT2 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT3 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT4 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT5 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT6 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT7 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT8 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT9 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT10 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT11 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT12 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT13 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT14 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT15 == PDL_ON)
/**
 ******************************************************************************
 ** \brief Enable PWC timer interrupt
 **
 ** \param [in]  pstcBt           BT instance
 ** \param [in]  pstcIntSel       PWC timer interrupt type
 ** \param [in]  pstcIntCallback  Pointer to interrupt callback function structure
 **
 ** \retval Ok                    Enable PWC timer interrupt successfully
 ** \retval ErrorInvalidParameter pstcBt == NULL
 **                               pstcIntCallback == NULL
 **                               Other invalid configuration
 ******************************************************************************/
en_result_t Bt_Pwc_EnableInt(volatile stc_btn_t* pstcBt, stc_pwc_int_sel_t* pstcIntSel, 
                             stc_pwc_int_cb_t*  pstcIntCallback)
{
    // Pointer to internal data
    stc_bt_intern_data_t* pstcBtInternData ;
    // Check parameter
    if ((pstcBt == NULL) ||
        (pstcIntSel->bPwcMeasCmpInt > 1) ||
        (pstcIntSel->bPwcMeasOverflowInt > 1) || 
        (pstcIntCallback == NULL))
    {
        return ErrorInvalidParameter ;
    }  
    
    // Get pointer to internal data structure ...
    pstcBtInternData = BtGetInternDataPtr( pstcBt ) ;
    
    if(pstcBtInternData == NULL)
    {
        return ErrorInvalidParameter ;
    }
    
    if(pstcIntSel->bPwcMeasCmpInt == 1)
    {
        pstcBtInternData->pfnBit2IntCallback = pstcIntCallback->pfnPwcMeasCmpCallback;
        ((FM4_BT_PWC_TypeDef*)pstcBt)->STC_f.EDIE = 1;
    }
    if(pstcIntSel->bPwcMeasOverflowInt == 1)
    {
        pstcBtInternData->pfnBit0IntCallback = pstcIntCallback->pfnPwcMeasOverflowCallback;
        ((FM4_BT_PWC_TypeDef*)pstcBt)->STC_f.OVIE = 1;
    }
    
    Bt_InitNvic(pstcBt);
    
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Disable PWC timer interrupt
 **
 ** \param [in]  pstcBt           BT instance
 ** \param [in]  pstcIntSel       PWC timer interrupt type structure pointer
 **
 ** \retval Ok                    Disable PWC timer interrupt successfully
 ** \retval ErrorInvalidParameter pstcBt == NULL
 **                               Other invalid configuration
 ******************************************************************************/
en_result_t Bt_Pwc_DisableInt(volatile stc_btn_t* pstcBt, stc_pwc_int_sel_t* pstcIntSel)
{
    // Pointer to internal data
    stc_bt_intern_data_t* pstcBtInternData ;
    // Check parameter
    if ((pstcBt == NULL) ||
        (pstcIntSel->bPwcMeasCmpInt > 1) ||
        (pstcIntSel->bPwcMeasOverflowInt > 1))
    {
        return ErrorInvalidParameter ;
    }  
    
    // Get pointer to internal data structure ...
    pstcBtInternData = BtGetInternDataPtr( pstcBt ) ;
    
    if(pstcBtInternData == NULL)
    {
        return ErrorInvalidParameter ;
    }
    
    if(pstcIntSel->bPwcMeasCmpInt == 1)
    {
        pstcBtInternData->pfnBit2IntCallback = NULL;
        ((FM4_BT_PWC_TypeDef*)pstcBt)->STC_f.EDIE = 0;
    }
    if(pstcIntSel->bPwcMeasOverflowInt == 1)
    {
        pstcBtInternData->pfnBit0IntCallback = NULL;
        ((FM4_BT_PWC_TypeDef*)pstcBt)->STC_f.OVIE = 0;
    }
    
    if( (((FM4_BT_PWC_TypeDef*)pstcBt)->STC_f.EDIE == 0) && 
        (((FM4_BT_PWC_TypeDef*)pstcBt)->STC_f.OVIE == 0))
    {  
        Bt_DeInitNvic(pstcBt);
    }
    
    return Ok;
}
#endif

/**
 ******************************************************************************
 ** \brief Get interrupt flag of PWC timer 
 **
 ** \param [in]  pstcBt           BT instance
 ** \param [in]  enIntType        PWC timer interrupt type
 **
 ** \retval PdlSet                Interrupt flag is set
 ** \retval PdlClr                Interrupt flag is clear
 ******************************************************************************/
en_int_flag_t Bt_Pwc_GetIntFlag(volatile stc_btn_t* pstcBt, en_bt_pwc_int_t enIntType)
{
    en_int_flag_t enFlag; 
    
    switch (enIntType)
    {
        case PwcMeasureCompleteInt:
            enFlag = ((FM4_BT_PWC_TypeDef*)pstcBt)->STC_f.EDIR ? PdlSet : PdlClr;
            break;
        case PwcMeasureOverflowInt:
            enFlag = ((FM4_BT_PWC_TypeDef*)pstcBt)->STC_f.OVIR ? PdlSet : PdlClr;
            break;
        default:
            break;
    }
    
    return enFlag;
}

/**
 ******************************************************************************
 ** \brief Clear interrupt flag of PWC timer 
 **
 ** \param [in]  pstcBt           BT instance
 ** \param [in]  enIntType        PWC timer interrupt type
 **
 ** \retval ErrorInvalidParameter pstcBt == NULL
 **                               enIntType > PwcMeasureOverflowInt
 ** \retval Ok                    Clear interrupt flag successfully
 ******************************************************************************/
en_result_t Bt_Pwc_ClrIntFlag(volatile stc_btn_t* pstcBt, en_bt_pwc_int_t enIntType)
{
    if((pstcBt == NULL) || (enIntType > PwcMeasureOverflowInt))
    {
        return ErrorInvalidParameter;
    }
  
    switch (enIntType)
    {
        case PwcMeasureCompleteInt:
            break;
        case PwcMeasureOverflowInt:
            ((FM4_BT_PWC_TypeDef*)pstcBt)->STC_f.OVIR = 0;
            break;
        default:
            break;
    }
    
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Get error flag of PWC timer
 **
 ** If the measured data is not read out when the next data is coming,
 ** the error flag will be set.
 **
 ** \param [in]  pstcBt           BT instance
 **
 ** \retval PdlSet                Error flag is set
 ** \retval PdlClr                Error flag is clear
 ******************************************************************************/
en_stat_flag_t Bt_Pwc_GetErrorFlag(volatile stc_btn_t* pstcBt)
{
    en_int_flag_t enFlag; 
    
    enFlag = ((FM4_BT_PWC_TypeDef*)pstcBt)->STC_f.ERR ? PdlSet : PdlClr;
    
    return enFlag;
}

/**
 ******************************************************************************
 ** \brief Get 16 bits measure data of PWC timer
 **
 ** This function reads measure data in 16-bit timer mode.
 **
 ** \param [in]  pstcBt           BT instance
 **
 ** \retval Measure data       
 ******************************************************************************/
uint16_t Bt_Pwc_Get16BitMeasureData(volatile stc_btn_t* pstcBt)
{ 
    uint16_t u16RegDtbf;
    u16RegDtbf = ((FM4_BT_PWC_TypeDef*)pstcBt)->DTBF;
    return u16RegDtbf;
}

/**
 ******************************************************************************
 ** \brief Get 32 bits measure data of PWC timer
 **
 ** This function reads measure data in 32-bit timer mode, the input BT instance
 ** should point to register base address of even channel. 
 **
 ** \param [in]  pstcBt           BT instance
 **
 ** \retval Measure data 
 ******************************************************************************/
uint32_t Bt_Pwc_Get32BitMeasureData(volatile stc_btn_t* pstcBt)
{ 
    uint16_t u16RegLow, u16RegHigh;
    
    if((pstcBt == &BT1) || (pstcBt == &BT3) || (pstcBt == &BT5) || (pstcBt == &BT7) ||
       (pstcBt == &BT9) || (pstcBt == &BT11) || (pstcBt == &BT13) || (pstcBt == &BT15))
        return 0;
    u16RegLow = ((FM4_BT_PWC_TypeDef*)pstcBt)->DTBF;
    u16RegHigh = ((FM4_BT_PWC_TypeDef*)((uint8_t*)pstcBt + 0x40))->DTBF;
    return ((u16RegHigh<<16) | u16RegLow);
}

/**
 ******************************************************************************
 ** \brief Set the Simultaneous Start register of Base timer
 **
 ** This function is only valid in BTIO mode 5 and mode 6, otherwise don't 
 ** use this function!
 **
 ** \param [in]  u16Value           Bit0 ----- Ch.0
 **                                 Bit1 ----- Ch.1
 **                                 ...
 **                                 Bit15 ----- Ch.15
 **
 ** \retval None
 ******************************************************************************/
void Bt_SetSimultaneousStart(uint16_t u16Value)
{
    FM4_SBSSR->BTSSSR = u16Value;
}

//@}

#endif

/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
