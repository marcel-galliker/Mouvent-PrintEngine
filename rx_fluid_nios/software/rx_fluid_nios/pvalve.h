/******************************************************************************/
/** pvalve.h
 **
 ** proportional valve
 **
 **	Copyright 2021 by radex AG, Switzerland. All rights reserved.
 **	Written by Marcel Galliker
 **
 ******************************************************************************/

#ifndef PVALVE_H_
#define PVALVE_H_

#define PV_CLOSED	0
#define PV_OPEN 	100

void pvalve_init(void);

int	pvalve_active(int isNo);

int pvalve_set_air(int isNo, int value);	// return 0==OK
int pvalve_set_bleed(int isNo, int value);// return 0==OK

#endif /* PVALVE_H_ */
