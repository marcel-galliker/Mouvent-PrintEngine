#include "bootloader.h"
#include "rx_robot_tcpip.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <ft900.h>
#include <status.h>
#include "FreeRTOS.h"
#include "task.h"

#include "network.h"
#include "robot_flash.h"
#include "rx_trace.h"

/* Defines */

// #define FLASH_TEST

// Task settings
#define TASK_BOOTLOADER_STACK_SIZE		(500)
#define TASK_BOOTLOADER_PRIORITY		(8)

#define DATA_TIMEOUT		100

/* Static variables */

// Status Flags
static bool _isInitialized = false;
static UINT32		_FileSize=0;
static UINT32		_FilePos=0;
static UINT8  		_FlashBuf[FLASH_SECTOR_SIZE]={0};
static TickType_t	_Timeout=0;

/* Prototypes */

static void _download_start(SBootloaderStartCmd* command);
static void _download_data(SBootloaderDataCmd* command);
static void _downlaod_reboot(void);
static void _request_data(UINT32 filePos);
static void _set_serialNo(SBootloaderSerialNoCmd *pmsg);

#ifdef FLASH_TEST
	static void _flash_test(int from, int to);
#endif

bool bootloader_start(void)
{
	_isInitialized = true;
	return true;
}

void bootloader_handle_command(void* msg)
{
	SMsgHdr* header = (SMsgHdr*)msg;

	switch(header->msgId)
	{
	case CMD_BOOTLOADER_START:		_download_start((SBootloaderStartCmd*)msg);	 break;
	case CMD_BOOTLOADER_DATA:		_download_data((SBootloaderDataCmd*)msg);	 break;
	case CMD_BOOTLOADER_REBOOT:		_downlaod_reboot();						     break;
	case CMD_BOOTLOADER_SERIALNO:	_set_serialNo((SBootloaderSerialNoCmd*)msg); break;
	default:
		break;
	}
}

//--- bootloader_main --------------------------------
void bootloader_main(void)
{
	if (_FileSize && _Timeout && xTaskGetTickCount()>_Timeout)
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

static void _request_data(UINT32 filePos)
{
	SBootloaderDataRequestCmd cmd;
	cmd.header.msgId = CMD_BOOTLOADER_DATA;
	cmd.header.msgLen= sizeof(cmd);
	cmd.filePos 	 = _FilePos;
	network_send(&cmd, sizeof(cmd));
	_Timeout = xTaskGetTickCount()+DATA_TIMEOUT;
}

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
			TrPrintf(true, "Write Flash block %d", sector);
			taskENTER_CRITICAL();
			flash_sector_erase(sector);
			memcpy_dat2flash(sector*FLASH_SECTOR_SIZE, _FlashBuf, FLASH_SECTOR_SIZE);
			memset(_FlashBuf, 0, sizeof(_FlashBuf));
			taskEXIT_CRITICAL();
			TrPrintf(true, "Flash block %d Written", sector);
			if (_FilePos>=_FileSize)
			{
				TrPrintf(true, "Download End");
				SMsgHdr cmd;
				cmd.msgId = CMD_BOOTLOADER_END;
				cmd.msgLen= sizeof(cmd);
				network_send(&cmd, sizeof(cmd));
				return;
			}
		}
		_request_data(_FilePos);
	}
}

//--- _downlaod_reboot --------------------------------------------
static void _downlaod_reboot(void)
{
	if (_FilePos>=_FileSize) chip_reboot();
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
