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
/** \file usbconfig.h
 **
 ** Part of FSEU USB Driver Module
 **
 ** History:
 **   - 2012-07-17    2.0  MSc  New Version for use with M3 L3 USB driver
 **   - 2012-10-02    2.1  MSc  use of external interrupts without L3 implemented
 **   - 2012-01-31    2.2  MSc  DMA settings added
 **   - 2013-06-04    2.3  MSc  FM4 support added
 **   - 2013-09-23    2.4  MSc  Version for PDL
 **   - 2013-11-29    2.5  MSc  PDL adjustments done
 *****************************************************************************/

#ifndef __USBCONFIG_H__
#define __USBCONFIG_H__

/*****************************************************************************/
/* Global pre-processor symbols/macros ('#define')                           */
/*****************************************************************************/

#define USEUSBDBG            0
#define USBWARNLEVEL         0           //0: no warnings, 1: hard warnings, 2: all warnings
#define USE_USBDESCRIPTORS_H 0           //<- 1 use usbdescriptors.h file for USB descriptors settings, 0 for disabling
#define USE_USBDEVICEHW_H    0           //<- 1 use usbdevicehw.h file for gpio settings, 0 for disabling
#define USE_USBHOSTHW_H      0           //<- 1 use usbhosthw.h file for gpio settings, 0 for disabling
#define USE_USBHOSTCLASSDRIVERTABLE_H 0  //<- 1 use usbhostclassdrivertable.h file
#define USB_USE_L3           0           //<- 1 use as part of L3 library, 0 for using without L3 library
#define USB_USE_PDL          1           //<- 1 use as part of PDL library, 0 for using without PDL library



/* START Middleware Modules */

#if ((USB_USE_PDL == 0) && (USB_USE_L3 == 0))
/* DEVICE */     
#define USBDEVICECDCCOM_ENABLED          OFF
#define USBDEVICEHIDCOM_ENABLED          OFF
#define USBDEVICEHIDJOYSTICK_ENABLED     OFF
#define USBDEVICEHIDKEYBOARD_ENABLED     OFF
#define USBDEVICEHIDMOUSE_ENABLED        OFF
#define USBDEVICELIBUSB_ENABLED          OFF
#define USBDEVICEPRINTER_ENABLED         OFF

/* HOST */
#define USBHOSTHIDCOM_ENABLED            OFF
#define USBHOSTHIDKEYBOARD_ENABLED       OFF
#define USBHOSTHIDMOUSE_ENABLED          OFF
#define USBHOSTMASSSTORAGE_ENABLED       OFF
     
/* END Middleware Modules */
     
/* only used if USB is used without L3 or PDL*/
#define USB0_HOST_ENABLED       0
#define USB0_DEVICE_ENABLED     0
#define USB1_HOST_ENABLED       0
#define USB1_DEVICE_ENABLED     0
#define USB0_DEVICE_IRQ_ENABLED 1
#define USB0_HOST_IRQ_ENABLED   1
#define USB1_DEVICE_IRQ_ENABLED 1
#define USB1_HOST_IRQ_ENABLED   1
#define USB_USE_EXT_INT         0
#define IRQ_LEVEL_USB0          3  
#define IRQ_LEVEL_USB1          3  
#define USB_USES_DMA            0  
#define USB_USES_DMA_0          1
#define USB_USES_DMA_1          1
#define USB_USES_DMA_2          0
#define USB_USES_DMA_3          0
#define USB_USES_DMA_4          0
#define USB_USES_DMA_5          0
#define USB_USES_DMA_6          0
#define USB_USES_DMA_7          0

#endif /* ((USB_USE_PDL == 0) && (USB_USE_L3 == 0)) */

#endif
