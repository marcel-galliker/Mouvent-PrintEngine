// ****************************************************************************
//
//	DIGITAL PRINTING - enc_test.h
//
//
// ****************************************************************************
//
//	Copyright 2016 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once


void test_init(void);

void test_cfg_done(void);

void test_do(int ticks);
void test_speed(int ticks);
void test_step_time(int ticks);
void test_write_csv(char *filename);


