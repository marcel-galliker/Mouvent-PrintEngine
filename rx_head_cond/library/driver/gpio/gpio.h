/*******************************************************************************
* Copyright (C) 2014 Spansion LLC. All Rights Reserved. 
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
/** \file gpio.h
 **
 ** Headerfile for GPIO functions
 **  
 ** History:
 **   - 2013-04-10  1.0  MWi    First version.
 **   - 2014-01-27  1.1  MWi    GpioGet(), GpioPut(), GpioInitOut(), amd
 **                             GpioInitIn() added
 **   - 2014-10-28  1.1a YI     Renew GPIO defines to support new FM4s
 **   - 2014-10-28  1.1b YI     Correct errors
 **
 ******************************************************************************/

#ifndef __GPIO_H__
#define __GPIO_H__

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
 ** \defgroup GpioGroup GPIO Defintions (GPIO)
 **
 ** Definitions of GPIO and resource pin relocation
 **
 ** \attention 
 **            - Carefully check in device documentation, whether SOUBOUT pin
 **              at SOUBOUT[_n] or TIOB0 pin should be output. TIOB0-SUBOUT
 **              is <b>not</b> provided by this driver!
 **            - Internal LSYN connection is not provided by this driver!
 ******************************************************************************/
//@{  
  
/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/

#define Gpio1pin_InitIn(p,settings)    do{ stc_gpio1pin_init_t __v__;\
                                         __v__.bPullup=0u;__v__.bInitVal=0u;\
                                         (settings);\
                                         p##_INITIN(__v__); }while(0)

#define Gpio1pin_InitOut(p,settings)   do{ stc_gpio1pin_init_t __v__;\
                                         __v__.bPullup=0u;__v__.bInitVal=0u;\
                                         (settings);\
                                         p##_INITOUT(__v__); }while(0)
                                      
#define Gpio1pin_Init(p,settings)      do{ stc_gpio1pin_init_t __v__;__v__.bOutput=0u;\
                                         __v__.bPullup=0u;__v__.bInitVal=0u;\
                                         (settings);\
                                         p##_INIT( __v__ ); }while(0) 

#define Gpio1pin_InitDirectionInput    (__v__.bOutput=0u)
#define Gpio1pin_InitDirectionOutput   (__v__.bOutput=1u)
#define Gpio1pin_InitPullup(v)         (__v__.bPullup=(v))
#define Gpio1pin_InitVal(v)            (__v__.bInitVal=(v))
		


#define Gpio1pin_Get(p)	         p##_GET  
#define Gpio1pin_Put(p,v)        p##_PUT(v)
  
#define PINRELOC_SET_EPFR(epfr,pos,width,value)    \
          ((epfr) = ((epfr) & ~(((1u<<(width))-1u)<<(pos)) | \
          ((value) << (pos))))

/******************************************************************************/
/* Parameterized Port Concatenators                                           */
/******************************************************************************/
#define GPIO_PASTER(x, p, y)           x##p##y
#define GPIO_EVALUATOR(x, p, y)        GPIO_PASTER(x, p, y)

#define GPIO_PASTER2(x, p1, p2, y)     x(##p1##p2##,##y)
#define GPIO_EVALUATOR2(x, p1, p2, y)  GPIO_PASTER2(x, p1, p2, y)

#define GpioGet(port)             GPIO_EVALUATOR(GPIO1PIN_, port, _GET)
#define GpioPut(port, value)      GPIO_EVALUATOR(GPIO1PIN_, port, _PUT( value ))

#define GpioInitOut(port, pullup) GPIO_EVALUATOR2(Gpio1pin_InitOut, GPIO1PIN_, port, Gpio1pin_InitVal( pullup ))
#define GpioInitIn(port, value)   GPIO_EVALUATOR2(Gpio1pin_InitIn,  GPIO1PIN_, port, Gpio1pin_InitPullup( value ))           
            
/******************************************************************************/
/* Inclusion of GPIO defines of user defined device                           */
/******************************************************************************/

#if (PDL_DEVICE_MB9BF16X == PDL_ON)
  #if (PDL_PACKAGE == PDL_DEVICE_PACKAGE_M)
    #include "gpio_mb9bf16xm.h"
  #elif (PDL_PACKAGE == PDL_DEVICE_PACKAGE_N)
    #include "gpio_mb9bf16xn.h"
  #elif (PDL_PACKAGE == PDL_DEVICE_PACKAGE_R)
    #include "gpio_mb9bf16xr.h"
  #elif (PDL_PACKAGE == PDL_DEVICE_PACKAGE_K)
    #include "gpio_mb9bf16xk.h"
  #elif (PDL_PACKAGE == PDL_DEVICE_PACKAGE_L)
    #include "gpio_mb9bf16xl.h"
  #else
    #error Package for MB9BF16X not found!
  #endif
#elif (PDL_DEVICE_MB9BF36X == PDL_ON)
  #if (PDL_PACKAGE == PDL_DEVICE_PACKAGE_M)
    #include "gpio_mb9bf36xm.h"
  #elif (PDL_PACKAGE == PDL_DEVICE_PACKAGE_N)
    #include "gpio_mb9bf36xn.h"
  #elif (PDL_PACKAGE == PDL_DEVICE_PACKAGE_R)
    #include "gpio_mb9bf36xr.h"
  #elif (PDL_PACKAGE == PDL_DEVICE_PACKAGE_K)
    #include "gpio_mb9bf36xk.h"
  #elif (PDL_PACKAGE == PDL_DEVICE_PACKAGE_L)
    #include "gpio_mb9bf36xl.h"
  #else
    #error Package for MB9BF36X not found!
  #endif
#elif (PDL_DEVICE_MB9BF46X == PDL_ON)
  #if (PDL_PACKAGE == PDL_DEVICE_PACKAGE_M)
    #include "gpio_mb9bf46xm.h"
  #elif (PDL_PACKAGE == PDL_DEVICE_PACKAGE_N)
    #include "gpio_mb9bf46xn.h"
  #elif (PDL_PACKAGE == PDL_DEVICE_PACKAGE_R)
    #include "gpio_mb9bf46xr.h"
  #elif (PDL_PACKAGE == PDL_DEVICE_PACKAGE_K)
    #include "gpio_mb9bf46xk.h"
  #elif (PDL_PACKAGE == PDL_DEVICE_PACKAGE_L)
    #include "gpio_mb9bf46xl.h"
  #else
    #error Package for MB9BF46X not found!
  #endif
#elif (PDL_DEVICE_MB9BF56X == PDL_ON)
  #if (PDL_PACKAGE == PDL_DEVICE_PACKAGE_M)
    #include "gpio_mb9bf56xm.h"
  #elif (PDL_PACKAGE == PDL_DEVICE_PACKAGE_N)
    #include "gpio_mb9bf56xn.h"
  #elif (PDL_PACKAGE == PDL_DEVICE_PACKAGE_R)
    #include "gpio_mb9bf56xr.h"
  #elif (PDL_PACKAGE == PDL_DEVICE_PACKAGE_K)
    #include "gpio_mb9bf56xk.h"
  #elif (PDL_PACKAGE == PDL_DEVICE_PACKAGE_L)
    #include "gpio_mb9bf56xl.h"
  #else
    #error Package for MB9BF56X not found!
  #endif
#elif (PDL_DEVICE_S6E2CCX == PDL_ON)
  #if (PDL_PACKAGE == PDL_DEVICE_PACKAGE_H)
    #include "gpio_s6e2ccxh.h"
  #elif (PDL_PACKAGE == PDL_DEVICE_PACKAGE_J)
    #include "gpio_s6e2ccxj.h"
  #elif (PDL_PACKAGE == PDL_DEVICE_PACKAGE_L)
    #include "gpio_s6e2ccxl.h"
  #else
    #error Package for S6E2CCX not found!
  #endif
#else
  #error Device not found!
#endif


/******************************************************************************/
/* Types                                                                      */
/******************************************************************************/

typedef struct stc_gpio1pin_init
{
    boolean_t bOutput;
    boolean_t bInitVal;
    boolean_t bPullup; 
} stc_gpio1pin_init_t;


//@} // GpioGroup

#ifdef __cplusplus
}
#endif

#endif /* __GPIO_H__ */

/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
