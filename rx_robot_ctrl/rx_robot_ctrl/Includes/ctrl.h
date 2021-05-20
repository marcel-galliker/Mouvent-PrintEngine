#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "rx_robot_tcpip.h"

extern SRobotStatusMsg RX_RobotStatus;

bool ctrl_handle_msg(void* message);
void ctrl_send_status(void);
