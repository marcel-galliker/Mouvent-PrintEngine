/******************************************************************************/
/** \file iomux.h
 ** 
 ** basic peripheral initialisation
 **
 **	Copyright 2016 by radex AG, Switzerland. All rights reserved.
 **	Written by Stefan Weber
 **
 ******************************************************************************/

#ifndef __IOMUX_H__
#define __IOMUX_H__

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "pdl.h"
#include "gpio/gpio.h"
#include "mfs/mfs.h"
#include "limits.h"

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/**
 ******************************************************************************
 ** \defgroup IomuxGroup IoMux Functions (IOMUX)
 **
 ** Provided functions of IOMUX module:
 ** 
 ** - Iomux_Init()
 ** - Iomux_DeInit()
 **
 ** Iomux_Init() initializes the GPIO ports and peripheral pin functions
 ** according to the user settings of the Pin Wizard.
 ** Iomux_DeInit() de-initializes the GPIO ports and peripheral pin functions
 ** according to the user settings of the Pin Wizard to the default values.
 **
 ******************************************************************************/
//@{

/*****************************************************************************/
/* Global pre-processor symbols/macros ('#define')                           */
/*****************************************************************************/

/******************************************************************************/
/* Local type definitions ('typedef')                                         */
/******************************************************************************/

/******************************************************************************/
/* Global variable definitions ('extern')                                     */
/******************************************************************************/

/******************************************************************************/
/* Global function prototypes (definition in C source)                        */
/******************************************************************************/
extern void Iomux_Init(void);
extern void Iomux_DeInit(void);

extern void prev_led_0(int state);
extern void prev_led_1(int state);
extern void next_led_0(int state);
extern void next_led_1(int state);
extern void lamp_switch(int state);	
extern void pump_pwm_0(int state);
extern void pump_pwm_1(int state);
extern void pump_rtry(int state);

//@} // IomuxGroup

#ifdef __cplusplus
}
#endif

#endif /* __IOMUX_H__ */
/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
