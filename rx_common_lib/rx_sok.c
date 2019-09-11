//
//	rx_sok.c
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

// ----------------------------------------------------------------------------
//	Windows:
//  --------
//
//	Reqesting and changing the Ethernet Address is not impelented in Windows 
//	at the moment.
//	Implementation needs the DLL: Iphlpapi.lib, set macro WIN_ADDR
//
// ----------------------------------------------------------------------------

#ifdef linux
	#include <stdlib.h>
	#include <stdio.h>
	#include <unistd.h>
	#include <netdb.h>
	#include <errno.h>
	#include <sys/ioctl.h>
	#include <sys/select.h>
	#include <sys/socket.h>
	#include <sys/time.h>
	#include <sys/types.h>
	#include <net/if.h>
	#include <netinet/in.h>
	#include <netinet/in_systm.h>
	#include <netinet/ether.h>
	#include <netinet/tcp.h>
	#include <netinet/udp.h>
	#include <linux/ip.h>
	#include <linux/if_packet.h>
	#include <arpa/inet.h>
	#include <string.h>
	#include <stdio.h>
#else
	#pragma warning(disable:4996)
	#include <winsock2.h>
	#include <iphlpapi.h>
	#undef WIN_ADDR
#endif
#include "rx_common.h"
#include "rx_error.h"
#include "rx_sok.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "tcp_ip.h"

//--- module globals ----------------------------------------------------------------

#define FIFO_SIZE	64

typedef struct SCilentThreadPar
{
	HANDLE			hthread;	
	HANDLE			hserver;
	RX_SOCKET		socket;
	msg_handler 	handle_msg;
	open_handler	handle_open;
	close_handler	handle_close;
	int				sent_tick;
	int				sendThreadRunning;
	
	HANDLE			hsendthread;
	HANDLE			sendSem;
	int				msgTime;
	int				msglen[FIFO_SIZE];
	BYTE			msgbuf[FIFO_SIZE][MAX_MESSAGE_SIZE];
	int				msgInIdx;
	int				msgOutIdx;
} SCilentThreadPar;

typedef struct SServer
{
	RX_SOCKET			socket;
	int					maxConnections;
	int					running;
	HANDLE 				mutex;
	msg_handler 		handle_msg;
	open_handler		handle_open;
	close_handler		handle_close;
	SCilentThreadPar	threadPar[1];	// generic 
} SServer;

typedef struct
{
	RX_SOCKET		socket;
	struct sockaddr *paddr;
	HANDLE			hdone;
	int				done;
} SConnectPar;

typedef struct
{
	RX_SOCKET	socket;
	int			ifindex;
	UINT64		macAddr;
	UINT32		ipAddr;
	UINT32		port;
} SRxRawSocket;

// static HANDLE 	sok_mutex=NULL;

//--- prototypes -------------------------------------------------------------------
static void *_tcp_connect_thread(void* lpParameter);
static void *_server_thread(void* lpParameter);
static void *_client_thread_tcp(void* lpParameter);
static void *_client_thread_tcp_send(void* lpParameter);
static void *_client_thread_udp(void* lpParameter);

static int _wsa_started=0;
static char _TermStr[256]={0};
static int _Clinent_overflowError=FALSE;
static RX_SOCKET	_DebugSocket=INVALID_SOCKET;

#ifdef WIN32

	#ifdef WIN_ADDR
	int _get_adapter_info(const char *deviceName, UINT64 *macAddr, char *ipAddr, int *pidx)
	{
		int s=0, i;
		IP_ADAPTER_INFO *pbuffer, *pitem;

		GetAdaptersInfo(NULL, &s);
		pbuffer = (IP_ADAPTER_INFO*)malloc(s);
		if (pbuffer && !GetAdaptersInfo(pbuffer, &s))
		{
			for (pitem=pbuffer, i=0; pitem; pitem=pitem->Next, i++)
			{
				printf("Adapter[%d].name=%s\n", i, pitem->AdapterName);
				pitem=pitem;
			}
		}
		if (pbuffer) free(pbuffer);

		ULONG result, size, idx;
		wchar_t wname[MAX_PATH];
		IP_ADAPTER_ADDRESSES	*adresses, *paddr;

		*macAddr = 0;
		*ipAddr  = 0;
		*pidx    = -1;
		size   = 0;

		if (GetAdaptersAddresses(AF_INET, 0, NULL, NULL, &size)==ERROR_BUFFER_OVERFLOW)
		{
			adresses = (IP_ADAPTER_ADDRESSES*)malloc(size);
			result = GetAdaptersAddresses(AF_INET, 0, NULL, adresses, &size);
			if (result==NO_ERROR)
			{
				char_to_wchar(deviceName, wname, SIZEOF(wname));
				for (paddr=adresses, idx=0; paddr; paddr=paddr->Next, idx++) 
				{			
					if (!wcsicmp(wname, (wchar_t*)paddr->FriendlyName))
					{
						*pidx = idx;
						memcpy(macAddr, paddr->PhysicalAddress, 6);
						LPSOCKADDR pipAddr;
						printf("Adapter.Name=%s\n", paddr->AdapterName);
						pipAddr = paddr->FirstUnicastAddress->Address.lpSockaddr;
						snprintf(ipAddr, size, "%d.%d.%d.%d", (UCHAR)pipAddr->sa_data[2], (UCHAR)pipAddr->sa_data[3], (UCHAR)pipAddr->sa_data[4], (UCHAR)pipAddr->sa_data[5]);	
						free(adresses);
						return REPLY_OK;
					}
				}
			}
			free(adresses);
		}
		return REPLY_ERROR;
	}
	#else // WIN_ADDR
	int _get_adapter_info(const char *deviceName, UINT64 *macAddr, char *ipAddr, int *pidx)
	{
		*macAddr= 0;
		strcpy(ipAddr, "localhost");
		*pidx   = 0;
		return REPLY_OK;
	}
	#endif // WIN_ADDR
#endif // WIN32 

#ifdef WIN32
int	sok_get_ifconfig(const char *ifname, SIfConfig *config)
{
	memset(config, 0, sizeof(SIfConfig));
	gethostname(config->hostname, sizeof(config->hostname));
	return REPLY_ERROR;
}
int	sok_set_ifconfig(const char *ifname, SIfConfig *config)
{
	return REPLY_ERROR;
}

//--- sok_get_ifcnt ----------------------------
int	sok_get_ifcnt(const char *grep)
{
	return 0;							
}

//--- sok_get_mac_address ----------------------------------------------------
void sok_get_mac_address(const char *deviceName, UINT64 *macAddr)
{
	char ipaddr[64];
	int idx;
	_get_adapter_info(deviceName, macAddr, ipaddr, &idx);
}
#endif // WIN32

#ifdef WIN32
//--- sok_get_ip_address_str -------------------------------------------------------
void sok_get_ip_address_str (const char *deviceName, char *ipAddr, int size)
{
	int i=0;
	i=1/i;
}
#endif // WIN32

#ifdef WIN32
//--- sok_set_ip_address_num ----------------------------------------------------------
void sok_set_ip_address_num (const char *deviceName, unsigned char addr[4])
{
	int i=0;
	i=1/i;
}

//--- sok_get_ip_address_num ----------------------------------------------------------
void sok_get_ip_address_num (const char *deviceName, UINT32 *addr)
{
	*addr = sok_addr_32(RX_CTRL_MAIN);
}
#endif // WIN32

//--- sok_set_ip_address_str -------------------------------------------------------
#ifdef WIN32
#ifdef WIN_ADDR
int sok_set_ip_address_str (const char *deviceName, const char *addr)
{
	char params[200];
	sprintf(params, "interface ipv4 set address name=%s static %s", deviceName, addr);
//	rx_process_execute(params, "D:/Temp/netsh.txt", 0);
	SHELLEXECUTEINFO shExInfo = {0};
	shExInfo.cbSize = sizeof(shExInfo);
	shExInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	shExInfo.hwnd = 0;
	shExInfo.lpVerb = "runas";           // Operation to perform
	shExInfo.lpFile = "netsh.exe";       // Application to start    
	shExInfo.lpParameters = params;          // Additional parameters
	shExInfo.lpDirectory = 0;
	shExInfo.nShow = SW_SHOW;
	shExInfo.hInstApp = 0;  

	if (ShellExecuteEx(&shExInfo))
	{
		WaitForSingleObject(shExInfo.hProcess, INFINITE);
		CloseHandle(shExInfo.hProcess);
	}
	return REPLY_OK;
	// HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Tcpip\Parameters\Interfaces\{E150598B-7C15-4FBC-A467-91A9675F7955} IPAddress
}
#else // WIN_ADDR
int sok_set_ip_address_str (const char *deviceName, const char *addr)
{
	return REPLY_OK;
}
#endif // WIN_ADDR
#endif // WIN32

#ifdef linux
//--- sok_get_mac_address --------------------------------------------------------
void sok_get_mac_address(const char *deviceName, UINT64 *macAddr)
{
	// Documentation: http://linux.die.net/man/7/netdevice
	int fd, ret;
	struct ifreq ifr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);

	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, deviceName, IFNAMSIZ - 1);

	if (ioctl(fd, SIOCGIFHWADDR, &ifr)==0) 
		memcpy(macAddr, ifr.ifr_hwaddr.sa_data, 6);
	else 	
		*macAddr=0; 

	close(fd);
}
#endif // linux


#ifdef linux

//--- sok_get_ifconfig ------------------------------------------------
int sok_get_ifconfig(const char *ifname, SIfConfig *pcfg)
{
	FILE *file;
	char str[256];
	int pos;
	int ret = REPLY_ERROR;

	memset(pcfg, 0, sizeof(*pcfg));
	file = fopen("/etc/hostname", "rt");
	if (file!=NULL)
	{
		fgets(pcfg->hostname, sizeof(pcfg->hostname), file);
		char *ch;
		for (ch=pcfg->hostname; *ch>=' '; ch++)
		{
		}
		*ch=0;
		fclose(file);
	}
	file = fopen("/etc/network/interfaces", "rt");
	if (file!=NULL)
	{
		while(fgets(str, sizeof(str), file))
		{
			if (str_start(str, "#")) continue;						
			if (ret==REPLY_OK) // found
			{
				if (strlen(str)<5) break; 
				if (str_start(str, "dhcp")) pcfg->dhcp=TRUE;
				sscanf(str, "address %d.%d.%d.%d", (int*)&pcfg->addr[0], (int*)&pcfg->addr[1], (int*)&pcfg->addr[2], (int*)&pcfg->addr[3]);
				sscanf(str, "netmask %d.%d.%d.%d", (int*)&pcfg->mask[0], (int*)&pcfg->mask[1], (int*)&pcfg->mask[2], (int*)&pcfg->mask[3]);
			}
			else if (strstr(str, ifname)) 
				ret=REPLY_OK;
		}
		fclose(file);
	}
	if (!pcfg->mask[0] && !pcfg->mask[1] && !pcfg->mask[2] && !pcfg->mask[3])
		pcfg->mask[0] = pcfg->mask[1] = pcfg->mask[2] = 255;  
	return ret;
}

//--- _write_config -------------------------------------------
static void _write_config(FILE *out, const char *ifname, SIfConfig *pcfg)
{
	if (pcfg->dhcp)
	{
		fprintf(out, "auto %s\n", ifname);
		fprintf(out, "iface %s inet dhcp\n", ifname);						
	}
	else 
	{
		if (pcfg->addr[0] || pcfg->addr[1] || pcfg->addr[2] || pcfg->addr[3]) 
			fprintf(out, "auto %s\n", ifname);
		fprintf(out, "iface %s inet static\n", ifname);						
		fprintf(out, "address %d.%d.%d.%d\n", pcfg->addr[0],  pcfg->addr[1],  pcfg->addr[2], pcfg->addr[3]);						
		fprintf(out, "netmask %d.%d.%d.%d\n", pcfg->mask[0],  pcfg->mask[1],  pcfg->mask[2], pcfg->mask[3]);						
	}
	fprintf(out, "\n");								
}

//--- sok_set_ifconfig ---------------------------------------
int	sok_set_ifconfig(const char *ifname, SIfConfig *pcfg)
{
	SIfConfig act;
	
	sok_get_ifconfig(ifname, &act);
	
	if (strcmp(pcfg->hostname, act.hostname))
	{
		FILE *file;		
		chmod("/etc/hostname", 0666);
		file = fopen("/etc/hostname", "wt");
		if (file!=NULL)
		{
			fprintf(file, "%s\n", pcfg->hostname);
			fclose(file);
		}
		Error(LOG, 0, "Write Hostname=>>%s<<", pcfg->hostname);
	}		
	
	if (pcfg->dhcp!=act.dhcp || memcmp(pcfg->addr, act.addr, sizeof(pcfg->addr)) || memcmp(pcfg->mask, act.mask, sizeof(pcfg->mask)))
	{
		FILE *file, *out;
		int i;
		int found=0;
		char str[256];
		
		file = fopen("/etc/network/interfaces", "rt");
		out =  fopen("/tmp/interfaces", "wt");
		if (file!=NULL)
		{
			while(fgets(str, sizeof(str), file))
			{
				if (found!=1 && str_start(str, "#")) 
				{
					fwrite(str, 1, strlen(str), out);
					continue;
				}
				if (found==0 && strstr(str, ifname)) 
				{
					found = 1;
					_write_config(out, ifname, pcfg);
				}
				if (found==1) // found, skip
				{
					if(strlen(str)<5) found=2; 
				}
				else fwrite(str, 1, strlen(str), out);
			}
			if (!found) _write_config(out, ifname, pcfg);

			fclose(file);
			fclose(out);
			chmod("/etc/network/int5erfaces", 0666);
			system("mv /tmp/interfaces /etc/network/interfaces");

			if (pcfg->dhcp)
			{
				sprintf(str, "ifconfig %s down", ifname); system(str);
				sprintf(str, "dhclient %s", ifname); system(str);
			}
			else
			{
				sprintf(str, "ifconfig %s down", ifname); system(str);
				sprintf(str, "dhclient -r %s", ifname); system(str);
				sprintf(str, "ifconfig %s %d.%d.%d.%d", ifname, pcfg->addr[0],  pcfg->addr[1],  pcfg->addr[2], pcfg->addr[3]); system(str);					
				sprintf(str, "ifconfig %s netmask %d.%d.%d.%d", ifname, pcfg->mask[0],  pcfg->mask[1],  pcfg->mask[2], pcfg->mask[3]); system(str);					
				sprintf(str, "ifconfig %s up", ifname); system(str);
			}
		}
	}
	return REPLY_OK;
}

//--- sok_get_ifcnt -----------------------------------
int	sok_get_ifcnt(const char *grep)
{
	char cmd[64];
	FILE *f;
	sprintf(cmd, "ls -A /sys/class/net | grep %s | wc -l", grep);
	f = popen(cmd, "r");
	char str[32];
	fgets(str, sizeof(str), f);
	pclose(f);
	return atoi(str);
}

//--- sok_get_ip_address_num -------------------------------------------------------
void sok_get_ip_address_num (const char *deviceName, UINT32 *addr)
{
	// Documentation: http://linux.die.net/man/7/netdevice
	int fd, ret;
	struct ifreq ifr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, deviceName, IFNAMSIZ - 1);
	ifr.ifr_addr.sa_family = AF_INET;
	ret = ioctl(fd, SIOCGIFADDR, &ifr);
	memcpy(addr, &ifr.ifr_addr.sa_data[2], sizeof(UINT32));
//	snprintf(addr, size, "%d.%d.%d.%d", (UCHAR)ifr.ifr_addr.sa_data[2], (UCHAR)ifr.ifr_addr.sa_data[3], (UCHAR)ifr.ifr_addr.sa_data[4], (UCHAR)ifr.ifr_addr.sa_data[5]);
	close(fd);
}

//--- sok_get_ip_address_str -------------------------------------------------------
void sok_get_ip_address_str (const char *deviceName, char *addr, int size)
{
	// Documentation: http://linux.die.net/man/7/netdevice
	int fd, ret;
	struct ifreq ifr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, deviceName, IFNAMSIZ - 1);
	ifr.ifr_addr.sa_family = AF_INET;
	ret = ioctl(fd, SIOCGIFADDR, &ifr);	
	if(ret) *addr = 0;
	else	snprintf(addr, size, "%d.%d.%d.%d", (UCHAR)ifr.ifr_addr.sa_data[2], (UCHAR)ifr.ifr_addr.sa_data[3], (UCHAR)ifr.ifr_addr.sa_data[4], (UCHAR)ifr.ifr_addr.sa_data[5]);
	close(fd);
}
#endif // linux


#ifdef linux
//--- sok_set_ip_address_num ----------------------------------------------------------
void sok_set_ip_address_num (const char *deviceName, unsigned char addr[4])
{
	// Documentation: http://linux.die.net/man/7/netdevice
	int fd;
	struct ifreq ifr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);

	strncpy(ifr.ifr_name, deviceName, IFNAMSIZ - 1);
	ifr.ifr_addr.sa_family = AF_INET;
	memcpy(&ifr.ifr_addr.sa_data[2], addr, 4);

	ioctl(fd, SIOCSIFADDR, &ifr);

	close(fd);
}
#endif // linux

#ifdef linux
//--- sok_set_ip_address_str -------------------------------------------------------
int sok_set_ip_address_str (const char *deviceName, const char *addr)
{
	// Documentation: http://linux.die.net/man/7/netdevice
	int fd, ret;
	int err=0;
	struct ifreq ifr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, deviceName, IFNAMSIZ - 1);
	ifr.ifr_addr.sa_family = AF_INET;
	if (inet_pton(AF_INET, addr, ifr.ifr_addr.sa_data + 2)!=1) err=errno;
	else if (ioctl(fd, SIOCSIFADDR, &ifr)==-1) err=errno;
	close(fd);
	return err;
}
#endif // linux

//--- sok_get_addr_str --------------------------------------
char* sok_get_addr_str(struct sockaddr *addr, char *str, int size)
{
	if (addr)
	{
		UCHAR *a = (UCHAR*)&addr->sa_data[2];
		snprintf(str, size, "%d.%d.%d.%d", a[0], a[1], a[2], a[3]);
	}
	else sprintf(str, "<NULL>");
	return str;		
}

//--- sok_mac_addr_str ---------------------------------------------
char*	sok_mac_addr_str(UINT64 macAddr, char *str, int size)
{
	*str=0;
	if (size>18)
	{
		UCHAR *ch = (UCHAR*)&macAddr;
		sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x", ch[0], ch[1], ch[2], ch[3], ch[4], ch[5]);
	}
	return str;
}

//--- sok_addr_32 ----------------------------------------
UINT32	sok_addr_32	(const char *addrStr)
{
	UINT32 addr;
	struct sockaddr_in ipAddr;
	sok_sockaddr(&ipAddr, addrStr, 0);

	memcpy(&addr, &ipAddr.sin_addr, sizeof(addr));
	return addr;
}

//---- sok_addr_str --------------------------------------
char*   sok_addr_str (UINT32 addr, char *str)
{
	UCHAR *no = (UCHAR*)&addr;
	sprintf(str, "%d.%d.%d.%d", no[0], no[1], no[2], no[3]);
	return str;	
}

//--- sok_get_peer_name ---------------------------------------------------------------
char* sok_get_peer_name(RX_SOCKET socket, char *namestr, char *ipstr, UINT32 *pport)
{
	struct sockaddr sender;
	unsigned  len = sizeof(sender);

	if (namestr) *namestr=0;
	if (ipstr)	 *ipstr=0;
	if (pport)	 *pport=0;
	if (socket!=0 && socket!=INVALID_SOCKET) 
	{	
		if (getpeername(socket, &sender, &len)==0)
		{
			if (ipstr!=NULL) sprintf(ipstr, "%d.%d.%d.%d", (UCHAR)sender.sa_data[2], (UCHAR)sender.sa_data[3], (UCHAR)sender.sa_data[4], (UCHAR)sender.sa_data[5]);
			if (pport!=NULL) *pport = (UCHAR)sender.sa_data[0]<<8|(UCHAR)sender.sa_data[1];
			if (namestr!=NULL) sprintf(namestr, "%d.%d.%d.%d:%d", (UCHAR)sender.sa_data[2], (UCHAR)sender.sa_data[3], (UCHAR)sender.sa_data[4], (UCHAR)sender.sa_data[5], (UCHAR)sender.sa_data[0]<<8|(UCHAR)sender.sa_data[1]);
		}
	}
	return namestr;
}

//--- sok_get_peer_name_raw ---------------------------------------------------------------
char* sok_get_peer_name_raw(RX_RAW_SOCKET socket, char *namestr, char *ipstr, UINT32 *pport)
{	
	SRxRawSocket *psok = (SRxRawSocket*)&socket;
	return sok_get_peer_name(psok->socket, namestr, ipstr, pport);
}

//--- sok_get_socket_name ---------------------------------------------------------------
char *sok_get_socket_name(RX_SOCKET socket, char *namestr, char *ipstr, UINT32 *pport)
{
	struct sockaddr sender;
	unsigned  len = sizeof(sender);
	
	memset(&sender, 0, sizeof(sender));
	getsockname(socket, &sender, &len);
	if (ipstr!=NULL) sprintf(ipstr, "%d.%d.%d.%d", (UCHAR)sender.sa_data[2], (UCHAR)sender.sa_data[3], (UCHAR)sender.sa_data[4], (UCHAR)sender.sa_data[5]);
	if (pport!=NULL) *pport = (UCHAR)sender.sa_data[0]<<8|(UCHAR)sender.sa_data[1];
	if (namestr!=NULL) sprintf(namestr, "%d.%d.%d.%d:%d", (UCHAR)sender.sa_data[2], (UCHAR)sender.sa_data[3], (UCHAR)sender.sa_data[4], (UCHAR)sender.sa_data[5], (UCHAR)sender.sa_data[0]<<8|(UCHAR)sender.sa_data[1]);
	return namestr;
}

//--- sok_get_server_name ---------------------------------
void	sok_get_server_name(HANDLE hserver, char *namestr, char *ipstr, UINT32 *pport)
{
	SServer *pserver = (SServer*)hserver;
	sok_get_socket_name(pserver->socket, namestr, ipstr, pport);
}

//--- sok_get_addr_32 -----------------------
UINT32	sok_get_addr_32(RX_SOCKET socket)
{
	struct sockaddr sender;
	unsigned  len = sizeof(sender);
	if (socket!=0 && socket!=INVALID_SOCKET) 
	{	
		if (getsockname(socket, &sender, &len)==0)
		{
			return *((UINT32*)&sender.sa_data[2]);
		}
	}
	return 0;
}

//--- sok_get_socket_name_raw ---------------------------------------------------------------
char *sok_get_socket_name_raw(RX_RAW_SOCKET socket, char *namestr, char *ipstr, UINT32 *pport)
{
	SRxRawSocket *psok = (SRxRawSocket*)&socket;
	return sok_get_socket_name(psok->socket, namestr, ipstr, pport);
}

//--- sok_get_srv_socket_name ---------------------------------
void	sok_get_srv_socket_name(HANDLE hserver, char *namestr, char *ipstr, UINT32 *pport)
{
	SServer *pserver = (SServer*)hserver;
	int thread;
	if (namestr!=NULL)	*namestr = 0;
	if (ipstr!=NULL)	*ipstr   = 0;
	for (thread = 0; thread < pserver->maxConnections; thread++)
	{
		if (pserver->threadPar[thread].socket)
		{
			sok_get_socket_name(pserver->threadPar[thread].socket, namestr, ipstr, pport);
			return;
		}
	}
}

//--- sok_error --------------------------------------
static int sok_error(RX_SOCKET *psocket)
{
	int err;
#ifdef WIN32
	err = WSAGetLastError();
#else // WIN32
	err = errno;
#endif // WIN32
	sok_close(psocket);
	return err;
}

//--- sok_sockaddr --------------------------------------------------------------------------------
int sok_sockaddr(struct sockaddr_in *ipAddr, const char *addr, int port)
{
//	rx_mutex_lock(sok_mutex);

	ipAddr->sin_family = AF_INET;
	ipAddr->sin_port = htons((short)port);
	if (addr)
	{
		//--- Resolve host name ---
		#ifdef linux
		if (inet_pton(ipAddr->sin_family, addr, &ipAddr->sin_addr)) return REPLY_OK;
		#else
			UCHAR a[20];
			if (sscanf(addr, "%d.%d.%d.%d", &a[0], &a[1], &a[2], &a[3])==4) 
			{
				memcpy(&ipAddr->sin_addr, a, 4);
				return REPLY_OK;
			}
		#endif

		struct hostent *hostInfo;
		hostInfo = gethostbyname(addr);				// (gethostbyname allocates one hostent structure per thread)
		if (hostInfo == NULL) return sok_error(NULL);
		memcpy((char *)&(ipAddr->sin_addr), (char *)hostInfo->h_addr_list[0], hostInfo->h_length);
	}
	else ipAddr->sin_addr.s_addr = INADDR_ANY;

//	rx_mutex_unlock(sok_mutex);

	return REPLY_OK;
}

//--- _sok_server_error -----------------------------------
static int _sok_server_error(SServer *pserver)
{
	int err=sok_error(&pserver->socket);
	free(pserver);
	return err;
}

//--- sok_start_server ----------------------------------------------------------
int sok_start_server(HANDLE *hserver, const char *addr, int port, int type, int maxConnections, msg_handler handle_msg, open_handler handle_open, close_handler handle_close)
{
	SServer	*pserver;
	struct sockaddr_in ipAddr;
	INT32 	init;
	int 	size;
	char	name[64];

//	if (sok_mutex==NULL) sok_mutex = rx_mutex_create();

	*hserver = NULL;
	if (type != SOCK_STREAM && type != SOCK_DGRAM) return REPLY_ERROR;

	if (type == SOCK_STREAM)	size = sizeof(SServer)+(maxConnections - 1)*sizeof(SCilentThreadPar);
	else						size = sizeof(SServer);
	pserver = (SServer*)malloc(size);
	if (pserver == NULL) return REPLY_ERROR;
	memset(pserver, 0, size);

	//--- threading ---------------------------------------
	pserver->running = TRUE;
	pserver->maxConnections = maxConnections;

	pserver->mutex = rx_mutex_create();

#ifdef WIN32
	// if (!_wsa_started)
	{
		_wsa_started = TRUE;

		WSADATA	data;
		WORD version;

		//--- Startup Winsockets ---
		version = MAKEWORD(2, 2);
		if (WSAStartup(version, &data))
		{
			version = MAKEWORD(1, 1);
			if (WSAStartup(version, &data)) return sok_error(NULL);
		}
	}
#endif // WIN32

	//--- open server socket -------------------------------
	pserver->handle_msg   = handle_msg;
	pserver->handle_open  = handle_open;
	pserver->handle_close = handle_close;
	pserver->socket 	  = socket(AF_INET, type, 0);
	if (pserver->socket == INVALID_SOCKET) return _sok_server_error(NULL);
	
	sok_sockaddr(&ipAddr, addr,  port);	
	init=1;
	//	if (setsockopt(pserver->socket, SOL_SOCKET, SO_DONTROUTE, (char*)&init, sizeof (init)))	return _sok_server_error(pserver->socket);
	if (setsockopt (pserver->socket, SOL_SOCKET, SO_REUSEADDR, (char*)&init, sizeof (init)))	return _sok_server_error(pserver);

	if (type == SOCK_STREAM)
	{
		if (setsockopt (pserver->socket, IPPROTO_TCP, TCP_NODELAY,  (char*)&init, sizeof (init)))	return _sok_server_error(pserver);
	}
	if (bind       (pserver->socket, (struct sockaddr *) &ipAddr, sizeof (ipAddr)))				return _sok_server_error(pserver);

	if (type == SOCK_STREAM)
	{
		if (listen(pserver->socket, pserver->maxConnections) == SOCKET_ERROR) return _sok_server_error(pserver);
		pserver->running = TRUE;
		sprintf(name, "TCP/IP Server %s:%d", addr, port);
		rx_thread_start(_server_thread, pserver, 0, name);
	}
	else
	{
		int thread = 0;
		pserver->threadPar[thread].socket		= pserver->socket;
		pserver->threadPar[thread].handle_msg	= handle_msg;
		sprintf(name, "UDP Server %s:%d", addr, port);
		pserver->threadPar[thread].hthread		= rx_thread_start(_client_thread_udp, &pserver->threadPar[thread], 0, name);
	}
	*hserver = pserver;
	return REPLY_OK;
}

//--- sok_stop_server -----------------------------------------------
int sok_stop_server (HANDLE    *hserver)
{
	int thread;
	SServer *pserver = (SServer*)*hserver;

	if (pserver==NULL) return REPLY_OK;
	if (pserver->mutex) 
	{
		rx_mutex_lock(pserver->mutex);
		rx_mutex_destroy(&pserver->mutex);
	}
	pserver->running = FALSE;

	for (thread=0;  thread<pserver->maxConnections; thread++)
	{
		sok_close(&pserver->threadPar[thread].socket);
	}
	sok_close(&pserver->socket);
	free(*hserver);
	*hserver = NULL;
	return REPLY_OK;
}

//--- sok_ping ------------------------------------------------------
int sok_ping(const char *addr)
{
	char cmd[100];

	if (!*addr) return REPLY_ERROR;

	#ifdef linux
//		sprintf(cmd, "ping %s -l 1 -c 1 -w 1 > /dev/null", addr);
		sprintf(cmd, "ping %s -c 1 -w 1 > /dev/null", addr);
		/*
		FILE* fp;
		char str[100];
		fp = popen(cmd, "r");
		while (!feof(fp))
		{
			fgets(str, 100, fp);
			printf("%s\n", str);
		}
		pclose(fp);
		*/
		int ret= system(cmd);
		return ret;
	#else
		sprintf(cmd, "ping -n 1 -w 20 %s", addr);
		return rx_process_execute(cmd, NULL, 0);
	#endif
}

//--- sok_open_client --------------------------------------------------------------
int sok_open_client(RX_SOCKET *psocket, const char *addr, int port, int type)
{
	//---opens TCP/IP (stream) Socket -------------

//	struct hostent *hostInfo;
	int init = TRUE;
	struct sockaddr_in ipAddr;
	RX_SOCKET sok;
	int time0, time;

	if (type != SOCK_STREAM && type != SOCK_DGRAM) return REPLY_ERROR;

//	if (type==SOCK_STREAM && sok_ping(addr)) return REPLY_ERROR;

#ifdef WIN32
	// if (!_wsa_started)
	{
		_wsa_started = TRUE;

		WSADATA	data;
		WORD version;
		//--- Startup Winsockets ---
		version = MAKEWORD(2, 2);
		if (WSAStartup(version, &data))
		{
			version = MAKEWORD(1, 1);
			if (WSAStartup(version, &data)) return sok_error(NULL);
		}
	}
#endif // WIN32
	
	//--- Get socket ---
	sok = socket(AF_INET, type, 0);
	if (sok == INVALID_SOCKET) return sok_error(NULL);
	
	time0 = rx_get_ticks();
	sok_sockaddr(&ipAddr, addr, port);
	time = rx_get_ticks()-time0;
	
	if (time>100) Error(WARN, 0, "sok_sockaddr takes %d ms", time);

	if (type == SOCK_STREAM)
	{
		if (setsockopt(sok, IPPROTO_TCP, TCP_NODELAY, (char*)&init, sizeof (init))) return sok_error(&sok);
	}
	#ifdef linux_NOOOO
	{
		int flags = 0, ret, error=0;
		fd_set rset, wset;
		socklen_t   len = sizeof(error);
		struct timeval tio;

		FD_ZERO(&rset);
		FD_SET(sok, &rset);
		wset = rset;
		tio.tv_sec  = 1;
		tio.tv_usec = 0;

		if( (flags = fcntl(sok, F_GETFL, 0)) < 0) return sok_error(&sok);
		fcntl(sok, F_SETFL, flags | O_NONBLOCK);
		ret = connect(sok, (struct sockaddr *)&ipAddr, sizeof(ipAddr));
		if (ret)
		{
			if (errno==EINPROGRESS)
			{
				ret = select((sok)+1, &rset, &wset, NULL, &tio);
				if (ret<0)  return sok_error(&sok);
				if (ret==0) 
				{
					errno = ETIMEDOUT;
					return sok_error(&sok);
				}
				    //we had a positivite return so a descriptor is ready
				if (FD_ISSET(sok, &rset) || FD_ISSET(sok, &wset))
				{
					if(getsockopt(sok, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
						return sok_error(&sok);
				}
			}
			else  return sok_error(&sok);
		}
		fcntl(sok, F_SETFL, flags | O_NONBLOCK);
	}
	#else // linux
	{
		// here socket MUST be blocking!!!
		#ifndef soc
		int ret;

		// ret = sok_ping(addr);	// ping activates connection! 
		ret=0;
		if (type==SOCK_STREAM && ret) 
		{
			#ifdef WIN32
				WSASetLastError(WSAETIMEDOUT);
			#else
				errno = ETIMEDOUT;
			#endif
			return sok_error(&sok);
		}
		#endif

		/*
		if (type==SOCK_DGRAM)
		{
			UINT32 val=0;
			UINT32 len=4;
			int ret;
			ret=getsockopt(sok, SOL_SOCKET, SO_SNDBUF, &val, &len);
			val = 1024*256;
			ret=setsockopt(sok, SOL_SOCKET, SO_SNDBUF, &val, len);
			ret=getsockopt(sok, SOL_SOCKET, SO_SNDBUF, &val, &len);
			val=val;
		}
		*/

		if (connect(sok, (struct sockaddr*)&ipAddr, sizeof (struct sockaddr))) return sok_error(&sok);

		/*
		HANDLE thread;
		SConnectPar par;
		par.socket	= sok;
		par.paddr   = (struct sockaddr*)&ipAddr;
		par.done	= -1;
		par.hdone   = rx_sem_create();
		thread = rx_thread_start(_tcp_connect_thread, &par, 0, "tcp_connect_thread");
		rx_sem_wait(par.hdone, 1000);
		rx_sem_destroy(&par.hdone);
//		TerminateThread(thread, 0);	// terminates also other threads!
		CloseHandle(thread);		
		if (par.done) 	
		{
			sok_close(&sok);
			return REPLY_ERROR;
		}
		*/
	}
	#endif // linux
	*psocket = sok;

	return 0;
}

//--- _tcp_connect_thread ----------------------------------
static void *_tcp_connect_thread(void* lpParameter)
{
	SConnectPar *par= (SConnectPar*)lpParameter;
	
	printf("Connect\n");
	par->done = connect(par->socket, par->paddr, sizeof (*par->paddr));
	// if (par->done) par->done = sok_error(&par->socket);
	// printf("Connected err=%d\n", WSAGetLastError());
	rx_sem_post(par->hdone);
	return NULL;
}

//--- sok_open_client_2 --------------------------------------------------------------
int sok_open_client_2(RX_SOCKET *psocket, const char *addr, int port, int type, msg_handler handle_msg, close_handler handle_close)
{
	int ret = sok_open_client(psocket, addr, port, type);
	if (ret==REPLY_OK)
	{
		char name[64];
		sprintf(name, "TCP-Client %s:%d", addr, port);
		
		SCilentThreadPar *par = (SCilentThreadPar*)malloc(sizeof(SCilentThreadPar));
		memset(par, 0, sizeof(SCilentThreadPar));
		par->hserver		= NULL;
		par->socket			= *psocket;
		par->handle_msg		= handle_msg;
		par->handle_close	= handle_close;
		par->hthread		= rx_thread_start(_client_thread_tcp, par, 0, name);
	}
	return ret;
}

//--- _server_thread --------------------------------------------------------
static void* _server_thread(void* lpParameter)
{
	SServer *pserver = (SServer*)lpParameter;
	RX_SOCKET clientSocket;
	char addr_svr[32];
	char addr_clnt[32];
	char name[64];
	int  errTooMany=FALSE;

	TrPrintfL(1, "sok:_server_thread %s started", sok_get_socket_name(pserver->socket, addr_svr, NULL, NULL));
	while (pserver->running)
	{
		clientSocket = accept(pserver->socket, NULL, 0);
		if (clientSocket != INVALID_SOCKET)
		{
			TrPrintfL(1, "Client Connected: %s to %s socket=%d", sok_get_peer_name(clientSocket, addr_clnt, NULL, NULL), addr_svr, clientSocket);
			int thread;
			for (thread = 0; thread<pserver->maxConnections; thread++)
			{
				if (pserver->threadPar[thread].hthread == 0)
				{
					TrPrintfL(1, "Connections=%d (max=%d)", thread+1, pserver->maxConnections);
					pserver->threadPar[thread].hserver		= pserver;
					pserver->threadPar[thread].socket		= clientSocket;
					pserver->threadPar[thread].handle_msg   = pserver->handle_msg;
					pserver->threadPar[thread].handle_open  = pserver->handle_open;
					pserver->threadPar[thread].handle_close = pserver->handle_close;
					pserver->threadPar[thread].sendSem		= rx_sem_create();
					pserver->threadPar[thread].sendThreadRunning = TRUE;

					sprintf(name, "TCP Client %s", addr_clnt);
					pserver->threadPar[thread].hthread	   = rx_thread_start(_client_thread_tcp,	  &pserver->threadPar[thread], 0, name);
					sprintf(name, "TCP Client-Send %s", addr_clnt);
					pserver->threadPar[thread].hsendthread = rx_thread_start(_client_thread_tcp_send, &pserver->threadPar[thread], 0, name);
					break;
				}
			}
			if (thread>=pserver->maxConnections)
			{
				if (!errTooMany) Error(ERR_CONT, 0, "Too many conenctions");
				errTooMany = TRUE;
				sok_close(&clientSocket);
			}
			else errTooMany=FALSE;
		}
	}
	TrPrintfL(1, "sok: _server_thread %s ended", addr_svr);
	return 0;
}

//--- _client_thread_tcp --------------------------------------------------------
static void* _client_thread_tcp(void* lpParameter)
{
	SCilentThreadPar *par = (SCilentThreadPar*)lpParameter;
	char peerName[64];
	RX_SOCKET socket = par->socket;
	sok_get_peer_name(par->socket, peerName, NULL, NULL);
	if (par->handle_open) par->handle_open(par->socket, peerName);
	sok_receiver(par->hserver, &par->socket, par->handle_msg, NULL);
	par->sendThreadRunning=FALSE;
	if (par->handle_close) par->handle_close(socket, peerName);
	if (par->sendSem) rx_sem_post(par->sendSem);	
	if (par->hserver) memset(par, 0, sizeof(*par));
	else free(par);
	return 0;
}

//--- _client_thread_tcp_send --------------------------------------------------------
static void* _client_thread_tcp_send(void* lpParameter)
{
	SCilentThreadPar *par = (SCilentThreadPar*)lpParameter;
	int sent;
	
	while (par->hserver!=NULL && par->sendSem && par->sendThreadRunning)
	{
		rx_sem_wait(par->sendSem, 0);
		while(par->msgOutIdx!=par->msgInIdx)
		{
			if (!par->sendThreadRunning) break;
			sent=send(par->socket, par->msgbuf[par->msgOutIdx], par->msglen[par->msgOutIdx], MSG_NOSIGNAL);
			if (sent==SOCKET_ERROR)
			{
				char peerName[64];
				char errStr[200];
				int err;
				err = sok_error(&par->socket);
				err_system_error(err, errStr, sizeof(errStr));
				sok_get_peer_name(par->socket, peerName, NULL, NULL);
				Error(LOG, 0, "_client_thread_tcp_send >>%s<<, error=>>%s<<", peerName, errStr);
			}
			par->msgTime = 0; 
			par->msglen[par->msgOutIdx]=0;
			par->msgOutIdx = (par->msgOutIdx+1) % FIFO_SIZE;
		}
	}
	if (par->sendSem) rx_sem_destroy(&par->sendSem);
	return 0;			
}

//--- sok_receiver --------------------------------------------------------
#define BUFFER_SIZE MAX_MESSAGE_SIZE
int sok_receiver(HANDLE hserver, RX_SOCKET *psocket, msg_handler handle_msg, void *par)
{
	SServer *pserver = (SServer*)hserver;
	char *buffer = NULL;
	int bufferLen = 2048;

	char addr[32];
	int len, start;
	int reply;
	int	socket = (int)*psocket;
	SMsgHdr		*phdr;
	
	sok_get_peer_name(*psocket, addr, NULL, NULL);
	TrPrintfL(TRUE, "ClientThread started, socket=%d addr %s", *psocket, addr);
	
	//--- set socket to non blocking ---
	/*
	{
		#ifdef linux
			if (fcntl(*psocket, F_SETFL, fcntl(*psocket, F_GETFL, 0) | O_NONBLOCK)==-1) 
				Error(ERR_CONT, 0, "Could not UNBLOCk socket, errno=%d, >>%s<<", sok_error(&socket), err_system_error(errno, buffer, sizeof(buffer)));
		#else // linux
			ULONG iMode = 1;
			if (ioctlsocket(socket, FIONBIO, &iMode)) Error(ERR_CONT, 0, "Could not UNBLOCk socket");
		#endif // linux
	}
	*/

	start = 0;
	reply = REPLY_OK;
	buffer = (char*)malloc(bufferLen);
	while (reply==REPLY_OK)
	{	
		/*
		fd_set read, err;
		TIMEVAL Timeout;
		Timeout.tv_sec = 0;
		Timeout.tv_usec = 1000;

		FD_ZERO(&read);
		FD_ZERO(&err);
		FD_SET(socket, &read);
		FD_SET(socket, &err);
 
		// check if the socket is ready
		select(0,&read,NULL,&err,&Timeout);			
		if(FD_ISSET(socket, &read))
		*/
		{
			len = recv(*psocket, &buffer[start], bufferLen - start, 0);
				
			// TrPrintf(TRUE, "received %d bytes", len);
			if (len == 0)
			{
				#ifdef linux
					if (errno==EWOULDBLOCK) continue;
				#else
					if (WSAGetLastError()==WSAEWOULDBLOCK) continue;
				#endif // linux			
				reply = sok_error(psocket);
				TrPrintf(TRUE, ">>%s<< received %d bytes, err=%d", addr, len, reply);
				break;
			}
			else if (len<0)
			{
				#ifdef linux
					if (errno==EWOULDBLOCK) continue;
				#else
					if (WSAGetLastError()==WSAEWOULDBLOCK) continue;
				#endif // linux			
				reply = sok_error(psocket);
				TrPrintf(TRUE, ">>%s<< received %d bytes, err=%d", addr, len, reply);
			//	{
			//		char str[256];
			//		Error(ERR_CONT, 0, "Socket %s Error: %d >>%s<<", addr, reply, err_system_error(reply, str, sizeof(str)));
			//	}
				return reply;
			}
			else if (handle_msg==NULL)
			{
				if (pserver) rx_mutex_lock(pserver->mutex);
				if (len+1>=sizeof(_TermStr)) len = sizeof(_TermStr)-1;
				memcpy(_TermStr, buffer, len);
				_TermStr[len]=0;
				if (pserver) rx_mutex_unlock(pserver->mutex);
			}
			else
			{
			//	if (len+start>bufferLen)
			//	{
			//		Error(ERR_CONT, 0, "Buffer Overflow");
			//	}
				//--- handle message -------------
				len += start; // complete len
				start = 0;
				while (len >= sizeof(SMsgHdr))
				{
					phdr = (SMsgHdr*)&buffer[start];

					if (phdr->msgLen > bufferLen) 
					{
						int nbufferLen = phdr->msgLen + 256;
						char *nbuffer = (char*)malloc(nbufferLen);
						memmove(nbuffer, &buffer[start], len);
//						TrPrintf(TRUE, "Change size of receive buffer. oldsize=%d, newsize=%d", bufferLen, nbufferLen);
						free(buffer);
						buffer = nbuffer;
						bufferLen = nbufferLen;
						start = 0;
						phdr = (SMsgHdr*)buffer;
					}

					if (phdr->msgLen > len) break;

					if (pserver) rx_mutex_lock(pserver->mutex);
					handle_msg(*psocket, phdr, phdr->msgLen, NULL, par);
					if (pserver) rx_mutex_unlock(pserver->mutex);					
					start += phdr->msgLen;
					len -= phdr->msgLen;
				}
				if (len)
				{
					if (start) memmove(buffer, &buffer[start], len);
					start = len;
				}
				else start = 0;
			}
		}
	}

	free(buffer);

	//--- end using the socket --------------------
	TrPrintfL(TRUE, "ClientThread ended, socket=%d addr %s", socket, addr);
	sok_close(psocket);
	*psocket = INVALID_SOCKET;
	return reply;
}

//--- _client_thread_udp --------------------------------------------------------
static void *_client_thread_udp(void *hserver)
{
	SCilentThreadPar *par = (SCilentThreadPar*)hserver;

	char buffer[MAX_UDP_DATA_SIZE];
	struct sockaddr	sender;
	int len;
	#if __WORDSIZE == 64
		INT64 reply=REPLY_OK;
	#else // __WORDSIZE
		INT32 reply=REPLY_OK;
	#endif // __WORDSIZE
	unsigned addrlen;

	TrPrintfL(1, "UDP ClientThread started, socket=%d\n", par->socket);
	while (!reply)
	{
		addrlen=sizeof(sender);
		len = recvfrom(par->socket, buffer, sizeof(buffer), 0, &sender, &addrlen);
		if (len == 0) break;
		else if (len == SOCKET_ERROR)
		{
			sok_close(&par->socket);
			Error(ERR_CONT, 0, "Socket Error: %d", errno);
			reply = errno;
			break;
		}

		par->handle_msg(par->socket, buffer, len, &sender, NULL);
	}

	//--- end using the socket --------------------
	TrPrintf(1, "_client_thread_udp ended socket=%d\n", par->socket);
	sok_close(&par->socket);
	return (void*)reply;
}

//--- sok_debug -----------------------------------------------------------
void	 sok_debug(RX_SOCKET socket)
{
	_DebugSocket = socket;
}


//--- sok_send -----------------------------------------------------
int sok_send(RX_SOCKET *socket, void *msg)
{
	SMsgHdr* phdr = (SMsgHdr*)msg;
	char	 *debug = (char*)msg;
	int sent;

	if (*socket==0) *socket=INVALID_SOCKET;
	if (*socket!=INVALID_SOCKET)
	{
		if (*socket==_DebugSocket)
			TrPrintfL(TRUE, "DebugSocket msgId=0x%08x, len=%d",  phdr->msgId, phdr->msgLen);
		if (phdr->msgLen > MAX_MESSAGE_SIZE) Error(ERR_CONT, 0, "Message too large, id=0x%08x, len=%d, max=%d", phdr->msgId, phdr->msgLen, MAX_MESSAGE_SIZE);
		else 
		{
			/*
			fd_set write, err;
			TIMEVAL timeout;
			timeout.tv_sec = 0;
			timeout.tv_usec = 1000;


			FD_ZERO(&write);
			FD_ZERO(&err);
			FD_SET(socket, &write);
			FD_SET(socket, &err);
 
			// check if the socket is ready
			select(0,NULL,&write,&err,&timeout);
			if (FD_ISSET(socket, &err))
			{
				return sok_error(&socket);
			}		
			if(FD_ISSET(socket, &write))
			{
				*/
				if (phdr->msgLen==0)
					Error(ERR_CONT, 0, "LEN=0, msgId=0x%08x, len=%d",  phdr->msgId, phdr->msgLen);

				int time=rx_get_ticks();					
				sent=send(*socket, (char*)phdr, phdr->msgLen, MSG_NOSIGNAL);
				if (*socket==_DebugSocket)
					TrPrintfL(TRUE, "DebugSocket msgId=0x%08x, len=%d, sent=%d",  phdr->msgId, phdr->msgLen, sent);
				if (sent==SOCKET_ERROR) return sok_error(socket);
				if (sent!=phdr->msgLen)
					Error(ERR_CONT, 0, "msgId=0x%08x, len=%d, sent=%d",  phdr->msgId, phdr->msgLen, sent);
					
				time = rx_get_ticks()-time;
				if (time>100) Error(ERR_CONT, 0, "sok_send time=%d ms, TelId=0x%08x, len=%d", time, phdr->msgId, phdr->msgLen);
//			}
		}
	}

	return REPLY_OK;
}

//--- sok_send_wide -----------------------------------------------------
int sok_send_wide(RX_SOCKET *socket, void *msg)
{
	SMsgHdr* phdr = (SMsgHdr*)msg;
	int sent;

	if (*socket == 0) *socket = INVALID_SOCKET;
	if (*socket != INVALID_SOCKET) {
		sent = send(*socket, (char*)phdr, phdr->msgLen, MSG_NOSIGNAL);
		if (sent == SOCKET_ERROR) return sok_error(socket);
	}
	return REPLY_OK;
}

/*
//--- sok_send_2 ---------------------------------------------------
int _sok_send_2(RX_SOCKET *socket, INT32 id, UINT32 dataLen, void *data)
{
	BYTE	 buffer[2048];
	SMsgHdr* pmsg;
	int		 size, ret;

	if (socket==0) return REPLY_ERROR;
	if (*socket==0) *socket=INVALID_SOCKET;
	if (*socket==INVALID_SOCKET) return REPLY_ERROR;

	size = sizeof(SMsgHdr)+dataLen;
	if (size > sizeof(buffer)) 
		return Error(ERR_CONT, 0, "Message too large, id=0x%08x, len=%d, max=%d", id, size, sizeof(buffer));

	pmsg = (SMsgHdr*)buffer;
	pmsg->msgId = id;
	pmsg->msgLen = size;
	if (dataLen && data) memcpy(&pmsg[1], data, dataLen);
	ret = sok_send(socket, pmsg);
	return ret;
}
*/

//--- sok_send_2 ---------------------------------------------------
int sok_send_2(RX_SOCKET *socket, INT32 id, UINT32 dataLen, void *data)
{
	BYTE	 buffer[4096];
	SMsgHdr* pmsg;
	int		 size, ret;
	struct sockaddr sender;
	unsigned  len = sizeof(sender);
	
	if (socket==0) return REPLY_ERROR;
	if (*socket==0) *socket=INVALID_SOCKET;
	if (*socket==INVALID_SOCKET) return REPLY_ERROR;	
				
	size = sizeof(SMsgHdr)+dataLen;
	if (size > sizeof(buffer)) 
		return Error(ERR_CONT, 0, "Message too large, id=0x%08x, len=%d, max=%d", id, size, sizeof(buffer));

	pmsg = (SMsgHdr*)buffer;
	pmsg->msgId = id;
	pmsg->msgLen = size;
	if (dataLen && data) memcpy(&pmsg[1], data, dataLen);
	ret = sok_send(socket, pmsg);
	return ret;
}

//--- sok_send_3 ---------------------------------------------------
/*
int sok_send_3(RX_SOCKET *socket, UINT32 dataLen, void *data)
{
	int sent;
	if (socket == 0) return REPLY_ERROR;
	if (*socket == 0) *socket = INVALID_SOCKET;
	if (*socket == INVALID_SOCKET) return REPLY_ERROR;

	sent = send(*socket, (char*)data, dataLen, MSG_NOSIGNAL);
	if (sent == SOCKET_ERROR) return sok_error(socket);

	return REPLY_OK;
}
*/

//--- sok_check_addr_str --------------------------------
int sok_check_addr_str(RX_SOCKET *psocket, char *addr, char *file, int line)
{
	if (psocket==NULL || *psocket==0 || *psocket==INVALID_SOCKET || !*addr) return REPLY_OK;
	
	char peeraddr[32];
	sok_get_peer_name(*psocket, NULL, peeraddr, NULL);
	if (*peeraddr && strcmp(peeraddr, addr)) 
	{
		Error(LOG_TYPE_LOG, file, line, 0, "socket=%d on wrong address %s expected %s", *psocket, peeraddr, addr);
		*psocket=INVALID_SOCKET;
		return REPLY_ERROR;
	}
	
	return REPLY_OK;
}

//--- sok_check_addr_32 --------------------------------
int sok_check_addr_32(RX_SOCKET *psocket, UINT32 addr, char *file, int line)
{
	char str[32];
	return sok_check_addr_str(psocket, sok_addr_str(addr, str), file, line);
}

//--- sok_send_to_clients ----------------------------------------
// return:  Client Count
int sok_send_to_clients(HANDLE hserver, void *pmsg)
{
	SMsgHdr* phdr 	 = (SMsgHdr*)pmsg;
	SServer *pserver = (SServer*)hserver;
	SCilentThreadPar	*par;
	
	int thread;
	int idx;
	char peerName[100];

	if (phdr->msgLen > MAX_MESSAGE_SIZE) Error(ERR_CONT, 0, "Message too large, id=0x%08x, len=%d, max=%d", phdr->msgId, phdr->msgLen, MAX_MESSAGE_SIZE);
	else if (hserver)
	{
		for (thread = 0; thread < pserver->maxConnections; thread++)
		{
			par = &pserver->threadPar[thread];
			if (par->hthread && par->socket && par->socket!=INVALID_SOCKET && par->sendSem)
			{
				idx = (par->msgInIdx+1) % FIFO_SIZE;
				if(idx==par->msgOutIdx)
				{
					if (!_Clinent_overflowError)
					{
						int time = rx_get_ticks()-par->msgTime;
						sok_get_peer_name(par->socket, peerName, NULL, NULL);
						Error(LOG, 0, "sok_send_to_clients[%d] >>%s<<, message buffer overflow, msgId=0x%08x, time since fill=%d, msgcnt=%d, socket closed", thread, peerName, phdr->msgId, time, (par->msgInIdx+FIFO_SIZE-par->msgOutIdx) % FIFO_SIZE);
						_Clinent_overflowError = TRUE;
					}
					TrPrintfL(TRUE, "sok_close %s:%d", __FILE__, __LINE__);						
					sok_close(&par->socket);
				}
				else
				{
					par->msgTime = rx_get_ticks();
					par->msglen[par->msgInIdx] = phdr->msgLen;
					memcpy(par->msgbuf[par->msgInIdx], pmsg, par->msglen[par->msgInIdx]);
					par->msgInIdx = idx;
					rx_sem_post(par->sendSem);					
				}
			}
		}
	}
	return 1;
}

//--- sok_send_to_clients_2 -----------------------------------------------------
// return:  Client Count
int sok_send_to_clients_2(HANDLE hserver, INT32 id, UINT32 len, void *data)
{
	SMsgHdr	*pmsg;
	int		size, cnt;
	
	size = sizeof(SMsgHdr)+len;
	if (size > MAX_MESSAGE_SIZE) Error(ERR_CONT, 0, "Message too large, id=0x%08x, len=%d, max=%d", id, size, MAX_MESSAGE_SIZE);
	pmsg = (SMsgHdr*)malloc(size);
	pmsg->msgId  = id;
	pmsg->msgLen = size;
	if(len && data) memcpy(&pmsg[1], data, len);
	cnt = sok_send_to_clients(hserver, pmsg);
	free(pmsg);
	return cnt;
}

//--- sok_send_to_clients_telnet ----------------------------------------
// return:  Client Count
int sok_send_to_clients_telnet(HANDLE hserver, const void *pmsg, UINT32 len)
{
	SMsgHdr* phdr 	 = (SMsgHdr*)pmsg;
	SServer *pserver = (SServer*)hserver;

	int thread;
	int cnt=0;
	int ticks = rx_get_ticks();
	TIMEVAL Timeout;
    Timeout.tv_sec = 0;
    Timeout.tv_usec = 1000;

	if (len > MAX_MESSAGE_SIZE) Error(ERR_CONT, 0, "Message too large, len=%d, max=%d", len, MAX_MESSAGE_SIZE);
	else if (hserver)
	{
		for (thread = 0; thread < pserver->maxConnections; thread++)
		{
			if (pserver->threadPar[thread].hthread)
			{
				fd_set write, err;
				FD_ZERO(&write);
				FD_ZERO(&err);
				FD_SET(pserver->threadPar[thread].socket, &write);
				FD_SET(pserver->threadPar[thread].socket, &err);
 
				// check if the socket is ready
				select(0,NULL,&write,&err,&Timeout);			
				if(FD_ISSET(pserver->threadPar[thread].socket, &write))
				{
					send(pserver->threadPar[thread].socket, (char*)pmsg, len, MSG_NOSIGNAL);
					pserver->threadPar[thread].sent_tick = ticks;
					cnt++;
				}
				
				if (ticks - pserver->threadPar[thread].sent_tick > 1000)
				{
					sok_close(&pserver->threadPar[thread].socket);
				}
			}
		}
	}
	return cnt;
}

//--- sok_sendto --------------------------------------------------------------------
int sok_sendto		(HANDLE hserver, const void *buf, UINT32 len, int flags, const struct sockaddr *dest_addr, int addrlen)
{
	SServer *pserver = (SServer*)hserver;
	return sendto(pserver->threadPar[0].socket, buf, len, flags, dest_addr, addrlen);
}

//--- sok_socket ----------------------------------------------------------------------
RX_SOCKET sok_socket(HANDLE    hserver)
{
	SServer *pserver = (SServer*)hserver;
	if (pserver->maxConnections) return 	pserver->threadPar[0].socket;
	else return pserver->socket;
}

//--- sok_broadcast --------------------------------------------------------------------
int sok_broadcast(RX_SOCKET socket, const void *buf, UINT32 len, const char *addr, int port)
{
	int ret;
	struct sockaddr_in ipAddr;

	sok_sockaddr(&ipAddr, addr,  port);

	INT32 so_broadcast=1;
	if (setsockopt (socket, SOL_SOCKET, SO_BROADCAST, (char*)&so_broadcast, sizeof (so_broadcast)))	return sok_error(&socket);
	ret=sendto(socket, buf, len, 0, (struct sockaddr*)&ipAddr, sizeof(ipAddr));
	// TrPrintfL(TRUE, "Broadcast len=%d", ret);
	return ret;
}

//--- sok_close --------------------------------------------------------------
int sok_close(RX_SOCKET *psocket)
{
	if (psocket==NULL) 			  return 0;
	if (*psocket==0) 			  return 0;
	if (*psocket==INVALID_SOCKET) return 0;

#ifdef WIN32
	closesocket(*psocket);
	//	WSACleanup();	// call only when closing application
#else // WIN32
	shutdown(*psocket, SHUT_RDWR);
	close(*psocket);
#endif // WIN32
	*psocket = INVALID_SOCKET;
	return 0;
}


//--- sok_open_raw ----------------------------------------------------
int sok_open_raw(RX_RAW_SOCKET *psocket, const char *if_name, const char *addr, int port)
{
#ifdef linux

	struct ifreq if_idx;
	SRxRawSocket *psok;

	//--- UBUNTU -------------------------------------
	// to run RAW IP Packages the user needs the capabiliity CAP_NET_RAW (setcap).
	// or it must be the 'root' user
	//
	// setting the password for the root user: (pwd = "radex")
	//
	// set password:	>sudo passwd root
	// remove password:	>sudo passwd -u root 
	//-----------------------------------------------

	*psocket = NULL;
	psok = (SRxRawSocket*)rx_malloc(sizeof(SRxRawSocket));

	if (psok==NULL) return REPLY_ERROR;

	sok_get_mac_address(if_name, &psok->macAddr);
	if (psok->macAddr==0) goto Error;
 
	//-- Open RAW socket to send on
	psok->socket = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW);
	if (psok->socket==INVALID_SOCKET) goto Error;
 
	//-- Get the index of the interface to send on
	memset(&if_idx, 0, sizeof(struct ifreq));
	strncpy(if_idx.ifr_name, if_name, IFNAMSIZ-1);
	if (ioctl(psok->socket, SIOCGIFINDEX, &if_idx)) goto Error;
	
	// use option IP_HDRINCL ?

	psok->ipAddr = sok_addr_32(addr);
	psok->port   = port;
	psok->ifindex = if_idx.ifr_ifindex;
	*psocket = psok;
	return REPLY_OK;

Error:
	{
		int ret = sok_error(&psok->socket);
		free(psok);
		return ret;
	}
#else
	return Error(ERR_ABORT, 0, "sok_open_raw not supported in windows");
#endif
}

#ifdef linux
	#pragma pack(1)
	//--- Message Frame ----------------------------------------
	typedef struct SMsgFrame
	{
		struct ether_header eth;
		struct iphdr		iph;
		struct udphdr		udph;
		BYTE				data[9000];	
	} SMsgFrame;

	//  96 bit (12 bytes) pseudo header needed for udp header checksum calculation 
	typedef struct Spseudo_header
	{
		UINT32	source_address;
		UINT32	dest_address;
		UINT8	placeholder;
		UINT8	protocol;
		UINT16	udp_length;
	} Spseudo_header;
	#pragma pack()

	//---  Generic checksum calculation function -----
	static unsigned short _csum(void *bptr,int nbytes) 
	{
		unsigned short *ptr = (USHORT*)bptr;
		register long sum;
		unsigned short oddbyte;
		register short answer;
 
		sum=0;
		while(nbytes>1) {
			sum+=*ptr++;
			nbytes-=2;
		}
		if(nbytes==1) {
			oddbyte=0;
			*((u_char*)&oddbyte)=*(u_char*)ptr;
			sum+=oddbyte;
		}
 
		sum = (sum>>16)+(sum & 0xffff);
		sum = sum + (sum>>16);
		answer=(short)~sum;
     
		return(answer);
	}
#endif

//--- sok_send_raw_udp -------------------------------------------------------------------
int sok_send_raw_udp(RX_RAW_SOCKET *psocket, UINT64 dstMacAddr, UINT32 dstIpAddr, UINT32 dstPort, void *data, int dataLen)
{
#ifdef linux
	SRxRawSocket *psok = (SRxRawSocket*)psocket;
	struct sockaddr_ll	dstSocket;
	SMsgFrame msg;
	struct iphdr iph;
	int msgLen, sent;
	char str1[32], str2[32];

//	TrPrintfL(TRUE, "sok_send_raw_udp (mac=>>%s<<, ip=>>%s<<, port=%d)", sok_mac_addr_str(dstMacAddr, str1, sizeof(str1)), sok_addr_str(dstIpAddr, str2), dstPort);

	if (*psocket==NULL) return REPLY_ERROR;

	memset(&msg, 0, sizeof(msg));

	//--- data ---------------------
	memcpy(msg.data, data, dataLen);
		
	//---Ethernet header -----------------------------------------------------------
	memcpy(msg.eth.ether_shost, &psok->macAddr, sizeof(msg.eth.ether_shost));
	memcpy(msg.eth.ether_dhost, &dstMacAddr,	sizeof(msg.eth.ether_dhost));
	msg.eth.ether_type = htons(ETH_P_IP);

	//--- IP header ------------------------------------
	iph.ihl = 5;
    iph.version = 4;
    iph.tos = 0;
    iph.tot_len = htons(sizeof (struct iphdr) + sizeof (struct udphdr) + dataLen);
    iph.id = htons (0); //Id of this packet
    iph.frag_off = 0;
    iph.ttl = 128;
    iph.protocol = IPPROTO_UDP;
    iph.check = 0;      //Set to 0 before calculating checksum
    iph.saddr = psok->ipAddr;  // Source Address
    iph.daddr = dstIpAddr;	// Destination Address
    iph.check = _csum (&iph, sizeof(iph));

	//--- UDP header -----------------------------------------------
	msg.udph.source = htons(psok->port);
	msg.udph.dest   = htons(dstPort);
	msg.udph.len	= htons(sizeof(msg.udph)+dataLen);
	msg.udph.check	= 0;

	if (TRUE)
	{
		BYTE *pseudoStart;
		//--- udp checksum (replace the IP-Header by a pseude header to cals checksum) ---
		pseudoStart = ((BYTE*)&msg.udph) - sizeof(Spseudo_header);
		Spseudo_header *phdr = (Spseudo_header*)pseudoStart;
		phdr->source_address = psok->ipAddr;
		phdr->dest_address   = dstIpAddr;
		phdr->placeholder	 = 0;
		phdr->protocol		 = IPPROTO_UDP;
		phdr->udp_length	 = htons(sizeof(struct udphdr) + dataLen);	
		msg.udph.check = _csum (pseudoStart, sizeof(Spseudo_header) + sizeof(struct udphdr) + dataLen);
	}
	memcpy(&msg.iph, &iph, sizeof(msg.iph));	// set the correct header again

	//--- Destination address --------------------------------------------------------
	memcpy(dstSocket.sll_addr, msg.eth.ether_dhost, sizeof(dstSocket.sll_addr));
	dstSocket.sll_ifindex = psok->ifindex;
	dstSocket.sll_halen = ETH_ALEN;

	//--- Send packet ---------------------------------------------------------------
	msgLen = sizeof(struct ether_header)+htons(iph.tot_len);
	sent = sendto(psok->socket, &msg, msgLen, 0, (struct sockaddr*)&dstSocket, sizeof(struct sockaddr_ll));
	if (sent<0) 
	{
		char err[256];
		TrPrintf(1, "Error >>%s<<", err_system_error(errno, err, sizeof(err)));

		return sent;
	}
	sent -= (sizeof(struct ether_header)+sizeof(struct iphdr)+sizeof(struct udphdr));
	
//	TrPrintfL(TRUE, "%d Data Bytes sent", sent);

	return sent;
#else
	return Error(ERR_ABORT, 0, "sok_send_raw_udp not supported in windows");
#endif
}

//--- sok_send_arp -----------------------------------------------------------------------
int sok_send_arp(RX_RAW_SOCKET *psocket, UINT64 dstMacAddr, void *data, int dataLen)
{
#ifdef linux
	SRxRawSocket *psok = (SRxRawSocket*)psocket;
	struct sockaddr_ll socket_address;
	int sent;

//	_arp_trace_msg(preply, "ARP REPLY");

	//--- prepare sockaddr_ll ---
	memset(&socket_address, 0, sizeof(socket_address));
	socket_address.sll_family	= PF_PACKET;
	socket_address.sll_protocol = htons(ETH_P_IP);
	socket_address.sll_ifindex	= psok->ifindex;
	socket_address.sll_hatype	= ARPHRD_ETHER;
	socket_address.sll_pkttype	= PACKET_OTHERHOST;
	socket_address.sll_halen	= ETH_ALEN;

	memcpy(socket_address.sll_addr, &dstMacAddr, 6);

	sent = sendto(psok->socket, data, dataLen, 0, (struct sockaddr*)&socket_address, sizeof(socket_address));
	if (sent == -1)
	{
		perror("sendto():");
		return errno;
	}
#endif
	return REPLY_OK;
}


//--- sok_close_raw --------------------------------------------------------------
int sok_close_raw(RX_RAW_SOCKET *psocket)
{
	SRxRawSocket *psok = (SRxRawSocket*)*psocket;
	if (psocket==NULL) return 0;
	sok_close(&psok->socket);
	free(psok);
	*psocket=NULL;	
	return 0;
}

#ifdef WIN32
//--- __get_adapter_info ----------------------------------------------
	static int __get_adapter_info(const char *deviceName, UINT64 *macAddr, char *ipAddr)
	{
		ULONG result, size, idx;
		wchar_t wname[MAX_PATH];
		IP_ADAPTER_ADDRESSES	*adresses, *paddr;
		LPSOCKADDR pipAddr;

		*macAddr = 0;
		*ipAddr  = 0;
		size   = 0;

		if (GetAdaptersAddresses(AF_INET, 0, NULL, NULL, &size)==ERROR_BUFFER_OVERFLOW)
		{
			adresses = (IP_ADAPTER_ADDRESSES*)malloc(size);
			result = GetAdaptersAddresses(AF_INET, 0, NULL, adresses, &size);
			if (result==NO_ERROR)
			{
				char_to_wchar(deviceName, wname, SIZEOF(wname));
				for (paddr=adresses, idx=0; paddr; paddr=paddr->Next, idx++) 
				{			
					pipAddr = paddr->FirstUnicastAddress->Address.lpSockaddr;
					sprintf(ipAddr, "%d.%d.%d.%d", (UCHAR)pipAddr->sa_data[2], (UCHAR)pipAddr->sa_data[3], (UCHAR)pipAddr->sa_data[4], (UCHAR)pipAddr->sa_data[5]);	 
				//	wprintf(L"Adaptername >>%s<<", (wchar_t*)paddr->FriendlyName);
				//	printf(" >>%s<<\n", addrstr);
					if (!wcsicmp(wname, (wchar_t*)paddr->FriendlyName) || !strcmp(ipAddr, deviceName))
					{
						memcpy(macAddr, paddr->PhysicalAddress, 6);
						free(adresses);
						return REPLY_OK;
					}
				}
			}
			free(adresses);
		}
		*ipAddr  = 0;
		return REPLY_ERROR;
	}

//--- sok_get_netspeed -------------------------------------
UINT32 sok_get_netspeed(RX_SOCKET socket)
{
	UINT32	speed=0;
	if (socket && socket!=INVALID_SOCKET)
	{
		char str[100];
		wchar_t wstr[MAX_PATH], *pspeed;
		char ipaddr[64];
		char fname[100];
		FILE *file;
		UINT64 macaddr;
		UCHAR *mac = (UCHAR*)&macaddr;

		sok_get_socket_name(socket, NULL, str, NULL);
	
		__get_adapter_info(str, &macaddr, ipaddr);
		sprintf(fname, PATH_TEMP "NetSpeed.txt");
		sprintf(str, "wmic NIC where MACAddress=\"%02X:%02X:%02X:%02X:%02X:%02X\" get NetConnectionId, Speed", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		rx_process_execute(str, fname, 0);
		file=fopen(fname, "rb");
		if (file)
		{
			fgetws(wstr, 2, file);	// unicode marking
			fgetws(wstr, SIZEOF(wstr), file);
			pspeed = wcsstr(wstr, L"Speed");
			if (pspeed) 
			{
				fgetws(wstr, SIZEOF(wstr), file);
				speed=_wtoi(pspeed);
			}
			fclose(file);
			remove(fname);
		}
	}
	return speed;
}
#endif

#ifdef linux
UINT32 sok_get_netspeed(const char *deviceName)
{
	char cmd[100];
	char *filepath = "/tmp/eth_speed";
	FILE *file;
	int pos;

	sprintf(cmd, "ethtool %s | grep -i speed >> %s", deviceName, filepath);
	system(cmd);
	file = fopen(filepath, "rt");
	if (file)
	{
		fgets(cmd, sizeof(cmd), file);
		fclose(file);
		remove(filepath);
		if (pos=str_start(cmd, "Speed: "))
		{
			return atoi(&cmd[pos]);
		}
	}
	return 0;
}
#endif

//--- sok_get_term_str ----------------------------------------
int  sok_get_term_str	(HANDLE hserver, char *str, int size)
{
	SServer *pserver = (SServer*)hserver;

	if (pserver) rx_mutex_lock(pserver->mutex);
	strncpy(str, _TermStr, size);
	_TermStr[0]=0;
	if (pserver) rx_mutex_unlock(pserver->mutex);
	return (str[0]!=0);
}

HANDLE debug_mutex(HANDLE hserver)
{
	SServer *pserver = (SServer*)hserver;
	if (hserver==NULL) return NULL;
	return pserver->mutex;
}
