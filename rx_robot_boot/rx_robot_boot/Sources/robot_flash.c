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
		ip_addr_t	ipAddr;
		UINT16		ipAddrChk;
	};
} SUserData;

static SUserData _UserData;
static bool		 _Initialized=FALSE;

//--- flash_init ---------------------
void flash_init(void)
{
	if (!_Initialized)
	{
		memcpy_flash2dat(&_UserData, FLASH_SECTOR_USER*FLASH_SECTOR_SIZE, FLASH_SECTOR_SIZE);
		/*
		if (!flash_serialNo_Valid())
		{
			_UserData.serialNo = 0;
		}
		*/
		_Initialized = TRUE;
	}
}

//--- _flash_write ----------------------------
void static _flash_write(void)
{
	flash_sector_erase(FLASH_SECTOR_USER);
	TrPrintf(TRUE, "Writing serialNo=%d to sector=%d", _UserData.serialNo, FLASH_SECTOR_USER);
	memcpy_dat2flash(FLASH_SECTOR_USER*FLASH_SECTOR_SIZE, &_UserData, FLASH_SECTOR_SIZE);
	memcpy_flash2dat(&_UserData, FLASH_SECTOR_USER*FLASH_SECTOR_SIZE, FLASH_SECTOR_SIZE);
	TrPrintf(TRUE, "Verify serialNo=%d to sector=%d", _UserData.serialNo, FLASH_SECTOR_USER);
}

//--- _chk ----------------------
static UINT16 _chk(void *pdata, int len)
{
	UINT8* pd = (UINT8*)pdata;
	UINT16 chk=0;
	for (int i=1; i<=len; i++, pd++)
	{
		chk = (chk<<3) | ((*pd)*i);
	}
	return chk;
}

//--- flash_serialNo_Valid -------------------------------
int	flash_serialNo_Valid(void)
{
	return _UserData.serialNo == ~_UserData.serialNoChk;
}

//--- flash_read_serialNo ----------------------------
UINT16 	flash_read_serialNo(void)
{
	flash_init();
	return _UserData.serialNo;
}

//--- flash_write_serialNo ----------------------------
void 	flash_write_serialNo(UINT16 serialNo)
{
	flash_init();
	_UserData.serialNo    = serialNo;
	_UserData.serialNoChk = ~serialNo;
	_flash_write();
}

//--- flash_read_ipAddr -----------------------------------
void flash_read_ipAddr(ip_addr_t *pipAddr, ip_addr_t ipAddrDefault)
{
	flash_init();
	if (_chk(&_UserData.ipAddr, sizeof(ip_addr_t)) == _UserData.ipAddrChk)
		memcpy(pipAddr, &_UserData.ipAddr, sizeof(ip_addr_t));
	else
		memcpy(pipAddr, &ipAddrDefault, sizeof(ip_addr_t));
}

//--- flash_write_ipAddr --------------------------------
void	flash_write_ipAddr(ip_addr_t *pipAddr)
{
	flash_init();
	memcpy(&_UserData.ipAddr, pipAddr, sizeof(ip_addr_t));
	_UserData.ipAddrChk = _chk(&_UserData.ipAddr, sizeof(ip_addr_t));
	_flash_write();
}

