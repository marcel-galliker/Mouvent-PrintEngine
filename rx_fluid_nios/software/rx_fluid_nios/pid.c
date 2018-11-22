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
	pPID->diff_suming = 0;
	pPID->diff_old = INVALID_VALUE;
	if (pPID->norm <= 0) pPID->norm = 1;
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
void pid_calc(const INT32 actVal, const INT32 setVal, SPID_par* pPID)
{
	INT32 diff;
	INT32 val;
	
	diff = setVal - actVal;
	if (diff == 0)
		return;

	// check fo valid values
	if (actVal == INVALID_VALUE)
		return;
    
	if (pPID->diff_old == INVALID_VALUE)
		pPID->diff_old = diff;

	if (pPID->I > 0)
    {
        // start integrating after timeout as in ramp up I value runs into anti windup
        if (pPID->I_rampup_to)
        {
            pPID->I_rampup_to--;
            pPID->diff_suming = 0;
        }
        else
        {
			if ((pPID->val > pPID->val_min) && (pPID->val < pPID->val_max))
                pPID->diff_suming += diff; // dt = 10 mS
        }
        // integral
        pPID->diff_I = pPID->diff_suming * pPID->I / 1000;
    }
	else
    {
		pPID->diff_suming	= 0;
		pPID->diff_I		= 0;
    }
	// pid function
	val = (pPID->P * diff) / pPID->norm + pPID->diff_I + (pPID->D * (diff - pPID->diff_old));

	// regulate around the offset value
	val += pPID->offset;

	// check boundries
	if (val < pPID->val_min)	val = pPID->val_min;
	if (val > pPID->val_max)	val = pPID->val_max;

	// set values
	pPID->val = val;
	pPID->diff_old = diff;
}
