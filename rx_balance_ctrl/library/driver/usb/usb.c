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
/** \file usb.c
 **
 ** A detailed description is available at 
 ** @link UsbGroup USB Module description @endlink
 **
 ** History:
 **   - 2012-08-28  2.0  MSc  First version (starting at version 2.0)
 **   - 2012-10-02  2.1  MSc  use of external interrupts without L3 implemented
 **   - 2012-11-13  2.2  MSc  Some problems while switching from Host to Device fixed
 **   - 2013-01-31  2.3  MSc  DMA added
 **   - 2013-05-07  2.4  MSc  Minor optimizations
 **   - 2013-06-04  2.5  MSc  FM4 support added
 **   - 2013-09-23  2.6  MSc  FM4 PDL support added
 **   - 2013-11-29  2.7  MSc  PDL adjustments done
 **   - 2014-02-28  2.8  MSc  Fixes for FM4 (IRQ naming, mcu headerfile typedef)
 **                           Fixes in SwitchUsb Routine
 **   - 2014-05-27  2.9  MSc  IRQ double definition with use of PDL > V1.0 solved
 **
 ******************************************************************************/


/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#define __USB_C__ 1
#include "usb.h"
#include "usbethernetclock.h"

#if ((FM_PERIPHERAL_ENABLE_USB0 == ON) || (FM_PERIPHERAL_ENABLE_USB1 == ON))

/**
 ******************************************************************************
 ** \ingroup UsbGroup
 ******************************************************************************/
//@{

/******************************************************************************/
/* Local pre-processor symbols/macros ('#define')                             */
/******************************************************************************/

#if (FM_PERIPHERAL_ENABLE_USB0 == ON)
   #if !defined(USB0)
       #error USB0 is not existing in the MCUs header file, maybe this MCU does not support USB, please change L3_PERIPHERAL_ENABLE_USB0
   #endif
#endif

#if (FM_PERIPHERAL_ENABLE_USB1 == ON)
   #if !defined(USB1)
       #error USB1 is not existing in the MCUs header file, maybe this MCU does not support more than one USB, please change L3_PERIPHERAL_ENABLE_USB1
   #endif
#endif

/******************************************************************************/
/* Global variable definitions (declared in header file with 'extern')        */
/******************************************************************************/

static boolean_t UsbInitDone = FALSE;

/******************************************************************************/
/* Local type definitions ('typedef')                                         */
/******************************************************************************/

/******************************************************************************/
/* Local function prototypes ('static')                                       */
/******************************************************************************/

static stc_usb_intern_data_t* UsbGetInternDataPtr(stc_usbn_t* pstcUsb);
static en_result_t Usb_InitIrq(stc_usbn_t* pstcUsb); 
static en_result_t Usb_DeinitIrq(stc_usbn_t* pstcUsb);
#if (FM_PERIPHERAL_USB_DEVICE_ENABLED == ON)
    static en_result_t Usb_InitDeviceMode(stc_usbn_t* pstcUsb);
    static en_result_t Usb_DeinitDeviceMode(stc_usbn_t* pstcUsb);
#endif /* (FM_PERIPHERAL_USB_DEVICE_ENABLED == ON) */
#if (FM_PERIPHERAL_USB_HOST_ENABLED == ON)
    static en_result_t Usb_InitHostMode(stc_usbn_t* pstcUsb);
    static en_result_t Usb_DeinitHostMode(stc_usbn_t* pstcUsb);
#endif /* (FM_PERIPHERAL_USB_HOST_ENABLED == ON) */
static void Usb_TimeoutHandler(stc_usbn_t* pstcUsb);
extern void UsbHost_DisconnectionCallback(stc_usbn_t* pstcUsb);

/// Macro to return the number of enabled USB instances
//#define USB_INSTANCE_COUNT (uint32_t)(sizeof(m_astcUsbInstanceDataLut) / sizeof(m_astcUsbInstanceDataLut[0]))

/******************************************************************************/
/* Global variable definitions (declared in header file with 'extern')        */
/******************************************************************************/

/// Look-up table for all enabled USB instances and their internal data
stc_usbn_instance_data_t m_astcUsbInstanceDataLut[] =
{
    #if ((FM_PERIPHERAL_ENABLE_USB0 == ON) && (defined(USB0)))
    { &USB0,  // pstcInstance
      0    // stcInternData (not initialized yet)
    },
    #endif
    #if ((FM_PERIPHERAL_ENABLE_USB1 == ON) && (defined(USB1)))
    { &USB1,  // pstcInstance
      0    // stcInternData (not initialized yet)
    },
    #endif
};

#if USB_USES_DMA == ON
#define USB_DMA_INSTANCE_COUNT (uint32_t)(sizeof(astcUsbDma) / sizeof(astcUsbDma[0]))
/// DMA table for all enabled DMA instances and their internal data
stc_usb_dma_t astcUsbDma[] =
{
#if USB_USES_DMA_0 == ON
  { FALSE,                          // This USB DMA currently not active
    DMAC0_IRQn,                     // IRQ of this DMA channel currently not active
    NULL,                           // Currently used with no enpoint handle
    &(FM3_DMAC->DMACA0),            // DMACA register of this DMA channel
    &(FM3_DMAC->DMACB0),            // DMACB register of this DMA channel
    &(FM3_DMAC->DMACSA0),           // DMACSA register of this DMA channel
    &(FM3_DMAC->DMACDA0)            // DMACDA register of this DMA channel
  },
#endif
#if USB_USES_DMA_1 == ON
  { FALSE,                          // This USB DMA currently not active
    DMAC1_IRQn,                     // IRQ of this DMA channel currently not active
    NULL,                           // Currently used with no enpoint handle
    &(FM3_DMAC->DMACA1),            // DMACA register of this DMA channel
    &(FM3_DMAC->DMACB1),            // DMACB register of this DMA channel
    &(FM3_DMAC->DMACSA1),           // DMACSA register of this DMA channel
    &(FM3_DMAC->DMACDA1)            // DMACDA register of this DMA channel
  },
#endif
#if USB_USES_DMA_2 == ON
  { FALSE,                          // This USB DMA currently not active
    DMAC0_IRQn,                     // IRQ of this DMA channel currently not active
    NULL,                           // Currently used with no enpoint handle
    &(FM3_DMAC->DMACA2),            // DMACA register of this DMA channel
    &(FM3_DMAC->DMACB2),            // DMACB register of this DMA channel
    &(FM3_DMAC->DMACSA2),           // DMACSA register of this DMA channel
    &(FM3_DMAC->DMACDA2)            // DMACDA register of this DMA channel
  },
#endif
#if USB_USES_DMA_3 == ON
  { FALSE,                          // This USB DMA currently not active
    DMAC3_IRQn,                     // IRQ of this DMA channel currently not active
    NULL,                           // Currently used with no enpoint handle
    &(FM3_DMAC->DMACA3),            // DMACA register of this DMA channel
    &(FM3_DMAC->DMACB3),            // DMACB register of this DMA channel
    &(FM3_DMAC->DMACSA3),           // DMACSA register of this DMA channel
    &(FM3_DMAC->DMACDA3)            // DMACDA register of this DMA channel
  },
#endif
#if USB_USES_DMA_4 == ON
  { FALSE,                          // This USB DMA currently not active
    DMAC4_IRQn,                     // IRQ of this DMA channel currently not active
    NULL,                           // Currently used with no enpoint handle
    &(FM3_DMAC->DMACA4),            // DMACA register of this DMA channel
    &(FM3_DMAC->DMACB4),            // DMACB register of this DMA channel
    &(FM3_DMAC->DMACSA4),           // DMACSA register of this DMA channel
    &(FM3_DMAC->DMACDA4)            // DMACDA register of this DMA channel
  },
#endif
#if USB_USES_DMA_5 == ON
  { FALSE,                          // This USB DMA currently not active
    DMAC5_IRQn,                     // IRQ of this DMA channel currently not active
    NULL,                           // Currently used with no enpoint handle
    &(FM3_DMAC->DMACA5),            // DMACA register of this DMA channel
    &(FM3_DMAC->DMACB5),            // DMACB register of this DMA channel
    &(FM3_DMAC->DMACSA5),           // DMACSA register of this DMA channel
    &(FM3_DMAC->DMACDA5)            // DMACDA register of this DMA channel
  },
#endif
#if USB_USES_DMA_6 == ON
  { FALSE,                          // This USB DMA currently not active
    DMAC6_IRQn,                     // IRQ of this DMA channel currently not active
    NULL,                           // Currently used with no enpoint handle
    &(FM3_DMAC->DMACA6),            // DMACA register of this DMA channel
    &(FM3_DMAC->DMACB6),            // DMACB register of this DMA channel
    &(FM3_DMAC->DMACSA6),           // DMACSA register of this DMA channel
    &(FM3_DMAC->DMACDA6)            // DMACDA register of this DMA channel
  },
#endif
#if USB_USES_DMA_7 == ON
  { FALSE,                          // This USB DMA currently not active
    DMAC7_IRQn,                     // IRQ of this DMA channel currently not active
    NULL,                           // Currently used with no enpoint handle
    &(FM3_DMAC->DMACA7),            // DMACA register of this DMA channel
    &(FM3_DMAC->DMACB7),            // DMACB register of this DMA channel
    &(FM3_DMAC->DMACSA7),           // DMACSA register of this DMA channel
    &(FM3_DMAC->DMACDA7)            // DMACDA register of this DMA channel
  },
#endif
};
#endif
/******************************************************************************/
/* Function implementation - global ('extern') and local ('static')           */
/******************************************************************************/

/**
 ******************************************************************************
 ** \brief Return the internal data for a certain USB instance.
 **
 ** \param pstcUsb Pointer to USB instance
 **
 ** \return Pointer to internal data or NULL if instance is not enabled (or not known)
 **
 ******************************************************************************/
static stc_usb_intern_data_t* UsbGetInternDataPtr(stc_usbn_t* pstcUsb) 
{
    volatile uint32_t u32Instance;
   
    for (u32Instance = 0; u32Instance < USB_INSTANCE_COUNT; u32Instance++)
    {
        if ((uint32_t)pstcUsb == (uint32_t)(m_astcUsbInstanceDataLut[u32Instance].pstcInstance))
        {
            return &m_astcUsbInstanceDataLut[u32Instance].stcInternData;
        }
    }

    return NULL;
}

/**
 ******************************************************************************
 ** \brief Device dependent initialization of interrupts according CMSIS with
 **        level defined in l3.h
 **
 ** \param pstcUsb Pointer to USB instance
 **
 ** \return Ok     Successful initialization
 **
 ******************************************************************************/
static en_result_t Usb_InitIrq(stc_usbn_t* pstcUsb) 
{
  #if ((FM_PERIPHERAL_ENABLE_USB0 == ON) && (defined(USB0)) && ((USB0_DEVICE_IRQ_ENABLED == ON) || (USB0_HOST_IRQ_ENABLED == ON)))
  if (pstcUsb == &USB0)
  {
    #if (defined(_MB9A110K_H_) || defined(_MB9A110L_H_) || defined(_MB9A110M_H_) || defined(_MB9A110N_H_) || \
         defined(_MB9A130K_H_) || defined(_MB9A130L_H_) || defined(_MB9A130M_H_) || defined(_MB9A130N_H_) || \
                                  defined(_MB9A310L_H_) || defined(_MB9A310M_H_) || defined(_MB9A310N_H_) || \
                                                           defined(_MB9B500N_H_) || defined(_MB9B500R_H_))                           
        #define USB0F_IRQn USBF_IRQn
        #define USB0F_USB0H_IRQn USBF_USBH_IRQn
    #endif
    #if defined(_MB9ABXXX_H_) || defined(_MB9B560R_H_)
        #define USB0F_IRQn USB0_IRQn
        #define USB0F_USB0H_IRQn USB0_HOST_IRQn
        #define USB1F_IRQn USB1_IRQn
        #define USB1F_USB1H_IRQn USB1_HOST_IRQn
    #endif



    // USB0 interrupt settings
    USBDBG("INIT ISR\n");
    NVIC_ClearPendingIRQ(USB0F_IRQn); 
    NVIC_EnableIRQ(USB0F_IRQn);
    NVIC_SetPriority(USB0F_IRQn,IRQ_LEVEL_USB0);
    
    NVIC_ClearPendingIRQ(USB0F_USB0H_IRQn); 
    NVIC_EnableIRQ(USB0F_USB0H_IRQn);
    NVIC_SetPriority(USB0F_USB0H_IRQn,IRQ_LEVEL_USB0);
  }
  #endif
  
  #if ((FM_PERIPHERAL_ENABLE_USB1 == ON) && (defined(USB1)) && ((USB1_DEVICE_IRQ_ENABLED == ON) || (USB1_HOST_IRQ_ENABLED == ON)))
  if (pstcUsb == &USB1)
  {
    // USB1 interrupt settings
    NVIC_ClearPendingIRQ(USB1F_IRQn); 
    NVIC_EnableIRQ(USB1F_IRQn);
    NVIC_SetPriority(USB1F_IRQn,IRQ_LEVEL_USB1);
    
    NVIC_ClearPendingIRQ(USB1F_USB1H_IRQn); 
    NVIC_EnableIRQ(USB1F_USB1H_IRQn);
    NVIC_SetPriority(USB1F_USB1H_IRQn,IRQ_LEVEL_USB1);
  }
  #endif

  return Ok;
}


/**
 ******************************************************************************
 ** \brief Device dependent deinitialization of interrupts according CMSIS with
 **        level defined in l3.h
 **
 ** \param pstcUsb Pointer to USB instance
 **
 ** \return Ok     Successful deinitialization
 **
 ******************************************************************************/
static en_result_t Usb_DeinitIrq(stc_usbn_t* pstcUsb) 
{
  #if (FM_PERIPHERAL_ENABLE_USB0 == ON)
  if (pstcUsb == &USB0)
  {
    #if (defined(_MB9A110K_H_) || defined(_MB9A110L_H_) || defined(_MB9A110M_H_) || defined(_MB9A110N_H_) || \
         defined(_MB9A130K_H_) || defined(_MB9A130L_H_) || defined(_MB9A130M_H_) || defined(_MB9A130N_H_) || \
                                  defined(_MB9A310L_H_) || defined(_MB9A310M_H_) || defined(_MB9A310N_H_) || \
                                                           defined(_MB9B500N_H_) || defined(_MB9B500R_H_))                           
        #define USB0F_IRQn USBF_IRQn
        #define USB0F_USB0H_IRQn USBF_USBH_IRQn
    #endif
    #if defined(_MB9ABXXX_H_) || defined(_MB9B560R_H_)
        #define USB0F_IRQn USB0_IRQn
        #define USB0F_USB0H_IRQn USB0_HOST_IRQn
        #define USB1F_IRQn USB1_IRQn
        #define USB1F_USB1H_IRQn USB1_HOST_IRQn
    #endif
    
    // USB0 interrupt settings
    #if (FM_PERIPHERAL_USB_DEVICE_ENABLED == ON)
        NVIC_ClearPendingIRQ(USB0F_IRQn); 
        NVIC_DisableIRQ(USB0F_IRQn);
        NVIC_SetPriority(USB0F_IRQn,15);
    #endif
    
    NVIC_ClearPendingIRQ(USB0F_USB0H_IRQn); 
    NVIC_DisableIRQ(USB0F_USB0H_IRQn);
    NVIC_SetPriority(USB0F_USB0H_IRQn,15);
  }
  #endif
  
  #if (FM_PERIPHERAL_ENABLE_USB1 == ON)
  if (pstcUsb == &USB1)
  {
    // USB1 interrupt settings
    #if (FM_PERIPHERAL_USB_DEVICE_ENABLED == ON)
        NVIC_ClearPendingIRQ(USB1F_IRQn); 
        NVIC_DisableIRQ(USB1F_IRQn);
        NVIC_SetPriority(USB1F_IRQn,15);
    #endif
    
    NVIC_ClearPendingIRQ(USB1F_USB1H_IRQn); 
    NVIC_DisableIRQ(USB1F_USB1H_IRQn);
    NVIC_SetPriority(USB1F_USB1H_IRQn,15);
  }
  #endif

  return Ok;
}


/**
 ******************************************************************************
 ** \brief Initialize USB in general
 **
 ** \param pstcUsb      USB handle
 **
 ** \param pstcConfig   USB configuration
 **
 ** \return #en_result_t
 **
 ******************************************************************************/
en_result_t Usb_Configure(stc_usbn_t* pstcUsb, stc_usb_config_t* pstcConfig)
{
    stc_usb_intern_data_t* pstcUsbIntern;
    pstcUsbIntern = UsbGetInternDataPtr(pstcUsb);
    if (pstcUsbIntern == NULL)
    {
        return ErrorUninitialized;
    }
    if (pstcUsbIntern->bUsbInstanceSet == FALSE)
    {
        Usb_InitInstance(pstcUsb);
    }
    if (UsbInitDone == FALSE)
    {
        Usb_Init();
    }
    if (pstcConfig->bUseInterrupts == TRUE)
    {
        Usb_InitIrq(pstcUsb);
    }
    else
    {
        Usb_DeinitIrq(pstcUsb);
    }
    
    pstcUsbIntern->bDeviceEnabled = TRUE;
    pstcUsbIntern->bHostEnabled = TRUE;
    
    #if (FM_PERIPHERAL_USB_DEVICE_ENABLED == ON)
        pstcUsbIntern->pu8DeviceDescriptor = pstcConfig->pu8DeviceDescriptor;
        pstcUsbIntern->pu8ConfigDescriptor = pstcConfig->pu8ConfigDescriptor;
    #endif
    
    pstcUsbIntern->pfnSetpCallback = pstcConfig->pfnSetpCallback;
    pstcUsbIntern->pfnConfCallback = pstcConfig->pfnConfCallback;
    pstcUsbIntern->pfnSofCallback = pstcConfig->pfnSofCallback;
    pstcUsbIntern->pfnDeviceVbus = pstcConfig->pfnDeviceVbus;
    pstcUsbIntern->pfnHostVbus = pstcConfig->pfnHostVbus;
    pstcUsbIntern->pfnHostPullDownHostEnable = pstcConfig->pfnHostPullDownHostEnable;
    pstcUsbIntern->pfnHostOvercurrent = pstcConfig->pfnHostOvercurrent;
    pstcUsbIntern->pfnDeviceInit = pstcConfig->pfnDeviceInit;
    pstcUsbIntern->pfnHostInit = pstcConfig->pfnHostInit;
    
    if (pstcUsbIntern->pfnHostVbus != NULL)
    {
        pstcUsbIntern->pfnHostVbus(UsbGpioInit);
        pstcUsbIntern->pfnHostVbus(UsbGpioClear);
    }
    if (pstcUsbIntern->pfnHostPullDownHostEnable != NULL)
    {
        pstcUsbIntern->pfnHostPullDownHostEnable(UsbGpioInit);
        pstcUsbIntern->pfnHostPullDownHostEnable(UsbGpioClear);
    }
    if (pstcUsbIntern->pfnHostOvercurrent != NULL)
    {
        pstcUsbIntern->pfnHostOvercurrent(UsbExtIntInit);
    }
    if (pstcUsbIntern->pfnDeviceVbus != NULL)
    {
        pstcUsbIntern->pfnDeviceVbus(UsbExtIntInit);
        if (pstcUsbIntern->pfnDeviceVbus(UsbExtIntGetLevel))
        {
            pstcUsbIntern->pfnDeviceVbus(UsbExtIntSetLowDetect);
        }
        else
        {
            pstcUsbIntern->pfnDeviceVbus(UsbExtIntSetHighDetect);
        }
        pstcUsbIntern->pfnDeviceVbus(UsbExtIntEnableIsr);
    }
      
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Initialize USB in general
 **
 ** \return #en_result_t
 **
 ******************************************************************************/
en_result_t Usb_Init( void )
{
    volatile uint32_t u32Instance;
   
    for (u32Instance = 0; u32Instance < USB_INSTANCE_COUNT; u32Instance++)
    {
        memset(&m_astcUsbInstanceDataLut[u32Instance].stcInternData,0,sizeof(m_astcUsbInstanceDataLut[u32Instance].stcInternData));
    }
    
    
    
    #if ((FM_PERIPHERAL_ENABLE_USB0 == ON) && (defined(USB0)))
    Usb_InitInstance((stc_usbn_t*)&USB0);
    #endif
    
    #if ((FM_PERIPHERAL_ENABLE_USB1 == ON) && (defined(USB1)))
    Usb_InitInstance((stc_usbn_t*)&USB1);
    #endif

    #if (USB_USE_STANDALONE == 1) || (USB_USE_PDL == 1)     
        #if ((FM_PERIPHERAL_ENABLE_USB0 == ON) && (defined(USB0)))
            
            #if defined(_MB9ABXXX_H_) || defined(_MB9B560R_H_)
            /*Peripheral Function Control Register 2 - USB Enable */
            *((volatile uint32_t*) 0x4003C120) |= 0x3;
            #endif
            
            #ifdef bFM4_GPIO_SPSR_USB0C
                bFM4_GPIO_SPSR_USB0C = 1;
            #endif
            #ifdef bFM3_GPIO_SPSR_USB0C
                bFM3_GPIO_SPSR_USB0C = 1;
            #endif
            
            #ifdef bFM4_GPIO_EPFR00_USBP0E
                bFM4_GPIO_EPFR00_USBP0E = 1;
            #endif    
            #ifdef bFM3_GPIO_EPFR00_USBP0E
                bFM3_GPIO_EPFR00_USBP0E = 1;
            #endif
            #ifdef bFM3_GPIO_EPFR00_USB0PE
                bFM3_GPIO_EPFR00_USB0PE = 1;
            #endif
            
            #ifdef bFM4_GPIO_PFR6_P61 
            /* USB Pull-up : P61 as UHCONX */
            bFM4_GPIO_PFR6_P61 = 1;
            #endif
            
            #ifdef bFM3_GPIO_PFR6_P1 
            /* USB Pull-up : P61 as UHCONX */
            bFM3_GPIO_PFR6_P1 = 1;
            #endif
        #endif
        #if ((FM_PERIPHERAL_ENABLE_USB1 == ON) && (defined(USB1)))
            #ifdef bFM3_GPIO_SPSR_USB1C
						bFM3_GPIO_SPSR_USB1C = 1;
						#endif
            #ifdef bFM3_GPIO_EPFR00_USBP1E
            bFM3_GPIO_EPFR00_USBP1E = 1;
            #endif
            #ifdef bFM3_GPIO_EPFR00_USB1PE
            bFM3_GPIO_EPFR00_USB1PE = 1;
            #endif
            #ifdef bFM3_GPIO_PFR6_P1 
            /* USB Pull-up : P61 as UHCONX */
            bFM3_GPIO_PFR2_P0 = 1;
            #endif
        #endif    
    #else
        #if ((FM_PERIPHERAL_ENABLE_USB0 == ON) && (defined(USB0)))
            bL3_FM3_GPIO_SPSR_USB0C = 1;
            bL3_FM3_GPIO_EPFR00_USB0PE = 1;
            bL3_FM3_GPIO_PFR6_P1 = 1;
        #endif
        #if ((FM_PERIPHERAL_ENABLE_USB1 == ON) && (defined(USB1)))
            bL3_FM3_GPIO_SPSR_USB1C = 1;
            bL3_FM3_GPIO_EPFR00_USB1PE = 1;
            bL3_FM3_GPIO_PFR2_P0 = 1;
        #endif   
    #endif        
    UsbEthernetClock_Init();
    UsbInitDone = TRUE;
    
    return Ok;
}


/**
 ******************************************************************************
 ** \brief Setup USB HAL endpoint
 **
 ** \param pstcUsb Pointer to USB instance
 **
 ** \param u8EndpointAddress Endpoint address
 **
 ** \param u16FifoSize Endpoint FIFO size
 **
 ** \param enType Type of endpoint, see also #en_usb_ep_type_t
 **
 ** \param bInterruptsEnabled Use interrupts
 **
 ** \return #en_result_t
 **
 ******************************************************************************/
en_result_t Usb_SetupHalEndpoint(stc_usbn_t* pstcUsb,uint8_t u8EndpointAddress , uint16_t u16FifoSize, en_usb_ep_type_t enType, boolean_t bInterruptsEnabled)
{
     stc_usbn_endpoint_data_t* pstcEndpoint;
     stc_usb_intern_data_t* pstcUsbIntern;
     pstcUsbIntern = UsbGetInternDataPtr(pstcUsb);
     if (pstcUsbIntern == NULL)
     {
        return ErrorUninitialized;
     }     
     pstcEndpoint = &(pstcUsbIntern->astcEndpoints[(u8EndpointAddress & 0x1F) + 1]);
       
     *(pstcEndpoint->pstcEpControlRegister) = u16FifoSize & 0x1FF;
     pstcEndpoint->u8EndpointAddress = u8EndpointAddress;
     pstcEndpoint->u16EndpointSize = (u16FifoSize & 0x1FF);
     *(pstcEndpoint->pstcEpControlRegister) |= (((u8EndpointAddress & 0x80) > 0) << 12);
     *(pstcEndpoint->pstcEpControlRegister) |= enType << 13;
     if ((bInterruptsEnabled == TRUE) && ((u8EndpointAddress & 0x80) == 0))
     {
         BITMASK_SET(*(pstcEndpoint->pstcEpStatusRegister),_EPNS_DRQIE);
     }
     else
     {
         BITMASK_CLEAR(*(pstcEndpoint->pstcEpStatusRegister),_EPNS_DRQIE);
     }
     BITMASK_SET(*(pstcEndpoint->pstcEpControlRegister),_EPNC_EPEN);
     return Ok;
}

/**
 ******************************************************************************
 ** \brief Switch "Task" between host / device mode
 **
 ** \param pstcUsb Pointer to USB instance
 **
 ** \param enType Type of switching, see also #en_usb_switch_t
 **
 ** \param u32SwitchDelay optional delay
 **
 ** \return #en_result_t
 **
 ******************************************************************************/
en_result_t Usb_SwitchUsb(stc_usbn_t* pstcUsb, en_usb_switch_t enType, uint32_t u32SwitchDelay)
{
    static volatile uint32_t u32InternalSwitchDelay;
    stc_usb_intern_data_t* pstcUsbIntern;
    pstcUsbIntern = UsbGetInternDataPtr(pstcUsb);
    if (pstcUsbIntern == NULL)
    {
        return ErrorUninitialized;
    }
    if (u32InternalSwitchDelay != 0) 
    {
        u32InternalSwitchDelay--;
        return ErrorNotReady;
    }
    switch(enType)
    {
        case UsbSwitchDependingDeviceVbus:
            if (pstcUsbIntern->pfnDeviceVbus != NULL)
            {
                if (pstcUsbIntern->pfnDeviceVbus(UsbExtIntGetLevel) == TRUE)
                {
                    #if (FM_PERIPHERAL_USB_HOST_ENABLED == ON)
                    if ((pstcUsbIntern->bHostActive == TRUE) && (pstcUsbIntern->bHostEnabled == TRUE))
                    {
                        pstcUsbIntern->bHostActive = !(Usb_DeinitHostMode(pstcUsb) == Ok);
                        u32InternalSwitchDelay = u32SwitchDelay;
                        return ErrorNotReady;
                    }
                    #endif

                    #if (FM_PERIPHERAL_USB_DEVICE_ENABLED == ON)
                    if ((pstcUsbIntern->bDeviceActive == FALSE) && (pstcUsbIntern->bDeviceEnabled == TRUE))
                    {
                        pstcUsbIntern->bDeviceActive = (Usb_InitDeviceMode(pstcUsb) == Ok);
                        if (pstcUsbIntern->bDeviceActive == TRUE)
                        {
                            return Ok;
                        }
                        return ErrorNotReady;
                    }
                    #endif
                    
                } else
                {
                    #if (FM_PERIPHERAL_USB_DEVICE_ENABLED == ON)
                    if ((pstcUsbIntern->bDeviceActive == TRUE) && (pstcUsbIntern->bDeviceEnabled == TRUE))
                    {
                        pstcUsbIntern->bDeviceActive = !(Usb_DeinitDeviceMode(pstcUsb) == Ok);
                        u32InternalSwitchDelay = u32SwitchDelay;
                        return ErrorNotReady;
                    }
                    #endif
                    #if (FM_PERIPHERAL_USB_HOST_ENABLED == ON)
                    if ((pstcUsbIntern->bHostActive == FALSE) && (pstcUsbIntern->bHostEnabled == TRUE))
                    {
                        pstcUsbIntern->bHostActive = (Usb_InitHostMode(pstcUsb) == Ok);
                        if (pstcUsbIntern->bHostActive == TRUE)
                        {
                            return Ok;
                        }
                        return ErrorNotReady;
                    }
                    #endif
                }
            }
            break;
        case UsbSwitchToDevice:
            #if (FM_PERIPHERAL_USB_HOST_ENABLED == ON)
            if ((pstcUsbIntern->bHostActive == TRUE) && (pstcUsbIntern->bHostEnabled == TRUE))
            {
                pstcUsbIntern->bHostActive = !(Usb_DeinitHostMode(pstcUsb) == Ok);
                u32InternalSwitchDelay = u32SwitchDelay;
                return ErrorNotReady;
            }
            #endif
            #if (FM_PERIPHERAL_USB_DEVICE_ENABLED == ON)
            if ((pstcUsbIntern->bDeviceActive == FALSE) && (pstcUsbIntern->bDeviceEnabled == TRUE))
            {
                pstcUsbIntern->bDeviceActive = (Usb_InitDeviceMode(pstcUsb) == Ok);
                if (pstcUsbIntern->bDeviceActive == TRUE)
                {
                    return Ok;
                }
                return ErrorNotReady;
            }
            #endif
            break;
        case UsbSwitchToHost:
          #if (FM_PERIPHERAL_USB_DEVICE_ENABLED == ON)
          if ((pstcUsbIntern->bDeviceActive == TRUE) && (pstcUsbIntern->bDeviceEnabled == TRUE))
          {
              pstcUsbIntern->bDeviceActive = !(Usb_DeinitDeviceMode(pstcUsb) == Ok);
              u32InternalSwitchDelay = u32SwitchDelay;
              return ErrorNotReady;
          }
          #endif
          #if (FM_PERIPHERAL_USB_HOST_ENABLED == ON)
          if ((pstcUsbIntern->bHostActive == FALSE) && (pstcUsbIntern->bHostEnabled == TRUE))
          {
              pstcUsbIntern->bHostActive = (Usb_InitHostMode(pstcUsb) == Ok);
              if (pstcUsbIntern->bHostActive == TRUE)
              {
                  return Ok;
              }
              return ErrorNotReady;
          }
          #endif
          break;
       case UsbSwitchAllOff:
          #if (FM_PERIPHERAL_USB_DEVICE_ENABLED == ON)
          if ((pstcUsbIntern->bDeviceActive == TRUE) && (pstcUsbIntern->bDeviceEnabled == TRUE))
          {
              pstcUsbIntern->bDeviceActive = !(Usb_DeinitDeviceMode(pstcUsb) == Ok);
              u32InternalSwitchDelay = u32SwitchDelay;
          }
          #endif
          #if (FM_PERIPHERAL_USB_HOST_ENABLED == ON)
          if ((pstcUsbIntern->bHostActive == TRUE) && (pstcUsbIntern->bHostEnabled == TRUE))
          {
              pstcUsbIntern->bHostActive = !(Usb_DeinitHostMode(pstcUsb) == Ok);
              u32InternalSwitchDelay = u32SwitchDelay;
          }
          #endif
          if ((pstcUsbIntern->bDeviceActive == FALSE) && (pstcUsbIntern->bHostActive == FALSE))
          {
              return Ok;
          }
          return ErrorNotReady;
    }
    return Ok;
}

#if (FM_PERIPHERAL_USB_DEVICE_ENABLED == ON)
/**
 ******************************************************************************
 ** \brief Init USB device
 **
 ** \param pstcUsb Pointer to USB instance
 **
 ******************************************************************************/
static en_result_t Usb_InitDeviceMode(stc_usbn_t* pstcUsb)
{
    volatile uint32_t i;
    stc_usb_config_t stcConfig;
    stc_usb_intern_data_t* pstcUsbIntern;
    pstcUsbIntern = UsbGetInternDataPtr(pstcUsb);
    if (pstcUsbIntern == NULL)
    {
        return ErrorUninitialized;
    }    
    if ((pstcUsbIntern->pu8DeviceDescriptor == NULL) || (pstcUsbIntern->pfnDeviceInit == NULL))
    {
        return ErrorUninitialized;
    }
    if (pstcUsbIntern->pfnHostPullDownHostEnable != NULL)
    {
        pstcUsbIntern->pfnHostPullDownHostEnable(UsbGpioClear);
    }
    if (pstcUsbIntern->pfnHostVbus != NULL)
    {
        pstcUsbIntern->pfnHostVbus(UsbGpioClear);
    }
    Usb_InitInstance(pstcUsb);
    Usb_DeinitDeviceMode(pstcUsb);
    
    pstcUsb->UDCC_f.RST = 1;    // Reset USB HAL
    pstcUsb->UDCC = 0x00A0;     // load initial values
    pstcUsb->HCNT = 0;          // Switch off host
    pstcUsb->HCNT_f.URST = 1;   // Switch off host
    pstcUsb->UDCC_f.USTP = 0;
    pstcUsb->UDCC_f.RESUM  = 0;  
    pstcUsb->UDCC_f.RFBK  = 0;  
    pstcUsb->UDCC_f.STALCLREN = 1;
    
    if (pstcUsbIntern->pu8ConfigDescriptor[7] == USBATTR_SELFPOWER) 
    {
        pstcUsb->UDCC_f.PWC = 1;
    }
    else
    {
        pstcUsb->UDCC_f.PWC = 0;
    }
    
    pstcUsb->EP0C = pstcUsbIntern->pu8DeviceDescriptor[7];
    
    pstcUsb->UDCC_f.RST = 0;
    
    pstcUsbIntern->pfnDeviceInit(pstcUsb,&stcConfig);
    
    
     
    pstcUsbIntern->pfnSetpCallback = stcConfig.pfnSetpCallback;
    pstcUsbIntern->pfnConfCallback = stcConfig.pfnConfCallback;
    
    for (i = pstcUsbIntern->u8NumberOfEndpoints; i > 0;i--)
    {
        BITMASK_CLEAR(*((uint16_t*)pstcUsbIntern->astcEndpoints[i - 1].pstcEpStatusRegister),_EPNS_BFINI);
    }
     
    pstcUsb->UDCIE_f.SUSPIE  = 1;
    pstcUsb->UDCIE_f.SOFIE   = 0;
    pstcUsb->UDCIE_f.BRSTIE  = 1;
    pstcUsb->UDCIE_f.WKUPIE  = 1;
    pstcUsb->UDCIE_f.CONFIE  = 1;
    pstcUsb->UDCIE_f.CONFN   = 0;
    
    pstcUsb->EP0IS_f.DRQIIE  = 0;
    pstcUsb->EP0OS_f.DRQOIE  = 1; 
    
    pstcUsb->UDCC_f.HCONX = 0;
    
    return Ok;
}
#endif

#if (FM_PERIPHERAL_USB_DEVICE_ENABLED == ON)
/**
 ******************************************************************************
 ** \brief Deinit USB device
 **
 ** \param pstcUsb Pointer to USB instance
 **
 ******************************************************************************/
static en_result_t Usb_DeinitDeviceMode(stc_usbn_t* pstcUsb)
{
    uint8_t i;
    stc_usb_intern_data_t* pstcUsbIntern;
    pstcUsbIntern = UsbGetInternDataPtr(pstcUsb);
    if (pstcUsbIntern == NULL)
    {
        return ErrorUninitialized;
    }
    for (i = 0;i < pstcUsbIntern->u8NumberOfEndpoints;i++)
    {
        BITMASK_CLEAR(*((uint16_t*)pstcUsbIntern->astcEndpoints[i - 1].pstcEpStatusRegister),_EPNC_EPEN);
        BITMASK_CLEAR(*((uint16_t*)pstcUsbIntern->astcEndpoints[i - 1].pstcEpStatusRegister),_EPNS_DRQIE);
    }
    
    pstcUsb->UDCIE_f.CONFN   = 0;
    pstcUsb->UDCC_f.HCONX  = 1;
    pstcUsb->UDCC_f.RST = 1;    // Reset USB HAL
    
    pstcUsb->UDCIE = 0;
    
    pstcUsb->EP0IS_f.DRQIIE  = 0;
    pstcUsb->EP0OS_f.DRQOIE  = 0;
    

    if (pstcUsbIntern->pfnConfCallback != NULL)
    {
        pstcUsbIntern->pfnConfCallback(pstcUsb);
    }
    
    Usb_DeinitInstance(pstcUsb);
    
    return Ok;
}
#endif

#if (FM_PERIPHERAL_USB_HOST_ENABLED == ON)
/**
 ******************************************************************************
 ** \brief Init USB host
 **
 ** \param pstcUsb Pointer to USB instance
 **
 ******************************************************************************/
static en_result_t Usb_InitHostMode(stc_usbn_t* pstcUsb)
{
    uint8_t i;
    stc_usb_config_t stcConfig;
    stc_usb_intern_data_t* pstcUsbIntern;
    USB_ZERO_STRUCT(stcConfig);
    pstcUsbIntern = UsbGetInternDataPtr(pstcUsb);
    if (pstcUsbIntern == NULL)
    {
        return ErrorUninitialized;
    }    
    pstcUsbIntern->bSofTimeoutEnabled = FALSE;
    for(i = 0;i<5;i++)
    {
        pstcUsbIntern->pstcUsbTimeOutHandler[i].Handler = NULL;
        pstcUsbIntern->pstcUsbTimeOutHandler[i].u16TimeOut = 0;
    }
    
    pstcUsb->HCNT_f.HOST = 0;
    pstcUsb->UDCC_f.HCONX  = 1; // disable HCONX
    pstcUsb->UDCC_f.RST = 1;    // Reset USB HAL
    
    Usb_InitInstance(pstcUsb);
    
    pstcUsb->EP1C_f.EPEN = 0;
    pstcUsb->EP2C_f.EPEN = 0;
    
    pstcUsb->EP0IS_f.DRQIIE  = 0;
    pstcUsb->EP0OS_f.DRQOIE  = 0;    
    pstcUsb->EP1C = 0x4000  | 256;
    pstcUsb->EP2C = 0x4000  | 64;
    pstcUsb->EP3C = 0x4000;
    pstcUsb->EP4C = 0x4000;
    pstcUsb->EP5C = 0x4000;
    
    
    //Usb_SetupHalEndpoint(pstcUsb,0x81, 256, EpTypeHost, FALSE); // EP 1 for IN transfers   
    //Usb_SetupHalEndpoint(pstcUsb,0x02, 64,  EpTypeHost, FALSE); // EP 2 for OUT transfers
    
    pstcUsb->EP1C_f.DIR = 0;    // EP1 is a HOST IN endpoint
    pstcUsb->EP2C_f.DIR = 1;    // EP2 is a HOST OUT endpoint

    pstcUsb->EP1C_f.EPEN = 1;   // enable endpoint 1
    pstcUsb->EP2C_f.EPEN = 1;   // enable endpoint 2
    
    pstcUsb->HFCOMP = 0;
    pstcUsb->HRTIMER0 = 0;
    pstcUsb->HRTIMER1 = 0;
    pstcUsb->HRTIMER2 = 0;
    pstcUsb->HSTATE_f.CSTAT = 0;
    
    pstcUsbIntern->pfnHostInit(pstcUsb,&stcConfig);
    pstcUsbIntern->pfnSofCallback = stcConfig.pfnSofCallback;
    pstcUsbIntern->pfnDirqCallback = stcConfig.pfnDirqCallback;
    pstcUsbIntern->pfnCnnirqCallback = stcConfig.pfnCnnirqCallback;
    pstcUsbIntern->pfnCmpirqCallback = stcConfig.pfnCmpirqCallback;
    pstcUsbIntern->pfnUrirqCallback = stcConfig.pfnUrirqCallback;
    pstcUsbIntern->pfnRwkirqCallback = stcConfig.pfnRwkirqCallback;
    pstcUsbIntern->pfnTcanCallback = stcConfig.pfnTcanCallback;
    
    pstcUsb->HADR = 0;  // Device Address = 0
    pstcUsb->HCNT_f.HOST = 1; // Enable Host
    pstcUsb->HCNT_f.HOST = 1; // Enable Host
    
    pstcUsb->HIRQ = 0;  /* IRQ clear */
    
    pstcUsb->HRTIMER0 = 0;
    pstcUsb->HRTIMER1 = 0;
    pstcUsb->HRTIMER2 = 0;
    
    pstcUsb->HCNT_f.RETRY = 0;
    
    pstcUsb->HEOF = 0x2c9; // Set the time where token are allowed in a frame
    
    pstcUsb->EP1S_f.BFINI = 1;
    pstcUsb->EP1S_f.BFINI = 0;
    
    pstcUsb->EP2S_f.BFINI = 1;
    pstcUsb->EP2S_f.BFINI = 0;
    
    pstcUsb->HFCOMP = 0x00;  /* SOF interrupt frame No. for INTERRUPT-IN transfer */
    
    pstcUsb->HCNT_f.HOST = 1;    // Enable Host 
    pstcUsb->HCNT_f.CNNIRE = 1;  // connection interrupt 
    pstcUsb->HCNT_f.DIRE = 1;    // disconnect interrupt 
    pstcUsb->HCNT_f.CMPIRE = 1;  // token completion interrupt 
    pstcUsb->HCNT_f.SOFIRE = 1;  // SOF interrupt 
    pstcUsb->HCNT_f.HOST = 1;    // Enable Host 
    
    if (pstcUsbIntern->pfnHostPullDownHostEnable != NULL)
    {
        pstcUsbIntern->pfnHostPullDownHostEnable(UsbGpioSet);
    }
    if (pstcUsbIntern->pfnHostVbus != NULL)
    {
        pstcUsbIntern->pfnHostVbus(UsbGpioSet);
    }

    return Ok;
}
#endif

#if (FM_PERIPHERAL_USB_HOST_ENABLED == ON)
/**
 ******************************************************************************
 ** \brief Deinit USB host
 **
 ** \param pstcUsb Pointer to USB instance
 **
 ******************************************************************************/
static en_result_t Usb_DeinitHostMode(stc_usbn_t* pstcUsb)
{
    uint8_t i;
    stc_usb_intern_data_t* pstcUsbIntern;
    stc_usb_config_t stcConfig;
    USB_ZERO_STRUCT(stcConfig);
    pstcUsbIntern = UsbGetInternDataPtr(pstcUsb);
    if (pstcUsbIntern == NULL)
    {
        return ErrorUninitialized;
    }    
    pstcUsb->HIRQ = 0;
    pstcUsbIntern->bSofTimeoutEnabled = FALSE;
    for(i = 0;i<5;i++)
    {
        pstcUsbIntern->pstcUsbTimeOutHandler[i].Handler = NULL;
        pstcUsbIntern->pstcUsbTimeOutHandler[i].u16TimeOut = 0;
    }
    
    pstcUsbIntern->pfnHostInit(pstcUsb,&stcConfig);
    pstcUsbIntern->pfnSofCallback = NULL;
    pstcUsbIntern->pfnDirqCallback = NULL;
    pstcUsbIntern->pfnCnnirqCallback = NULL;
    pstcUsbIntern->pfnCmpirqCallback = NULL;
    pstcUsbIntern->pfnUrirqCallback = NULL;
    pstcUsbIntern->pfnRwkirqCallback = NULL;
    pstcUsbIntern->pfnTcanCallback = NULL;
    
    UsbHost_DisconnectionCallback(pstcUsb);
    
    pstcUsb->UDCC_f.HCONX  = 1; // disable HCONX
    pstcUsb->UDCC_f.RST = 1;    // Reset USB HAL
    pstcUsb->EP1C = 0x4000;
    pstcUsb->EP2C = 0x4000;
    pstcUsb->EP3C = 0x4000;
    pstcUsb->EP4C = 0x4000;
    pstcUsb->EP5C = 0x4000;
    
    
    pstcUsb->HADR = 0;
    
    
    if (pstcUsbIntern->pfnHostPullDownHostEnable != NULL)
    {
        pstcUsbIntern->pfnHostPullDownHostEnable(UsbGpioClear);
    }
    if (pstcUsbIntern->pfnHostVbus != NULL)
    {
        pstcUsbIntern->pfnHostVbus(UsbGpioClear);
    }
    pstcUsb->HCNT = 0;
    
    Usb_DeinitInstance(pstcUsb);
    
    return Ok;
}
#endif


/**
 ******************************************************************************
 ** \brief Init USB instance
 **
 ** \param pstcUsb Pointer to USB instance
 **
 ******************************************************************************/
en_result_t Usb_InitInstance(stc_usbn_t* pstcUsb) 
{
    uint8_t i;
    stc_usb_intern_data_t* pstcUsbIntern;
    pstcUsbIntern = UsbGetInternDataPtr(pstcUsb);
    if (pstcUsbIntern == NULL)
    {
        return ErrorUninitialized;
    }
    pstcUsbIntern->bUsbInstanceSet = TRUE;
    #if (FM_PERIPHERAL_USB_DEVICE_ENABLED == ON)
    pstcUsbIntern->u8NumberOfEndpoints = 7;
    #else
    pstcUsbIntern->u8NumberOfEndpoints = 2;
    #endif
    
    pstcUsbIntern->bSofTimeoutEnabled = FALSE;
    for(i = 0;i<5;i++)
    {
        pstcUsbIntern->pstcUsbTimeOutHandler[i].Handler = NULL;
        pstcUsbIntern->pstcUsbTimeOutHandler[i].u16TimeOut = 0;
    }
    
    pstcUsb->UDCC = 0xA0; // load initial values
    pstcUsb->EP0C = 0;    // load initial values
    pstcUsb->EP1C = 0;    // load initial values
    pstcUsb->EP2C = 0;    // load initial values 
    pstcUsb->EP3C = 0;    // load initial values 
    pstcUsb->EP4C = 0;    // load initial values 
    pstcUsb->EP5C = 0;    // load initial values 
    pstcUsb->UDCS = 0;    // load initial values 
    pstcUsb->UDCIE = 0;   // load initial values 
    pstcUsb->EP0IS = 0;   // load initial values 
    pstcUsb->EP0OS = 0;   // load initial values 
    pstcUsb->EP1S = 0;    // load initial values 
    pstcUsb->EP2S = 0;    // load initial values 
    pstcUsb->EP3S = 0;    // load initial values 
    pstcUsb->EP4S = 0;    // load initial values 
    pstcUsb->EP5S = 0;    // load initial values 
    pstcUsb->HCNT = 0;    // load initial values 
    pstcUsb->HIRQ = 0;    // load initial values 
    pstcUsb->HERR = 0;    // load initial values 
    pstcUsb->HSTATE = 0x10;  // load initial values 
    pstcUsb->HADR = 0x10;    // load initial values 
    
    #if (FM_PERIPHERAL_USB_DEVICE_ENABLED == ON)
    pstcUsbIntern->astcEndpoints[0].u8EndpointAddress = 0x80;
    pstcUsbIntern->astcEndpoints[0].u16EndpointSize   = 64;
    pstcUsbIntern->astcEndpoints[0].pstcEpStatusRegister   = &(pstcUsb->EP0IS);
    pstcUsbIntern->astcEndpoints[0].pstcEpControlRegister  = &(pstcUsb->EP0C);
    pstcUsbIntern->astcEndpoints[0].pstcEpDataRegister     = &(pstcUsb->EP0DT);
    pstcUsbIntern->astcEndpoints[0].pfnRxTxCallback = NULL;
    pstcUsbIntern->astcEndpoints[0].bAutomaticNullTermination = FALSE;
    pstcUsbIntern->astcEndpoints[0].pstcUsbInstance = pstcUsb;
      
    pstcUsbIntern->astcEndpoints[1].u8EndpointAddress = 0x00;
    pstcUsbIntern->astcEndpoints[1].u16EndpointSize   = 64;
    pstcUsbIntern->astcEndpoints[1].pstcEpStatusRegister   = &(pstcUsb->EP0OS);
    pstcUsbIntern->astcEndpoints[1].pstcEpControlRegister  = &(pstcUsb->EP0C);
    pstcUsbIntern->astcEndpoints[1].pstcEpDataRegister     = &(pstcUsb->EP0DT);
    pstcUsbIntern->astcEndpoints[1].pfnRxTxCallback = NULL;
    pstcUsbIntern->astcEndpoints[1].bAutomaticNullTermination = FALSE;
    pstcUsbIntern->astcEndpoints[1].pstcUsbInstance = pstcUsb;
    
    pstcUsbIntern->astcEndpoints[2].u8EndpointAddress = 0x01;
    pstcUsbIntern->astcEndpoints[2].u16EndpointSize   = 0x00;
    pstcUsbIntern->astcEndpoints[2].pstcEpStatusRegister   = &(pstcUsb->EP1S);
    pstcUsbIntern->astcEndpoints[2].pstcEpControlRegister  = &(pstcUsb->EP1C);
    pstcUsbIntern->astcEndpoints[2].pstcEpDataRegister     = &(pstcUsb->EP1DT);
    pstcUsbIntern->astcEndpoints[2].pfnRxTxCallback = NULL;
    pstcUsbIntern->astcEndpoints[2].bAutomaticNullTermination = FALSE;
    pstcUsbIntern->astcEndpoints[2].pstcUsbInstance = pstcUsb;
    
    pstcUsbIntern->astcEndpoints[3].u8EndpointAddress = 0x02;
    pstcUsbIntern->astcEndpoints[3].u16EndpointSize   = 0x00;
    pstcUsbIntern->astcEndpoints[3].pstcEpStatusRegister   = &(pstcUsb->EP2S);
    pstcUsbIntern->astcEndpoints[3].pstcEpControlRegister  = &(pstcUsb->EP2C);
    pstcUsbIntern->astcEndpoints[3].pstcEpDataRegister     = &(pstcUsb->EP2DT);
    pstcUsbIntern->astcEndpoints[3].pfnRxTxCallback = NULL;
    pstcUsbIntern->astcEndpoints[3].bAutomaticNullTermination = FALSE;
    pstcUsbIntern->astcEndpoints[3].pstcUsbInstance = pstcUsb;

    pstcUsbIntern->astcEndpoints[4].u8EndpointAddress = 0x03;
    pstcUsbIntern->astcEndpoints[4].u16EndpointSize   = 0x00;
    pstcUsbIntern->astcEndpoints[4].pstcEpStatusRegister   = &(pstcUsb->EP3S);
    pstcUsbIntern->astcEndpoints[4].pstcEpControlRegister  = &(pstcUsb->EP3C);
    pstcUsbIntern->astcEndpoints[4].pstcEpDataRegister     = &(pstcUsb->EP3DT);
    pstcUsbIntern->astcEndpoints[4].pfnRxTxCallback = NULL;  
    pstcUsbIntern->astcEndpoints[4].bAutomaticNullTermination = FALSE;  
    pstcUsbIntern->astcEndpoints[4].pstcUsbInstance = pstcUsb;

    pstcUsbIntern->astcEndpoints[5].u8EndpointAddress = 0x04;
    pstcUsbIntern->astcEndpoints[5].u16EndpointSize   = 0x00;
    pstcUsbIntern->astcEndpoints[5].pstcEpStatusRegister   = &(pstcUsb->EP4S);
    pstcUsbIntern->astcEndpoints[5].pstcEpControlRegister  = &(pstcUsb->EP4C);
    pstcUsbIntern->astcEndpoints[5].pstcEpDataRegister     = &(pstcUsb->EP4DT);
    pstcUsbIntern->astcEndpoints[5].pfnRxTxCallback = NULL; 
    pstcUsbIntern->astcEndpoints[5].bAutomaticNullTermination = FALSE;
    pstcUsbIntern->astcEndpoints[5].pstcUsbInstance = pstcUsb;

    pstcUsbIntern->astcEndpoints[6].u8EndpointAddress = 0x05;
    pstcUsbIntern->astcEndpoints[6].u16EndpointSize   = 0x00;
    pstcUsbIntern->astcEndpoints[6].pstcEpStatusRegister   = &(pstcUsb->EP5S);
    pstcUsbIntern->astcEndpoints[6].pstcEpControlRegister  = &(pstcUsb->EP5C);
    pstcUsbIntern->astcEndpoints[6].pstcEpDataRegister     = &(pstcUsb->EP5DT);
    pstcUsbIntern->astcEndpoints[6].pfnRxTxCallback = NULL; 
    pstcUsbIntern->astcEndpoints[6].bAutomaticNullTermination = FALSE;
    pstcUsbIntern->astcEndpoints[6].pstcUsbInstance = pstcUsb;

#else
    pstcUsbIntern->astcEndpoints[0].u8EndpointAddress = 0x01;
    pstcUsbIntern->astcEndpoints[0].u16EndpointSize   = 0x00;
    pstcUsbIntern->astcEndpoints[0].pstcEpStatusRegister   = &(pstcUsb->EP1S);
    pstcUsbIntern->astcEndpoints[0].pstcEpControlRegister  = &(pstcUsb->EP1C);
    pstcUsbIntern->astcEndpoints[0].pstcEpDataRegister     = &(pstcUsb->EP1DT);
    pstcUsbIntern->astcEndpoints[0].pfnRxTxCallback = NULL;
    pstcUsbIntern->astcEndpoints[0].bAutomaticNullTermination = FALSE;
    pstcUsbIntern->astcEndpoints[0].pstcUsbInstance = pstcUsb;
    
    pstcUsbIntern->astcEndpoints[1].u8EndpointAddress = 0x02;
    pstcUsbIntern->astcEndpoints[1].u16EndpointSize   = 0x00;
    pstcUsbIntern->astcEndpoints[1].pstcEpStatusRegister   = &(pstcUsb->EP2S);
    pstcUsbIntern->astcEndpoints[1].pstcEpControlRegister  = &(pstcUsb->EP2C);
    pstcUsbIntern->astcEndpoints[1].pstcEpDataRegister     = &(pstcUsb->EP2DT);
    pstcUsbIntern->astcEndpoints[1].pfnRxTxCallback = NULL;
    pstcUsbIntern->astcEndpoints[1].bAutomaticNullTermination = FALSE;
    pstcUsbIntern->astcEndpoints[1].pstcUsbInstance = pstcUsb;
#endif
    
    return Ok;
}


/**
 ******************************************************************************
 ** \brief Deinit USB instance
 **
 ** \param pstcUsb Pointer to USB instance
 **
 ******************************************************************************/
en_result_t Usb_DeinitInstance(stc_usbn_t* pstcUsb) 
{
    pstcUsb->UDCC = 0xA0; // load initial values
    pstcUsb->EP0C = 0;    // load initial values
    pstcUsb->EP1C = 0;    // load initial values
    pstcUsb->EP2C = 0;    // load initial values 
    pstcUsb->EP3C = 0;    // load initial values 
    pstcUsb->EP4C = 0;    // load initial values 
    pstcUsb->EP5C = 0;    // load initial values 
    pstcUsb->UDCS = 0;    // load initial values 
    pstcUsb->UDCIE = 0;   // load initial values 
    pstcUsb->EP0IS = 0;   // load initial values 
    pstcUsb->EP0OS = 0;   // load initial values 
    pstcUsb->EP1S = 0;    // load initial values 
    pstcUsb->EP2S = 0;    // load initial values 
    pstcUsb->EP3S = 0;    // load initial values 
    pstcUsb->EP4S = 0;    // load initial values 
    pstcUsb->EP5S = 0;    // load initial values 
    pstcUsb->HCNT = 0;    // load initial values 
    pstcUsb->HIRQ = 0;    // load initial values 
    pstcUsb->HERR = 0;    // load initial values 
    pstcUsb->HSTATE = 0x10;  // load initial values 
    pstcUsb->HADR = 0x10;    // load initial values 
    return Ok;
}   

/**
 ******************************************************************************
 ** \brief Set RxTx Callback of an endpoint
 **
 ** \param pstcEndpoint Pointer to endpoint instance
 **
 ** \param pfnRxTxCallback Callback routine
 ** 
 ** \return none
 **
 ******************************************************************************/
void Usb_SetEndpointRxTxCallback(stc_usbn_endpoint_data_t* pstcEndpoint, usb_endpoint_datatransferred_func_ptr_t pfnRxTxCallback)
{
    BITMASK_CLEAR(*(pstcEndpoint->pstcEpStatusRegister),_EPNS_DRQIE);
    pstcEndpoint->pfnRxTxCallback = pfnRxTxCallback;
    if ((pfnRxTxCallback != NULL) && ((pstcEndpoint->u8EndpointAddress & 0x80) == 0))
    {
        BITMASK_SET(*(pstcEndpoint->pstcEpStatusRegister),_EPNS_DRQIE);
    }
}

/**
 ******************************************************************************
 ** \brief Get endpoint pointer from endpointaddress
 **
 ** \param pstcUsb Pointer to USB instance
 **
 ** \param u8EndpointAddress u8EndpointAddress
 ** 
 ** \return Pointer to endpoint, NULL if no enpoint was found
 **
 ******************************************************************************/
stc_usbn_endpoint_data_t* Usb_GetEndpointPtr(stc_usbn_t* pstcUsb, uint8_t u8EndpointAddress)
{
    stc_usb_intern_data_t* pstcUsbIntern;
    pstcUsbIntern = UsbGetInternDataPtr(pstcUsb);
    if (pstcUsbIntern == NULL)
    {
        return NULL;
    }
    if ((u8EndpointAddress & 0x1F) == 0)
    {
        if ((u8EndpointAddress & 0x80) > 0)
        {
            return &(pstcUsbIntern->astcEndpoints[0]);
        }
        else
        {
            return &(pstcUsbIntern->astcEndpoints[1]);
        }
    }
    else
    {
        if ((u8EndpointAddress & 0x1F) <= (pstcUsbIntern->u8NumberOfEndpoints - 2))
        {
            if ((pstcUsbIntern->astcEndpoints[(u8EndpointAddress & 0x1F) + 1].u8EndpointAddress) == u8EndpointAddress) 
            {
                return &(pstcUsbIntern->astcEndpoints[(u8EndpointAddress & 0x1F) + 1]);
            }
        }
    }
    return NULL;
}

/**
 ******************************************************************************
 ** \brief Stall endpoint.
 **
 ** \param pstcEpHandle Pointer to endpoint instance
 ** 
 ** \return Ok on success
 **
 ******************************************************************************/
en_result_t Usb_HalStallEndpoint(stc_usbn_endpoint_data_t* pstcEpHandle)
{
    BITMASK_SET(*(pstcEpHandle->pstcEpControlRegister), _EPNC_STAL);
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Unstall endpoint.
 **
 ** \param pstcEpHandle Pointer to endpoint instance
 ** 
 ** \return Ok on success
 **
 ******************************************************************************/
en_result_t Usb_HalUnstallEndpoint(stc_usbn_endpoint_data_t* pstcEpHandle)
{
    BITMASK_CLEAR(*(pstcEpHandle->pstcEpControlRegister), _EPNC_STAL);
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Unstall endpoint.
 **
 ** \param pstcEpHandle Pointer to endpoint instance
 ** 
 ** \return Ok on success
 **
 ******************************************************************************/
boolean_t Usb_HalEndpointIsBusy(stc_usbn_endpoint_data_t* pstcEpHandle)
{
    if (BITMASK_ISSET(*(pstcEpHandle->pstcEpStatusRegister), _EPNS_BUSY))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/**
 ******************************************************************************
 ** \brief Send data via endpoint.
 **
 ** \param pstcEpHandle Pointer to endpoint instance
 **
 ** \param pu8Data Data to write
 **
 ** \param u16Size Data size
 **
 ** \param pu16ByteCount Pointer to data written
 ** 
 ** \return Ok on success
 **
 ******************************************************************************/
en_result_t Usb_HalSend(stc_usbn_endpoint_data_t* pstcEpHandle, uint8_t* pu8Data, uint16_t u16Size, uint16_t* pu16ByteCount)
{
    boolean_t bEvenData = 0;
    USBDBGVAL8("HAL Snd ",pstcEpHandle->u8EndpointAddress);
    USBDBGVAL8("Sze ",u16Size);
    if ( pu16ByteCount != NULL )
    {
        *pu16ByteCount = 0;
    }
    
    if ( (pstcEpHandle == NULL) || (((pu8Data  == NULL) || (pu16ByteCount == NULL)) && (u16Size != 0)) )
    {
      return ErrorInvalidParameter ;
    }
    
    if (BITMASK_ISCLEARED(*(pstcEpHandle->pstcEpStatusRegister),_EPNS_DRQ)) 
    {
        return ErrorNotReady;
    }
    
    if (u16Size > pstcEpHandle->u16EndpointSize)
    {
        u16Size = pstcEpHandle->u16EndpointSize;
    }
    USBDBGVAL8("New Sze ",u16Size);
    bEvenData = ((u16Size % 2) == 0);
    
    if (u16Size == 0) 
    {
        bEvenData = TRUE;
    } 
    else
    {
        for(*pu16ByteCount = 0;*pu16ByteCount < (u16Size - 1);*pu16ByteCount = *pu16ByteCount + 2)
        {
            *(pstcEpHandle->pstcEpDataRegister) = *((uint16_t*)pu8Data);
            pu8Data += 2;
        }
    }
    
    if (!bEvenData)
    {
        *pu16ByteCount = *pu16ByteCount + 1;
        *((__IO uint8_t*)(pstcEpHandle->pstcEpDataRegister)) = *pu8Data;
    }
    
    BITMASK_CLEAR(*(pstcEpHandle->pstcEpStatusRegister), _EPNS_DRQ);
    return Ok;
}


#if USB_USES_DMA == ON
uint8_t u8EndpointBit;
/**
 ******************************************************************************
 ** \brief Send data via endpoint. (via DMA)
 **
 ** \param pstcEpHandle Pointer to endpoint instance
 **
 ** \param pu8Data Data to write
 **
 ** \param u16Size Data size
 **
 ** \param pu16ByteCount Pointer to data written
 ** 
 ** \return Ok on success
 **
 ******************************************************************************/

en_result_t Usb_HalSendDma(stc_usbn_endpoint_data_t* pstcEpHandle, uint8_t* pu8Data, uint16_t u16Size, uint16_t* pu16ByteCount)
{
    boolean_t bEvenData = FALSE;
    volatile uint32_t temp = 0;
    u8EndpointBit = (1 << (((pstcEpHandle->u8EndpointAddress) & 0x0F) - 1));
    uint8_t i;
    USBDBGVAL8("HAL Snd DMA ",pstcEpHandle->u8EndpointAddress);
    USBDBGVAL8("Sze ",u16Size);
    if ( pu16ByteCount != NULL )
    {
        *pu16ByteCount = 0;
    }
    
    if ( (pstcEpHandle == NULL) || (((pu8Data  == NULL) || (pu16ByteCount == NULL)) && (u16Size != 0)) )
    {
      return ErrorInvalidParameter ;
    }
    if ((u16Size % 2) == 0)
    {
        bEvenData = TRUE;
    }
    else
    {
        bEvenData = FALSE;
    }
    if (BITMASK_ISCLEARED(*(pstcEpHandle->pstcEpStatusRegister),_EPNS_DRQ)) 
    {
        return ErrorNotReady;
    }

    FM3_DMAC->DMACR |= 0x80000000;
    for(i = 0; i < USB_DMA_INSTANCE_COUNT;i++)
    {
        if(!(astcUsbDma[i].bIsActive))
        {
            NVIC_ClearPendingIRQ(astcUsbDma[i].u8IRQ);                                 // Setup DMA interrupt
            NVIC_DisableIRQ(astcUsbDma[i].u8IRQ);                                      // Disable DMA interrupt
            NVIC_SetPriority(astcUsbDma[i].u8IRQ,IRQ_LEVEL_USB0);                   // Setup DMA interrupt
            
            astcUsbDma[i].pstcEpHandle = pstcEpHandle;                                 // store used endpoint for this DMA channel
            pstcEpHandle->bIsActive = TRUE;                                            // mark enpoint as in use
            astcUsbDma[i].bIsActive = TRUE;                                            // mark DMA channel as in use
            *(astcUsbDma[i].pstcDMACA) &= ~(1 << 31);                                  // clear enable flag for this channel
            
            BITMASK_SET(*(pstcEpHandle->pstcEpControlRegister),_EPNC_DMAE);            // Setup endpoint for DMA usage
            BITMASK_SET(*(pstcEpHandle->pstcEpControlRegister),_EP1C_NULE);            // Setup endpoint for DMA usage
            
            *(astcUsbDma[i].pstcDMACB)= (2 << 28) | (1 << 26) | (1 << 24);             // Demand transfer mode, half-word, fixed source
            
            temp = (((pstcEpHandle->u8EndpointAddress) & 0x0F) - 1) | 0x20;            // IDREQ input select settings
            temp = temp << 23;                                                         // IDREQ input select settings
            #if FM3MCUTYPE != FM3MCUTYPE_TYPE0
                #if (FM_PERIPHERAL_ENABLE_USB0 == ON)
                    if (pstcEpHandle->pstcUsbInstance == &USB0)
                    {
                        if (bEvenData)
                        {
                            //FM3_INTREQ->ODDPKS &= ~(u8EndpointBit << 24);
                            temp += (u16Size / 2) - 1;
                            FM3_INTREQ->ODDPKS = 0x0;
                        }
                        else
                        {
                            //FM3_INTREQ->ODDPKS |= (uint32_t)(u8EndpointBit << 24);
                            FM3_INTREQ->ODDPKS = 0xFF;
                            temp += (u16Size / 2);
                        }
                    } 
                #endif
                #if (FM_PERIPHERAL_ENABLE_USB1 == ON)
                    if (pstcEpHandle->pstcUsbInstance == &USB1)
                    {
                        if (bEvenData)
                        {
                            FM3_INTREQ->ODDPKS1 &= ~(u8EndpointBit << 24);
                            temp += (u16Size / 2) - 1;
                        }
                        else
                        {
                            FM3_INTREQ->ODDPKS1 |= (u8EndpointBit << 24);
                            temp += (u16Size / 2);
                        }
                    }
                #endif
                *pu16ByteCount = u16Size; 
            #else
                temp += (u16Size / 2) - 1;
                *pu16ByteCount = (u16Size / 2) * 2;  // update data transferred information
            #endif
            // IDREQ input select settings
            *(astcUsbDma[i].pstcDMACA) = temp;                                         
            
            *(astcUsbDma[i].pstcDMACSA) = (uint32_t)pu8Data;                                                  // data source
            *(astcUsbDma[i].pstcDMACDA) = (uint32_t)(pstcEpHandle->pstcEpDataRegister);                       // endpoint data register as data destination
            FM3_INTREQ->DRQSEL |= (uint32_t)(1 << ((uint32_t)(pstcEpHandle->u8EndpointAddress) & 0x0F) - 1);  // select DMA usage instead of IRQ for endpoint
            BITMASK_SET(*(pstcEpHandle->pstcEpControlRegister),_EPNS_DRQIE);                                  // enable interrupt handling for this endpoint
            *(astcUsbDma[i].pstcDMACB) |= (uint32_t)(1 << 19);                                                // DMA completion interrupt enabled
            
            NVIC_ClearPendingIRQ(astcUsbDma[i].u8IRQ);                                                        // Setup DMA interrupt
            NVIC_EnableIRQ(astcUsbDma[i].u8IRQ);                                                              // Setup DMA interrupt
            NVIC_SetPriority(astcUsbDma[i].u8IRQ,IRQ_LEVEL_USB0);                                          // Setup DMA interrupt  
            *(astcUsbDma[i].pstcDMACA) |= (uint32_t)(1 << 31);                                                // DMA enable channel
                                                                                                                         
            return Ok;
        }
    }
    return ErrorNotReady;
}
#endif

/**
 ******************************************************************************
 ** \brief Read data via endpoint.
 **
 ** \param pstcEpHandle Pointer to endpoint instance
 **
 ** \param pu8Data Databuffer to write in
 **
 ** \param pu16ByteCount Pointer to data read
 **
 ** \param bClearDrq Set FIFO ready for next data
 ** 
 ** \return Ok on success
 **
 ******************************************************************************/
en_result_t Usb_HalReceive(stc_usbn_endpoint_data_t* pstcEpHandle, uint8_t* pu8Data, uint16_t* pu16ByteCount, boolean_t bClearDrq)
{
    boolean_t bEvenData = 0;
    
    uint16_t u16i;
    
    if ( pu16ByteCount != NULL )
    {
        *pu16ByteCount = 0;
    }
    
    if (pstcEpHandle == NULL)
    {
      return ErrorInvalidParameter ;
    }
    
    if (BITMASK_ISCLEARED(*(pstcEpHandle->pstcEpStatusRegister),_EPNS_DRQ)) 
    {
        return ErrorNotReady;
    }
    
    *pu16ByteCount = (*(pstcEpHandle->pstcEpStatusRegister)) & 0x7F;
    
    bEvenData = (((*pu16ByteCount) % 2) == 0);
    
    if (*pu16ByteCount == 0) 
    {
        bEvenData = TRUE;
    } 
    else
    {
        for(u16i = 0;u16i < (*pu16ByteCount - 1);u16i += 2)
        {
            *((uint16_t*)pu8Data) = *(pstcEpHandle->pstcEpDataRegister);
            pu8Data += 2;
        }
    }
    
    if (!bEvenData)
    {
        *pu8Data = *((__IO uint8_t*)(pstcEpHandle->pstcEpDataRegister));
    }
    if (bClearDrq)
    {
        BITMASK_CLEAR(*(pstcEpHandle->pstcEpStatusRegister), _EPNS_DRQ);
    }
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Clear Drq flag of FIFO: FIFO ready for next data
 **
 ** \param pstcEpHandle Pointer to endpoint instance
 ** 
 ** \return Ok on success
 **
 ******************************************************************************/
en_result_t Usb_HalClearDrq(stc_usbn_endpoint_data_t* pstcEpHandle)
{
    if (pstcEpHandle == NULL)
    {
      return ErrorInvalidParameter ;
    }
    
    if (BITMASK_ISCLEARED(*(pstcEpHandle->pstcEpStatusRegister),_EPNS_DRQ)) 
    {
        return ErrorNotReady;
    }
    BITMASK_CLEAR(*(pstcEpHandle->pstcEpStatusRegister), _EPNS_DRQ);
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Get information about the specified endpoint status
 **
 ** \param pstcEpHandle Endpoint handle 
 **
 ** \return Status
 **
 ******************************************************************************/
en_usb_endpoint_status_t Usb_HalEndpointStatus(stc_usbn_endpoint_data_t* pstcEpHandle)
{
    if (pstcEpHandle == NULL)
    {
        return UsbEndpointStatusNotReady;
    }
    
    if (BITMASK_ISCLEARED(*(pstcEpHandle->pstcEpControlRegister),_EPNC_EPEN))
    {
        return UsbEndpointStatusNotReady;
    }
    
    if (BITMASK_ISSET(*(pstcEpHandle->pstcEpControlRegister), _EPNC_STAL))
    {
        return UsbEndpointStatusStall;
    }
    

    if (BITMASK_ISCLEARED(*(pstcEpHandle->pstcEpStatusRegister), _EPNS_DRQ)) 
    {
        return UsbEndpointStatusTransfer;
    }
    else
    {
        return UsbEndpointStatusIdle;
    }
   
    
    //return UsbEndpointStatusIdle;
}

/**
 ******************************************************************************
 ** \brief Get information if the specified endpoint is ready
 **
 ** \param pstcEpHandle Endpoint handle 
 **
 ** \return TRUE if ready
 **
 ******************************************************************************/
boolean_t Usb_HalEpReady(stc_usbn_endpoint_data_t* pstcEpHandle)
{
    if (pstcEpHandle == NULL)
    {
        return FALSE;
    }
    
    if (BITMASK_ISCLEARED(*(pstcEpHandle->pstcEpControlRegister),_EPNC_EPEN))
    {
        return FALSE;
    }

    if (BITMASK_ISCLEARED(*(pstcEpHandle->pstcEpStatusRegister), _EPNS_DRQ)) 
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
    
}

/**
 ******************************************************************************
 ** Usb Device get current configuration
 ** 
 ** \param pstcUsb USB handle
 **
 *****************************************************************************/
uint8_t Usb_GetConfiguration(stc_usbn_t* pstcUsb)
{
    if (pstcUsb->UDCC_f.HCONX == 1)
    {
        return 0;
    }
    return pstcUsb->UDCIE_f.CONFN;
}


/**
 ******************************************************************************
 ** SOF callback - Realize Timeouts
 ** 
 ** \param pstcUsb USB handle
 **
 *****************************************************************************/
static void Usb_TimeoutHandler(stc_usbn_t* pstcUsb)
{
    volatile uint8_t i;
    boolean_t bNothingToBeDone = TRUE;
    stc_usb_intern_data_t* pstcUsbIntern;
    pstcUsbIntern = UsbGetInternDataPtr(pstcUsb);
    if (pstcUsbIntern == NULL)
    {
        return;
    }
    for(i = 0;i<5;i++)
    {
        if (pstcUsbIntern->pstcUsbTimeOutHandler[i].Handler != 0)
        {
            bNothingToBeDone = FALSE;
            if (pstcUsbIntern->pstcUsbTimeOutHandler[i].u16TimeOut == 0)
            {
                pstcUsbIntern->pstcUsbTimeOutHandler[i].Handler(pstcUsb);
                pstcUsbIntern->pstcUsbTimeOutHandler[i].Handler = 0;
                return;
            }
            else
            {
                pstcUsbIntern->pstcUsbTimeOutHandler[i].u16TimeOut--;
            }
        }
    }
    if (bNothingToBeDone == TRUE)
    {
         pstcUsbIntern->bSofTimeoutEnabled = FALSE;
    }
}

/**
 ******************************************************************************
 ** Add a timeout handler
 **
 ** \param pstcUsb USB handle
 ** 
 ** \param Handler handler executed after timeout
 ** 
 ** \param u16TimeOut timeout in ms
 **
 ** \return TRUE if command was successful
 **
 *****************************************************************************/
boolean_t Usb_AddTimeOut(stc_usbn_t* pstcUsb, void (* Handler)(stc_usbn_t* pstcUsb), uint16_t u16TimeOut)
{
    volatile uint8_t i;
    boolean_t bAdded = FALSE;
    stc_usb_intern_data_t* pstcUsbIntern;
    pstcUsbIntern = UsbGetInternDataPtr(pstcUsb);
    if (pstcUsbIntern == NULL)
    {
        return ErrorUninitialized;
    }
    for(i = 0;i<5;i++)
    {
        if (pstcUsbIntern->pstcUsbTimeOutHandler[i].Handler == 0)
        {
            pstcUsbIntern->bSofTimeoutEnabled = FALSE;
            pstcUsbIntern->pstcUsbTimeOutHandler[i].Handler = Handler;
            pstcUsbIntern->pstcUsbTimeOutHandler[i].u16TimeOut = u16TimeOut;
            pstcUsbIntern->bSofTimeoutEnabled = TRUE;
            bAdded = TRUE;
            break;
        }
    }
    pstcUsbIntern->bSofTimeoutEnabled = TRUE;
    return bAdded;
}

/**
 ******************************************************************************
 ** Remove a timeout handler
 **
 ** \param pstcUsb USB handle
 ** 
 ** \param Handler handler executed after timeout
 ** 
 ** \return none
 **
 *****************************************************************************/
void Usb_RemoveTimeOut(stc_usbn_t* pstcUsb, void (* Handler)(stc_usbn_t* pstcUsb))
{
    volatile uint8_t i;
    stc_usb_intern_data_t* pstcUsbIntern;
    pstcUsbIntern = UsbGetInternDataPtr(pstcUsb);
    if (pstcUsbIntern == NULL)
    {
        return;
    }
    for(i = 0;i<5;i++)
    {
        if (pstcUsbIntern->pstcUsbTimeOutHandler[i].Handler == Handler)
        {
            pstcUsbIntern->pstcUsbTimeOutHandler[i].Handler = 0;
            pstcUsbIntern->pstcUsbTimeOutHandler[i].u16TimeOut = 0;
            break;
        }
    }
}

/******************************************************************************/
/* Interrupt Handling                                                         */
/******************************************************************************/

void Usb_Tick(void)
{
	  #if (FM_PERIPHERAL_ENABLE_USB0 == ON)
	      #if (FM_PERIPHERAL_USB_DEVICE_ENABLED == ON)
	         UsbIrqHandlerF((stc_usbn_t*)&USB0, &(m_astcUsbInstanceDataLut[UsbInstanceIndexUsb0].stcInternData));
	      #endif
	      UsbIrqHandler((stc_usbn_t*)&USB0, &(m_astcUsbInstanceDataLut[UsbInstanceIndexUsb0].stcInternData));
	  #endif
	  #if (FM_PERIPHERAL_ENABLE_USB1 == ON)
	      #if (FM_PERIPHERAL_USB_DEVICE_ENABLED == ON)
	         UsbIrqHandlerF((stc_usbn_t*)&USB1, &(m_astcUsbInstanceDataLut[UsbInstanceIndexUsb1].stcInternData));
	      #endif
	      UsbIrqHandler((stc_usbn_t*)&USB1, &(m_astcUsbInstanceDataLut[UsbInstanceIndexUsb1].stcInternData));
	  #endif
}

                  
#if (USB_USE_STANDALONE == ON) || (USB_USE_PDL == 1)
  #if (FM_PERIPHERAL_ENABLE_USB0 == ON)
  
    #if (FM_PERIPHERAL_USB_DEVICE_ENABLED == ON)
    /**
     ******************************************************************************
     ** USB 0 function interrupt handler (only used in standalone mode)
     **
     *****************************************************************************/
    void USBF_Handler(void)
    {
        UsbIrqHandlerF((stc_usbn_t*)&USB0, &(m_astcUsbInstanceDataLut[UsbInstanceIndexUsb0].stcInternData));
    }
    #endif
    
    /**
     ******************************************************************************
     ** USB 0 interrupt handler (only used in standalone mode)
     **
     *****************************************************************************/
    void USB_Handler(void)
    {
        UsbIrqHandler((stc_usbn_t*)&USB0, &(m_astcUsbInstanceDataLut[UsbInstanceIndexUsb0].stcInternData));
    }
    
    #if (FM_PERIPHERAL_USB_DEVICE_ENABLED == ON)
    /**
     ******************************************************************************
     ** USB 0 function interrupt handler (only used in standalone mode)
     **
     *****************************************************************************/
    void USB0F_Handler(void)
    {
        UsbIrqHandlerF((stc_usbn_t*)&USB0, &(m_astcUsbInstanceDataLut[UsbInstanceIndexUsb0].stcInternData));
    }
    #endif
    
    /**
     ******************************************************************************
     ** USB 0 interrupt handler (only used in standalone mode)
     **
     *****************************************************************************/
    void USB0_Handler(void)
    {
        UsbIrqHandler((stc_usbn_t*)&USB0, &(m_astcUsbInstanceDataLut[UsbInstanceIndexUsb0].stcInternData));
    }

    #if (FM_PERIPHERAL_USB_DEVICE_ENABLED == ON) && (USB_USE_PDL == 0)
    /**
     ******************************************************************************
     ** USB 0 function interrupt handler (only used in standalone mode)
     **
     *****************************************************************************/
    void IRQ078_Handler(void)
    {
        UsbIrqHandlerF((stc_usbn_t*)&USB0, &(m_astcUsbInstanceDataLut[UsbInstanceIndexUsb0].stcInternData));
    }
    #endif
    
    #if (USB_USE_PDL == 0)
    /**
     ******************************************************************************
     ** USB 0 interrupt handler (only used in standalone mode)
     **
     *****************************************************************************/
    void IRQ079_Handler(void)
    {
        UsbIrqHandler((stc_usbn_t*)&USB0, &(m_astcUsbInstanceDataLut[UsbInstanceIndexUsb0].stcInternData));
    }
    #endif

    #if (FM_PERIPHERAL_USB_DEVICE_ENABLED == ON)
    void USB0_IRQHandler(void)
    {
        UsbIrqHandlerF((stc_usbn_t*)&USB0, &(m_astcUsbInstanceDataLut[UsbInstanceIndexUsb0].stcInternData));
    }
    #endif

    void USB0_HOST_IRQHandler(void)
    {
        UsbIrqHandler((stc_usbn_t*)&USB0, &(m_astcUsbInstanceDataLut[UsbInstanceIndexUsb0].stcInternData));
    }
  #endif
  #if (FM_PERIPHERAL_ENABLE_USB1 == ON)
    #if (FM_PERIPHERAL_USB_DEVICE_ENABLED == ON)

    /**
     ******************************************************************************
     ** USB 1 function interrupt handler (only used in standalone mode)
     **
     *****************************************************************************/
    void USB1F_Handler(void)
    {
        UsbIrqHandlerF((stc_usbn_t*)&USB1, &(m_astcUsbInstanceDataLut[UsbInstanceIndexUsb1].stcInternData));
    }
    #endif
    
    
    /**
     ******************************************************************************
     ** USB 1 interrupt handler (only used in standalone mode)
     **
     *****************************************************************************/
    void USB1_Handler(void)
    {
        UsbIrqHandler((stc_usbn_t*)&USB1, &(m_astcUsbInstanceDataLut[UsbInstanceIndexUsb1].stcInternData));
    }
  #endif
  #if USB_USES_DMA == ON
  #if USB_USES_DMA_0 == ON
  void DMAC0_Handler(void)
  {
      FM3_INTREQ->DRQSEL &= (uint32_t)~(1 << ((uint32_t)(astcUsbDma[UsbDmaInstanceIndex0].pstcEpHandle->u8EndpointAddress) & 0x0F) - 1);
      BITMASK_CLEAR(*(astcUsbDma[UsbDmaInstanceIndex0].pstcEpHandle->pstcEpControlRegister),_EPNC_DMAE);
      BITMASK_CLEAR(*(astcUsbDma[UsbDmaInstanceIndex0].pstcEpHandle->pstcEpControlRegister),_EP1C_NULE);
      *(astcUsbDma[UsbDmaInstanceIndex0].pstcDMACA) &= (uint32_t)(1 << 31); // disable channel
      *(astcUsbDma[UsbDmaInstanceIndex0].pstcDMACB) &= (uint32_t)(1 << 19); // disable interrupt
      astcUsbDma[UsbDmaInstanceIndex0].bIsActive = FALSE;
      UsbIrqHandlerF((stc_usbn_t*)astcUsbDma[UsbDmaInstanceIndex0].pstcEpHandle->pstcUsbInstance, &(m_astcUsbInstanceDataLut[UsbDmaInstanceIndex0].stcInternData));
  }
  #endif
  #if USB_USES_DMA_1 == ON
  void DMAC1_Handler(void)
  {
      FM3_INTREQ->DRQSEL &= (uint32_t)~(1 << ((uint32_t)(astcUsbDma[UsbDmaInstanceIndex1].pstcEpHandle->u8EndpointAddress) & 0x0F) - 1);
      BITMASK_CLEAR(*(astcUsbDma[UsbDmaInstanceIndex1].pstcEpHandle->pstcEpControlRegister),_EPNC_DMAE);
      BITMASK_CLEAR(*(astcUsbDma[UsbDmaInstanceIndex1].pstcEpHandle->pstcEpControlRegister),_EP1C_NULE);
      *(astcUsbDma[UsbDmaInstanceIndex1].pstcDMACA) &= (uint32_t)(1 << 31); // disable channel
      *(astcUsbDma[UsbDmaInstanceIndex1].pstcDMACB) &= (uint32_t)(1 << 19); // disable interrupt
      astcUsbDma[UsbDmaInstanceIndex1].bIsActive = FALSE;
      UsbIrqHandlerF((stc_usbn_t*)astcUsbDma[UsbDmaInstanceIndex1].pstcEpHandle->pstcUsbInstance, &(m_astcUsbInstanceDataLut[UsbDmaInstanceIndex1].stcInternData));
  }
  #endif
  #if USB_USES_DMA_2 == ON
  void DMAC2_Handler(void)
  {
      FM3_INTREQ->DRQSEL &= (uint32_t)~(1 << ((uint32_t)(astcUsbDma[UsbDmaInstanceIndex2].pstcEpHandle->u8EndpointAddress) & 0x0F) - 1);
      BITMASK_CLEAR(*(astcUsbDma[UsbDmaInstanceIndex2].pstcEpHandle->pstcEpControlRegister),_EPNC_DMAE);
      BITMASK_CLEAR(*(astcUsbDma[UsbDmaInstanceIndex2].pstcEpHandle->pstcEpControlRegister),_EP1C_NULE);
      *(astcUsbDma[UsbDmaInstanceIndex2].pstcDMACA) &= (uint32_t)(1 << 31); // disable channel
      *(astcUsbDma[UsbDmaInstanceIndex2].pstcDMACB) &= (uint32_t)(1 << 19); // disable interrupt
      astcUsbDma[UsbDmaInstanceIndex2].bIsActive = FALSE;
      UsbIrqHandlerF((stc_usbn_t*)astcUsbDma[UsbDmaInstanceIndex2].pstcEpHandle->pstcUsbInstance, &(m_astcUsbInstanceDataLut[UsbDmaInstanceIndex2].stcInternData));
  }
  #endif
  #if USB_USES_DMA_3 == ON
  void DMAC3_Handler(void)
  {
      FM3_INTREQ->DRQSEL &= (uint32_t)~(1 << ((uint32_t)(astcUsbDma[UsbDmaInstanceIndex3].pstcEpHandle->u8EndpointAddress) & 0x0F) - 1);
      BITMASK_CLEAR(*(astcUsbDma[UsbDmaInstanceIndex3].pstcEpHandle->pstcEpControlRegister),_EPNC_DMAE);
      BITMASK_CLEAR(*(astcUsbDma[UsbDmaInstanceIndex3].pstcEpHandle->pstcEpControlRegister),_EP1C_NULE);
      *(astcUsbDma[UsbDmaInstanceIndex3].pstcDMACA) &= (uint32_t)(1 << 31); // disable channel
      *(astcUsbDma[UsbDmaInstanceIndex3].pstcDMACB) &= (uint32_t)(1 << 19); // disable interrupt
      astcUsbDma[UsbDmaInstanceIndex3].bIsActive = FALSE;
      UsbIrqHandlerF((stc_usbn_t*)astcUsbDma[UsbDmaInstanceIndex3].pstcEpHandle->pstcUsbInstance, &(m_astcUsbInstanceDataLut[UsbDmaInstanceIndex3].stcInternData));
  }
  #endif
  #if USB_USES_DMA_4 == ON
  void DMAC4_Handler(void)
  {
      FM3_INTREQ->DRQSEL &= (uint32_t)~(1 << ((uint32_t)(astcUsbDma[UsbDmaInstanceIndex4].pstcEpHandle->u8EndpointAddress) & 0x0F) - 1);
      BITMASK_CLEAR(*(astcUsbDma[UsbDmaInstanceIndex4].pstcEpHandle->pstcEpControlRegister),_EPNC_DMAE);
      BITMASK_CLEAR(*(astcUsbDma[UsbDmaInstanceIndex4].pstcEpHandle->pstcEpControlRegister),_EP1C_NULE);
      *(astcUsbDma[UsbDmaInstanceIndex4].pstcDMACA) &= (uint32_t)(1 << 31); // disable channel
      *(astcUsbDma[UsbDmaInstanceIndex4].pstcDMACB) &= (uint32_t)(1 << 19); // disable interrupt
      astcUsbDma[UsbDmaInstanceIndex4].bIsActive = FALSE;
      UsbIrqHandlerF((stc_usbn_t*)astcUsbDma[UsbDmaInstanceIndex4].pstcEpHandle->pstcUsbInstance, &(m_astcUsbInstanceDataLut[UsbDmaInstanceIndex4].stcInternData));
  }  
  #endif
  #if USB_USES_DMA_5 == ON
  void DMAC5_Handler(void)
  {
      FM3_INTREQ->DRQSEL &= (uint32_t)~(1 << ((uint32_t)(astcUsbDma[UsbDmaInstanceIndex5].pstcEpHandle->u8EndpointAddress) & 0x0F) - 1);
      BITMASK_CLEAR(*(astcUsbDma[UsbDmaInstanceIndex5].pstcEpHandle->pstcEpControlRegister),_EPNC_DMAE);
      BITMASK_CLEAR(*(astcUsbDma[UsbDmaInstanceIndex5].pstcEpHandle->pstcEpControlRegister),_EP1C_NULE);
      *(astcUsbDma[UsbDmaInstanceIndex5].pstcDMACA) &= (uint32_t)(1 << 31); // disable channel
      *(astcUsbDma[UsbDmaInstanceIndex5].pstcDMACB) &= (uint32_t)(1 << 19); // disable interrupt
      astcUsbDma[UsbDmaInstanceIndex5].bIsActive = FALSE;
      UsbIrqHandlerF((stc_usbn_t*)astcUsbDma[UsbDmaInstanceIndex5].pstcEpHandle->pstcUsbInstance, &(m_astcUsbInstanceDataLut[UsbDmaInstanceIndex5].stcInternData));
  } 
  #endif
  #if USB_USES_DMA_6 == ON
  void DMAC6_Handler(void)
  {
      FM3_INTREQ->DRQSEL &= (uint32_t)~(1 << ((uint32_t)(astcUsbDma[UsbDmaInstanceIndex6].pstcEpHandle->u8EndpointAddress) & 0x0F) - 1);
      BITMASK_CLEAR(*(astcUsbDma[UsbDmaInstanceIndex6].pstcEpHandle->pstcEpControlRegister),_EPNC_DMAE);
      BITMASK_CLEAR(*(astcUsbDma[UsbDmaInstanceIndex6].pstcEpHandle->pstcEpControlRegister),_EP1C_NULE);
      *(astcUsbDma[UsbDmaInstanceIndex6].pstcDMACA) &= (uint32_t)(1 << 31); // disable channel
      *(astcUsbDma[UsbDmaInstanceIndex6].pstcDMACB) &= (uint32_t)(1 << 19); // disable interrupt
      astcUsbDma[UsbDmaInstanceIndex6].bIsActive = FALSE;
      UsbIrqHandlerF((stc_usbn_t*)astcUsbDma[UsbDmaInstanceIndex6].pstcEpHandle->pstcUsbInstance, &(m_astcUsbInstanceDataLut[UsbDmaInstanceIndex6].stcInternData));
  } 
  #endif
  #if USB_USES_DMA_7 == ON
  void DMAC7_Handler(void)
  {
      FM3_INTREQ->DRQSEL &= (uint32_t)~(1 << ((uint32_t)(astcUsbDma[UsbDmaInstanceIndex7].pstcEpHandle->u8EndpointAddress) & 0x0F) - 1);
      BITMASK_CLEAR(*(astcUsbDma[UsbDmaInstanceIndex7].pstcEpHandle->pstcEpControlRegister),_EPNC_DMAE);
      BITMASK_CLEAR(*(astcUsbDma[UsbDmaInstanceIndex7].pstcEpHandle->pstcEpControlRegister),_EP1C_NULE);
      *(astcUsbDma[UsbDmaInstanceIndex7].pstcDMACA) &= (uint32_t)(1 << 31); // disable channel
      *(astcUsbDma[UsbDmaInstanceIndex7].pstcDMACB) &= (uint32_t)(1 << 19); // disable interrupt
      astcUsbDma[UsbDmaInstanceIndex7].bIsActive = FALSE;
      UsbIrqHandlerF((stc_usbn_t*)astcUsbDma[UsbDmaInstanceIndex7].pstcEpHandle->pstcUsbInstance, &(m_astcUsbInstanceDataLut[UsbDmaInstanceIndex7].stcInternData));
  }
  #endif
  #endif
#endif

#if (FM_PERIPHERAL_USB_DEVICE_ENABLED == ON)
/**
 ******************************************************************************
 ** USB function interrupt handler (used for HAL endpoint interrupts)
 **
 ** \param pstcUsb USB handle
 **
 ** \param pstcUsbInternData Internal data USB handle 
 **
 *****************************************************************************/
void UsbIrqHandlerF(stc_usbn_t* pstcUsb, stc_usb_intern_data_t* pstcUsbInternData)
{ 
    volatile uint32_t i;
    uint32_t temp;

    for(i = 2;i < pstcUsbInternData->u8NumberOfEndpoints;i++)
    {
        if ((BITMASK_ISSET(*(pstcUsbInternData->astcEndpoints[i].pstcEpStatusRegister),_EPNS_DRQ)) && (BITMASK_ISSET(*(pstcUsbInternData->astcEndpoints[i].pstcEpStatusRegister),_EPNS_DRQIE)))
        {
            while(pstcUsbInternData->astcEndpoints[i].bIsActive == TRUE)
            {
                pstcUsbInternData->astcEndpoints[i].bIsActive = FALSE;
            }
            if (pstcUsbInternData->astcEndpoints[i].pfnRxTxCallback != NULL)
            {
                temp = i; //needed to avoid warning, because of unknown volatile access
                pstcUsbInternData->astcEndpoints[(uint8_t)temp].pfnRxTxCallback(pstcUsb,&(pstcUsbInternData->astcEndpoints[(uint8_t)temp]));
            }
        }
    }
}
#endif

/**
 ******************************************************************************
 ** Operation System Tick Handle
 *****************************************************************************/
void Usb_OsTickHandle(void)
{
    #if (FM_PERIPHERAL_USB_DEVICE_ENABLED == ON)
        #if ((FM_PERIPHERAL_ENABLE_USB0 == ON) && (defined(USB0)))
            UsbIrqHandlerF((stc_usbn_t*)&USB0,&(m_astcUsbInstanceDataLut[UsbInstanceIndexUsb0].stcInternData));
        #endif
        #if ((FM_PERIPHERAL_ENABLE_USB1 == ON) && (defined(USB1)))
            UsbIrqHandlerF((stc_usbn_t*)&USB1,&(m_astcUsbInstanceDataLut[UsbInstanceIndexUsb1].stcInternData));
        #endif
    #endif   
    #if ((FM_PERIPHERAL_ENABLE_USB0 == ON) && (defined(USB0)))
        UsbIrqHandler((stc_usbn_t*)&USB0,&(m_astcUsbInstanceDataLut[UsbInstanceIndexUsb0].stcInternData));
    #endif
    #if ((FM_PERIPHERAL_ENABLE_USB1 == ON) && (defined(USB1)))
        UsbIrqHandler((stc_usbn_t*)&USB1,&(m_astcUsbInstanceDataLut[UsbInstanceIndexUsb1].stcInternData));
    #endif
}

/**
 ******************************************************************************
 ** USB interrupt handler
 **
 ** \param pstcUsb USB handle
 **
 ** \param pstcUsbInternData Internal data USB handle 
 **
 *****************************************************************************/
void UsbIrqHandler(stc_usbn_t* pstcUsb, stc_usb_intern_data_t* pstcUsbInternData)
{
    volatile uint32_t i;
    
    if (pstcUsbInternData->bDeviceActive)
    {
        if (pstcUsb->UDCS_f.CONF == 1)
        {
            pstcUsb->UDCS_f.CONF = 0;
            if (pstcUsbInternData->pfnConfCallback != NULL)
            {
                pstcUsbInternData->pfnConfCallback(pstcUsb);
            }
        }
        
        
        if (pstcUsb->EP0C_f.STAL == 1)
        {
            pstcUsb->EP0C_f.STAL = 0;
        }
        
        if ((pstcUsb->UDCS_f.SETP == 1) && (pstcUsb->EP0OS_f.DRQOIE == 0))
        {
            pstcUsb->EP0OS_f.DRQOIE = 1;
        }
             
        if ((pstcUsb->EP0OS_f.DRQO == 1) && (pstcUsb->EP0OS_f.DRQOIE == 1))
        {
            if (pstcUsb->UDCS_f.SETP == 1)
            {
                pstcUsb->UDCS_f.SETP = 0;
                if (pstcUsbInternData->pfnSetpCallback != NULL)
                {
                    USBDBG(">> SETP\n");
                    pstcUsbInternData->pfnSetpCallback(pstcUsb);
                }
            }
            else
            {
                if ((pstcUsb->EP0OS & 0x1F) == 0)
                {
                   USBDBG(" zerro package "); 
                   // Zerro Package Received
                }
                if (pstcUsbInternData->astcEndpoints[1].pfnRxTxCallback != NULL)
                {
                    pstcUsbInternData->astcEndpoints[1].pfnRxTxCallback(pstcUsb,&(pstcUsbInternData->astcEndpoints[1]));
                }
                else
                {
                    USBDBG("EP0");
                    if ((pstcUsb->EP0OS & 0x1F) == 0)
                    {
                       USBDBG(" zerro package "); 
                       // Zerro Package Received
                    }
                    pstcUsb->EP0OS_f.DRQO = 0;
                    USBDBG("received (not handled)\n");
                }
            }
        }
    
        if ((pstcUsb->EP0IS_f.DRQI == 1) && (pstcUsb->EP0IS_f.DRQIIE == 1))
        {
            if (pstcUsbInternData->astcEndpoints[0].pfnRxTxCallback != NULL)
            {
                pstcUsbInternData->astcEndpoints[0].pfnRxTxCallback(pstcUsb,&(pstcUsbInternData->astcEndpoints[0]));
            }
        }
        
        if (pstcUsb->UDCS_f.WKUP == 1)
        {
            pstcUsb->UDCS_f.WKUP = 0;
            if (pstcUsbInternData->pfnWkupCallback != NULL)
            {
                pstcUsbInternData->pfnWkupCallback(pstcUsb);
            }
        }
        if (pstcUsb->UDCS_f.BRST == 1)
        {
            pstcUsb->UDCS_f.BRST = 0;
            for (i = pstcUsbInternData->u8NumberOfEndpoints; i > 0;i--)
            {
                BITMASK_SET(*((uint16_t*)pstcUsbInternData->astcEndpoints[i - 1].pstcEpStatusRegister),_EPNS_BFINI);
                BITMASK_CLEAR(*((uint16_t*)pstcUsbInternData->astcEndpoints[i - 1].pstcEpStatusRegister),_EPNS_BFINI);
            }
            pstcUsb->EP0IS_f.DRQIIE  = 0;
            pstcUsb->EP0OS_f.DRQOIE  = 1;
            if (pstcUsbInternData->pfnBrstCallback != NULL)
            {
                pstcUsbInternData->pfnBrstCallback(pstcUsb);
            }
        }
        if (pstcUsb->UDCS_f.SOF == 1)
        {
            pstcUsb->UDCS_f.SOF = 0;
            if (pstcUsbInternData->bSofTimeoutEnabled == TRUE)
            {
                Usb_TimeoutHandler(pstcUsb);
            }
            
            if (pstcUsbInternData->pfnSofCallback != NULL)
            {
                pstcUsbInternData->pfnSofCallback(pstcUsb);
            }
        }
        if (pstcUsb->UDCS_f.SUSP == 1)
        {
            pstcUsb->UDCS_f.SUSP = 0;
            if (pstcUsbInternData->pfnSuspCallback != NULL)
            {
                pstcUsbInternData->pfnSuspCallback(pstcUsb);
            }
        }
    }
    if (pstcUsbInternData->bHostActive)
    {
        if (pstcUsb->HIRQ_f.CMPIRQ == 1)
        {
            pstcUsb->HIRQ_f.CMPIRQ = 0;
            if (pstcUsbInternData->pfnCmpirqCallback != NULL)
            {
                pstcUsbInternData->pfnCmpirqCallback(pstcUsb);
            }
        }
        if (pstcUsb->HIRQ_f.SOFIRQ == 1)
        {
            pstcUsb->HIRQ_f.SOFIRQ = 0;
            
            if (pstcUsbInternData->bSofTimeoutEnabled == TRUE)
            {
                Usb_TimeoutHandler(pstcUsb);
            }
            
            if (pstcUsbInternData->pfnSofCallback != NULL)
            {
                pstcUsbInternData->pfnSofCallback(pstcUsb);
            }
        }
        if (pstcUsb->HIRQ_f.DIRQ == 1)
        {
            pstcUsb->HIRQ_f.DIRQ = 0;
            if (pstcUsbInternData->pfnDirqCallback != NULL)
            {
                pstcUsbInternData->pfnDirqCallback(pstcUsb);
            }
        }
        if (pstcUsb->HIRQ_f.CNNIRQ == 1)
        {
            if (pstcUsbInternData->pfnCnnirqCallback != NULL)
            {
                pstcUsbInternData->pfnCnnirqCallback(pstcUsb);
            }
            else
            {
                pstcUsb->HIRQ_f.CNNIRQ = 0;
            }
        }
        if (pstcUsb->HIRQ_f.URIRQ == 1)
        {
            if (pstcUsbInternData->pfnUrirqCallback != NULL)
            {
                pstcUsbInternData->pfnUrirqCallback(pstcUsb);
            }
            else
            {
                pstcUsb->HIRQ_f.URIRQ = 0;
            }
        }
        if (pstcUsb->HIRQ_f.RWKIRQ == 1)
        {
            pstcUsb->HIRQ_f.RWKIRQ = 0;
            if (pstcUsbInternData->pfnRwkirqCallback != NULL)
            {
                pstcUsbInternData->pfnRwkirqCallback(pstcUsb);
            }
        }
        if (pstcUsb->HIRQ_f.TCAN == 1)
        {
            pstcUsb->HIRQ_f.TCAN = 0;
            if (pstcUsbInternData->pfnTcanCallback != NULL)
            {
                pstcUsbInternData->pfnTcanCallback(pstcUsb);
            }
        }
    }
}

//@} // UsbGroup



#endif // #if ((FM_PERIPHERAL_ENABLE_USB0 == ON) || (FM_PERIPHERAL_ENABLE_USB0 == ON))
/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/

