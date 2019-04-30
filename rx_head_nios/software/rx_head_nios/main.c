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
#include "i2c.h"
#include "timer.h"
#include "trprintf.h"
#include "rx_ink_common.h"
#include "nios_def_head.h"
#include "version.h"
#include "uart.h"
#include "conditioner_bootloader.h"
#include "altera_avalon_uart_regs.h"
#include "adc_thermistor.h"
#include "cond_def_head.h"
#include "comm.h"
#include "watchdog.h"
#include "head_eeprom.h"
#include "power.h"
#include "pres.h"
#include "cooler.h"
#include "rx_mem.h"

//Prototype

SNiosMemory *arm_ptr;
SNiosCfg *pRX_Config;
SNiosStat *pRX_Status;
static UINT32 _StaticErrors;

typedef struct {
	int timer;
	UINT32 alive;
	UINT32 error;
	UINT32 arm_alive;
} _SConditioner;

static _SConditioner _Cond[MAX_HEADS_BOARD];

//--- main_handle_cond_msg ---------------------------
static void _handle_cond_msg(int condNo) {
	_Cond[condNo].timer = 3;
	if (_Cond[condNo].error & COND_ERR_rebooted) {
		pRX_Status->cond[condNo].error |= COND_ERR_rebooted;
	} else {
		if (pRX_Status->cond[condNo].alive < _Cond[condNo].alive
				&& _Cond[condNo].alive < 0x80000000)
			_Cond[condNo].error |= COND_ERR_rebooted;
		_Cond[condNo].alive = pRX_Status->cond[condNo].alive;
	}

	/*
	 if (condNo==3 && pRX_Status->cond[condNo].logCnt)
	 {
	 int i;
	 for (i=0; i<pRX_Status->cond[condNo].logCnt; i++)
	 {
	 trprintf("%03d: %03d %03d\n", pRX_Status->cond[condNo].log[i].no, pRX_Status->cond[condNo].log[i].pressure, pRX_Status->cond[condNo].log[i].pump);
	 }

	 }
	 */

	/*
	 trprintf("Cond[%d]: Version: %d.%d.%d.%d, ",
	 condNo,
	 (int)arm_ptr->stat.cond[condNo].version.major,
	 (int)arm_ptr->stat.cond[condNo].version.minor,
	 (int)arm_ptr->stat.cond[condNo].version.revision,
	 (int)arm_ptr->stat.cond[condNo].version.build);
	 trprintf("alive=%d, temp=%d, presin=%d, presout=%d, pump=%d, valve=%d\n",
	 (int)pRX_Status->cond[condNo].alive,
	 (int)pRX_Status->cond[condNo].temp,
	 (int)pRX_Status->cond[condNo].pressure_in,
	 (int)pRX_Status->cond[condNo].pressure_out,
	 (int)pRX_Status->cond[condNo].pump,
	 (int)pRX_Status->cond[condNo].info.valve);
	 */
}

//--- main_rebooting_cond ------------------------------
void main_rebooting_cond(void) {
	int i;
	for (i = 0; i < MAX_HEADS_BOARD; i++)
		_Cond[i].alive = 0xFFFFFFFF;
}

//--- main_tick_1000ms ------------------------------
void main_tick_1000ms(void) {
	int condNo;

	if (!bootloader_running()) {
		for (condNo = 0; condNo < MAX_HEADS_BOARD; condNo++) {
			if (pRX_Status->cond[condNo].info.connected) {
				if (--_Cond[condNo].timer < 0) {
					pRX_Status->cond[condNo].info.connected = FALSE;
					pRX_Status->cond[condNo].error |= COND_ERR_connection_lost;
				}
				pRX_Status->error.arm_timeout = (_Cond[condNo].arm_alive == pRX_Config->cond[condNo].alive);
				_Cond[condNo].arm_alive = pRX_Config->cond[condNo].alive;
			}
		}
	}
}

//--- alt_avalon_sysid_qsys_test ------------------------------------------------
alt_32 alt_avalon_sysid_qsys_test(void) {
	// Read the hardware-tag, aka value0, from the hardware. */
	alt_u32 hardware_id = IORD_ALTERA_AVALON_SYSID_QSYS_ID(SYSID_QSYS_BASE);

	/* Read the time-of-generation, aka value1, from the hardware register. */
	alt_u32 hardware_timestamp = IORD_ALTERA_AVALON_SYSID_QSYS_TIMESTAMP(
			SYSID_QSYS_BASE);

	/* Return 0 if the hardware and software appear to be in sync. */
	if ((SYSID_QSYS_TIMESTAMP == hardware_timestamp)
			&& (SYSID_QSYS_ID == hardware_id)) {
		return 0;
	}

	/*
	 *  Return 1 if software appears to be older than hardware (that is,
	 *  the value returned by the hardware is larger than that recorded by
	 *  the generator function).
	 *  If the hardware time happens to match the generator program's value
	 *  (but the hardware tag, value0, doesn't match or 0 would have been
	 *  returned above), return an arbitrary value, let's say -1.
	 */
	return ((alt_32) (hardware_timestamp - SYSID_QSYS_TIMESTAMP)) > 0 ? 1 : -1;
}

//--- main_error_reset ------------------------------
void main_error_reset(void) {
	pRX_Status->error.err = _StaticErrors;
}

/*
//--- _eeprom_test -----------------------------
static void _eeprom_test(void)
{
	int temp_count=0;
	BYTE temp_head_user_data_read[34];
	BYTE temp_head_user_data_write[34];

	memset(temp_head_user_data_read,  0, sizeof(temp_head_user_data_read));
	memset(temp_head_user_data_write, 0, sizeof(temp_head_user_data_write));

	temp_head_user_data_write[0]=0x12;//0xb7;
	temp_head_user_data_write[1]=0x34;//0x86;

	// test with head 1
	//int head_eeprom_read_user_data(alt_u32 	head, alt_u8 * eeprom_data, alt_u32 number_of_byte_to_read, alt_u32 start_adr)
	if(head_eeprom_read_user_data(1, &temp_head_user_data_read[0], 33, 0x00))
	{
		pRX_Status->error.head_eeprom_read = TRUE;
	}

	for(temp_count=0; temp_count<34;temp_count++)
	{
		temp_head_user_data_write[temp_count]=0xa0+temp_count;
	}

	//head_eeprom_write_user_data_seq(alt_u32 head, alt_u8 * eeprom_data, alt_u32 number_of_byte_to_write, alt_u32 start_adr)
	if(head_eeprom_write_user_data_seq(1, &temp_head_user_data_write[0], 32, 0x00))
	{
		pRX_Status->error.head_eeprom_write = TRUE;
	}

	// test with head 1
	//int head_eeprom_read_user_data(alt_u32 	head, alt_u8 * eeprom_data, alt_u32 number_of_byte_to_read, alt_u32 start_adr)
	if(head_eeprom_read_user_data(1, &temp_head_user_data_read[0], 33, 0x00))
	{
		pRX_Status->error.head_eeprom_read = TRUE;
	}
}
*/

//--- main ----------------------------------------------------
int main() {
	// _DEBUG must only be enabled when downloading through hardware debugger.
	// Otherwise modes (PRINT, OFF, ..) do not work correctly
//	tr_debug();

	// Dual port ram initialisation
	arm_ptr = (SNiosMemory *) DP_RAM_NIOS_ARM_BASE;
	pRX_Config = &arm_ptr->cfg;
	pRX_Status = &arm_ptr->stat;
	mem_init(&arm_ptr[1], DP_RAM_NIOS_ARM_SIZE_VALUE - sizeof(SNiosMemory));

	memset(_Cond, 0, sizeof(_Cond));
	memset(pRX_Config, 0, sizeof(*pRX_Config));
	memset(pRX_Status, 0, sizeof(*pRX_Status));

	get_version(&pRX_Status->version);

	pRX_Status->QSYS_id 		= SYSID_QSYS_ID;
	pRX_Status->QSYS_timestamp 	= SYSID_QSYS_TIMESTAMP;

	if (alt_avalon_sysid_qsys_test())
		pRX_Status->error.fpga_incompatible = TRUE;
	// pRX_Status->info.sys_id_checked=1;

	IOWR_ALTERA_AVALON_PIO_DATA(PIO_NIOS_LED_BASE, 0x03);// Nios all LED's off

	// Analog monitor initialisation
	if (init_AMC7891())
	{
		pRX_Status->error.amc7891 = TRUE;

		pRX_Status->u_minus_36v = INVALID_VALUE;
		pRX_Status->u_minus_5v = INVALID_VALUE;
		pRX_Status->u_plus_5v = INVALID_VALUE;
		pRX_Status->u_plus_2v5 = INVALID_VALUE;
		pRX_Status->headcon_amc_temp = INVALID_VALUE;
//		for(val=0; val < MAX_HEADS_BOARD; val++)
//			pRX_Status->head_temp[val] = pRX_Config->cond[val].tempHead = INVALID_VALUE;
	}

	// I2C initialisation
	IOWR_16DIRECT(AMC7891_0_BASE, AMC7891_DAC0_DATA, _3V3);// Power for Levelshifter I2C and Flow Check Sensor
	init_I2C(I2C_MASTER_0_BASE);// Head EEPROM (I2C Adr. 0x52/0x54/0x56/0x58)
	init_I2C(I2C_MASTER_1_BASE);// 8 x MOSFET Temperatures, Flow Check Pressure (I2C Adr. 0x78), Thermistor (0x90) Cooler PCB

	cooler_init();
	power_init();

	// set default values for conditioner boards
	IOWR_ALTERA_AVALON_PIO_DATA(COND_PIO_0_BASE, 0b0001); //reserve=0, oe_bootloader=0; boot_uc=0, reset=1	//disable oe_bootloader
	IOWR_ALTERA_AVALON_PIO_DATA(COND_PIO_1_BASE, 0b0001); //reserve=0, oe_bootloader=0; boot_uc=0, reset=1	//disable oe_bootloader
	IOWR_ALTERA_AVALON_PIO_DATA(COND_PIO_2_BASE, 0b0001); //reserve=0, oe_bootloader=0; boot_uc=0, reset=1	//disable oe_bootloader
	IOWR_ALTERA_AVALON_PIO_DATA(COND_PIO_3_BASE, 0b0001); //reserve=0, oe_bootloader=0; boot_uc=0, reset=1	//disable oe_bootloader

	pres_init();	// init Flow Check Pressure Sensor
	timer_init();
	uart_init();

	trprintf("MAIN STARTED\n");

	if (head_eeprom_read())
	{
		pRX_Status->error.head_eeprom_read = TRUE;
		//init_I2C(I2C_MASTER_0_BASE);	// reinitialize I2C, as there is no head Connected
	};

	{
		int head;
		for (head = 0; head < MAX_HEADS_BOARD; head++)
		{
			head_eeprom_read_user_data(head, pRX_Status->user_eeprom[head], sizeof(pRX_Status->user_eeprom[head]), 0x00);
		}
	}
//	_eeprom_test();

	_StaticErrors = pRX_Status->error.err;
	pRX_Status->info.nios_ready = TRUE;
	pRX_Status->memSize = mem_available();

	// Event loop never exits
	while (1) {
		int condNo;

		timer_main();
		UCHAR data;
		for (condNo = 0; condNo < MAX_HEADS_BOARD; condNo++){
			if (uart_read(condNo, &data)) {
				if (comm_received(condNo, data)) {
					int length;
					SConditionerStat_mcu data;
					if (comm_get_data(condNo, (UCHAR*) &data, sizeof(data), &length))
					{
						if (length == sizeof(pRX_Status->cond[condNo]))
						{
							memcpy(&pRX_Status->cond[condNo], &data, sizeof(pRX_Status->cond[condNo]));
							_handle_cond_msg(condNo);
						}
						else pRX_Status->cond[condNo].error |= COND_ERR_status_struct_missmatch;
					}
				}
			}
		}
	}	//end while (1)
	return (0);
}	// end int main()
