#include <atmel_start.h>
#include "component\pio.h"
#include "i2c.h"
#include "spi.h"
#include "timer_3.h"
#include "extIRQ.h"
#include "ethernet.h"
#include "systick.h"
#include "densio.h"

int main(void)
{
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();
	
	// powerstep01 reset
	gpio_set_pin_level(SM_STBY_RESET,0);
	gpio_set_pin_level(PHY_RESET,0);

	spi0_init();
	i2c0_init();		

	gpio_set_pin_level(SM_STBY_RESET,1);
	gpio_set_pin_level(PHY_RESET,1);
		
	timer_3_init();

	extIRQ_init();
	
	ethernet_init();
	
	systick_init();
	
	while (1) 
	{
		ethernet_tick();
		densio_tick();
	}
}

