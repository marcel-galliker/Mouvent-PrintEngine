#include "ctrl.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <ft900.h>

#include "rx_robot_tcpip.h"
#include "network.h"
#include "version.h"
#include "rx_trace.h"
#include "robot_flash.h"


SRobotStatusMsg RX_RobotStatus;

static void _set_serialNo(SBootloaderSerialNoCmd *pmsg);
static void _download_reboot(void);


//--- ctrl_handle_msg --------------------------------------------------
bool ctrl_handle_msg(void* message)
{
	SMsgHdr* header = (SMsgHdr*)message;

	switch(header->msgId)
	{
	case CMD_STATUS_GET:			ctrl_send_status();	return TRUE;
	case CMD_BOOTLOADER_SERIALNO:	_set_serialNo(message);	return TRUE;
	default:				break;
	}
	return FALSE;
}

//--- _set_serialNo -----------------
static void _set_serialNo(SBootloaderSerialNoCmd *pmsg)
{
	if (pmsg->serialNo!=flash_read_serialNo())
	{
		flash_write_serialNo(pmsg->serialNo);

	//	_download_reboot();
		chip_reboot();
	}
}

//--- _download_reboot --------------------------------------------
static void _download_reboot(void)
{
	interrupt_disable_globally();
	net_set_link_down();
	arch_ft900_recv_off();
	sys_disable(sys_device_timer_wdt);
	sys_disable(sys_device_ethernet);

	//--- starting downloaded binary ------------------
	//	__asm__ ("jmp 0x1908c"::);
	{
		typedef int(*start_fct)	(void);

		start_fct start;
#ifdef DEBUG
		start = (start_fct)(0x008c);
#else
		start = (start_fct)(FLASH_SECTOR_APP*FLASH_SECTOR_SIZE+0x008c);
#endif
		start();
	}
}


//--- ctrl_send_status ------------------------------------
void ctrl_send_status(void)
{
	if (!RX_RobotStatus.header.msgId)
	{
		RX_RobotStatus.header.msgId  = CMD_STATUS_GET;
		RX_RobotStatus.header.msgLen = sizeof(RX_RobotStatus);
		memcpy(RX_RobotStatus.version, version, sizeof(RX_RobotStatus.version));
	}
	RX_RobotStatus.serialNo = flash_read_serialNo();
	RX_RobotStatus.alive++;

	network_send_ctrl_msg(&RX_RobotStatus, sizeof(RX_RobotStatus));
}

