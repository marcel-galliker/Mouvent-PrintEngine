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
#include "tcp_ip.h"


//--- external variables -------------------
int arg_debug			= FALSE;
int arg_simuPLC			= FALSE;
int arg_simuEncoder		= FALSE;
int arg_simuHeads		= FALSE;
int arg_simuChiller		= FALSE;
int arg_hamster			= FALSE;

//--- args_init -----------------------------------
void args_init(int argc, char** argv)
{
	// set the default value of SUBNET (-localsubnet can overwrite it)
	strcpy(RX_CTRL_SUBNET, DRX_CTRL_SUBNET);
	for (int i = 1; i < argc; i++)
	{
		if		(!strcmp(argv[i],  "-debug"))	arg_debug=TRUE;
		else if	(!stricmp(argv[i], "-simuplc"))	arg_simuPLC=TRUE;
		else if	(!stricmp(argv[i], "-simuencoder"))	arg_simuEncoder=TRUE;
		else if	(!stricmp(argv[i], "-simuheads"))	arg_simuHeads=TRUE;
		else if	(!stricmp(argv[i], "-simuchiller"))	arg_simuChiller=TRUE;
		else if	(!stricmp(argv[i], "-hamster"))		arg_hamster=arg_simuPLC=TRUE;
		else if	(!stricmp(argv[i], "-localsubnet"))	strcpy(RX_CTRL_SUBNET, "127.168.200.");
		else printf("argument >>%s<< not known\n", argv[i]);	
	}
	// update BROADCAST ip regarding the real subnet
	strcpy(RX_CTRL_BROADCAST, RX_CTRL_SUBNET);
	strcat(RX_CTRL_BROADCAST, "255");
	printf("subnet>>%s<< %s\n", RX_CTRL_SUBNET, RX_CTRL_BROADCAST);
}