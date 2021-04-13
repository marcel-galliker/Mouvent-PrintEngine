#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "../../includes/rx_robot_def.h"

bool motor_manager_start(void);
bool motor_manager_is_initalized(void);
SMotorStatus* motor_manager_get_status(void);
void motor_handle_message(void* message);
void motor_main(void);
