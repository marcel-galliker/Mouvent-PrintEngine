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
/** \file usb.h
 **
 ** Headerfile for USB functions
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

#ifndef __USB_H__
#define __USB_H__

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "mcu.h"

#include "usbconfig.h"
#include "string.h"
#if (USB_USE_L3 == 1)
    #include "l3.h"
    #include "base_types.h"
#elif (USB_USE_PDL == 1)
    #include "pdl.h"
    #include "base_types.h"
#elif defined(FM4_USB0)
    #include "base_types.h"
#elif defined(FM3_USB0)
    #include "base_types.h"
#else
    #error Not supported configuration
#endif   


/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif



/**
 ******************************************************************************
 ** \defgroup UsbGroup USB General HAL Functionalities
 **
 ** Provided functions of USB module:
 ** 
 ** - Usb_Configure()
 ** - Usb_Init()
 ** - Usb_SetupHalEndpoint()
 ** - Usb_SwitchUsb()
 ** - Usb_InitInstance()
 ** - Usb_GetEndpointPtr()
 ** - Usb_HalSend()
 ** - Usb_HalReceive()
 ** - Usb_HalEpReady()
 ** - Usb_HalEndpointIsBusy()
 ** - Usb_GetConfiguration()
 ** - Usb_AddTimeOut()
 ** - Usb_RemoveTimeOut()
 **
 ** Used to initialize the HAL and using low level functionality. Usb_Configure() is used to setup 
 ** the low level driver. Usb_SetupHalEndpoint() is used to setup an endpoint in hardware (for device mode).
 ** Usb_SwitchUsb() can be called, to switch USB on or off or between host and device functionality.
 ** Usb_SwitchUsb() is called cyclic or via external interrupt when VBUS state changes. Usb_InitInstance() 
 ** initializes all neccesary registers. Usb_GetEndpointPtr() returns pointer of endpoint struct.
 ** Usb_HalSend() and Usb_HalReceive() can be used to transfer data to or from the endpoint FIFO.
 ** Usb_HalEpReady() gives an information wether the endpoint is ready to use while Usb_HalEndpointIsBusy()
 ** gives the information if the endpoint is transferring data. While endpoints having double buffers, the endpoint
 ** can be ready for next data, but is still busy. Usb_GetConfiguration() returns if the HAL in device mode is 
 ** configured or not. Usb_AddTimeOut() adds timeout functionality via SOF interrupt. Usb_RemoveTimeOut() removes timeouts.
 **
 ** \note For USB Host functionality, see also @link UsbHostGroup USB Host Module description @endlink
 **       For USB Device functionality see also @link UsbDeviceGroup USB Device Module description @endlink
 ******************************************************************************/
//@{

/*****************************************************************************/
/* Global pre-processor symbols/macros ('#define')                           */
/*****************************************************************************/
#if !defined(OFF)
    #define OFF 0U
#endif
#if !defined(ON)
    #define ON 1U
#endif

#if !defined(USB_USE_PDL)
#define USB_USE_PDL                                       0
#endif
  
#if !defined(USB_USE_L3)
#define USB_USE_L3                                        0
#endif
    
#if !defined(USB_USES_DMA)
    #define USB_USES_DMA OFF
#endif  
    
#if !defined(USB_USE_STANDALONE)
    #if (USB_USE_PDL == OFF) && (USB_USE_L3 == OFF)
        #define USB_USE_STANDALONE ON
    #else
        #define USB_USE_STANDALONE OFF
    #endif
#endif  
  

#if (USB_USE_STANDALONE == ON)
    #if defined(FM4_USB0_BASE)
        #define USB0       (*((volatile stc_usbn_t *) FM4_USB0_BASE))
    #endif
    #if defined(FM4_USB0_BASE)
        #define USB1       (*((volatile stc_usbn_t *) FM4_USB1_BASE))
    #endif  
    #if defined(FM3_USB_BASE)
        #define USB0       (*((volatile stc_usbn_t *) FM3_USB_BASE))
    #endif
    #if defined(FM3_USB0_BASE)
        #define USB0       (*((volatile stc_usbn_t *) FM3_USB0_BASE))
    #endif
    #if defined(FM3_USB0_BASE)
        #define USB1       (*((volatile stc_usbn_t *) FM3_USB1_BASE))
    #endif 
     
    #ifndef USB0_DEVICE_IRQ_ENABLED
        #define USB0_DEVICE_IRQ_ENABLED 1
    #endif
    #ifndef USB0_HOST_IRQ_ENABLED
        #define USB0_HOST_IRQ_ENABLED   1
    #endif
    #ifndef USB1_DEVICE_IRQ_ENABLED
        #define USB1_DEVICE_IRQ_ENABLED 1
    #endif
    #ifndef USB1_HOST_IRQ_ENABLED
        #define USB1_HOST_IRQ_ENABLED   1
    #endif
     
    #define FM_PERIPHERAL_ENABLE_USB0_HOST   USB0_HOST_ENABLED
    #define FM_PERIPHERAL_ENABLE_USB1_HOST   USB1_HOST_ENABLED
    #define FM_PERIPHERAL_ENABLE_USB0_DEVICE USB0_DEVICE_ENABLED             
    #define FM_PERIPHERAL_ENABLE_USB1_DEVICE USB1_DEVICE_ENABLED             
#elif (USB_USE_L3 == ON)
    #if defined(FM4_USB0)
        #error L3 library is not available for FM4!
    #endif
    #define USB0       (*((volatile stc_usbn_t *) L3_USB0_BASE))
    #define USB1       (*((volatile stc_usbn_t *) L3_USB1_BASE))
    #if USB_USES_DMA != OFF
        #undef USB_USES_DMA 
        #define USB_USES_DMA OFF
    #endif
    #if defined(L3_IRQ_LEVEL_USB0)
        #undef IRQ_LEVEL_USB0
        #define IRQ_LEVEL_USB0      L3_IRQ_LEVEL_USB0   
    #endif
    #if defined(L3_IRQ_LEVEL_USB1)
        #undef IRQ_LEVEL_USB1
        #define IRQ_LEVEL_USB1      L3_IRQ_LEVEL_USB1   
    #endif 
    #ifndef USB0_DEVICE_IRQ_ENABLED
        #define USB0_DEVICE_IRQ_ENABLED 1
    #endif
    #ifndef USB0_HOST_IRQ_ENABLED
        #define USB0_HOST_IRQ_ENABLED   1
    #endif
    #ifndef USB1_DEVICE_IRQ_ENABLED
        #define USB1_DEVICE_IRQ_ENABLED 1
    #endif
    #ifndef USB1_HOST_IRQ_ENABLED
        #define USB1_HOST_IRQ_ENABLED   1
    #endif     
    #define FM_PERIPHERAL_ENABLE_USB0_HOST     L3_PERIPHERAL_ENABLE_USB0_HOST 
    #define FM_PERIPHERAL_ENABLE_USB0_DEVICE   L3_PERIPHERAL_ENABLE_USB0_DEVICE
    #define FM_PERIPHERAL_ENABLE_USB1_HOST     L3_PERIPHERAL_ENABLE_USB1_HOST 
    #define FM_PERIPHERAL_ENABLE_USB1_DEVICE   L3_PERIPHERAL_ENABLE_USB1_DEVICE
#elif (USB_USE_PDL == ON)
    #if defined(FM3_USB0) || defined(FM3_USB)
        #error PDL library is not available for FM3!
    #endif
    #if defined(FM4_USB0_BASE)
        #define USB0       (*((volatile stc_usbn_t *) FM4_USB0_BASE))
    #endif
    #if defined(FM4_USB0_BASE)
        #define USB1       (*((volatile stc_usbn_t *) FM4_USB1_BASE))
    #endif  
    #if USB_USES_DMA != OFF
        #undef USB_USES_DMA 
        #define USB_USES_DMA OFF
    #endif

    #if defined(PDL_IRQ_LEVEL_USB0)
        #undef IRQ_LEVEL_USB0  
        #define IRQ_LEVEL_USB0      PDL_IRQ_LEVEL_USB0   
    #endif
    #if defined(PDL_IRQ_LEVEL_USB1)
        #undef IRQ_LEVEL_USB1
        #define IRQ_LEVEL_USB1      PDL_IRQ_LEVEL_USB1   
    #endif  
    #if defined(USB0_DEVICE_IRQ_ENABLED)
        #undef USB0_DEVICE_IRQ_ENABLED 
    #endif
    #if defined(USB0_HOST_IRQ_ENABLED) 
        #undef USB0_HOST_IRQ_ENABLED 
    #endif
    #if defined(USB1_DEVICE_IRQ_ENABLED)
        #undef USB1_DEVICE_IRQ_ENABLED
    #endif
    #if defined(USB1_HOST_IRQ_ENABLED) 
        #undef USB1_HOST_IRQ_ENABLED 
    #endif
     
    #define USB0_DEVICE_IRQ_ENABLED PDL_INTERRUPT_ENABLE_USB0_DEVICE
    #define USB0_HOST_IRQ_ENABLED   PDL_INTERRUPT_ENABLE_USB0_HOST
    #define USB1_DEVICE_IRQ_ENABLED PDL_INTERRUPT_ENABLE_USB1_DEVICE
    #define USB1_HOST_IRQ_ENABLED   PDL_INTERRUPT_ENABLE_USB1_HOST
     
    #define FM_PERIPHERAL_ENABLE_USB0_HOST     PDL_PERIPHERAL_ENABLE_USB0_HOST 
    #define FM_PERIPHERAL_ENABLE_USB0_DEVICE   PDL_PERIPHERAL_ENABLE_USB0_DEVICE 
    #define FM_PERIPHERAL_ENABLE_USB1_HOST     PDL_PERIPHERAL_ENABLE_USB1_HOST 
    #define FM_PERIPHERAL_ENABLE_USB1_DEVICE   PDL_PERIPHERAL_ENABLE_USB1_DEVICE 

    #ifdef USBDEVICECDCCOM_ENABLED
        #undef USBDEVICECDCCOM_ENABLED
    #endif
    #ifdef USBDEVICEHIDJOYSTICK_ENABLED
        #undef USBDEVICEHIDJOYSTICK_ENABLED
    #endif
    #ifdef USBDEVICEHIDKEYBOARD_ENABLED
        #undef USBDEVICEHIDKEYBOARD_ENABLED
    #endif
    #ifdef USBDEVICEHIDMOUSE_ENABLED
        #undef USBDEVICEHIDMOUSE_ENABLED
    #endif
    #ifdef USBDEVICELIBUSB_ENABLED
        #undef USBDEVICELIBUSB_ENABLED
    #endif
    #ifdef USBDEVICEPRINTER_ENABLED
        #undef USBDEVICEPRINTER_ENABLED
    #endif
    
    
    #ifdef USBHOSTHIDCOM_ENABLED
        #undef USBHOSTHIDCOM_ENABLED
    #endif
    #ifdef USBHOSTHIDKEYBOARD_ENABLED
        #undef USBHOSTHIDKEYBOARD_ENABLED
    #endif
    #ifdef USBHOSTHIDMOUSE_ENABLED
        #undef USBHOSTHIDMOUSE_ENABLED
    #endif
    #ifdef USBHOSTMASSSTORAGE_ENABLED
        #undef USBHOSTMASSSTORAGE_ENABLED
    #endif

    #define USBDEVICECDCCOM_ENABLED          PDL_USBDEVICECDCCOM_ENABLED
    #define USBDEVICEHIDCOM_ENABLED          PDL_USBDEVICEHIDCOM_ENABLED
    #define USBDEVICEHIDJOYSTICK_ENABLED     PDL_USBDEVICEHIDJOYSTICK_ENABLED
    #define USBDEVICEHIDKEYBOARD_ENABLED     PDL_USBDEVICEHIDKEYBOARD_ENABLED
    #define USBDEVICEHIDMOUSE_ENABLED        PDL_USBDEVICEHIDMOUSE_ENABLED
    #define USBDEVICELIBUSB_ENABLED          PDL_USBDEVICELIBUSB_ENABLED
    #define USBDEVICEPRINTER_ENABLED         PDL_USBDEVICEPRINTER_ENABLED
    
    #define USBHOSTHIDCOM_ENABLED            PDL_USBHOSTHIDCOM_ENABLED     
    #define USBHOSTHIDKEYBOARD_ENABLED       PDL_USBHOSTHIDKEYBOARD_ENABLED  
    #define USBHOSTHIDMOUSE_ENABLED          PDL_USBHOSTHIDMOUSE_ENABLED      
    #define USBHOSTMASSSTORAGE_ENABLED       PDL_USBHOSTMASSSTORAGE_ENABLED  
    
#else
    #error USB low level driver must be used standalone, with L3 or with PDL
#endif

#if !defined(FM_PERIPHERAL_ENABLE_USB0_DEVICE)
    #define FM_PERIPHERAL_ENABLE_USB0_DEVICE OFF
#endif
    
#if !defined(FM_PERIPHERAL_ENABLE_USB1_DEVICE)
    #define FM_PERIPHERAL_ENABLE_USB1_DEVICE OFF
#endif
  
#if !defined(FM_PERIPHERAL_ENABLE_USB0_HOST)
    #define FM_PERIPHERAL_ENABLE_USB0_HOST OFF
#endif
    
#if !defined(FM_PERIPHERAL_ENABLE_USB1_HOST)
    #define FM_PERIPHERAL_ENABLE_USB1_HOST OFF
#endif

#if (FM_PERIPHERAL_ENABLE_USB0_DEVICE == ON) || (FM_PERIPHERAL_ENABLE_USB0_HOST == ON)
    #define FM_PERIPHERAL_ENABLE_USB0 ON
#else
    #define FM_PERIPHERAL_ENABLE_USB0 OFF
#endif

#if (FM_PERIPHERAL_ENABLE_USB1_DEVICE == ON) || (FM_PERIPHERAL_ENABLE_USB1_HOST == ON)
    #define FM_PERIPHERAL_ENABLE_USB1 ON
#else
    #define FM_PERIPHERAL_ENABLE_USB1 OFF
#endif

#if (FM_PERIPHERAL_ENABLE_USB0_HOST == ON) || (FM_PERIPHERAL_ENABLE_USB1_HOST == ON)
    #define FM_PERIPHERAL_USB_HOST_ENABLED  ON
#else
    #define FM_PERIPHERAL_USB_HOST_ENABLED  OFF
    #define USB_DISBALE_HOST_FUNCTIONALITY OFF
#endif

#if (FM_PERIPHERAL_ENABLE_USB0_DEVICE == ON) || (FM_PERIPHERAL_ENABLE_USB1_DEVICE == ON)
    #define FM_PERIPHERAL_USB_DEVICE_ENABLED  ON
#else
    #define FM_PERIPHERAL_USB_DEVICE_ENABLED  OFF
    #define USB_DISBALE_DEVICE_FUNCTIONALITY OFF
#endif
  
#define USB_ENABLED                  ((FM_PERIPHERAL_ENABLE_USB0 == ON) || (FM_PERIPHERAL_ENABLE_USB1 == ON))
#define USB_INTERFACES_COUNT         ((FM_PERIPHERAL_ENABLE_USB0) + (FM_PERIPHERAL_ENABLE_USB1))
#define USB_INSTANCE_COUNT           ((FM_PERIPHERAL_ENABLE_USB0) + (FM_PERIPHERAL_ENABLE_USB1))
#define USB_DEVICE_INTERFACES_COUNT  ((FM_PERIPHERAL_ENABLE_USB0_DEVICE) + (FM_PERIPHERAL_ENABLE_USB1_DEVICE))
#define USB_HOST_INTERFACES_COUNT    ((FM_PERIPHERAL_ENABLE_USB0_HOST) + (FM_PERIPHERAL_ENABLE_USB1_HOST))

  
#if (USEUSBDBG == 1)
#include "uart.h"
#define USBDBG(x) DBGSPACES(u8DebugDepth); puts(x)
#define USBDBGVAL8(x,y) DBGSPACES(u8DebugDepth); puts(x); puthex((uint32_t)y,2); puts("\n")
#define USBDBGVAL16(x,y) DBGSPACES(u8DebugDepth); puts(x); puthex((uint32_t)y,4); puts("\n") 
#define USBDBGVAL32(x,y) DBGSPACES(u8DebugDepth); puts(x); puthex((uint32_t)y,8); puts("\n")
#define DBGPROCENTER(x) DBGSPACES(u8DebugDepth); u8DebugDepth++; puts(">"); puts(x);puts("\n");
#define DBGPROCRETURN(x) if (u8DebugDepth > 0) u8DebugDepth--;DBGSPACES(u8DebugDepth);puts("<"); puts(x);puts("\n");
#define DBGSPACES(x)    for(u8Dbgi = 0;u8Dbgi < x*2;u8Dbgi++) putch(' ')
volatile extern uint8_t u8DebugDepth;
volatile extern uint8_t u8Dbgi;
#else
#define USBDBG(x) 
#define USBDBGVAL8(x,y) 
#define USBDBGVAL16(x,y)
#define USBDBGVAL32(x,y)
#define DBGPROCENTER(x)
#define DBGPROCRETURN(x)
#define DBGSPACES(x)
#endif

#define SETACTIVEFLAG(x) while((x) == FALSE) (x) = TRUE;
#define CLEARACTIVEFLAG(x) while((x) == TRUE) (x) = FALSE;
   
#define USB_ZERO_STRUCT(x)    memset(&x,0,sizeof(x))
#define ZERO_STRUCT(x)    memset(&x,0,sizeof(x))
  
#if defined(__USB_C__)
    #if (USEUSBDBG == 1)
        volatile uint8_t u8DebugDepth = 0;
        volatile uint8_t u8Dbgi = 0;
        #warning USB DEBUG ENABLED
    #endif
    #if (USBWARNLEVEL == 2)
  
        #warning *************** Library Usage **************************
        #if (USB_USE_STANDALONE == ON)
            #warning Info: USB_USE_STANDALONE == ON
        #else
            #warning Info: USB_USE_STANDALONE == OFF
        #endif
      
        #if (USB_USE_L3 == ON)
            #warning Info: USB_USE_L3 == ON
        #else
            #warning Info: USB_USE_L3 == OFF
        #endif

        #if (USB_USE_PDL == ON)
            #warning Info: USB_USE_PDL == ON
        #else
            #warning Info: USB_USE_PDL == OFF
        #endif
  
        #warning *************** USB Enabled Interfaces *****************
        #if USB_INTERFACES_COUNT == 0
            #warning Info: USB_INTERFACES_COUNT == 0
        #elif USB_INTERFACES_COUNT == 2
            #warning Info: USB_INTERFACES_COUNT == 2
        #elif USB_INTERFACES_COUNT == 3
            #warning Info: USB_INTERFACES_COUNT == 3
        #elif USB_INTERFACES_COUNT == 4
            #warning Info: USB_INTERFACES_COUNT == 4
        #elif USB_INTERFACES_COUNT == 5
            #warning Info: USB_INTERFACES_COUNT == 5
        #elif USB_INTERFACES_COUNT == 6
            #warning Info: USB_INTERFACES_COUNT == 6
        #endif
  
        #if (FM_PERIPHERAL_USB_HOST_ENABLED == OFF)
            #warning Info: HOST EXCLUDED
        #endif  

        #if (FM_PERIPHERAL_USB_DEVICE_ENABLED == OFF)
            #warning Info: DEVICE EXCLUDED
        #endif  
          
        #if (FM_PERIPHERAL_ENABLE_USB0_HOST == ON)
            #warning Info: FM_PERIPHERAL_ENABLE_USB0_HOST == ON
        #else
            #warning Info: FM_PERIPHERAL_ENABLE_USB0_HOST == OFF  
        #endif  
          
        #if (FM_PERIPHERAL_ENABLE_USB1_HOST == ON)
            #warning Info: FM_PERIPHERAL_ENABLE_USB1_HOST == ON
        #else
            #warning Info: FM_PERIPHERAL_ENABLE_USB1_HOST == OFF  
        #endif  
          
        #if (FM_PERIPHERAL_ENABLE_USB0_DEVICE == ON)
            #warning Info: FM_PERIPHERAL_ENABLE_USB0_DEVICE == ON
        #else
            #warning Info: FM_PERIPHERAL_ENABLE_USB0_DEVICE == OFF  
        #endif  

        #if (FM_PERIPHERAL_ENABLE_USB1_DEVICE == ON)
            #warning Info: FM_PERIPHERAL_ENABLE_USB1_DEVICE == ON
        #else
            #warning Info: FM_PERIPHERAL_ENABLE_USB1_DEVICE == OFF  
        #endif  

#warning *************** USB Middleware *************************
    #if  USBDEVICECDCCOM_ENABLED == ON
        #warning Info: USBDEVICECDCCOM_ENABLED == ON
    #else
        #warning Info: USBDEVICECDCCOM_ENABLED == OFF
    #endif
    
    #if  USBDEVICEHIDJOYSTICK == ON
        #warning Info: USBDEVICEHIDJOYSTICK == ON
    #else
        #warning Info: USBDEVICEHIDJOYSTICK == OFF
    #endif
    
    #if  USBDEVICEHIDKEYBOARD_ENABLED == ON
        #warning Info: USBDEVICEHIDKEYBOARD_ENABLED == ON
    #else
        #warning Info: USBDEVICEHIDKEYBOARD_ENABLED == OFF
    #endif
    
    #if  USBDEVICEMOUSE_ENABLED == ON
        #warning Info: USBDEVICEMOUSE_ENABLED == ON
    #else
        #warning Info: USBDEVICEMOUSE_ENABLED == OFF
    #endif
    
    #if  USBDEVICELIBUSB_ENABLED == ON
        #warning Info: USBDEVICELIBUSB_ENABLED == ON
    #else
        #warning Info: USBDEVICELIBUSB_ENABLED == OFF
    #endif
    
    #if  USBDEVICEPRINTER_ENABLED == ON
        #warning Info: USBDEVICEPRINTER_ENABLED == ON
    #else
        #warning Info: USBDEVICEPRINTER_ENABLED == OFF
    #endif
    
    
        #if  USBHOSTHIDCOM_ENABLED == ON
                        #warning Info: USBHOSTHIDCOM_ENABLED == ON
        #else
                        #warning Info: USBHOSTHIDCOM_ENABLED == OFF
        #endif
        
        #if  USBHOSTHIDKEYBOARD_ENABLED == ON
                        #warning Info: USBHOSTHIDKEYBOARD_ENABLED == ON
        #else
                        #warning Info: USBHOSTHIDKEYBOARD_ENABLED == OFF
        #endif
        
        #if  USBHOSTHIDMOUSE_ENABLED == ON
                        #warning Info: USBHOSTHIDMOUSE_ENABLED == ON
        #else
                        #warning Info: USBHOSTHIDMOUSE_ENABLED == OFF
        #endif
        
        #if  USBHOSTMASSSTORAGE_ENABLED == ON
                        #warning Info: USBHOSTMASSSTORAGE_ENABLED == ON
        #else
                        #warning Info: USBHOSTMASSSTORAGE_ENABLED == OFF
        #endif
		
        #warning *************** USB IRQ Settings ***********************
        #if USB0_DEVICE_IRQ_ENABLED == ON
            #warning Info: USB0_DEVICE_IRQ_ENABLED == ON
        #else
            #warning Info: USB0_DEVICE_IRQ_ENABLED == OFF
        #endif
        #if USB0_HOST_IRQ_ENABLED == ON
            #warning Info: USB0_HOST_IRQ_ENABLED == ON
        #else
            #warning Info: USB0_HOST_IRQ_ENABLED == OFF
        #endif
        #if USB1_DEVICE_IRQ_ENABLED == ON
            #warning Info: USB1_DEVICE_IRQ_ENABLED == ON
        #else
            #warning Info: USB1_DEVICE_IRQ_ENABLED == OFF
        #endif
        #if USB1_HOST_IRQ_ENABLED == ON
            #warning Info: USB1_HOST_IRQ_ENABLED == ON
        #else
            #warning Info: USB1_HOST_IRQ_ENABLED == OFF
        #endif
        #if IRQ_LEVEL_USB0 == 0
            #warning Info: IRQ_LEVEL_USB0 == 0
        #elif IRQ_LEVEL_USB0 == 1
            #warning Info: IRQ_LEVEL_USB0 == 1  
        #elif IRQ_LEVEL_USB0 == 2
            #warning Info: IRQ_LEVEL_USB0 == 2  
        #elif IRQ_LEVEL_USB0 == 3
            #warning Info: IRQ_LEVEL_USB0 == 3 
        #elif IRQ_LEVEL_USB0 == 4
            #warning Info: IRQ_LEVEL_USB0 == 4 
        #elif IRQ_LEVEL_USB0 == 5
            #warning Info: IRQ_LEVEL_USB0 == 5
        #elif IRQ_LEVEL_USB0 == 6
            #warning Info: IRQ_LEVEL_USB0 == 6
        #elif IRQ_LEVEL_USB0 == 7
            #warning Info: IRQ_LEVEL_USB0 == 7
        #elif IRQ_LEVEL_USB0 == 8
            #warning Info: IRQ_LEVEL_USB0 == 8  
        #elif IRQ_LEVEL_USB0 == 9
            #warning Info: IRQ_LEVEL_USB0 == 9 
        #elif IRQ_LEVEL_USB0 == 10
            #warning Info: IRQ_LEVEL_USB0 == 10
        #elif IRQ_LEVEL_USB0 == 11
            #warning Info: IRQ_LEVEL_USB0 == 11
        #elif IRQ_LEVEL_USB0 == 12
            #warning Info: IRQ_LEVEL_USB0 == 12
        #elif IRQ_LEVEL_USB0 == 13
            #warning Info: IRQ_LEVEL_USB0 == 13
        #elif IRQ_LEVEL_USB0 == 14
            #warning Info: IRQ_LEVEL_USB0 == 14
        #elif IRQ_LEVEL_USB0 == 15
            #warning Info: IRQ_LEVEL_USB0 == 15 
        #endif
  
        #if IRQ_LEVEL_USB1 == 0
            #warning Info: IRQ_LEVEL_USB1 == 0
        #elif IRQ_LEVEL_USB1 == 1
            #warning Info: IRQ_LEVEL_USB1 == 1  
        #elif IRQ_LEVEL_USB1 == 2
            #warning Info: IRQ_LEVEL_USB1 == 2  
        #elif IRQ_LEVEL_USB1 == 3
            #warning Info: IRQ_LEVEL_USB1 == 3 
        #elif IRQ_LEVEL_USB1 == 4
            #warning Info: IRQ_LEVEL_USB1 == 4 
        #elif IRQ_LEVEL_USB1 == 5
            #warning Info: IRQ_LEVEL_USB1 == 5
        #elif IRQ_LEVEL_USB1 == 6
            #warning Info: IRQ_LEVEL_USB1 == 6
        #elif IRQ_LEVEL_USB1 == 7
            #warning Info: IRQ_LEVEL_USB1 == 7
        #elif IRQ_LEVEL_USB1 == 8
            #warning Info: IRQ_LEVEL_USB1 == 8  
        #elif IRQ_LEVEL_USB1 == 9
            #warning Info: IRQ_LEVEL_USB1 == 9 
        #elif IRQ_LEVEL_USB1 == 10
            #warning Info: IRQ_LEVEL_USB1 == 10
        #elif IRQ_LEVEL_USB1 == 11
            #warning Info: IRQ_LEVEL_USB1 == 11
        #elif IRQ_LEVEL_USB1 == 12
            #warning Info: IRQ_LEVEL_USB1 == 12
        #elif IRQ_LEVEL_USB1 == 13
            #warning Info: IRQ_LEVEL_USB1 == 13
        #elif IRQ_LEVEL_USB1 == 14
            #warning Info: IRQ_LEVEL_USB1 == 14
        #elif IRQ_LEVEL_USB1 == 15
            #warning Info: IRQ_LEVEL_USB1 == 15 
        #endif
    #endif
      
    #if IRQ_LEVEL_USB0 > 15
        #error IRQ_LEVEL_USB0 must be <= 15
    #endif 
      
    #if IRQ_LEVEL_USB1 > 15
        #error IRQ_LEVEL_USB1 must be <= 15
    #endif 
    
#endif
  
/**
 ******************************************************************************
 ** \cond FM3DeviceTypes FM3 Device Types 
 ******************************************************************************/
//@{

#ifndef FM3MCUTYPE_TYPE0
    #define FM3MCUTYPE_TYPE0 0
#endif
#ifndef FM3MCUTYPE_TYPE1
    #define FM3MCUTYPE_TYPE1 1
#endif
#ifndef FM3MCUTYPE_TYPE2
    #define FM3MCUTYPE_TYPE2 2
#endif
#ifndef FM3MCUTYPE_TYPE3
    #define FM3MCUTYPE_TYPE3 3
#endif
#ifndef FM3MCUTYPE_TYPE4
    #define FM3MCUTYPE_TYPE4 4
#endif
#ifndef FM3MCUTYPE_TYPE5
    #define FM3MCUTYPE_TYPE5 5
#endif
#ifndef FM3MCUTYPE_TYPE6
    #define FM3MCUTYPE_TYPE6 6
#endif
#ifndef FM3MCUTYPE_TYPE7
    #define FM3MCUTYPE_TYPE7 7
#endif
   
  
#ifndef FM4MCUTYPE_TYPE0
    #define FM4MCUTYPE_TYPE0 0
#endif


#ifdef FM3_DEVICE_TYPE 
    #ifndef FM3MCUTYPE
         #define FM3MCUTYPE FM3_DEVICE_TYPE
    #endif
#endif

#ifdef FM4_DEVICE_TYPE 
    #ifndef FM4MCUTYPE
         #define FM4MCUTYPE FM4_DEVICE_TYPE
    #endif
#endif
   
#if (0 == USB_USE_L3)
  #ifndef FM4MCUTYPE
    #ifdef  _MB9ABXXX_H_
        #define FM4MCUTYPE FM4MCUTYPE_TYPE0
    #endif
    #ifdef  _MB9B560R_H_
        #define FM4MCUTYPE FM4MCUTYPE_TYPE0
    #endif
  #endif
  
  #ifndef FM3MCUTYPE
      #ifdef _MB9B120K_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE9
      #endif
      #ifdef _MB9B320K_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE9
      #endif
      #ifdef _MB9B420K_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE9
      #endif
      #ifdef _MB9B520K_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE9
      #endif
      #ifdef _MB9B120L_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE9
      #endif
      #ifdef _MB9B320L_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE9
      #endif
      #ifdef _MB9B420L_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE9
      #endif
      #ifdef _MB9B520L_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE9
      #endif
      #ifdef _MB9B120M_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE9
      #endif
      #ifdef _MB9B320M_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE9
      #endif
      #ifdef _MB9B420M_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE9
      #endif
      #ifdef _MB9B520M_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE9
      #endif
      #ifdef _MB9B500N_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE0
      #endif
      #ifdef _MB9B400N_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE0
      #endif
      #ifdef _MB9B300N_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE0
      #endif
      #ifdef _MB9B100N_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE0
      #endif
      #ifdef _MB9B500R_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE0
      #endif
      #ifdef _MB9B400R_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE0
      #endif
      #ifdef _MB9B300R_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE0
      #endif
      #ifdef _MB9B100R_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE0
      #endif
      #ifdef _MB9BD10S_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE2
      #endif
      #ifdef _MB9B610S_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE2
      #endif
      #ifdef _MB9B510S_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE2
      #endif
      #ifdef _MB9B410S_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE2
      #endif
      #ifdef _MB9B310S_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE2
      #endif
      #ifdef _MB9B210S_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE2
      #endif
      #ifdef _MB9B110S_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE2
      #endif
      #ifdef _MB9BD10T_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE2
      #endif
      #ifdef _MB9B610T_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE2
      #endif
      #ifdef _MB9B210T_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE2
      #endif
      #ifdef _MB9B510T_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE2
      #endif
      #ifdef _MB9B410T_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE2
      #endif
      #ifdef _MB9B310T_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE2
      #endif
      #ifdef _MB9B110T_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE2
      #endif
      #ifdef _MB9B510N_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE4
      #endif
      #ifdef _MB9B410N_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE4
      #endif
      #ifdef _MB9B310N_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE4
      #endif
      #ifdef _MB9B110N_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE4
      #endif
      #ifdef _MB9B510R_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE4
      #endif
      #ifdef _MB9B410R_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE4
      #endif
      #ifdef _MB9B310R_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE4
      #endif
      #ifdef _MB9B110R_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE4
      #endif
      #ifdef _MB9A310K_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE5
      #endif
      #ifdef _MB9A110K_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE5
      #endif
      #ifdef _MB9A310L_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE1
      #endif
      #ifdef _MB9A110L_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE1
      #endif
      #ifdef _MB9A310M_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE1
      #endif
      #ifdef _MB9A110M_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE1
      #endif
      #ifdef _MB9A310N_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE1
      #endif
      #ifdef _MB9A100N_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE0
      #endif
      #ifdef _MB9A110N_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE1
      #endif
      #ifdef _MB9A100R_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE0
      #endif
      #ifdef _MB9AB40L_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE6
      #endif
      #ifdef _MB9AA40L_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE6
      #endif
      #ifdef _MB9A340L_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE6
      #endif
      #ifdef _MB9A140L_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE6
      #endif
      #ifdef _MB9AB40M_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE6
      #endif
      #ifdef _MB9AA40M_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE6
      #endif
      #ifdef _MB9A340M_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE6
      #endif
      #ifdef _MB9A140M_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE6
      #endif
      #ifdef _MB9AB40N_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE6
      #endif
      #ifdef _MB9AA40N_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE6
      #endif
      #ifdef _MB9A340N_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE6
      #endif
      #ifdef _MB9A140N_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE6
      #endif
      #ifdef _MB9A150M_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE8
      #endif
      #ifdef _MB9A150N_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE8
      #endif
      #ifdef _MB9A150R_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE8
      #endif
      #ifdef _MB9A130K_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE3
      #endif
      #ifdef _MB9A130L_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE3
      #endif
      #ifdef _MB9AA30L_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE7
      #endif
      #ifdef _MB9AA30M_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE7
      #endif
      #ifdef _MB9A130M_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE7
      #endif
      #ifdef _MB9AA30N_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE7
      #endif
      #ifdef _MB9A130N_H_
          #define FM3MCUTYPE FM3MCUTYPE_TYPE7
      #endif
  #endif
#else // (0 == USB_USE_L3)
  #ifndef FM3MCUTYPE
    #if (L3_DEVICE_TYPE == L3_TYPE0)
      #define FM3MCUTYPE FM3MCUTYPE_TYPE0
    #elif (L3_DEVICE_TYPE == L3_TYPE1)
      #define FM3MCUTYPE FM3MCUTYPE_TYPE1
    #elif (L3_DEVICE_TYPE == L3_TYPE2)
      #define FM3MCUTYPE FM3MCUTYPE_TYPE2
    #elif (L3_DEVICE_TYPE == L3_TYPE3)
      #define FM3MCUTYPE FM3MCUTYPE_TYPE3
    #elif (L3_DEVICE_TYPE == L3_TYPE4)
      #define FM3MCUTYPE FM3MCUTYPE_TYPE4
    #elif (L3_DEVICE_TYPE == L3_TYPE5)
      #define FM3MCUTYPE FM3MCUTYPE_TYPE5
    #elif (L3_DEVICE_TYPE == L3_TYPE6)
      #define FM3MCUTYPE FM3MCUTYPE_TYPE6
    #elif (L3_DEVICE_TYPE == L3_TYPE7)
      #define FM3MCUTYPE FM3MCUTYPE_TYPE7
    #endif
  #endif
#endif // (0 == USB_USE_L3)



//@} // FM3DeviceTypes
/**
 ******************************************************************************
 ** \endcond FM3DeviceTypes End FM3 Device Types 
 ******************************************************************************/




#define USBDESCR_DEVICE         1
#define USBDESCR_CONFIG         2
#define USBDESCR_STRING         3
#define USBDESCR_INTERFACE      4
#define USBDESCR_ENDPOINT       5

#define USB_DEVREQ_STDTYPE    0x00
#define USB_DEVREQ_CLSTYPE    0x20
#define USB_DEVREQ_VNDTYPE    0x40

#define USB_DEVREQ_GET_DESC    0x06
#define USB_DEVREQ_SET_DESC    0x07
#define USB_DEVREQ_SET_IDLE    0x0A
  
#define USB_GET_STATUS        0x00
#define USB_CLEAR_FEATURE     0x01
#define USB_SET_FEATURE       0x03
#define USB_SET_ADDRESS       0x05
#define USB_GET_DESCRIPTOR    0x06
#define USB_SET_DESCRIPTOR    0x07
#define USB_GET_CONFIGURATION 0x08
#define USB_SET_CONFIGURATION 0x09
#define USB_GET_INTERFACE     0x0A
#define USB_SET_INTERFACE     0x0B
#define USB_SYNCH_FRAME       0x0C
  
#define USBATTR_BUSPOWER        0x80
#define USBATTR_SELFPOWER       0xC0
#define USBATTR_REMOTEWAKE      0x20

#define USB_FUNC_INTR_IN    0xF000
#define USB_FUNC_INTR_OUT    0xE000
#define USB_FUNC_BULK_IN    0xD000
#define USB_FUNC_BULK_OUT    0xC000

#define USB_MASK_EPC_PKS            0x007F
#define USB_MASK_EPC_PKS1           0x01FF
#define USB_MASK_EPC_STAL           0x0200
#define USB_MASK_EPC_NULE           0x0400
#define USB_MASK_EPC_DMAE           0x0800
#define USB_MASK_EPC_DIR            0x1000
#define USB_MASK_EPC_TYPE           0x6000
#define USB_MASK_EPC_EPEN           0x8000

#define USB_MASK_EPS_DRQIIE         0x4000
#define USB_MASK_EPS_BFINI          0x8000
#define USB_MASK_EPS_SIZE1          0x01FF
#define USB_MASK_EPS_SIZE           0x007F
#define USB_MASK_EPS_SPK            0x0200
#define USB_MASK_EPS_DRQI           0x0400
#define USB_MASK_EPS_DRQO           0x0400
#define USB_MASK_EPS_DRQ            0x0400
#define USB_MASK_EPS_BUSY           0x0800
#define USB_MASK_EPS_SPKIE          0x2000
#define USB_MASK_EPS_DRQIE          0x4000
#define USB_MASK_EPS_DRQOIE         0x4000
#define USB_MASK_EPS_BFINI          0x8000

#ifndef BITMASK_ISSET
#define BITMASK_ISSET(x,y)  ((x & y) > 0) 
#endif
  
#ifndef BITMASK_ISCLEARED
#define BITMASK_ISCLEARED(x,y)  ((x & y) == 0) 
#endif  
  
#ifndef BITMASK_SET
#define BITMASK_SET(x,y)  x |= y 
#endif  

#ifndef BITMASK_CLEAR
#define BITMASK_CLEAR(x,y)  x &= ~y 
#endif  
  
#ifndef __USBBITDEFINITIONS_H__
  #define __USBBITDEFINITIONS_H__
  #define _UDCC_RST	(1 << 7) ///< Function reset bit
  #define _UDCC_RESUM	(1 << 6) ///< Resume setting bit
  #define _UDCC_HCONX	(1 << 5) ///< Host connection bit
  #define _UDCC_USTP	(1 << 4) ///< USB operation clock stop bit
  #define _UDCC_RFBK	(1 << 1) ///< Data toggle mode bit (Rate feedback mode)
  #define _UDCC_PWC	(1 << 0) ///< Power supply control bit
  
  #define _EP0C_PKS0	0x7F     ///< EP0 packet size setting bits
  #define _EP0C_STAL  (1 << 9) ///< STALL set bit


  #define _EPNC_STAL  (1 << 9)  ///< STALL set bit
  #define _EPNC_NULE  (1 << 10) ///< NULL automatic transfer enable bit
  #define _EPNC_DMAE  (1 << 11) ///< DMA automatic transfer enable bit
  #define _EPNC_DIR   (1 << 12) ///< Endpoint direction selection bit
  #define _EPNC_TYPE  0x6000    ///< Endpoint transfer type bit
  #define _EPNC_EPEN  (1 << 15) ///< Endpoint Enable bit

  #define _EP1C_PKS1	0x1FF   ///< EP1 packet size setting bits
  #define _EP1C_STAL  (1 << 9)  ///< STALL set bit
  #define _EP1C_NULE  (1 << 10) ///< NULL automatic transfer enable bit
  #define _EP1C_DMAE  (1 << 11) ///< DMA automatic transfer enable bit
  #define _EP1C_DIR   (1 << 12) ///< Endpoint direction selection bit
  #define _EP1C_TYPE  0x6000    ///< Endpoint transfer type bit
  #define _EP1C_EPEN  (1 << 15) ///< Endpoint Enable bit
  
  #define _EP2C_PKS2	0xFF    ///< EP2 packet size setting bits
  #define _EP2C_STAL  (1 << 9)  ///< STALL set bit
  #define _EP2C_NULE  (1 << 10) ///< NULL automatic transfer enable bit
  #define _EP2C_DMAE  (1 << 11) ///< DMA automatic transfer enable bit
  #define _EP2C_DIR   (1 << 12) ///< Endpoint direction selection bit
  #define _EP2C_TYPE  0x6000    ///< Endpoint transfer type bit
  #define _EP2C_EPEN  (1 << 15) ///< Endpoint Enable bit
  
  #define _EP3C_PKS3	0xFF    ///< EP3 packet size setting bits
  #define _EP3C_STAL  (1 << 9)  ///< STALL set bit
  #define _EP3C_NULE  (1 << 10) ///< NULL automatic transfer enable bit
  #define _EP3C_DMAE  (1 << 11) ///< DMA automatic transfer enable bit
  #define _EP3C_DIR   (1 << 12) ///< Endpoint direction selection bit
  #define _EP3C_TYPE  0x6000    ///< Endpoint transfer type bit
  #define _EP3C_EPEN  (1 << 15) ///< Endpoint Enable bit
  
  #define _EP4C_PKS4	0xFF    ///< EP4 packet size setting bits
  #define _EP4C_STAL  (1 << 9)  ///< STALL set bit
  #define _EP4C_NULE  (1 << 10) ///< NULL automatic transfer enable bit
  #define _EP4C_DMAE  (1 << 11) ///< DMA automatic transfer enable bit
  #define _EP4C_DIR   (1 << 12) ///< Endpoint direction selection bit
  #define _EP4C_TYPE  0x6000    ///< Endpoint transfer type bit
  #define _EP4C_EPEN  (1 << 15) ///< Endpoint Enable bit
  
  #define _EP5C_PKS5	0xFF    ///< EP  packet size setting bits
  #define _EP5C_STAL  (1 << 9)  ///< STALL set bit
  #define _EP5C_NULE  (1 << 10) ///< NULL automatic transfer enable bit
  #define _EP5C_DMAE  (1 << 11) ///< DMA automatic transfer enable bit
  #define _EP5C_DIR   (1 << 12) ///< Endpoint direction selection bit
  #define _EP5C_TYPE  0x6000    ///< Endpoint transfer type bit
  #define _EP5C_EPEN  (1 << 15) ///< Endpoint Enable bit
  
  #define _EPC_TYPE_BULK       0x4000
  #define _EPC_TYPE_INTERRUPT  0x6000
  
  #define _UDCS_CONF	(1 << 0) ///< Configuration detection bit
  #define _UDCS_SETP	(1 << 1) ///< Setup stage detection bit
  #define _UDCS_WKUP	(1 << 2) ///< Wake-up detection bit
  #define _UDCS_BRST	(1 << 3) ///< Bus reset detection bit
  #define _UDCS_SOF	(1 << 4)     ///< SOF reception detection bit
  #define _UDCS_SUSP	(1 << 5) ///< Suspend detection bit
  
  #define _UDCIE_CONFIE (1 << 0)  ///< Configuration interrupt enable bit
  #define _UDCIE_CONFN	 (1 << 1) ///< Configuration number bit
  #define _UDCIE_WKUPIE (1 << 2)  ///< Wake-up interrupt enable bit
  #define _UDCIE_BRSTIE (1 << 3)  ///< Bus reset interrupt enable bit
  #define _UDCIE_SOFIE  (1 << 4)  ///< SOF receive interrupt enable bit
  #define _UDCIE_SUSPIE (1 << 5)  ///< Suspend interrupt enable bit
  
  #define _EPNS_SPK     (1 <<  9)  ///< Short Packet interrupt flag bit
  #define _EPNS_DRQ     (1 << 10)  ///< Packet transfer interrupt flag bit
  #define _EPNS_BUSY    (1 << 11)  ///< Busy flag bit
  #define _EPNS_SPKIE   (1 << 13)  ///< Short packet interrupt enable bit
  #define _EPNS_DRQIE   (1 << 14)  ///< DRQ Interrupt enable bit
  #define _EPNS_BFINI   (1 << 15)  ///< Transmission/Reception buffer initialization bit

  #define _EP0IS_DRQI   (1 << 10) ///< Transmit Data interrupt request bit
  #define _EP0IS_DRQIIE (1 << 14) ///< Transmit Data Interrupt enable bit
  #define _EP0IS_BFINI  (1 << 15) ///< Transmission buffer initialization bit
  
  #define _EP0OS_SIZE        0x7F  ///< Packet size bits
  #define _EP0OS_SPK    (1 <<  9)  ///< Short Packet interrupt flag bit
  #define _EP0OS_DRQO   (1 << 10)  ///< Received data interrupt flag bit
  #define _EP0OS_SPKIE  (1 << 13)  ///< Short packet interrupt enable bit
  #define _EP0OS_DRQOIE (1 << 14)  ///< Receive Data Interrupt enable bit
  #define _EP0OS_BFINI  (1 << 15)  ///< Receive buffer initialization bit
  
  #define _EP1S_SIZE        0x1FF  ///< Packet size bits
  #define _EP1S_SPK     (1 <<  9)  ///< Short Packet interrupt flag bit
  #define _EP1S_DRQ     (1 << 10)  ///< Packet transfer interrupt flag bit
  #define _EP1S_BUSY    (1 << 11)  ///< Busy flag bit
  #define _EP1S_SPKIE   (1 << 13)  ///< Short packet interrupt enable bit
  #define _EP1S_DRQIE   (1 << 14)  ///< DRQ Interrupt enable bit
  #define _EP1S_BFINI   (1 << 15)  ///< Transmission/Reception buffer initialization bit
  
  #define _EP2S_SIZE         0xFF  ///< Packet size bits
  #define _EP2S_SPK     (1 <<  9)  ///< Short Packet interrupt flag bit
  #define _EP2S_DRQ     (1 << 10)  ///< Packet transfer interrupt flag bit
  #define _EP2S_BUSY    (1 << 11)  ///< Busy flag bit
  #define _EP2S_SPKIE   (1 << 13)  ///< Short packet interrupt enable bit
  #define _EP2S_DRQIE   (1 << 14)  ///< DRQ Interrupt enable bit
  #define _EP2S_BFINI   (1 << 15)  ///< Transmission/Reception buffer initialization bit
  
  #define _EP3S_SIZE         0xFF  ///< Packet size bits
  #define _EP3S_SPK     (1 <<  9)  ///< Short Packet interrupt flag bit
  #define _EP3S_DRQ     (1 << 10)  ///< Packet transfer interrupt flag bit
  #define _EP3S_BUSY    (1 << 11)  ///< Busy flag bit
  #define _EP3S_SPKIE   (1 << 13)  ///< Short packet interrupt enable bit
  #define _EP3S_DRQIE   (1 << 14)  ///< DRQ Interrupt enable bit
  #define _EP3S_BFINI   (1 << 15)  ///< Transmission/Reception buffer initialization bit
  
  #define _EP4S_SIZE         0xFF  ///< Packet size bits
  #define _EP4S_SPK     (1 <<  9)  ///< Short Packet interrupt flag bit
  #define _EP4S_DRQ     (1 << 10)  ///< Packet transfer interrupt flag bit
  #define _EP4S_BUSY    (1 << 11)  ///< Busy flag bit
  #define _EP4S_SPKIE   (1 << 13)  ///< Short packet interrupt enable bit
  #define _EP4S_DRQIE   (1 << 14)  ///< DRQ Interrupt enable bit
  #define _EP4S_BFINI   (1 << 15)  ///< Transmission/Reception buffer initialization bit
  
  #define _EP5S_SIZE         0xFF  ///< Packet size bits
  #define _EP5S_SPK     (1 <<  9)  ///< Short Packet interrupt flag bit
  #define _EP5S_DRQ     (1 << 10)  ///< Packet transfer interrupt flag bit
  #define _EP5S_BUSY    (1 << 11)  ///< Busy flag bit
  #define _EP5S_SPKIE   (1 << 13)  ///< Short packet interrupt enable bit
  #define _EP5S_DRQIE   (1 << 14)  ///< DRQ Interrupt enable bit
  #define _EP5S_BFINI   (1 << 15)  ///< Transmission/Reception buffer initialization bit
  
  #define _HCNT_HOST    (1 <<  0)  ///< HOST Mode bit
  #define _HCNT_URST	  (1 <<  1) ///< USB Bus Reset bit
  #define _HCNT_SOFIRE  (1 <<  2) ///< SOF interrupt Enable bit
  #define _HCNT_DIRE    (1 <<  3) ///< Disconnection interrupt Enable bit
  #define _HCNT_CNNIRE  (1 <<  4) ///< Connection interrupt Enable bit
  #define _HCNT_CMPIRE  (1 <<  5) ///< Completion interrupt Enable bit
  #define _HCNT_URIRE   (1 <<  6) ///< USB bus Reset interrupt Enable bit
  #define _HCNT_RWKIRE  (1 <<  7) ///< Remote Wake-up interrupt Enable bit
  #define _HCNT_RETRY   (1 <<  8) ///< Retry enable bit
  #define _HCNT_CANCEL  (1 <<  9) ///< Token cancel enable bit
  #define _HCNT_SOFSTEP (1 << 10) ///< SOF interrupt trigger bit
  
  #define _HIRQ_SOFIRQ  (1 <<  0) ///< HOST Mode bit
  #define _HIRQ_DIRQ	  (1 <<  1) ///< USB Bus Reset bit
  #define _HIRQ_CNNIRQ  (1 <<  2) ///< SOF interrupt Enable bit
  #define _HIRQ_CMPIRQ  (1 <<  3) ///< Disconnection interrupt Enable bit
  #define _HIRQ_URIRQ   (1 <<  4) ///< Connection interrupt Enable bit
  #define _HIRQ_RWKIRQ  (1 <<  5) ///< Completion interrupt Enable bit
  #define _HIRQ_TCAN    (1 <<  7) ///< Remote Wake-up interrupt Enable bit
  
  #define _HERR_HS           0x03 ///< Handshake Status
  #define _HERR_STUFF	  (1 <<  2) ///< Stuffing Error bit
  #define _HERR_TGERR   (1 <<  3) ///< Toggle Error bit
  #define _HERR_CRC     (1 <<  4) ///< CRC Error bit
  #define _HERR_TOUT    (1 <<  5) ///< Time Out bit
  #define _HERR_RERR    (1 <<  6) ///< Receive Error bit
  #define _HERR_LSTSOF  (1 <<  7) ///< Lost SOF bit
  
  #define _HSTATE_CSTAT   (1 <<  0) ///< Connection status bit
  #define _HSTATE_TMODE	(1 <<  1) ///< Transfer Mode bit
  #define _HSTATE_SUSP    (1 <<  2) ///< Suspend bit
  #define _HSTATE_SOFBUSY (1 <<  3) ///< SOF timer operation bit
  #define _HSTATE_CLKSEL  (1 <<  4) ///< Clock Selection bit (Full / Low Speed)
  #define _HSTATE_ALIVE   (1 <<  5) ///< keep alive bit (for low speed)
#endif // __USBBITDEFINITIONS_H__  
  
#ifndef __IO 
  #define     __IO    volatile             /*!< Defines 'read / write' permissions              */
#endif
  
#ifndef __USBSPEC_H__
  #define __USBSPEC_H__
  

  #define GET_DESCRIPTOR 0x06
  #define USB_DEVICE_DESCRIPTOR_TYPE                    0x01 ///< Usb Device Descriptor Type
  #define USB_CONFIGURATION_DESCRIPTOR_TYPE             0x02 ///< Usb Configuration Descriptor Type
  #define USB_STRING_DESCRIPTOR_TYPE                    0x03 ///< Usb String Descriptor Type
  #define USB_INTERFACE_DESCRIPTOR_TYPE                 0x04 ///< Usb Interface Descriptor Type
  #define USB_ENDPOINT_DESCRIPTOR_TYPE                  0x05 ///< Usb Endpoint Descriptor Type
  #define USB_DEVICE_QUALIFIER_DESCRIPTOR_TYPE          0x06 ///< Usb Device Qualifier Descriptor Type
  #define USB_OTHER_SPEED_CONFIGURATION_DESCRIPTOR_TYPE 0x07 ///< Usb Other Speed Configuratuion Descriptor Type
  #define USB_HID_DESCRIPTOR_TYPE                       0x21 ///< Usb HID Descriptor Type
  
  #define USB_IN_DIRECTION 0x80                              ///< Usb IN direction
  #define USB_OUT_DIRECTION 0x00                             ///< Usb OUT direction
  
  #define USB_EP_TYPE_CONTROL  0x00                          ///< Control Endpoint Type
  #define USB_EP_TYPE_ISO  0x01                              ///< ISO Endpoint Type
  #define USB_EP_TYPE_BULK 0x02                              ///< Bulk Endpoint Type
  #define USB_EP_TYPE_INT  0x03                              ///< Interrupt Endpoint Type
  
  // Bit-Band alias redefinition for L3 usage
  #define bL3_FM3_GPIO_EPFR00_USB0PE   *((volatile unsigned int*)(0x4266C024UL))
  #define bL3_FM3_GPIO_PFR6_P1         *((volatile unsigned int*)(0x42660304UL))
  #define bL3_FM3_GPIO_SPSR_USB0C      *((volatile unsigned int*)(0x4266B010UL))
  #define bL3_FM3_GPIO_EPFR00_USB1PE   *((volatile unsigned int*)(0x4266C034UL))
  #define bL3_FM3_GPIO_PFR2_P0         *((volatile unsigned int*)(0x42660100UL))
  #define bL3_FM3_GPIO_SPSR_USB1C      *((volatile unsigned int*)(0x4266B014UL))
  
  /******************************************************************************
   ** \brief Bitmask USB request type
   ** 
   ******************************************************************************/
  typedef struct stc_bmRequestType
  {
      uint8_t Recipient                  :5;   ///< Recipient
      uint8_t Type                       :2;   ///< Type
      uint8_t DataPhaseTransferDirection :1;   ///< Transfer direction

  } stc_bmRequestType_t;


  /******************************************************************************
   ** \brief USB setup request type
   ** 
   ******************************************************************************/    
  typedef struct stc_usb_request 
  {
      union {
          uint8_t bmRequestType;                 ///< Usb request type as byte
          stc_bmRequestType_t bmRequestType_f;   ///< Usb request type as bit field
      };
      uint8_t bRequest;                          ///< request
      uint16_t wValue;                           ///< value
      uint16_t wIndex;                           ///< index
      uint16_t wLength;                          ///< length
  } stc_usb_request_t;
  
  
  /******************************************************************************
   ** \brief Setup package
   ** 
   ******************************************************************************/   
  typedef struct stc_usb_setup_package
  {
      stc_usb_request_t* pstcRequest;    ///< USB setup request
      uint8_t*      pu8Buffer;           ///< Buffer with data
  } stc_usb_setup_package_t;
#endif // __USBSPEC_H__
  
  
/**
 ******************************************************************************
 ** \cond USB_MODULE USB Module
 ******************************************************************************/

//@{
  
/******************************************************************************
 * USB_MODULE
 ******************************************************************************/
/* USB_MODULE register bit fields */
typedef struct stc_usbn_hcnt_field
{
  __IO uint16_t HOST       : 1;
  __IO uint16_t URST       : 1;
  __IO uint16_t SOFIRE     : 1;
  __IO uint16_t DIRE       : 1;
  __IO uint16_t CNNIRE     : 1;
  __IO uint16_t CMPIRE     : 1;
  __IO uint16_t URIRE      : 1;
  __IO uint16_t RWKIRE     : 1;
  __IO uint16_t RETRY      : 1;
  __IO uint16_t CANCEL     : 1;
  __IO uint16_t SOFSTEP    : 1;
} stc_usbn_hcnt_field_t;

typedef struct stc_usbn_hcnt0_field
{
  __IO  uint8_t HOST       : 1;
  __IO  uint8_t URST       : 1;
  __IO  uint8_t SOFIRE     : 1;
  __IO  uint8_t DIRE       : 1;
  __IO  uint8_t CNNIRE     : 1;
  __IO  uint8_t CMPIRE     : 1;
  __IO  uint8_t URIRE      : 1;
  __IO  uint8_t RWKIRE     : 1;
} stc_usbn_hcnt0_field_t;

typedef struct stc_usbn_hcnt1_field
{
  __IO  uint8_t RETRY      : 1;
  __IO  uint8_t CANCEL     : 1;
  __IO  uint8_t SOFSTEP    : 1;
} stc_usbn_hcnt1_field_t;

typedef struct stc_usbn_hirq_field
{
  __IO  uint8_t SOFIRQ     : 1;
  __IO  uint8_t DIRQ       : 1;
  __IO  uint8_t CNNIRQ     : 1;
  __IO  uint8_t CMPIRQ     : 1;
  __IO  uint8_t URIRQ      : 1;
  __IO  uint8_t RWKIRQ     : 1;
        uint8_t RESERVED1  : 1;
  __IO  uint8_t TCAN       : 1;
} stc_usbn_hirq_field_t;

typedef struct stc_usbn_herr_field
{
  __IO  uint8_t HS0        : 1;
  __IO  uint8_t HS1        : 1;
  __IO  uint8_t STUFF      : 1;
  __IO  uint8_t TGERR      : 1;
  __IO  uint8_t CRC        : 1;
  __IO  uint8_t TOUT       : 1;
  __IO  uint8_t RERR       : 1;
  __IO  uint8_t LSTOF      : 1;
} stc_usbn_herr_field_t;

typedef struct stc_usbn_hstate_field
{
  __IO  uint8_t CSTAT      : 1;
  __IO  uint8_t TMODE      : 1;
  __IO  uint8_t SUSP       : 1;
  __IO  uint8_t SOFBUSY    : 1;
  __IO  uint8_t CLKSEL     : 1;
  __IO  uint8_t ALIVE      : 1;
} stc_usbn_hstate_field_t;

typedef struct stc_usbn_hfcomp_field
{
  __IO  uint8_t FRAMECOMP0 : 1;
  __IO  uint8_t FRAMECOMP1 : 1;
  __IO  uint8_t FRAMECOMP2 : 1;
  __IO  uint8_t FRAMECOMP3 : 1;
  __IO  uint8_t FRAMECOMP4 : 1;
  __IO  uint8_t FRAMECOMP5 : 1;
  __IO  uint8_t FRAMECOMP6 : 1;
  __IO  uint8_t FRAMECOMP7 : 1;
} stc_usbn_hfcomp_field_t;

typedef struct stc_usbn_hrtimer_field
{
  __IO uint16_t RTIMER0    : 1;
  __IO uint16_t RTIMER1    : 1;
  __IO uint16_t RTIMER2    : 1;
  __IO uint16_t RTIMER3    : 1;
  __IO uint16_t RTIMER4    : 1;
  __IO uint16_t RTIMER5    : 1;
  __IO uint16_t RTIMER6    : 1;
  __IO uint16_t RTIMER7    : 1;
  __IO uint16_t RTIMER8    : 1;
  __IO uint16_t RTIMER9    : 1;
  __IO uint16_t RTIMER10   : 1;
  __IO uint16_t RTIMER11   : 1;
  __IO uint16_t RTIMER12   : 1;
  __IO uint16_t RTIMER13   : 1;
  __IO uint16_t RTIMER14   : 1;
  __IO uint16_t RTIMER15   : 1;
} stc_usbn_hrtimer_field_t;

typedef struct stc_usbn_hrtimer0_field
{
  __IO  uint8_t RTIMER00   : 1;
  __IO  uint8_t RTIMER01   : 1;
  __IO  uint8_t RTIMER02   : 1;
  __IO  uint8_t RTIMER03   : 1;
  __IO  uint8_t RTIMER04   : 1;
  __IO  uint8_t RTIMER05   : 1;
  __IO  uint8_t RTIMER06   : 1;
  __IO  uint8_t RTIMER07   : 1;
} stc_usbn_hrtimer0_field_t;

typedef struct stc_usbn_hrtimer1_field
{
  __IO  uint8_t RTIMER10   : 1;
  __IO  uint8_t RTIMER11   : 1;
  __IO  uint8_t RTIMER12   : 1;
  __IO  uint8_t RTIMER13   : 1;
  __IO  uint8_t RTIMER14   : 1;
  __IO  uint8_t RTIMER15   : 1;
  __IO  uint8_t RTIMER16   : 1;
  __IO  uint8_t RTIMER17   : 1;
} stc_usbn_hrtimer1_field_t;

typedef struct stc_usbn_hrtimer2_field
{
  __IO  uint8_t RTIMER20   : 1;
  __IO  uint8_t RTIMER21   : 1;
  __IO  uint8_t RTIMER22   : 1;
} stc_usbn_hrtimer2_field_t;

typedef struct stc_usbn_hadr_field
{
  __IO  uint8_t ADDRESS0   : 1;
  __IO  uint8_t ADDRESS1   : 1;
  __IO  uint8_t ADDRESS2   : 1;
  __IO  uint8_t ADDRESS3   : 1;
  __IO  uint8_t ADDRESS4   : 1;
  __IO  uint8_t ADDRESS5   : 1;
  __IO  uint8_t ADDRESS6   : 1;
} stc_usbn_hadr_field_t;

typedef struct stc_usbn_heof_field
{
  __IO uint16_t EOF0       : 1;
  __IO uint16_t EOF1       : 1;
  __IO uint16_t EOF2       : 1;
  __IO uint16_t EOF3       : 1;
  __IO uint16_t EOF4       : 1;
  __IO uint16_t EOF5       : 1;
  __IO uint16_t EOF6       : 1;
  __IO uint16_t EOF7       : 1;
  __IO uint16_t EOF8       : 1;
  __IO uint16_t EOF9       : 1;
  __IO uint16_t EOF10      : 1;
  __IO uint16_t EOF11      : 1;
  __IO uint16_t EOF12      : 1;
  __IO uint16_t EOF13      : 1;
  __IO uint16_t EOF14      : 1;
  __IO uint16_t EOF15      : 1;
} stc_usbn_heof_field_t;

typedef struct stc_usbn_heof0_field
{
  __IO  uint8_t EOF00      : 1;
  __IO  uint8_t EOF01      : 1;
  __IO  uint8_t EOF02      : 1;
  __IO  uint8_t EOF03      : 1;
  __IO  uint8_t EOF04      : 1;
  __IO  uint8_t EOF05      : 1;
  __IO  uint8_t EOF06      : 1;
  __IO  uint8_t EOF07      : 1;
} stc_usbn_heof0_field_t;

typedef struct stc_usbn_heof1_field
{
  __IO  uint8_t EOF10      : 1;
  __IO  uint8_t EOF11      : 1;
  __IO  uint8_t EOF12      : 1;
  __IO  uint8_t EOF13      : 1;
  __IO  uint8_t EOF14      : 1;
  __IO  uint8_t EOF15      : 1;
} stc_usbn_heof1_field_t;

typedef struct stc_usbn_hframe_field
{
  __IO uint16_t FRAME0     : 1;
  __IO uint16_t FRAME1     : 1;
  __IO uint16_t FRAME2     : 1;
  __IO uint16_t FRAME3     : 1;
  __IO uint16_t FRAME4     : 1;
  __IO uint16_t FRAME5     : 1;
  __IO uint16_t FRAME6     : 1;
  __IO uint16_t FRAME7     : 1;
  __IO uint16_t FRAME8     : 1;
  __IO uint16_t FRAME9     : 1;
  __IO uint16_t FRAME10    : 1;
} stc_usbn_hframe_field_t;

typedef struct stc_usbn_hframe0_field
{
  __IO  uint8_t FRAME00    : 1;
  __IO  uint8_t FRAME01    : 1;
  __IO  uint8_t FRAME02    : 1;
  __IO  uint8_t FRAME03    : 1;
  __IO  uint8_t FRAME04    : 1;
  __IO  uint8_t FRAME05    : 1;
  __IO  uint8_t FRAME06    : 1;
  __IO  uint8_t FRAME07    : 1;
} stc_usbn_hframe0_field_t;

typedef struct stc_usbn_hframe1_field
{
  __IO  uint8_t FRAME10    : 1;
  __IO  uint8_t FRAME11    : 1;
  __IO  uint8_t FRAME12    : 1;
  __IO  uint8_t FRAME13    : 1;
} stc_usbn_hframe1_field_t;

typedef struct stc_usbn_htoken_field
{
  __IO  uint8_t ENDPT0     : 1;
  __IO  uint8_t ENDPT1     : 1;
  __IO  uint8_t ENDPT2     : 1;
  __IO  uint8_t ENDPT3     : 1;
  __IO  uint8_t TKNEN0     : 1;
  __IO  uint8_t TKNEN1     : 1;
  __IO  uint8_t TKNEN2     : 1;
  __IO  uint8_t TGGL       : 1;
} stc_usbn_htoken_field_t;

typedef struct stc_usbn_udcc_field
{
  __IO uint16_t PWC        : 1;
  __IO uint16_t RFBK       : 1;
       uint16_t RESERVED1  : 1;
  __IO uint16_t STALCLREN  : 1;
  __IO uint16_t USTP       : 1;
  __IO uint16_t HCONX      : 1;
  __IO uint16_t RESUM      : 1;
  __IO uint16_t RST        : 1;
} stc_usbn_udcc_field_t;

typedef struct stc_usbn_ep0c_field
{
  __IO uint16_t PKS00      : 1;
  __IO uint16_t PKS01      : 1;
  __IO uint16_t PKS02      : 1;
  __IO uint16_t PKS03      : 1;
  __IO uint16_t PKS04      : 1;
  __IO uint16_t PKS05      : 1;
  __IO uint16_t PKS06      : 1;
       uint16_t RESERVED1  : 2;
  __IO uint16_t STAL       : 1;
} stc_usbn_ep0c_field_t;

typedef struct stc_usbn_ep1c_field
{
  __IO uint16_t PKS10      : 1;
  __IO uint16_t PKS11      : 1;
  __IO uint16_t PKS12      : 1;
  __IO uint16_t PKS13      : 1;
  __IO uint16_t PKS14      : 1;
  __IO uint16_t PKS15      : 1;
  __IO uint16_t PKS16      : 1;
  __IO uint16_t PKS17      : 1;
  __IO uint16_t PKS18      : 1;
  __IO uint16_t STAL       : 1;
  __IO uint16_t NULE       : 1;
  __IO uint16_t DMAE       : 1;
  __IO uint16_t DIR        : 1;
  __IO uint16_t TYPE0      : 1;
  __IO uint16_t TYPE1      : 1;
  __IO uint16_t EPEN       : 1;
} stc_usbn_ep1c_field_t;

typedef struct stc_usbn_ep2c_field
{
  __IO uint16_t PKS20      : 1;
  __IO uint16_t PKS21      : 1;
  __IO uint16_t PKS22      : 1;
  __IO uint16_t PKS23      : 1;
  __IO uint16_t PKS24      : 1;
  __IO uint16_t PKS25      : 1;
  __IO uint16_t PKS26      : 1;
       uint16_t RESERVED1  : 2;
  __IO uint16_t STAL       : 1;
  __IO uint16_t NULE       : 1;
  __IO uint16_t DMAE       : 1;
  __IO uint16_t DIR        : 1;
  __IO uint16_t TYPE0      : 1;
  __IO uint16_t TYPE1      : 1;
  __IO uint16_t EPEN       : 1;
} stc_usbn_ep2c_field_t;

typedef struct stc_usbn_ep3c_field
{
  __IO uint16_t PKS30      : 1;
  __IO uint16_t PKS31      : 1;
  __IO uint16_t PKS32      : 1;
  __IO uint16_t PKS33      : 1;
  __IO uint16_t PKS34      : 1;
  __IO uint16_t PKS35      : 1;
  __IO uint16_t PKS36      : 1;
       uint16_t RESERVED1  : 2;
  __IO uint16_t STAL       : 1;
  __IO uint16_t NULE       : 1;
  __IO uint16_t DMAE       : 1;
  __IO uint16_t DIR        : 1;
  __IO uint16_t TYPE0      : 1;
  __IO uint16_t TYPE1      : 1;
  __IO uint16_t EPEN       : 1;
} stc_usbn_ep3c_field_t;

typedef struct stc_usbn_ep4c_field
{
  __IO uint16_t PKS40      : 1;
  __IO uint16_t PKS41      : 1;
  __IO uint16_t PKS42      : 1;
  __IO uint16_t PKS43      : 1;
  __IO uint16_t PKS44      : 1;
  __IO uint16_t PKS45      : 1;
  __IO uint16_t PKS46      : 1;
       uint16_t RESERVED1  : 2;
  __IO uint16_t STAL       : 1;
  __IO uint16_t NULE       : 1;
  __IO uint16_t DMAE       : 1;
  __IO uint16_t DIR        : 1;
  __IO uint16_t TYPE0      : 1;
  __IO uint16_t TYPE1      : 1;
  __IO uint16_t EPEN       : 1;
} stc_usbn_ep4c_field_t;

typedef struct stc_usbn_ep5c_field
{
  __IO uint16_t PKS50      : 1;
  __IO uint16_t PKS51      : 1;
  __IO uint16_t PKS52      : 1;
  __IO uint16_t PKS53      : 1;
  __IO uint16_t PKS54      : 1;
  __IO uint16_t PKS55      : 1;
  __IO uint16_t PKS56      : 1;
       uint16_t RESERVED1  : 2;
  __IO uint16_t STAL       : 1;
  __IO uint16_t NULE       : 1;
  __IO uint16_t DMAE       : 1;
  __IO uint16_t DIR        : 1;
  __IO uint16_t TYPE0      : 1;
  __IO uint16_t TYPE1      : 1;
  __IO uint16_t EPEN       : 1;
} stc_usbn_ep5c_field_t;

typedef struct stc_usbn_tmsp_field
{
  __IO uint16_t TMSP0      : 1;
  __IO uint16_t TMSP1      : 1;
  __IO uint16_t TMSP2      : 1;
  __IO uint16_t TMSP3      : 1;
  __IO uint16_t TMSP4      : 1;
  __IO uint16_t TMSP5      : 1;
  __IO uint16_t TMSP6      : 1;
  __IO uint16_t TMSP7      : 1;
  __IO uint16_t TMSP8      : 1;
  __IO uint16_t TMSP9      : 1;
  __IO uint16_t TMSP10     : 1;
} stc_usbn_tmsp_field_t;

typedef struct stc_usbn_udcs_field
{
  __IO  uint8_t CONF       : 1;
  __IO  uint8_t SETP       : 1;
  __IO  uint8_t WKUP       : 1;
  __IO  uint8_t BRST       : 1;
  __IO  uint8_t SOF        : 1;
  __IO  uint8_t SUSP       : 1;
} stc_usbn_udcs_field_t;

typedef struct stc_usbn_udcie_field
{
  __IO  uint8_t CONFIE     : 1;
  __IO  uint8_t CONFN      : 1;
  __IO  uint8_t WKUPIE     : 1;
  __IO  uint8_t BRSTIE     : 1;
  __IO  uint8_t SOFIE      : 1;
  __IO  uint8_t SUSPIE     : 1;
} stc_usbn_udcie_field_t;

typedef struct stc_usbn_ep0is_field
{
       uint16_t RESERVED1  : 10;
  __IO uint16_t DRQI       : 1;
       uint16_t RESERVED2  : 3;
  __IO uint16_t DRQIIE     : 1;
  __IO uint16_t BFINI      : 1;
} stc_usbn_ep0is_field_t;

typedef struct stc_usbn_ep0os_field
{
  __IO uint16_t SIZE0      : 1;
  __IO uint16_t SIZE1      : 1;
  __IO uint16_t SIZE2      : 1;
  __IO uint16_t SIZE3      : 1;
  __IO uint16_t SIZE4      : 1;
  __IO uint16_t SIZE5      : 1;
  __IO uint16_t SIZE6      : 1;
       uint16_t RESERVED1  : 2;
  __IO uint16_t SPK        : 1;
  __IO uint16_t DRQO       : 1;
       uint16_t RESERVED2  : 2;
  __IO uint16_t SPKIE      : 1;
  __IO uint16_t DRQOIE     : 1;
  __IO uint16_t BFINI      : 1;
} stc_usbn_ep0os_field_t;

typedef struct stc_usbn_ep1s_field
{
  __IO uint16_t SIZE10     : 1;
  __IO uint16_t SIZE11     : 1;
  __IO uint16_t SIZE12     : 1;
  __IO uint16_t SIZE13     : 1;
  __IO uint16_t SIZE14     : 1;
  __IO uint16_t SIZE15     : 1;
  __IO uint16_t SIZE16     : 1;
  __IO uint16_t SIZE17     : 1;
  __IO uint16_t SIZE18     : 1;
  __IO uint16_t SPK        : 1;
  __IO uint16_t DRQ        : 1;
  __IO uint16_t BUSY       : 1;
       uint16_t RESERVED1  : 1;
  __IO uint16_t SPKIE      : 1;
  __IO uint16_t DRQIE      : 1;
  __IO uint16_t BFINI      : 1;
} stc_usbn_ep1s_field_t;

typedef struct stc_usbn_ep2s_field
{
  __IO uint16_t SIZE20     : 1;
  __IO uint16_t SIZE21     : 1;
  __IO uint16_t SIZE22     : 1;
  __IO uint16_t SIZE23     : 1;
  __IO uint16_t SIZE24     : 1;
  __IO uint16_t SIZE25     : 1;
  __IO uint16_t SIZE26     : 1;
       uint16_t RESERVED1  : 2;
  __IO uint16_t SPK        : 1;
  __IO uint16_t DRQ        : 1;
  __IO uint16_t BUSY       : 1;
       uint16_t RESERVED2  : 1;
  __IO uint16_t SPKIE      : 1;
  __IO uint16_t DRQIE      : 1;
  __IO uint16_t BFINI      : 1;
} stc_usbn_ep2s_field_t;

typedef struct stc_usbn_ep4s_field
{
  __IO uint16_t SIZE40     : 1;
  __IO uint16_t SIZE41     : 1;
  __IO uint16_t SIZE42     : 1;
  __IO uint16_t SIZE43     : 1;
  __IO uint16_t SIZE44     : 1;
  __IO uint16_t SIZE45     : 1;
  __IO uint16_t SIZE46     : 1;
       uint16_t RESERVED1  : 2;
  __IO uint16_t SPK        : 1;
  __IO uint16_t DRQ        : 1;
  __IO uint16_t BUSY       : 1;
       uint16_t RESERVED2  : 1;
  __IO uint16_t SPKIE      : 1;
  __IO uint16_t DRQIE      : 1;
  __IO uint16_t BFINI      : 1;
} stc_usbn_ep4s_field_t;

typedef struct stc_usbn_ep5s_field
{
  __IO uint16_t SIZE50     : 1;
  __IO uint16_t SIZE51     : 1;
  __IO uint16_t SIZE52     : 1;
  __IO uint16_t SIZE53     : 1;
  __IO uint16_t SIZE54     : 1;
  __IO uint16_t SIZE55     : 1;
  __IO uint16_t SIZE56     : 1;
       uint16_t RESERVED1  : 2;
  __IO uint16_t SPK        : 1;
  __IO uint16_t DRQ        : 1;
  __IO uint16_t BUSY       : 1;
       uint16_t RESERVED2  : 1;
  __IO uint16_t SPKIE      : 1;
  __IO uint16_t DRQIE      : 1;
  __IO uint16_t BFINI      : 1;
} stc_usbn_ep5s_field_t;


typedef struct stc_usbn_epns_field
{
  __IO uint16_t SIZE       : 8;
  __IO uint16_t SPK        : 1;
  __IO uint16_t DRQ        : 1;
  __IO uint16_t BUSY       : 1;
       uint16_t RESERVED1  : 1;
  __IO uint16_t SPKIE      : 1;
  __IO uint16_t DRQIE      : 1;
  __IO uint16_t BFINI      : 1;
} stc_usbn_epns_field_t;

typedef struct stc_usbn_epns
{
    union {
        __IO uint16_t EPNS;
        stc_usbn_epns_field_t EPNS_f;
    };
    uint8_t RESERVED[2];
} stc_usbn_epns_t;

typedef struct stc_usbn_epnc_field
{
  __IO uint16_t PKS        : 9;
  __IO uint16_t STAL       : 1;
  __IO uint16_t NULE       : 1;
  __IO uint16_t DMAE       : 1;
  __IO uint16_t DIR        : 1;
  __IO uint16_t TYPE0      : 1;
  __IO uint16_t TYPE1      : 1;
  __IO uint16_t EPEN       : 1;
} stc_usbn_epnc_field_t;

typedef struct stc_usbn_epnc
{
    union {
        __IO uint16_t EPNC;
        stc_usbn_epnc_field_t EPNC_f;
    };
    uint8_t RESERVED[2];

} stc_usbn_epnc_t;

typedef struct stc_usbn_epdt
{
  union {
    __IO uint16_t EPNDT;
    struct {
      __IO  uint8_t EPNDTL;
      __IO  uint8_t EPNDTH;
    };
  };
  uint8_t RESERVED[2];
} stc_usbn_epdt_t;

/******************************************************************************
 * USB_MODULE
 ******************************************************************************/
/* USB channel 0 registers */
typedef struct stc_usbn
{
  union {
    union {
      __IO uint16_t HCNT;
      stc_usbn_hcnt_field_t HCNT_f;
    };
    struct {
      union {
        __IO  uint8_t HCNT0;
        stc_usbn_hcnt0_field_t HCNT0_f;
      };
      union {
        __IO  uint8_t HCNT1;
        stc_usbn_hcnt1_field_t HCNT1_f;
      };
    };
  };
        uint8_t RESERVED0[2];
  union {
    __IO  uint8_t HIRQ;
    stc_usbn_hirq_field_t HIRQ_f;
  };
  union {
    __IO  uint8_t HERR;
    stc_usbn_herr_field_t HERR_f;
  };
        uint8_t RESERVED1[2];
  union {
    __IO  uint8_t HSTATE;
    stc_usbn_hstate_field_t HSTATE_f;
  };
  union {
    __IO  uint8_t HFCOMP;
    stc_usbn_hfcomp_field_t HFCOMP_f;
  };
        uint8_t RESERVED2[2];
  union {
    union {
      __IO uint16_t HRTIMER;
      stc_usbn_hrtimer_field_t HRTIMER_f;
    };
    struct {
      union {
        __IO  uint8_t HRTIMER0;
        stc_usbn_hrtimer0_field_t HRTIMER0_f;
      };
      union {
        __IO  uint8_t HRTIMER1;
        stc_usbn_hrtimer1_field_t HRTIMER1_f;
      };
    };
  };
        uint8_t RESERVED3[2];
  union {
    __IO  uint8_t HRTIMER2;
    stc_usbn_hrtimer2_field_t HRTIMER2_f;
  };
  union {
    __IO  uint8_t HADR;
    stc_usbn_hadr_field_t HADR_f;
  };
        uint8_t RESERVED4[2];
  union {
    union {
      __IO uint16_t HEOF;
      stc_usbn_heof_field_t HEOF_f;
    };
    struct {
      union {
        __IO  uint8_t HEOF0;
        stc_usbn_heof0_field_t HEOF0_f;
      };
      union {
        __IO  uint8_t HEOF1;
        stc_usbn_heof1_field_t HEOF1_f;
      };
    };
  };
        uint8_t RESERVED5[2];
  union {
    union {
      __IO uint16_t HFRAME;
      stc_usbn_hframe_field_t HFRAME_f;
    };
    struct {
      union {
        __IO  uint8_t HFRAME0;
        stc_usbn_hframe0_field_t HFRAME0_f;
      };
      union {
        __IO  uint8_t HFRAME1;
        stc_usbn_hframe1_field_t HFRAME1_f;
      };
    };
  };
        uint8_t RESERVED6[2];
  union {
    __IO  uint8_t HTOKEN;
    stc_usbn_htoken_field_t HTOKEN_f;
  };
        uint8_t RESERVED7[3];
  union {
    __IO uint16_t UDCC;
    stc_usbn_udcc_field_t UDCC_f;
  };
        uint8_t RESERVED8[2];
  union {
    __IO uint16_t EP0C;
    stc_usbn_ep0c_field_t EP0C_f;
  };
        uint8_t RESERVED9[2];
  union {
    __IO uint16_t EP1C;
    stc_usbn_ep1c_field_t EP1C_f;
  };
        uint8_t RESERVED10[2];
  union {
    __IO uint16_t EP2C;
    stc_usbn_ep2c_field_t EP2C_f;
  };
        uint8_t RESERVED11[2];
  union {
    __IO uint16_t EP3C;
    stc_usbn_ep3c_field_t EP3C_f;
  };
        uint8_t RESERVED12[2];
  union {
    __IO uint16_t EP4C;
    stc_usbn_ep4c_field_t EP4C_f;
  };
        uint8_t RESERVED13[2];
  union {
    __IO uint16_t EP5C;
    stc_usbn_ep5c_field_t EP5C_f;
  };
        uint8_t RESERVED14[2];
  union {
    __IO uint16_t TMSP;
    stc_usbn_tmsp_field_t TMSP_f;
  };
        uint8_t RESERVED15[2];
  union {
    __IO  uint8_t UDCS;
    stc_usbn_udcs_field_t UDCS_f;
  };
  union {
    __IO  uint8_t UDCIE;
    stc_usbn_udcie_field_t UDCIE_f;
  };
        uint8_t RESERVED16[2];
  union {
    __IO uint16_t EP0IS;
    stc_usbn_ep0is_field_t EP0IS_f;
  };
        uint8_t RESERVED17[2];
  union {
    __IO uint16_t EP0OS;
    stc_usbn_ep0os_field_t EP0OS_f;
  };
        uint8_t RESERVED18[2];
  union {
    __IO uint16_t EP1S;
    stc_usbn_ep1s_field_t EP1S_f;
  };
        uint8_t RESERVED19[2];
  union {
    __IO uint16_t EP2S;
    stc_usbn_ep2s_field_t EP2S_f;
  };
        uint8_t RESERVED20[2];
  __IO uint16_t EP3S;
        uint8_t RESERVED21[2];
  union {
    __IO uint16_t EP4S;
    stc_usbn_ep4s_field_t EP4S_f;
  };
        uint8_t RESERVED22[2];
  union {
    __IO uint16_t EP5S;
    stc_usbn_ep5s_field_t EP5S_f;
  };
        uint8_t RESERVED23[2];
  union {
    __IO uint16_t EP0DT;
    struct {
      __IO  uint8_t EP0DTL;
      __IO  uint8_t EP0DTH;
    };
  };
        uint8_t RESERVED24[2];
  union {
    __IO uint16_t EP1DT;
    struct {
      __IO  uint8_t EP1DTL;
      __IO  uint8_t EP1DTH;
    };
  };
        uint8_t RESERVED25[2];
  union {
    __IO uint16_t EP2DT;
    struct {
      __IO  uint8_t EP2DTL;
      __IO  uint8_t EP2DTH;
    };
  };
        uint8_t RESERVED26[2];
  union {
    __IO uint16_t EP3DT;
    struct {
      __IO  uint8_t EP3DTL;
      __IO  uint8_t EP3DTH;
    };
  };
        uint8_t RESERVED27[2];
  union {
    __IO uint16_t EP4DT;
    struct {
      __IO  uint8_t EP4DTL;
      __IO  uint8_t EP4DTH;
    };
  };
        uint8_t RESERVED28[2];
  union {
    __IO uint16_t EP5DT;
    struct {
      __IO  uint8_t EP5DTL;
      __IO  uint8_t EP5DTH;
    };
  };
} stc_usbn_t;
#define haha123 1
//@} // USB_MODULE
/**
 ******************************************************************************
 ** \endcond USB_MODULE End USB Module
 ******************************************************************************/
 
 
/******************************************************************************
 ** \brief String Descriptor
 ** 
 ******************************************************************************/
typedef struct stc_usbdevice_stringdescriptor
{
    uint8_t* pu8String;             ///< String as ASCII
    uint8_t* pu8UniCodeString;      ///< String as Unicode
} stc_usbdevice_stringdescriptor_t;

/******************************************************************************
 ** \brief Report Descriptor
 ** 
 ******************************************************************************/
typedef struct stc_usbdevice_reportdescriptor
{
    uint8_t* pu8Descriptor;          ///< Descriptor Data
    uint16_t u16Size;                ///< Size
} stc_usbdevice_reportdescriptor_t;

/******************************************************************************
 ** \brief Timeout handler
 **
 ** Used to create timeout callbacks via SOF interrupt
 ** 
 ******************************************************************************/
typedef struct stc_usb_sof_timeout_handler
{
    uint16_t u16TimeOut;                     ///< Timeout in ms
    void (* Handler)(stc_usbn_t* pstcUsb);  ///< Callback after timeout
} stc_usb_sof_timeout_handler_t;


/******************************************************************************
 ** \brief GPIO / external interrupt settings enumeration
 ** 
 ******************************************************************************/
typedef enum  en_usb_extint_param
{
    UsbExtIntDeinit         = 0, ///< Deinitialize GPIO / external interrupt
    UsbExtIntInit           = 1, ///< Initialize GPIO / external interrupt
    UsbExtIntDisableIsr     = 2, ///< Disable external interrupt
    UsbExtIntEnableIsr      = 3, ///< Enable external interrupt
    UsbExtIntClearIsrFlag   = 4, ///< Clear external interrupt flag
    UsbExtIntIsSetIsrFlag   = 5, ///< Check external interrupt flag is set
    UsbExtIntSetLowDetect   = 6, ///< Set low level external interrupt
    UsbExtIntSetHighDetect  = 7, ///< Set high level external interrupt
    UsbExtIntGetLevel       = 8  ///< Get level of external interrupt
} en_usb_extint_param_t;


/******************************************************************************
 ** \brief GPIO settings enumeration
 ** 
 ******************************************************************************/
typedef enum  en_usb_gpio_param
{
    UsbGpioDeinit           = 0, ///< Deinitialize GPIO
    UsbGpioInit             = 1, ///< Initialize GPIO
    UsbGpioSet              = 2, ///< Set GPIO
    UsbGpioClear            = 3, ///< Clear GPIO
    UsbGpioGet              = 4  ///< Get state of GPIO
} en_usb_gpio_param_t;


/******************************************************************************
 ** \brief Device alternative HCONX callback
 **
 ** \param en_usb_gpio_param_t type of callback
 **
 ** \return depending of type of callback TRUE or FALSE
 ** 
 ******************************************************************************/
typedef boolean_t (*usb_device_alternative_hconx_t)(en_usb_gpio_param_t enType);

/******************************************************************************
 ** \brief Device VBUS callback
 **
 ** \param en_usb_gpio_param_t type of callback
 **
 ** \return depending of type of callback TRUE or FALSE
 ** 
 ******************************************************************************/
typedef boolean_t (*usb_device_vbus_t)(en_usb_extint_param_t enType);

/******************************************************************************
 ** \brief Host VBUS callback
 **
 ** \param en_usb_gpio_param_t type of callback
 **
 ** \return depending of type of callback TRUE or FALSE
 ** 
 ******************************************************************************/
typedef boolean_t (*usb_host_vbus_t)(en_usb_gpio_param_t enType);

/******************************************************************************
 ** \brief Host pull-down callback
 **
 ** \param en_usb_gpio_param_t type of callback
 **
 ** \return depending of type of callback TRUE or FALSE
 ** 
 ******************************************************************************/
typedef boolean_t (*usb_host_pulldown_t)(en_usb_gpio_param_t enType);

/******************************************************************************
 ** \brief Host overcurrent callback
 **
 ** \param en_usb_gpio_param_t type of callback
 **
 ** \return depending of type of callback TRUE or FALSE
 ** 
 ******************************************************************************/
typedef boolean_t (*usb_host_overcurrent_t)(en_usb_extint_param_t enType);


/******************************************************************************
 ** \brief USB endpoint type
 ** 
 ******************************************************************************/
typedef enum en_usb_ep_type
{
    EpTypeHost      = 0, ///< used as host endpoint
    EpTypeIso       = 1, ///< ISO
    EpTypeBulk      = 2, ///< Bulk
    EpTypeInterrupt = 3  ///< Interrupt
} en_usb_ep_type_t;


/******************************************************************************
 ** \brief USB endpoint buffer
 ** 
 ******************************************************************************/
typedef struct stc_usbn_endpoint_buffer
{
    uint8_t* pu8Buffer;       ///< Buffer
    uint8_t* pu8BufferPos;    ///< Current position
    uint32_t u32BufferSize;   ///< Buffer size
    uint32_t u32DataSize;     ///< Data size
} stc_usbn_endpoint_buffer_t;

struct stc_usbn_endpoint_data;
struct stc_usb_config;

/******************************************************************************
 ** \brief USB device init callback
 ** 
 ** \param pstcUsb USB handle
 **
 ** \param pstcConfig pointer to configurtion #stc_usb_config_t
 **
 ******************************************************************************/
typedef void (*usb_device_init_t)(stc_usbn_t* pstcUsb, struct stc_usb_config* pstcConfig);

/******************************************************************************
 ** \brief USB host init callback
 ** 
 ** \param pstcUsb USB handle
 **
 ** \param pstcConfig pointer to configurtion #stc_usb_config_t
 **
 ******************************************************************************/
typedef void (*usb_host_init_t)(stc_usbn_t* pstcUsb,struct stc_usb_config* pstcConfig);

/******************************************************************************
 ** \brief USB endpoint data transferred callback
 ** 
 ** \param pstcUsb USB handle
 **
 ** \param pstcEndpoint endoint handle
 **
 ******************************************************************************/
typedef void (*usb_endpoint_datatransferred_func_ptr_t)(stc_usbn_t* pstcUsb, struct stc_usbn_endpoint_data* pstcEndpoint);

/******************************************************************************
 ** \brief USB connect callback
 ** 
 ** \param pstcUsb USB handle
 **
 ******************************************************************************/
typedef void (*usb_connect_func_ptr_t)(stc_usbn_t* pstcUsb);

/******************************************************************************
 ** \brief USB disconnect callback
 ** 
 ** \param pstcUsb USB handle
 **
 ******************************************************************************/
typedef void (*usb_disconnect_func_ptr_t)(stc_usbn_t* pstcUsb);

/******************************************************************************
 ** \brief USB device configuration changed callback
 ** 
 ** \param pstcUsb USB handle
 **
 ******************************************************************************/
typedef void (*usb_conf_isr_func_ptr_t)(stc_usbn_t* pstcUsb);

/******************************************************************************
 ** \brief USB device setup received callback
 ** 
 ** \param pstcUsb USB handle
 **
 ******************************************************************************/
typedef void (*usb_setp_isr_func_ptr_t)(stc_usbn_t* pstcUsb);

/******************************************************************************
 ** \brief USB suspend callback
 ** 
 ** \param pstcUsb USB handle
 **
 ******************************************************************************/
typedef void (*usb_susp_isr_func_ptr_t)(stc_usbn_t* pstcUsb);

/******************************************************************************
 ** \brief USB wakeup callback
 ** 
 ** \param pstcUsb USB handle
 **
 ******************************************************************************/
typedef void (*usb_wkup_isr_func_ptr_t)(stc_usbn_t* pstcUsb);

/******************************************************************************
 ** \brief USB bus reset callback
 ** 
 ** \param pstcUsb USB handle
 **
 ******************************************************************************/
typedef void (*usb_brst_isr_func_ptr_t)(stc_usbn_t* pstcUsb);

/******************************************************************************
 ** \brief USB start of frame callback
 ** 
 ** \param pstcUsb USB handle
 **
 ******************************************************************************/
typedef void (*usb_sof_isr_func_ptr_t)(stc_usbn_t* pstcUsb);

/******************************************************************************
 ** \brief USB host token canceled callback
 ** 
 ** \param pstcUsb USB handle
 **
 ******************************************************************************/
typedef void (*usb_tcan_isr_func_ptr_t)(stc_usbn_t* pstcUsb);

/******************************************************************************
 ** \brief USB host remote wakeup callback
 ** 
 ** \param pstcUsb USB handle
 **
 ******************************************************************************/
typedef void (*usb_rwkirq_isr_func_ptr_t)(stc_usbn_t* pstcUsb);

/******************************************************************************
 ** \brief USB host bus reset callback
 ** 
 ** \param pstcUsb USB handle
 **
 ******************************************************************************/
typedef void (*usb_urirq_isr_func_ptr_t)(stc_usbn_t* pstcUsb);

/******************************************************************************
 ** \brief USB host token complete callback
 ** 
 ** \param pstcUsb USB handle
 **
 ******************************************************************************/
typedef void (*usb_cmpirq_isr_func_ptr_t)(stc_usbn_t* pstcUsb);

/******************************************************************************
 ** \brief USB host connect callback
 ** 
 ** \param pstcUsb USB handle
 **
 ******************************************************************************/
typedef void (*usb_cnnirq_isr_func_ptr_t)(stc_usbn_t* pstcUsb);

/******************************************************************************
 ** \brief USB host disconnect callback
 ** 
 ** \param pstcUsb USB handle
 **
 ******************************************************************************/
typedef void (*usb_dirq_isr_func_ptr_t)(stc_usbn_t* pstcUsb);

/**
 ******************************************************************************
 ** \brief USB control stage
 **
 ******************************************************************************/
typedef enum en_usb_endpoint_status
{
    UsbEndpointStatusNotReady = 0,  ///< USB endpoint not ready
    UsbEndpointStatusIdle = 1,      ///< USB endpoint status idle
    UsbEndpointStatusTransfer = 2,  ///< USB endpoint status transferring data
    UsbEndpointStatusStall = 3,     ///< USB endpoint status stall
} en_usb_endpoint_status_t;


/******************************************************************************
 ** \brief USB HAL endpoint informations
 ** 
 **
 ******************************************************************************/
typedef struct stc_usbn_endpoint_data
{
  volatile uint8_t  u8EndpointAddress;                                    ///< endpoint address
  volatile uint16_t u16EndpointSize;                                      ///< FIFO size
  __IO uint16_t* pstcEpStatusRegister;                                    ///< status register
  __IO uint16_t* pstcEpControlRegister;                                   ///< control register
  __IO uint16_t* pstcEpDataRegister;                                      ///< data register
  stc_usbn_endpoint_buffer_t* pstcEndpointBuffer;                         ///< endpoint buffer
  volatile usb_endpoint_datatransferred_func_ptr_t pfnRxTxCallback;       ///< data transferred callback
  volatile usb_endpoint_datatransferred_func_ptr_t pfnRxTxBlockCallback;  ///< data block transferred callback
  boolean_t bAutomaticNullTermination;                                    ///< automatic null termination
  volatile boolean_t bIsActive;                                           ///< Endpoint is sending / receiving?
  stc_usbn_t*  pstcUsbInstance;                                           ///< Ref. USB instance

} stc_usbn_endpoint_data_t;

/// Enumeration to define an index for each enabled USB instance
typedef enum en_usb_instance_index
{
    #if (FM_PERIPHERAL_ENABLE_USB0 == ON)
    UsbInstanceIndexUsb0,
    #endif
    // Case if no DT instance is selected
    #if (FM_PERIPHERAL_ENABLE_USB1 == ON)
    UsbInstanceIndexUsb1,
    #endif
    UsbInstanceIndexMax
} en_usb_instance_index_t;

/// Enumeration to define an index for each enabled USB DMA instance
typedef enum en_usb_dma_instance_index
{
#if USB_USES_DMA_0 == ON
    UsbDmaInstanceIndex0,
#endif
#if USB_USES_DMA_1 == ON
    UsbDmaInstanceIndex1,
#endif
#if USB_USES_DMA_2 == ON
    UsbDmaInstanceIndex2,
#endif
#if USB_USES_DMA_3 == ON
    UsbDmaInstanceIndex3,
#endif
#if USB_USES_DMA_4 == ON
    UsbDmaInstanceIndex4,
#endif
#if USB_USES_DMA_5 == ON
    UsbDmaInstanceIndex5,
#endif
#if USB_USES_DMA_6 == ON
    UsbDmaInstanceIndex6,
#endif
#if USB_USES_DMA_7 == ON
    UsbDmaInstanceIndex7,
#endif
    UsbDmaInstanceIndexMax
} en_usb_dma_instance_index_t;

/******************************************************************************
 ** \brief USB internal data
 ** 
 **
 ******************************************************************************/
typedef struct stc_usb_intern_data
{
  boolean_t bHostActive;                          ///< Host active?
  boolean_t bDeviceActive;                        ///< Device active?
  boolean_t bHostEnabled;                         ///< Host enabled?
  boolean_t bDeviceEnabled;                       ///< Device enabled?
  boolean_t bUsbInstanceSet;                      ///< USB was initialized?
  volatile boolean_t bSofTimeoutEnabled;          ///< Timeout handling via SOF?
  usb_conf_isr_func_ptr_t pfnConfCallback;        ///< Configuration changed callback
  usb_setp_isr_func_ptr_t pfnSetpCallback;        ///< Setup received callback
  usb_susp_isr_func_ptr_t pfnSuspCallback;        ///< Suspend callback
  usb_wkup_isr_func_ptr_t pfnWkupCallback;        ///< Wakeup callback
  usb_brst_isr_func_ptr_t pfnBrstCallback;        ///< Bus reset callback
  usb_sof_isr_func_ptr_t  pfnSofCallback;         ///< Start of frame callback 
  usb_dirq_isr_func_ptr_t  pfnDirqCallback;       ///< USB host disconnecion callback
  usb_cnnirq_isr_func_ptr_t  pfnCnnirqCallback;   ///< USB host connecion callback
  usb_cmpirq_isr_func_ptr_t  pfnCmpirqCallback;   ///< USB host token complete callback
  usb_urirq_isr_func_ptr_t  pfnUrirqCallback;     ///< USB host bus reset callback
  usb_rwkirq_isr_func_ptr_t  pfnRwkirqCallback;   ///< USB host remote wakeup callback
  usb_tcan_isr_func_ptr_t  pfnTcanCallback;       ///< USB host token cancelled callback
  usb_device_init_t pfnDeviceInit;                ///< USB device init callback
  usb_host_init_t pfnHostInit;                    ///< USB host init callback
  usb_device_vbus_t pfnDeviceVbus;                ///< USB device vbus callback
  usb_host_vbus_t pfnHostVbus;                    ///< USB host vbus callback
  usb_host_pulldown_t pfnHostPullDownHostEnable;  ///< USB host pull-down callback
  usb_host_overcurrent_t pfnHostOvercurrent;      ///< USB host overcurrent callback
  uint8_t u8NumberOfEndpoints;                    ///< USB number of HAL endpoints
  #if (FM_PERIPHERAL_USB_DEVICE_ENABLED == ON)
  stc_usbn_endpoint_data_t astcEndpoints[7];      ///< Endpoint handles
  uint8_t* pu8DeviceDescriptor;                   ///< Device descriptor buffer
  uint8_t* pu8ConfigDescriptor;                   ///< Configuration descriptor buffer
  #else
  stc_usbn_endpoint_data_t astcEndpoints[2];      ///< Endpoint handles
  #endif
  stc_usb_sof_timeout_handler_t pstcUsbTimeOutHandler[5]; ///< Timeout handles
} stc_usb_intern_data_t; 

/// USB module internal data, storing internal information for each enabled USB instance.
typedef struct stc_usbn_instance_data
{
    volatile stc_usbn_t*  pstcInstance;  ///< pointer to registers of an instance
    stc_usb_intern_data_t stcInternData; ///< module internal data of instance
} stc_usbn_instance_data_t;


/**
 ******************************************************************************
 ** \brief USB control stage
 **
 ******************************************************************************/
typedef enum en_usb_control_stage
{
  ControlStageUnknown = 0, ///< USB control stage unknown
  SetupStage          = 1, ///< USB control stage setup
  DataStageIN         = 2, ///< USB control stage data IN
  DataStageOUT        = 3, ///< USB control stage data OUT
  StatusStage         = 4  ///< USB control stage status
} en_usb_control_stage_t;


/**
 ******************************************************************************
 ** \brief Usb Status
 **
 ******************************************************************************/
typedef enum en_usb_status
{
  UsbDisconnected = 0, ///< USB is disconnected
  UsbConnected    = 1, ///< USB is connected
  UsbAddressed    = 2, ///< USB is addressed
  UsbConfigured   = 3  ///< USB is configured
} en_usb_status_t;

/**
 ******************************************************************************
 ** \brief Usb mode
 ** 
 ** To select between device and host
 **
 ******************************************************************************/
typedef enum en_usb_mode
{
  UsbDisabled          = 0,  ///< USB disabled
  UsbDeviceEnabled     = 1,  ///< USB device enabled, USB host disabled
  UsbHostEnabled       = 2,  ///< USB device disabled, USB host enabled
  UsbHostDeviceEnabled = 3   ///< USB device enabled, USB host enabled
} en_usb_mode_t;

/**
 ******************************************************************************
 ** \brief Usb force switch to
 **
 ******************************************************************************/
typedef enum en_usb_switch
{
  UsbSwitchDependingDeviceVbus  = 0,  ///< Do not force switch
  UsbSwitchToDevice             = 1,  ///< USB device enabled, USB host disabled
  UsbSwitchToHost               = 2,  ///< USB device disabled, USB host enabled
  UsbSwitchAllOff               = 3   ///< USB device disabled, USB host disabled
} en_usb_switch_t;


/**
 ******************************************************************************
 ** \brief Usb configuration.
 **
 ** Contains all parameter for configuratin an USB channel.  
 ******************************************************************************/
typedef struct stc_usb_config
{
    en_usb_mode_t                             enMode;          ///< See description of #en_usb_mode_t.
    boolean_t                                 bUseInterrupts;  ///< Use interrupts?
    #if (FM_PERIPHERAL_USB_DEVICE_ENABLED == ON)
    uint8_t* pu8DeviceDescriptor;                                ///< Usb Device Descriptor buffer
    uint8_t* pu8ConfigDescriptor;                                ///< Usb Configuration Descriptor buffer
    #endif
    usb_conf_isr_func_ptr_t                   pfnConfCallback;   ///< Configuration changed callback
    usb_setp_isr_func_ptr_t                   pfnSetpCallback;   ///< Setup received callback
    usb_susp_isr_func_ptr_t                   pfnSuspCallback;   ///< Device suspend callback
    usb_wkup_isr_func_ptr_t                   pfnWkupCallback;   ///< Device wakeup callback
    usb_brst_isr_func_ptr_t                   pfnBrstCallback;   ///< Device busreset callback
    usb_sof_isr_func_ptr_t                    pfnSofCallback;    ///< Start of frame callback
    usb_dirq_isr_func_ptr_t                   pfnDirqCallback;   ///< Host disconnection callback
    usb_cnnirq_isr_func_ptr_t                 pfnCnnirqCallback; ///< Host connection callback
    usb_cmpirq_isr_func_ptr_t                 pfnCmpirqCallback; ///< Host token completion callback
    usb_urirq_isr_func_ptr_t                  pfnUrirqCallback;  ///< Host bus reset callback
    usb_rwkirq_isr_func_ptr_t                 pfnRwkirqCallback; ///< Host remote wakeup callback
    usb_tcan_isr_func_ptr_t                   pfnTcanCallback;   ///< Host token canceled callback
    usb_device_init_t                         pfnDeviceInit;     ///< Device init callback
    usb_host_init_t                           pfnHostInit;       ///< Host init callback
    usb_device_vbus_t                         pfnDeviceVbus;     ///< Device VBUS callback
    usb_host_vbus_t                           pfnHostVbus;       ///< Host VBUS callback
    usb_host_pulldown_t                       pfnHostPullDownHostEnable; ///< Host pull-down callback
    usb_host_overcurrent_t                    pfnHostOvercurrent;        ///< Host overcurrent callback
} stc_usb_config_t;

#if USB_USES_DMA == ON
/**
 ******************************************************************************
 ** \brief Usb DMA handle
 **
 ** Contains all parameter for configuratin an USB channel.  
 ******************************************************************************/
typedef struct stc_usb_dma
{
    boolean_t bIsActive;
    IRQn_Type u8IRQ;
    stc_usbn_endpoint_data_t* pstcEpHandle;
    __IO uint32_t* pstcDMACA;
    __IO uint32_t* pstcDMACB;
    __IO uint32_t* pstcDMACSA;
    __IO uint32_t* pstcDMACDA;
    
} stc_usb_dma_t;
#endif

/******************************************************************************/
/* Global variable definitions ('extern')                                     */
/******************************************************************************/

/// Look-up table for all enabled USB instances and their internal data
extern stc_usbn_instance_data_t m_astcUsbInstanceDataLut[];

#if (USB_ENABLED == ON)

en_result_t Usb_Configure(stc_usbn_t* pstcUsb, stc_usb_config_t* pstcConfig);

en_result_t Usb_Init( void );

en_result_t Usb_SetupHalEndpoint(stc_usbn_t* pstcUsb,uint8_t u8EndpointAddress , uint16_t u16FifoSize, en_usb_ep_type_t enType, boolean_t bInterruptsEnabled);

en_result_t Usb_SwitchUsb(stc_usbn_t* pstcUsb, en_usb_switch_t enType, uint32_t u32SwitchDelay);

en_result_t Usb_InitInstance(stc_usbn_t* pstcUsb);

en_result_t Usb_DeinitInstance(stc_usbn_t* pstcUsb);

stc_usbn_endpoint_data_t* Usb_GetEndpointPtr(stc_usbn_t* pstcUsb, uint8_t u8EndpointAddress);

en_result_t Usb_HalSend(stc_usbn_endpoint_data_t* pstcEpHandle, uint8_t* pu8Data, uint16_t u16Size, uint16_t* pu16ByteCount);

en_result_t Usb_HalReceive(stc_usbn_endpoint_data_t* pstcEpHandle, uint8_t* pu8Data, uint16_t* pu16ByteCount, boolean_t bClearDrq);

en_result_t Usb_HalClearDrq(stc_usbn_endpoint_data_t* pstcEpHandle);

en_result_t Usb_HalStallEndpoint(stc_usbn_endpoint_data_t* pstcEpHandle);

en_result_t Usb_HalUnstallEndpoint(stc_usbn_endpoint_data_t* pstcEpHandle);

boolean_t Usb_HalEpReady(stc_usbn_endpoint_data_t* pstcEpHandle);

boolean_t Usb_HalEndpointIsBusy(stc_usbn_endpoint_data_t* pstcEpHandle);

uint8_t Usb_GetConfiguration(stc_usbn_t* pstcUsb);

boolean_t Usb_AddTimeOut(stc_usbn_t* pstcUsb, void (* Handler)(stc_usbn_t* pstcUsb), uint16_t u16TimeOut);

void Usb_RemoveTimeOut(stc_usbn_t* pstcUsb, void (* Handler)(stc_usbn_t* pstcUsb));

void Usb_SetEndpointRxTxCallback(stc_usbn_endpoint_data_t* pstcEndpoint, usb_endpoint_datatransferred_func_ptr_t pfnRxTxCallback);

#if USB_USES_DMA == ON
en_result_t Usb_HalSendDma(stc_usbn_endpoint_data_t* pstcEpHandle, uint8_t* pu8Data, uint16_t u16Size, uint16_t* pu16ByteCount);
#endif

#if (FM_PERIPHERAL_USB_DEVICE_ENABLED == ON)
extern void UsbIrqHandlerF(stc_usbn_t* pstcUsb, stc_usb_intern_data_t* pstcUsbInternData);
#endif

void Usb_OsTickHandle(void);

extern void UsbIrqHandler(stc_usbn_t* pstcUsb, stc_usb_intern_data_t* pstcUsbInternData);

void UsbConfig_UsbInit(void);

void UsbConfig_SwitchMode(void);

#endif /* USB_ENABLED == ON */

#ifdef __cplusplus
}
#endif

//@} // UsbGroup
#endif /* __USB_H__ */
/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
