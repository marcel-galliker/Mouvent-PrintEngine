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
#include "rx_term.h"
#include "args.h"


//--- external variables -------------------
int arg_debug		= FALSE;
int arg_test		= FALSE;
int arg_fhnw		= FALSE;
int arg_cleaf		= FALSE;
int arg_testslide	= FALSE;
int arg_simu_uv		= FALSE;

//--- args_init -----------------------------------
void args_init(int argc, char** argv)
{
	int i;

	for (i=1; i<argc; i++)
	{
		if		(!strcmp(argv[i], "-debug"))		arg_debug=TRUE;
		else if (!strcmp(argv[i], "-test"))		{   arg_debug=TRUE; arg_test=TRUE; }
		else if (!strcmp(argv[i], "-fhnw"))			arg_fhnw=TRUE;
		else if (!strcmp(argv[i], "-cleaf"))		arg_cleaf=TRUE;	// cleaf WEB machine
		else if (!strcmp(argv[i], "-testslide")){	arg_debug=TRUE; arg_testslide=TRUE; }	// test slide Sarmenstorf
		else if (!strcmp(argv[i], "-simu_uv"))		arg_simu_uv=TRUE;
		else term_printf("argument >>%s<< not known\n", argv[i]);	
	}
}