// ****************************************************************************
//
//	DIGITAL PRINTING - args.h
//
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once


extern int arg_debug;
extern int arg_offline;
extern int arg_simu_machine;
extern int arg_test_fire;
extern int arg_fhnw;
extern int arg_simu_conditioner;

void args_init(int argc, char** argv);
