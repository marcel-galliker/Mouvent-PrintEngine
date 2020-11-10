#include <stdio.h>
#include "alt_types.h"
#include "sys/alt_irq.h"
#include "altera_avalon_timer.h"
#include "altera_avalon_timer_regs.h"
#include "altera_avalon_pio_regs.h"
#include "altera_avalon_sysid_qsys.h"
#include "altera_avalon_sysid_qsys_regs.h"
#include <unistd.h>
#include "sys/alt_stdio.h"
#include "system.h"
#include "io.h"
#include "sys/alt_alarm.h"
#include "AMC7891.h"
#include "MAX31865.h"
#include "i2c.h"
#include "timer.h"
#include "nios_def_fluid.h"
#include "trprintf.h"
#include "log.h"
#include "pid.h"
#include "pio.h"
#include "pres.h"
#include "ink_ctrl.h"
#include "fpga_def_fluid.h"
#include "version.h"
#include "heater.h"

/* Watchdog bit description
 *
 * 31 -> 1 = ignore watchdog counter value, 0 = needs toggling of bit 30
 * 30 -> toggle to refill counter value
 * 29 -> 1 = ignore firepulse errors; not used
 * 27 -> 1 = set watchdog output to '1'
 * 26 -> (26 DOWNTO 0) = watchdog counter value
 */
#define WATCHDOG_PERIOD_10MS 	25000
#define WATCHDOG_DISABLE_MASK	(1 << 31)
#define WATCHDOG_TOGGLE_MASK 	(1 << 30)
#define WATCHDOG_OUTPUT_MASK 	(1 << 27)
#define WATCHDOG_ON				2000
#define WATCHDOG_OFF			4000
#define WATCHDOG_RELOAD_VALUE	0x000ffff
#define WATCHDOG_RELOAD_VAL_MAX	0x3ffffff

#if (WATCHDOG_RELOAD_VALUE > WATCHDOG_RELOAD_VAL_MAX)
#error "WATCHDOG_RELOAD_VALUE has only 26 bits and must not be bigger than 0x3ffffff"
#endif

//--- global variables -----------------------------------------------------
SNiosFluidStat *pRX_Status;
SNiosFluidCfg  *pRX_Config;

static UINT32	_StaticErrors;
static UINT32	_Watchdog_ok;

//--- _delay ----------------------------------
static void _delay(void)
{
	UINT32 i;

	for(i = 0; i < 6000; i++)
		;
}

//--- main_error_reset ------------------------------
void main_error_reset(void)
{
	pRX_Status->error = _StaticErrors;

	ink_error_reset();
	heater_error_reset();

	// re-initialize SPI after error to be able to read temperature
	init_AMC7891(AVALON_SPI_AMC7891_1_BASE);
	_Watchdog_ok = IORD_32DIRECT(AXI_LW_SLAVE_REGISTER_0_BASE, WATCHDOG_ERR);
	//init_AMC7891(AXI_LW_SLAVE_REGISTER_0_BASE);
}

void get_version(SVersion *pversion)
{
	int val[4], i;
	const char *ch;
	memset(val, 0, sizeof(val));
	for (i=0,ch=version; *ch; ch++)
	{
		if (*ch=='.') i++;
		if (*ch>='0' && *ch<='9') val[i] = 10*val[i]+*ch-'0';
	}
	pversion->major 	= val[0];
	pversion->minor 	= val[1];
	pversion->revision 	= val[2];
	pversion->build 	= val[3];
};

//--- main --------------------------
int main()
{
	SNiosFluidMemory *arm_ptr;
	int i;
	static const int RETRY = 3;
	int retry;		// number of retries for HW initialization

	// _DEBUG must only be enabled when downloading through hardware debugger.
	// Otherwise modes (PRINT, OFF, ..) do not work correctly
	// tr_debug();

	arm_ptr	= (SNiosFluidMemory *) ONCHIP_MEMORY_NIOS_ARM_BASE;
	pRX_Status = &arm_ptr->stat;
	pRX_Config = &arm_ptr->cfg;

	memset(pRX_Status, 0, sizeof(*pRX_Status));
	memset(pRX_Config, 0, sizeof(*pRX_Config));
	pRX_Status->info.is_shutdown = 0;

	//--- FPGA Version -------------------------------------------------
	pRX_Status->FpgaVersion.major 		= IORD_32DIRECT(AXI_LW_SLAVE_REGISTER_0_BASE, 0x00);
	pRX_Status->FpgaVersion.minor 		= IORD_32DIRECT(AXI_LW_SLAVE_REGISTER_0_BASE, 0x04);
	pRX_Status->FpgaVersion.revision 	= IORD_32DIRECT(AXI_LW_SLAVE_REGISTER_0_BASE, 0x08);
	pRX_Status->FpgaVersion.build 		= IORD_32DIRECT(AXI_LW_SLAVE_REGISTER_0_BASE, 0x0c);

	//--- check QSYS-ID and Timestamp ----------------------------------
	pRX_Status->qsys_id  		= SYSID_ID;
	pRX_Status->qsys_timestamp 	= SYSID_TIMESTAMP;

	if (alt_avalon_sysid_qsys_test())
		pRX_Status->error |= err_fpga_incompatible;

	//--- NIOS Version -------------------------------------------------
	get_version(&pRX_Status->version);
	
	for(i=0; i<NIOS_INK_SUPPLY_CNT; i++)
	{
		pRX_Config->ink_supply[i].ctrl_mode = ctrl_off;
		pRX_Config->ink_supply[i].condPresIn = INVALID_VALUE;
	}

	IOWR_ALTERA_AVALON_PIO_DATA    (PIO_OUTPUT_BASE,	0x0000);				// All output off

	// PWM Initialisation
//	IOWR_ALTERA_AVALON_PIO_SET_BITS(PIO_OUTPUT_BASE,	SENSOR_POWER_ENABLE);	// 24V Enable
	IOWR_ALTERA_AVALON_PIO_DATA(FLUSH_PUMP_PWM_DUTY_CYCLE_BASE,	0x0000);		// PWM OFF
	IOWR_ALTERA_AVALON_PIO_DATA(FLUSH_PUMP_PWM_FREQ_BASE,		0x1000);		// PWM OFF

	// I2C initialisation
	init_I2C(I2C_MASTER_IS1_BASE);	// IS Adapter 1
	init_I2C(I2C_MASTER_IS2_BASE);	// IS Adapter 2
	init_I2C(I2C_MASTER_IS3_BASE);	// IS Adapter 3
	init_I2C(I2C_MASTER_IS4_BASE);	// IS Adapter 4
	init_I2C(I2C_MASTER_F_BASE);	// Flush
	init_I2C(I2C_MASTER_D_BASE);	// Pressure
	init_I2C(I2C_MASTER_P_BASE);	// Vacuum

	// Analog monitor initialisation FluidBoard
	for(retry=0; (init_AMC7891(AVALON2FPGA_SLAVE_0_BASE)!=0) && (retry<RETRY); retry++)
		_delay();

	if(retry>=RETRY) pRX_Status->error |= err_amc_fluid;

	// Analog monitor initialisation HeaterBoard
	for(retry=0; (init_AMC7891(AVALON_SPI_AMC7891_1_BASE)!=0) && (retry<RETRY); retry++)
	//for(retry=0; (init_AMC7891(AXI_LW_SLAVE_REGISTER_0_BASE)!=0) && (retry<RETRY); retry++)
		_delay();

	if(retry>=RETRY) pRX_Status->error |= err_amc_heater;
	else			 heater_init();

	//todo if no heater board is connected on purpose there should *NOT* be an error!

	// PT100 ADC initialisation
	init_MAX31865();

	// SPI Board 1
	IOWR_16DIRECT(AXI_LW_SLAVE_REGISTER_0_BASE, DAC_REG_0, 0x0000);	// Pumpe IS1
	IOWR_16DIRECT(AXI_LW_SLAVE_REGISTER_0_BASE, DAC_REG_1, 0x0000);	// Pumpe IS2
	IOWR_16DIRECT(AXI_LW_SLAVE_REGISTER_0_BASE, DAC_REG_2, 0x0000);	// Pumpe IS3
	IOWR_16DIRECT(AXI_LW_SLAVE_REGISTER_0_BASE, DAC_REG_3, 0x0000);	// Pumpe IS4

	// Watchdog
	IOWR_32DIRECT(AXI_LW_SLAVE_REGISTER_0_BASE, WATCHDOG_FREQ, WATCHDOG_PERIOD_10MS);

	pres_init();
	ink_init();
	timer_init();
	log_init();

	// turn on output for chiller
	IOWR_ALTERA_AVALON_PIO_SET_BITS(PIO_OUTPUT_BASE, CHILLER_OUT);

	_StaticErrors = pRX_Status->error;

	// turn off LEDs
	IOWR_ALTERA_AVALON_PIO_SET_BITS(PIO_OUTPUT_BASE, FPGA_LED_1_OUT);
	IOWR_ALTERA_AVALON_PIO_SET_BITS(PIO_OUTPUT_BASE, FPGA_LED_2_OUT);

	IOWR_32DIRECT(AXI_LW_SLAVE_REGISTER_0_BASE, WATCHDOG_CNT, (WATCHDOG_OUTPUT_MASK | WATCHDOG_RELOAD_VALUE));
	_Watchdog_ok = IORD_32DIRECT(AXI_LW_SLAVE_REGISTER_0_BASE, WATCHDOG_ERR);
	// Event loop never exits
	while (1)
	{
		// reload watchdog
		if (tr_debug_on())
			IOWR_ALTERA_AVALON_PIO_DATA(PIO_WATCHDOG_CNT_BASE, 0xffffffff);	// bit 31 set -> watchdog automatically reloaded
		else
			IOWR_32DIRECT(AXI_LW_SLAVE_REGISTER_0_BASE, WATCHDOG_CNT, (WATCHDOG_OUTPUT_MASK | WATCHDOG_RELOAD_VALUE));

		if (IORD_32DIRECT(AXI_LW_SLAVE_REGISTER_0_BASE, WATCHDOG_ERR)!=_Watchdog_ok)
			pRX_Status->error |= err_watchdog;

		timer_main();
	}

	return (0);
}
