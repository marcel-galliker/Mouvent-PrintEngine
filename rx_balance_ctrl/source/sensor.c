/******************************************************************************/
/** \file sensor.c
 ** 
 ** read data from humidity and temperatur sensor
 **
 **	Copyright 2017 by radex AG, Switzerland. All rights reserved.
 **	Written by Stefan Weber
 **
 ******************************************************************************/

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "IOMux.h"
#include "i2c.h"
#include "sensor.h"
#include "crc_calc.h"
#include "balance_def.h"

/*****************************************************************************/
/* Defines                                                                   */
/*****************************************************************************/
#define ADDR_SENSOR 0x40

#define TIME_OUT_HUMIDITY 300

#define HUMID_MEASURE_START_NO_HOLD 0xF5
#define TEMP_MEASURE_START_NO_HOLD 0xF3

volatile uint32_t _sensor_error=0;
volatile uint32_t sensor_state=0;
uint16_t humidity;
uint16_t temperature;

//--- prototypes ------------------------
int _sensor_measure(int command);
int _sensor_read(int command);

//--- sensor_state_machine ------------------------
void sensor_state_machine(void)
{
	switch (sensor_state)
	{
		case 0:
			if(_sensor_measure(HUMID_MEASURE_START_NO_HOLD))
			{
				_sensor_error++;
			}
			else	
			{
				sensor_state=1;
			}
			break;
		case 1:
			if(_sensor_read(HUMID_MEASURE_START_NO_HOLD))
			{
				_sensor_error++;
				humidity=0;
				sensor_state=0;
			}
			else		
			{
				sensor_state=2;
			}
			break;
		case 2:
			if(_sensor_measure(TEMP_MEASURE_START_NO_HOLD))
			{
				_sensor_error++;
			}
			else		
			{
				sensor_state=3;
			}
			break;
		case 3:
			if(_sensor_read(TEMP_MEASURE_START_NO_HOLD))
			{
				_sensor_error++;
				temperature=0;
			}
			sensor_state=0;
			break;
		default:		sensor_state=0;
			break;
	}
}
//--- _sensor_measure --------------------------------------------------------
int _sensor_measure(int command)
{
	int		ret;
	ret=I2cStartWrite((stc_mfsn_t*)FM4_MFS3_BASE, ADDR_SENSOR);
	if (!ret)	ret = I2cSendByte((stc_mfsn_t*)FM4_MFS3_BASE, command);
	I2cStopWrite((stc_mfsn_t*)FM4_MFS3_BASE);
	if(!ret) return 0;
	return 1;
}

//--- _sensor_read ------------------------------------------------------------
int _sensor_read(int command)
{
	int		value=0;
	uint8_t	checksum=0;
	int 	timeout;
	int		ret;
	unsigned char	*pvalue = (unsigned char*)&value;	

	for(timeout=TIME_OUT_HUMIDITY; timeout; timeout--)	//if there is no reponse, break after timeout!
	{
		ret=I2cStartRead((stc_mfsn_t*)FM4_MFS3_BASE, ADDR_SENSOR);
		if (!ret) 	ret = I2cReceiveByteSeq((stc_mfsn_t*)FM4_MFS3_BASE, &pvalue[1], FALSE);
		if (!ret)		ret = I2cReceiveByteSeq((stc_mfsn_t*)FM4_MFS3_BASE, &pvalue[0], FALSE);
		if (!ret)		ret = I2cReceiveByteSeq((stc_mfsn_t*)FM4_MFS3_BASE, &checksum, TRUE);
		I2cStopRead((stc_mfsn_t*)FM4_MFS3_BASE);
		if(ret) return -1;
		break;
	}
	if(timeout==0) 
	{
		I2cStopRead((stc_mfsn_t*)FM4_MFS3_BASE);
		return -1;
	}
	
	ret= MakeCRC (pvalue, 2, checksum);
	if(ret) return -1;
	
	if(command==HUMID_MEASURE_START_NO_HOLD)
	{
		humidity=(value*125/65536)-6;
		RX_BalanceStat[0].humidity=value;
	}
	else
	{
		temperature=(value*175.72/65536)-46.85;
		RX_BalanceStat[0].temp=value;
	}
	return 0;
}
