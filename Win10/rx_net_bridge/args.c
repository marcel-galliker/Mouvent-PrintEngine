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
#include "stdafx.h"
#include "args.h"

//--- external variables -------------------
// char arg_provider[MAX_PATH]="Surface Ethernet Adapter";
char arg_provider[MAX_PATH]="192.168.200.";
char arg_client  [MAX_PATH]="TeamViewer VPN Adapter";

//--- args_init -----------------------------------
void args_init(int argc, char** argv)
{
	int i;

	for (i=1; i<argc; i++)
	{
		if		(!strcmp(argv[i], "-provider"))		strcpy(arg_provider, argv[++i]);
		else if (!strcmp(argv[i], "-client"))		strcpy(arg_client,   argv[++i]);
		else printf("argument >>%s<< not known\n", argv[i]);	
	}
}
