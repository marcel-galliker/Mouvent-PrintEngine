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
/** \file interrupts.h
 **
 ** PDL Interrupt Handlers
 **
 ** History:
 **   - 2013-03-21  0.0.1  MWi        First version.
 **   - 2013-08-20  0.0.2  Mwi   IRQMON bit patterns now complete until MFS15
 **
 ******************************************************************************/
#ifndef __INTERRUPTS_H__
#define __INTERRUPTS_H__

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "pdl.h"

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif
  
/**
 ******************************************************************************
 ** \defgroup PdlInterrupts PDL Interrupt handling
 **
 ** \brief Description of the PDL Interrupt handling
 **
 ** Each device group of the FM4 MCUs may have individual interrupt service
 ** routine vector names. For this reason this file defines the device
 ** depenent vectors, if it corresponding resource is set to active in pdl.h.
 **
 ** Interrupt and Callback flow in PDL:
 ** \image html Interrupt_flow.png
 **
 ******************************************************************************/
//@{
 
/*****************************************************************************/
/* Global pre-processor symbols/macros ('#define')                           */
/*****************************************************************************/

// MFS IRQMON registers and bitpatterns
#if (MFS_INT == INT_ACTIVE)
  #if (PDL_MCU_INT_TYPE == PDL_INT_TYPE_A)
    #define PDL_IRQMON_MFS0_RX                 IRQ060MON
    #define PDL_IRQMON_MFS1_RX                 IRQ062MON
    #define PDL_IRQMON_MFS2_RX                 IRQ064MON
    #define PDL_IRQMON_MFS3_RX                 IRQ066MON
    #define PDL_IRQMON_MFS4_RX                 IRQ068MON
    #define PDL_IRQMON_MFS5_RX                 IRQ070MON
    #define PDL_IRQMON_MFS6_RX                 IRQ072MON
    #define PDL_IRQMON_MFS7_RX                 IRQ074MON
    #define PDL_IRQMON_MFS8_RX                 IRQ119MON
    #define PDL_IRQMON_MFS9_RX                 IRQ121MON
    #define PDL_IRQMON_MFS10_RX                IRQ123MON
    #define PDL_IRQMON_MFS11_RX                IRQ125MON
    #define PDL_IRQMON_MFS12_RX                IRQ119MON 
    #define PDL_IRQMON_MFS13_RX                IRQ121MON 
    #define PDL_IRQMON_MFS14_RX                IRQ123MON 
    #define PDL_IRQMON_MFS15_RX                IRQ125MON 

    #define PDL_IRQMON_MFS0_TX_STATUS          IRQ061MON
    #define PDL_IRQMON_MFS1_TX_STATUS          IRQ063MON
    #define PDL_IRQMON_MFS2_TX_STATUS          IRQ065MON
    #define PDL_IRQMON_MFS3_TX_STATUS          IRQ067MON
    #define PDL_IRQMON_MFS4_TX_STATUS          IRQ069MON
    #define PDL_IRQMON_MFS5_TX_STATUS          IRQ071MON
    #define PDL_IRQMON_MFS6_TX_STATUS          IRQ073MON
    #define PDL_IRQMON_MFS7_TX_STATUS          IRQ075MON
    #define PDL_IRQMON_MFS8_TX_STATUS          IRQ120MON
    #define PDL_IRQMON_MFS9_TX_STATUS          IRQ122MON
    #define PDL_IRQMON_MFS10_TX_STATUS         IRQ124MON
    #define PDL_IRQMON_MFS11_TX_STATUS         IRQ126MON
    #define PDL_IRQMON_MFS12_TX_STATUS         IRQ120MON
    #define PDL_IRQMON_MFS13_TX_STATUS         IRQ122MON
    #define PDL_IRQMON_MFS14_TX_STATUS         IRQ124MON
    #define PDL_IRQMON_MFS15_TX_STATUS         IRQ126MON 

    #define PDL_IRQMON_MFS0_RX_BITPATTERN      0x00000001
    #define PDL_IRQMON_MFS1_RX_BITPATTERN      0x00000001
    #define PDL_IRQMON_MFS2_RX_BITPATTERN      0x00000001
    #define PDL_IRQMON_MFS3_RX_BITPATTERN      0x00000001
    #define PDL_IRQMON_MFS4_RX_BITPATTERN      0x00000001
    #define PDL_IRQMON_MFS5_RX_BITPATTERN      0x00000001
    #define PDL_IRQMON_MFS6_RX_BITPATTERN      0x00000001
    #define PDL_IRQMON_MFS7_RX_BITPATTERN      0x00000001
    #define PDL_IRQMON_MFS8_RX_BITPATTERN      0x00000001
    #define PDL_IRQMON_MFS9_RX_BITPATTERN      0x00000001
    #define PDL_IRQMON_MFS10_RX_BITPATTERN     0x00000001
    #define PDL_IRQMON_MFS11_RX_BITPATTERN     0x00000001
    #define PDL_IRQMON_MFS12_RX_BITPATTERN     0x00000001
    #define PDL_IRQMON_MFS13_RX_BITPATTERN     0x00000001 
    #define PDL_IRQMON_MFS14_RX_BITPATTERN     0x00000001 
    #define PDL_IRQMON_MFS15_RX_BITPATTERN     0x00000001 

    #define PDL_IRQMON_MFS0_TX_BITPATTERN      0x00000001
    #define PDL_IRQMON_MFS1_TX_BITPATTERN      0x00000001
    #define PDL_IRQMON_MFS2_TX_BITPATTERN      0x00000001
    #define PDL_IRQMON_MFS3_TX_BITPATTERN      0x00000001
    #define PDL_IRQMON_MFS4_TX_BITPATTERN      0x00000001
    #define PDL_IRQMON_MFS5_TX_BITPATTERN      0x00000001
    #define PDL_IRQMON_MFS6_TX_BITPATTERN      0x00000001
    #define PDL_IRQMON_MFS7_TX_BITPATTERN      0x00000001
    #define PDL_IRQMON_MFS8_TX_BITPATTERN      0x00000001
    #define PDL_IRQMON_MFS9_TX_BITPATTERN      0x00000001
    #define PDL_IRQMON_MFS10_TX_BITPATTERN     0x00000001
    #define PDL_IRQMON_MFS11_TX_BITPATTERN     0x00000001
    #define PDL_IRQMON_MFS12_TX_BITPATTERN     0x00000001 
    #define PDL_IRQMON_MFS13_TX_BITPATTERN     0x00000001
    #define PDL_IRQMON_MFS14_TX_BITPATTERN     0x00000001 
    #define PDL_IRQMON_MFS15_TX_BITPATTERN     0x00000001

    #define PDL_IRQMON_MFS0_STATUS_BITPATTERN  0x00000002
    #define PDL_IRQMON_MFS1_STATUS_BITPATTERN  0x00000002
    #define PDL_IRQMON_MFS2_STATUS_BITPATTERN  0x00000002
    #define PDL_IRQMON_MFS3_STATUS_BITPATTERN  0x00000002
    #define PDL_IRQMON_MFS4_STATUS_BITPATTERN  0x00000002
    #define PDL_IRQMON_MFS5_STATUS_BITPATTERN  0x00000002
    #define PDL_IRQMON_MFS6_STATUS_BITPATTERN  0x00000002
    #define PDL_IRQMON_MFS7_STATUS_BITPATTERN  0x00000002
    #define PDL_IRQMON_MFS8_STATUS_BITPATTERN  0x00000002
    #define PDL_IRQMON_MFS9_STATUS_BITPATTERN  0x00000002
    #define PDL_IRQMON_MFS10_STATUS_BITPATTERN 0x00000002
    #define PDL_IRQMON_MFS11_STATUS_BITPATTERN 0x00000002
    #define PDL_IRQMON_MFS12_STATUS_BITPATTERN 0x00000002 
    #define PDL_IRQMON_MFS13_STATUS_BITPATTERN 0x00000002 
    #define PDL_IRQMON_MFS14_STATUS_BITPATTERN 0x00000002 
    #define PDL_IRQMON_MFS15_STATUS_BITPATTERN 0x00000002

  #elif (PDL_MCU_INT_TYPE == PDL_INT_TYPE_C)
     #error IRQMON not defined.
  #endif
#endif // #if (MFS_INT == INT_ACTIVE)

//@} // PdlInterrupts
  
#ifdef __cplusplus
}
#endif  
  
#endif /* __INTERRUPTS_H__ */

/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
