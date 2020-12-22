#include <atmel_start.h>
#include "timer_3.h"
#include "power_step.h"
#include "wd.h"

static struct timer_task TIMER_3_task1, TIMER_3_task2;

void timer_3_init(void)
{
	TIMER_3_task1.interval = 100;
	TIMER_3_task1.cb       = ps_timer_cb;
	TIMER_3_task1.mode     = TIMER_TASK_REPEAT;
	
	TIMER_3_task2.interval = 10;
	TIMER_3_task2.cb       = wd_timer_cb;
	TIMER_3_task2.mode     = TIMER_TASK_REPEAT;

	timer_add_task(&TIMER_3, &TIMER_3_task1);
	timer_add_task(&TIMER_3, &TIMER_3_task2);
	timer_start(&TIMER_3);
}