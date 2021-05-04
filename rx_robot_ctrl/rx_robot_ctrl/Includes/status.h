#ifndef INCLUDES_STATUS_H_
#define INCLUDES_STATUS_H_

#include <stdint.h>
#include <stdbool.h>

#include "rx_robot_tcpip.h"

extern SRobotStatusMsg RX_RobotStatus;

bool status_start(void);
void status_handle_message(void* message);
void status_send(void);

#endif /* INCLUDES_STATUS_H_ */
