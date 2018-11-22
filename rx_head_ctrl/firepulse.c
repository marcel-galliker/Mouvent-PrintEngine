// ****************************************************************************
//
//	DIGITAL PRINTING - firepulse.c
//
// ****************************************************************************
//
//	Copyright 2017 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

//--- includes ----------------------------------------------------------------
#ifdef linux
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <sys/mman.h>
	#include <unistd.h>
#endif
#include "rx_error.h"
#include "firepulse.h"

#define DAC_MEM_SIZE	0x10000

#define TEST

//--- DAC Registers ----------------------
#define FP1						15
#define FP2						16
#define FP3						17
#define FPA					 	5

#define AD9102_SPICONFIG		0x00
#define AD9102_POWERCONFIG		0x01
#define AD9102_CLOCKCONFIG		0x02
#define AD9102_REFADJ			0x03
#define AD9102_DACGAIN			0x07
#define AD9102_DACRANGE			0x08
#define AD9102_DACRSET			0x0C
#define AD9102_CALCONFIG		0x0D
#define AD9102_COMPOFFSET		0x0E
#define AD9102_RAMUPDATE		0x1D
#define AD9102_PAT_STATUS 		0x1E
#define AD9102_PAT_TYPE			0x1F
#define AD9102_PATTERN_DLY		0x20
#define AD9102_DACDOF			0x25
#define AD9102_WAV_CONFIG		0x27
#define AD9102_PAT_TIMEBASE		0x28
#define AD9102_PAT_PERIOD		0x29
#define AD9102_DAC_PAT			0x2B
#define AD9102_DOUT_START		0x2C
#define AD9102_DOUT_CONFIG		0x2D
#define AD9102_DAC_CST			0x31
#define AD9102_DAC_DGAIN		0x35
#define AD9102_SAW_CONFIG		0x37
#define AD9102_DDS_TW32			0x3E
#define AD9102_DDS_TW1			0x3F
#define AD9102_DDS_PW			0x43
#define AD9102_TRIG_TW_SEL		0x44
#define AD9102_DDS_CONFIG		0x45
#define AD9102_TW_RAM_CONFIG	0x47
#define AD9102_START_DLY		0x5C
#define AD9102_START_ADDR		0x5D
#define AD9102_STOP_ADDR		0x5E
#define AD9102_DDS_CYC			0x5F
#define AD9102_CFG_ERROR		0x60
#define AD9102_SRAM_DATA  		0x6000


//--- globals -----------------------------------------------------------------
static UINT16		*_DAC[HEAD_CNT];
static UINT32		*_DacReset = NULL;
static BYTE*		_AxiLite;

//--- prototypes -------------------------------------------
static int _dac_init(int no);

//--- fp_init ---------------------------------------
int fp_init(int memId)
{	
	//--- reset DAC ----------------------------------------------
	_AxiLite    = (BYTE*)mmap(NULL, 0x1000, PROT_READ|PROT_WRITE, MAP_SHARED, memId, 0xff210000);
	_DacReset  = (UINT32*)&_AxiLite[0x00a0];
	
	//--- init addresses -------------------------------------
	_DAC[0]   =  (UINT16*)mmap(NULL, DAC_MEM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, memId, 0xc0070000);
	_DAC[1]   =  (UINT16*)mmap(NULL, DAC_MEM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, memId, 0xc0050000);
	_DAC[2]   =  (UINT16*)mmap(NULL, DAC_MEM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, memId, 0xc0040000);
	_DAC[3]   =  (UINT16*)mmap(NULL, DAC_MEM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, memId, 0xc0030000);

	return REPLY_OK;
}

//--- fp_end -------------------------------------------
int fp_end(void)
{
	int i;
	for (i=0; i<HEAD_CNT; i++) 	if (munmap(_DAC[i],	DAC_MEM_SIZE))	Error(ERR_CONT, 0, "Could not UNMAP Waveform DAC[%d]-Memory", i);
	if (munmap(_AxiLite,		0x1000))	Error(ERR_CONT, 0, "Could not UNMAP Axi-Lite-Memory");

	return REPLY_OK;	
}

//--- _dac_init --------------------------------------------
static int _dac_init(int no)
{
	*_DacReset = 0x0f;
	
	_DAC[no][AD9102_PAT_STATUS] = 0x00;
	_DAC[no][AD9102_SPICONFIG]	= 0x2004;	// Reset all
	_DAC[no][AD9102_SPICONFIG]	= 0x0000;
	_DAC[no][AD9102_WAV_CONFIG]	= 0x00;		//	Wave Select Register
	_DAC[no][AD9102_DAC_CST]	= 0 << 4;	//
	_DAC[no][AD9102_DACDOF]		= 0x0000;	//	digital offset [4..15]
	_DAC[no][AD9102_DAC_DGAIN]	= 0x3ff0;	//
	_DAC[no][AD9102_START_DLY]	= 0x0;		//	Pattern Start Delay
	_DAC[no][AD9102_PAT_TYPE]	= 0x01;		//	pattern repeats the number of times 0x2A and 0x2B
	_DAC[no][AD9102_DAC_PAT]	= 0x01;		//	The Number Of Pattern repeat cycles
	_DAC[no][AD9102_START_ADDR] = 0x0000;	//	Pattern Start Address [15:4]
	_DAC[no][AD9102_PAT_PERIOD]	= 0x0000;	//	Pattern Periode
	_DAC[no][AD9102_RAMUPDATE]	= 0x01;		//	Update all SPI settings with a new configuration (self-clearing)

	if(_DAC[no][AD9102_DAC_DGAIN]==0x3ff0) return REPLY_OK;
	return REPLY_ERROR;
}

//--- fp_set_waveform ------------------------------------------
int fp_set_waveform(int no, int voltageCnt, UINT16 voltage[VOLTAGE_SIZE])
{
	int i;

	if (no<0 || no>=HEAD_CNT) return REPLY_ERROR;
	
	_dac_init(no);
	
	_DAC[no][AD9102_PAT_STATUS] =0x04;

	for(i=0; i<VOLTAGE_SIZE ; i++)
	{
		_DAC[no][AD9102_SRAM_DATA+i] = voltage[i] << 4;
	}

	_DAC[no][AD9102_STOP_ADDR]  = voltageCnt << 4;	// pattern stop address  [15:4]	todo
	_DAC[no][AD9102_PAT_PERIOD] = voltageCnt;		//	Pattern Periode
	_DAC[no][AD9102_RAMUPDATE]	= 0x01;				// Update all SPI settings with a new configuration (self-clearing)
	_DAC[no][AD9102_PAT_STATUS]	= 0x01;				// Allows the pattern generation, and stop pattern after trigger

	return REPLY_OK;	
}
