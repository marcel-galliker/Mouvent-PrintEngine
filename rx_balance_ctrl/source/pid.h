/******************************************************************************/
/** \file pid.h
 ** 
 ** control loop function 
 **
 **	Copyright 2016 by radex AG, Switzerland. All rights reserved.
 **	Written by Marcel Galliker
 **
 ******************************************************************************/
 
#ifndef __PID_H__
#define __PID_H__

/*****************************************************************************/
/* Include files                                                             */
/*****************************************************************************/
#include "rx_ink_common.h"

/*****************************************************************************/
/* Defines                                                                   */
/*****************************************************************************/

typedef struct
{
	INT32 interval;            // Abtastzeit in ms

	INT32 I;                   	// Integralanteil
	INT32 P;                  	// Verstärkung
	INT32 D;                   	// Differenzieller Anteil

	INT32 val;					// Stellgröße
	INT32 val_min;
	INT32 val_max;
	INT32 val_invalid;

	INT32 diff_min;				// 

	//--- internal -----------------------------
	INT32 diff_old;             // Regelabweichung zum ZP z-1
	INT32 diff_sum;             // Summe der Regelabweichungen
} SPID_par;              	// Struktur PID_Einstellungen erzeugen

/*****************************************************************************/
/* Function prototype                                                        */
/*****************************************************************************/
void 	pid_reset(SPID_par* pPID);
void 	pid_calc (uint32_t actVal, uint32_t setVal, SPID_par* pPID);

#endif /* __PID_H__ */
