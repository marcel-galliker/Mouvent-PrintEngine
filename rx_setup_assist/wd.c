#include "wd.h"
#include "atmel_start_pins.h"

void wd_timer_cb(const struct timer_task *const timer_task)
{
	static unsigned int toggle;

	toggle = !toggle;

	gpio_set_pin_level(Watchdog,toggle);
}