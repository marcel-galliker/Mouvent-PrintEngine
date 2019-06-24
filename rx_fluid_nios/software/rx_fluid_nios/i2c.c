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
#include "i2c_master.h"
#include "i2c.h"

void init_I2C(alt_u32 base)
{
	I2C_init(base,ALT_CPU_FREQ,100000);
	I2C_init(base,ALT_CPU_FREQ,100000);
}

