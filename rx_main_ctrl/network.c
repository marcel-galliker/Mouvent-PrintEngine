// ****************************************************************************
//
//	network.c
//
//	main controller tasks.
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

//--- includes ----------------------------------------------------------------
#include "rx_common.h"
#include "rx_def.h"
#include "rx_error.h"
#include "rx_file.h"
#include "rx_sok.h"
#include "rx_trace.h"
#include "rx_setup_file.h"
#include "rx_mac_address.h"
#include "args.h"
#include "rfs.h"
#include "setup.h"
#include "tcp_ip.h"
#include "ctrl_svr.h"
#include "gui_svr.h"
#include "boot_svr.h"
#include "network.h"

//--- externals ----------------------------------------------------------------

//--- statics ------------------------------------------------------------------
#define MAX_DEVICES 64

static int			_Flashing	= -1;
static int			_Connected[MAX_DEVICES];
static int			_ConnectTime[MAX_DEVICES];
static int			_Checked[MAX_DEVICES];
static UCHAR		_Booted[MAX_DEVICES];
static int			_CheckTime;		
static SRxNetwork	_Network;

//--- prototypes ---------------------------------------------------------------
static void		_send_item(RX_SOCKET socket, int i);
static EDevice  _get_device(char *deviceStr, UINT64 mac);
static void		_net_check(void);


//--- net_init -----------------------------------------------------------------
void net_init(void)
{
	HANDLE file=NULL;

	file = setup_create();
	memset(_Connected, 0, sizeof(_Connected));
	memset(_ConnectTime, 0, sizeof(_ConnectTime));
	memset(_Checked, 0, sizeof(_Checked));
	memset(_Booted,  0, sizeof(_Booted));
	_CheckTime=0;
	setup_load(file, PATH_USER FILENAME_NETWORK);
	setup_network(file, &RX_Network, READ);
	if (RX_Config.printer.type!=printer_DP803)
	{
		int i;
		for (i=0; i<SIZEOF(RX_Network.item); i++)
		{
			if (RX_Network.item[i].deviceType==dev_enc) RX_Network.item[i].deviceNo=0;
		}			
	}
	memcpy(&_Network, &RX_Network, sizeof(_Network));
	setup_destroy(file);
	{
		SNetworkItem item;
		memset(&item, 0, sizeof(item));
		strcpy(item.deviceTypeStr, DeviceStr[dev_main]);
		item.deviceType = dev_main;
		item.deviceNo   = 0;
		item.macAddr	= 1;
		net_register(&item);
	}
}

//--- net_end ------------------------------------------------------------------
void net_end(void)
{

}

//--- net_tick -----------------------------------------------
void net_tick(void)
{
	_net_check();		
}

//--- Save --------------------------------------------------------------------
void net_save(const char *filename)
{
	char path[MAX_PATH];
	HANDLE file;

	file = setup_create();
	memcpy(&RX_Network, &_Network, sizeof(RX_Network));
	sprintf(path, "%s%s", PATH_USER, filename);
	rx_mkdir_path(path);
	setup_load(file, path);
	setup_network(file, &RX_Network, WRITE);
	setup_save(file, path);
	setup_destroy(file);
	memset(_Connected, 0, sizeof(_Connected));
	boot_request(CMD_BOOT_PING);
}

//--- _DevStrToDev -------------------
EDevice _DevStrToDev(char *str)
{
	int i;

	for (i = 1; DeviceStr[i]; i++)
	{
		if (!_stricmp(DeviceStr[i], str)) return (EDevice)i;
	}
	return dev_undef;
}

//--- _get_device ---------------------------------
static EDevice  _get_device(char *deviceStr, UINT64 mac)
{
	if ((mac & MAC_OUI_MASK) != MAC_OUI_ALTERA && !strcmp(deviceStr, "Spooler")) return dev_spooler;
	if ((mac & MAC_ID_MASK)  == MAC_HEAD_CTRL_ARM)	return dev_head;
	if ((mac & MAC_ID_MASK)  == MAC_ENCODER_CTRL)	return dev_enc;
	if ((mac & MAC_ID_MASK)  == MAC_ENC32_CTRL)		return dev_enc32;
	if ((mac & MAC_ID_MASK)  == MAC_FLUID_CTRL)		return dev_fluid;
	if ((mac & MAC_ID_MASK)  == MAC_STEPPER_CTRL)	return dev_stepper;
	return dev_undef;
}

//--- net_get_ip_addr ------------------------------------------------------- 
void net_get_ip_addr(SNetworkItem *item, char *ipAddr)
{
	int i;
	EDevice device = _get_device(item->deviceTypeStr, item->macAddr);
	if (device!=dev_undef) item->deviceType = device;		
	for (i=0; i<SIZEOF(_Network.item); i++)
	{		
		if (_Network.item[i].macAddr==item->macAddr)
		{
			net_device_to_ipaddr(item->deviceType, _Network.item[i].deviceNo, ipAddr, sizeof(item->ipAddr));
			return;
		}
		if (_Network.item[i].deviceType==dev_undef)
		{		
			memcpy(&_Network.item[i], item, sizeof(SNetworkItem));
			_Network.item[i].deviceNo = -1;
			net_device_to_ipaddr(item->deviceType, _Network.item[i].deviceNo, ipAddr, sizeof(item->ipAddr));
			return;						
		}
	}
}

//---  net_register ------------------------------------------------------------
int net_register(SNetworkItem *item)	// register and calculate IP-Address
{
	int i;
	int time=0;
	char str[32];

	if (item->macAddr==0) return 0;
	
	item->deviceType = _DevStrToDev(item->deviceTypeStr);
	if (item->deviceType==dev_undef)
	{
		Error(ERR_CONT, 0, "MacAddr %s: Device >>%s %s<< not defined.", sok_mac_addr_str(item->macAddr, str, sizeof(str)), item->deviceTypeStr, item->serialNo);
		return FALSE;
	}
	
	//---  check if already registered ---
	for (i = 0; i < SIZEOF(_Network.item); i++)
	{
		if (item->macAddr == _Network.item[i].macAddr)
		{	
			time = _ConnectTime[i];
			_Checked[i]   = rx_get_ticks();
			_ConnectTime[i] = rx_get_ticks();
			memcpy(&_Network.item[i].ports, item->ports, sizeof(_Network.item[i].ports));
			// if (item->ports[0]!=0) printf("Port %s:%d Listening\n", item->ipAddr, item->ports[0]);
			if (!_Connected[i])
			{
				_Connected[i] = TRUE;
				_send_item(NO_SOCKET, i);
				return time;
			}
			return time;
		}
	}

	TrPrintfL(TRUE, "net_register: Device >>%s %d (serial=%s)<<", item->deviceTypeStr, item->deviceNo+1, item->serialNo);

	//--- search first gap ---
	for (i = 0; i < SIZEOF(_Network.item); i++)
	{
		if (_Network.item[i].macAddr == 0)
		{
			_Checked[i] = _Connected[i] = FALSE;
			_ConnectTime[i] = rx_get_ticks();

//			net_device_to_ipaddr((EDevice)_Network.item[i].deviceType, _Network.item[i].deviceNo, item->ipAddr, sizeof(item->ipAddr));
			net_device_to_ipaddr(item->deviceType, item->deviceNo, item->ipAddr, sizeof(item->ipAddr));
			memcpy	(&_Network.item[i], item, sizeof(_Network.item[0]));
			_send_item(NO_SOCKET, i);
			TrPrintfL(TRUE, "Device >>%s %d (serial=%s)<<: connected", _Network.item[i].deviceTypeStr, _Network.item[i].deviceNo+1, _Network.item[i].serialNo);
			return time;
		}
	}
	Error(ERR_CONT, 0, "Network list overflow!");
	return 0;
}

BOOL net_is_connected(EDevice dev, int no) 
{
	for (int i = 0; i < SIZEOF(_Network.item); i++)
	{
		if (dev == _Network.item[i].deviceType && no == _Network.item[i].deviceNo)
		{
			return _Connected[i];
		}
	}
	return FALSE; // not register
}

//--- net_register_by_device --------------------------------------
void net_register_by_device(EDevice dev, int no)
{
	int i;
	for (i=0; i<SIZEOF(_Network.item); i++)
	{
		if (dev == _Network.item[i].deviceType  && no==_Network.item[i].deviceNo )
		{			
			_Checked[i]   = rx_get_ticks();
			return;
		}
	}
}

//---  net_unregister ------------------------------------------------------------
void net_unregister(SNetworkItem *item)
{
	int i;
	for (i=0; i < SIZEOF(_Network.item); i++)
	{
		if (_Network.item[i].deviceType==item->deviceType && (_Network.item[i].macAddr==item->macAddr))
		{
			while (_Network.item[i+1].macAddr)
			{
				memmove(&_Network.item[i], &_Network.item[i+1], sizeof(SNetworkItem));
				i++;
			}
			memset(&_Network.item[i], 0, sizeof(SNetworkItem));

			net_send_items(NO_SOCKET, TRUE);
			return;
		}
	}	
}

//--- net_disconnnected ------------------------------------
void net_disconnnected(SNetworkItem *item)
{
	int i;
	for (i=0; i < SIZEOF(_Network.item); i++)
	{
		if (_Network.item[i].deviceType==item->deviceType && (_Network.item[i].macAddr==item->macAddr))
		{
			_Network.item[i].connected = FALSE;
			_Connected[i]=FALSE;
			_Booted[i] = FALSE;
			_send_item(NO_SOCKET, i);
		}
	}	
}

//--- net_is_registered ---------------------------------
int net_is_registered(EDevice dev, int no)
{
	int i;
	for (i=0; i < SIZEOF(_Network.item); i++)
	{
		if (_Network.item[i].deviceType==dev && (_Network.item[i].deviceNo==no))
		{
			return TRUE;
		}
	}
	return FALSE;
}

//--- net_port_listening ------------------------------------------------
int  net_port_listening(EDevice dev, int no, int port)
{
	int i, p;
	for (i=0; i < SIZEOF(_Network.item); i++)
	{
		if (_Network.item[i].deviceType==dev && _Network.item[i].deviceNo==no)
		{
			if (_Connected[i] && _Booted[i])
			{
				for (p=0; p<SIZEOF(_Network.item[i].ports); p++)
				{
					if (_Network.item[i].ports[p]==0)	 return FALSE;						
					if (_Network.item[i].ports[p]==port) return TRUE;						
				}				
			}
		}
	}
	return FALSE;
}

//--- net_booted ----------------------------------------------------
int	 net_booted(UINT64 macAddr)
{
	int i;
	for (i=0; i < SIZEOF(_Network.item); i++)
	{
		if (_Network.item[i].macAddr == macAddr)
		{
			_Booted[i]=TRUE;
			break;
		}
	}
	return REPLY_OK;
}

//--- net_is_booted --------------------------------------------------
int net_is_booted(UINT64 macAddr)
{
	int i;
	for (i=0; i < SIZEOF(_Network.item); i++)
	{
		if (_Network.item[i].macAddr == macAddr) return _Booted[i];
	}
	return FALSE;
}

/*
//--- net_device_connected ---------------------------------
void net_device_connected(EDevice dev, int no)
{
	int i;
	for (i=0; i < SIZEOF(_Network.item); i++)
	{
		if (_Network.item[i].deviceType==dev && (_Network.item[i].deviceNo==no))
		{
			_Network.item[i].connected = TRUE;
			_Registered[i]=TRUE;
			_send_item(NO_SOCKET, i);
		}
	}	
}

//--- net_device_disconnected ---------------------------------
void net_device_disconnected(EDevice dev, int no)
{
	int i;
	for (i=0; i < SIZEOF(_Network.item); i++)
	{
		if (_Network.item[i].deviceType==dev && (_Network.item[i].deviceNo==no))
		{
			_Network.item[i].connected = FALSE;
			_Registered[i]=FALSE;
			_send_item(NO_SOCKET, i);
		}
	}	
}
*/

//--- net_device_to_ipaddr ----------------------------------------------------------
int net_device_to_ipaddr(EDevice dev, int no, char *ipAddr, int size)
{
	*ipAddr = 0;
	switch (dev)
	{
	case dev_main:	sprintf(ipAddr, RX_CTRL_MAIN);  break;

	case dev_enc:	if(RX_Config.printer.type == printer_DP803 && no>=0 && no<2) sprintf(ipAddr, "%s%d", RX_CTRL_SUBNET, RX_CTRL_ENC_0 + no); 
					else														 sprintf(ipAddr, "%s%d", RX_CTRL_SUBNET, RX_CTRL_ENC_0);
					break;

	case dev_head:	if (no<0 || no>100) return REPLY_ERROR; 
					if (rx_def_is_lb(RX_Config.printer.type))
					{
						int barsize=((RX_Config.headsPerColor+MAX_HEADS_BOARD-1)/MAX_HEADS_BOARD);
						int color = 0;
						if (barsize!=0) color = no/barsize;
						sprintf(ipAddr, "%s%d", RX_CTRL_SUBNET, RX_CTRL_HEAD_0+10*color+no-color*barsize);
					}
					else sprintf(ipAddr, "%s%d", RX_CTRL_SUBNET, RX_CTRL_HEAD_0+no); 
					break;

	case dev_fluid:	if (no<0 || no>1) return REPLY_ERROR; 
					sprintf(ipAddr, "%s%d", RX_CTRL_SUBNET, RX_CTRL_FLUID_0+no); 
					break;

	case dev_plc:	sprintf(ipAddr, "%s%d", RX_CTRL_SUBNET, RX_CTRL_PLC); 
					break;

	case dev_stepper:	if (rx_def_is_lb(RX_Config.printer.type))
						{
							if (no<0 || no>3)
							{
								printf("Error\n");
								return REPLY_ERROR; 		
							}
							sprintf(ipAddr, "%s%d", RX_CTRL_SUBNET, RX_CTRL_STEPPER_0+1+10*no);  														
						}
						else
						{
							if (no<0 || no>1) return REPLY_ERROR; 
							sprintf(ipAddr, "%s%d", RX_CTRL_SUBNET, RX_CTRL_STEPPER_0+no);  							
						}
						break;

	case dev_spooler:
					if (no<=0 || no>=255)	sprintf(ipAddr, "%s%d", RX_CTRL_SUBNET, 200); 
					else					sprintf(ipAddr, "%s%d", RX_CTRL_SUBNET, 200+no); 
					break;
	case dev_iq500:
		sprintf(ipAddr, "%s%d", RX_CTRL_SUBNET, RX_CTRL_IQ500);
					break;
	default:		return REPLY_ERROR;
	}
	return REPLY_OK;
}


//--- net_head_ctrl_addr ----------------------------------------------
UINT32 net_head_ctrl_addr	(int headNo)
{
	char ipAddr[64];
	net_device_to_ipaddr(dev_head, headNo, ipAddr, sizeof(ipAddr));
	return sok_addr_32(ipAddr);
}

//--- net_head_data_addr -----------------------------------------------
UINT32 net_head_data_addr	(int headNo, int udpNo, int ethPortCnt, int udpPortCnt)
// udpPortCnt:  number of IP-Ports on the server
// dataPortCnt: number of Data- IP-Ports on the head board
{
	char ipAddr[64];
	int addr[4];
	net_device_to_ipaddr(dev_head, headNo, ipAddr, sizeof(ipAddr));
	sscanf(ipAddr, "%d.%d.%d.%d", &addr[0], &addr[1], &addr[2], &addr[3]);
	if (udpNo>=udpPortCnt) return 0;
	if (ethPortCnt)	sprintf(ipAddr, "%d.%d.%d.%d", addr[0], addr[1], addr[2] + 1 + ((headNo*udpPortCnt + udpNo) % ethPortCnt), addr[3]);
	else			sprintf(ipAddr, "%d.%d.%d.%d", addr[0], addr[1], addr[2], 50+10*udpNo+headNo);
	return sok_addr_32(ipAddr);
}

//--- net_ipaddr_to_device -------------------------------------------
void net_ipaddr_to_device	(const char *ipAddr, EDevice *pdevice, int *pno)
{
	int i;

	*pno = 0;
	*pdevice = dev_undef;
	i = str_start(ipAddr, RX_CTRL_SUBNET);
	if (i)
	{
		i = atoi(&ipAddr[i]);
		if (i==1)
		{
			*pdevice = dev_spooler;
			*pno     = 0;				
		}
		else if (i>200) 
		{
			*pdevice = dev_spooler;
			*pno     = i-200;
		}
		else if (i>100)
		{
			*pdevice = dev_head;
			*pno     = i-100;
		}
	}
}

//--- net_get_ipaddr -------------------------------------------------------------
int net_get_ipaddr(SNetworkItem *item, char *ipAddr, int size)
{
	int i;

	//---  check if already registered ---
	for (i = 0; i < SIZEOF(_Network.item); i++)
	{
		if (_Network.item[i].deviceType==item->deviceType && !strcmp(_Network.item[i].serialNo, item->serialNo))
		{
			if (_Network.item[i].deviceNo==0) return REPLY_OK;
			int ret= net_device_to_ipaddr((EDevice)_Network.item[i].deviceType, _Network.item[i].deviceNo, ipAddr, size);
			return ret;
		}
	}
	return REPLY_ERROR;
}

//--- _send_item ---------------------------------------------------------------------
static void _send_item(RX_SOCKET socket, int i)
{
	SNetworkMsg	msg;

	msg.hdr.msgId  = EVT_NETWORK_ITEM;
	msg.hdr.msgLen = sizeof(msg);
	if (_Network.item[i].deviceType==dev_undef)
		printf("Error\n");
	memcpy(&msg.item, &_Network.item[i], sizeof(msg.item));
	net_device_to_ipaddr((EDevice)_Network.item[i].deviceType, _Network.item[i].deviceNo, msg.item.ipAddr, sizeof(msg.item.ipAddr));
	msg.item.connected = _Connected[i] || !strcmp(msg.item.ipAddr, RX_CTRL_MAIN) || (msg.item.macAddr==MAC_SIMU_PLC);
//						|| (msg.item.deviceType==dev_spooler) 
//						|| (msg.item.deviceType==dev_main);
//						|| ctrl_is_connected((EDevice)_Network.item[i].deviceType, _Network.item[i].deviceNo);
	msg.flash = (_Flashing==i);
	
//	TrPrintfL(TRUE, "send Network Item: >>%s %s<< connected=%d, registered=%d, connected=%d", msg.item.deviceTypeStr, msg.item.serialNo, ret, _Registered[i], msg.connected);
	gui_send_msg(socket, &msg);
}

//--- net_reset ------------------------------------------------------------------
void net_reset(void)
{
	SMsgHdr cmd;

	cmd.msgLen= sizeof(cmd);
	cmd.msgId = REP_NETWORK_RELOAD;
	gui_send_msg(NO_SOCKET, &cmd);

	memset(_Connected, 0, sizeof(_Connected));
	boot_request(CMD_BOOT_INFO_REQ);

	memcpy(&_Network, &RX_Network, sizeof(_Network));
	net_send_items(NO_SOCKET, TRUE);
}

//--- _net_check -------------------------------------------------------------------
static void _net_check(void)
{
	int i; 
	UINT32 time = rx_get_ticks();
	
	for (i=0; i<SIZEOF(_Network.item); i++)
	{
		if (_Network.item[i].deviceType==dev_plc || _Network.item[i].deviceType==dev_main) _Checked[i]=time;
		if (_Connected[i] && (time-_Checked[i])>10000)
		{
			ErrorEx(_Network.item[i].deviceType, _Network.item[i].deviceNo, ERR_CONT, 0, "connection lost");
			_Connected[i]=FALSE;
			_Booted[i]=FALSE;
			_send_item(NO_SOCKET, i);
			{
				char addr[32];
				net_device_to_ipaddr(_Network.item[i].deviceType, _Network.item[i].deviceNo, addr, sizeof(addr));
				rfs_reset(addr);
			}
		}
	}
	
	if (_CheckTime==0 || time-_CheckTime>900)
	{
//		memset(_Checked, 0, sizeof(_Checked));
		boot_request(CMD_BOOT_PING);
		_CheckTime = time;
	//	TrPrintfL(TRUE, "Request CMD_BOOT_PING");
	}
}

//--- net_send_config --------------------------------
void net_send_config(RX_SOCKET socket)
{
	SIfConfig config;

	sok_get_ifconfig("em1", &config); 
	gui_send_msg_2(socket, REP_NETWORK_SETTINGS, sizeof(config), &config);
}

//--- net_set_config --------------------------------
void net_set_config	(RX_SOCKET socket, SIfConfig *config)
{
	sok_set_ifconfig("em1", config); 			
//	net_send_config(socket);
}


//--- net_send_items ---------------------------------------------------------------
void net_send_items(RX_SOCKET socket, int reload)
{
	int i;

	if (reload)
	{
		SMsgHdr cmd;
		cmd.msgLen= sizeof(cmd);
		cmd.msgId = REP_NETWORK_RELOAD;
		gui_send_msg(socket, &cmd);
	}

	for (i = 0; i < SIZEOF(_Network.item); i++)
	{
		if (_Network.item[i].macAddr) _send_item(socket, i);
	}
}

//--- net_send_item --------------------------------------------
void net_send_item	(EDevice deviceType, int no)
{
	int i;

	for (i = 0; i < SIZEOF(_Network.item); i++)
	{
		if (_Network.item[i].deviceType==deviceType && _Network.item[i].deviceNo==no) 
			_send_item(NO_SOCKET, i);
	}	
}

//--- net_set_item --------------------------------------------------------------
int net_set_item(RX_SOCKET socket, SNetworkItem *item, int flash)
{
	int i, old;

	for (i = 0; i < SIZEOF(_Network.item); i++)
	{
		if (_Network.item[i].macAddr == item->macAddr)
		{
			_Network.item[i].deviceNo = item->deviceNo;
			if ((i == _Flashing) && (!flash || _Flashing>=0))
			{
				old=_Flashing;
				_Flashing = -1;
				if(item->deviceType==dev_main)	system("ipmitool chassis identify 0");
				else							boot_set_flashing(0);
				if (old>=0) _send_item(socket, old);
			}
			if (flash && _Flashing<0)
			{
				_Flashing  = i;
				if(item->deviceType==dev_main)	system("ipmitool chassis identify force");
				else							boot_set_flashing(item->macAddr);					
			}
			_send_item(socket, i);
			return REPLY_OK;
		}
	}
	return REPLY_ERROR;
}

//--- net_delete_item --------------------------------------------------------------
int net_delete_item(RX_SOCKET socket, SNetworkItem *item)
{
	int i;

	for (i = 0; i < SIZEOF(_Network.item); i++)
	{
		if (_Network.item[i].macAddr == item->macAddr)
		{
			for(;i+1<SIZEOF(_Network.item);i++)
			{
				_Network.item[i] = _Network.item[i+1];
				_Connected[i]    = _Connected[i+1];
				_Checked[i]      = _Checked[i+1];
			}
			_Network.item[SIZEOF(_Network.item)-1].macAddr=0;
			net_send_items(NO_SOCKET, TRUE);
			return REPLY_OK;
		}
	}
	return REPLY_ERROR;
}