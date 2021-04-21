#ifndef INCLUDES_STATUS_MANAGER_H_
#define INCLUDES_STATUS_MANAGER_H_

#include <stdint.h>
#include <stdbool.h>

#include "rx_robot_tcpip.h"

extern SRobotStatusMsg RX_RobotStatus;

bool status_manager_start(void);
void status_handle_message(void* message);
void status_manager_send_status(void);

#endif /* INCLUDES_STATUS_MANAGER_H_ */
