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
/** \file usbethernetclock.h
 **
 ** Headerfile for USB / Ethernet functions
 ** @link UsbEthernetGroup USB/Ethernet Clock Module description @endlink
 **
 ** History:
 **   - 2012-08-10  1.0  MSc  First version.
 **   - 2012-08-23  1.1  MSc  GNU compiler compatible
 **   - 2012-08-30  1.2  MSc  Correction in USB 0/1 clock enable
 **   - 2012-09-19  1.3  MSc  PLL settings for type 6 devices corrected
 **   - 2012-11-02  1.4  MSc  PLL settings for ethernet corrected
 **   - 2012-11-02  1.5  CNo  Integration Ethernet driver
 **   - 2012-11-22  1.6  MSc  typedefinitions for ethernetclock added 
 **                           (for use without header file)
 **   - 2013-01-14  1.7  MSc  type 6 12MHz setting added 
 **   - 2013-01-16  1.8  MSc  type 9 setting added 
 **   - 2013-06-04  1.9  MSc  FM4 support added
 **   - 2014-02-28  1.9  MSc  Updates in FM4 defines
 **
 ******************************************************************************/

#include "usbethernetclock.h"

/**
 ******************************************************************************
 ** \ingroup UsbEthernetGroup
 ******************************************************************************/
//@{

/*****************************************************************************/
/* Static variable declarations ('static', definition in C source)           */
/*****************************************************************************/

static boolean_t bUsbEthernetClockEnabled = FALSE;

/*****************************************************************************/
/* Global function  ('extern', definition in C source)                       */
/*****************************************************************************/

/**
 ******************************************************************************
 ** \brief Initalize PLL for USB / Ethernet Clock.
 **
 ******************************************************************************/
void UsbEthernetClock_Init(void)
{
    if (bUsbEthernetClockEnabled)
    {
        return;
    }
    /* Initialize default settings */
    #if defined(FM_USBCLK)
        /* Clear USB/Ethernet Clock Setting Register */
        FM_USBCLK->UCCR = 0;

        /* Disable USB Clock */
        bFM_USBCLK_USBEN_USBEN = 0;

        /* PLL Disable */
        FM_USBCLK->UPCR1 = 0x00;

    #elif defined(FM_USBETHERNETCLK)
        /* Clear USB/Ethernet Clock Setting Register */
        FM_USBETHERNETCLK->UCCR = 0;

        /* Disable Ethernet PLL */
        bFM_USBETHERNETCLK_UPCR7_EPLLEN = 0;

        /* Disable USB PLL */
        bFM_USBETHERNETCLK_UPCR1_UPLLEN = 0;

        /* Disable USB Clock 0 */
        #if defined(bFM_USBCLK_USBEN0_USBEN)
          bFM_USBCLK_USBEN0_USBEN = 0;
        #elif defined(bFM_USBETHERNETCLK_UCCR_UCEN0)
          bFM_USBETHERNETCLK_UCCR_UCEN0 = 0;
        #else
          #error No bit definition for UCCR -> UCEN0
        #endif

        /* Disable USB Clock 1 */
        #if defined(bFM_USBCLK_USBEN1_USBEN)
          bFM_USBCLK_USBEN1_USBEN = 0;
        #elif defined(bFM_USBETHERNETCLK_UCCR_UCEN1)
          bFM_USBETHERNETCLK_UCCR_UCEN0 = 0;
        #else
          #error No bit definition for UCCR -> UCEN1
        #endif


        /* PLL Disable */
        FM_USBETHERNETCLK->UPCR1 = 0x00;
    #else
        #error Incorrect USB/Ethernet clock module
    #endif
    /* End of Initialize default settings */

    #ifndef USBETHERNETCLOCK_UCSEL
        #define USBETHERNETCLOCK_UCSEL 0x00
    #endif

    #ifndef USBETHERNETCLOCK_ECSEL
        #define USBETHERNETCLOCK_ECSEL 0x00
    #endif

    /* Check Clock Settings */
    #if USBETHERNETCLOCK_PLL_K > 63
        #error USB ETHERNET CLOCK: (K - 1) must be < 64
    #endif

    #if ((FM3MCUTYPE == FM3MCUTYPE_TYPE6) || (FM3MCUTYPE == FM3MCUTYPE_TYPE0))
        #if USBETHERNETCLOCK_PLL_N == 0
            #error USB ETHERNET CLOCK: (N - 1) can't be 0
        #endif
        #if USBETHERNETCLOCK_PLL_N > 63
            #error USB ETHERNET CLOCK: (N - 1) can't be > 63
        #endif
    #endif
    #if ((FM3MCUTYPE == FM3MCUTYPE_TYPE1) || (FM3MCUTYPE == FM3MCUTYPE_TYPE4) || (FM3MCUTYPE == FM3MCUTYPE_TYPE5))
        #if USBETHERNETCLOCK_PLL_N < 13
            #error USB ETHERNET CLOCK: (N - 1) must be > 12
        #endif
        #if USBETHERNETCLOCK_PLL_N > 99
            #error USB ETHERNET CLOCK: (N - 1) can't be > 99
        #endif
    #endif

    #if ((FM3MCUTYPE == FM3MCUTYPE_TYPE1) || (FM3MCUTYPE == FM3MCUTYPE_TYPE4) || (FM3MCUTYPE == FM3MCUTYPE_TYPE5))
        #if USBETHERNETCLOCK_PLL_M > 15
            #error USB ETHERNET CLOCK: (M - 1) can't be > 15
        #endif
    #endif

    #if ((FM3MCUTYPE == FM3MCUTYPE_TYPE6))
        #if USBETHERNETCLOCK_PLL_M > 15
            #error USB ETHERNET CLOCK: (M - 1) can't be > 15
        #endif
    #endif
    /* End Check Clock Settings */


    /* Setup PLL */
    #if defined(FM_USBETHERNETCLK)
        /* PLL Disable */
        FM3_USBETHERNETCLK->UPCR1 = 0x00;

        /* wait 1ms */
        FM3_USBETHERNETCLK->UPCR2 = 0x03;

        #ifdef USBETHERNETCLOCK_PLL_K
            FM3_USBETHERNETCLK->UPCR3 = USBETHERNETCLOCK_PLL_K - 1;
        #endif
        #ifdef USBETHERNETCLOCK_PLL_K
            FM3_USBETHERNETCLK->UPCR4 = USBETHERNETCLOCK_PLL_N - 1;
        #endif
        #ifdef USBETHERNETCLOCK_PLL_M
            FM3_USBETHERNETCLK->UPCR5 = USBETHERNETCLOCK_PLL_M - 1;
        #endif
    #elif defined(FM_USBCLK)
        /* PLL Disable */
        FM_USBCLK->UPCR1 = 0x00;
        /* wait 1ms */
        FM_USBCLK->UPCR2 = 0x03;
        #ifdef USBETHERNETCLOCK_PLL_K
            FM_USBCLK->UPCR3 = USBETHERNETCLOCK_PLL_K - 1;
        #endif
        #ifdef USBETHERNETCLOCK_PLL_K
            FM_USBCLK->UPCR4 = USBETHERNETCLOCK_PLL_N - 1;
        #endif
        #ifdef USBETHERNETCLOCK_PLL_M
            FM_USBCLK->UPCR5 = USBETHERNETCLOCK_PLL_M - 1;
        #endif
    #else
        #error Incorrect USB/Ethernet clock module
    #endif
    /* End Setup PLL */


    /* Enable PLL and wait PLL gets ready */
    #if defined(FM_USBCLK)
        /* Enable USB Part of PLL */
        bFM_USBCLK_UPCR1_UPLLEN = 1;

        /* Wait PLL gets ready */
        while(bFM_USBCLK_UP_STR_UPRDY == 0);

        /* Select USB clock source */
        FM_USBCLK->UCCR = 0x01 | (USBETHERNETCLOCK_UCSEL << 1);

        /* Enable USB */
        bFM_USBCLK_USBEN_USBEN = 1;
    #elif defined(FM_USBETHERNETCLK)
        #if defined(USBETHERNETCLOCK_UBSR)
            FM_USBETHERNETCLK->UPCR6 = (USBETHERNETCLOCK_UBSR & 0x0F);
        #endif


        /* Enable USB Part of PLL */
        bFM_USBETHERNETCLK_UPCR1_UPLLEN = 1;

        /* Enable Ethernet Part of PLL */
        bFM_USBETHERNETCLK_UPCR7_EPLLEN = 1;


        /* Wait PLL gets ready */
        while(bFM_USBETHERNETCLK_UP_STR_UPRDY == 0);

        #if ((L3_PERIPHERAL_ENABLE_EMAC0 == L3_ON) || (L3_PERIPHERAL_ENABLE_EMAC1 == L3_ON))
             FM_USBETHERNETCLK->UCCR |= (USBETHERNETCLOCK_UCSEL << 1) | (USBETHERNETCLOCK_ECSEL << 5);


             bFM_USBETHERNETCLK_UCCR_ECEN = 1;
        #endif

        #if ((FM_PERIPHERAL_ENABLE_USB0 == ON) && (defined(USB0)))
            FM_USBETHERNETCLK->UCCR |= (USBETHERNETCLOCK_UCSEL << 1) | (USBETHERNETCLOCK_ECSEL << 5);

            #ifdef bFM_USBETHERNETCLK_UCCR_UCEN0
            bFM_USBETHERNETCLK_UCCR_UCEN0 = 1;
            #endif


            #ifdef bFM_USBCLK_USBEN0_USBEN
            bFM_USBCLK_USBEN0_USBEN = 1;
            #endif
            #ifdef bFM_USBETHERNETCLK_USBEN0_USBEN0
            bFM_USBETHERNETCLK_USBEN0_USBEN0 = 1;
            #endif
        #endif
        #if ((FM_PERIPHERAL_ENABLE_USB1 == ON) && (defined(USB1)))
            FM_USBETHERNETCLK->UCCR |= (USBETHERNETCLOCK_UCSEL << 1) | (USBETHERNETCLOCK_ECSEL << 5);

            #ifdef bFM_USBETHERNETCLK_UCCR_UCEN1
            bFM_USBETHERNETCLK_UCCR_UCEN1 = 1;
            #endif


            #ifdef bFM_USBCLK_USBEN1_USBEN
            bFM_USBCLK_USBEN1_USBEN = 1;
            #endif
            #ifdef bFM_USBETHERNETCLK_USBEN1_USBEN1
            bFM_USBETHERNETCLK_USBEN1_USBEN1 = 1;
            #endif
        #endif
    #else
        #error Incorrect USB/Ethernet clock module
    #endif
    /* End Enable PLL and wait PLL gets ready */

    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();

    bUsbEthernetClockEnabled = TRUE;
}

/**
 ******************************************************************************
 ** \brief Deinitalize PLL for USB / Ethernet Clock.
 **
 ******************************************************************************/
void UsbEthernetClock_Deinit(void)
{
     #if defined(FM_USBCLK)
        FM_USBCLK->UCCR = 0;
        bFM_USBCLK_USBEN_USBEN = 0;
        FM_USBCLK->UPCR1 = 0x00; /* PLL Disable */
     #elif defined(FM3_USBETHERNETCLK)

        /* Disable Ethernet PLL */
        bFM3_USBETHERNETCLK_UPCR7_EPLLEN = 0;

        /* Disable USB PLL */
        bFM3_USBETHERNETCLK_UPCR1_UPLLEN = 0;

        FM3_USBETHERNETCLK->UCCR = 0;
        #ifdef bFM3_USBCLK_USBEN0_USBEN
        bFM3_USBCLK_USBEN0_USBEN = 0;
        #endif
        #ifdef bFM3_USBETHERNETCLK_UCCR_UCEN0
        bFM3_USBETHERNETCLK_UCCR_UCEN0 = 0;
        #endif

        #ifdef bFM3_USBCLK_USBEN1_USBEN
        bFM3_USBCLK_USBEN1_USBEN = 0;
        #endif
        #ifdef bFM3_USBETHERNETCLK_UCCR_UCEN1
        bFM3_USBETHERNETCLK_UCCR_UCEN1 = 0;
        #endif
        FM3_USBETHERNETCLK->UPCR1 = 0x00; /* PLL Disable */
    #else
        #error Incorrect USB/Ethernet clock module
    #endif
        
    #if  defined(FM3_USBETHERNETCLK)
        FM3_USBETHERNETCLK->UPCR1 = 0x00; /* PLL Disable */
    #elif defined(FM_USBCLK)
        FM_USBCLK->UPCR1 = 0x00; /* PLL Disable */
    #else
        #error Incorrect USB/Ethernet clock module
    #endif
    bUsbEthernetClockEnabled = FALSE;
}
//@} // UsbEthernetClockGroup


