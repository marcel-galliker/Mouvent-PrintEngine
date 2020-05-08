// ****************************************************************************
//
//	DIGITAL PRINTING - args.c
//
//	Communication between ARM processor and FPGA
//
// ****************************************************************************
//
//	Copyright 2013 by radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

//--- includes ----------------------------------------------------------------
#include "help.h"


//--- calc_sin ------------------------------
void calc_sin(INT32 *pdata, int cnt)
{
	int number_of_periods = 38;
	int	enc_steps_per_rev=cnt;
	int length = enc_steps_per_rev*number_of_periods;
	int	periode = enc_steps_per_rev;
			
}
