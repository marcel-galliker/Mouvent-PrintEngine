/******************************************************************************/
/** ink_ctrl.h
 ** 
 ** pressure sensors
 **
 **	Copyright 2016 by radex AG, Switzerland. All rights reserved.
 **	Written by Marcel Galliker
 **
 ******************************************************************************/
 
#ifndef __ink_ctrl_h__
#define __ink_ctrl_h__

void ink_init(void);
void ink_tick_10ms(void);
void ink_tick_1000ms(void);

#endif /* __ink_ctrl_h__ */
