/******************************************************************************/
/** \file pid.c
 ** 
 ** pump regulator
 **
 **	Copyright 2016 by radex AG, Switzerland. All rights reserved.
 **	Written by Marcel Galliker
 **
 ******************************************************************************/
 
/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/

#include "pid.h"

void pid_reset(SPID_par* pPID)
{
	pPID->diff_I = 0;
}

/**
 ******************************************************************************
 ** \brief update PID
 **
 ** \param [in] actVal current value
 ** \param [in] setVal target value
 ** \param [in] pPID   Pointer to PID instance
 ** 
 ******************************************************************************/
void pid_calc(const INT32 actVal, SPID_par* pPID)
{
	INT32 diff;
	INT32 val;
	
	// check fo valid values
	if (actVal == INVALID_VALUE) diff = 0;
	else diff = pPID->Setpoint - actVal;

	if ((pPID->I > 0)&&(pPID->Start_Integrator))
    {
        // start integrating after timeout as in ramp up I value runs into anti windup
        if (pPID->val < pPID->val_max) pPID->diff_I += (diff * 10);
    }
	else pPID->diff_I = 0;

	// pid function
	val = pPID->P * (diff + (pPID->diff_I / pPID->I));  // + (pPID->D * (diff - pPID->diff_old)));
	val /= 1000;

	// check boundries
	if (val < pPID->val_min)	val = pPID->val_min;
	if (val > pPID->val_max)	val = pPID->val_max;

	// set values
	pPID->val = val;
}
