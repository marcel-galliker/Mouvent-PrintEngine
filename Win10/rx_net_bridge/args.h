// ****************************************************************************
//
//	DIGITAL PRINTING - args.h
//
//
// ****************************************************************************
//
//	Copyright 2017 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once

extern char arg_provider[MAX_PATH];
extern char arg_client  [MAX_PATH];

void args_init(int argc, char** argv);
