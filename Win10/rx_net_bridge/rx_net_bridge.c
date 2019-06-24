// ****************************************************************************
//
//	DIGITAL PRINTING - rx_net_bridge
//
// ****************************************************************************
//
//	Copyright 2017 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#include "stdafx.h"

#include "args.h"
#include "bridge.h"	


///--- main ---------------------------------------------------------------
int main(int argc, char** argv)
{
	args_init(argc, argv);

	if (*arg_provider && *arg_client)
	{
		bridge_init(arg_provider, arg_client);
		while(TRUE)
		{
			Sleep(1000);
		}
	}
	return 0;
}

