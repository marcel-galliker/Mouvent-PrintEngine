#include <type_defs.h>
#include "densio.h"
#include <atmel_start.h>
#include "component\pio.h"
#include "systick.h"
#include "sa_tcp_ip.h"

static SDensioStatus _densioStatus;
static INT32 _timeout;

void densio_tick(void)
{
	if((_timeout != 0) && (_timeout < sys_now()))
	{
		gpio_set_pin_level(TriggerInput, 0);
		_timeout = 0;
	}
}

SDensioStatus* densio_get_densio_status(void)
{
	
	_densioStatus.discretSignalOutputFlag = gpio_get_pin_level(DiscreteSignalOutput); // DiscreteSignalOutput
	
	return &_densioStatus;
}

void densio_trigger(SetupAssist_OutTriggerCmd* triggerCommand)
{
	_timeout = sys_now() + triggerCommand->time_ms;
	gpio_set_pin_level(TriggerInput, 1);
}