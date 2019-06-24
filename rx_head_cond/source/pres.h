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
#include "mfs/mfs.h"

/*****************************************************************************/
/* Defines                                                                   */
/*****************************************************************************/


/*****************************************************************************/
/* Function prototype                                                        */
/*****************************************************************************/

void pres_init(void);
void pres_tick_10ms(void);
int  pres_valid(void);

void pres_del_user_offset(void);
void pres_calibration_start(void);
int  pres_calibration_done(void);

#endif /* __pres_h__ */
