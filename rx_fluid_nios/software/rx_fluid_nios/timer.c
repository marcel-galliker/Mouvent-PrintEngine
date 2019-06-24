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
#include "timer.h"
#include "AMC7891.h"
#include "MAX31865.h"
#include "log.h"
#include "pid.h"
#include "ink_ctrl.h"
#include <limits.h>
#include "trprintf.h"
#include "pio.h"
#include "i2c_master_regs.h"
#include "i2c_master.h"
#include "main.h"
#include "pres.h"

#include "nios_def_fluid.h"

#include "heater.h"

//--- defines ------------------------------------------------------

//--- statics ----------------------------------------------------

static int _Alive[NIOS_INK_SUPPLY_CNT];

static int		Timer_10_in=0;
static int		Timer_10_out=0;

static int		Timer_1000_in=0;
static int		Timer_1000_out=0;

//--- prototypes ---------------------------------------------
int alt_irq_register (alt_u32 id, void*   context, alt_isr_func handler);	// external defined where?

static void _do_10ms_timer(void);
static void _do_1000ms_timer(void);


//--- timer_main -------------------------
void timer_main(void)
{
	while (Timer_10_out  !=Timer_10_in) 	{ _do_10ms_timer();  Timer_10_out++;}
	while (Timer_1000_out!=Timer_1000_in)   { _do_1000ms_timer();Timer_1000_out++;}
}

//--- _do_10ms_timer ----------------------------------
static void _do_10ms_timer(void)
{
	pres_tick_10ms();
	ink_tick_10ms();
	log_tick_10ms();
	heater_tick_10ms();

	//IOWR_ALTERA_AVALON_PIO_DATA(PIO_WATCHDOG_CNT_BASE,0x0FFFFFFF);			// 5 sec
	//IOWR_ALTERA_AVALON_PIO_DATA(PIO_WATCHDOG_CNT_BASE,0x4FFFFFFF);			// 5 sec
}

//--- _do_1000ms_timer ------------------------------------------
static void _do_1000ms_timer(void)
{
	alt_u16				val;
	int					count;

	ink_tick_1000ms();

	heater_tick_1000ms();

	if(pRX_Config->cmd.shutdown)
	{
		// todo alles ausschalten

		pRX_Config->cmd.shutdown 		 = 0;
		pRX_Status->info.is_shutdown	 = 1;
	}

	if(pRX_Config->cmd.error_reset)
	{
		main_error_reset();
		pRX_Config->cmd.error_reset = FALSE;
	}

	// MAX31865 Status
	val = IORD_8DIRECT(AVALON_SPI_MAX31865_0_BASE, MAX31865_CONFIG);		// Configuration

	// MAX31865 Fault Status
	val = IORD_8DIRECT(AVALON_SPI_MAX31865_0_BASE, MAX31865_FAULT_STATUS);		// Fault Status

	//--- check conditioner connected -------------------------------------
	{
		int i;
		for (i=0; i<NIOS_INK_SUPPLY_CNT; i++)
		{
			if (pRX_Config->ink_supply[i].alive == _Alive[i])
			{
				pRX_Config->ink_supply[i].condPresIn = INVALID_VALUE;
			}
			else _Alive[i] = pRX_Config->ink_supply[i].alive;
		}
	}

	// Read PT100 Temperature
	for(count = 0 ; count < 6 ; count++)
	{
		//IOWR_8DIRECT(AVALON_SPI_MAX31865_0_BASE + count*0x10, MAX31865_CONFIG, 0xA1); // single shot
		val = (IORD_8DIRECT(AVALON_SPI_MAX31865_0_BASE + count*0x10,MAX31865_RTD_DATA_MSB) <<8);	// Read Resistance MSB
		val += IORD_8DIRECT(AVALON_SPI_MAX31865_0_BASE + count*0x10,MAX31865_RTD_DATA_LSB);		// Read Resistance LSB

		if((val&0x01)==0) //temp is valid
		{
			val =  (val>>6); // shift 1 bit for valid bit and 5 bit for conversion to Celsius
			val =  val-256; // Temp in Celsius
		}
		else
		{
			val=0;
			IOWR_8DIRECT(AVALON_SPI_MAX31865_0_BASE + count*0x10, MAX31865_CONFIG, 0xC3); // Reset fault
		}
		pRX_Status->pt100[count]=val;
	}

	// Report to Marcel
	pRX_Status->input		= (alt_u32)(~(IORD_ALTERA_AVALON_PIO_DATA(PIO_INPUT_BASE) >> 8) & 0x0f);

	pRX_Status->alive++;

	if(pRX_Status->alive & 0x01)	IOWR_ALTERA_AVALON_PIO_SET_BITS  (PIO_OUTPUT_BASE, FPGA_LED_1_OUT);
	else							IOWR_ALTERA_AVALON_PIO_CLEAR_BITS(PIO_OUTPUT_BASE, FPGA_LED_1_OUT);

	trprintf("Alive %d, timer 10ms=%d\n", pRX_Status->alive, Timer_10_in);
}

//--- TIMER 0 -> 10ms -----------------------------------------------------
static void handle_timer_0_interrupt(void* context)
{
	alt_u32 				tmp;

	Timer_10_in++;

    // Clear the TO bit in the status register (read-modify-write)
    tmp 	= IORD_ALTERA_AVALON_TIMER_STATUS(TIMER_0_BASE);
    tmp 	&= ~ALTERA_AVALON_TIMER_STATUS_TO_MSK;
    IOWR_ALTERA_AVALON_TIMER_STATUS(TIMER_0_BASE, tmp);
}

// TIMER 1 -> 1s
static void handle_timer_1_interrupt(void* context)
{
	alt_u32 			tmp;

	Timer_1000_in++;

    // Clear the TO bit in the status register (read-modify-write)
    tmp 	= IORD_ALTERA_AVALON_TIMER_STATUS(TIMER_1_BASE);
    tmp 	&= ~ALTERA_AVALON_TIMER_STATUS_TO_MSK;
    IOWR_ALTERA_AVALON_TIMER_STATUS(TIMER_1_BASE, tmp);
}

//--- timer_init -----------------------------------
int timer_init(void)
{
    memset(_Alive, 0, sizeof(_Alive));

    int ret = -1;
    static alt_u32 period_timer_0 = ALT_CPU_CPU_FREQ/100;    	// 100 Hz = 10ms 	-> 20n ns * 50k
    static alt_u32 period_timer_1 = ALT_CPU_CPU_FREQ;    		// 1s -> 20n ns


    // TIMER 0

    // Set Timer Period Register
    IOWR_ALTERA_AVALON_TIMER_PERIODH(TIMER_0_BASE, period_timer_0 >> 16);
    IOWR_ALTERA_AVALON_TIMER_PERIODL(TIMER_0_BASE, period_timer_0 & 0xffff);

    // Set Timer Control Register
    IOWR_ALTERA_AVALON_TIMER_CONTROL(TIMER_0_BASE, ALTERA_AVALON_TIMER_CONTROL_CONT_MSK |
                                                      ALTERA_AVALON_TIMER_CONTROL_START_MSK |
                                                      ALTERA_AVALON_TIMER_CONTROL_ITO_MSK );

    // Register ISR
    alt_irq_register(TIMER_0_IRQ, NULL, handle_timer_0_interrupt);

    // TIMER 1

    // Set Timer Period Register
    IOWR_ALTERA_AVALON_TIMER_PERIODH(TIMER_1_BASE, period_timer_1 >> 16);
    IOWR_ALTERA_AVALON_TIMER_PERIODL(TIMER_1_BASE, period_timer_1 & 0xffff);

    // Set Timer Control Register
    IOWR_ALTERA_AVALON_TIMER_CONTROL(TIMER_1_BASE, ALTERA_AVALON_TIMER_CONTROL_CONT_MSK |
                                                      ALTERA_AVALON_TIMER_CONTROL_START_MSK |
                                                      ALTERA_AVALON_TIMER_CONTROL_ITO_MSK );

    // Register ISR
    alt_irq_register(TIMER_1_IRQ, NULL, handle_timer_1_interrupt);

	return (ret);
}
