
#include "ft900.h"
#include "ft900_memctl.h"
#include "ft900_wdt.h"

#include "rx_types.h"
#include "network.h"
#include "rx_timer.h"


// Prototypes
void watchdog_ISR(void);

#ifndef DEBUG
#define WTD_ENABLE
#endif


int main(void)
{
	uint32_t tick = 0;
	uint32_t tick_old = 0;

	if(rx_timer_init() != REPLY_OK)
		return REPLY_ERROR;

	if (network_init() != REPLY_OK)
		chip_reboot();


#ifdef WTD_ENABLE
	if(interrupt_attach(interrupt_wdg, 17, watchdog_ISR) != REPLY_OK) return REPLY_ERROR;
	sys_enable(sys_device_timer_wdt);
	wdt_init(wdt_counter_1G_clocks);
#else
//	sys_disable(sys_device_timer_wdt);
#endif
	interrupt_enable_globally();

	for(;;)
	{
		tick = rx_get_ticks();

		if(tick > tick_old)
		{
			network_tick(tick);
			rx_boot_main(tick);
			tick_old = tick;
		}
	}
}


void watchdog_ISR(void)
{
	chip_reboot();
}
