/*************************************************************************/
/*				Copyright (c) 2000-2016 NT Kernel Resources.		     */
/*                           All Rights Reserved.                        */
/*                          http://www.ntkernel.com                      */
/*                           ndisrd@ntkernel.com                         */
/* Module Name:  NetworkInterface.cpp	                                 */
/*                                                                       */
/* Abstract: CNetworkInterface implementation file			             */
/*                                                                       */
/*                                                                       */
/*************************************************************************/

#include "StdAfx.h"
#include "networkinterface.h"

//--- net_init -------------------------
void  net_init(CNetworkInterface *pinterface)
{
	memset(pinterface, 0, sizeof(*pinterface));
}

//--- net_end -----------------------------
void  net_end(CNetworkInterface *pinterface)
{
	CIpAddrList *old;
	CIpAddrList *ipAddr = pinterface->m_IpList;
	while(ipAddr)
	{
		old    = ipAddr;
		ipAddr = ipAddr->next;
		free(old);
	}
	pinterface->m_IpList=NULL;
}

// Uses IP helper APi to query system for the MTU's associated with current network interface
void net_initMTUInformation(CNetworkInterface *pinterface)
{
	PMIB_IFTABLE		ifTable;
	DWORD				dwSize = 0;
	DWORD				dwRetVal = 0;
	DWORD				dwMTU	= 0;
	DWORD				dwSpeed	= 0;


	// Allocate memory for our pointers
	ifTable = (MIB_IFTABLE*) malloc(sizeof(MIB_IFTABLE));

	if(!ifTable)
		return;

	// Make an initial call to GetIfTable to get the
	// necessary size into the dwSize variable
	if (GetIfTable(ifTable, &dwSize, 0) == ERROR_INSUFFICIENT_BUFFER)
	{
		free(ifTable);
		
		ifTable = (MIB_IFTABLE *) malloc (dwSize);

		if(!ifTable)
			return;

		if (GetIfTable(ifTable, &dwSize, 0) != NO_ERROR)
		{
			if(ifTable)
				free(ifTable);

			return;
		}
	}

	for(unsigned int m = 0; m < ifTable->dwNumEntries; m++)
	{
		if(pinterface->m_Index == ifTable->table[m].dwIndex)
		{	
			pinterface->m_MTU	= (unsigned short)ifTable->table[m].dwMtu;

			break;
		}

	}

	if(ifTable)
		free(ifTable);

	return;
}

// Uses IP helper APi to query system for the IP's associated with current network interface
int net_initIPInformation(CNetworkInterface *pinterface)
{
	DWORD				dwOutputBufferZize = 0;
	PIP_ADAPTER_INFO	pAdapterInfo = NULL;
	PIP_ADAPTER_INFO	pAdapter = NULL;
	PIP_ADDR_STRING		pIpString = NULL;

	if (ERROR_BUFFER_OVERFLOW == GetAdaptersInfo(pAdapterInfo, &dwOutputBufferZize))
	{
		// Allocate required amount of memory from the heap
		pAdapterInfo = (PIP_ADAPTER_INFO)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwOutputBufferZize);

		if(pAdapterInfo)
		{
			if(ERROR_SUCCESS == GetAdaptersInfo(pAdapterInfo, &dwOutputBufferZize))
			{
				// Walk the list of adapters and get IP's associated with this adapter
				pAdapter = pAdapterInfo;
				while (pAdapter)
				{
					if ((((pAdapter->Type == MIB_IF_TYPE_PPP)||(pAdapter->Type == MIB_IF_TYPE_SLIP))&&(pinterface->m_bIsWan))||
						(!memcmp(pinterface->m_chMACAddr, pAdapter->Address, ETHER_ADDR_LENGTH)))
					{
						pIpString = &pAdapter->IpAddressList;
						
						while (pIpString)
						{
							CIpAddrList *ipAddr = malloc(sizeof(CIpAddrList));
							ipAddr->next=NULL;
							strcpy(ipAddr->addr.m_szIp, pIpString->IpAddress.String);
							strcpy(ipAddr->addr.m_szMask, pIpString->IpMask.String);
							ipAddr->addr.m_Ip.S_un.S_addr = htonl(inet_addr(ipAddr->addr.m_szIp));

							if (pinterface->m_IpList==NULL) pinterface->m_IpList = ipAddr;
							else 
							{
								CIpAddrList *ptr=pinterface->m_IpList;
								while (ptr->next!=NULL) ptr=ptr->next;
								ptr->next=ipAddr;
							}

							pIpString = pIpString->Next;

							pinterface->m_Index = pAdapter->Index;
						}
					}

					pAdapter = pAdapter->Next;
				}
			}

			// Free buffer
			HeapFree(GetProcessHeap(), 0, pAdapterInfo);
		}
	}
	return 0;
}

// Returnes TRUE if specified IP belongs to current interface
BOOL net_isLocalAddress(CNetworkInterface *pinterface, struct in_addr* pIp)
{
	CIpAddrList* ptr;
	for (ptr=pinterface->m_IpList; ptr!=NULL; ptr=ptr->next)
	{
		if (ptr->addr.m_Ip.S_un.S_addr == ntohl(pIp->S_un.S_addr)) return TRUE;
	}

	return FALSE;
}
