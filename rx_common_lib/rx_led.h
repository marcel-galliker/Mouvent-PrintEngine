// ****************************************************************************
//
//	rx_led.h
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once

//--- led names
//	fpga_led0 ... fpga_led3
//	hps_led0 ... hps_led3

void led_off(char *led);
void led_on(char *led);
void led_blink(char *led, int interval);
