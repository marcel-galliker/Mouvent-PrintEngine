#include <atmel_start.h>
#include <type_defs.h>
#include "component\pio.h"
#include "extIRQ.h"
#include "sa_tcp_ip.h"
#include "ps_common.h"
#include "power_step.h"

static void initiator_1_irq(void)
{	
	volatile uint8_t cs_delay;

	SMotorStatus *motorStatus		= ps_get_motor_status();
	SMotorReference *motorReference = ps_get_motor_reference();

	if(gpio_get_port_level(GPIO_PORTA)& PIO_PER_P18_Msk)	// rising edge
	{
			gpio_set_pin_level(SM_EXT_INPUT,false);			
	}
	else													// falling edge   move on switch
	{
		if(motorReference->refSearch == true || ((motorStatus->powerStepStatus & POWERSTEP01_STATUS_DIR) == 0))
			gpio_set_pin_level(SM_EXT_INPUT,false);
	}

	if(motorReference->refSearch == true)
	{
		motorStatus->refDone		= true;
		motorReference->refSearch	= false;

		// Reset Position counter		
		reset_pos();
	}
	
	for(cs_delay = 0; cs_delay < 0x7f ; cs_delay++)
		;

	gpio_set_pin_level(SM_EXT_INPUT,true);
}

static void initiator_2_irq(void)	
{	
	volatile uint8_t cs_delay;

	if(gpio_get_port_level(GPIO_PORTA)& PIO_PER_P17_Msk)	// rising edge
	{
		gpio_set_pin_level(SM_EXT_INPUT,false);
		gpio_set_pin_level(LED,false);		
	}
	else													// falling edge
	{
		//gpio_set_pin_level(SM_EXT_INPUT,false);
		gpio_set_pin_level(LED,true);		
	}

	for(cs_delay = 0; cs_delay < 0x7f ; cs_delay++)
	;

	gpio_set_pin_level(SM_EXT_INPUT,true);
}


static void phy_irq(void)
{
}

void extIRQ_init(void)
{
	ext_irq_register(PIO_PA17_IDX, initiator_2_irq);
	ext_irq_register(PIO_PA18_IDX, initiator_1_irq);
	ext_irq_register(PIO_PA31_IDX, phy_irq);
}
