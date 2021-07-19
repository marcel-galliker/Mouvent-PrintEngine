#ifndef INCLUDES_GPIO_H_
#define INCLUDES_GPIO_H_

#include <stdint.h>
#include <stdbool.h>


bool gpio_init(void);
void gpio_tick(int tick);
bool gpio_handle_msg(void* message);
void gpio_tick(int tick);
void gpio_set_output(uint8_t output, bool value);
bool gpio_get_input(uint8_t input);
void gpio_enable_motor(uint8_t motor, int enable);
void gpio_start_motor(uint8_t motor);
void gpio_stop_motor(uint8_t motor);

#endif /* INCLUDES_GPIO_H_ */
