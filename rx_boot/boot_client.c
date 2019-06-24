// ****************************************************************************
//
//	DIGITAL PRINTING - boot_client.c
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

//--- includes ----------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
#include <errno.h>
#include "rx_error.h"
#include "rx_led.h"
#include "rx_sok.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "tcp_ip.h"
#include "file_server.h"
#include "boot_client.h"

//--- defines ------------------------------------------------------------------------

#define LED	 	"hps_led0"

#define TRACE FALSE

//--- module globals -----------------------------------------------------------------
HANDLE			_HServer=NULL;
SNetworkItem	_Item;
char			_DeviceName[64];
int				_Confirmed=FALSE;
int				_RfsStarted=FALSE;
int				_Flashing=FALSE;
HANDLE			_BootThread=NULL;

//--- prototypes ---------------------------------------------------------------------

static void _send_boot_info(UINT32 id);
static void _send_msg(void *msg, int size);

static int _handle_msg(RX_SOCKET socket, void *msg, int len, struct sockaddr	*sender, void *par);
static int _do_set_addr(struct sockaddr *sender, SBootAddrSetCmd *cmd);
static int _set_if_addr(struct sockaddr *sender, char *deviceName, int group, int no);
static int _do_flash_on(SBootAddrSetCmd *cmd);
static void _enum_ports(void);

static void* _boot_thread(void* par);


//--- boot_init -------------------------------------
void boot_init(char *commName, char* deviceType, UINT32 serialNo)
{
	strncpy(_DeviceName, commName, sizeof(_DeviceName));
	memset(&_Item, 0, sizeof(_Item));
	strncpy(_Item.deviceTypeStr, deviceType, sizeof(_Item.deviceTypeStr));
	sprintf(_Item.serialNo, "%d", serialNo);
	sok_get_mac_address(_DeviceName, &_Item.macAddr);
}

//--- boot_start -------------------------------------
void boot_start()
{
	// on linux Receiver must be on INADDR_ANY to revceive BROADCASTS
	if (sok_start_server(&_HServer, NULL, PORT_UDP_BOOT_CLNT, SOCK_DGRAM, 0, _handle_msg, NULL, NULL)!=REPLY_OK)
	{
		char err[100];
		TrPrintfL(1, "Error >>%s<<", err_system_error(errno, err, sizeof(err)));
	}
	sok_get_ip_address_str(_DeviceName, _Item.ipAddr, sizeof(_Item.ipAddr));
	if (_BootThread==NULL) _BootThread = rx_thread_start(_boot_thread, NULL, 0x1000, "_boot_thread");
}

//--- _boot_thread -------------------------------------------------
static void* _boot_thread(void* par)
{
	while (TRUE)
	{
		rx_sleep(1000);
		if (!_Confirmed || _RfsStarted)
			_send_boot_info(REP_BOOT_INFO);
		_RfsStarted = FALSE;
	}
	_BootThread=NULL;
	return NULL;
}

//--- _send_msg -------------------------------------------
static void _send_msg(void *msg, int size)
{
	if (_HServer!=NULL)
	{
		int len = sok_broadcast(sok_socket(_HServer), msg, size, RX_CTRL_BROADCAST, PORT_UDP_BOOT_SVR);
		if (len!=size)
		{
			char err[100];
			int  no=errno;
			TrPrintfL(1, "send error >>%s<<", err_system_error(errno, err, sizeof(err)));
		}
	}
}

//--- _enum_ports ---------------------------------------
static void _enum_ports(void)
{
	FILE* file;
	char str[100];
	int ports=0;
	char* ret;

	file=popen("netstat -lt", "r");
	memset(_Item.ports, 0, sizeof(_Item.ports));
	while (!feof(file) && ports<SIZEOF(_Item.ports))
	{
		ret=fgets(str, 100, file);
		if (str[26]==':') 
		{
			_Item.ports[ports]=atoi(&str[27]);
			if (_Item.ports[ports]>=7000) ports++;
		}
	}
	pclose(file);
}

//--- _send_boot_info ------------------------------------
static void _send_boot_info(UINT32 id)
{
	static SBootInfoMsg last={};
	static int			lastTime=0;

	SBootInfoMsg msg;
	int			 time = rx_get_ticks();
	
	msg.id  	= id;

	_Item.rfsPort = fs_get_port();
	memcpy(&msg.item, &_Item, sizeof(msg.item));
	
//		TrPrintfL(1, "_send_boot_info: >>%s %s<<", msg.item.deviceTypeStr, msg.item.serialNo);
	rx_sleep(rx_get_ticks()%50); // sleep random to avoid collusions on ethernet
	_enum_ports();

	_send_msg(&msg, sizeof(msg));
	/*
	if (time-lastTime>1000 || memcmp(&msg, &last,sizeof(SBootInfoMsg)))
	{
		TrPrintfL(TRACE, "_send_boot_info");
		_send_msg(&msg, sizeof(msg));
		memcpy(&last, &msg, sizeof(SBootInfoMsg));
		lastTime = time;		
	}
	*/
}

//--- _handle_msg ---------------------------------------------------------
static int _handle_msg(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *par)
{
	int reply = REPLY_OK;
	UINT32	*pcmd=(UINT32*)msg;
	
	//--- handle the message --------------
	switch (*pcmd)
	{
	case CMD_BOOT_INFO_REQ:		TrPrintfL(TRACE, "got CMD_BOOT_INFO_REQ");
								_Confirmed=FALSE;
								_send_boot_info(REP_BOOT_INFO);
								break;
	case CMD_BOOT_PING:			TrPrintfL(TRACE, "got CMD_BOOT_PING");
								_send_boot_info(REP_BOOT_PING);
								break;
	case CMD_BOOT_ADDR_SET:		TrPrintfL(TRACE, "got CMD_BOOT_ADDR_SET");
								_do_set_addr(sender, (SBootAddrSetCmd*)msg);	break;
	case CMD_BOOT_FLASH_ON:		_do_flash_on((SBootAddrSetCmd*)msg);	break;

	default:					TrPrintfL(1, "Unknown Command 0x%04x", *pcmd);
								reply = REPLY_ERROR;
								break;
	}

	return reply;
};

//--- _set_if_addr ----------------------------
static int _set_if_addr(struct sockaddr *sender, char *deviceName, int group, int no)
{
	int err;
	char addr[32];
	char newaddr[32];
	sprintf(newaddr, "192.168.%d.%d", group, no);
	sok_get_ip_address_str(deviceName, addr, sizeof(addr));
	if  (*addr && strcmp(newaddr, addr))
	{
		fs_end();

		TrPrintfL(1, "\n   *** Inferface[%s] Changing from >>%s<< to >>%s<< ***\n", deviceName, addr, newaddr);

		TrPrintfL(1, "_set_if_addr Tets 1");

		err=sok_set_ip_address_str(deviceName, newaddr);
		if (err) 
		{
			char buffer[128];
			TrPrintfL(1, "Error >>%s<<", err_system_error(err, buffer, sizeof(buffer)));				
		}
		if (group==200) sok_get_ip_address_str(deviceName, _Item.ipAddr, sizeof(_Item.ipAddr));

		TrPrintfL(1, "_set_if_addr Tets 2");
			
		//---- activate the new address with a PING -----
		sok_get_addr_str(sender, addr, sizeof(addr));
		TrPrintfL(1, "_set_if_addr Tets 3");
		sok_ping(addr);
		TrPrintfL(1, "_set_if_addr Tets END");
		return TRUE;
	}
	return FALSE;
}

//--- _do_set_addr ----------------------------------------------------------
static int _do_set_addr( struct sockaddr *sender, SBootAddrSetCmd *cmd)
{
	char addr[32];
	int err;
	if (cmd->macAddr==_Item.macAddr && *cmd->ipAddr)
	{
		UINT32 a = sok_addr_32(cmd->ipAddr);
		int no = (a>>24)&0xff;

		//--- spooler ------------------
		if (!strcmp(_Item.deviceTypeStr, "Spooler"))
		{
			int i;
			char *devicename[]= {"p1p1", "p1p2", "p1p3", "p1p4", "p2p1", "p2p2", "p2p3", "p2p4"};
			for (i=0; i<SIZEOF(devicename); i++)
			{
				_set_if_addr(sender, devicename[i], 201+i, no);
			}
		}		
		
		if (!_set_if_addr(sender, _DeviceName, 200, no))
		{
			TrPrintfL(1, "Address >>%s<< confirmed", cmd->ipAddr);
			_Confirmed =TRUE;
			if (fs_get_port()==0)
			{
				sok_get_addr_str(sender, addr, sizeof(addr));
				sok_ping(addr);
				sok_ping(addr);
				fs_start(_DeviceName);
				_RfsStarted = TRUE;
			}
		}
	}
	return REPLY_OK;
}

//--- _do_flash_on ----------------------------------------------------------
static int _do_flash_on(SBootAddrSetCmd *cmd)
{
	#ifdef soc
		if (cmd->macAddr==_Item.macAddr)	led_blink(LED, 500);
		else 								led_off(LED);
	#else
		int ret;
		if (cmd->macAddr==_Item.macAddr)	ret=system("ipmitool chassis identify force");
		else 								ret=system("ipmitool chassis identify 0");
	#endif
	return REPLY_OK;
}
