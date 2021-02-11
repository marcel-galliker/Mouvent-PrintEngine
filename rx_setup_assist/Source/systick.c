#include "systick.h"
#include "atmel_start.h"

static struct timer_task _systickTask;
static UINT32 _ticks;

static void systick_task_cb(const struct timer_task *const timer_task);

void systick_init(void)
{
	_systickTask.interval = 1;
	_systickTask.cb       = systick_task_cb;
	_systickTask.mode     = TIMER_TASK_REPEAT;

	timer_add_task(&TIMER_0, &_systickTask);
	timer_start(&TIMER_0);
}

UINT32 sys_now(void)
{
	return _ticks;
}

static void systick_task_cb(const struct timer_task *const timer_task)
{
	_ticks++;
}