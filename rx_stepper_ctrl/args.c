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
int arg_debug	= FALSE;
int arg_simu	= FALSE;

//--- args_init -----------------------------------
void args_init(int argc, char** argv)
{
	int i;

	for (i=1; i<argc; i++)
	{
		if		(!strcmp(argv[i], "-debug"))	arg_debug=TRUE;
		else if	(!strcmp(argv[i], "-simu"))		arg_simu=TRUE;
		else if	(!strcmp(argv[i], "-tt"))		{ RX_StepperCfg.boardNo=0; RX_StepperCfg.printerType=printer_test_table; }
		else if	(!strcmp(argv[i], "-tx801"))	{ RX_StepperCfg.boardNo=0; RX_StepperCfg.printerType=printer_TX801;}
		else if	(!strcmp(argv[i], "-txro"))		{ RX_StepperCfg.boardNo=1; RX_StepperCfg.printerType=printer_TX801;}
		else if	(!strcmp(argv[i], "-cleaf"))	{ RX_StepperCfg.boardNo=0; RX_StepperCfg.printerType=printer_cleaf;}
		else if	(!strcmp(argv[i], "-cln"))		{ RX_StepperCfg.boardNo=1; RX_StepperCfg.printerType=printer_cleaf;}
		else if	(!strcmp(argv[i], "-lb701"))	{ RX_StepperCfg.boardNo=0; RX_StepperCfg.printerType=printer_LB701;}
		else if	(!strcmp(argv[i], "-lb701_ro"))	{ RX_StepperCfg.boardNo=1; RX_StepperCfg.printerType=printer_LB701;}
		else if	(!strcmp(argv[i], "-web"))		{ RX_StepperCfg.boardNo=1; RX_StepperCfg.printerType=printer_LB701;}
		else if	(!strcmp(argv[i], "-lb702"))	{ RX_StepperCfg.boardNo=0; RX_StepperCfg.printerType=printer_LB702_UV;}
		else if	(!strcmp(argv[i], "-lb702_ro"))	{ RX_StepperCfg.boardNo=1; RX_StepperCfg.printerType=printer_LB702_UV;}
		else if	(!strcmp(argv[i], "-test"))		{ RX_StepperCfg.boardNo=0; RX_StepperCfg.printerType=printer_undef;}
		else printf("argument >>%s<< not known\n", argv[i]);	
	}
}