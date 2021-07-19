#include <bootloader.h>
#include "rx_robot_tcpip.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>

#include <ft900.h>

#include "rx_timer.h"
#include "network.h"
#include "robot_flash.h"
#include "rx_trace.h"
#include "main.h"

/* Defines */

// #define FLASH_TEST

#define DATA_TIMEOUT		100

/* Static variables */

// Status Flags
static UINT32		_FileSize=0;
static UINT32		_FilePos=0;
static UINT8  		_FlashBuf[FLASH_SECTOR_SIZE]={0};
static UINT32		_Timeout=0;
// static main_fct 	_AppStart=NULL;

/* Prototypes */

static void _download_get_status(void);
static void _download_start(SBootloaderStartCmd* command);
static void _download_data(SBootloaderDataCmd* command);
static void _download_reboot(void);
static void _request_data(UINT32 filePos);
static void _set_serialNo(SBootloaderSerialNoCmd *pmsg);

#ifdef FLASH_TEST
	static void _flash_test(int from, int to);
#endif

void bootloader_handle_msg(void* msg)
{
	SMsgHdr* header = (SMsgHdr*)msg;

	switch(header->msgId)
	{
	case CMD_STATUS_GET:			_download_get_status();						 break;
	case CMD_BOOTLOADER_START:		_download_start((SBootloaderStartCmd*)msg);	 break;
	case CMD_BOOTLOADER_DATA:		_download_data((SBootloaderDataCmd*)msg);	 break;
	case CMD_BOOTLOADER_REBOOT:		_download_reboot();						     break;
	case CMD_BOOTLOADER_SERIALNO:	_set_serialNo((SBootloaderSerialNoCmd*)msg); break;
	default:
		break;
	}
}

//--- bootloader_main --------------------------------
void bootloader_main(int tick)
{
	if (_FileSize && _Timeout && tick >_Timeout)
	{
		_request_data(_FilePos);
	}
}

#ifdef FLASH_TEST
static void _flash_test(int from, int to)
{
	UINT32 *addr = (UINT32*)_FlashBuf;
	TrPrintf(TRUE, "_flash_test (from=%d, to=%d", from, to);

	//--- write addresses --------------------
	for (int block=from; block<to; block++)
	{
		for (int i=0; i<1024; i++)
		{
			addr[i] = block*1024+i;
		}
		flash_sector_erase(block);
		memcpy_dat2flash(block*FLASH_SECTOR_SIZE, _FlashBuf, FLASH_SECTOR_SIZE);
	}

	//--- read addresses
	for (int block=from; block<to; block++)
	{
		memcpy_flash2dat(_FlashBuf, block*FLASH_SECTOR_SIZE, FLASH_SECTOR_SIZE);
		int val;
		int ok=TRUE;
		for (int i=0; i<1024; i++)
		{
			val=block*1024+i;
			if (addr[i]!=val)
			{
				ok=FALSE;
			//	TrPrintf(TRUE, "Error Data[%d]=%d", addr[i], (block*1024+i));
			}
		}
		if (ok) TrPrintf(TRUE, "Flash Block[%02d]: OK", block);
		else    TrPrintf(TRUE, "Flash Block[%02d]: ERROR", block);
	}
	TrPrintf(TRUE, "_flash_test done");
}
#endif


//--- _download_get_status -----------------
static void _download_get_status(void)
{
	SBootloaderStatusMsg cmd;
	memset(&cmd, 0, sizeof(cmd));
	cmd.header.msgId  = CMD_STATUS_GET;
	cmd.header.msgLen = sizeof(cmd);
	strcpy(cmd.version, "0.0.0.0");
	cmd.serialNo = flash_read_serialNo();
	network_send_ctrl_msg(&cmd, sizeof(cmd));
}

static void _download_start(SBootloaderStartCmd* cmd)
{
	TrPrintf(true, "Download Start");
	_FileSize=0;
	_FilePos=0;
	memset(_FlashBuf, 0, sizeof(_FlashBuf));

	if(cmd->size <= FLASH_SIZE)
	{
		_FileSize = cmd->size;

		#ifdef FLASH_TEST
		_flash_test(0, _FileSize/FLASH_SECTOR_SIZE+1);
	//	_flash_test(0, FLASH_SECTOR_CNT-1);
		#endif
		_request_data(_FilePos);
	}
}

//--- _request_data ------------------------------------
static void _request_data(UINT32 filePos)
{
	SBootloaderDataRequestCmd cmd;
	cmd.header.msgId = CMD_BOOTLOADER_DATA;
	cmd.header.msgLen= sizeof(cmd);
	cmd.filePos 	 = _FilePos;
	network_send_ctrl_msg(&cmd, sizeof(cmd));
	_Timeout = rx_get_ticks()+DATA_TIMEOUT;
}

//--- _download_data --------------------------------
static void _download_data(SBootloaderDataCmd* cmd)
{
	int sector;
	_Timeout = 0;
	if (_FileSize && cmd->filePos+cmd->length<FLASH_SIZE && cmd->filePos==_FilePos)
	{
		memcpy(&_FlashBuf[_FilePos%FLASH_SECTOR_SIZE], cmd->data, cmd->length);
		sector = _FilePos/FLASH_SECTOR_SIZE;
		_FilePos += cmd->length;
		if (_FilePos>=_FileSize || (_FilePos%FLASH_SECTOR_SIZE)==0)
		{
			sector += FLASH_SECTOR_APP;
			memcpy_dat2pm(sector*FLASH_SECTOR_SIZE, _FlashBuf, FLASH_SECTOR_SIZE);
			TrPrintf(true, "Flash sector %d Written", sector);
			memset(_FlashBuf, 0, sizeof(_FlashBuf));
			if (_FilePos>=_FileSize)
			{
				TrPrintf(true, "Download End");
				SMsgHdr cmd;
				cmd.msgId = CMD_BOOTLOADER_END;
				cmd.msgLen= sizeof(cmd);
				network_send_ctrl_msg(&cmd, sizeof(cmd));
				return;
			}
		}
		_request_data(_FilePos);
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
		start = (start_fct)(FLASH_SECTOR_APP*FLASH_SECTOR_SIZE+0x008c);
		start();
	}
}

//--- _set_serialNo -----------------
static void _set_serialNo(SBootloaderSerialNoCmd *pmsg)
{
	if (pmsg->serialNo!=flash_read_serialNo())
	{
		flash_write_serialNo(pmsg->serialNo);
		if (_FileSize==0) chip_reboot();
	}
}
