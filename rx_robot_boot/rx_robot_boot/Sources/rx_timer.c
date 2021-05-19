#include <ft900.h>
#include "rx_types.h"
#include "rx_timer.h"

#define TIMER_PRESCALER 		(200)
#define TIMER_ONE_MILLISECOND 	(500)
#define TIMER_ONE_SECOND 		(1000 * TIMER_ONE_MILLISECOND)

#define SYSTICK_INTERVAL	1

// #define USB_TIMER_VAL		(5 * TIMER_ONE_MILLISECOND)
// #define USB_TICK_VAL			(1 * TIMER_ONE_MILLISECOND)
#define SYSTICK_TIMER_VAL		(SYSTICK_INTERVAL * TIMER_ONE_MILLISECOND)
// #define RUNTICK_TIMER_VAL		(20 * TIMER_ONE_MILLISECOND)

void timer_interrupt(void);

static uint32_t _tickts_ms=0;

int32_t rx_timer_init(void)
{
	if(sys_enable(sys_device_timer_wdt) != REPLY_OK) return REPLY_ERROR;

#if defined(__FT900__)
	if (sys_check_ft900_revB())
	{
		timer_prescaler(TIMER_PRESCALER);
	}
	else
#endif
	{
		timer_prescaler(timer_select_a, TIMER_PRESCALER);
		timer_prescaler(timer_select_b, TIMER_PRESCALER);
		timer_prescaler(timer_select_c, TIMER_PRESCALER);
		timer_prescaler(timer_select_d, TIMER_PRESCALER);
	}

	if(timer_init(timer_select_a, SYSTICK_TIMER_VAL, timer_direction_down, timer_prescaler_select_on, timer_mode_continuous) != REPLY_OK) return REPLY_ERROR;
//	if(timer_init(timer_select_b, USB_TIMER_VAL, timer_direction_down, timer_prescaler_select_on, timer_mode_continuous) != REPLY_OK) return REPLY_ERROR;
//	if(timer_init(timer_select_c, RUNTICK_TIMER_VAL, timer_direction_down, timer_prescaler_select_on, timer_mode_continuous) != REPLY_OK) return REPLY_ERROR;
//	if(timer_init(timer_select_d, USB_TICK_VAL, timer_direction_down, timer_prescaler_select_on, timer_mode_continuous) != REPLY_OK) return REPLY_ERROR;

	if(interrupt_attach(interrupt_timers, 17, timer_interrupt) != REPLY_OK) return REPLY_ERROR;

	if(timer_enable_interrupt(timer_select_a) != REPLY_OK) return REPLY_ERROR;
//	if(timer_enable_interrupt(timer_select_b) != REPLY_OK) return REPLY_ERROR;
//	if(timer_enable_interrupt(timer_select_c) != REPLY_OK) return REPLY_ERROR;
//	if(timer_enable_interrupt(timer_select_d) != REPLY_OK) return REPLY_ERROR;

	if(timer_start(timer_select_a) != REPLY_OK) return REPLY_ERROR;
//	if(timer_start(timer_select_b) != REPLY_OK) return REPLY_ERROR;
//	if(timer_start(timer_select_c) != REPLY_OK) return REPLY_ERROR;
//	if(timer_start(timer_select_d) != REPLY_OK) return REPLY_ERROR;

	return REPLY_OK;
}

// used for LWIP
uint32_t millis(void)
{
	return _tickts_ms;
}

uint32_t rx_get_ticks(void)
{
	return _tickts_ms;
}

void timer_interrupt(void)
{
	if(timer_is_interrupted(timer_select_a) == 1)
	{
		_tickts_ms++;
	}

	/*
	if(timer_is_interrupted(timer_select_b) == 1)
	{
		_run_usb();
	}
	*/

	/*
	if(timer_is_interrupted(timer_select_c) == 1)
	{
		_runtick++;
	}
	*/

	/*
	if(timer_is_interrupted(timer_select_d) == 1)
	{
		USBH_timer();
	}
	*/
}
