#include "pumptime.h"
#include "eeprom.h"
#include "i2c_bitbang.h"
#include "work_flash_operation.h"

// --- save_pumptime -----------------------------
void save_pumptime(UINT32 pumptime, int write_flash)
{
    eeprom_write_setting32(EE_ADDR_PUMPTIME, pumptime);
}

// --- load_pumptime -----------------------------
UINT32 load_pumptime(void)
{
	INT32 time;
    if (eeprom_read_setting32(EE_ADDR_PUMPTIME, &time)==0) return time;
	else return 0;
}

// --- reset_pumptime -----------------------------
void reset_pumptime(void)
{	
	if (load_pumptime()) eeprom_write_setting32(EE_ADDR_PUMPTIME, 0);
    RX_Status.pumptime = 0;
}
