#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "sys/types.h"

#include "ft900.h"

#include "gpio.h"
#include "robot_flash.h"
#include "network.h"
#include "rx_boot.h"
#include "rx_trace.h"
#include "rx_timer.h"
#include "motor.h"

int main(void)
{
	rx_timer_init();
	network_init();
	gpio_init();
	motor_init();

#ifdef WTD_ENABLE
	if(interrupt_attach(interrupt_wdg, 17, watchdog_ISR) != REPLY_OK) return REPLY_ERROR;
	sys_enable(sys_device_timer_wdt);
	wdt_init(wdt_counter_1G_clocks);
#else
//	sys_disable(sys_device_timer_wdt);
#endif
	interrupt_enable_globally();

	TrEnable(TRUE);
	uint32_t tick = 0;
	uint32_t tick_old = 0;
	for(;;)
	{
		tick = rx_get_ticks();

		if(tick > tick_old)
		{
			network_tick(tick);
			rx_boot_main(tick);
			motor_tick(tick);
			gpio_tick(tick);
			tick_old = tick;
		}
	}
}
