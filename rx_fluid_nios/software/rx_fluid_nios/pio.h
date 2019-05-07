/******************************************************************************/
/** \file pid.h
 ** 
 ** control loop function 
 **
 **	Copyright 2016 by radex AG, Switzerland. All rights reserved.
 **	Written by Marcel Galliker
 **
 ******************************************************************************/
 
#ifndef __PIO_H__
#define __PIO_H__

//--- PIO ------------------------------------------

// Outputs
#define WATCHDOG_OUT			0x01
#define SENSOR_POWER_ENABLE		0x02
#define PRESSURE_VALVE_OUT		0x04
#define DEGASS_VALVE_OUT		0x08
#define FLUSH_PUMP_OUT			0x10
#define AIR_PUMP_OUT			0x20
#define OUTPUT1_OUT				0x40
#define OUTPUT2_OUT				0x80
#define OUTPUT3_OUT				0x100
#define OUTPUT4_OUT				0x200
#define OUTPUT5_OUT				0x400
#define OUTPUT6_OUT				0x800
#define OUTPUT7_OUT				0x1000
#define CHILLER_OUT				0x2000
#define FPGA_LED_1_OUT			0x4000
#define FPGA_LED_2_OUT			0x8000

// Inputs
#define CHILLER_IN				0x01
#define INPUT2_IN				0x02
#define INPUT3_IN				0x04
#define INPUT4_IN				0x08

// valves
/*
#define	BleedIS1				0x001
#define	AirCushionIS1			0x002
#define	BleedIS2				0x004
#define	AirCushionIS2			0x008
#define	BleedIS3				0x010
#define	AirCushionIS3			0x020
#define	BleedIS4				0x040
#define	AirCushionIS4			0x080
*/
#define BLEED_OUT(isNo)			(0x0001<<(2*(isNo)))
#define	AIR_CUSSION_OUT(isNo)	(0x0002<<(2*(isNo)))

#endif /* __PIO_H__ */
