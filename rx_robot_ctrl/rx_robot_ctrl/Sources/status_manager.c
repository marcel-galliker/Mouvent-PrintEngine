#include "status_manager.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <ft900.h>

#include "rx_robot_tcpip.h"
#include "network_manager.h"


/* Defines */

/* Static variables */

SRobotStatusMsg RX_RobotStatus;


/* Prototypes */

bool status_manager_start(void)
{
	RX_RobotStatus.header.msgId  = CMD_STATUS_GET;
	RX_RobotStatus.header.msgLen = sizeof(RX_RobotStatus);
	return true;
}

//--- status_handle_message --------------------------------------------------
void status_handle_message(void* message)
{
	SMsgHdr* header = (SMsgHdr*)message;

	switch(header->msgId)
	{
	case CMD_STATUS_GET:	status_manager_send_status();	break;
	default:	break;
	}
}

//--- status_manager_send_status ------------------------------------
void status_manager_send_status(void)
{
	RX_RobotStatus.alive++;

	network_manager_send(&RX_RobotStatus, sizeof(RX_RobotStatus));
}

