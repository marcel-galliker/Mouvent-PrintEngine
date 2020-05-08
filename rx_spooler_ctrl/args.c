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
int arg_debug		= FALSE;
char arg_ipAddr[64] = "";
int arg_tracePQ = FALSE;	

//--- args_init -----------------------------------
void args_init(int argc, char** argv)
{
	int i;
	
	strcpy(arg_ipAddr, "");

	for (i=1; i<argc; i++)
	{
		if		(!strcmp(argv[i], "-debug"))		arg_debug=TRUE;
		if		(!strcmp(argv[i], "-ipaddr"))		strcpy(arg_ipAddr, argv[++i]);
		else printf("argument >>%s<< not known\n", argv[i]);	
	}
}