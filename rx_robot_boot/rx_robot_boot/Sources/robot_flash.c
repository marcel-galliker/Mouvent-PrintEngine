#include <stdlib.h>
#include <ft900.h>

#include "rx_trace.h"
#include "robot_flash.h"

typedef union SUserData
{
	UINT8	data[FLASH_SECTOR_SIZE];
	struct
	{
		UINT16	serialNo;
		UINT16	serialNoChk;
	};
} SUserData;

static SUserData _UserData;

//--- flash_init ---------------------
void flash_init(void)
{
	memcpy_flash2dat(&_UserData, FLASH_SECTOR_USER*FLASH_SECTOR_SIZE, FLASH_SECTOR_SIZE);
	/*
	if (!flash_serialNo_Valid())
	{
		_UserData.serialNo = 0;
	}
	*/
}

//--- flash_serialNo_Valid -------------------------------
int	flash_serialNo_Valid(void)
{
	return _UserData.serialNo == ~_UserData.serialNoChk;
}

//--- flash_read_serialNo ----------------------------
UINT16 	flash_read_serialNo(void)
{
	return _UserData.serialNo;
}

//--- flash_write_serialNo ----------------------------
void 	flash_write_serialNo(UINT16 serialNo)
{
	_UserData.serialNo    = serialNo;
	_UserData.serialNoChk = ~serialNo;
	flash_sector_erase(FLASH_SECTOR_USER);
	TrPrintf(TRUE, "Writing serialNo=%d to sector=%d", _UserData.serialNo, FLASH_SECTOR_USER);
	memcpy_dat2flash(FLASH_SECTOR_USER*FLASH_SECTOR_SIZE, &_UserData, FLASH_SECTOR_SIZE);
	memcpy_flash2dat(&_UserData, FLASH_SECTOR_USER*FLASH_SECTOR_SIZE, FLASH_SECTOR_SIZE);
	TrPrintf(TRUE, "Verify serialNo=%d to sector=%d", _UserData.serialNo, FLASH_SECTOR_USER);
}

