// ****************************************************************************
//
//	DIGITAL PRINTING - tw8.c
//
//	head-control-client
//
// ****************************************************************************
//
//	Copyright 2018 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

//--- includes ------------------------------------------------------

#include "rx_error.h"
#include "rx_term.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "comm.h"
#include "tw8_def.h"
#include "tw8_reg.h"
#include "tw8.h"


//--- defines -------------------------------------------------------
#define SERIAL_PORT "/dev/ttyS0"

#define CHIP_CNT	2
#define REG_BASE	0x8000
#define REG_CNT		0x4000

#define TRACE_CHIP_1	FALSE

//--- Errors ----------------------------------------
static UINT32 _ErrorFlags;

//---prototypes ----------------------------
static void* _tw8_thread(void *par);
static int	 _read(void);
static int   _write(int chip, BYTE cmdcode, UINT16 addr, BYTE len, void *data);
static void  _tw8_display_status(void);
static void _tw8_display_cfg(void);
static void _tw8_display_var(void);
static void _tw8_display_par(void);
static void _tw8_check_err(void);
static void _test(void);

//--- file gobals --------------------------
typedef struct 
{
	UINT16 addr; 
	UINT8 len;
} SADDR_LEN;

static FILE *_tw8_out;
static FILE *_tw8_in;
static int	_ShowStatus = FALSE;
static BYTE _Registers[CHIP_CNT][REG_CNT];
static UINT16	_test_mem[6];
static const SADDR_LEN _RegList[] = 
{
	{ 0x8000, 12 },
	{ ADDR_CFG, sizeof(STW8_CFG_Regs) },
	{ 0x809A, 4 },
	{ 0x80A4, 6 },
	{ 0x80AE, 6 },
	{ 0x806A, 14 },
	{ 0x8078, 14 },
		
	{ ADDR_POS, sizeof(STW8_WM_PosRegs) }
};
//			0x8002, 0x8004, 0x8008, 
//											0x8044, 0x8046, 0x8048, 0x804A, 0x804C, 0x804E, 
//							                0x8050, 0x8052, 0x8054, 0x8056, 0x8058, 0x805A, 0x805C, 0x805E,
//											0x8060, 0x8062, 0x8064, 0x8066, 
//																				    0x809A, 0x809C, // 0x809E,
//															0x80A4,	0x80A6, 0x80A8,					0x80AE, 
//											0x80B0};

STW8_CFG_Regs   *_CFG[CHIP_CNT];
STW8_WM_PosRegs *_PosSpeed[CHIP_CNT];
STW8_VAR_Regs	*_Var[CHIP_CNT];
STW8_PAR_Regs	*_Par_Now[CHIP_CNT];
STW8_PAR_Regs	*_Par_Bak[CHIP_CNT];

#define FIFO_SIZE	128
static  rx_tw8_ctrl_cmd _Fifo[FIFO_SIZE];
static	int				_FifoInIdx;
static  int				_FifoOutIdx;

static int				_Present=FALSE;

//--- tw8_init --------------------------------------
int tw8_init(void)
{
	int i;
	memset(_Registers, 0, sizeof(_Registers));
	comm_init(0);
	_Present = FALSE;

	system("stty -F " SERIAL_PORT " raw cs8 -ixon -cstopb -parenb -echo 115200");

	_tw8_out = fopen(SERIAL_PORT, "w");
	_tw8_in  = fopen(SERIAL_PORT, "r");

	memset(_Fifo, 0, sizeof(_Fifo));
	_FifoInIdx  = 0;
	_FifoOutIdx = 0;
	rx_thread_start(_tw8_thread, NULL, 0, "tw8_thread");
	
	for (i = 0; i < CHIP_CNT; i++)
	{
		_CFG[i]		 = (STW8_CFG_Regs*)   &_Registers[i][ADDR_CFG - REG_BASE];
		_Var[i]		 = (STW8_VAR_Regs*)	  &_Registers[i][ADDR_VAR - REG_BASE];
		_Par_Now[i]	 = (STW8_PAR_Regs*)	  &_Registers[i][ADDR_PAR_NOW - REG_BASE];
		_Par_Bak[i]  = (STW8_PAR_Regs*)	  &_Registers[i][ADDR_PAR_BAK - REG_BASE];
		_PosSpeed[i] = (STW8_WM_PosRegs*) &_Registers[i][ADDR_POS - REG_BASE];
	}
	
	_ErrorFlags = 0;

//	_test();
	
	rx_sleep(500); // Wait for Analog-Encoder-Board to be ready
	
	tw8_config(0, 60, printer_TX801);
	tw8_config(1, 60, printer_TX801);
	
	return REPLY_OK;
}

//--- test ------------------
/*
static void _test(void)
{
	UINT32	test32 = 0x12345678;
	UINT16	test16 = 0xabcd;
	UINT8	test8  = 0xef;
	
	char	mem8[12];
	
	memset(mem8, 0, sizeof(mem8));

	tw8_write(0, 0x8002, sizeof(test32), &test32);
	memcpy(&mem8[2], &test32, sizeof(test32));
					
	tw8_write(0, 0x8007, sizeof(test16), &test16);
	memcpy(&mem8[7], &test16, sizeof(test16));
	
	tw8_write(0, 0x800b, sizeof(test8), &test8);
	memcpy(&mem8[11], &test8, sizeof(test8));

	memcpy(_test_mem, mem8, sizeof(_test_mem));
};
*/

//--- tw8_end --------------------------------------
int tw8_end(void)
{
	return REPLY_OK;
}

//--- tw8_main --------------------------------------
int tw8_main(int ticks, int menu)
{
	if (menu) _tw8_display_status();
		
	return REPLY_OK;
}

//--- tw8_menu_print -------------------
int tw8_menu_print(void)
{
	term_printf("tw8 --------------------\n");
	if (!_ShowStatus) term_printf("t: Show TW8 Status\n");
	else			
	{
		term_printf("t:	Hide TW8 Status\n");
		term_printf("r: reset errors\n");
		//term_printf("r<chip> <addr> <data>:	set register (addr and data in hex)\n");
	}
	return REPLY_OK;			
}

//--- tw8_menu_cmd --------------------------------
int tw8_menu_cmd(char *str)
{
	
	switch (str[0])
	{
	case 't':	_ShowStatus = !_ShowStatus; break;
	case 'r':  tw8_reset_error(); break;
	}
	return REPLY_OK;			
}

//--- tw8_config ---------------------------------
int	tw8_config(int chip, int speed_mminn, EPrinterType printerType)
{
	STW8_CFG_Regs cfg;
	memset(&cfg, 0, sizeof(cfg));
	
	//cfg.MAIN_CFG.reg		= 0x01;
	cfg.MAIN_CFG.wp			= 0x01; // 1bit // Pin WP - EEPROM Write Lock
	cfg.MAIN_CFG.bit_1		= 0x0;
	cfg.MAIN_CFG.pull		= 0x0; // 1bit // Input Pull-Up/Pull-Down Resistors
	cfg.MAIN_CFG.clkout		= 0x0; // 1bit // Signal Output to Pin CLOCK and Pin FRAME
	
	//cfg.MAIN_DSM.reg		= 0x00;
	cfg.MAIN_DSM.freq		= 0x00; // 3bit // PWM Output Frequency
	cfg.MAIN_DSM.calmp		= 0x00; // 1bit // PWM Generator Clamping
	
	// cfg.MAIN_INTER
	if(rx_def_is_scanning(printerType))	cfg.MAIN_INTER	= 2000; // 16bit // AB Output Resolution in Edges
	else								cfg.MAIN_INTER	= 1000; // 16bit // AB Output Resolution in Edges
	
	// cfg.MAIN_HYST
	cfg.MAIN_HYST			= 0x0f; // 9bit // hyst = MAIN_HYSTx360/8192 // max 1FF // 0x00;  // Signal Path Output Hysteresis
	
	//cfg.MAIN_FLTR.reg		= 0x72;
	cfg.MAIN_FLTR.fb		= 0x0; // 1bit // Output Filter Feedback Path Delay
	cfg.MAIN_FLTR.p	= 0x3; // 0x0; // 3bit // Output Filder Proportional Gain
	cfg.MAIN_FLTR.i	= 0x7; // 3bit // Output Filter Integral Gain

	//cfg.MAIN_OUT	.reg	= 0x0A; 
	cfg.MAIN_OUT.start		= 0xA; // 4bit // Power-Up Wait Time for Analog Circuit Setting
	cfg.MAIN_OUT.mode		= 0x0; // 2bit // Operating Mode
	
	//cfg.MAIN_CLOCK.reg	= 0x87; 
	cfg.MAIN_CLOCK.xtal		= 0x1; // 1bit // Clock Source
	cfg.MAIN_CLOCK.div		= 0x3; // 2bit // Internal Oscillator
	cfg.MAIN_CLOCK.freq		= 0x0; // 4bit // Internal Oscillator Frequency Tuning
	cfg.MAIN_CLOCK.xforce	= 0x1; // 1bit // Clock Source on Low Power
	// cfg.MAIN_ZPOS
	cfg.MAIN_ZPOS			= 0x00; // 14bit //  0x00; //0x72; // Z Output Index Position
	
	//cfg.MAIN_Z.reg		= 0x00; 
	cfg.MAIN_Z.th			= 0x00; // 5bit //
	cfg.MAIN_Z.reset		= 0x0; // 1bit //
	
	//cfg.AB_CFG0.reg		= 0x21; 
	cfg.AB_CFG0.dir			= 0x1; // 1bit // AB Counting Direction
	cfg.AB_CFG0.apol		= 0x0; // 1bit // A Output Polarity / State at Z Position -> changes direction!
	cfg.AB_CFG0.bpol		= 0x0; // 1bit // B Output Polarity / State at Z Position -> changes direction!
	cfg.AB_CFG0.zpol		= 0x0; // 1bit // Z Output Polarity
	cfg.AB_CFG0.startmode	= 0x1; // 2bit // ABZ Output Startup Behavior
	
	//cfg.AB_CFG1.reg		= 0x00; 
	cfg.AB_CFG1.div			= 0x00; // 5bit // 0x00 // Post-AB Divider
	// cfg.AB_ZWIDTH
	cfg.AB_ZWIDTH			= 0x00;  // 8bit // 0x00 // Z Output PUlse Width
	// cfg.VTOP
	cfg.AB_VTOP				= 0x3; // 7bit // 0x03; // AB Output Frequency Limit
	
	// cfg.ADPT_CFG.reg		= 0x1F; // Auto Adaption
	cfg.ADPT_CFG.doff		= 0x1; // 1bit // Digital Offset
	cfg.ADPT_CFG.dgain		= 0x1; // 1bit // Digital Gain
	cfg.ADPT_CFG.dphase		= 0x1; // 1bit // Digital Phase
	cfg.ADPT_CFG.aoff		= 0x1; // 1bit // Analog Offset
	cfg.ADPT_CFG.again		= 0x1; // 1bit // Analog Gain
	cfg.ADPT_CFG.lut		= 0x0; // 1bit // Look-Up Table
	cfg.ADPT_CFG.store		= 0x0; // 1bit // Auto-Storage to EEPROM
	
	//cfg.ADPT_DETAIL.reg	= 0x26; 
	cfg.ADPT_DETAIL.flimit	= 0x06; // 3bit // Auto-Adaption Frequency Limit
	cfg.ADPT_DETAIL.tbase	= 0x04; // 3bit // Auto-Adaption Correction Time Base
	cfg.ADPT_DETAIL.fault	= 0x00; // 1bit // Auto-Adaption During ADC Fault
	
	//cfg.ADPT_STORE.reg	= 0x00; 
	cfg.ADPT_STORE.offth	= 0x00; // 4bit // Dig. Offs. Threshold for Auto-Storage to EEPROM
	cfg.ADPT_STORE.gainth	= 0x00; // 4bit //  Dig. Gain Threshold for Auto-Storage to EEPROM
	
	//cfg.ADPT_CORR.reg		= 0x4A; 
	cfg.ADPT_CORR.prop	= 0x0; // 0x02; // 2bit // 0x0 = linear // 0x1 - 0x3 = exponential correction from slow to fast // Auto-Adaption Control
	cfg.ADPT_CORR.offtol	= 0x02; // 3bit // Auto-Adaption Dig. Offset Tolerance
	cfg.ADPT_CORR.gaintol	= 0x02; // 3bit // Auto-Adaption Dig. Gain Tolerance
	
	//cfg.MON_CFG.reg		= 0x00; 
	cfg.MON_CFG.adc			= 0x0; // 1bit // ADC Quality Monitoring Enable
	cfg.MON_CFG.adapt		= 0x0; // 1bit // Adaption Quality Monitoring Enable 
	cfg.MON_CFG.pwm			= 0x0; // 2bit //  Status Output PWM Control
	cfg.MON_CFG.adcth		= 0x0; // 1bit //  ADC Quality Threshold MSB of MON_ADC
	
	//cfg.MON_ADC.reg		= 0x00;
	cfg.MON_ADC.limit		= 0x00; // 4bit // ADC Underflow Fault Limit
	cfg.MON_ADC.th			= 0x00; // 4bit // ADC Quality Threshold
	
	//cfg.MON_OFF.reg		= 0x0A; 
	cfg.MON_OFF.limit		= 0xA; // 4bit // Dig. Offset Adaption Limit
	cfg.MON_OFF.th			= 0x0; // 4bit // Dig. Offset Adaption Quality Threshold
	
	//cfg.MON_GAIN.reg		= 0x0A; 
	cfg.MON_GAIN.limit		= 0xA; // 4bit // Dig. Gain Match Adaption Limit
	cfg.MON_GAIN.th			= 0x0; // 4bit // Dig. Gain Match Adaption Quality Threshold
	
	//cfg.MON_PHASE.reg		= 0x0A; 
	cfg.MON_PHASE.limit		= 0xA; // 4bit // Dig. Phase Adaption Limit
	cfg.MON_PHASE.th		= 0x0; // 4bit // Dig. Phase Adaption Quality Threshold
	
	//cfg.FLT_CFG.reg		= 0x11; 
	cfg.FLT_CFG.vwarn		= 0x01;  // 1bit // Pin Fauld indication of Operational Warnings
	cfg.FLT_CFG.hold		= 0x00;  // 2bit // AB Output Hold
	cfg.FLT_CFG.pol			= 0x00;  // 1bit // Pin FAULT Polarity
	cfg.FLT_CFG.long_		= 0x01;  // 1bit // Pin FAULT indication Time
	
	//cfg.FLT_STAT.reg		= 0x0A; 
	cfg.FLT_STAT.ee			= 0x0; // 1bit //  EEPROM Fault
	cfg.FLT_STAT.adc		= 0x1; // 1bit // ADC Fault 
	cfg.FLT_STAT.adapt		= 0x0; // 1bit //  Adaption Fault
	cfg.FLT_STAT.vwarn		= 0x1; // 1bit //  Overspeed Warning
	cfg.FLT_STAT.vfatal		= 0x0; // 1bit //  Fatal Overspeed Fault
	cfg.FLT_STAT.intern		= 0x0; // 1bit //  Internal Fault
	
	//cfg.FLT_EN.reg		= 0x11; 
	cfg.FLT_EN.ee			= 0x01; // 1bit // Fault Latching Enable
	cfg.FLT_EN.adc			= 0x00; // 1bit // ADC Fault
	cfg.FLT_EN.adapt		= 0x00; // 1bit // Adaption Fault
	cfg.FLT_EN.vwarn		= 0x00; // 1bit // Overspeed Warning
	cfg.FLT_EN.vfatal		= 0x01; // 1bit // Fatal Overspeed Fault
	cfg.FLT_EN.intern		= 0x00; // 1bit // Internal Fault
	
	tw8_write(chip, ADDR_CFG, sizeof(STW8_CFG_Regs), &cfg);
	
	STW8_PAR_Regs par;
	memset(&par, 0, sizeof(par));
	par.AGAIN = 0x4;
	par.AOFFC = 0x0;
	par.AOFFS = 0x0;
	
	//tw8_write(chip, ADDR_PAR_NOW, sizeof(STW8_PAR_Regs), &par); // Set current Values
	
	UINT8	update_cmd  = 0x2;
	tw8_write(chip, 0x80B0, sizeof(update_cmd), &update_cmd);
}

//--- _tw8_display_status ---------------------------------------
static void _tw8_display_status(void)
{
	int cnt = CHIP_CNT;
	int reg, i;
	int n=0;
	int addr = _RegList[0].addr;
	INT16 val;
	static int test = 0;
	
	if (_ShowStatus)
	{
		term_printf("\n");
		term_printf("--- TW8 LUT ----------------------------------------\n");
		for (reg = 0; reg < _RegList[0].len; reg += 2, addr += 2)
		{
			term_printf("  0x%04x:        ", addr); 
			for (i = 0; i < cnt; i++) 
			{
				memcpy(&val, &_Registers[i][addr - REG_BASE], sizeof(val));
				term_printf("0x%04x  ", val);		
			}
			term_printf("test: 0x%04x", _test_mem[n++]);
			term_printf("\n");	
		}		
		_tw8_display_cfg();
		_tw8_display_var();
		_tw8_display_par();
		_tw8_check_err();
	}
	
	test++;
	tw8_write(0, 0x8000, sizeof(UINT8), &test);
	tw8_write(1, 0x8000, sizeof(UINT8), &test);
	if (test >= 0xFF)
		test = 0;
}

static void _print_addr(char *name, void *reg)
{
	int addr = (UINT8*)reg - &_Registers[0][0];
	
	term_printf("  0x%04x: %-12s", REG_BASE + addr, name); 					
}

//--- _tw8_display_cfg ---------------------------
static void _tw8_display_cfg(void)
{
	int chip = 0;
#define ALL for(chip=0; chip<CHIP_CNT; chip++)
	
	term_printf("--- TW8 - CONFIGURATION -----------------------------\n");
	_print_addr("MAIN_CFG",		&_CFG[0]->MAIN_CFG.reg);	ALL term_printf("0x%04X  ", _CFG[chip]->MAIN_CFG.reg);		term_printf("\n");
	_print_addr("MAIN_DSM",		&_CFG[0]->MAIN_DSM.reg);	ALL term_printf("0x%04X  ", _CFG[chip]->MAIN_DSM.reg);		term_printf("\n");
	_print_addr("MAIN_INTER",	&_CFG[0]->MAIN_INTER);		ALL term_printf("0x%04X  ", _CFG[chip]->MAIN_INTER);		term_printf("\n");
	_print_addr("MAIN_HYST",	&_CFG[0]->MAIN_HYST);		ALL term_printf("0x%04X  ", _CFG[chip]->MAIN_HYST);			term_printf("\n");
	_print_addr("MAIN_FLTR",	&_CFG[0]->MAIN_FLTR.reg);	ALL term_printf("0x%04X  ", _CFG[chip]->MAIN_FLTR.reg);		term_printf("\n");
	_print_addr("MAIN_OUT",		&_CFG[0]->MAIN_OUT.reg);	ALL term_printf("0x%04X  ", _CFG[chip]->MAIN_OUT.reg);		term_printf("\n");
	_print_addr("MAIN_CLOCK",	&_CFG[0]->MAIN_CLOCK.reg);	ALL term_printf("0x%04X  ", _CFG[chip]->MAIN_CLOCK.reg);	term_printf("\n");
	_print_addr("MAIN_ZPOS",	&_CFG[0]->MAIN_ZPOS);		ALL term_printf("0x%04X  ", _CFG[chip]->MAIN_ZPOS);			term_printf("\n");
	_print_addr("MAIN_Z",		&_CFG[0]->MAIN_Z.reg);		ALL term_printf("0x%04X  ", _CFG[chip]->MAIN_Z.reg);		term_printf("\n");
	_print_addr("AB_CFG0",		&_CFG[0]->AB_CFG0.reg);		ALL term_printf("0x%04X  ", _CFG[chip]->AB_CFG0.reg);		term_printf("\n");
	_print_addr("AB_CFG1",		&_CFG[0]->AB_CFG1.reg);		ALL term_printf("0x%04X  ", _CFG[chip]->AB_CFG1.reg);		term_printf("\n");
	_print_addr("AB_ZWIDTH",	&_CFG[0]->AB_ZWIDTH);		ALL term_printf("0x%04X  ", _CFG[chip]->AB_ZWIDTH);			term_printf("\n");
	_print_addr("AB_VTOP",		&_CFG[0]->AB_VTOP);			ALL term_printf("0x%04X  ", _CFG[chip]->AB_VTOP);			term_printf("\n");
	_print_addr("ADPT_CFG",		&_CFG[0]->ADPT_CFG.reg);	ALL term_printf("0x%04X  ", _CFG[chip]->ADPT_CFG.reg);		term_printf("\n");
	_print_addr("ADPT_DETAIL",	&_CFG[0]->ADPT_DETAIL.reg); ALL term_printf("0x%04X  ", _CFG[chip]->ADPT_DETAIL.reg);	term_printf("\n");
	_print_addr("ADPT_STORE",	&_CFG[0]->ADPT_STORE.reg);	ALL term_printf("0x%04X  ", _CFG[chip]->ADPT_STORE.reg);	term_printf("\n");
	_print_addr("ADPT_CORR",	&_CFG[0]->ADPT_CORR.reg);	ALL term_printf("0x%04X  ", _CFG[chip]->ADPT_CORR.reg);		term_printf("\n");
	_print_addr("MON_CFG",		&_CFG[0]->MON_CFG.reg);		ALL term_printf("0x%04X  ", _CFG[chip]->MON_CFG.reg);		term_printf("\n");
	_print_addr("MON_ADC",		&_CFG[0]->MON_ADC.reg);		ALL term_printf("0x%04X  ", _CFG[chip]->MON_ADC.reg);		term_printf("\n");
	_print_addr("MON_OFF",		&_CFG[0]->MON_OFF.reg);		ALL term_printf("0x%04X  ", _CFG[chip]->MON_OFF.reg);		term_printf("\n");
	_print_addr("MON_GAIN",		&_CFG[0]->MON_GAIN.reg);	ALL term_printf("0x%04X  ", _CFG[chip]->MON_GAIN.reg);		term_printf("\n");
	_print_addr("MON_PHASE",	&_CFG[0]->MON_PHASE.reg);	ALL term_printf("0x%04X  ", _CFG[chip]->MON_PHASE.reg);		term_printf("\n");
	_print_addr("FLT_CFG",		&_CFG[0]->FLT_CFG.reg);		ALL term_printf("0x%04X  ", _CFG[chip]->FLT_CFG.reg);		term_printf("\n");
	_print_addr("FLT_STAT",		&_CFG[0]->FLT_STAT.reg);	ALL term_printf("0x%04X  ", _CFG[chip]->FLT_STAT.reg);		term_printf("\n");
	_print_addr("FLT_EN",		&_CFG[0]->FLT_EN.reg);		ALL term_printf("0x%04X  ", _CFG[chip]->FLT_EN.reg);		term_printf("\n");
	
	term_printf("--- TW8 - Speed -----------------------------\n");
	_print_addr("POS",			&_PosSpeed[0]->pos);		ALL term_printf("%06d  ", _PosSpeed[chip]->pos);			term_printf("\n");
	_print_addr("SPEED",		&_PosSpeed[0]->speed);		ALL term_printf("%06d  ", _PosSpeed[chip]->speed);			term_printf("\n");
}

//--- _tw8_display_var ---------------------------
static void _tw8_display_var(void)
{
	int chip = 0;
#define ALL for(chip=0; chip<CHIP_CNT; chip++)
	
	term_printf("--- TW8 - VAR-BLOCK -----------------------------\n");
	_print_addr("MAIN_CFG",		&_Var[0]->STAT_SP.reg);	ALL term_printf("0x%04X  ", _Var[chip]->STAT_SP.reg);	term_printf("\n");
	_print_addr("STAT_EE",		&_Var[0]->STAT_EE.reg);	ALL term_printf("0x%04X  ", _Var[chip]->STAT_EE.reg);	term_printf("\n");
	_print_addr("ADC_SIN",		&_Var[0]->ADC_SIN);		ALL term_printf("0x%04X  ", _Var[chip]->ADC_SIN);	term_printf("\n");
	_print_addr("ADC_COS",		&_Var[0]->ADC_COS);		ALL term_printf("0x%04X  ", _Var[chip]->ADC_COS);	term_printf("\n");
	_print_addr("ADC_AMP",		&_Var[0]->ADC_AMP);		ALL term_printf("0x%04X  ", _Var[chip]->ADC_AMP);	term_printf("\n");
	_print_addr("QM_ADC",		&_Var[0]->QM_ADC);		ALL term_printf("0x%04X  ", _Var[chip]->QM_ADC);	term_printf("\n");
	_print_addr("QM_ADAPT",		&_Var[0]->QM_ADAPT);	ALL term_printf("0x%04X  ", _Var[chip]->QM_ADAPT);	term_printf("\n");
	_print_addr("CMD",			&_Var[0]->CMD);			ALL term_printf("0x%04X  ", _Var[chip]->CMD);		term_printf("\n");
}

//--- _tw8_display_par ---------------------------
static void _tw8_display_par(void)
{
	int chip = 0;
#define ALL for(chip=0; chip<CHIP_CNT; chip++)
	
	term_printf("--- TW8 - PAR-NOW-BLOCK -----------------------------\n");
	_print_addr("DGAIN", &_Par_Now[0]->DGAIN); ALL term_printf("0x%04X  ", _Par_Now[chip]->DGAIN); term_printf("\n");
	_print_addr("DOFFS", &_Par_Now[0]->DOFFS); ALL term_printf("0x%04X  ", _Par_Now[chip]->DOFFS); term_printf("\n");
	_print_addr("DOFFC", &_Par_Now[0]->DOFFC); ALL term_printf("0x%04X  ", _Par_Now[chip]->DOFFC); term_printf("\n");
	_print_addr("DPH",   &_Par_Now[0]->DPH);   ALL term_printf("0x%04X  ", _Par_Now[chip]->DPH);   term_printf("\n");
	_print_addr("AGAIN", &_Par_Now[0]->AGAIN); ALL term_printf("0x%04X  ", _Par_Now[chip]->AGAIN); term_printf("\n");
	_print_addr("AOFFS", &_Par_Now[0]->AOFFS); ALL term_printf("0x%04X  ", _Par_Now[chip]->AOFFS); term_printf("\n");
	_print_addr("AOFFC", &_Par_Now[0]->AOFFC); ALL term_printf("0x%04X  ", _Par_Now[chip]->AOFFC); term_printf("\n");
	
	term_printf("--- TW8 - PAR-BAK-BLOCK -----------------------------\n");
	_print_addr("DGAIN", &_Par_Bak[0]->DGAIN); ALL term_printf("0x%04X  ", _Par_Bak[chip]->DGAIN); term_printf("\n");
	_print_addr("DOFFS", &_Par_Bak[0]->DOFFS); ALL term_printf("0x%04X  ", _Par_Bak[chip]->DOFFS); term_printf("\n");
	_print_addr("DOFFC", &_Par_Bak[0]->DOFFC); ALL term_printf("0x%04X  ", _Par_Bak[chip]->DOFFC); term_printf("\n");
	_print_addr("DPH",   &_Par_Bak[0]->DPH);   ALL term_printf("0x%04X  ", _Par_Bak[chip]->DPH);   term_printf("\n");
	_print_addr("AGAIN", &_Par_Bak[0]->AGAIN); ALL term_printf("0x%04X  ", _Par_Bak[chip]->AGAIN); term_printf("\n");
	_print_addr("AOFFS", &_Par_Bak[0]->AOFFS); ALL term_printf("0x%04X  ", _Par_Bak[chip]->AOFFS); term_printf("\n");
	_print_addr("AOFFC", &_Par_Bak[0]->AOFFC); ALL term_printf("0x%04X  ", _Par_Bak[chip]->AOFFC); term_printf("\n");
}

//--- _tw8_check_err ---------------------------
static void _tw8_check_err(void)
{	
	return;
	
	int chip;
	int flag=0x01;
	for(chip=0; chip<CHIP_CNT; chip++)
	{
		 // ADC Overflow
		if (_Var[chip]->STAT_SP.adcof == 1)		ErrorFlag(ERR_CONT, &_ErrorFlags, flag, 0, "Analog Encoder (chip %d): ADC Overflow", chip);
		flag<<=1;
		 // ADC Underflow
		if (_Var[chip]->STAT_SP.adcuf == 1)		ErrorFlag(ERR_CONT, &_ErrorFlags, flag, 0, "Analog Encoder (chip %d): ADC Underflow", chip);
		flag<<=1;
		 // Excessive Adaption
		if (_Var[chip]->STAT_SP.adapt == 1)		ErrorFlag(ERR_CONT, &_ErrorFlags, flag, 0, "Analog Encoder (chip %d): Excessive Adaption", chip); 
		flag<<=1;
		 // Speed Limit Exceeded
		if (_Var[chip]->STAT_SP.vtop == 1)		ErrorFlag(ERR_CONT, &_ErrorFlags, flag, 0, "Analog Encoder (chip %d): Speed Limit Exceeded", chip); // Exaple if cfg.AB_VTOP	= 0x7F; -> not sticky
		flag<<=1;
		 // Overspeed Warning
		if (_Var[chip]->STAT_SP.ospeed == 1)	ErrorFlag(WARN,		&_ErrorFlags, flag, 0, "Analog Encoder (chip %d): Overspeed Warning", chip);	
		flag<<=1;
		 // Fatal Overspeed Fault
		if (_Var[chip]->STAT_SP.fatal == 1)		ErrorFlag(ERR_CONT, &_ErrorFlags, flag, 0, "Analog Encoder (chip %d): Fatal Overspeed Fault", chip); 
		flag<<=1;
		 // Excessive Filter Lag (faulty AB output signals)
		if (_Var[chip]->STAT_SP.lag == 1)		ErrorFlag(ERR_CONT, &_ErrorFlags, flag, 0, "Analog Encoder (chip %d): Excessive Filter Lag", chip);
		flag<<=1;
		 // Excessive AB Output Lag
		if (_Var[chip]->STAT_SP.ab == 1)		ErrorFlag(ERR_CONT, &_ErrorFlags, flag, 0, "Analog Encoder (chip %d): Excessive AB Output Lag, above AB output freq limit", chip);  // Exaple if cfg.AB_VTOP	= 0x7F;
		flag<<=1;
		 // Interface Timeout
		if (_Var[chip]->STAT_SP.one_wire == 1)	ErrorFlag(WARN,		&_ErrorFlags, flag, 0, "Analog Encoder (chip %d): Interface Timeout", chip);
		flag<<=1;
		 // External Crystal Fault
		if (_Var[chip]->STAT_SP.xtal == 1)		ErrorFlag(ERR_CONT, &_ErrorFlags, flag, 0, "Analog Encoder (chip %d): External Crystal Fault", chip);	
		flag<<=1;
		 // EEPROM Error
		if (_Var[chip]->STAT_EE.reg > 0)		ErrorFlag(ERR_CONT, &_ErrorFlags, flag, 0, "Analog Encoder (chip %d): EEPROM Error %s", read, chip);			
		flag<<=1;
	}
}

//--- tw8_reset_error ---------------------------
void tw8_reset_error(void)
{
	return;
	
	int chip;	
	for(chip=0; chip<CHIP_CNT; chip++)
	{
		UINT16	clear_err  = 0x0;
		tw8_write(chip, 0x809A, sizeof(clear_err), &clear_err);
		tw8_write(chip, 0x809C, sizeof(clear_err), &clear_err);		
	}
	
	_ErrorFlags = 0;
	
	if (_Var[chip]->STAT_SP.adapt == 1) Error(ERR_CONT, 0, "Analog Encoder Error: Excessive Adaption Error needs a Reboot");	
}

//--- _write ---------------------------------------------
static int _write(int chip, BYTE cmdcode, UINT16 addr, BYTE len, void *data)
{
	rx_tw8_ctrl_cmd *pcmd;
	int idx1 = (_FifoInIdx + 1) % FIFO_SIZE;
	if (idx1 == _FifoOutIdx) 
	{
		TrPrintfL(TRUE, "FIFO Overfilled\n"); 
		return REPLY_ERROR;
	}
		
	pcmd = &_Fifo[_FifoInIdx];
	pcmd->chip   = CHIP_0 + chip;
	pcmd->cmd	 = cmdcode;
	pcmd->addr	 = addr;
	pcmd->length = len;
	if (data != NULL) memcpy(pcmd->data, data, len);
		
	_FifoInIdx = idx1;
	
	return REPLY_OK;
}

//--- _read -------------------------------------------------
static int _read(void)
{
	int					len;
	UINT16				*reg;
	rx_tw8_ctrl_answer	answer;
	memset(&answer, 0, sizeof(answer));
	while (TRUE)
	{
		if (comm_received(0, fgetc(_tw8_in)))
		{
			if (comm_get_data(0, (BYTE*)&answer, sizeof(answer), &len))
			{
				if (len == sizeof(answer) - sizeof(answer.data) + answer.length)
				{
					TrPrintfL(TRACE_CHIP_1, "TW8 RECV: Reg[%d][0x%04x], len=%d, data=0x%04x, err=%d", answer.chip - CHIP_0, answer.addr, answer.length, *((UINT16*)&answer.data[0]), answer.err);
					if (answer.err)
					{
						TrPrintfL(TRUE, "TW8 RECV: Reg[%d][0x%04x], len=%d, data=0x%04x, err=%d", answer.chip - CHIP_0, answer.addr, answer.length, *((UINT16*)&answer.data[0]), answer.err);
					}
					else if (answer.addr >= REG_BASE && answer.addr + answer.length < REG_BASE + REG_CNT)
					{
						memcpy(&_Registers[answer.chip - CHIP_0][answer.addr - REG_BASE], answer.data, answer.length);
						_Present = TRUE;
					}
					break;
				}
			}
		}
	}
	return REPLY_OK;
}

//--- _tw8_thread ---------------------------------
static void* _tw8_thread(void *par)
{
	int idx = 0;
	int	len;
	BYTE buf[1024];
	rx_tw8_ctrl_cmd *pcmd;

	while (TRUE)
	{
		_write(idx % CHIP_CNT, TW8_READ_MEM, _RegList[idx / CHIP_CNT].addr, _RegList[idx / CHIP_CNT].len, 0);
		while (_FifoOutIdx != _FifoInIdx)
		{
			pcmd = &_Fifo[_FifoOutIdx];
			TrPrintfL(TRACE_CHIP_1, "TW8 SEND: Reg[%d][0x%04x], len=%d", pcmd->chip - CHIP_0, pcmd->addr, pcmd->length);
			len = sizeof(rx_tw8_ctrl_cmd) - DATA_SIZE + pcmd->length;
			comm_encode(&_Fifo[_FifoOutIdx], len, buf, sizeof(buf), &len);
	
			fwrite(&buf, 1, len, _tw8_out);
			fflush(_tw8_out);
			
			_FifoOutIdx = (_FifoOutIdx + 1) % FIFO_SIZE;
			
			_read();
		}
		idx = (idx + 1) % (CHIP_CNT*SIZEOF(_RegList));
	}
}
	
//--- tw8_write --------------------------------------
int		tw8_write(int chip, UINT32 addr, BYTE len, void *data)
{
	return _write(chip, TW8_WRITE_MEM, addr, len, data);
}

//--- tw8_read --------------------------------------
int	tw8_read(int chip, UINT32 addr, BYTE len)
{
	return _write(chip, TW8_READ_MEM, addr, len, NULL);
}

//--- tw8_present ----------------------------------------
int	tw8_present(void)
{
	return _Present;				
}
