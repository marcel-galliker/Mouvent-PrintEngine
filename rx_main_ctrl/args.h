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
extern int arg_simuPLC;
extern int arg_simuEncoder;
extern int arg_simuHeads;
extern int arg_simuChiller;
extern int arg_hamster;

void args_init(int argc, char** argv);
