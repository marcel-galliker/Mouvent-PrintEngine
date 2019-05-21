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
/** \file interrupts.c
 **
 ** PDL Interrupt Handler
 **
 ** A detailed description is available at 
 ** @link PdlInterrupts PDL Interrupt handling description @endlink
 **
 ** History:
 **   - 2013-03-21  0.0.1  MWi        First version.
 **   - 2013-08-20  0.0.2  MWi        DSTC IRQ-Handlers added
 **   - 2014-01-15  0.0.3  EZ         Correct a bug in MFS IRQ handler.
 **
 ******************************************************************************/

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "pdl.h"
#include "interrupts.h"

/******************************************************************************/
/*********************************** NMI **************************************/
/******************************************************************************/
#if (PDL_PERIPHERAL_ENABLE_NMI == PDL_ON) || (PDL_PERIPHERAL_ENABLE_HWWDG == PDL_ON)
void NMI_Handler(void)
{
#if (PDL_PERIPHERAL_ENABLE_NMI == PDL_ON)
#endif
#if (PDL_PERIPHERAL_ENABLE_HWWDG == PDL_ON)
    if (FM4_HWWDT->WDG_RIS_f.RIS == 1u)
    {
        HwwdgIrqHandler();
    }
#endif
}
#endif

/******************************************************************************/
/******************************* SW watchdog **********************************/
/******************************************************************************/
#if (PDL_PERIPHERAL_ENABLE_SWWDG == PDL_ON) && (PDL_INTERRUPT_ENABLE_SWWDG == PDL_ON)
void SWDT_IRQHandler(void)
{
    SwwdgIrqHandler();
}
#endif

/******************************************************************************/
/*********************************** ADC **************************************/
/******************************************************************************/
#if (PDL_PERIPHERAL_ENABLE_ADC0 == PDL_ON) && (PDL_INTERRUPT_ENABLE_ADC0 == PDL_ON)
void ADC0_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_ADC0_PRIO) || (PDL_ON == PDL_DSTC_ENABLE_ADC0_SCAN)
    Dstc_AdcIrqHandler(DSTC_IRQ_NUMBER_ADC0_PRIO, DSTC_IRQ_NUMBER_ADC0_SCAN);
  #else  
  AdcIrqHandler((stc_adcn_t*)&ADC0, &(m_astcAdcInstanceDataLut[AdcInstanceIndexAdc0].stcInternData));
  #endif
}
#endif

#if (PDL_PERIPHERAL_ENABLE_ADC1 == PDL_ON) && (PDL_INTERRUPT_ENABLE_ADC1 == PDL_ON)
void ADC1_IRQHandler(void)
{
  AdcIrqHandler((stc_adcn_t*)&ADC1, &(m_astcAdcInstanceDataLut[AdcInstanceIndexAdc1].stcInternData));
}
#endif

#if (PDL_PERIPHERAL_ENABLE_ADC2 == PDL_ON) && (PDL_INTERRUPT_ENABLE_ADC2 == PDL_ON)
void ADC2_IRQHandler(void)
{
  AdcIrqHandler((stc_adcn_t*)&ADC2, &(m_astcAdcInstanceDataLut[AdcInstanceIndexAdc2].stcInternData));
}
#endif

/******************************************************************************/
/******************************* Dual Timer ***********************************/
/******************************************************************************/
#if (PDL_PERIPHERAL_ENABLE_DT0 == PDL_ON) && (PDL_INTERRUPT_ENABLE_DT0 == PDL_ON)
void DT1_2_IRQHandler(void)
{
    uint32_t u32IrqMon = FM4_INTREQ->IRQ047MON;

    if (0 != (u32IrqMon & 0x00000001u))
    {
        DtIrqHandler(DtChannel0);
    }
    if (0 != (u32IrqMon & 0x00000002u))
    {
        DtIrqHandler(DtChannel1);
    }
}
#endif

/******************************************************************************/
/*********************************** CAN **************************************/
/******************************************************************************/

#if (PDL_PERIPHERAL_ENABLE_CAN0 == PDL_ON)
void CAN0_IRQHandler(void)
{
  CanIrqHandler((stc_cann_t*)&CAN0, &(m_astcCanInstanceDataLut[CanInstanceIndexCan0].stcInternData));
}
#endif

#if (PDL_PERIPHERAL_ENABLE_CAN1 == PDL_ON)
void CAN1_IRQHandler(void)
{
  CanIrqHandler((stc_cann_t*)&CAN1, &(m_astcCanInstanceDataLut[CanInstanceIndexCan1].stcInternData));
}
#endif

/******************************************************************************/
/*********************************** CLK **************************************/
/******************************************************************************/

#if (PDL_INTERRUPT_ENABLE_CLK == PDL_ON)
void TIM_IRQHandler(void)
{
  ClkIrqHandler();
}
#endif

/******************************************************************************/
/*********************************** CSV **************************************/
/******************************************************************************/

#if (PDL_INTERRUPT_ENABLE_CSV == PDL_ON)
void CSV_IRQHandler(void)
{
  Csv_IrqHandler();
}
#endif

/******************************************************************************/
/*********************************** DMA **************************************/
/******************************************************************************/

#if (PDL_INTERRUPT_ENABLE_DMA0 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_DMA0 == PDL_ON)
/**
 ******************************************************************************
 ** \brief DMA ch. 0 IRQ handler
 ******************************************************************************
 */
void DMAC0_IRQHandler(void)
{
    DmaIrqHandler(0u);
}
#endif // #if (PDL_INTERRUPT_ENABLE_DMA0 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_DMA0 == PDL_ON)

#if (PDL_INTERRUPT_ENABLE_DMA1 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_DMA1 == PDL_ON)
/**
 ******************************************************************************
 ** \brief DMA ch. 1 IRQ handler
 ******************************************************************************
 */
void DMAC1_IRQHandler(void)
{
    DmaIrqHandler(1u);
}
#endif // #if (PDL_INTERRUPT_ENABLE_DMA1 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_DMA1 == PDL_ON)

#if (PDL_INTERRUPT_ENABLE_DMA2 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_DMA2 == PDL_ON)
/**
 ******************************************************************************
 ** \brief DMA ch. 2 IRQ handler
 ******************************************************************************
 */
void DMAC2_IRQHandler(void)
{
    DmaIrqHandler(2u);
}
#endif // #if (PDL_INTERRUPT_ENABLE_DMA2 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_DMA2 == PDL_ON)

#if (PDL_INTERRUPT_ENABLE_DMA3 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_DMA3 == PDL_ON)
/**
 ******************************************************************************
 ** \brief DMA ch. 3 IRQ handler
 ******************************************************************************
 */
void DMAC3_IRQHandler(void)
{
    DmaIrqHandler(3u);
}
#endif // #if (PDL_INTERRUPT_ENABLE_DMA3 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_DMA3 == PDL_ON)

#if (PDL_INTERRUPT_ENABLE_DMA4 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_DMA4 == PDL_ON)
/**
 ******************************************************************************
 ** \brief DMA ch. 4 IRQ handler
 ******************************************************************************
 */
void DMAC4_IRQHandler(void)
{
    DmaIrqHandler(4u);
}
#endif // #if (PDL_INTERRUPT_ENABLE_DMA4 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_DMA4 == PDL_ON)

#if (PDL_INTERRUPT_ENABLE_DMA5 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_DMA5 == PDL_ON)
/**
 ******************************************************************************
 ** \brief DMA ch. 5 IRQ handler
 ******************************************************************************
 */
void DMAC5_IRQHandler(void)
{
    DmaIrqHandler(5u);
}
#endif // #if (PDL_INTERRUPT_ENABLE_DMA5 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_DMA5 == PDL_ON)

#if (PDL_INTERRUPT_ENABLE_DMA6 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_DMA6 == PDL_ON)
/**
 ******************************************************************************
 ** \brief DMA ch. 6 IRQ handler
 ******************************************************************************
 */
void DMAC6_IRQHandler(void)
{
    DmaIrqHandler(6u);
}
#endif // #if (PDL_INTERRUPT_ENABLE_DMA6 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_DMA6 == PDL_ON)

#if (PDL_INTERRUPT_ENABLE_DMA7 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_DMA7 == PDL_ON)
/**
 ******************************************************************************
 ** \brief DMA ch. 7 IRQ handler
 ******************************************************************************
 */
void DMAC7_IRQHandler(void)
{
    DmaIrqHandler(7u);
}
#endif // #if (PDL_INTERRUPT_ENABLE_DMA7 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_DMA7 == PDL_ON)

/******************************************************************************/
/*********************************** LVD **************************************/
/******************************************************************************/
#if (PDL_INTERRUPT_ENABLE_LVD == PDL_ON) && (PDL_PERIPHERAL_ENABLE_LVD == PDL_ON)
/**
 ******************************************************************************
 ** \brief LVD IRQ handler
 ******************************************************************************
 */
void LVD_IRQHandler(void)
{
    LvdIrqHandler();
}
#endif // #if (PDL_INTERRUPT_ENABLE_LVD == PDL_ON) && (PDL_PERIPHERAL_ENABLE_LVD == PDL_ON)

/******************************************************************************/
/*********************************** BT **************************************/
/******************************************************************************/

#if (PDL_INTERRUPT_ENABLE_BT0 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_BT0 == PDL_ON)
/**
 ******************************************************************************
 ** \brief BT ch. 0 IRQ handler
 ******************************************************************************
 */
void BT0_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_BT0_IRQ0) || (PDL_ON == PDL_DSTC_ENABLE_BT0_IRQ1)
    Dstc_BtIrqHandler(DSTC_IRQ_NUMBER_BT0_IRQ0, DSTC_IRQ_NUMBER_BT0_IRQ1);
  #else  
    Bt_IrqHandler((stc_btn_t*)&BT0, &m_astcBtInstanceDataLut[BtInstanceIndexBt0].stcInternData);
  #endif
}
#endif

#if (PDL_INTERRUPT_ENABLE_BT1 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_BT1 == PDL_ON)
/**
 ******************************************************************************
 ** \brief BT ch. 1 IRQ handler
 ******************************************************************************
 */
void BT1_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_BT1_IRQ0) || (PDL_ON == PDL_DSTC_ENABLE_BT1_IRQ1)
    Dstc_BtIrqHandler(DSTC_IRQ_NUMBER_BT1_IRQ0, DSTC_IRQ_NUMBER_BT1_IRQ1);
  #else 
    Bt_IrqHandler((stc_btn_t*)&BT1, &m_astcBtInstanceDataLut[BtInstanceIndexBt1].stcInternData);
  #endif
}
#endif

#if (PDL_INTERRUPT_ENABLE_BT2 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_BT2 == PDL_ON)
/**
 ******************************************************************************
 ** \brief BT ch. 2 IRQ handler
 ******************************************************************************
 */
void BT2_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_BT2_IRQ0) || (PDL_ON == PDL_DSTC_ENABLE_BT2_IRQ1)
    Dstc_BtIrqHandler(DSTC_IRQ_NUMBER_BT2_IRQ0, DSTC_IRQ_NUMBER_BT2_IRQ1);
  #else 
    Bt_IrqHandler((stc_btn_t*)&BT2, &m_astcBtInstanceDataLut[BtInstanceIndexBt2].stcInternData);
  #endif
}
#endif

#if (PDL_INTERRUPT_ENABLE_BT3 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_BT3 == PDL_ON)
/**
 ******************************************************************************
 ** \brief BT ch. 3 IRQ handler
 ******************************************************************************
 */
void BT3_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_BT3_IRQ0) || (PDL_ON == PDL_DSTC_ENABLE_BT3_IRQ1)
    Dstc_BtIrqHandler(DSTC_IRQ_NUMBER_BT3_IRQ0, DSTC_IRQ_NUMBER_BT3_IRQ1);
  #else 
    Bt_IrqHandler((stc_btn_t*)&BT3, &m_astcBtInstanceDataLut[BtInstanceIndexBt3].stcInternData);
  #endif
}
#endif

#if (PDL_INTERRUPT_ENABLE_BT4 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_BT4 == PDL_ON)
/**
 ******************************************************************************
 ** \brief BT ch. 4 IRQ handler
 ******************************************************************************
 */
void BT4_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_BT4_IRQ0) || (PDL_ON == PDL_DSTC_ENABLE_BT4_IRQ1)
    Dstc_BtIrqHandler(DSTC_IRQ_NUMBER_BT4_IRQ0, DSTC_IRQ_NUMBER_BT4_IRQ1);
  #else 
    Bt_IrqHandler((stc_btn_t*)&BT4, &m_astcBtInstanceDataLut[BtInstanceIndexBt4].stcInternData);
  #endif
}
#endif

#if (PDL_INTERRUPT_ENABLE_BT5 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_BT5 == PDL_ON)
/**
 ******************************************************************************
 ** \brief BT ch. 5 IRQ handler
 ******************************************************************************
 */
void BT5_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_BT5_IRQ0) || (PDL_ON == PDL_DSTC_ENABLE_BT5_IRQ1)
    Dstc_BtIrqHandler(DSTC_IRQ_NUMBER_BT5_IRQ0, DSTC_IRQ_NUMBER_BT5_IRQ1);
  #else 
    Bt_IrqHandler((stc_btn_t*)&BT5, &m_astcBtInstanceDataLut[BtInstanceIndexBt5].stcInternData);
  #endif
}
#endif

#if (PDL_INTERRUPT_ENABLE_BT6 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_BT6 == PDL_ON)
/**
 ******************************************************************************
 ** \brief BT ch. 6 IRQ handler
 ******************************************************************************
 */
void BT6_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_BT6_IRQ0) || (PDL_ON == PDL_DSTC_ENABLE_BT6_IRQ1)
    Dstc_BtIrqHandler(DSTC_IRQ_NUMBER_BT6_IRQ0, DSTC_IRQ_NUMBER_BT6_IRQ1);
  #else 
    Bt_IrqHandler((stc_btn_t*)&BT6, &m_astcBtInstanceDataLut[BtInstanceIndexBt6].stcInternData);
  #endif
}
#endif

#if (PDL_INTERRUPT_ENABLE_BT7 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_BT7 == PDL_ON)
/**
 ******************************************************************************
 ** \brief BT ch. 7 IRQ handler
 ******************************************************************************
 */
void BT7_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_BT7_IRQ0) || (PDL_ON == PDL_DSTC_ENABLE_BT7_IRQ1)
    Dstc_BtIrqHandler(DSTC_IRQ_NUMBER_BT7_IRQ0, DSTC_IRQ_NUMBER_BT7_IRQ1);
  #else 
    Bt_IrqHandler((stc_btn_t*)&BT7, &m_astcBtInstanceDataLut[BtInstanceIndexBt7].stcInternData);
  #endif
}
#endif

#if (PDL_INTERRUPT_ENABLE_BT8 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_BT8 == PDL_ON)
/**
 ******************************************************************************
 ** \brief BT ch. 8 IRQ handler
 ******************************************************************************
 */
void BT8_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_BT8_IRQ0) || (PDL_ON == PDL_DSTC_ENABLE_BT8_IRQ1)
    Dstc_BtIrqHandler(DSTC_IRQ_NUMBER_BT8_IRQ0, DSTC_IRQ_NUMBER_BT8_IRQ1);
  #else 
    Bt_IrqHandler((stc_btn_t*)&BT8, &m_astcBtInstanceDataLut[BtInstanceIndexBt8].stcInternData);
  #endif
}
#endif

#if (PDL_INTERRUPT_ENABLE_BT9 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_BT9 == PDL_ON)
/**
 ******************************************************************************
 ** \brief BT ch. 9 IRQ handler
 ******************************************************************************
 */
void BT9_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_BT9_IRQ0) || (PDL_ON == PDL_DSTC_ENABLE_BT9_IRQ1)
    Dstc_BtIrqHandler(DSTC_IRQ_NUMBER_BT9_IRQ0, DSTC_IRQ_NUMBER_BT9_IRQ1);
  #else 
    Bt_IrqHandler((stc_btn_t*)&BT9, &m_astcBtInstanceDataLut[BtInstanceIndexBt9].stcInternData);
  #endif
}
#endif

#if (PDL_INTERRUPT_ENABLE_BT10 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_BT10 == PDL_ON)
/**
 ******************************************************************************
 ** \brief BT ch. 10 IRQ handler
 ******************************************************************************
 */
void BT10_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_BT10_IRQ0) || (PDL_ON == PDL_DSTC_ENABLE_BT10_IRQ1)
    Dstc_BtIrqHandler(DSTC_IRQ_NUMBER_BT10_IRQ0, DSTC_IRQ_NUMBER_BT10_IRQ1);
  #else 
    Bt_IrqHandler((stc_btn_t*)&BT10, &m_astcBtInstanceDataLut[BtInstanceIndexBt10].stcInternData);
  #endif
}
#endif

#if (PDL_INTERRUPT_ENABLE_BT11 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_BT11 == PDL_ON)
/**
 ******************************************************************************
 ** \brief BT ch. 11 IRQ handler
 ******************************************************************************
 */
void BT11_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_BT11_IRQ0) || (PDL_ON == PDL_DSTC_ENABLE_BT11_IRQ1)
    Dstc_BtIrqHandler(DSTC_IRQ_NUMBER_BT11_IRQ0, DSTC_IRQ_NUMBER_BT11_IRQ1);
  #else 
    Bt_IrqHandler((stc_btn_t*)&BT11, &m_astcBtInstanceDataLut[BtInstanceIndexBt11].stcInternData);
  #endif
}
#endif

#if ((PDL_INTERRUPT_ENABLE_BT12 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_BT12 == PDL_ON)) || \
    ((PDL_INTERRUPT_ENABLE_BT13 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_BT13 == PDL_ON)) || \
    ((PDL_INTERRUPT_ENABLE_BT14 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_BT14 == PDL_ON)) || \
    ((PDL_INTERRUPT_ENABLE_BT15 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_BT15 == PDL_ON))
/**
 ******************************************************************************
 ** \brief BT ch. 12, 13, 14, 15 IRQ handler
 ******************************************************************************
 */
void BT12_13_14_15_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_BT12_IRQ0) || (PDL_ON == PDL_DSTC_ENABLE_BT12_IRQ1)
    if ((TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_BT12_IRQ0)) || (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_BT12_IRQ1)))
    {
      Dstc_BtIrqHandler(DSTC_IRQ_NUMBER_BT12_IRQ0, DSTC_IRQ_NUMBER_BT12_IRQ1);
    }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_BT13_IRQ0) || (PDL_ON == PDL_DSTC_ENABLE_BT13_IRQ1)
    if ((TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_BT13_IRQ0)) || (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_BT13_IRQ1)))
    {
      Dstc_BtIrqHandler(DSTC_IRQ_NUMBER_BT13_IRQ0, DSTC_IRQ_NUMBER_BT13_IRQ1);
    }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_BT14_IRQ0) || (PDL_ON == PDL_DSTC_ENABLE_BT14_IRQ1)
    if ((TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_BT14_IRQ0)) || (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_BT14_IRQ1)))
    {
      Dstc_BtIrqHandler(DSTC_IRQ_NUMBER_BT14_IRQ0, DSTC_IRQ_NUMBER_BT14_IRQ1);
    }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_BT15_IRQ0) || (PDL_ON == PDL_DSTC_ENABLE_BT15_IRQ1)
    if ((TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_BT15_IRQ0)) || (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_BT15_IRQ1)))
    {
      Dstc_BtIrqHandler(DSTC_IRQ_NUMBER_BT15_IRQ0, DSTC_IRQ_NUMBER_BT15_IRQ1);
    }
  #endif

  #if (PDL_ON != PDL_DSTC_ENABLE_BT12_IRQ0) && (PDL_ON != PDL_DSTC_ENABLE_BT12_IRQ1) && \
      (PDL_ON != PDL_DSTC_ENABLE_BT13_IRQ0) && (PDL_ON != PDL_DSTC_ENABLE_BT13_IRQ1) && \
      (PDL_ON != PDL_DSTC_ENABLE_BT14_IRQ0) && (PDL_ON != PDL_DSTC_ENABLE_BT14_IRQ1) && \
      (PDL_ON != PDL_DSTC_ENABLE_BT15_IRQ0) && (PDL_ON != PDL_DSTC_ENABLE_BT15_IRQ1)
    #if ((PDL_INTERRUPT_ENABLE_BT12 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_BT12 == PDL_ON)) 
    if (0u != ((FM4_IRQMON->IRQ102MON) & 0x3u))
    {
        Bt_IrqHandler((stc_btn_t*)&BT12, &m_astcBtInstanceDataLut[BtInstanceIndexBt12].stcInternData);
    }
#endif
    #if ((PDL_INTERRUPT_ENABLE_BT13 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_BT13 == PDL_ON)) 
    if (0u != ((FM4_IRQMON->IRQ102MON) & 0xCu))
    {
      Bt_IrqHandler((stc_btn_t*)&BT13, &m_astcBtInstanceDataLut[BtInstanceIndexBt13].stcInternData);
    }
    #endif
    #if ((PDL_INTERRUPT_ENABLE_BT14 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_BT14 == PDL_ON)) 
    if (0u != ((FM4_IRQMON->IRQ102MON) & 0x30u))
    {
      Bt_IrqHandler((stc_btn_t*)&BT14, &m_astcBtInstanceDataLut[BtInstanceIndexBt14].stcInternData);
    }
    #endif
    #if ((PDL_INTERRUPT_ENABLE_BT15 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_BT15 == PDL_ON)) 
    if (0u != ((FM4_IRQMON->IRQ102MON) & 0xC0u))
    {
      Bt_IrqHandler((stc_btn_t*)&BT15, &m_astcBtInstanceDataLut[BtInstanceIndexBt15].stcInternData);
    }
    #endif
  #endif
}
#endif

/******************************************************************************/
/********************************** DSTC **************************************/
/******************************************************************************/
#if (PDL_INTERRUPT_ENABLE_DSTC == PDL_ON) && (PDL_PERIPHERAL_ENABLE_DSTC == PDL_ON)
/**
 ******************************************************************************
 ** \brief DSTC IRQ handler
 ******************************************************************************
 */
void DSTC_IRQHandler(void)
{
    Dstc_IrqHandler();
}
#endif

/******************************************************************************/
/********************************* EXINT **************************************/
/******************************************************************************/
#if (PDL_INTERRUPT_ENABLE_EXINT0 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT0 == PDL_ON)
/**
 ******************************************************************************
 ** \brief EXINT ch. 0 IRQ handler
 ******************************************************************************
 */
void EXINT0_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_EXINT0)
    Dstc_ExintIrqHandler(DSTC_IRQ_NUMBER_EXTINT0);
  #else  
    Exint_IrqHandler(0u);
  #endif
}
#endif

#if (PDL_INTERRUPT_ENABLE_EXINT1 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT1 == PDL_ON)
/**
 ******************************************************************************
 ** \brief EXINT ch. 1 IRQ handler
 ******************************************************************************
 */
void EXINT1_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_EXINT1)
    Dstc_ExintIrqHandler(DSTC_IRQ_NUMBER_EXTINT1);
  #else  
    Exint_IrqHandler(1u);
  #endif
}
#endif

#if (PDL_INTERRUPT_ENABLE_EXINT2 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT2 == PDL_ON)
/**
 ******************************************************************************
 ** \brief EXINT ch. 2 IRQ handler
 ******************************************************************************
 */
void EXINT2_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_EXINT2)
    Dstc_ExintIrqHandler(DSTC_IRQ_NUMBER_EXTINT2);
  #else  
    Exint_IrqHandler(2u);
  #endif
}
#endif

#if (PDL_INTERRUPT_ENABLE_EXINT3 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT3 == PDL_ON)
/**
 ******************************************************************************
 ** \brief EXINT ch. 3 IRQ handler
 ******************************************************************************
 */
void EXINT3_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_EXINT3)
    Dstc_ExintIrqHandler(DSTC_IRQ_NUMBER_EXTINT3);
  #else  
    Exint_IrqHandler(3u);
  #endif
}
#endif

#if (PDL_INTERRUPT_ENABLE_EXINT4 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT4 == PDL_ON)
/**
 ******************************************************************************
 ** \brief EXINT ch. 4 IRQ handler
 ******************************************************************************
 */
void EXINT4_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_EXINT4)
    Dstc_ExintIrqHandler(DSTC_IRQ_NUMBER_EXTINT4);
  #else  
    Exint_IrqHandler(4u);
  #endif
}
#endif

#if (PDL_INTERRUPT_ENABLE_EXINT5 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT5 == PDL_ON)
/**
 ******************************************************************************
 ** \brief EXINT ch. 5 IRQ handler
 ******************************************************************************
 */
void EXINT5_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_EXINT5)
    Dstc_ExintIrqHandler(DSTC_IRQ_NUMBER_EXTINT5);
  #else  
    Exint_IrqHandler(5u);
  #endif
}
#endif

#if (PDL_INTERRUPT_ENABLE_EXINT6 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT6 == PDL_ON)
/**
 ******************************************************************************
 ** \brief EXINT ch. 6 IRQ handler
 ******************************************************************************
 */
void EXINT6_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_EXINT6)
    Dstc_ExintIrqHandler(DSTC_IRQ_NUMBER_EXTINT6);
  #else  
    Exint_IrqHandler(6u);
  #endif
}
#endif

#if (PDL_INTERRUPT_ENABLE_EXINT7 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT7 == PDL_ON)
/**
 ******************************************************************************
 ** \brief EXINT ch. 7 IRQ handler
 ******************************************************************************
 */
void EXINT7_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_EXINT7)
    Dstc_ExintIrqHandler(DSTC_IRQ_NUMBER_EXTINT7);
  #else  
    Exint_IrqHandler(7u);
  #endif
}
#endif

#if (PDL_INTERRUPT_ENABLE_EXINT8 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT8 == PDL_ON)
/**
 ******************************************************************************
 ** \brief EXINT ch. 8 IRQ handler
 ******************************************************************************
 */
void EXINT8_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_EXINT8)
    Dstc_ExintIrqHandler(DSTC_IRQ_NUMBER_EXTINT8);
  #else  
    Exint_IrqHandler(8u);
  #endif
}
#endif

#if (PDL_INTERRUPT_ENABLE_EXINT9 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT9 == PDL_ON)
/**
 ******************************************************************************
 ** \brief EXINT ch. 9 IRQ handler
 ******************************************************************************
 */
void EXINT9_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_EXINT9)
    Dstc_ExintIrqHandler(DSTC_IRQ_NUMBER_EXTINT9);
  #else  
    Exint_IrqHandler(9u);
  #endif
}
#endif

#if (PDL_INTERRUPT_ENABLE_EXINT10 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT10 == PDL_ON)
/**
 ******************************************************************************
 ** \brief EXINT ch. 10 IRQ handler
 ******************************************************************************
 */
void EXINT10_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_EXINT10)
    Dstc_ExintIrqHandler(DSTC_IRQ_NUMBER_EXTINT10);
  #else  
    Exint_IrqHandler(10u);
  #endif
}
#endif

#if (PDL_INTERRUPT_ENABLE_EXINT11 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT11 == PDL_ON)
/**
 ******************************************************************************
 ** \brief EXINT ch. 11 IRQ handler
 ******************************************************************************
 */
void EXINT11_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_EXINT11)
    Dstc_ExintIrqHandler(DSTC_IRQ_NUMBER_EXTINT11);
  #else  
    Exint_IrqHandler(11u);
  #endif
}
#endif

#if (PDL_INTERRUPT_ENABLE_EXINT12 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT12 == PDL_ON)
/**
 ******************************************************************************
 ** \brief EXINT ch. 12 IRQ handler
 ******************************************************************************
 */
void EXINT12_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_EXINT12)
    Dstc_ExintIrqHandler(DSTC_IRQ_NUMBER_EXTINT12);
  #else  
    Exint_IrqHandler(12u);
  #endif
}
#endif

#if (PDL_INTERRUPT_ENABLE_EXINT13 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT13 == PDL_ON)
/**
 ******************************************************************************
 ** \brief EXINT ch. 13 IRQ handler
 ******************************************************************************
 */
void EXINT13_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_EXINT13)
    Dstc_ExintIrqHandler(DSTC_IRQ_NUMBER_EXTINT13);
  #else  
    Exint_IrqHandler(13u);
  #endif
}
#endif

#if (PDL_INTERRUPT_ENABLE_EXINT14 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT14 == PDL_ON)
/**
 ******************************************************************************
 ** \brief EXINT ch. 14 IRQ handler
 ******************************************************************************
 */
void EXINT14_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_EXINT14)
    Dstc_ExintIrqHandler(DSTC_IRQ_NUMBER_EXTINT14);
  #else  
    Exint_IrqHandler(14u);
  #endif
}
#endif

#if (PDL_INTERRUPT_ENABLE_EXINT15 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT15 == PDL_ON)
/**
 ******************************************************************************
 ** \brief EXINT ch. 15 IRQ handler
 ******************************************************************************
 */
void EXINT15_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_EXINT15)
    Dstc_ExintIrqHandler(DSTC_IRQ_NUMBER_EXTINT15);
  #else  
    Exint_IrqHandler(15u);
  #endif
}
#endif

#if ((PDL_INTERRUPT_ENABLE_EXINT16 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT16 == PDL_ON)) || \
    ((PDL_INTERRUPT_ENABLE_EXINT17 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT17 == PDL_ON)) || \
    ((PDL_INTERRUPT_ENABLE_EXINT18 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT18 == PDL_ON)) || \
    ((PDL_INTERRUPT_ENABLE_EXINT19 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT19 == PDL_ON))
/**
 ******************************************************************************
 ** \brief EXINT ch. 16, 17, 18, 19 IRQ handler
 ******************************************************************************
 */
void EXINT16_17_18_19_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_EXINT16)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_EXTINT16))
    {
      Dstc_ExintIrqHandler(DSTC_IRQ_NUMBER_EXTINT16);
    }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_EXINT17)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_EXTINT17))
    {
      Dstc_ExintIrqHandler(DSTC_IRQ_NUMBER_EXTINT17);
    }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_EXINT18)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_EXTINT18))
    {
      Dstc_ExintIrqHandler(DSTC_IRQ_NUMBER_EXTINT18);
    }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_EXINT19)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_EXTINT19))
    {
      Dstc_ExintIrqHandler(DSTC_IRQ_NUMBER_EXTINT19);
    }
  #endif
    
  #if (PDL_ON != PDL_DSTC_ENABLE_EXINT16) && (PDL_ON != PDL_DSTC_ENABLE_EXINT17) || \
      (PDL_ON != PDL_DSTC_ENABLE_EXINT18) && (PDL_ON != PDL_DSTC_ENABLE_EXINT19)
    #if ((PDL_INTERRUPT_ENABLE_EXINT16 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT16 == PDL_ON))
  if (0x1u == ((FM4_IRQMON->IRQ092MON) & 0x1u))
  {
    Exint_IrqHandler(16u);
  }
    #endif
    #if ((PDL_INTERRUPT_ENABLE_EXINT17 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT17 == PDL_ON))
  if (0x2u == ((FM4_IRQMON->IRQ092MON) & 0x2u))
  {
    Exint_IrqHandler(17u);
  }
    #endif
    #if ((PDL_INTERRUPT_ENABLE_EXINT18 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT18 == PDL_ON))
  if (0x4u == ((FM4_IRQMON->IRQ092MON) & 0x4u))
  {
    Exint_IrqHandler(18u);
  }  
    #endif
    #if ((PDL_INTERRUPT_ENABLE_EXINT19 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT19 == PDL_ON))
  if (0x8u == ((FM4_IRQMON->IRQ092MON) & 0x8u))
  {
    Exint_IrqHandler(19u);
  }  
    #endif
  #endif
}
#endif

#if ((PDL_INTERRUPT_ENABLE_EXINT20 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT20 == PDL_ON)) || \
    ((PDL_INTERRUPT_ENABLE_EXINT21 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT21 == PDL_ON)) || \
    ((PDL_INTERRUPT_ENABLE_EXINT22 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT22 == PDL_ON)) || \
    ((PDL_INTERRUPT_ENABLE_EXINT23 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT23 == PDL_ON))
/**
 ******************************************************************************
 ** \brief EXINT ch. 20, 21, 22, 23 IRQ handler
 ******************************************************************************
 */
void EXINT20_21_22_23_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_EXINT20)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_EXTINT20))
    {
      Dstc_ExintIrqHandler(DSTC_IRQ_NUMBER_EXTINT20);
    }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_EXINT21)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_EXTINT21))
    {
      Dstc_ExintIrqHandler(DSTC_IRQ_NUMBER_EXTINT21);
    }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_EXINT22)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_EXTINT22))
    {
      Dstc_ExintIrqHandler(DSTC_IRQ_NUMBER_EXTINT22);
    }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_EXINT23)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_EXTINT23))
    {
      Dstc_ExintIrqHandler(DSTC_IRQ_NUMBER_EXTINT23);
    }
  #endif
    
  #if (PDL_ON != PDL_DSTC_ENABLE_EXINT20) && (PDL_ON != PDL_DSTC_ENABLE_EXINT21) && \
      (PDL_ON != PDL_DSTC_ENABLE_EXINT22) && (PDL_ON != PDL_DSTC_ENABLE_EXINT23)
    #if ((PDL_INTERRUPT_ENABLE_EXINT20 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT20 == PDL_ON))
  if (0x1u == ((FM4_IRQMON->IRQ092MON) & 0x1u))
  {
    Exint_IrqHandler(20u);
  }
    #endif
    #if ((PDL_INTERRUPT_ENABLE_EXINT21 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT21 == PDL_ON))
  if (0x2u == ((FM4_IRQMON->IRQ093MON) & 0x2u))
  {
    Exint_IrqHandler(21u);
  }
    #endif
    #if ((PDL_INTERRUPT_ENABLE_EXINT22 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT22 == PDL_ON))
  if (0x4u == ((FM4_IRQMON->IRQ093MON) & 0x4u))
  {
    Exint_IrqHandler(22u);
  }  
    #endif
    #if ((PDL_INTERRUPT_ENABLE_EXINT23 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT23 == PDL_ON))
  if (0x8u == ((FM4_IRQMON->IRQ093MON) & 0x8u))
  {
    Exint_IrqHandler(23u);
  }  
    #endif
  #endif
}
#endif

#if ((PDL_INTERRUPT_ENABLE_EXINT24 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT24 == PDL_ON)) || \
    ((PDL_INTERRUPT_ENABLE_EXINT25 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT25 == PDL_ON)) || \
    ((PDL_INTERRUPT_ENABLE_EXINT26 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT26 == PDL_ON)) || \
    ((PDL_INTERRUPT_ENABLE_EXINT27 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT27 == PDL_ON))
/**
 ******************************************************************************
 ** \brief EXINT ch. 24, 25, 26, 27 IRQ handler
 ******************************************************************************
 */
void EXINT24_25_26_27_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_EXINT24)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_EXTINT24))
    {
      Dstc_ExintIrqHandler(DSTC_IRQ_NUMBER_EXTINT24);
    }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_EXINT25)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_EXTINT25))
    {
      Dstc_ExintIrqHandler(DSTC_IRQ_NUMBER_EXTINT25);
    }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_EXINT26)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_EXTINT26))
    {
      Dstc_ExintIrqHandler(DSTC_IRQ_NUMBER_EXTINT26);
    }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_EXINT27)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_EXTINT27))
    {
      Dstc_ExintIrqHandler(DSTC_IRQ_NUMBER_EXTINT27);
    }
  #endif
    
  #if (PDL_ON != PDL_DSTC_ENABLE_EXINT24) && (PDL_ON != PDL_DSTC_ENABLE_EXINT25) && \
      (PDL_ON != PDL_DSTC_ENABLE_EXINT26) && (PDL_ON != PDL_DSTC_ENABLE_EXINT27)
    #if ((PDL_INTERRUPT_ENABLE_EXINT24 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT24 == PDL_ON))
  if (0x1u == ((FM4_IRQMON->IRQ094MON) & 0x1u))
  {
    Exint_IrqHandler(24u);
  }
    #endif
    #if ((PDL_INTERRUPT_ENABLE_EXINT25 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT25 == PDL_ON))
  if (0x2u == ((FM4_IRQMON->IRQ094MON) & 0x2u))
  {
    Exint_IrqHandler(25u);
  }
    #endif
    #if ((PDL_INTERRUPT_ENABLE_EXINT26 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT26 == PDL_ON))
  if (0x4u == ((FM4_IRQMON->IRQ094MON) & 0x4u))
  {
    Exint_IrqHandler(26u);
  }  
    #endif
    #if ((PDL_INTERRUPT_ENABLE_EXINT27 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT27 == PDL_ON))
  if (0x8u == ((FM4_IRQMON->IRQ094MON) & 0x8u))
  {
    Exint_IrqHandler(27u);
  }  
    #endif
  #endif
}
#endif

#if ((PDL_INTERRUPT_ENABLE_EXINT28 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT28 == PDL_ON)) || \
    ((PDL_INTERRUPT_ENABLE_EXINT29 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT29 == PDL_ON)) || \
    ((PDL_INTERRUPT_ENABLE_EXINT30 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT30 == PDL_ON)) || \
    ((PDL_INTERRUPT_ENABLE_EXINT31 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT31 == PDL_ON))
/**
 ******************************************************************************
 ** \brief EXINT ch. 28, 29, 30, 31 IRQ handler
 ******************************************************************************
 */
void EXINT28_29_30_31_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_EXINT28)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_EXTINT28))
    {
      Dstc_ExintIrqHandler(DSTC_IRQ_NUMBER_EXTINT28);
    }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_EXINT29)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_EXTINT29))
    {
      Dstc_ExintIrqHandler(DSTC_IRQ_NUMBER_EXTINT29);
    }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_EXINT30)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_EXTINT30))
    {
      Dstc_ExintIrqHandler(DSTC_IRQ_NUMBER_EXTINT30);
    }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_EXINT31)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_EXTINT31))
    {
      Dstc_ExintIrqHandler(DSTC_IRQ_NUMBER_EXTINT31);
    }
  #endif
    
  #if (PDL_ON != PDL_DSTC_ENABLE_EXINT28) && (PDL_ON != PDL_DSTC_ENABLE_EXINT29) && \
      (PDL_ON != PDL_DSTC_ENABLE_EXINT30) && (PDL_ON != PDL_DSTC_ENABLE_EXINT31)
    #if ((PDL_INTERRUPT_ENABLE_EXINT28 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT28 == PDL_ON))
  if (0x1u == ((FM4_IRQMON->IRQ095MON) & 0x1u))
  {
    Exint_IrqHandler(28u);
  }
    #endif
    #if ((PDL_INTERRUPT_ENABLE_EXINT29 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT29 == PDL_ON))
  if (0x2u == ((FM4_IRQMON->IRQ095MON) & 0x2u))
  {
    Exint_IrqHandler(29u);
  }
    #endif
    #if ((PDL_INTERRUPT_ENABLE_EXINT30 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT30 == PDL_ON))
  if (0x4u == ((FM4_IRQMON->IRQ095MON) & 0x4u))
  {
    Exint_IrqHandler(30u);
  }  
    #endif
    #if ((PDL_INTERRUPT_ENABLE_EXINT31 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXINT31 == PDL_ON))
  if (0x8u == ((FM4_IRQMON->IRQ095MON) & 0x8u))
  {
    Exint_IrqHandler(31u);
  }  
    #endif
  #endif
}
#endif

/******************************************************************************/
/********************************** EXTIF *************************************/
/******************************************************************************/
#if (PDL_INTERRUPT_ENABLE_EXTIF == PDL_ON) && (PDL_PERIPHERAL_ENABLE_EXTIF == PDL_ON)
void EXTBUS_ERR_IRQHandler(void)
{
	ExtifIrqHandler();
}
#endif // 

/******************************************************************************/
/*********************************** MFT **************************************/
/******************************************************************************/
#if (PDL_INTERRUPT_ENABLE_MFT0_FRT == PDL_ON) && (PDL_PERIPHERAL_ENABLE_MFT0_FRT == PDL_ON)
/**
 ******************************************************************************
 ** \brief MFT0's FRT peak match IRQ handler
 ******************************************************************************
 */
void FRT0_PEAK_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT0_FRT0_PEAK)  
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT0_FRT0_PEAK))
    {  
      Dstc_MftFrtPeakIrqHandler(DSTC_IRQ_NUMBER_MFT0_FRT0_PEAK);
    }
  #endif  
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT0_FRT1_PEAK)  
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT0_FRT1_PEAK))
    {  
      Dstc_MftFrtPeakIrqHandler(DSTC_IRQ_NUMBER_MFT0_FRT1_PEAK);
    }
  #endif 
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT0_FRT2_PEAK)  
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT0_FRT2_PEAK))
    {  
      Dstc_MftFrtPeakIrqHandler(DSTC_IRQ_NUMBER_MFT0_FRT2_PEAK);
    }
  #endif 
  #if (PDL_ON != PDL_DSTC_ENABLE_MFT0_FRT0_PEAK) && \
      (PDL_ON != PDL_DSTC_ENABLE_MFT0_FRT1_PEAK) && \
      (PDL_ON != PDL_DSTC_ENABLE_MFT0_FRT2_PEAK)  
    Mft_Frt_IrqHandler((stc_mftn_frt_t*)&MFT0_FRT, &m_astcMftFrtInstanceDataLut[FrtInstanceIndexFrt0].stcInternData);
  #endif
}
/**
 ******************************************************************************
 ** \brief MFT0's zero match IRQ handler
 ******************************************************************************
 */
void FRT0_ZERO_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT0_FRT0_ZERO)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT0_FRT0_ZERO))
    {
      Dstc_MftFrtZeroIrqHandler(DSTC_IRQ_NUMBER_MFT0_FRT0_ZERO);
    }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT0_FRT1_ZERO)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT0_FRT1_ZERO))
    {
      Dstc_MftFrtZeroIrqHandler(DSTC_IRQ_NUMBER_MFT0_FRT1_ZERO);
    }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT0_FRT2_ZERO)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT0_FRT2_ZERO))
    {
      Dstc_MftFrtZeroIrqHandler(DSTC_IRQ_NUMBER_MFT0_FRT2_ZERO);
    }
  #endif
  #if (PDL_ON != PDL_DSTC_ENABLE_MFT0_FRT0_ZERO) && \
      (PDL_ON != PDL_DSTC_ENABLE_MFT0_FRT1_ZERO) && \
      (PDL_ON != PDL_DSTC_ENABLE_MFT0_FRT2_ZERO)
    Mft_Frt_IrqHandler((stc_mftn_frt_t*)&MFT0_FRT, &m_astcMftFrtInstanceDataLut[FrtInstanceIndexFrt0].stcInternData);
  #endif
}
#endif

#if (PDL_INTERRUPT_ENABLE_MFT1_FRT == PDL_ON) && (PDL_PERIPHERAL_ENABLE_MFT1_FRT == PDL_ON)
/**
 ******************************************************************************
 ** \brief MFT1's FRT peak match IRQ handler
 ******************************************************************************
 */
void FRT1_PEAK_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT1_FRT0_PEAK)  
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT1_FRT0_PEAK))
    {  
      Dstc_MftFrtPeakIrqHandler(DSTC_IRQ_NUMBER_MFT1_FRT0_PEAK);
    }
  #endif  
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT1_FRT1_PEAK)  
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT1_FRT1_PEAK))
    {  
      Dstc_MftFrtPeakIrqHandler(DSTC_IRQ_NUMBER_MFT1_FRT1_PEAK);
    }
  #endif 
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT1_FRT2_PEAK)  
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT1_FRT2_PEAK))
    {  
      Dstc_MftFrtPeakIrqHandler(DSTC_IRQ_NUMBER_MFT1_FRT2_PEAK);
    }
  #endif 
  #if (PDL_ON != PDL_DSTC_ENABLE_MFT1_FRT0_PEAK) && \
      (PDL_ON != PDL_DSTC_ENABLE_MFT1_FRT1_PEAK) && \
      (PDL_ON != PDL_DSTC_ENABLE_MFT1_FRT2_PEAK) 
    Mft_Frt_IrqHandler((stc_mftn_frt_t*)&MFT1_FRT, &m_astcMftFrtInstanceDataLut[FrtInstanceIndexFrt1].stcInternData);
  #endif
}
/**
 ******************************************************************************
 ** \brief MFT1's zero match IRQ handler
 ******************************************************************************
 */
void FRT1_ZERO_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT1_FRT0_ZERO)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT1_FRT0_ZERO))
    {
      Dstc_MftFrtZeroIrqHandler(DSTC_IRQ_NUMBER_MFT1_FRT0_ZERO);
    }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT1_FRT1_ZERO)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT1_FRT1_ZERO))
    {
      Dstc_MftFrtZeroIrqHandler(DSTC_IRQ_NUMBER_MFT1_FRT1_ZERO);
    }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT1_FRT2_ZERO)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT1_FRT2_ZERO))
    {
      Dstc_MftFrtZeroIrqHandler(DSTC_IRQ_NUMBER_MFT1_FRT2_ZERO);
    }
  #endif
  #if (PDL_ON != PDL_DSTC_ENABLE_MFT1_FRT0_ZERO) && \
      (PDL_ON != PDL_DSTC_ENABLE_MFT1_FRT1_ZERO) && \
      (PDL_ON != PDL_DSTC_ENABLE_MFT1_FRT2_ZERO)
    Mft_Frt_IrqHandler((stc_mftn_frt_t*)&MFT1_FRT, &m_astcMftFrtInstanceDataLut[FrtInstanceIndexFrt1].stcInternData);
  #endif
}
#endif

#if (PDL_INTERRUPT_ENABLE_MFT2_FRT == PDL_ON) && (PDL_PERIPHERAL_ENABLE_MFT2_FRT == PDL_ON)
/**
 ******************************************************************************
 ** \brief MFT2's FRT peak match IRQ handler
 ******************************************************************************
 */
void FRT2_PEAK_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT2_FRT0_PEAK)  
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT2_FRT0_PEAK))
    {  
      Dstc_MftFrtPeakIrqHandler(DSTC_IRQ_NUMBER_MFT2_FRT0_PEAK);
    }
  #endif  
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT2_FRT1_PEAK)  
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT2_FRT1_PEAK))
    {  
      Dstc_MftFrtPeakIrqHandler(DSTC_IRQ_NUMBER_MFT2_FRT1_PEAK);
    }
  #endif 
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT2_FRT2_PEAK)  
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT2_FRT2_PEAK))
    {  
      Dstc_MftFrtPeakIrqHandler(DSTC_IRQ_NUMBER_MFT2_FRT2_PEAK);
    }
  #endif 
  #if (PDL_ON != PDL_DSTC_ENABLE_MFT2_FRT0_PEAK) && \
      (PDL_ON != PDL_DSTC_ENABLE_MFT2_FRT1_PEAK) && \
      (PDL_ON != PDL_DSTC_ENABLE_MFT2_FRT2_PEAK)  
    Mft_Frt_IrqHandler((stc_mftn_frt_t*)&MFT2_FRT, &m_astcMftFrtInstanceDataLut[FrtInstanceIndexFrt2].stcInternData);
  #endif
}
/**
 ******************************************************************************
 ** \brief MFT2's zero match IRQ handler
 ******************************************************************************
 */
void FRT2_ZERO_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT2_FRT0_ZERO)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT2_FRT0_ZERO))
    {
      Dstc_MftFrtZeroIrqHandler(DSTC_IRQ_NUMBER_MFT2_FRT0_ZERO);
    }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT2_FRT1_ZERO)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT2_FRT1_ZERO))
    {
      Dstc_MftFrtZeroIrqHandler(DSTC_IRQ_NUMBER_MFT2_FRT1_ZERO);
    }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT2_FRT2_ZERO)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT2_FRT2_ZERO))
    {
      Dstc_MftFrtZeroIrqHandler(DSTC_IRQ_NUMBER_MFT2_FRT2_ZERO);
    }
  #endif
  #if (PDL_ON != PDL_DSTC_ENABLE_MFT2_FRT0_ZERO) && \
      (PDL_ON != PDL_DSTC_ENABLE_MFT2_FRT1_ZERO) && \
      (PDL_ON != PDL_DSTC_ENABLE_MFT2_FRT2_ZERO)  
    Mft_Frt_IrqHandler((stc_mftn_frt_t*)&MFT2_FRT, &m_astcMftFrtInstanceDataLut[FrtInstanceIndexFrt2].stcInternData);
  #endif
}
#endif

#if (PDL_INTERRUPT_ENABLE_MFT0_OCU == PDL_ON) && (PDL_PERIPHERAL_ENABLE_MFT0_OCU == PDL_ON)
/**
 ******************************************************************************
 ** \brief MFT0's OCU IRQ handler
 ******************************************************************************
 */
void OCU0_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT0_OCU0)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT0_OCU0))
    {
      Dstc_MftOcuIrqHandler(DSTC_IRQ_NUMBER_MFT0_OCU0);
    }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT0_OCU1)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT0_OCU1))
    {
      Dstc_MftOcuIrqHandler(DSTC_IRQ_NUMBER_MFT0_OCU1);
    }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT0_OCU2)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT0_OCU2))
    {
      Dstc_MftOcuIrqHandler(DSTC_IRQ_NUMBER_MFT0_OCU2);
    }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT0_OCU3)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT0_OCU3))
    {
      Dstc_MftOcuIrqHandler(DSTC_IRQ_NUMBER_MFT0_OCU3);
    }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT0_OCU4)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT0_OCU4))
    {
      Dstc_MftOcuIrqHandler(DSTC_IRQ_NUMBER_MFT0_OCU4);
    }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT0_OCU5)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT0_OCU5))
    {
      Dstc_MftOcuIrqHandler(DSTC_IRQ_NUMBER_MFT0_OCU5);
    }
  #endif
  #if (PDL_ON != PDL_DSTC_ENABLE_MFT0_OCU0) && \
      (PDL_ON != PDL_DSTC_ENABLE_MFT0_OCU1) && \
      (PDL_ON != PDL_DSTC_ENABLE_MFT0_OCU2) && \
      (PDL_ON != PDL_DSTC_ENABLE_MFT0_OCU3) && \
      (PDL_ON != PDL_DSTC_ENABLE_MFT0_OCU4) && \
      (PDL_ON != PDL_DSTC_ENABLE_MFT0_OCU5)
    Mft_Ocu_IrqHandler((stc_mftn_ocu_t*)&MFT0_OCU, &m_astcMftOcuInstanceDataLut[OcuInstanceIndexOcu0].stcInternData);
  #endif
}
#endif

#if (PDL_INTERRUPT_ENABLE_MFT1_OCU == PDL_ON) && (PDL_PERIPHERAL_ENABLE_MFT1_OCU == PDL_ON)
/**
 ******************************************************************************
 ** \brief MFT1's OCU IRQ handler
 ******************************************************************************
 */
void OCU1_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT1_OCU0)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT1_OCU0))
    {
      Dstc_MftOcuIrqHandler(DSTC_IRQ_NUMBER_MFT1_OCU0);
    }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT1_OCU1)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT1_OCU1))
    {
      Dstc_MftOcuIrqHandler(DSTC_IRQ_NUMBER_MFT1_OCU1);
    }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT1_OCU2)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT1_OCU2))
    {
      Dstc_MftOcuIrqHandler(DSTC_IRQ_NUMBER_MFT1_OCU2);
    }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT1_OCU3)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT1_OCU3))
    {
      Dstc_MftOcuIrqHandler(DSTC_IRQ_NUMBER_MFT1_OCU3);
    }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT1_OCU4)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT1_OCU4))
    {
      Dstc_MftOcuIrqHandler(DSTC_IRQ_NUMBER_MFT1_OCU4);
    }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT1_OCU5)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT1_OCU5))
    {
      Dstc_MftOcuIrqHandler(DSTC_IRQ_NUMBER_MFT1_OCU5);
    }
  #endif
  #if (PDL_ON != PDL_DSTC_ENABLE_MFT1_OCU0) && \
      (PDL_ON != PDL_DSTC_ENABLE_MFT1_OCU1) && \
      (PDL_ON != PDL_DSTC_ENABLE_MFT1_OCU2) && \
      (PDL_ON != PDL_DSTC_ENABLE_MFT1_OCU3) && \
      (PDL_ON != PDL_DSTC_ENABLE_MFT1_OCU4) && \
      (PDL_ON != PDL_DSTC_ENABLE_MFT1_OCU5)
    Mft_Ocu_IrqHandler((stc_mftn_ocu_t*)&MFT1_OCU, &m_astcMftOcuInstanceDataLut[OcuInstanceIndexOcu1].stcInternData);
  #endif
}
#endif

#if (PDL_INTERRUPT_ENABLE_MFT2_OCU == PDL_ON) && (PDL_PERIPHERAL_ENABLE_MFT2_OCU == PDL_ON)
/**
 ******************************************************************************
 ** \brief MFT2's OCU IRQ handler
 ******************************************************************************
 */
void OCU2_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT2_OCU0)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT2_OCU0))
    {
      Dstc_MftOcuIrqHandler(DSTC_IRQ_NUMBER_MFT2_OCU0);
    }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT2_OCU1)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT2_OCU1))
    {
      Dstc_MftOcuIrqHandler(DSTC_IRQ_NUMBER_MFT2_OCU1);
    }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT2_OCU2)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT2_OCU2))
    {
      Dstc_MftOcuIrqHandler(DSTC_IRQ_NUMBER_MFT2_OCU2);
    }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT2_OCU3)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT2_OCU3))
    {
      Dstc_MftOcuIrqHandler(DSTC_IRQ_NUMBER_MFT2_OCU3);
    }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT2_OCU4)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT2_OCU4))
    {
      Dstc_MftOcuIrqHandler(DSTC_IRQ_NUMBER_MFT2_OCU4);
    }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT2_OCU5)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT2_OCU5))
    {
      Dstc_MftOcuIrqHandler(DSTC_IRQ_NUMBER_MFT2_OCU5);
    }
  #endif
  #if (PDL_ON != PDL_DSTC_ENABLE_MFT2_OCU0) && \
      (PDL_ON != PDL_DSTC_ENABLE_MFT2_OCU1) && \
      (PDL_ON != PDL_DSTC_ENABLE_MFT2_OCU2) && \
      (PDL_ON != PDL_DSTC_ENABLE_MFT2_OCU3) && \
      (PDL_ON != PDL_DSTC_ENABLE_MFT2_OCU4) && \
      (PDL_ON != PDL_DSTC_ENABLE_MFT2_OCU5)
    Mft_Ocu_IrqHandler((stc_mftn_ocu_t*)&MFT2_OCU, &m_astcMftOcuInstanceDataLut[OcuInstanceIndexOcu2].stcInternData);
  #endif
}
#endif

#if (PDL_INTERRUPT_ENABLE_MFT0_WFG == PDL_ON) && (PDL_PERIPHERAL_ENABLE_MFT0_WFG == PDL_ON)
/**
 ******************************************************************************
 ** \brief MFT0's WFG IRQ handler
 ******************************************************************************
 */
void WFG0_DTIF0_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT0_WFG10)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT0_WFG10))
    {
      Dstc_MftWfgIrqHandler(DSTC_IRQ_NUMBER_MFT0_WFG10);
    }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT0_WFG32)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT0_WFG32))
    {
      Dstc_MftWfgIrqHandler(DSTC_IRQ_NUMBER_MFT0_WFG32);
    }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT0_WFG54)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT0_WFG54))
    {
      Dstc_MftWfgIrqHandler(DSTC_IRQ_NUMBER_MFT0_WFG54);
    }
  #endif
  #if (PDL_ON != PDL_DSTC_ENABLE_MFT0_WFG10) && \
      (PDL_ON != PDL_DSTC_ENABLE_MFT0_WFG32) && \
      (PDL_ON != PDL_DSTC_ENABLE_MFT0_WFG54)
    Mft_Wfg_IrqHandler((stc_mftn_wfg_t*)&MFT0_WFG, &m_astcMftWfgInstanceDataLut[WfgInstanceIndexWfg0].stcInternData);
  #endif
}
#endif

#if (PDL_INTERRUPT_ENABLE_MFT1_WFG == PDL_ON) && (PDL_PERIPHERAL_ENABLE_MFT1_WFG == PDL_ON)
/**
 ******************************************************************************
 ** \brief MFT1's WFG IRQ handler
 ******************************************************************************
 */
void WFG1_DTIF1_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT1_WFG10)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT1_WFG10))
    {
      Dstc_MftWfgIrqHandler(DSTC_IRQ_NUMBER_MFT1_WFG10);
    }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT1_WFG32)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT1_WFG32))
    {
      Dstc_MftWfgIrqHandler(DSTC_IRQ_NUMBER_MFT1_WFG32);
    }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT1_WFG54)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT1_WFG54))
    {
      Dstc_MftWfgIrqHandler(DSTC_IRQ_NUMBER_MFT1_WFG54);
    }
  #endif
  #if (PDL_ON != PDL_DSTC_ENABLE_MFT1_WFG10) && \
      (PDL_ON != PDL_DSTC_ENABLE_MFT1_WFG32) && \
      (PDL_ON != PDL_DSTC_ENABLE_MFT1_WFG54)
    Mft_Wfg_IrqHandler((stc_mftn_wfg_t*)&MFT1_WFG, &m_astcMftWfgInstanceDataLut[WfgInstanceIndexWfg1].stcInternData);
  #endif
}
#endif

#if (PDL_INTERRUPT_ENABLE_MFT2_WFG == PDL_ON) && (PDL_PERIPHERAL_ENABLE_MFT2_WFG == PDL_ON)
/**
 ******************************************************************************
 ** \brief MFT2's WFG IRQ handler
 ******************************************************************************
 */
void WFG2_DTIF2_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT2_WFG10)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT2_WFG10))
    {
      Dstc_MftWfgIrqHandler(DSTC_IRQ_NUMBER_MFT2_WFG10);
    }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT2_WFG32)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT2_WFG32))
    {
      Dstc_MftWfgIrqHandler(DSTC_IRQ_NUMBER_MFT2_WFG32);
    }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT2_WFG54)
    if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT2_WFG54))
    {
      Dstc_MftWfgIrqHandler(DSTC_IRQ_NUMBER_MFT2_WFG54);
    }
  #endif
  #if (PDL_ON != PDL_DSTC_ENABLE_MFT2_WFG10) && \
      (PDL_ON != PDL_DSTC_ENABLE_MFT2_WFG32) && \
      (PDL_ON != PDL_DSTC_ENABLE_MFT2_WFG54)
    Mft_Wfg_IrqHandler((stc_mftn_wfg_t*)&MFT2_WFG, &m_astcMftWfgInstanceDataLut[WfgInstanceIndexWfg2].stcInternData);
  #endif
}
#endif

#if (PDL_INTERRUPT_ENABLE_MFT0_ICU == PDL_ON) && (PDL_PERIPHERAL_ENABLE_MFT0_ICU == PDL_ON)
/**
 ******************************************************************************
 ** \brief MFT0's ICU IRQ handler
 ******************************************************************************
 */
void ICU0_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT0_ICU0) 
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT0_ICU0))
  { 
    Dstc_MftIcuIrqHandler(DSTC_IRQ_NUMBER_MFT0_ICU0);
  }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT0_ICU1) 
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT0_ICU1))
  { 
    Dstc_MftIcuIrqHandler(DSTC_IRQ_NUMBER_MFT0_ICU1);
  }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT0_ICU2) 
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT0_ICU2))
  { 
    Dstc_MftIcuIrqHandler(DSTC_IRQ_NUMBER_MFT0_ICU2);
  }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT0_ICU3) 
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT0_ICU3))
  { 
    Dstc_MftIcuIrqHandler(DSTC_IRQ_NUMBER_MFT0_ICU3);
  }
  #endif
  #if (PDL_ON != PDL_DSTC_ENABLE_MFT0_ICU0) && (PDL_ON != PDL_DSTC_ENABLE_MFT0_ICU1) && \
      (PDL_ON != PDL_DSTC_ENABLE_MFT0_ICU2) && (PDL_ON != PDL_DSTC_ENABLE_MFT0_ICU3)
    Mft_Icu_IrqHandler((stc_mftn_icu_t*)&MFT0_ICU, &m_astcMftIcuInstanceDataLut[IcuInstanceIndexIcu0].stcInternData);
  #endif
}
#endif

#if (PDL_INTERRUPT_ENABLE_MFT1_ICU == PDL_ON) && (PDL_PERIPHERAL_ENABLE_MFT1_ICU == PDL_ON)
/**
 ******************************************************************************
 ** \brief MFT1's ICU IRQ handler
 ******************************************************************************
 */
void ICU1_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT1_ICU0) 
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT1_ICU0))
  { 
    Dstc_MftIcuIrqHandler(DSTC_IRQ_NUMBER_MFT1_ICU0);
  }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT1_ICU1) 
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT1_ICU1))
  { 
    Dstc_MftIcuIrqHandler(DSTC_IRQ_NUMBER_MFT1_ICU1);
  }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT1_ICU2) 
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT1_ICU2))
  { 
    Dstc_MftIcuIrqHandler(DSTC_IRQ_NUMBER_MFT1_ICU2);
  }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT1_ICU3) 
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT1_ICU3))
  { 
    Dstc_MftIcuIrqHandler(DSTC_IRQ_NUMBER_MFT1_ICU3);
  }
  #endif
  #if (PDL_ON != PDL_DSTC_ENABLE_MFT1_ICU0) && (PDL_ON != PDL_DSTC_ENABLE_MFT1_ICU1) && \
      (PDL_ON != PDL_DSTC_ENABLE_MFT1_ICU2) && (PDL_ON != PDL_DSTC_ENABLE_MFT1_ICU3)
    Mft_Icu_IrqHandler((stc_mftn_icu_t*)&MFT1_ICU, &m_astcMftIcuInstanceDataLut[IcuInstanceIndexIcu1].stcInternData);
  #endif
}
#endif

#if (PDL_INTERRUPT_ENABLE_MFT2_ICU == PDL_ON) && (PDL_PERIPHERAL_ENABLE_MFT2_ICU == PDL_ON)
/**
 ******************************************************************************
 ** \brief MFT2's ICU IRQ handler
 ******************************************************************************
 */
void ICU2_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT2_ICU0) 
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT2_ICU0))
  { 
    Dstc_MftIcuIrqHandler(DSTC_IRQ_NUMBER_MFT2_ICU0);
  }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT2_ICU1) 
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT2_ICU1))
  { 
    Dstc_MftIcuIrqHandler(DSTC_IRQ_NUMBER_MFT2_ICU1);
  }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT2_ICU2) 
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT2_ICU2))
  { 
    Dstc_MftIcuIrqHandler(DSTC_IRQ_NUMBER_MFT2_ICU2);
  }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_MFT2_ICU3) 
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_MFT2_ICU3))
  { 
    Dstc_MftIcuIrqHandler(DSTC_IRQ_NUMBER_MFT2_ICU3);
  }
  #endif
  #if (PDL_ON != PDL_DSTC_ENABLE_MFT2_ICU0) && (PDL_ON != PDL_DSTC_ENABLE_MFT2_ICU1) && \
      (PDL_ON != PDL_DSTC_ENABLE_MFT2_ICU2) && (PDL_ON != PDL_DSTC_ENABLE_MFT2_ICU3)
    Mft_Icu_IrqHandler((stc_mftn_icu_t*)&MFT2_ICU, &m_astcMftIcuInstanceDataLut[IcuInstanceIndexIcu2].stcInternData);
  #endif
}
#endif

/******************************************************************************/
/*********************************** PPG **************************************/
/******************************************************************************/
#if (PDL_PERIPHERAL_ENABLE_PPG == PDL_ON) && (PDL_INTERRUPT_ENABLE_PPG00_02_04 == PDL_ON)
/**
 ******************************************************************************
 ** \brief PPG ch.0,2,4 IRQ handler
 ******************************************************************************
 */
void PPG00_02_04_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_PPG0) 
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_PPG0))
  { 
    Dstc_PpgIrqHandler(DSTC_IRQ_NUMBER_PPG0);
  }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_PPG2) 
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_PPG2))
  { 
    Dstc_PpgIrqHandler(DSTC_IRQ_NUMBER_PPG2);
  }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_PPG4) 
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_PPG4))
  { 
    Dstc_PpgIrqHandler(DSTC_IRQ_NUMBER_PPG4);
  }
  #endif
  #if (PDL_ON != PDL_DSTC_ENABLE_PPG0) && (PDL_ON != PDL_DSTC_ENABLE_PPG2) && (PDL_ON != PDL_DSTC_ENABLE_PPG4)
    Ppg_IrqHandler(PpgCh024);
  #endif
}
#endif

#if (PDL_PERIPHERAL_ENABLE_PPG == PDL_ON) && (PDL_INTERRUPT_ENABLE_PPG08_10_12 == PDL_ON)
/**
 ******************************************************************************
 ** \brief PPG ch.8,10,12 IRQ handler
 ******************************************************************************
 */
void PPG08_10_12_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_PPG8) 
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_PPG8))
  { 
    Dstc_PpgIrqHandler(DSTC_IRQ_NUMBER_PPG8);
  }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_PPG10) 
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_PPG10))
  { 
    Dstc_PpgIrqHandler(DSTC_IRQ_NUMBER_PPG10);
  }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_PPG12) 
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_PPG12))
  { 
    Dstc_PpgIrqHandler(DSTC_IRQ_NUMBER_PPG12);
  }
  #endif
  #if (PDL_ON != PDL_DSTC_ENABLE_PPG8) && (PDL_ON != PDL_DSTC_ENABLE_PPG10) && (PDL_ON != PDL_DSTC_ENABLE_PPG12)
    Ppg_IrqHandler(PpgCh81012);
  #endif
}
#endif

#if (PDL_PERIPHERAL_ENABLE_PPG == PDL_ON) && (PDL_INTERRUPT_ENABLE_PPG16_18_20 == PDL_ON)
/**
 ******************************************************************************
 ** \brief PPG ch. 16, 18, 20 IRQ handler
 ******************************************************************************
 */
void PPG16_18_20_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_PPG16) 
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_PPG16))
  { 
    Dstc_PpgIrqHandler(DSTC_IRQ_NUMBER_PPG16);
  }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_PPG18) 
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_PPG18))
  { 
    Dstc_PpgIrqHandler(DSTC_IRQ_NUMBER_PPG18);
  }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_PPG20) 
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_PPG20))
  { 
    Dstc_PpgIrqHandler(DSTC_IRQ_NUMBER_PPG20);
  }
  #endif
  #if (PDL_ON != PDL_DSTC_ENABLE_PPG16) && (PDL_ON != PDL_DSTC_ENABLE_PPG18) && (PDL_ON != PDL_DSTC_ENABLE_PPG20)
    Ppg_IrqHandler(PpgCh161820);
  #endif
}
#endif

/******************************************************************************/
/*********************************** RTC **************************************/
/******************************************************************************/
#if (PDL_PERIPHERAL_ENABLE_RTC0 == PDL_ON) && (PDL_INTERRUPT_ENABLE_RTC0 == PDL_ON)
void RTC_IRQHandler(void)
{
    RtcIrqHandler();
}
#endif

/******************************************************************************/
/*********************************** MFS **************************************/
/******************************************************************************/

#if (PDL_INTERRUPT_ENABLE_MFS0 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_MFS0 == PDL_ON)
/**
 ******************************************************************************
 ** \brief MFS ch. 0 RX IRQ handler
 ******************************************************************************
 */
void MFS0_RX_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFS0_RX)
    Dstc_MfsRxIrqHandler(DSTC_IRQ_NUMBER_MFS0_RX);
  #else
    MfsIrqHandlerRx((stc_mfsn_t*)&MFS0, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs0].stcInternData));            
  #endif
}

/**
 ******************************************************************************
 ** \brief MFS ch. 0 TX/Status IRQ handler
 ******************************************************************************
 */
void MFS0_TX_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFS0_TX)
    Dstc_MfsRxIrqHandler(DSTC_IRQ_NUMBER_MFS0_TX);
  #else
    uint32_t u32IrqMon = FM4_INTREQ->PDL_IRQMON_MFS0_TX_STATUS;

    if((u32IrqMon & (PDL_IRQMON_MFS0_TX_BITPATTERN)) == PDL_IRQMON_MFS0_TX_BITPATTERN)
    {
        MfsIrqHandlerTx((stc_mfsn_t*)&MFS0, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs0].stcInternData));
    }

    if((u32IrqMon & (PDL_IRQMON_MFS0_STATUS_BITPATTERN)) == PDL_IRQMON_MFS0_STATUS_BITPATTERN)
    {
        MfsIrqHandlerStatus((stc_mfsn_t*)&MFS0, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs0].stcInternData)); 
    } 
  #endif
}
#endif // #if (PDL_INTERRUPT_ENABLE_MFS0 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_MFS0 == PDL_ON)

#if (PDL_INTERRUPT_ENABLE_MFS1 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_MFS1 == PDL_ON)
/**
 ******************************************************************************
 ** \brief MFS ch. 1 RX IRQ handler
 ******************************************************************************
 */
void MFS1_RX_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFS1_RX)
    Dstc_MfsRxIrqHandler(DSTC_IRQ_NUMBER_MFS1_RX);
  #else
    MfsIrqHandlerRx((stc_mfsn_t*)&MFS1, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs1].stcInternData));    
  #endif
}

/**
 ******************************************************************************
 ** \brief MFS ch. 1 TX/Status IRQ handler
 ******************************************************************************
 */
void MFS1_TX_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFS1_TX)
    Dstc_MfsRxIrqHandler(DSTC_IRQ_NUMBER_MFS1_TX);
  #else
    uint32_t u32IrqMon = FM4_INTREQ->PDL_IRQMON_MFS1_TX_STATUS; 

    if((u32IrqMon & (PDL_IRQMON_MFS1_TX_BITPATTERN)) == PDL_IRQMON_MFS1_TX_BITPATTERN)
    {
        MfsIrqHandlerTx((stc_mfsn_t*)&MFS1, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs1].stcInternData));
    }
    
    if((u32IrqMon & (PDL_IRQMON_MFS1_STATUS_BITPATTERN)) == PDL_IRQMON_MFS1_STATUS_BITPATTERN)
    {
        MfsIrqHandlerStatus((stc_mfsn_t*)&MFS1, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs1].stcInternData)); 
    }
  #endif
}
#endif // #if (PDL_INTERRUPT_ENABLE_MFS1 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_MFS1 == PDL_ON)

#if (PDL_INTERRUPT_ENABLE_MFS2 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_MFS2 == PDL_ON)
/**
 ******************************************************************************
 ** \brief MFS ch. 2 and RX IRQ handler
 ******************************************************************************
 */
void MFS2_RX_IRQHandler(void)
{

  #if (PDL_ON == PDL_DSTC_ENABLE_MFS2_RX)
    Dstc_MfsRxIrqHandler(DSTC_IRQ_NUMBER_MFS2_RX);
  #else
    MfsIrqHandlerRx((stc_mfsn_t*)&MFS2, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs2].stcInternData));    
  #endif
}

/**
 ******************************************************************************
 ** \brief MFS ch. 2 TX/Status IRQ handler
 ******************************************************************************
 */
void MFS2_TX_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFS2_TX)
    Dstc_MfsRxIrqHandler(DSTC_IRQ_NUMBER_MFS2_TX);
  #else
    uint32_t u32IrqMon = FM4_INTREQ->PDL_IRQMON_MFS2_TX_STATUS; 

    if((u32IrqMon & (PDL_IRQMON_MFS2_TX_BITPATTERN)) == PDL_IRQMON_MFS2_TX_BITPATTERN)
    {
        MfsIrqHandlerTx((stc_mfsn_t*)&MFS2, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs2].stcInternData));
    }
    
    if((u32IrqMon & (PDL_IRQMON_MFS2_STATUS_BITPATTERN)) == PDL_IRQMON_MFS2_STATUS_BITPATTERN)
    {
        MfsIrqHandlerStatus((stc_mfsn_t*)&MFS2, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs2].stcInternData)); 
    }
  #endif
}
#endif // #if (PDL_INTERRUPT_ENABLE_MFS2 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_MFS2 == PDL_ON)

#if (PDL_INTERRUPT_ENABLE_MFS3 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_MFS3 == PDL_ON)
/**
 ******************************************************************************
 ** \brief MFS ch. 3 RX IRQ handler
 ******************************************************************************
 */
void MFS3_RX_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFS3_RX)
    Dstc_MfsRxIrqHandler(DSTC_IRQ_NUMBER_MFS3_RX);
  #else    
    MfsIrqHandlerRx((stc_mfsn_t*)&MFS3, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs3].stcInternData)); 
  #endif
}

/**
 ******************************************************************************
 ** \brief MFS ch. 3 TX/Status IRQ handler
 ******************************************************************************
 */
void MFS3_TX_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFS3_TX)
    Dstc_MfsRxIrqHandler(DSTC_IRQ_NUMBER_MFS3_TX);
  #else
    uint32_t u32IrqMon = FM4_INTREQ->PDL_IRQMON_MFS3_TX_STATUS; 
  
    if((u32IrqMon & (PDL_IRQMON_MFS3_TX_BITPATTERN)) == PDL_IRQMON_MFS3_TX_BITPATTERN)
    {
        MfsIrqHandlerTx((stc_mfsn_t*)&MFS3, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs3].stcInternData));
    }
    
    if((u32IrqMon & (PDL_IRQMON_MFS3_STATUS_BITPATTERN)) == PDL_IRQMON_MFS3_STATUS_BITPATTERN)
    {
        MfsIrqHandlerStatus((stc_mfsn_t*)&MFS3, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs3].stcInternData)); 
    }
  #endif
}   
#endif // #if (PDL_INTERRUPT_ENABLE_MFS3 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_MFS3 == PDL_ON)

#if (PDL_INTERRUPT_ENABLE_MFS4 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_MFS4 == PDL_ON)
/**
 ******************************************************************************
 ** \brief MFS ch. 4 RX IRQ handler
 ******************************************************************************
 */
void MFS4_RX_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFS4_RX)
    Dstc_MfsRxIrqHandler(DSTC_IRQ_NUMBER_MFS4_RX);
  #else
    MfsIrqHandlerRx((stc_mfsn_t*)&MFS4, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs4].stcInternData)); 
  #endif
}

/**
 ******************************************************************************
 ** \brief MFS ch. 4 TX/Status IRQ handler
 ******************************************************************************
 */
void MFS4_TX_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFS4_TX)
    Dstc_MfsRxIrqHandler(DSTC_IRQ_NUMBER_MFS4_TX);
  #else
    uint32_t u32IrqMon = FM4_INTREQ->PDL_IRQMON_MFS4_TX_STATUS; 
      
    if((u32IrqMon & (PDL_IRQMON_MFS4_TX_BITPATTERN)) == PDL_IRQMON_MFS4_TX_BITPATTERN)
    {
        MfsIrqHandlerTx((stc_mfsn_t*)&MFS4, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs4].stcInternData));
    }
    
    if((u32IrqMon & (PDL_IRQMON_MFS4_STATUS_BITPATTERN)) == PDL_IRQMON_MFS4_STATUS_BITPATTERN)
    {
        MfsIrqHandlerStatus((stc_mfsn_t*)&MFS4, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs4].stcInternData)); 
    }
  #endif
}  
#endif // #if (PDL_INTERRUPT_ENABLE_MFS4 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_MFS4 == PDL_ON)

#if (PDL_INTERRUPT_ENABLE_MFS5 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_MFS5 == PDL_ON)
/**
 ******************************************************************************
 ** \brief MFS ch. 5 RX IRQ handler
 ******************************************************************************
 */
void MFS5_RX_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFS5_RX)
    Dstc_MfsRxIrqHandler(DSTC_IRQ_NUMBER_MFS5_RX);
  #else
     MfsIrqHandlerRx((stc_mfsn_t*)&MFS5, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs5].stcInternData)); 
  #endif
}

/**
 ******************************************************************************
 ** \brief MFS ch. 5 TX/Status IRQ handler
 ******************************************************************************
 */
void MFS5_TX_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFS5_TX)
    Dstc_MfsRxIrqHandler(DSTC_IRQ_NUMBER_MFS5_TX);
  #else
    uint32_t u32IrqMon = FM4_INTREQ->PDL_IRQMON_MFS5_TX_STATUS; 

    if((u32IrqMon & (PDL_IRQMON_MFS5_TX_BITPATTERN)) == PDL_IRQMON_MFS5_TX_BITPATTERN)
    {
        MfsIrqHandlerTx((stc_mfsn_t*)&MFS5, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs5].stcInternData));
    }
    
    if((u32IrqMon & (PDL_IRQMON_MFS5_STATUS_BITPATTERN)) == PDL_IRQMON_MFS5_STATUS_BITPATTERN)
    {
        MfsIrqHandlerStatus((stc_mfsn_t*)&MFS5, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs5].stcInternData)); 
    }
  #endif
}
#endif // #if (PDL_INTERRUPT_ENABLE_MFS5 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_MFS5 == PDL_ON)

#if (PDL_INTERRUPT_ENABLE_MFS6 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_MFS6 == PDL_ON)
/**
 ******************************************************************************
 ** \brief MFS ch. 6 RX IRQ handler
 ******************************************************************************
 */
void MFS6_RX_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFS6_RX)
    Dstc_MfsRxIrqHandler(DSTC_IRQ_NUMBER_MFS6_RX);
  #else
    MfsIrqHandlerRx((stc_mfsn_t*)&MFS6, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs6].stcInternData)); 
  #endif
}

/**
 ******************************************************************************
 ** \brief MFS ch. 6 TX/Status IRQ handler
 ******************************************************************************
 */
void MFS6_TX_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFS6_TX)
    Dstc_MfsRxIrqHandler(DSTC_IRQ_NUMBER_MFS6_TX);
  #else
    uint32_t u32IrqMon = FM4_INTREQ->PDL_IRQMON_MFS6_TX_STATUS; 

    if((u32IrqMon & (PDL_IRQMON_MFS6_TX_BITPATTERN)) == PDL_IRQMON_MFS6_TX_BITPATTERN)
    {
        MfsIrqHandlerTx((stc_mfsn_t*)&MFS6, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs6].stcInternData));
    }
    
    if((u32IrqMon & (PDL_IRQMON_MFS6_STATUS_BITPATTERN)) == PDL_IRQMON_MFS6_STATUS_BITPATTERN)
    {
        MfsIrqHandlerStatus((stc_mfsn_t*)&MFS6, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs6].stcInternData)); 
    }
  #endif
}
#endif // #if (PDL_INTERRUPT_ENABLE_MFS6 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_MFS6 == PDL_ON)

#if (PDL_INTERRUPT_ENABLE_MFS7 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_MFS7 == PDL_ON)
/**
 ******************************************************************************
 ** \brief MFS ch. 7 RX IRQ handler
 ******************************************************************************
 */
void MFS7_RX_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFS7_RX)
    Dstc_MfsRxIrqHandler(DSTC_IRQ_NUMBER_MFS7_RX);
  #else
    MfsIrqHandlerRx((stc_mfsn_t*)&MFS7, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs7].stcInternData)); 
  #endif
}

/**
 ******************************************************************************
 ** \brief MFS ch. 7 TX/Status IRQ handler
 ******************************************************************************
 */
void MFS7_TX_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFS7_TX)
    Dstc_MfsRxIrqHandler(DSTC_IRQ_NUMBER_MFS7_TX);
  #else
    uint32_t u32IrqMon = FM4_INTREQ->PDL_IRQMON_MFS7_TX_STATUS; 

    if((u32IrqMon & (PDL_IRQMON_MFS7_TX_BITPATTERN)) == PDL_IRQMON_MFS7_TX_BITPATTERN)
    {
        MfsIrqHandlerTx((stc_mfsn_t*)&MFS7, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs7].stcInternData));
    }
    
    if((u32IrqMon & (PDL_IRQMON_MFS7_STATUS_BITPATTERN)) == PDL_IRQMON_MFS7_STATUS_BITPATTERN)
    {
        MfsIrqHandlerStatus((stc_mfsn_t*)&MFS7, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs7].stcInternData)); 
    }
  #endif
}
#endif // #if (PDL_INTERRUPT_ENABLE_MFS7 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_MFS7 == PDL_ON)

#if (PDL_INTERRUPT_ENABLE_MFS8 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_MFS8 == PDL_ON)
/**
 ******************************************************************************
 ** \brief MFS ch. 8 RX IRQ handler
 ******************************************************************************
 */
void MFS8_RX_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFS8_RX)
    Dstc_MfsRxIrqHandler(DSTC_IRQ_NUMBER_MFS8_RX);
  #else
    MfsIrqHandlerRx((stc_mfsn_t*)&MFS8, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs8].stcInternData)); 
  #endif
}

/**
 ******************************************************************************
 ** \brief MFS ch. 8 TX/Status IRQ handler
 ******************************************************************************
 */
void MFS8_TX_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFS8_TX)
    Dstc_MfsRxIrqHandler(DSTC_IRQ_NUMBER_MFS8_TX);
  #else
    uint32_t u32IrqMon = FM4_INTREQ->PDL_IRQMON_MFS8_TX_STATUS; 

    if((u32IrqMon & (PDL_IRQMON_MFS8_TX_BITPATTERN)) == PDL_IRQMON_MFS8_TX_BITPATTERN)
    {
        MfsIrqHandlerTx((stc_mfsn_t*)&MFS8, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs8].stcInternData));
    }
    
    if((u32IrqMon & (PDL_IRQMON_MFS8_STATUS_BITPATTERN)) == PDL_IRQMON_MFS8_STATUS_BITPATTERN)
    {
        MfsIrqHandlerStatus((stc_mfsn_t*)&MFS8, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs8].stcInternData)); 
    }
  #endif
}
#endif // #if (PDL_INTERRUPT_ENABLE_MFS8 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_MFS8 == PDL_ON)

#if (PDL_INTERRUPT_ENABLE_MFS9 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_MFS9 == PDL_ON)
/**
 ******************************************************************************
 ** \brief MFS ch. 9 RX IRQ handler
 ******************************************************************************
 */
void MFS9_RX_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFS9_RX)
    Dstc_MfsRxIrqHandler(DSTC_IRQ_NUMBER_MFS9_RX);
  #else
    MfsIrqHandlerRx((stc_mfsn_t*)&MFS9, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs9].stcInternData)); 
  #endif
}

/**
 ******************************************************************************
 ** \brief MFS ch. 9 TX/Status IRQ handler
 ******************************************************************************
 */
void MFS9_TX_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFS9_TX)
    Dstc_MfsRxIrqHandler(DSTC_IRQ_NUMBER_MFS9_TX);
  #else
    uint32_t u32IrqMon = FM4_INTREQ->PDL_IRQMON_MFS9_TX_STATUS; 

    if((u32IrqMon & (PDL_IRQMON_MFS9_TX_BITPATTERN)) == PDL_IRQMON_MFS9_TX_BITPATTERN)
    {
        MfsIrqHandlerTx((stc_mfsn_t*)&MFS9, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs9].stcInternData));
    }
    
    if((u32IrqMon & (PDL_IRQMON_MFS9_STATUS_BITPATTERN)) == PDL_IRQMON_MFS9_STATUS_BITPATTERN)
    {
        MfsIrqHandlerStatus((stc_mfsn_t*)&MFS9, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs9].stcInternData)); 
    }
  #endif
}
#endif // #if (PDL_INTERRUPT_ENABLE_MFS9 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_MFS9 == PDL_ON)

#if (PDL_INTERRUPT_ENABLE_MFS10 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_MFS10 == PDL_ON)
/**
 ******************************************************************************
 ** \brief MFS ch. 10 RX IRQ handler
 ******************************************************************************
 */
void MFS10_RX_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFS10_RX)
    Dstc_MfsRxIrqHandler(DSTC_IRQ_NUMBER_MFS10_RX);
  #else
    MfsIrqHandlerRx((stc_mfsn_t*)&MFS10, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs10].stcInternData)); 
  #endif
}

/**
 ******************************************************************************
 ** \brief MFS ch. 10 TX/Status IRQ handler
 ******************************************************************************
 */
void MFS10_TX_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFS10_TX)
    Dstc_MfsRxIrqHandler(DSTC_IRQ_NUMBER_MFS10_TX);
  #else
    uint32_t u32IrqMon = FM4_INTREQ->PDL_IRQMON_MFS10_TX_STATUS; 

    if((u32IrqMon & (PDL_IRQMON_MFS10_TX_BITPATTERN)) == PDL_IRQMON_MFS10_TX_BITPATTERN)
    {
        MfsIrqHandlerTx((stc_mfsn_t*)&MFS10, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs10].stcInternData));
    }
    
    if((u32IrqMon & (PDL_IRQMON_MFS10_STATUS_BITPATTERN)) == PDL_IRQMON_MFS10_STATUS_BITPATTERN)
    {
        MfsIrqHandlerStatus((stc_mfsn_t*)&MFS10, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs10].stcInternData)); 
    }
  #endif
}
#endif // #if (PDL_INTERRUPT_ENABLE_MFS10 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_MFS10 == PDL_ON)

#if (PDL_INTERRUPT_ENABLE_MFS11 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_MFS11 == PDL_ON)
/**
 ******************************************************************************
 ** \brief MFS ch. 11 RX IRQ handler
 ******************************************************************************
 */
void MFS11_RX_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFS11_RX)
    Dstc_MfsRxIrqHandler(DSTC_IRQ_NUMBER_MFS11_RX);
  #else
    MfsIrqHandlerRx((stc_mfsn_t*)&MFS11, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs11].stcInternData)); 
  #endif
}

/**
 ******************************************************************************
 ** \brief MFS ch. 11 TX/Status IRQ handler
 ******************************************************************************
 */
void MFS11_TX_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFS11_TX)
    Dstc_MfsRxIrqHandler(DSTC_IRQ_NUMBER_MFS11_TX);
  #else
    uint32_t u32IrqMon = FM4_INTREQ->PDL_IRQMON_MFS11_TX_STATUS; 

    if((u32IrqMon & (PDL_IRQMON_MFS11_TX_BITPATTERN)) == PDL_IRQMON_MFS11_TX_BITPATTERN)
    {
        MfsIrqHandlerTx((stc_mfsn_t*)&MFS11, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs11].stcInternData));
    }
    
    if((u32IrqMon & (PDL_IRQMON_MFS11_STATUS_BITPATTERN)) == PDL_IRQMON_MFS11_STATUS_BITPATTERN)
    {
        MfsIrqHandlerStatus((stc_mfsn_t*)&MFS11, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs11].stcInternData)); 
    }
  #endif
}
#endif // #if (PDL_INTERRUPT_ENABLE_MFS11 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_MFS11 == PDL_ON)

#if (PDL_INTERRUPT_ENABLE_MFS12 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_MFS12 == PDL_ON)
/**
 ******************************************************************************
 ** \brief MFS ch. 12 RX IRQ handler
 ******************************************************************************
 */
void MFS12_RX_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFS12_RX)
    Dstc_MfsRxIrqHandler(DSTC_IRQ_NUMBER_MFS12_RX);
  #else
    MfsIrqHandlerRx((stc_mfsn_t*)&MFS12, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs12].stcInternData));
  #endif
}

/**
 ******************************************************************************
 ** \brief MFS ch. 12 TX/Status IRQ handler
 ******************************************************************************
 */
void MFS12_TX_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFS12_TX)
    Dstc_MfsRxIrqHandler(DSTC_IRQ_NUMBER_MFS12_TX);
  #else
    uint32_t u32IrqMon = FM4_INTREQ->PDL_IRQMON_MFS12_TX_STATUS; 

    if((u32IrqMon & (PDL_IRQMON_MFS12_TX_BITPATTERN)) == PDL_IRQMON_MFS12_TX_BITPATTERN)
    {
        MfsIrqHandlerTx((stc_mfsn_t*)&MFS12, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs12].stcInternData));
    }
    
    if((u32IrqMon & (PDL_IRQMON_MFS12_STATUS_BITPATTERN)) == PDL_IRQMON_MFS12_STATUS_BITPATTERN)
    {
        MfsIrqHandlerStatus((stc_mfsn_t*)&MFS12, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs12].stcInternData)); 
    }
  #endif
}
#endif // #if (PDL_INTERRUPT_ENABLE_MFS12 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_MFS12 == PDL_ON)

#if (PDL_INTERRUPT_ENABLE_MFS13 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_MFS13 == PDL_ON)
/**
 ******************************************************************************
 ** \brief MFS ch. 13 RX IRQ handler
 ******************************************************************************
 */
void MFS13_RX_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFS13_RX)
    Dstc_MfsRxIrqHandler(DSTC_IRQ_NUMBER_MFS13_RX);
  #else
    MfsIrqHandlerRx((stc_mfsn_t*)&MFS13, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs13].stcInternData));
  #endif
}

/**
 ******************************************************************************
 ** \brief MFS ch. 13 TX/Status IRQ handler
 ******************************************************************************
 */
void MFS13_TX_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFS13_TX)
    Dstc_MfsRxIrqHandler(DSTC_IRQ_NUMBER_MFS13_TX);
  #else
    uint32_t u32IrqMon = FM4_INTREQ->PDL_IRQMON_MFS13_TX_STATUS; 

    if((u32IrqMon & (PDL_IRQMON_MFS13_TX_BITPATTERN)) == PDL_IRQMON_MFS13_TX_BITPATTERN)
    {
        MfsIrqHandlerTx((stc_mfsn_t*)&MFS13, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs13].stcInternData));
    }
    
    if((u32IrqMon & (PDL_IRQMON_MFS13_STATUS_BITPATTERN)) == PDL_IRQMON_MFS13_STATUS_BITPATTERN)
    {
        MfsIrqHandlerStatus((stc_mfsn_t*)&MFS13, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs13].stcInternData)); 
    }
  #endif
}
#endif // #if (PDL_INTERRUPT_ENABLE_MFS13 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_MFS13 == PDL_ON)

#if (PDL_INTERRUPT_ENABLE_MFS14 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_MFS14 == PDL_ON)
/**
 ******************************************************************************
 ** \brief MFS ch. 14 RX IRQ handler
 ******************************************************************************
 */
void MFS14_RX_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFS14_RX)
    Dstc_MfsRxIrqHandler(DSTC_IRQ_NUMBER_MFS14_RX);
  #else
    MfsIrqHandlerRx((stc_mfsn_t*)&MFS14, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs14].stcInternData));
  #endif
}

/**
 ******************************************************************************
 ** \brief MFS ch. 14 TX/Status IRQ handler
 ******************************************************************************
 */
void MFS14_TX_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFS14_TX)
    Dstc_MfsRxIrqHandler(DSTC_IRQ_NUMBER_MFS14_TX);
  #else
    uint32_t u32IrqMon = FM4_INTREQ->PDL_IRQMON_MFS14_TX_STATUS; 

    if((u32IrqMon & (PDL_IRQMON_MFS14_TX_BITPATTERN)) == PDL_IRQMON_MFS14_TX_BITPATTERN)
    {
        MfsIrqHandlerTx((stc_mfsn_t*)&MFS14, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs14].stcInternData));
    }
    
    if((u32IrqMon & (PDL_IRQMON_MFS14_STATUS_BITPATTERN)) == PDL_IRQMON_MFS14_STATUS_BITPATTERN)
    {
        MfsIrqHandlerStatus((stc_mfsn_t*)&MFS14, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs14].stcInternData)); 
    }
  #endif
}
#endif // #if (PDL_INTERRUPT_ENABLE_MFS14 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_MFS14 == PDL_ON)

#if (PDL_INTERRUPT_ENABLE_MFS15 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_MFS15 == PDL_ON)
/**
 ******************************************************************************
 ** \brief MFS ch. 15 RX IRQ handler
 ******************************************************************************
 */
void MFS15_RX_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFS15_RX)
    Dstc_MfsRxIrqHandler(DSTC_IRQ_NUMBER_MFS15_RX);
  #else
    MfsIrqHandlerRx((stc_mfsn_t*)&MFS15, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs15].stcInternData));
  #endif
}

/**
 ******************************************************************************
 ** \brief MFS ch. 15 TX/Status IRQ handler
 ******************************************************************************
 */
void MFS15_TX_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_MFS15_TX)
    Dstc_MfsRxIrqHandler(DSTC_IRQ_NUMBER_MFS15_TX);
  #else
    uint32_t u32IrqMon = FM4_INTREQ->PDL_IRQMON_MFS15_TX_STATUS; 

    if((u32IrqMon & (PDL_IRQMON_MFS15_TX_BITPATTERN)) == PDL_IRQMON_MFS15_TX_BITPATTERN)
    {
        MfsIrqHandlerTx((stc_mfsn_t*)&MFS15, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs15].stcInternData));
    }
    
    if((u32IrqMon & (PDL_IRQMON_MFS15_STATUS_BITPATTERN)) == PDL_IRQMON_MFS15_STATUS_BITPATTERN)
    {
        MfsIrqHandlerStatus((stc_mfsn_t*)&MFS15, &(m_astcMfsInstanceDataLut[MfsInstanceIndexMfs15].stcInternData)); 
    }
  #endif
}
#endif // #if (PDL_INTERRUPT_ENABLE_MFS15 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_MFS15 == PDL_ON)

/******************************************************************************/
/*********************************** QPRC *************************************/
/******************************************************************************/
#if (PDL_INTERRUPT_ENABLE_QPRC0 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_QPRC0 == PDL_ON)
/**
 ******************************************************************************
 ** \brief QPRC ch.0 IRQ handler
 ******************************************************************************
 */
void QPRC0_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_QPRC0_COUNT_INVERSION)
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_QPRC0_COUNT_INVERSION))
  {
    Dstc_QprcIrqHandler(DSTC_IRQ_NUMBER_QPRC0_COUNT_INVERSION);
  }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_QPRC0_OUT_OF_RANGE)
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_QPRC0_OUT_OF_RANGE))
  {
    Dstc_QprcIrqHandler(DSTC_IRQ_NUMBER_QPRC0_OUT_OF_RANGE);
  }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_QPRC0_PC_MATCH)
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_QPRC0_PC_MATCH))
  {
    Dstc_QprcIrqHandler(DSTC_IRQ_NUMBER_QPRC0_PC_MATCH);
  }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_QPRC0_PC_MATCH_RC_MATCH)
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_QPRC0_PC_MATCH_RC_MATCH))
  {
    Dstc_QprcIrqHandler(DSTC_IRQ_NUMBER_QPRC0_PC_MATCH_RC_MATCH);
  }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_QPRC0_PC_RC_MATCH)
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_QPRC0_PC_RC_MATCH))
  {
    Dstc_QprcIrqHandler(DSTC_IRQ_NUMBER_QPRC0_PC_RC_MATCH);
  }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_QPRC0_UFL_OFL_Z)
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_QPRC0_UFL_OFL_Z))
  {
    Dstc_QprcIrqHandler(DSTC_IRQ_NUMBER_QPRC0_UFL_OFL_Z);
  }
  #endif
  #if (PDL_ON != PDL_DSTC_ENABLE_QPRC0_COUNT_INVERSION)   && \
      (PDL_ON != PDL_DSTC_ENABLE_QPRC0_OUT_OF_RANGE)      && \
      (PDL_ON != PDL_DSTC_ENABLE_QPRC0_PC_MATCH)          && \
      (PDL_ON != PDL_DSTC_ENABLE_QPRC0_PC_MATCH_RC_MATCH) && \
      (PDL_ON != PDL_DSTC_ENABLE_QPRC0_PC_RC_MATCH)       && \
      (PDL_ON != PDL_DSTC_ENABLE_QPRC0_UFL_OFL_Z)
    Qprc_IrqHandler((stc_qprcn_t*)&QPRC0, &m_astcQprcInstanceDataLut[QprcInstanceIndexQprc0].stcInternData);
  #endif
}
#endif

#if (PDL_INTERRUPT_ENABLE_QPRC1 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_QPRC1 == PDL_ON)
/**
 ******************************************************************************
 ** \brief QPRC ch.1 IRQ handler
 ******************************************************************************
 */
void QPRC1_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_QPRC1_COUNT_INVERSION)
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_QPRC1_COUNT_INVERSION))
  {
    Dstc_QprcIrqHandler(DSTC_IRQ_NUMBER_QPRC1_COUNT_INVERSION);
  }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_QPRC1_OUT_OF_RANGE)
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_QPRC1_OUT_OF_RANGE))
  {
    Dstc_QprcIrqHandler(DSTC_IRQ_NUMBER_QPRC1_OUT_OF_RANGE);
  }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_QPRC1_PC_MATCH)
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_QPRC1_PC_MATCH))
  {
    Dstc_QprcIrqHandler(DSTC_IRQ_NUMBER_QPRC1_PC_MATCH);
  }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_QPRC1_PC_MATCH_RC_MATCH)
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_QPRC1_PC_MATCH_RC_MATCH))
  {
    Dstc_QprcIrqHandler(DSTC_IRQ_NUMBER_QPRC1_PC_MATCH_RC_MATCH);
  }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_QPRC1_PC_RC_MATCH)
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_QPRC1_PC_RC_MATCH))
  {
    Dstc_QprcIrqHandler(DSTC_IRQ_NUMBER_QPRC1_PC_RC_MATCH);
  }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_QPRC1_UFL_OFL_Z)
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_QPRC1_UFL_OFL_Z))
  {
    Dstc_QprcIrqHandler(DSTC_IRQ_NUMBER_QPRC1_UFL_OFL_Z);
  }
  #endif
  #if (PDL_ON != PDL_DSTC_ENABLE_QPRC1_COUNT_INVERSION)   && \
      (PDL_ON != PDL_DSTC_ENABLE_QPRC1_OUT_OF_RANGE)      && \
      (PDL_ON != PDL_DSTC_ENABLE_QPRC1_PC_MATCH)          && \
      (PDL_ON != PDL_DSTC_ENABLE_QPRC1_PC_MATCH_RC_MATCH) && \
      (PDL_ON != PDL_DSTC_ENABLE_QPRC1_PC_RC_MATCH)       && \
      (PDL_ON != PDL_DSTC_ENABLE_QPRC1_UFL_OFL_Z)
    Qprc_IrqHandler((stc_qprcn_t*)&QPRC1, &m_astcQprcInstanceDataLut[QprcInstanceIndexQprc1].stcInternData);
  #endif
}
#endif

#if (PDL_INTERRUPT_ENABLE_QPRC2 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_QPRC2 == PDL_ON)
/**
 ******************************************************************************
 ** \brief QPRC ch.2 IRQ handler
 ******************************************************************************
 */
void QPRC2_IRQHandler(void)
{
    #if (PDL_ON == PDL_DSTC_ENABLE_QPRC2_COUNT_INVERSION)
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_QPRC2_COUNT_INVERSION))
  {
    Dstc_QprcIrqHandler(DSTC_IRQ_NUMBER_QPRC2_COUNT_INVERSION);
  }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_QPRC2_OUT_OF_RANGE)
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_QPRC2_OUT_OF_RANGE))
  {
    Dstc_QprcIrqHandler(DSTC_IRQ_NUMBER_QPRC2_OUT_OF_RANGE);
  }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_QPRC2_PC_MATCH)
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_QPRC2_PC_MATCH))
  {
    Dstc_QprcIrqHandler(DSTC_IRQ_NUMBER_QPRC2_PC_MATCH);
  }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_QPRC2_PC_MATCH_RC_MATCH)
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_QPRC2_PC_MATCH_RC_MATCH))
  {
    Dstc_QprcIrqHandler(DSTC_IRQ_NUMBER_QPRC2_PC_MATCH_RC_MATCH);
  }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_QPRC2_PC_RC_MATCH)
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_QPRC2_PC_RC_MATCH))
  {
    Dstc_QprcIrqHandler(DSTC_IRQ_NUMBER_QPRC2_PC_RC_MATCH);
  }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_QPRC2_UFL_OFL_Z)
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_QPRC2_UFL_OFL_Z))
  {
    Dstc_QprcIrqHandler(DSTC_IRQ_NUMBER_QPRC2_UFL_OFL_Z);
  }
  #endif
  #if (PDL_ON != PDL_DSTC_ENABLE_QPRC2_COUNT_INVERSION)   && \
      (PDL_ON != PDL_DSTC_ENABLE_QPRC2_OUT_OF_RANGE)      && \
      (PDL_ON != PDL_DSTC_ENABLE_QPRC2_PC_MATCH)          && \
      (PDL_ON != PDL_DSTC_ENABLE_QPRC2_PC_MATCH_RC_MATCH) && \
      (PDL_ON != PDL_DSTC_ENABLE_QPRC2_PC_RC_MATCH)       && \
      (PDL_ON != PDL_DSTC_ENABLE_QPRC2_UFL_OFL_Z)
    Qprc_IrqHandler((stc_qprcn_t*)&QPRC2, &m_astcQprcInstanceDataLut[QprcInstanceIndexQprc2].stcInternData);
  #endif
}
#endif

#if (PDL_INTERRUPT_ENABLE_QPRC3 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_QPRC3 == PDL_ON)
/**
 ******************************************************************************
 ** \brief QPRC ch.3 IRQ handler
 ******************************************************************************
 */
void QPRC3_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_QPRC3_COUNT_INVERSION)
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_QPRC3_COUNT_INVERSION))
  {
    Dstc_QprcIrqHandler(DSTC_IRQ_NUMBER_QPRC3_COUNT_INVERSION);
  }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_QPRC3_OUT_OF_RANGE)
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_QPRC3_OUT_OF_RANGE))
  {
    Dstc_QprcIrqHandler(DSTC_IRQ_NUMBER_QPRC3_OUT_OF_RANGE);
  }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_QPRC3_PC_MATCH)
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_QPRC3_PC_MATCH))
  {
    Dstc_QprcIrqHandler(DSTC_IRQ_NUMBER_QPRC3_PC_MATCH);
  }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_QPRC3_PC_MATCH_RC_MATCH)
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_QPRC3_PC_MATCH_RC_MATCH))
  {
    Dstc_QprcIrqHandler(DSTC_IRQ_NUMBER_QPRC3_PC_MATCH_RC_MATCH);
  }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_QPRC3_PC_RC_MATCH)
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_QPRC3_PC_RC_MATCH))
  {
    Dstc_QprcIrqHandler(DSTC_IRQ_NUMBER_QPRC3_PC_RC_MATCH);
  }
  #endif
  #if (PDL_ON == PDL_DSTC_ENABLE_QPRC3_UFL_OFL_Z)
  if (TRUE == Dstc_ReadHwintBit(DSTC_IRQ_NUMBER_QPRC3_UFL_OFL_Z))
  {
    Dstc_QprcIrqHandler(DSTC_IRQ_NUMBER_QPRC3_UFL_OFL_Z);
  }
  #endif
  #if (PDL_ON != PDL_DSTC_ENABLE_QPRC3_COUNT_INVERSION)   && \
      (PDL_ON != PDL_DSTC_ENABLE_QPRC3_OUT_OF_RANGE)      && \
      (PDL_ON != PDL_DSTC_ENABLE_QPRC3_PC_MATCH)          && \
      (PDL_ON != PDL_DSTC_ENABLE_QPRC3_PC_MATCH_RC_MATCH) && \
      (PDL_ON != PDL_DSTC_ENABLE_QPRC3_PC_RC_MATCH)       && \
      (PDL_ON != PDL_DSTC_ENABLE_QPRC3_UFL_OFL_Z)
    Qprc_IrqHandler((stc_qprcn_t*)&QPRC3, &m_astcQprcInstanceDataLut[QprcInstanceIndexQprc3].stcInternData);
  #endif
}
#endif

/******************************************************************************/
/*********************************** WC *************************************/
/******************************************************************************/
#if (PDL_INTERRUPT_ENABLE_WC0 == PDL_ON) && (PDL_PERIPHERAL_ENABLE_WC0 == PDL_ON)
/**
 ******************************************************************************
 ** \brief Watch Counter IRQ handler
 ******************************************************************************
 */
void WC_IRQHandler(void)
{
  #if (PDL_ON == PDL_DSTC_ENABLE_WC)
    Dstc_WcIrqHandler();
  #else
    Wc_IrqHandler((stc_wcn_t*)&WC0, &m_astcWcInstanceDataLut[WcInstanceIndexWc0].stcInternData);
  #endif
}
#endif

/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
