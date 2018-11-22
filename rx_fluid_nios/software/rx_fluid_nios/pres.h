/******************************************************************************/
/** pres.h
 ** 
 ** pressure sensors
 **
 **	Copyright 2016 by radex AG, Switzerland. All rights reserved.
 **	Written by Marcel Galliker
 **
 ******************************************************************************/
 
#ifndef __pres_h__
#define __pres_h__

/*****************************************************************************/
/* Include files                                                             */
/*****************************************************************************/

#include "rx_ink_common.h"

/*****************************************************************************/
/* Defines                                                                   */
/*****************************************************************************/

/*****************************************************************************/
/* Function prototype                                                        */
/*****************************************************************************/

void pres_init(void);
void pres_tick_10ms(void);
int  pres_valid(int isNo);


#endif /* __pres_h__ */
