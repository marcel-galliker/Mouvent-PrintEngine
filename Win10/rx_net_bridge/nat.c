// ****************************************************************************
//
//	DIGITAL PRINTING - nat.c
//
// ****************************************************************************
//
//	Copyright 2017 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#include "stdafx.h"
#include "iphlp.h"
#include "PortNat.h"
#include "IcmNat.h"
#include "nat.h"

static HANDLE _hNATThread;
static HANDLE _hNATTerminateEvent;

//--- prototypes -----------------------------------
unsigned __stdcall nat_thread ( void* pArguments );
static BOOL _isNeedToForceRouting(BYTE *MACAddress, DWORD dwDestIp, DWORD dwProviderIndex);
static void _checkMTUCorrelation(SNatThreadArgs *pargs, PINTERMEDIATE_BUFFER pBuffer, iphdr_ptr pIpHeader, tcphdr_ptr pTcpHeader);

//--- nat_start ----------------------------------------
void nat_start(SNatThreadArgs *pargs)
{
	unsigned	dwID;

	// Create thread terminate event
	_hNATTerminateEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	// Start NAT processing thread
	_hNATThread = CreateThread (NULL, 0, nat_thread, pargs, 0, &dwID);
}

//--- nat_end -------------------------------------
void nat_end()
{
	if (_hNATThread && _hNATTerminateEvent)
	{
		SetEvent(_hNATTerminateEvent);
		WaitForSingleObject(_hNATThread, INFINITE);
		_hNATThread = NULL;
	}
}


unsigned __stdcall nat_thread ( void* pArguments )
{
	SNatThreadArgs*		pArgs = (SNatThreadArgs*)pArguments;
	HANDLE				hEvents[ADAPTER_LIST_SIZE + 1];
	CNetworkInterface*	hAdapters [ADAPTER_LIST_SIZE + 1];
	CNetworkInterface	*pNetCard, *pProviderCard;
	unsigned			dwActiveAdaptersCount = 1;
	ADAPTER_MODE		Mode;
	ETH_REQUEST			Request;
	INTERMEDIATE_BUFFER PacketBuffer;
	DWORD				dwWait, dwIndex;
	ether_header*		pEthHeader;
	iphdr*				pIpHeader;
	tcphdr*				pTcpHeader;
	udphdr*				pUdpHeader;
	icmphdr*			pIcmpHeader;

	BOOL				bInit = FALSE;
	BYTE				MACClient[ETHER_ADDR_LENGTH];
	BYTE				MACServer[ETHER_ADDR_LENGTH];
	
	CPortNATTable		TcpNatTable; // TCP NAT table
	CPortNATTable		UdpNatTable; // UDP NAT table
	CIcmpNATTable		IcmpNatTable; // ICMP NAT table
	
	BOOL				bForceRouting	= FALSE;
	BOOL				bNeedToBeRouted = FALSE;

	static int			dns = 0;

	Mode.dwFlags = MSTCP_FLAG_SENT_TUNNEL|MSTCP_FLAG_RECV_TUNNEL|MSTCP_FLAG_LOOPBACK_BLOCK|MSTCP_FLAG_LOOPBACK_FILTER;

	hEvents[0] = _hNATTerminateEvent;

	// Walk adapters list and initialize provider and clients interfaces
	for (int i = 0; i < pArgs->netCardCnt; ++i)
	{
		pNetCard = &pArgs->netCards[i];

		if ((pNetCard->m_NATState == CLIENT) || (pNetCard->m_NATState == PROVIDER))
		{
			hAdapters[dwActiveAdaptersCount] = pNetCard;
			hEvents[dwActiveAdaptersCount] = CreateEvent(NULL, TRUE, FALSE, NULL);
			SetPacketEvent(pArgs->hNdisApi, pNetCard->m_hAdapter, hEvents[dwActiveAdaptersCount]);
			Mode.hAdapterHandle = pNetCard->m_hAdapter;
			SetAdapterMode(pArgs->hNdisApi, &Mode);
			dwActiveAdaptersCount++;

			if(pNetCard->m_NATState == PROVIDER)
			{
				pProviderCard = pNetCard;

//				pDlg->m_ProviderMTU = pNetCard->m_MTU;
				/*
				if(pDlg->IsWindows7())
				{
					if(pNetCard->m_szInternalName == CString("\\DEVICE\\NDISWANIP"))
					{
						RAS_LINKS RasLinks;

						pDlg->m_NdisApi.GetRasLinks(pNetCard->m_hAdapter, &RasLinks);
						
						for ( unsigned k = 0; k < RasLinks.nNumberOfLinks; ++k )
						{
							DWORD dwWanIp = *((PDWORD)&(RasLinks.RasLinks[k].ProtocolBuffer[584]));
							
							if(ntohl(dwWanIp) == pNetCard->m_NATIp.S_un.S_addr)
							{
								memcpy(MACClient, RasLinks.RasLinks[k].LocalAddress, ETH_ALEN);
								memcpy(MACServer, RasLinks.RasLinks[k].RemoteAddress, ETH_ALEN);

								bForceRouting = TRUE;

								break;
							}
						}
						
					}
				}
				*/
			}
			else
			{
			//	pDlg->m_ClientMTU = pNetCard->m_MTU;
			}
		}
	}

	// Initialize Request
	ZeroMemory ( &Request,		sizeof(ETH_REQUEST) );
	ZeroMemory ( &PacketBuffer, sizeof(INTERMEDIATE_BUFFER) );
	portnat_init( &TcpNatTable);
	portnat_init( &UdpNatTable);
	icmnat_init	( &IcmpNatTable);
	
	Request.EthPacket.Buffer = &PacketBuffer;

	static int Num = 0;

	do 
	{
		dwWait = WaitForMultipleObjects(dwActiveAdaptersCount, hEvents, FALSE, INFINITE);

		dwIndex = dwWait - WAIT_OBJECT_0;

		if (!dwIndex)
			continue;

		Request.hAdapterHandle = hAdapters[dwIndex]->m_hAdapter;

		// Read all queued packets from the specified interface
		while(ReadPacket(pArgs->hNdisApi,	&Request))
		{
			pEthHeader = (ether_header*)PacketBuffer.m_IBuffer;
			
			if ( ntohs(pEthHeader->h_proto) == ETH_P_IP )
			{
				pIpHeader = (iphdr*)(PacketBuffer.m_IBuffer + ETHER_HEADER_LENGTH);

				// Check if connection is established from local system (we don't do NAT processing
				// for local system)
				
				BOOL bIsLocalAddress = net_isLocalAddress(hAdapters[dwIndex], &pIpHeader->ip_src);

				if (bIsLocalAddress && (PacketBuffer.m_dwDeviceFlags == PACKET_FLAG_ON_SEND))
				{
					// Place packet on the network interface
					SendPacketToAdapter(pArgs->hNdisApi, &Request);					
					continue;
				}

				
				if((bForceRouting) &&(hAdapters[dwIndex]->m_NATState == CLIENT) && (PacketBuffer.m_dwDeviceFlags == PACKET_FLAG_ON_RECEIVE))
				{
					bNeedToBeRouted = _isNeedToForceRouting(pEthHeader->h_dest, pIpHeader->ip_dst.S_un.S_addr, pProviderCard->m_Index);
				}

				// TCP packet processing
				if (pIpHeader->ip_p == IPPROTO_TCP)
				{
					// This is TCP packet, get TCP header pointer
					pTcpHeader = (tcphdr*)(((PUCHAR)pIpHeader) + sizeof(DWORD)*pIpHeader->ip_hl);

					// Outgoing TCP packets processing
					if(((bForceRouting) && (bNeedToBeRouted) &&(hAdapters[dwIndex]->m_NATState == CLIENT)&&(PacketBuffer.m_dwDeviceFlags == PACKET_FLAG_ON_RECEIVE))
					||((!bForceRouting) && (hAdapters[dwIndex]->m_NATState == PROVIDER)&&(PacketBuffer.m_dwDeviceFlags == PACKET_FLAG_ON_SEND)))
					{
						CPortNATEntry* pTcpNE = NULL;
						
						if (pTcpHeader->th_flags == TH_SYN)
						{
							// New TCP connnection established, allocate dynamic NAT entry
							pTcpNE = portnat_Allocate(&TcpNatTable, pIpHeader->ip_src, pTcpHeader->th_sport, pIpHeader->ip_dst, pTcpHeader->th_dport);
							
							if(pTcpNE)
							{
								pTcpNE->base.m_IpNAT = bForceRouting?pProviderCard->m_NATIp:hAdapters[dwIndex]->m_NATIp;
							}

							_checkMTUCorrelation(pArgs, &PacketBuffer, pIpHeader, pTcpHeader);
						}
						else
						{
							// Try to locate existing NAT entry
							pTcpNE = portnat_Find(&TcpNatTable, pIpHeader->ip_src, pTcpHeader->th_sport, pIpHeader->ip_dst, pTcpHeader->th_dport);
						}

						if (pTcpNE)
						{
							// If NAT entry is found perform NAT processing
							pIpHeader->ip_src.S_un.S_addr = htonl(pTcpNE->base.m_IpNAT.S_un.S_addr);
							pTcpHeader->th_sport = htons(pTcpNE->m_usNATPort);
							// Recalculate checksums
							RecalculateTCPChecksum (&PacketBuffer);
							RecalculateIPChecksum (&PacketBuffer);

							if (bForceRouting)
							{
								memcpy(pEthHeader->h_dest, MACServer, ETHER_ADDR_LENGTH);
								memcpy(pEthHeader->h_source, MACClient, ETHER_ADDR_LENGTH);

								PacketBuffer.m_dwDeviceFlags = PACKET_FLAG_ON_SEND;

								Request.hAdapterHandle = pProviderCard->m_hAdapter;
								
								goto finish;
							}
						}
					}
					
					// Incoming TCP packets processing
					if ((hAdapters[dwIndex]->m_NATState == PROVIDER)&&
					   (PacketBuffer.m_dwDeviceFlags == PACKET_FLAG_ON_RECEIVE))
					{
						// Map connection to the NAT entry if the one exists
						CPortNATEntry* pTcpNE = portnat_Map(&TcpNatTable, pTcpHeader->th_dport);
						if (pTcpNE)
						{
							// NAT entry exists, make NAT processing
							if (htonl(pTcpNE->base.m_IpDst.S_un.S_addr) == pIpHeader->ip_src.S_un.S_addr)
							{
								pIpHeader->ip_dst.S_un.S_addr = htonl(pTcpNE->base.m_IpSrc.S_un.S_addr);
								pTcpHeader->th_dport = htons(pTcpNE->m_usSrcPort);
								RecalculateTCPChecksum (&PacketBuffer);
								RecalculateIPChecksum (&PacketBuffer);
							}
						}
					}

								
				}
				// UDP packets processing
				if (pIpHeader->ip_p == IPPROTO_UDP)
				{
					// This is UDP packet, get UDP header pointer
					pUdpHeader = (udphdr*)(((PUCHAR)pIpHeader) + sizeof(DWORD)*pIpHeader->ip_hl);

					
					 //DNS hook
					 //If we receive DNS packet on the NAT client adapter then we redirect it 
					 //to this system configured DNS server
					if((pArgs->dNSIp.S_un.S_addr != INADDR_ANY) && (pArgs->dNSIp.S_un.S_addr != INADDR_NONE))
					{
						if ((hAdapters[dwIndex]->m_NATState == CLIENT)&&
					   (PacketBuffer.m_dwDeviceFlags == PACKET_FLAG_ON_RECEIVE))
						{
							if (ntohs(pUdpHeader->th_dport) == 53/*DNS port*/)
							{
								// Save the DNS IP used by the NAT client system
								hAdapters[dwIndex]->m_LocalDNS.S_un.S_addr = ntohl(pIpHeader->ip_dst.S_un.S_addr);
								
								pIpHeader->ip_dst.S_un.S_addr = pArgs->dNSIp.S_un.S_addr;
							
								if(bForceRouting) 
								{
									bNeedToBeRouted = _isNeedToForceRouting(pEthHeader->h_dest, pIpHeader->ip_dst.S_un.S_addr, pProviderCard->m_Index);
								}

								RecalculateIPChecksum (&PacketBuffer);

							}
						}

					
						// DNS reply came, substitute source IP back to the original DNS address
						if ((hAdapters[dwIndex]->m_NATState == CLIENT)&&
						   (PacketBuffer.m_dwDeviceFlags == PACKET_FLAG_ON_SEND))
						{
							if (ntohs(pUdpHeader->th_sport) == 53/*DNS port*/)
							{
								pIpHeader->ip_src.S_un.S_addr = htonl(hAdapters[dwIndex]->m_LocalDNS.S_un.S_addr);
								RecalculateIPChecksum (&PacketBuffer);
							}
						}
					}
					// Outgoing UDP NAT processing
					if(((bForceRouting) && (bNeedToBeRouted) &&(hAdapters[dwIndex]->m_NATState == CLIENT)&&(PacketBuffer.m_dwDeviceFlags == PACKET_FLAG_ON_RECEIVE))
						||
						((!bForceRouting) &&(hAdapters[dwIndex]->m_NATState == PROVIDER)&&(PacketBuffer.m_dwDeviceFlags == PACKET_FLAG_ON_SEND)))
					{
						CPortNATEntry* pUdpNE = NULL;
						// Try to find existing entry
						pUdpNE = portnat_Find(&UdpNatTable, pIpHeader->ip_src, pUdpHeader->th_sport, pIpHeader->ip_dst, pUdpHeader->th_dport);
						// If not found -> allocate a new one
						if (!pUdpNE)
						{
							pUdpNE = portnat_Allocate(&UdpNatTable, pIpHeader->ip_src, pUdpHeader->th_sport, pIpHeader->ip_dst, pUdpHeader->th_dport);
							
							if(pUdpNE)
							{
								pUdpNE->base.m_IpNAT = bForceRouting?pProviderCard->m_NATIp:hAdapters[dwIndex]->m_NATIp;
	 						}
						}
						// NAT processing
						if (pUdpNE)
						{
							pIpHeader->ip_src.S_un.S_addr = htonl(pUdpNE->base.m_IpNAT.S_un.S_addr);
							pUdpHeader->th_sport = htons(pUdpNE->m_usNATPort);
							RecalculateUDPChecksum (&PacketBuffer);
							RecalculateIPChecksum (&PacketBuffer);

							if (bForceRouting)
							{
								memcpy(pEthHeader->h_dest, MACServer, ETHER_ADDR_LENGTH);
								memcpy(pEthHeader->h_source, MACClient, ETHER_ADDR_LENGTH);

								PacketBuffer.m_dwDeviceFlags = PACKET_FLAG_ON_SEND;

								Request.hAdapterHandle = pProviderCard->m_hAdapter;

								goto finish;
							}
						}
					}
					// Incoming UDP packets processing
					if ((hAdapters[dwIndex]->m_NATState == PROVIDER)&&
					   (PacketBuffer.m_dwDeviceFlags == PACKET_FLAG_ON_RECEIVE))
					{
						CPortNATEntry* pUdpNE = portnat_Map(&UdpNatTable, pUdpHeader->th_dport);
						if (pUdpNE)
						{
							if (htonl(pUdpNE->base.m_IpDst.S_un.S_addr) == pIpHeader->ip_src.S_un.S_addr)
							{
								pIpHeader->ip_dst.S_un.S_addr = htonl(pUdpNE->base.m_IpSrc.S_un.S_addr);
								pUdpHeader->th_dport = htons(pUdpNE->m_usSrcPort);
								RecalculateUDPChecksum (&PacketBuffer);
								RecalculateIPChecksum (&PacketBuffer);
							}
						}
						
					}
				}

				// ICMP packets processing
				if (pIpHeader->ip_p == IPPROTO_ICMP)
				{
					// This is UDP packet, get UDP header pointer
					pIcmpHeader = (icmphdr*)(((PUCHAR)pIpHeader) + sizeof(DWORD)*pIpHeader->ip_hl);

					// Outgoing UDP NAT processing
					if(((bForceRouting) && (bNeedToBeRouted) &&(hAdapters[dwIndex]->m_NATState == CLIENT)&&(PacketBuffer.m_dwDeviceFlags == PACKET_FLAG_ON_RECEIVE))
						||
						((!bForceRouting) &&(hAdapters[dwIndex]->m_NATState == PROVIDER)&&(PacketBuffer.m_dwDeviceFlags == PACKET_FLAG_ON_SEND)))
					{
						CIcmpNATEntry* pIcmpNE = NULL;
						// Try to find existing entry
						pIcmpNE = icmnat_Find(&IcmpNatTable, pIpHeader->ip_src, pIpHeader->ip_dst, pIcmpHeader->id);
						// If not found -> allocate a new one
						if (!pIcmpNE)
						{
							pIcmpNE = icmnat_Allocate(&IcmpNatTable, pIpHeader->ip_src, pIpHeader->ip_dst, pIcmpHeader->id);
							
							if(pIcmpNE)
							{
								pIcmpNE->base.m_IpNAT = bForceRouting?pProviderCard->m_NATIp:hAdapters[dwIndex]->m_NATIp;
	 						}
						}
						// NAT processing
						if (pIcmpNE)
						{
							pIpHeader->ip_src.S_un.S_addr = htonl(pIcmpNE->base.m_IpNAT.S_un.S_addr);
							pIcmpHeader->id = htons(pIcmpNE->m_usNATIcmpId);
							RecalculateICMPChecksum (&PacketBuffer);
							RecalculateIPChecksum (&PacketBuffer);

							if (bForceRouting)
							{
								memcpy(pEthHeader->h_dest, MACServer, ETHER_ADDR_LENGTH);
								memcpy(pEthHeader->h_source, MACClient, ETHER_ADDR_LENGTH);

								PacketBuffer.m_dwDeviceFlags = PACKET_FLAG_ON_SEND;

								Request.hAdapterHandle = pProviderCard->m_hAdapter;

								goto finish;
							}
						}
					}
					// Incoming ICMP packets processing
					if ((hAdapters[dwIndex]->m_NATState == PROVIDER)&&
					   (PacketBuffer.m_dwDeviceFlags == PACKET_FLAG_ON_RECEIVE))
					{
						CIcmpNATEntry* pIcmpNE = icmnat_Map(&IcmpNatTable, pIcmpHeader->id);
						if (pIcmpNE)
						{
							if (htonl(pIcmpNE->base.m_IpDst.S_un.S_addr) == pIpHeader->ip_src.S_un.S_addr)
							{
								pIpHeader->ip_dst.S_un.S_addr = htonl(pIcmpNE->base.m_IpSrc.S_un.S_addr);
								pIcmpHeader->id = htons(pIcmpNE->m_usIcmpId);
								RecalculateICMPChecksum (&PacketBuffer);
								RecalculateIPChecksum (&PacketBuffer);
							}
						}
						
					}
				}
				
			}
finish:
			// Reinject packet into the stack
			if (PacketBuffer.m_dwDeviceFlags == PACKET_FLAG_ON_SEND)
			{
				// Place packet on the network interface
				SendPacketToAdapter(pArgs->hNdisApi, &Request);
			}
			else
			{
				// Indicate packet to MSTCP
				SendPacketToMstcp(pArgs->hNdisApi, &Request);
			}

			Request.hAdapterHandle = hAdapters[dwIndex]->m_hAdapter;
		}

		ResetEvent(hEvents[dwIndex]);
	

	}while (dwIndex);

// Free all NAT entries
	portnat_end(&TcpNatTable);
	portnat_end(&UdpNatTable);
	icmnat_end(&IcmpNatTable);

	printf("NAT End\n");

	for (unsigned i = 1; i < dwActiveAdaptersCount; ++i)
	{
		Mode.dwFlags = 0;
		Mode.hAdapterHandle = hAdapters[i]->m_hAdapter;

		// Set NULL event to release previously set event object
		SetPacketEvent(pArgs->hNdisApi, hAdapters[i]->m_hAdapter, NULL);

		// Close Event
		if (hEvents[i]) CloseHandle ( hEvents[i] );

		// Set default adapter mode
		SetAdapterMode(pArgs->hNdisApi, &Mode);

		// Empty adapter packets queue
		FlushAdapterPacketQueue (pArgs->hNdisApi, hAdapters[i]->m_hAdapter);
	}

	ExitThread( 0 );
	return 0;
}


//--- _isNeedToForceRouting -------------------------------------
static BOOL _isNeedToForceRouting(BYTE *MACAddress, DWORD dwDestIp, DWORD dwProviderIndex)
{
	static BYTE BroadcastAddress[ETHER_ADDR_LENGTH] ={0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; 
	DWORD dwNextHopIndex = 0;

	if(!memcmp(MACAddress, BroadcastAddress, ETHER_ADDR_LENGTH))
		return FALSE;

	GetBestInterface(dwDestIp, &dwNextHopIndex);

	if ((dwNextHopIndex == dwProviderIndex))
	{
		return TRUE;
	}

	return FALSE;
}

//--- _checkMTUCorrelation --------------------------------------------------------------
static void _checkMTUCorrelation(SNatThreadArgs *pargs, PINTERMEDIATE_BUFFER pBuffer, iphdr_ptr pIpHeader, tcphdr_ptr pTcpHeader)
{
	mss_tcp_options_ptr	pTcpOptions;

	//Check if provider MTU is bigger than client MTU everything is OK.
	if(pargs->providerCard->m_MTU >= pargs->clientCard->m_MTU)
		return;

	if(pTcpHeader->th_off == TCP_NO_OPTIONS)//No tcp options is on header
	{
		//Set mss corresponded with provider MTU
	
		pTcpOptions = (mss_tcp_options_ptr)((PUCHAR)pTcpHeader + sizeof(tcphdr));

		pTcpOptions->mss_type = MSS_TYPE;
		pTcpOptions->mss_option_length = sizeof(DWORD);
		pTcpOptions->mss_value = ntohs(pargs->providerCard->m_MTU - sizeof(iphdr) - sizeof(tcphdr));
	
		pTcpHeader->th_off += sizeof(mss_tcp_options)/sizeof(DWORD);

		pIpHeader->ip_len = ntohs(ntohs(pIpHeader->ip_len) + sizeof(mss_tcp_options));

		pBuffer->m_Length+= sizeof(mss_tcp_options);
	}
	else
	{
		//Find existed MSS TCP option in header
		PUCHAR	pOption = (PUCHAR)((PUCHAR)pTcpHeader + sizeof(tcphdr));
		
		while(TRUE)
		{
			if(*pOption == MSS_TYPE)
			{
				pTcpOptions = (mss_tcp_options_ptr)pOption;

				pTcpOptions->mss_value = ntohs(pargs->providerCard->m_MTU - sizeof(iphdr) - sizeof(tcphdr));

				return;
			}
			else
			{
				if(*pOption == 0)//End of options list
				{
					break;
				}
				
				if(*pOption == 1)//No operation (NOP, Padding) 
				{
					pOption += 1;
				}
				else
				{
					pOption += *(pOption + 1);
				}
			}

			if((unsigned)(pOption - ((PUCHAR)pTcpHeader + sizeof(tcphdr))) >= (pTcpHeader->th_off - TCP_NO_OPTIONS)*sizeof(DWORD))
				break;
		}

		//MSS options is not found in tcp options
		pOption = (PUCHAR)((PUCHAR)pTcpHeader + sizeof(tcphdr));

		memcpy(pOption + sizeof(mss_tcp_options), pOption, (pTcpHeader->th_off - TCP_NO_OPTIONS) *sizeof(DWORD));
		
		pTcpOptions = (mss_tcp_options_ptr)pOption;

		pTcpOptions->mss_type = MSS_TYPE;
		pTcpOptions->mss_option_length = sizeof(DWORD);
		pTcpOptions->mss_value = ntohs(pargs->providerCard->m_MTU - sizeof(iphdr) - sizeof(tcphdr));

		pTcpHeader->th_off += sizeof(mss_tcp_options)/sizeof(DWORD);

		pIpHeader->ip_len = ntohs(ntohs(pIpHeader->ip_len) + sizeof(mss_tcp_options));

		pBuffer->m_Length+= sizeof(mss_tcp_options);
	}
};