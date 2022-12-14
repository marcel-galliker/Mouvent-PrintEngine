#include "eeprom.h"
#include "cond_def_head.h"
#include "ctr.h"

static UINT32 _PumpTime=0;

// --- ctr_save -----------------------------
void ctr_save(void)
{
	if (RX_Config.serialNo && RX_Config.serialNo != RX_Status.serialNo) 
	{
		eeprom_write_setting32(EE_ADDR_SERIAL_NO, RX_Config.serialNo);
		RX_Status.serialNo = RX_Config.serialNo;
	}

	if (RX_Config.clusterNo && RX_Config.clusterNo != RX_Status.clusterNo) 
	{
		eeprom_write_setting32(EE_ADDR_CLUSTER_NO, RX_Config.clusterNo);
		RX_Status.clusterNo = RX_Config.clusterNo;
	}

	if (RX_Status.pumptime!=_PumpTime)
	{
		eeprom_write_setting32(EE_ADDR_PUMPTIME, RX_Status.pumptime);
		_PumpTime = RX_Status.pumptime;
	}
	if (RX_Config.clusterTime && RX_Config.clusterTime != RX_Status.clusterTime) 
	{
		eeprom_write_setting32(EE_ADDR_CLUSTER_TIME, RX_Config.clusterTime);
		RX_Status.clusterTime = RX_Config.clusterTime;
	}
}

// --- ctr_load -----------------------------
void ctr_load(void)
{
    eeprom_read_setting32(EE_ADDR_PUMPTIME, 		(INT32*)&RX_Status.pumptime);
	eeprom_read_setting32(EE_ADDR_CLUSTER_NO, 		(INT32*)&RX_Status.clusterNo);
	eeprom_read_setting32(EE_ADDR_CLUSTER_TIME, 	(INT32*)&RX_Status.clusterTime);
	eeprom_read_setting32(EE_ADDR_SERIAL_NO, 		(INT32*)&RX_Status.serialNo);	
}

