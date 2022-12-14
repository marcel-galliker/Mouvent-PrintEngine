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

static void _temp_savety_100ms(void);

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

//--- _send_cond_data_100ms ----------------------
static void _send_cond_data_100ms(void)
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
		_send_cond_data_100ms();
		pres_tick_100ms();
		cooler_tick_100ms();
		power_tick_100ms();
		_temp_savety_100ms();
	}
}

//--- _measure_head_temp ---------------------------------------
static void _measure_head_temp(void)
{
	volatile UINT16 dummy;
	int i;
	int sum, cnt;

	/* First read does start AD conversion, result is only available at second read.
	 * That's why the code seems one off!
	 */
	dummy = IORD_16DIRECT(AMC7891_0_BASE,AMC7891_ADC4_DATA); // start conversion
	dummy++; // to ignore warning
	pRX_Status->head_temp_org[0] = convert_head_temp(IORD_16DIRECT(AMC7891_0_BASE,AMC7891_ADC5_DATA) & 0x3ff);
	pRX_Status->head_temp_org[1] = convert_head_temp(IORD_16DIRECT(AMC7891_0_BASE,AMC7891_ADC6_DATA) & 0x3ff);
	pRX_Status->head_temp_org[2] = convert_head_temp(IORD_16DIRECT(AMC7891_0_BASE,AMC7891_ADC7_DATA) & 0x3ff);
	pRX_Status->head_temp_org[3] = convert_head_temp(IORD_16DIRECT(AMC7891_0_BASE,AMC7891_TEMP_DATA) & 0x3ff);

	//--- compensate not working sensors ----------------
	for (i=0; i<MAX_HEADS_BOARD; i++)
	{
		if (pRX_Status->head_temp_org[i]==INVALID_VALUE)
		{
			sum=0; cnt=0;
			if (i>0 && pRX_Status->head_temp_org[i-1]!=INVALID_VALUE)
			{
				sum += pRX_Status->head_temp_org[i-1];
				cnt++;
			}
			if (i<MAX_HEADS_BOARD-1 && pRX_Status->head_temp_org[i+1]!=INVALID_VALUE)
			{
				sum += pRX_Status->head_temp_org[i+1];
				cnt++;
			}
			if (cnt) pRX_Config->cond[i].tempHead = sum/cnt;
			else pRX_Config->cond[i].tempHead=INVALID_VALUE;
		}
		else pRX_Config->cond[i].tempHead = pRX_Status->head_temp_org[i];
		pRX_Status->head_temp[i] = pRX_Config->cond[i].tempHead;
	}
}

//--- _do_250ms_timer -------------------------------------
static void _do_250ms_timer(void)
{
	_measure_head_temp();

	// Head Thermistor: NCP03XH103F25RL R0=10k 1% @ T0=25???C
	// B-Constant 25- 50???C 						[K] = 3380+-0.7%
	// B-Constant (Reference value) 25- 80???C 	[K] = 3428
	// B-Constant (Reference value) 25- 85???C 	[K] = 3434
	// B-Constant (Reference value) 25-100???C 	[K] = 3455
	// ADC Update all 16us

	UINT32 val=IORD_16DIRECT(AMC7891_0_BASE,AMC7891_TEMP_DATA)&0x3ff;
	if (val==0x3ff) pRX_Status->headcon_amc_temp = INVALID_VALUE;
	else            pRX_Status->headcon_amc_temp = 1000*val/8;

	//---  command exe_valid -------------------------------
	if (pRX_Config->cmd.exe_valid)
	{
		pRX_Config->cmd.exe_valid = FALSE;
		memset(&pRX_Status->cond, 0, sizeof(pRX_Status->cond));
		bootloader_start();
	}

	//--- user eeprom ----------------------------------------
	{
		int head;
		for (head=0; head<MAX_HEADS_BOARD; head++)
		{
			//--- read fuji -----------------------
			if (pRX_Status->eeprom_fuji_readCnt[head]!=pRX_Config->eeprom_fuji_readCnt[head])
			{
				head_eeprom_read(head, pRX_Status->eeprom_fuji[head], sizeof(pRX_Status->eeprom_fuji[head]));
				pRX_Status->eeprom_fuji_readCnt[head]=pRX_Config->eeprom_fuji_readCnt[head];
			}

			//--- read mvt -----------------------
			if (pRX_Status->eeprom_mvt_readCnt[head]!=pRX_Config->eeprom_mvt_readCnt[head])
			{
				head_eeprom_read_user_data(head, EEPROM_MVT_ADDR, (alt_u8*)&pRX_Status->eeprom_mvt[head], sizeof(pRX_Status->eeprom_mvt[head]));
				pRX_Status->eeprom_mvt_readCnt[head]=pRX_Config->eeprom_mvt_readCnt[head];
			}
			//--- write mvt -----------------------
			if (pRX_Config->eeprom_mvt_writeCnt[head]!=pRX_Status->eeprom_mvt_writeCnt[head])
			{
				if (head_eeprom_change_user_data(head, EEPROM_MVT_ADDR, (alt_u8*)&pRX_Status->eeprom_mvt[head], (alt_u8*)&pRX_Config->eeprom_mvt[head], sizeof(pRX_Config->eeprom_mvt[head]))==REPLY_OK)
					pRX_Status->eeprom_mvt_writeCnt[head]=pRX_Config->eeprom_mvt_writeCnt[head];
			}

			//--- read density -----------------------
			if (pRX_Status->eeprom_density_readCnt[head]!=pRX_Config->eeprom_density_readCnt[head])
			{
				head_eeprom_read_user_data(head, EEPROM_DENSITY_ADDR, (alt_u8*)&pRX_Status->eeprom_density[head], sizeof(pRX_Status->eeprom_density[head]));
				pRX_Status->eeprom_density_readCnt[head]=pRX_Config->eeprom_density_readCnt[head];
			}
			//--- write density -----------------------
			if (pRX_Config->eeprom_density_writeCnt[head]!=pRX_Status->eeprom_density_writeCnt[head])
			{
				pRX_Status->eeprom_density_writeRes[head]=head_eeprom_change_user_data(head, EEPROM_DENSITY_ADDR, (alt_u8*)&pRX_Status->eeprom_density[head], (alt_u8*)&pRX_Config->eeprom_density[head], sizeof(pRX_Config->eeprom_density[head]));
				pRX_Status->eeprom_density_writeCnt[head]=pRX_Config->eeprom_density_writeCnt[head];
			}
		}
	}
}

//--- _temp_savety_100ms ---------------
static void _temp_savety_100ms(void)
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

	//---  command error_reset -------------------------------
	if(pRX_Config->cmd.error_reset)
	{
		main_error_reset();
		pRX_Config->cmd.error_reset = FALSE;
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

