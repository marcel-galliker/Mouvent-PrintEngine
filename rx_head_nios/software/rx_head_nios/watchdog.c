/*
 * watchdog.c
 *
 *  Created on: 31.08.2016
 *      Author: stefan
 */
#include <stdio.h>
#include "alt_types.h"
#include "sys/alt_irq.h"
#include "altera_avalon_pio_regs.h"
#include <unistd.h>
#include "sys/alt_stdio.h"
#include "system.h"
#include "io.h"
#include "watchdog.h"
#include "nios_def_head.h"
#include "amc7891.h"
#include "trprintf.h"


#define FREQ 50000000					// clock frequency of qsys in Hz
#define watchdog_max	0x1FFFFFFF		// 29bit = maximal 10'737 ms

#define debug_mode_bit 			0xa0000000
#define ignore_fp_err_bit		0x20000000
#define watchdog_tgl_bit 		0x40000000

//--- watchdog_init ---------------------------------------------
void watchdog_init(void)
{
	//Clear Watchdog Register
	IOWR_ALTERA_AVALON_PIO_DATA(WATCHDOG_CNT_PIO_BASE, 0x00000000);
}

//--- watchdog_start_debug_mode -------------------------------
void watchdog_start_debug_mode(void)
{
	//Watchdog always enabled -> for debug purpose only
	IOWR_ALTERA_AVALON_PIO_DATA(WATCHDOG_CNT_PIO_BASE, debug_mode_bit);
}

//--- watchdog_toggle ---------------------------------------------
void watchdog_toggle(int watchdog_time, int check_FP)
{
	static int toggle=0;
	int data;

	toggle = !toggle;
	data=FREQ/1000*watchdog_time; //watchdog_time in ms

	if(data>watchdog_max) data=watchdog_max; // saturate at watchdog_max

	if (!check_FP) data |= ignore_fp_err_bit; // enable/disable watchdog reacting on fire-pulse error

	if (toggle) data |= watchdog_tgl_bit; // toggle to refresh counter

	IOWR_ALTERA_AVALON_PIO_DATA(WATCHDOG_CNT_PIO_BASE, data);
}

//--- watchdog_check_electronic_errors --------------------------
int watchdog_check_electronic_errors(int check)
{
	int error=0;

	// FIREPULS AC CURRENT
	pRX_Status->fp_ac_error=IORD_ALTERA_AVALON_PIO_DATA(FIREPULS_AC_ERR_IN_BASE);
	if(pRX_Status->fp_ac_error)
	{
		if (check) pRX_Status->error.fp_ac=TRUE;
		error |= 2;
	}

	// FIREPULS DC CURRENT
	pRX_Status->fp_dc_error=IORD_ALTERA_AVALON_PIO_DATA(FIREPULS_DC_ERR_IN_BASE);
	if(pRX_Status->fp_dc_error)
	{
		if (check) pRX_Status->error.fp_dc=TRUE;
		error|=4;
	}

	// WATCHDOG MODE readback
	pRX_Status->info.watchdog_dbg_mode = (IORD_ALTERA_AVALON_PIO_DATA(WATCHDOG_CNT_PIO_BASE)==debug_mode_bit);

	// Gain Test
	/*
	IOWR_ALTERA_AVALON_PIO_DATA(FIREPULS_GAIN_OUT_BASE, 0);	// all gain off
	IOWR_ALTERA_AVALON_PIO_DATA(FIREPULS_GAIN_OUT_BASE, 1); // only gain fp0 on
	IOWR_ALTERA_AVALON_PIO_DATA(FIREPULS_GAIN_OUT_BASE, 2); // only gain fp1 on
	IOWR_ALTERA_AVALON_PIO_DATA(FIREPULS_GAIN_OUT_BASE, 4); // only gain fp2 on
	IOWR_ALTERA_AVALON_PIO_DATA(FIREPULS_GAIN_OUT_BASE, 8); // only gain fp3 on
	IOWR_ALTERA_AVALON_PIO_DATA(FIREPULS_GAIN_OUT_BASE, 0); // all off
	*/

	// Head 3V3 float error Test
	/*
	switch(test)
	{
		case 0:		IOWR_16DIRECT(AMC7891_0_BASE,AMC7891_GPIO_OUT,0x00FF);	// 3V3float all on
			break;

		case 1:		IOWR_16DIRECT(AMC7891_0_BASE,AMC7891_GPIO_OUT,0x0004);	// H1_3V3float_B on, others off
			break;
		case 2:		IOWR_16DIRECT(AMC7891_0_BASE,AMC7891_GPIO_OUT,0x0020);	// H1_3V3float_F on, others off
			break;
		case 3:		IOWR_16DIRECT(AMC7891_0_BASE,AMC7891_GPIO_OUT,0x0024);	// H1_3V3float_B and H1_3V3float_B on, others off
			break;

		case 4:		IOWR_16DIRECT(AMC7891_0_BASE,AMC7891_GPIO_OUT,0x0008);	// H2_3V3float_B on, others off
			break;
		case 5:		IOWR_16DIRECT(AMC7891_0_BASE,AMC7891_GPIO_OUT,0x0010);	// H2_3V3float_F on, others off
			break;
		case 6:		IOWR_16DIRECT(AMC7891_0_BASE,AMC7891_GPIO_OUT,0x0018);	// H2_3V3float_B and H2_3V3float_B on, others off
			break;

		case 7:		IOWR_16DIRECT(AMC7891_0_BASE,AMC7891_GPIO_OUT,0x0001);	// H3_3V3float_B on, others off
			break;
		case 8:		IOWR_16DIRECT(AMC7891_0_BASE,AMC7891_GPIO_OUT,0x0080);	// H3_3V3float_F on, others off
			break;
		case 9:		IOWR_16DIRECT(AMC7891_0_BASE,AMC7891_GPIO_OUT,0x0081);	// H3_3V3float_B and H3_3V3float_B on, others off
			break;

		case 10:	IOWR_16DIRECT(AMC7891_0_BASE,AMC7891_GPIO_OUT,0x0002);	// H4_3V3float_B on, others off
			break;
		case 11:	IOWR_16DIRECT(AMC7891_0_BASE,AMC7891_GPIO_OUT,0x0040);	// H4_3V3float_F on, others off
			break;
		case 12:	IOWR_16DIRECT(AMC7891_0_BASE,AMC7891_GPIO_OUT,0x0042);	// H4_3V3float_B and H4_3V3float_B on, others off
			break;

		case 13:	IOWR_16DIRECT(AMC7891_0_BASE,AMC7891_GPIO_OUT,0x0000);	// all off
			break;
		default:
			break;
	}*/
	return error;
}

