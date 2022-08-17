// ****************************************************************************
//
//	boot_srv.cpp
//
//	needs linux V19++ on the boards
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

//--- includes ----------------------------------------------------------------

#include "rx_common.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "rx_sok.h"
#include "rx_error.h"
#include "errno.h"
#include "rfs.h"
#include "tcp_ip.h"
#include "network.h"
#include "rx_mac_address.h"
#include "boot_svr.h"

//--- Defines -----------------------------------------------------------------


//--- Externals ---------------------------------------------------------------


//--- Modlue Globals -----------------------------------------------------------------

HANDLE  _BoorSvr=NULL;
RX_SOCKET _BootClient=0;

//--- Prototypes ------------------------------------------------------
static int _handle_boot_msg(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *par);
static int _open_boot_srv   (RX_SOCKET socket, const char *peerName);
static int _close_boot_srv   (RX_SOCKET socket, const char *peerName);

static void _do_boot_info  (RX_SOCKET socket, struct sockaddr *sender, SBootInfoMsg* msg);
static void _do_ping_reply (RX_SOCKET socket, struct sockaddr *sender, SBootInfoMsg* msg);
static void _start_app     (RX_SOCKET socket, struct sockaddr *sender, SBootInfoMsg* msg);

//--- boot_start -------------------------------------------------------
void boot_start(void)
{
	if (_BoorSvr == NULL)
	{
		sok_start_server(&_BoorSvr, NULL, PORT_UDP_BOOT_SVR, SOCK_DGRAM, 0, _handle_boot_msg, NULL, NULL);
		rx_sleep(100);
		sok_open_client(&_BootClient, RX_CTRL_MAIN, PORT_UDP_BOOT_SVR, SOCK_DGRAM);
		boot_request(CMD_BOOT_INFO_REQ);
	}
};

//--- boot_ipsettings_ok ------------------------------
int boot_ipsettings_ok(void)
{
	char addr[32];
	sok_get_socket_name(_BootClient, addr, NULL, NULL);
	if (str_start(addr, RX_CTRL_MAIN)) return TRUE;
	return FALSE;
}

static int _open_boot_srv   (RX_SOCKET socket, const char *peerName)
{
	TrPrintf(TRUE, "Server opened: >>%s<<", peerName);
	return REPLY_OK;
}

static int _close_boot_srv   (RX_SOCKET socket, const char *peerName)
{
	TrPrintf(TRUE, "Server closed: >>%s<<", peerName);
	return REPLY_OK;
}

//--- boot_end ----------------------------------------------------------, 
void boot_end(void)
{
	
};

//--- boot_request --------------------------------------------------------
void boot_request(UINT32 msgId)
{
	SBootInfoReqCmd		msg;
	int len;
	msg.cmd = msgId;
//	TrPrintfL(TRUE, "PING REQUEST");
	len = sok_broadcast(_BootClient, (char*)&msg, sizeof(msg), RX_CTRL_BROADCAST, PORT_UDP_BOOT_CLNT);
}

//--- _handle_boot_msg --------------------------------------------------------
static int _handle_boot_msg(RX_SOCKET socket, void *pmsg, int len, struct sockaddr *sender, void *par)
{
//	UINT32 *pcmd = (UINT32*)msg;
	SBootInfoMsg msg;		// for old boards!
	char addr[32];
	
	sok_get_addr_str(sender, addr, sizeof(addr));
	
	memset(&msg, 0, sizeof(msg));
	msg.item.rfsPort = PORT_REMOTE_FILE_SERVER;
	memcpy(&msg, pmsg, len);

	//	TrPrintfL(TRUE, "Boot:  Received cmd=0x%08x", *pcmd);
	switch(msg.id)
	{
	case REP_BOOT_INFO:	if (FALSE)
						{
							if ((msg.item.macAddr&MAC_ID_MASK) == MAC_HEAD_CTRL_ARM)
							Error(LOG, 0, "REP_BOOT_INFO (%s) from %s", msg.item.ipAddr, addr);
							
						}
						_do_boot_info (socket, sender, &msg);			
						break;

	case REP_BOOT_PING:	if(FALSE)
						{
							if ((msg.item.macAddr&MAC_ID_MASK) == MAC_HEAD_CTRL_ARM)
							Error(LOG, 0, "REP_BOOT_PING (%s) from %s", msg.item.ipAddr, addr);
						}
						_do_ping_reply(socket, sender, &msg);
						_do_boot_info (socket, sender, &msg);
//						if (msg.item.rfsPort) _start_app (socket, sender, &msg);
						break;
	}
	return REPLY_OK;
};

//--- _do_boot_info -------------------------------------------------------------
static void _do_boot_info(RX_SOCKET socket, struct sockaddr *sender, SBootInfoMsg* msg)
{
	SBootAddrSetCmd	cmd;
	int		time;
	int		headNo;
	char	exe_str[64];
	static char	test[256]={0};
	
	sok_mac_addr_str(msg->item.macAddr, exe_str, sizeof(exe_str));
//	TrPrintfL(TRUE, "BOOT INFO 1 >>%s serialNo:%s<<: mac=>>%s<< act=>>%s<<", msg->item.deviceTypeStr, msg->item.serialNo, exe_str, msg->item.ipAddr);

	//---  get address --------------------
	cmd.id		= CMD_BOOT_ADDR_SET;
	cmd.macAddr = msg->item.macAddr;
	net_get_ip_addr(&msg->item, cmd.ipAddr);
		
	if (*cmd.ipAddr==0)
	{
		printf("net_get_ip_addr(%s[%d]): ERROR\n", msg->item.deviceTypeStr, msg->item.deviceNo);
		return;
	}
	
	time = net_register(&msg->item);
//	TrPrintfL(TRUE, "BOOT INFO 2 >>%s serialNo:%s<<: mac=>>%s<< act=>>%s<< time=%d", msg->item.deviceTypeStr, msg->item.serialNo, exe_str, msg->item.ipAddr, rx_get_ticks()-time);	
	if (rx_get_ticks()-time<500 && msg->item.rfsPort==0) return;
	
	sok_mac_addr_str(msg->item.macAddr, exe_str, sizeof(exe_str));
//	TrPrintfL(TRUE, "BOOT INFO 3 >>%s serialNo:%s<<: mac=>>%s<< act=>>%s<< new=>>%s<< rfsPort=%d appPort=%d", msg->item.deviceTypeStr, msg->item.serialNo, exe_str, msg->item.ipAddr, cmd.ipAddr, msg->item.rfsPort, msg->item.ports[0]);

	if (!strcmp(msg->item.deviceTypeStr, "Head"))
		headNo=0;
	
	//--- send answer ------------- 
	if(strcmp(cmd.ipAddr, msg->item.ipAddr))
	{
		TrPrintfL(TRUE,"BOOT Broadcast >>%s serialNo:%s<<: mac=>>%s<< act=>>%s<< new=>>%s<<",msg->item.deviceTypeStr,msg->item.serialNo,exe_str,msg->item.ipAddr,cmd.ipAddr);
		Error(LOG, 0, "BOOT (%s) change to %s", msg->item.ipAddr, cmd.ipAddr);
		sok_broadcast(socket,(char*)&cmd,sizeof(cmd),RX_CTRL_BROADCAST,PORT_UDP_BOOT_CLNT); // change Address
	}
	else if (!net_is_booted(msg->item.macAddr) && msg->item.rfsPort==0)
	{
//		Error(LOG, 0, "BOOT (%s) start RFS", msg->item.ipAddr);
		sok_broadcast(socket,(char*)&cmd,sizeof(cmd),RX_CTRL_BROADCAST,PORT_UDP_BOOT_CLNT); // start the Renote File Server			
	}
	//---  start applicaiton if address is ok ----
//	TrPrintfL(TRUE, "BOOT INFO 4 >>%s serialNo:%s<<: mac=>>%s<< act=>>%s<< new=>>%s<< rfsPort=%d appPort=%d, isBooted=%d", msg->item.deviceTypeStr, msg->item.serialNo, exe_str, msg->item.ipAddr, cmd.ipAddr, msg->item.rfsPort, msg->item.ports[0], net_is_booted(msg->item.macAddr));

	if(strcmp(cmd.ipAddr, msg->item.ipAddr))
	{
//		TrPrintfL(TRUE,"rfs_reset(%s)",msg->item.ipAddr);
		rfs_reset(cmd.ipAddr);		
	}
	else if(!net_is_booted(msg->item.macAddr) && msg->item.rfsPort)
	{	
		exe_name(msg->item.deviceTypeStr,exe_str,sizeof(exe_str));
		rfs_update_start(msg->item.macAddr, cmd.ipAddr, msg->item.rfsPort, PATH_BIN, msg->item.deviceTypeStr, PATH_BIN_LX, exe_str);
	}
//	else TrPrintfL(TRUE, "no action",msg->item.ipAddr);		
};

//--- _start_app -----------------------------------------------------
static void _start_app(RX_SOCKET socket, struct sockaddr *sender, SBootInfoMsg* msg)
{
	char	exe_str[64];
	if (!net_is_booted(msg->item.macAddr) && msg->item.rfsPort)
	{
		exe_name(msg->item.deviceTypeStr, exe_str, sizeof(exe_str));
		rfs_update_start(msg->item.macAddr, msg->item.ipAddr, msg->item.rfsPort, PATH_BIN, msg->item.deviceTypeStr, PATH_BIN_LX, exe_str);
	}
};

//--- _do_ping_reply --------------------------------------------------------------
static void _do_ping_reply(RX_SOCKET socket, struct sockaddr *sender, SBootInfoMsg* msg)
{
	char str[32];
	sok_get_addr_str(sender, str, sizeof(str));

//	TrPrintfL(TRUE, "Got ping: %s: >>%s serialNo=%s<<", str, msg->item.deviceTypeStr, msg->item.serialNo);
	net_register(&msg->item);
}


//--- boot_set_flashing --------------------------------------
void boot_set_flashing(UINT64  macAddr)
{
	SBootAddrSetCmd	cmd;
	int len;

	memset(&cmd, 0, sizeof(cmd));
	cmd.id			 = CMD_BOOT_FLASH_ON;
	cmd.macAddr		 = macAddr;
	len = sok_broadcast(_BootClient, (char*)&cmd, sizeof(cmd), RX_CTRL_BROADCAST, PORT_UDP_BOOT_CLNT);
}