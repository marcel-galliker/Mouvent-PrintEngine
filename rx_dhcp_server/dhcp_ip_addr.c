#include<stdio.h>
#include<string.h>

#include "rx_error.h"
#include "rx_mac_address.h"
#include "rx_setup_file.h"

#include "dhcp_server.h"
#include "dhcp_server_def.h"


#define MIN_ADDR	231
#define LIFE_TIME	10

typedef struct
{
	UINT64	macAddr;
	UINT64	timestamp;
} SGuiItem;

static SGuiItem _GuiList[10]={0};

static char _FilePath[MAX_PATH]={0};

//--- dhcp_addr_init -----------------
int dhcp_addr_init(char *path)
{
	HANDLE file = setup_create();
	int	   i;
	
	strcpy(_FilePath, path);
	memset(_GuiList, 0, sizeof(_GuiList));
	setup_load(file, _FilePath);
	for (i=0; i<SIZEOF(_GuiList); i++)
	{
		if (setup_chapter(file, "gui", i, READ)==REPLY_OK)	
		{
			setup_mac_addr(file, "macAddr",		READ, &_GuiList[i].macAddr,		0);
			setup_uint64  (file, "timestamp",	READ, &_GuiList[i].timestamp,	0);
			setup_chapter (file, "..", i, READ);
		}
	}
	setup_destroy(file);

	return REPLY_OK;
}

//--- dhcp_addr_save -----------------
int _dhcp_addr_save(void)
{
	HANDLE file = setup_create();
	int	   i;
	for (i=0; i<SIZEOF(_GuiList); i++)
	{
		if (setup_chapter(file, "gui", i, WRITE)==REPLY_OK)	
		{
			setup_mac_addr(file, "macAddr",		WRITE, &_GuiList[i].macAddr,	0);
			setup_uint64  (file, "timestamp",	WRITE, &_GuiList[i].timestamp,	0);				
			setup_chapter(file, "..", i, WRITE);
		}				
	}
	setup_save(file, _FilePath);
	setup_destroy(file);
	return REPLY_OK;
}

//--- _use_addr -------------------------------
static int _use_addr(struct network_config *pcfg, int i)
{
	char   str[32];
	UINT64	macAddr;
	
	memcpy(&macAddr, pcfg->hardware_address, sizeof(macAddr));
	_GuiList[i].macAddr  = macAddr;
	rx_get_system_time(&_GuiList[i].timestamp);
	
	pcfg->router	= sok_addr_32("");
	pcfg->netmask	= sok_addr_32("255.255.255.0");
	pcfg->dns1		= sok_addr_32("");
	pcfg->dns2		= sok_addr_32("");
	sprintf(str, "192.168.200.%d", MIN_ADDR+i);
	pcfg->ip_address = sok_addr_32(str);
	
	_dhcp_addr_save();
	return 0;			
}

//--- dhcp_addr_get_cfg -------------------------------------
int dhcp_addr_get_cfg(struct network_config *pcfg)
{		
	UINT64 macAddr, timestamp;
	int i, oldest;
	
	memcpy(&macAddr, pcfg->hardware_address, sizeof(macAddr));
//	if ((macAddr & MAC_OUI_MASK) == MAC_OUI_MICROSOFT)
	
	if(TRUE)
	{	
		for (i=0; i<SIZEOF(_GuiList); i++)
		{
			if (_GuiList[i].macAddr==macAddr || _GuiList[i].macAddr==0) return _use_addr(pcfg, i);
		}		
		// find oldest
		timestamp = _GuiList[0].timestamp;
		oldest    = 0;
		for (i=1; i<SIZEOF(_GuiList); i++)
		{
			if (_GuiList[i].timestamp<timestamp)
			{
				oldest=i;
				timestamp = _GuiList[i].timestamp;
			}
		}		
		return _use_addr(pcfg, oldest);
	}
	
	return -1;
}
