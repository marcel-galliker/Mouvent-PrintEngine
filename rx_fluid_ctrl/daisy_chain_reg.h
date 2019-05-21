// ****************************************************************************
//
//	DIGITAL PRINTING - daisy_chain_reg.h
//
//	Communication between Fluid and UART connected boards
//
// ****************************************************************************
//
//	Copyright 2018 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Patrick Walther
//
// ****************************************************************************
#pragma once

//--- includes ----------------------------------------------------------------
#include "rx_def.h"

#pragma pack(1) // exact fit - no padding

typedef union SAdcRegId
{
	struct
	{
		uint8_t dev_id : 3;
		uint8_t reserved : 5;
	} config;
	uint8_t id;
} SAdcRegId;

typedef union SAdcRegStatus
{
	struct
	{
		uint8_t fl_ref_l0 : 1;
		uint8_t fl_ref_l1 : 1;
		uint8_t fl_n_railn : 1;
		uint8_t fl_n_railp : 1;
		uint8_t fl_p_railn : 1;
		uint8_t fl_p_railp : 1;
		uint8_t rdy : 1;
		uint8_t fl_por : 1;
	} config;
	uint8_t status;
} SAdcRegStatus;

typedef union SAdcRegInpmux
{
	struct
	{
		uint8_t muxn : 4;
		uint8_t muxp : 4;
	} config;
	uint8_t inpmux;
} SAdcRegInpmux;

typedef union SAdcRegPga
{
	struct
	{
		uint8_t gain : 3;
		uint8_t pga_en : 2;
		uint8_t delay : 3;
	} config;
	uint8_t pga;
} SAdcRegPga;

typedef union SAdcRegDatarate
{
	struct
	{
		uint8_t dr : 4;
		uint8_t filter : 1;
		uint8_t mode : 1;
		uint8_t clk : 1;
		uint8_t g_chop : 1;
	} config;
	uint8_t datarate;
} SAdcRegDatarate;

typedef union SAdcRegRef
{
	struct
	{
		uint8_t refcon : 2;
		uint8_t refsel : 2;
		uint8_t refn_buf : 1;
		uint8_t refp_buf : 1;
		uint8_t fl_ref_en : 2;
	} config;
	uint8_t ref;
} SAdcRegRef;

typedef union SAdcRegIdacmag
{
	struct
	{
		uint8_t imag : 4;
		uint8_t unused : 2;
		uint8_t psw : 1;
		uint8_t fl_rail_en : 1;
	} config;
	uint8_t idacmag;
} SAdcRegIdacmag;

typedef union SAdcRegIdacmux
{
	struct
	{
		uint8_t l1mux : 4;
		uint8_t l2mux : 4;
	} config;
	uint8_t idacmux;
} SAdcRegIdacmux;

typedef union SAdcRegVbias
{
	struct
	{
		uint8_t vb_ain0 : 1;
		uint8_t vb_ain1 : 1;
		uint8_t vb_ain2 : 1;
		uint8_t vb_ain3 : 1;
		uint8_t vb_ain4 : 1;
		uint8_t vb_ain5 : 1;
		uint8_t vb_ainc : 1;
		uint8_t vb_level : 1;
	} config;
	uint8_t vbias;
} SAdcRegVbias;

typedef union SAdcRegSys
{
	struct
	{
		uint8_t sendstat : 1;
		uint8_t crc : 1;
		uint8_t timeout : 1;
		uint8_t cal_samp : 2;
		uint8_t sys_mon : 3;
	} config;
	uint8_t sys;
} SAdcRegSys;

typedef union SAdcRegOfcal
{
	uint8_t ofc;
} SAdcRegOfcal;

typedef union SAdcRegFsc
{
	uint8_t fsc;
} SAdcRegFsc;

typedef union SAdcRegGpiodat
{
	struct
	{
		uint8_t dat : 4;
		uint8_t dir : 4;
	} config;
	uint8_t gpiodat;
} SAdcRegGpiodat;

typedef union SAdcRegGpiocon
{
	struct
	{
		uint8_t con : 4;
		uint8_t unused : 4;
	} config;
	uint8_t gpiocon;
} SAdcRegGpiocon;

typedef struct SAdcConfigRegs
{
	SAdcRegId 			id_reg;
	SAdcRegStatus 		status_reg;
	SAdcRegInpmux 		inpmux_reg;
	SAdcRegPga 			pga_reg;
	SAdcRegDatarate 	datarate_reg;
	SAdcRegRef			ref_reg;
	SAdcRegIdacmag		idacmag_reg;
	SAdcRegIdacmux		idacmux_reg;
	SAdcRegVbias		vbias_reg;
	SAdcRegSys			sys_reg;
	SAdcRegOfcal		ofcal0_reg;
	SAdcRegOfcal		ofcal1_reg;
	SAdcRegOfcal		ofcal2_reg;
	SAdcRegFsc			fscal0_reg;
	SAdcRegFsc			fscal1_reg;
	SAdcRegFsc			fscal2_reg;
	SAdcRegGpiodat		gpiodat_reg;
	SAdcRegGpiocon		gpiocon_reg;
} SAdcConfigRegs;

#pragma pack() //back to whatever the previous packing mode was 