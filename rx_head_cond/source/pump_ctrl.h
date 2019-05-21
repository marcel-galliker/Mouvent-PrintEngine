/******************************************************************************/
/** \file pump_ctrl.h
 ** 
 ** 
 **
 **	Copyright 2016 by radex AG, Switzerland. All rights reserved.
 **	Written by Marcel Galliker
 **
 ******************************************************************************/
 
#ifndef __PUMP_CTRL_H__
#define __PUMP_CTRL_H__

//--- includes --------------------------------------------------
#include "rx_ink_common.h"

void pump_init(void);
void pump_watchdog(void);
void pump_tick_10ms(void);
void pump_tick_1000ms(void);
void turn_off_pump(void);

#endif /* __PUMP_CTRL_H__ */
