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
#include "comm.h"
#include "sys/alt_alarm.h"
#include "timer.h"
#include "AMC7891.h"
#include "cooler.h"

#include "i2c_master_regs.h"
#include "i2c_master.h"

#include "nios_def_head.h"

#include "power.h"

#include "altera_avalon_uart.h"
#include "altera_avalon_uart_fd.h"
#include "altera_avalon_uart_regs.h"
#include "adc_thermistor.h"
#include "trprintf.h"
#include "main.h"
#include "head_eeprom.h"
#include "nios_def_head.h"
#include "pres.h"
#include "cooler.h"
#include "uart.h"
#include "conditioner_bootloader.h"

volatile UINT32		Timer_Tick=0;

static UINT32		Timer_10ms=0;

static int			Timer_10_in=0;
static int			Timer_10_out=0;

static int			Timer_250_in=0;
static int			Timer_250_out=0;

static int			Timer_1000_in=0;
static int			Timer_1000_out=0;

//--- prototypes ------------------------------------
static void _do_10ms_timer(void);
static void _do_250ms_timer(void);
static void _do_1000ms_timer(void);

static void _temp_savety(void);

int alt_irq_register (alt_u32 id, void* context, alt_isr_func handler);

//--- timer_getTick -----------------------
UINT32 timer_getTick(void)
{
	return Timer_Tick;
}

//--- timer_sleep -----------------------
void timer_sleep(UINT32 ms)
{
	UINT32 end = Timer_Tick+ms;
	while(Timer_Tick<end);
}

//--- timer_main -------------------------
void timer_main(void)
{
	while (Timer_10_out  !=Timer_10_in) 	{ _do_10ms_timer();  Timer_10_out++;}
	while (Timer_250_out !=Timer_250_in) 	{ _do_250ms_timer(); Timer_250_out++;}
	while (Timer_1000_out!=Timer_1000_in)   { _do_1000ms_timer();Timer_1000_out++;}
}

//--- _send_cond_data ----------------------
static void _send_cond_data(void)
{
	if (bootloader_running()) return;

	int condNo;
	for(condNo=0;condNo<MAX_HEADS_BOARD;condNo++)
	{
		UCHAR buf[2*sizeof(SConditionerCfg_mcu)+10];
		int len;
	//	trprintf("%d: Send Conditioner[%d] data\n", timer_getTick(), condNo);
		if (comm_encode(&pRX_Config->cond[condNo], sizeof(SConditionerCfg_mcu), buf, sizeof(buf), &len))
		{
			uart_write(condNo, buf, len);
		}
	}
}

//--- _do_10ms_timer ----------------------------------
static void _do_10ms_timer(void)
{
	Timer_10ms ++;
	if (!(Timer_10ms%10))
	{
		_send_cond_data();
		pres_tick_10ms();
		cooler_tick_10ms();
		power_tick_10ms();

		_temp_savety();
	}
}

//--- _do_250ms_timer -------------------------------------
static void _do_250ms_timer(void)
{
	volatile UINT16 dummy;

	/* First read does start AD conversion, result is only available at second read.
	 * That's why the code seems one off!
	 */
	dummy = IORD_16DIRECT(AMC7891_0_BASE,AMC7891_ADC4_DATA); // start conversion
	dummy++; // to ignore warning
	pRX_Status->head_temp[0] = pRX_Config->cond[0].tempHead = convert_head_temp(IORD_16DIRECT(AMC7891_0_BASE,AMC7891_ADC5_DATA) & 0x3ff);
	pRX_Status->head_temp[1] = pRX_Config->cond[1].tempHead = convert_head_temp(IORD_16DIRECT(AMC7891_0_BASE,AMC7891_ADC6_DATA) & 0x3ff);
	pRX_Status->head_temp[2] = pRX_Config->cond[2].tempHead = convert_head_temp(IORD_16DIRECT(AMC7891_0_BASE,AMC7891_ADC7_DATA) & 0x3ff);
	pRX_Status->head_temp[3] = pRX_Config->cond[3].tempHead = convert_head_temp(IORD_16DIRECT(AMC7891_0_BASE,AMC7891_TEMP_DATA) & 0x3ff);

	// Head Thermistor: NCP03XH103F25RL R0=10k 1% @ T0=25°C
	// B-Constant 25- 50°C 						[K] = 3380+-0.7%
	// B-Constant (Reference value) 25- 80°C 	[K] = 3428
	// B-Constant (Reference value) 25- 85°C 	[K] = 3434
	// B-Constant (Reference value) 25-100°C 	[K] = 3455
	// ADC Update all 16us

	UINT32 val=IORD_16DIRECT(AMC7891_0_BASE,AMC7891_TEMP_DATA)&0x3ff;
	if (val==0x3ff) pRX_Status->headcon_amc_temp = INVALID_VALUE;
	else            pRX_Status->headcon_amc_temp = 1000*val/8;
}

/*
 * This is called every 10 ms ?
 */
static void _temp_savety(void)
{
	static const INT32 MAX_DIE = 80000;

	if (pRX_Status->headcon_amc_temp > MAX_DIE)
		pRX_Status->error.head_pcb_overheated = TRUE;
}

//--- _do_1000ms_timer -------------------------------------------
static void _do_1000ms_timer(void)
{
//	trprintf("TIMER: %d %d %d %d\n", Timer_Tick, Timer_10ms, Timer_500ms, Timer_1000ms);
	main_tick_1000ms();

	// each 250ms new values, mean over 4 values, write out each 1s

	IOWR_ALTERA_AVALON_PIO_DATA(PIO_NIOS_LED_BASE, ~IORD_ALTERA_AVALON_PIO_DATA(PIO_NIOS_LED_BASE));

	//---  command exe_valid -------------------------------
	if (pRX_Config->cmd.exe_valid)
	{
		pRX_Config->cmd.exe_valid = FALSE;
		memset(&pRX_Status->cond, 0, sizeof(pRX_Status->cond));
		bootloader_start();
	}

	//---  command error_reset -------------------------------
	if(pRX_Config->cmd.error_reset)
	{
		main_error_reset();
		pRX_Config->cmd.error_reset = FALSE;
	}

	//--- user eeprom ----------------------------------------
	if (pRX_Config->cmd.write_user_eeprom)
	{
		int head;
		for (head=0; head<SIZEOF(pRX_Config->user_eeprom); head++)
		{
			head_eeprom_change_user_data(head, pRX_Status->user_eeprom[head], pRX_Config->user_eeprom[head], sizeof(pRX_Config->user_eeprom[head]), 0);
		}
		pRX_Config->cmd.write_user_eeprom = FALSE;
	}
}

//--- handle_timer_0_interrupt ----------------------------------------
void handle_timer_0_interrupt(void* context) //every 1ms, high priority
{
	alt_u32 		tmp;

	Timer_Tick += 10;
	Timer_10_in++;

	// Clear the TO bit in the status register (read-modify-write)
    tmp 	= IORD_ALTERA_AVALON_TIMER_STATUS(CPU_TIMER_0_BASE);
    tmp 	&= ~ALTERA_AVALON_TIMER_STATUS_TO_MSK;
    IOWR_ALTERA_AVALON_TIMER_STATUS(CPU_TIMER_0_BASE, tmp);
}

//--- handle_timer_1_interrupt ------------------------------------------------
static void handle_timer_1_interrupt(void* context) //every 250ms, middle priority
{
	alt_u32			tmp;
	Timer_250_in++;

	// Clear the TO bit in the status register (read-modify-write)
    tmp 	= IORD_ALTERA_AVALON_TIMER_STATUS(CPU_TIMER_1_BASE);
    tmp 	&= ~ALTERA_AVALON_TIMER_STATUS_TO_MSK;
    IOWR_ALTERA_AVALON_TIMER_STATUS(CPU_TIMER_1_BASE, tmp);
}

//--- handle_timer_2_interrupt -------------------------------------------
static void handle_timer_2_interrupt(void* context)	//every 1s, low priority
{
	alt_u32 		tmp;

	Timer_1000_in++;
	pRX_Status->alive++;

    // Clear the TO bit in the status register (read-modify-write)
    tmp 	= IORD_ALTERA_AVALON_TIMER_STATUS(CPU_TIMER_2_BASE);
    tmp 	&= ~ALTERA_AVALON_TIMER_STATUS_TO_MSK;
    IOWR_ALTERA_AVALON_TIMER_STATUS(CPU_TIMER_2_BASE, tmp);
}

// Init the timers

//--- timer_init ------------------------------------------------------------------------
int timer_init(void)
{
	static alt_u32 period_timer_0 = ALT_CPU_CPU_FREQ/100;    	// 100 Hz = 10ms -> 20n ns * 500k	//priority 5
    static alt_u32 period_timer_1 = ALT_CPU_CPU_FREQ/4;    		// 4 Hz   = 250ms ->20n ns * 25M	//priority 6
    static alt_u32 period_timer_2 = ALT_CPU_CPU_FREQ;    		// 1 Hz   = 1s   -> 20n ns * 50M	//priority 7
    //static alt_u32 period_timer_2 = ALT_CPU_CPU_FREQ/15;    	// 3.3 Hz = 300ms-> 20n ns * 15M
    //static alt_u32 period_timer_2 = ALT_CPU_CPU_FREQ;    		// 1 Hz   = 1s   -> 20n ns * 50M
    //static alt_u32 period_timer_2 = ALT_CPU_CPU_FREQ/10;    	// 10 Hz  = 100ms-> 20n ns * 5M
    //static alt_u32 period_timer_2 = ALT_CPU_CPU_FREQ/100;    	// 100 Hz = 10ms -> 20n ns * 500k

    // Set Timer Period Register
    IOWR_ALTERA_AVALON_TIMER_PERIODH(CPU_TIMER_0_BASE, period_timer_0 >> 16);
    IOWR_ALTERA_AVALON_TIMER_PERIODL(CPU_TIMER_0_BASE, period_timer_0 & 0xffff);

    IOWR_ALTERA_AVALON_TIMER_PERIODH(CPU_TIMER_1_BASE, period_timer_1 >> 16);
    IOWR_ALTERA_AVALON_TIMER_PERIODL(CPU_TIMER_1_BASE, period_timer_1 & 0xffff);

    IOWR_ALTERA_AVALON_TIMER_PERIODH(CPU_TIMER_2_BASE, period_timer_2 >> 16);
    IOWR_ALTERA_AVALON_TIMER_PERIODL(CPU_TIMER_2_BASE, period_timer_2 & 0xffff);

    // Set Timer Control Register
    IOWR_ALTERA_AVALON_TIMER_CONTROL(CPU_TIMER_0_BASE, ALTERA_AVALON_TIMER_CONTROL_CONT_MSK |
                                                      ALTERA_AVALON_TIMER_CONTROL_START_MSK |
                                                      ALTERA_AVALON_TIMER_CONTROL_ITO_MSK );

    IOWR_ALTERA_AVALON_TIMER_CONTROL(CPU_TIMER_1_BASE, ALTERA_AVALON_TIMER_CONTROL_CONT_MSK |
                                                      ALTERA_AVALON_TIMER_CONTROL_START_MSK |
                                                      ALTERA_AVALON_TIMER_CONTROL_ITO_MSK );

    IOWR_ALTERA_AVALON_TIMER_CONTROL(CPU_TIMER_2_BASE, ALTERA_AVALON_TIMER_CONTROL_CONT_MSK |
                                                      ALTERA_AVALON_TIMER_CONTROL_START_MSK |
                                                      ALTERA_AVALON_TIMER_CONTROL_ITO_MSK );

    // Register ISR
    alt_irq_register(CPU_TIMER_0_IRQ, NULL, handle_timer_0_interrupt);	//priority 5
    alt_irq_register(CPU_TIMER_1_IRQ, NULL, handle_timer_1_interrupt);	//priority 6
    alt_irq_register(CPU_TIMER_2_IRQ, NULL, handle_timer_2_interrupt);	//priority 7

	return 0;
}

