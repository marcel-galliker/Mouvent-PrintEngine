// ********************************************************void ********************
//
//	DIGITAL PRINTING - fpga_def_fluid.h
//
//	Communication between ARM processor and FPGA
//
// ****************************************************************************
//
//	Copyright 2016 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once

#ifdef NIOS
	#include "rx_ink_common.h"
#else
	#include "rx_common.h"
	#include "rx_def.h"
#endif

//--- defines ---------------------------------------------
#define SYSTEM_CLOCK	50000000 // 50 MHz
#define CYCLE_TIME		20		 // ns

//--- adddress mapping -----------------------------------------
#define ADDR_FPGA_QSYS		0xff200000
// #define ADDR_FPGA_BASE		0xc0020000				// !!!!! RESERVED FOR NIOS ACCESS ONLY !!!!!
#define ADDR_FPGA_BASE		0xff210000				// !!!!! RESERVED FOR NIOS ACCESS ONLY !!!!!

#define ADDR_FPGA_STAT		(ADDR_FPGA_BASE+0x0000)
#define ADDR_FPGA_PAR		(ADDR_FPGA_BASE+0x1000)

#define ADC_REG_0		    0x0010
#define ADC_REG_1		    0x0014
#define ADC_REG_2		    0x0018
#define ADC_REG_3		    0x001c
#define ADC_REG_4		    0x0020
#define ADC_REG_5		    0x0024
#define ADC_REG_6		    0x0028
#define ADC_REG_7		    0x002c
#define GPIO_REG_IN		    0x0030
#define BOARD_REG_TEMP		0x0034
#define ADC_FREQ_0		    0x0038
#define ADC_FREQ_1		    0x003c
#define ADC_FREQ_2		    0x0040
#define ADC_FREQ_3		    0x0044
#define ADC_FREQ_4		    0x0048
#define ADC_FREQ_5		    0x004c
#define ADC_FREQ_6		    0x0050
#define ADC_FREQ_7		    0x0054
#define WATCHDOG_ERR		0x0058

#define ENABLE_RDWR			0x1000
#define DAC_REG_0		    0x1004
#define DAC_REG_1		    0x1008
#define DAC_REG_2	        0x100C
#define DAC_REG_3		    0x1010
#define GPIO_REG_OUT	    0x1014
#define ADC_THRESH_0		0x1018
#define ADC_THRESH_1		0x101c
#define ADC_THRESH_2		0x1020
#define ADC_THRESH_3		0x1024
#define ADC_THRESH_4		0x1028
#define ADC_THRESH_5		0x102c
#define ADC_THRESH_6		0x1030
#define ADC_THRESH_7		0x1034
#define RESET_CNT   		0X1038
#define WATCHDOG_FREQ		0X103c
#define WATCHDOG_CNT		0x1040

typedef struct
{
	// ADDR_FPGA_QSYS	0xff20	0000				
	UINT32	jtag_uart;		//		0000:
	UINT32	res_04;
	UINT32	qsys_id;		//		0008:
	UINT32	qsys_timestamp;

	//--- reserved ------------------
	UINT32	res_130_200[(0x20000-0x10)/4];	// FF20000c - FF220000

	UINT32	res_200;	//	0xff22	0000:
	UINT32	res_204;
	UINT32	res_208;
	UINT32	res_20c;

	UINT32	pio_reset_nios;	//	0xff22	0010:
	UINT32	res_214;
	UINT32	res_218;
	UINT32	res_21c;

} SFpgaQSys;

//--- SFluidFpgaStatus ----------------------------------------------
typedef struct
{
	SVersion version;
	//	UINT32	major;		// FF21 0000: [32 Bit] 
	//	UINT32	minor;		//      0004: [32 Bit] 
	//	UINT32	revision;	//      0008: [32 Bit]
	//	UINT32	build;		//      000c: [32 Bit]
	UINT32	adc_reg_0;		//      0010: [16 Bit] -- from AMC7891 only 10bit used
	UINT32	adc_reg_1;		//      0014: [16 Bit]
	UINT32	adc_reg_2;		//      0018: [16 Bit] 
	UINT32	adc_reg_3;		//      001c: [16 Bit]
	UINT32	adc_reg_4;		//      0020: [16 Bit]
	UINT32	adc_reg_5;		//      0024: [16 Bit]
	UINT32	adc_reg_6;		//      0028: [16 Bit]
	UINT32	adc_reg_7;		//      002c: [16 Bit]
	UINT32	gpio_reg_in;	//      0030: [ 4 Bit] SolenoidStatus
	UINT32	board_reg_temp;	//      0034: [16 Bit] 
	UINT32	adc_freq_0;		//      0038: [16 Bit] -- from AMC7891 only 10bit used
	UINT32	adc_freq_1;		//      003c: [16 Bit]
	UINT32	adc_freq_2;		//      0040: [16 Bit] 
	UINT32	adc_freq_3;		//      0044: [16 Bit]
	UINT32	adc_freq_4;		//      0048: [16 Bit]
	UINT32	adc_freq_5;		//      004c: [16 Bit]
	UINT32	adc_freq_6;		//      0050: [16 Bit]
	UINT32	adc_freq_7;		//      0054: [16 Bit]
	UINT32	watchdog_err;	//      0058: [16 Bit]
	
} SFluidFpgaStatus;

//--- SFluidFpgaConfig ---------------------------------------------
typedef struct
{	
					// FF21
	UINT32	enable_rdwr;		//	1000: [ 1 Bit] enable AMC7891 Register read/write for FPGA -> after Nios configured AMC7891
	UINT32	dac_reg_0;		    //      1004: [16 Bit] PumpVctrl
	UINT32	dac_reg_1;		    //      1008: [16 Bit] PumpVctrl
	UINT32	dac_reg_2;	        //      100C: [16 Bit] PumpVctrl
	UINT32	dac_reg_3;		    //      1010: [16 Bit] PumpVctrl
	UINT32	gpio_reg_out;	    //      1014: [ 8 Bit] Solenoid
	UINT32	adc_thresh_0;		//      1018: [16 Bit] -- from AMC7891 only 10bit used
	UINT32	adc_thresh_1;		//      101c: [16 Bit]
	UINT32	adc_thresh_2;		//      1020: [16 Bit]
	UINT32	adc_thresh_3;		//      1024: [16 Bit]
	UINT32	adc_thresh_4;		//      1028: [16 Bit]
	UINT32	adc_thresh_5;		//      102c: [16 Bit]
	UINT32	adc_thresh_6;		//      1030: [16 Bit]
	UINT32	adc_thresh_7;		//      1034: [16 Bit]
	UINT32	reset_cnt;			//	    1038: [ 1 Bit] reset adc_freq counter, write "1" to trigger reset	
	UINT32	watchdog_freq;		//      103C: [32 Bit]
	UINT32	watchdog_cnt;		//      1040: [32 Bit]
	UINT32	reset_err;			//	    1044: [ 1 Bit] reset watchdog_err counter, write "1" to trigger reset	
	
} SFluidFpgaConfig;

//--- SFluidFpga ---------------------------------------
typedef struct
{
	SFluidFpgaStatus	stat;
//	SFluidFpgaConfig	cfg;
} SFluidFpga;
