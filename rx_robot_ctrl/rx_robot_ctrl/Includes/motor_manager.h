#ifndef INCLUDES_MOTOR_MANAGER_H_
#define INCLUDES_MOTOR_MANAGER_H_

#include <stdint.h>
#include <stdbool.h>

#include "communication_def.h"

bool motor_manager_start(void);
bool motor_manager_is_initalized(void);
MotorStatus_t* motor_manager_get_status(void);


#endif /* INCLUDES_MOTOR_MANAGER_H_ */
