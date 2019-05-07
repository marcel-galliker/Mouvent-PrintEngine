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
#include "rx_def.h"
#include "args.h"


//--- external variables -------------------
int arg_debug			 = FALSE;
int arg_offline			 = FALSE;
int arg_simu_machine	 = FALSE;
int arg_test_fire		 = FALSE;
int arg_fhnw			 = FALSE;
int arg_simu_conditioner = FALSE;

//--- args_init -----------------------------------
void args_init(int argc, char** argv)
{
	int i;

	for (i=1; i<argc; i++)
	{
		if		(!strcmp(argv[i], "-debug"))		arg_debug=TRUE;
		else if (!strcmp(argv[i], "-offline"))		arg_offline=TRUE;
		else if (!strcmp(argv[i], "-simumachine"))	arg_simu_machine=TRUE;
		else if (!strcmp(argv[i], "-testfire"))		arg_test_fire=TRUE;
		else if (!strcmp(argv[i], "-fhnw"))			arg_fhnw=TRUE;
		else if (!strcmp(argv[i], "-simucond"))		arg_simu_conditioner=TRUE;
		else printf("argument >>%s<< not known\n", argv[i]);	
	}
}