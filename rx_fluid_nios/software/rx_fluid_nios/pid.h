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
	INT32 Setpoint;                // Pressure setpoint
	INT32 P;                // Verstärkung
	INT32 I;                // Integralanteil
	//INT32 D;                // Differenzieller Anteil
	//INT32 norm;
    //INT32 offset;

	INT32 val;				// regulator value
	INT32 val_min;
	INT32 val_max;

	//--- internal -----------------------------
	//INT32 I_rampup_to;		// tineout for I-value to rampup
	INT32 Start_Integrator;		// =1 to start the integrator
	//INT32 diff_old;			// old value for D
	INT32 diff_I;			// actual I-value
} SPID_par;              	// Struktur PID_Einstellungen erzeugen

/*****************************************************************************/
/* Function prototype                                                        */
/*****************************************************************************/
void 	pid_reset(SPID_par* pPID);
void 	pid_calc (const INT32 actVal, SPID_par* pPID);

#endif /* __PID_H__ */
