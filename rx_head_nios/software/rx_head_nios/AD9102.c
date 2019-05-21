#include <stdio.h>
#include "alt_types.h"
#include "sys/alt_irq.h"
#include "altera_avalon_timer.h"
#include "altera_avalon_timer_regs.h"
#include "altera_avalon_pio_regs.h"
#include <unistd.h>
#include "sys/alt_stdio.h"
#include "system.h"
#include "io.h"
#include "sys/alt_alarm.h"
#include "AD9102.h"
#include "cond_def_head.h"
#include "nios_def_head.h"

extern SNiosMemory *arm_ptr;

int init_DAC(alt_u32 base)
{
	int ret;

	IOWR_16DIRECT(base,AD9102_PAT_STATUS,0x00);
	IOWR_16DIRECT(base,AD9102_SPICONFIG,0x2004);				// Reset all
	IOWR_16DIRECT(base,AD9102_SPICONFIG,0x0000);
	IOWR_16DIRECT(base,AD9102_WAV_CONFIG,0x00);			    	//	Wave Select Register
	IOWR_16DIRECT(base,AD9102_DAC_CST,0 << 4);		    		//
	IOWR_16DIRECT(base,AD9102_DACDOF,0x0000);					//	digital offset [4..15]
	IOWR_16DIRECT(base,AD9102_DAC_DGAIN,0x3ff0);				//
	IOWR_16DIRECT(base,AD9102_START_DLY,0x0);					//	Pattern Start Delay
	IOWR_16DIRECT(base,AD9102_PAT_TYPE,0x01);					//	pattern repaeats the number of times 0x2A and 0x2B
	IOWR_16DIRECT(base,AD9102_DAC_PAT,0x01);					//	The Number Of Pattern repeat cycles
	IOWR_16DIRECT(base,AD9102_START_ADDR,0x0000);				//	Pattern Start Address [15:4]
	IOWR_16DIRECT(base,AD9102_PAT_PERIOD,0x0000);				//	Pattern Periode
	IOWR_16DIRECT(base,AD9102_RAMUPDATE,0x01);					//	Update all SPI settings with a new configuration (self-clearing)

	ret=IORD_16DIRECT(base, AD9102_DAC_DGAIN);				// Read back DAC DGAIN
	if(ret!=0x3ff0) return -1;
	return 0;
}

int fill_wave_ram(alt_u32 base,alt_u8 fpNr)
{
	int ret = -1;
	int ix;

	// SRAM write access enabled

	IOWR_16DIRECT(base, AD9102_PAT_STATUS,0x04);

	// Copy DP Ram to wave Ram
	for(ix = 0 ; ix < arm_ptr->cfg.waveForm[fpNr].voltageCnt ; ix++)
	{
		IOWR_16DIRECT(base, AD9102_SRAM_DATA+ix+ix, arm_ptr->cfg.waveForm[fpNr].voltage[ix] << 4);
	}

	for(; ix < 4096 ; ix++)
	{
		IOWR_16DIRECT(base, AD9102_SRAM_DATA+ix+ix, 0);
	}

	IOWR_16DIRECT(base, AD9102_STOP_ADDR,  arm_ptr->cfg.waveForm[fpNr].voltageCnt << 4);	// pattern stop address  [15:4]	todo
	IOWR_16DIRECT(base, AD9102_PAT_PERIOD, arm_ptr->cfg.waveForm[fpNr].voltageCnt);			//	Pattern Periode
	IOWR_16DIRECT(base, AD9102_RAMUPDATE,  0x01);												// Update all SPI settings with a new configuration (self-clearing)
	IOWR_16DIRECT(base, AD9102_PAT_STATUS, 0x01);												// Allows the pattern generation, and stop pattern after trigger

	return ret;
}
