// ****************************************************************************
//
//	DIGITAL PRINTING - bridge.cpp
//
// ****************************************************************************
//
//	Copyright 2017 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#include <share.h>
#include "nat.h"
#include "bridge.h"


//--- globals ------------------

SNatThreadArgs _Args;
static IP_ADAPTER_INFO	_ProviderAdapter;
static IP_ADAPTER_INFO	_ClientAdapter;
static FILE *_TraceFile = NULL;

//--- prototypes --------------------------------------

static int _init_net_list(void);
static int _init_nat_interfaces(char *provider, char *client);
static int _updateDNSByIp(int nIp);
static int _getDNSIp(void);

static int _findAdapter(IP_ADAPTER_INFO *info, char *id);

//--- bridge_init ---------------------------------
int bridge_init(char *provider, char *client)
{	
	memset(&_Args, 0, sizeof(_Args));
//	_TraceFile = _fsopen("c:/radex/rx_bridge_trace.txt", "w", _SH_DENYNO);
	if (_TraceFile==NULL) _TraceFile = stdout;
	fprintf(_TraceFile, "Provider: "); _findAdapter(&_ProviderAdapter, provider);
	fprintf(_TraceFile, "Client:   "); _findAdapter(&_ClientAdapter,   client);
	_init_net_list();
	_init_nat_interfaces(provider, client);
	nat_start(&_Args);
	fflush(_TraceFile);
	return 0;
}

//--- _init_nat_interfaces ----------------------------------------
static int 	_init_nat_interfaces(char *provider, char *client)
{
	int i;
	for (i=0; i<_Args.netCardCnt; i++)
	{
//		if (!stricmp(_Args.netCards[i].m_szUserFriendlyName, provider))
		if (strstr(_Args.netCards[i].m_szInternalName, _ProviderAdapter.AdapterName))
		{
			if (_Args.netCards[i].m_IpList)
			{
				CIpAddr *ipAddr = &_Args.netCards[i].m_IpList->addr;
				struct in_addr	  DNS;
				_Args.providerCard = &_Args.netCards[i];
				_Args.providerCard->m_NATState = PROVIDER;
				_Args.providerCard->m_NATIp    = ipAddr->m_Ip;
				DNS.S_un.S_addr = ntohl(_updateDNSByIp(ntohl(ipAddr->m_Ip.S_un.S_addr)));
								
				if((DNS.S_un.S_addr == INADDR_ANY) || (DNS.S_un.S_addr == INADDR_NONE))DNS.S_un.S_addr = ntohl(_getDNSIp());
				if(DNS.S_un.S_addr == INADDR_LOOPBACK) DNS.S_un.S_addr = INADDR_ANY;
				_Args.dNSIp = DNS;
			} 
		}

//		if (!stricmp(_Args.netCards[i].m_szUserFriendlyName, client))
		if (strstr(_Args.netCards[i].m_szInternalName, _ClientAdapter.AdapterName))
		{
			_Args.clientCard = &_Args.netCards[i];
			_Args.clientCard->m_NATState = CLIENT;
		}
	}
	return 0;
}

//--- bridge_end ----------------------------------
int bridge_end(void)
{
	return 0;
}

//--- _findAdapter -----------------
static int _findAdapter(IP_ADAPTER_INFO *info, char *id)
{
	// Docu: https://msdn.microsoft.com/en-us/library/windows/desktop/aa365917(v=vs.85).aspx

    PIP_ADAPTER_INFO pAdapterInfo;
    PIP_ADAPTER_INFO pAdapter = NULL;
    DWORD dwRetVal = 0;
	
    UINT i;

	//  variables used to print DHCP time info
    struct tm newtime;
    char buffer[32];
    errno_t error;	

	memset(info, 0, sizeof(PIP_ADAPTER_INFO));

    ULONG ulOutBufLen = sizeof (IP_ADAPTER_INFO);
    pAdapterInfo = (IP_ADAPTER_INFO *) malloc(sizeof (IP_ADAPTER_INFO));
    if (pAdapterInfo == NULL) 
	{ 
		fprintf(_TraceFile, "Error allocating memory needed to call GetAdaptersinfo\n");
        return 1;
    }
// Make an initial call to GetAdaptersInfo to get
// the necessary size into the ulOutBufLen variable
    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) 
	{
        free(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO *) malloc(ulOutBufLen);
        if (pAdapterInfo == NULL) 
		{
            fprintf(_TraceFile, "Error allocating memory needed to call GetAdaptersinfo\n");
            return 1;
        }
    }

    if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) 
	{
        for (pAdapter=pAdapterInfo; pAdapter; pAdapter = pAdapter->Next) 
		{
			if (id && (!strnicmp(pAdapter->IpAddressList.IpAddress.String, id, strlen(id))))
			{
				memcpy(info, pAdapter, sizeof(IP_ADAPTER_INFO));
			}
			else if (id && !stricmp(pAdapter->Description, id))
			{
				memcpy(info, pAdapter, sizeof(IP_ADAPTER_INFO));
			}
			if (FALSE)
			{
				fprintf(_TraceFile, "\tComboIndex: \t%d\n", pAdapter->ComboIndex);
				fprintf(_TraceFile, "\tAdapter Name: \t%s\n", pAdapter->AdapterName);
				fprintf(_TraceFile, "\tAdapter Desc: \t%s\n", pAdapter->Description);
				fprintf(_TraceFile, "\tAdapter Addr: \t");
				for (i = 0; i < pAdapter->AddressLength; i++) 
				{
					if (i == (pAdapter->AddressLength - 1)) fprintf(_TraceFile, "%.2X\n", (int) pAdapter->Address[i]);
					else fprintf(_TraceFile, "%.2X-", (int) pAdapter->Address[i]);
				}
				fprintf(_TraceFile, "\tIndex: \t%d\n", pAdapter->Index);
				fprintf(_TraceFile, "\tType: \t");
				switch (pAdapter->Type) {
				case MIB_IF_TYPE_OTHER:		fprintf(_TraceFile, "Other\n");		break;
				case MIB_IF_TYPE_ETHERNET:	fprintf(_TraceFile, "Ethernet\n");	break;
				case MIB_IF_TYPE_TOKENRING:	fprintf(_TraceFile, "Token Ring\n");	break;
				case MIB_IF_TYPE_FDDI:		fprintf(_TraceFile, "FDDI\n");		break;
				case MIB_IF_TYPE_PPP:		fprintf(_TraceFile, "PPP\n");		break;
				case MIB_IF_TYPE_LOOPBACK:	fprintf(_TraceFile, "Lookback\n");	break;
				case MIB_IF_TYPE_SLIP:		fprintf(_TraceFile, "Slip\n");		break;
				default:					fprintf(_TraceFile, "Unknown type %ld\n", pAdapter->Type);	break;
				}

				fprintf(_TraceFile, "\tIP Address: \t%s\n", pAdapter->IpAddressList.IpAddress.String);
				fprintf(_TraceFile, "\tIP Mask: \t%s\n", pAdapter->IpAddressList.IpMask.String);
				fprintf(_TraceFile, "\tGateway: \t%s\n", pAdapter->GatewayList.IpAddress.String);
				fprintf(_TraceFile, "\t***\n");

				if (pAdapter->DhcpEnabled) 
				{
					fprintf(_TraceFile, "\tDHCP Enabled: Yes\n");
					fprintf(_TraceFile, "\t  DHCP Server: \t%s\n", pAdapter->DhcpServer.IpAddress.String);

					fprintf(_TraceFile, "\t  Lease Obtained: ");
					// Display local time 
					error = _localtime32_s(&newtime, (__time32_t*) &pAdapter->LeaseObtained);
					if (error) fprintf(_TraceFile, "Invalid Argument to _localtime32_s\n");
					else 
					{
						// Convert to an ASCII representation 
						error = asctime_s(buffer, 32, &newtime);
						if (error) fprintf(_TraceFile, "Invalid Argument to asctime_s\n");
						else       fprintf(_TraceFile, "%s", buffer); // asctime_s returns the string terminated by \n\0
					}

					fprintf(_TraceFile, "\t  Lease Expires:  ");
					error = _localtime32_s(&newtime, (__time32_t*) &pAdapter->LeaseExpires);
					if (error)	fprintf(_TraceFile, "Invalid Argument to _localtime32_s\n");
					else 
					{
						// Convert to an ASCII representation 
						error = asctime_s(buffer, 32, &newtime);
						if (error)	fprintf(_TraceFile, "Invalid Argument to asctime_s\n");
						else        fprintf(_TraceFile, "%s", buffer);// asctime_s returns the string terminated by \n\0 
					}
				} 
				else fprintf(_TraceFile, "\tDHCP Enabled: No\n");

				if (pAdapter->HaveWins) 
				{
					fprintf(_TraceFile, "\tHave Wins: Yes\n");
					fprintf(_TraceFile, "\t  Primary Wins Server:    %s\n", pAdapter->PrimaryWinsServer.IpAddress.String);
					fprintf(_TraceFile, "\t  Secondary Wins Server:  %s\n", pAdapter->SecondaryWinsServer.IpAddress.String);
				} 
				else fprintf(_TraceFile, "\tHave Wins: No\n");
				fprintf(_TraceFile, "\n");
			}
		}
    } 
	else 
	{
        fprintf(_TraceFile, "GetAdaptersInfo failed with error: %d\n", dwRetVal);
    }
    if (pAdapterInfo) free(pAdapterInfo);

	if (*info->AdapterName) fprintf(_TraceFile, "Adapter >>%s<< found\n", id);
	else                    fprintf(_TraceFile, "ERROR: Adapter >>%s<< NOT FOUND\n", id);
    return 0;
}

//--- _init_net_list ------------------------
static int _init_net_list(void)
{
	OSVERSIONINFO	verInfo;
	char			szFriendlyName[MAX_PATH*4];

	_Args.hNdisApi = OpenFilterDriver(DRIVER_NAME_A);
	verInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx ( &verInfo );

	if(IsDriverLoaded(_Args.hNdisApi))
	{		
		GetTcpipBoundAdaptersInfo (_Args.hNdisApi,  &_Args.adapterList);
		_Args.netCardCnt = _Args.adapterList.m_nAdapterCount;

		_Args.netCards = malloc(_Args.netCardCnt*sizeof(CNetworkInterface));
		memset(_Args.netCards, 0, _Args.netCardCnt*sizeof(CNetworkInterface));

		for (unsigned i = 0; i < _Args.adapterList.m_nAdapterCount; ++i)
		{
			// Build the list of network inetrface instances
			CNetworkInterface *pnetCard = &_Args.netCards[i];
			strcpy(pnetCard->m_szInternalName, (LPSTR)_Args.adapterList.m_szAdapterNameList[i]);

			if (verInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
			{
				if (verInfo.dwMajorVersion >= 5)
				{
					// Windows 2000 or XP
					ConvertWindows2000AdapterName((const char*)_Args.adapterList.m_szAdapterNameList[i], szFriendlyName, MAX_PATH*4);
				}
				else if (verInfo.dwMajorVersion == 4)
				{
					// Windows NT 4.0	
					ConvertWindowsNTAdapterName((const char*)_Args.adapterList.m_szAdapterNameList[i], szFriendlyName, MAX_PATH*4);
				}
			}
			else
			{
				// Windows 9x/ME
				ConvertWindows9xAdapterName((const char*)_Args.adapterList.m_szAdapterNameList[i], szFriendlyName, MAX_PATH*4);
			}

			strcpy(pnetCard->m_szUserFriendlyName, szFriendlyName);
			memcpy (pnetCard->m_chMACAddr, _Args.adapterList.m_czCurrentAddress[i], ETHER_ADDR_LENGTH);
			pnetCard->m_hAdapter = _Args.adapterList.m_nAdapterHandle[i];
			pnetCard->m_bIsWan   = (_Args.adapterList.m_nAdapterMediumList[i]!=0);

			net_initIPInformation(pnetCard);
			
			net_initMTUInformation(pnetCard);
		}

		//--- trace interfaces -------------------
		if (FALSE)
		{
			for (int i = 0; i < _Args.netCardCnt; ++i)
			{
				fprintf(_TraceFile, "Network %d: %s --------------------------\n", i, _Args.netCards[i].m_szUserFriendlyName);
				if (_Args.netCards[i].m_IpList) fprintf(_TraceFile, "IpAddr  %s\n", _Args.netCards[i].m_IpList->addr.m_szIp);
				fprintf(_TraceFile, "MTU     %d\n", _Args.netCards[i].m_MTU);
				fprintf(_TraceFile, "\n");
			}
		}
		return 1;
	}
	else
	{
		fprintf(_TraceFile, "WinPK Driver not loaded\n");
	}
	return 0;
}

//--- _updateDNSByIp --------------------------------------------------------------
static int _updateDNSByIp(int nIp)
{
	TCHAR    achKey[64];   // buffer for subkey name
	DWORD    cbName = 64;  // size of name string 

	DWORD  retCode; 
	DWORD  dwType;
	DWORD	dwSize = 0;

	DWORD nParameters = 0;
	DWORD nInterfaces = 0;

	TCHAR  achInterface[MAX_PATH]; 
	TCHAR  achValue[MAX_PATH]; 
	CHAR  achIpAddr[MAX_PATH]; 
	CHAR  achDnsIpAddr[MAX_PATH]; 
	DWORD  cchValue = MAX_PATH; 

	HKEY hKey;
	HKEY hSubKey;

	int DNSIp = 0;


	if( RegOpenKeyEx( HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces"), 0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		while(TRUE) 
		{
			cbName = 64;
			retCode = RegEnumKeyEx(hKey, nInterfaces, achKey, &cbName, NULL, NULL, NULL, NULL); 

			if (retCode == ERROR_SUCCESS) 
			{
				achInterface[0] = '\0';
				strcat(achInterface, TEXT("SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces\\"));
				strcat(achInterface, achKey);
				if( RegOpenKeyEx( HKEY_LOCAL_MACHINE, achInterface, 0, KEY_QUERY_VALUE, &hSubKey) == ERROR_SUCCESS)
				{
					nParameters = 0;
					while(TRUE) 
					{ 
						cchValue = MAX_PATH; 
						retCode = RegEnumValue(hSubKey, nParameters, achValue, &cchValue, NULL, NULL, NULL, NULL);
						if (retCode == ERROR_SUCCESS ) 
						{ 
							if((!strcmp(achValue, "IPAddress")) || (!strcmp(achValue, "DhcpIPAddress")))
							{
								dwSize =  MAX_PATH;
								if(RegQueryValueExA( hSubKey, achValue, 0, &dwType, (LPBYTE) achIpAddr, &dwSize) == ERROR_SUCCESS)
								{
									BOOL bFound  = FALSE;
									char *pIpStr = achIpAddr;
									int  nShift = 0;

									while(TRUE)
									{
										if(inet_addr(pIpStr) == nIp)
										{
											bFound = TRUE;
											break;
										}
										else
										{
											nShift = (int)strlen(pIpStr) + 1;
											if(nShift != dwSize)
											{
												pIpStr += nShift;
												dwSize -= nShift; 
											}
											else break;
										}
									}

									if(bFound)
									{
										dwSize =  MAX_PATH;
										if(RegQueryValueExA( hSubKey, "NameServer", 0, &dwType, (LPBYTE) achDnsIpAddr, &dwSize) == ERROR_SUCCESS)
										{
											if(achDnsIpAddr[0] == 0) //pure string
											{
												dwSize =  MAX_PATH;
												RegQueryValueExA( hSubKey, "DhcpNameServer", 0, &dwType, (LPBYTE) achDnsIpAddr, &dwSize);
											}
										}

										char* pFirstDnsEnd = strchr(achDnsIpAddr, 32);

										if(pFirstDnsEnd)
										{
											*pFirstDnsEnd = 0;
										}
										else
										{
											pFirstDnsEnd = strchr(achDnsIpAddr, ',');
											if(pFirstDnsEnd)
											{
												*pFirstDnsEnd = 0;
											}
										}
										DNSIp = inet_addr(achDnsIpAddr);
										break;
									}
								}
							}
							nParameters++;
						}
						else break;
					}
					RegCloseKey(hSubKey);
				}
				nInterfaces++;
			}
			else break;
		}
		RegCloseKey(hKey);
	} 
	return DNSIp;
}

//--- _getDNSIp ---------------------------------------------
// Retrieves DNS IP address using IP helper API
static int _getDNSIp(void)
{
	PFIXED_INFO pInfo = NULL;
	DWORD dwInfoSize = 0;
	struct in_addr DNSIp;

	if(ERROR_BUFFER_OVERFLOW == GetNetworkParams(pInfo, &dwInfoSize))
	{
		pInfo = (PFIXED_INFO)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwInfoSize);

		if (ERROR_SUCCESS == GetNetworkParams(pInfo, &dwInfoSize))
		{
			DNSIp.S_un.S_addr = inet_addr((LPCSTR)pInfo->DnsServerList.IpAddress.String);			
		}
		if (pInfo) HeapFree(GetProcessHeap(), 0, pInfo);
	}
	return DNSIp.S_un.S_addr;
}