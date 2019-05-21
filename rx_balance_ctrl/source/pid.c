/******************************************************************************/
/** \file inktank_controller.c
 ** 
 ** control loop function 
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
	pPID->diff_sum = 0;
	pPID->diff_old = INVALID_VALUE;
}

/**
 ******************************************************************************
 ** \brief update PID
 **
 ** \param [in] x    istwert   
 ** \param [in] w    sollwert   
 ** \param [in] PID  Pointer to PID instance    
 ** 
 ******************************************************************************/
void pid_calc (uint32_t actVal, uint32_t setVal, SPID_par * pPID)
{
	INT32 diff;
	INT32 val;
	
	if (actVal==INVALID_VALUE)
	{
		pPID->diff_old = INVALID_VALUE;
		pPID->val	   = pPID->val_invalid;
		return;
	}
	
	diff = setVal - actVal;                              				
	
	if (diff>=pPID->diff_min || diff<=-pPID->diff_min)			
	{
		if (pPID->diff_old == INVALID_VALUE)
		{
			pPID->diff_old = diff;
			pPID->val	   = pPID->val_invalid;
		}
		else
		{
			// Reglergleichung
			val = (pPID->P * diff)+(pPID->I * pPID->diff_sum) + (pPID->D * (diff - pPID->diff_old));
			pPID->diff_old = diff;                             					
			pPID->diff_sum += diff;
			
			// stellwert begrenzen
			if      (val > pPID->val_max) pPID->val = pPID->val_max;
			else if (val < pPID->val_min) pPID->val = pPID->val_min;
			else 						  pPID->val = val;
		}
		pPID->val = pPID->val;
	}
}
