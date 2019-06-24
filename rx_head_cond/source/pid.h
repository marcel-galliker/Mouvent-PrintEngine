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
	INT32 Setpoint;				// Meniscus setpoint
	INT32 P;                  	// Verstärkung
	INT32 I;                   	// Integralanteil
	INT32 P_start;             	// P divide by P_start during phase OFF->PRINT
	//INT32 norm;
   
	INT32 val;					// regulator value
	INT32 val_min;
	INT32 val_max;
	
	INT32 start_integrator;		// to start the integrator at the best time

	//--- internal -----------------------------
	//INT32 diff_delay;		// tineout for I-value to rampup
	//INT32 diff_old;			// old value for D
	INT32 diff_I;			// actual I-value
} SPID_par;              	// Struktur PID_Einstellungen erzeugen

/*****************************************************************************/
/* Function prototype                                                        */
/*****************************************************************************/
void 	pid_reset(SPID_par* pPID);
void 	pid_calc (INT32 actVal, SPID_par* pPID);

#endif /* __PID_H__ */
